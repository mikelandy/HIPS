#ifndef lint
static char sccsid[] = "%W% %G%";
#endif
/*
	Copyright 1988 Alan Shaw and Eric Schwartz.
	No part of this software may be distributed or sold without the prior
	agreement of Prof. Eric Schwartz, Dept. of Psychiatry, NYU School of
	Medicine, 550 1st Ave., New York, New York, 10016.
 
	Changelog:
	12/15/08 - rld - re-defined INFINITY to INFINITY_HIPS for compatibility with OSX
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<math.h>

#include	<hipl_format.h>
#include	"cscan.h"

#define        SIZE_LUT 256

int		OUTBINS = 256;
int		PIXELS, ROWS, COLUMNS;
static h_boolean	DITHER;
static h_boolean	EUCLIDEAN = FALSE;	/* whether mapping of input colors into
					lut values is to be done by Euclidean
					distance */
static int	(*scan)();
static void	(*mapping)();

u_char		*lr, *lg, *lb;	/* output lut buffers */

static Flag_Format flagfmt[] = {
	{"s",{LASTFLAG},1,{{PTSTRING,"p","scanflag"},LASTPARAMETER}},
	{"m",{LASTFLAG},1,{{PTSTRING,"e","mappingflag"},LASTPARAMETER}},
	{"b",{LASTFLAG},1,{{PTINT,"256","outbins"},LASTPARAMETER}},
	{"d",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,LASTTYPE};
void error(),dither(),advance();

int main(argc, argv)
  int argc;
  char **argv;
{
  struct header	hd,hdr,hdg,hdb;
  int		method,rc;
  u_char        *red_inbuf, *green_inbuf, *blue_inbuf, *image_outbuf, 
                *luts_outbuf;
  void		colorscan();
  int		pscan(), mscan();
  void		mapping_peano(), mapping_grow();
  int		i;
  char		*SCANFLAG, *MAPPINGFLAG;
  Filename filename;
  FILE *fp;
  
  Progname = strsave(*argv);
  parseargs(argc,argv,flagfmt,&SCANFLAG,&MAPPINGFLAG,&OUTBINS,&DITHER,
	FFONE,&filename);
  fp = hfopenr(filename);
  
  switch (SCANFLAG[0]) {
    case 'p':
      scan = pscan;
      break;

    case 'm':
      scan = mscan;
      break;

    default:
      scan = pscan;
      break;
    }
  
  switch (MAPPINGFLAG[0]) {
    case 'p':
      if (scan != pscan)
	error("p mapping requires p scan");
      mapping = mapping_peano;
      break;

    case 'g':
      mapping = mapping_grow;
      break;

    case 'e':
    default:
      EUCLIDEAN = TRUE;
      break;
    }
  fread_hdr_a(fp,&hd,filename);
  method = fset_conversion(&hd,&hdr,types,filename);
  dup_headern(&hdr,&hdg);
  alloc_image(&hdg);
  dup_headern(&hdr,&hdb);
  alloc_image(&hdb);
  
  rc = hdr.orows * hdr.ocols;
  ROWS = hdr.orows;
  COLUMNS = hdr.ocols;
  PIXELS = rc;
  if (hdr.num_frame/hdr.numcolor != 1)
	perr(HE_MSG,"can't handle more than one frame sequences");
  if (hdr.numcolor != 3)
	perr(HE_MSG,"number of color planes must be precisely 3");
  fread_imagec(fp,&hd,&hdr,method,0,filename);
  fread_imagec(fp,&hd,&hdg,method,1,filename);
  fread_imagec(fp,&hd,&hdb,method,2,filename);
  fclose(fp);
  
  lr = luts_outbuf = (u_char *) halloc(768, sizeof(u_char));
  lg = lr + SIZE_LUT;
  lb = lg + SIZE_LUT;
  
  colorscan(hdr.image, hdg.image, hdb.image, hdb.image, lr, lg, lb);

  hdb.numcolor = 1;
  setparam(&hdb, "cmap", PFBYTE, 3 * SIZE_LUT, luts_outbuf);
  hdb.num_frame = hdb.numcolor = 1;
  write_headeru(&hdb,argc,argv);
  fwrite(hdb.image, PIXELS, 1, stdout);
  exit(0);
}

/* //////////////////////////////////////////////////////////////////////// */

/* The following function puts lut indices into image_outbuf and lut
*  values into lr, lg, and lb.  This is done in two phases: choosing
*  a colormap and mapping original colors to colormap representatives
*  (lut values).
*/

u_char	LUTMAP[LEVELS][LEVELS][LEVELS];

void colorscan(red_inbuf, green_inbuf, blue_inbuf, image_outbuf, lr, lg, lb)
  u_char *red_inbuf, *green_inbuf, *blue_inbuf, *image_outbuf, *lr, *lg, *lb;
{
  register int	i;
  u_char   lutmap();
  void     distsq_setup(), cellist_setup();
  
  scan(red_inbuf, green_inbuf, blue_inbuf);
  
  /* At this point the lut colors have been selected; now, for
     each pixel in the original image, image_outbuf will be filled
     with the nearest lut value as returned from the lut map,
     after possible error propagation (dither) from previously processed
     pixels.
     */
  
  if (EUCLIDEAN) {
      distsq_setup();
#ifdef LOCALSEARCH
      cellist_setup();
#endif
    }
  else
    mapping();
  
  /*	If the mapping is being done by Peano-index or grow,
	then the entire lutmap has already been constructed;
	otherwise map by (square of) true Euclidean distance in rgb space: */
  
  if (DITHER)
    dither(red_inbuf, green_inbuf, blue_inbuf, image_outbuf, lr, lg, lb);
  else {
      if (EUCLIDEAN)
	for (i = 0; i < PIXELS; i++)
	  *image_outbuf++ = 
	    lutmap(*red_inbuf++, *green_inbuf++, *blue_inbuf++);
      else
	for (i = 0; i < PIXELS; i++)
	  *image_outbuf++ =
	    (*(*(*(LUTMAP + trunc(*red_inbuf++))
		 + trunc(*green_inbuf++)) + trunc(*blue_inbuf++)));
    }
}
 
/* //////////////////////////////////////////////////////////////////////// */

struct triple {
    short	r, g, b;
  };

void dither(redbuf, greenbuf, bluebuf, imagebuf, lr, lg, lb)
  u_char	*redbuf, *greenbuf, *bluebuf, *imagebuf, *lr, *lg, *lb;
{
  int			i;
  int			error_red = 0, error_green = 0, error_blue = 0;
  int			three_eighths_red, three_eighths_green,
  three_eighths_blue,
  quarter_red, quarter_green, quarter_blue;
  int			R, G, B;
  short			outindex;
  u_char			lutmap();
  struct triple		*adjbuf, *thisline, *nextline;
  
  if ((adjbuf = (struct triple *)
       calloc(2 * COLUMNS, sizeof(struct triple))) == NULL)
    error("can't allocate core");
  
  thisline = adjbuf;
  nextline = adjbuf + COLUMNS;
  
  for (i = 0;
       i < PIXELS;
       i++, redbuf++, greenbuf++, bluebuf++, imagebuf++) {
      R = (int)(*redbuf) + thisline->r;
      if (R > 255) {
	  error_red += R - 255;
	  R = 255;
	}
      else if (R < 0) {
	  error_red += R;
	  R = 0;
	}
      G = (int)(*greenbuf) + thisline->g;
      if (G > 255) {
	  error_green += G - 255;
	  G = 255;
	}
      else if (G < 0) {
	  error_green += G;
	  G = 0;
	}
      B = (int)(*bluebuf) + thisline->b;
      if (B > 255) {
	  error_blue += B - 255;
	  B = 255;
	}
      else if (B < 0) {
	  error_blue += B;
	  B = 0;
	}
      
      thisline->r = thisline->g = thisline->b = 0;
      
      if (EUCLIDEAN)
	*imagebuf = lutmap((u_char)R, (u_char)G, (u_char)B);
      else
	*imagebuf =
	  (*(*(*(LUTMAP + trunc(R)) + trunc(G)) + trunc(B)));
      outindex = (short) *imagebuf;
      
      
      error_red	+= (int)(*redbuf) - (int)(*(lr + outindex));
      error_green	+= (int)(*greenbuf) - (int)(*(lg + outindex));
      error_blue	+= (int)(*bluebuf) - (int)(*(lb + outindex));
      
      quarter_red	= error_red / 4;
      quarter_green	= error_green / 4;
      quarter_blue	= error_blue / 4;
      three_eighths_red = quarter_red + error_red / 8;
      three_eighths_green = quarter_green + error_green / 8;
      three_eighths_blue = quarter_blue + error_blue / 8;
      
      error_red = error_green = error_blue = 0;
      
      if (i + COLUMNS < PIXELS) {		/* not last row */
	  nextline->r	+= three_eighths_red;
	  nextline->g	+= three_eighths_green;
	  nextline->b	+= three_eighths_blue;
	}
      
      if ((i + COLUMNS < PIXELS)
	  &&  (i % COLUMNS != COLUMNS - 1)) {
	  (nextline + 1)->r	+= quarter_red;
	  (nextline + 1)->g	+= quarter_green;
	  (nextline + 1)->b	+= quarter_blue;
	}
      
      if (i % COLUMNS != COLUMNS - 1) {	/* not last column */
	  (thisline + 1)->r	+= three_eighths_red;
	  (thisline + 1)->g	+= three_eighths_green;
	  (thisline + 1)->b	+= three_eighths_blue;
	}
      
      thisline++, nextline++;
      if (thisline - adjbuf == 2 * COLUMNS)
	thisline = adjbuf;
      else if (nextline - adjbuf == 2 * COLUMNS)
	nextline = adjbuf;
    }
  
  free(adjbuf);
}

/* //////////////////////////////////////////////////////////////////////// */

static h_boolean	mapped[LEVELS][LEVELS][LEVELS];

extern short	**redsquares, **greensquares, **bluesquares;
/* these are referred to by the macro distsq. */

u_char lutmap(rval, gval, bval)
  u_char rval, gval, bval;
{
  short		R = trunc(rval), G = trunc(gval), B = trunc(bval);
  h_boolean	*mapt;
  short		i, point, distance, nudistance;
  struct Cellist	*cellist();
  
#ifdef	LOCALSEARCH
  struct Cellist	*ptr;
#endif
  
  if (*(mapt = *(*(mapped + R) + G) + B))
    return(*(*(*(LUTMAP + R) + G) + B));
  
  point = 0;
  distance = INFINITY_HIPS;
  
#ifdef LOCALSEARCH
  
  for (ptr = cellist(R, G, B);
       ptr && distance > ptr->celldistsq;
       ptr = ptr->next)
    if ((nudistance = distsq(ptr->bindex, R, G, B)) < distance) {
	distance = nudistance;
	point = ptr->bindex;
      }
  
#else
  
  for (i = 0; i < OUTBINS; i++)
    if ((nudistance = distsq(i, R, G, B)) < distance) {
	distance = nudistance;
	point = i;
      }
  
#endif
  
  *mapt	= TRUE;
  return(*(*(*(LUTMAP + R) + G) + B) = (u_char)point);
}


/* //////////////////////////////////////////////////////////////////////// */

struct frontier {
    short		r, g, b;
    struct frontier	*next;
  } **FRONTIER;

void mapping_grow()	/* map by distance (more or less) in rgb space;
			   this is done by growing cubical regions out from
			   the lut points simultaneously */
{
  register short	i;
  h_boolean	done = FALSE;
  struct frontier	*falloc();
  void		falloc_setup();
  h_boolean	odd = TRUE;
  h_boolean	*mapt;
  
  FRONTIER = (struct frontier **)
    calloc(OUTBINS, sizeof(struct frontier *));
  
  falloc_setup();
  
  for (i = 0; i < OUTBINS; i++)
    if (!
	*(mapt =
	  *(*(mapped+trunc(*(lr+i)))+trunc(*(lg+i)))+trunc(*(lb+i)))) {
	/* test for duplicate lut points */
	*(*(*(LUTMAP+trunc(*(lr+i)))+trunc(*(lg+i)))+trunc(*(lb+i)))
	  = (u_char)i;
	*mapt	= TRUE;
	*(FRONTIER + i) = falloc();
	(*(FRONTIER + i))->r = trunc(*(lr + i));
	(*(FRONTIER + i))->g = trunc(*(lg + i));
	(*(FRONTIER + i))->b = trunc(*(lb + i));
      }
  
  while (!done) {
      done = TRUE;
      if (odd) {
	  for (i = 0; i < OUTBINS; i++)
	    /* actually this shd be done in parallel */
	    if (*(FRONTIER + i)) {
		advance(i);
		done = FALSE;
	      }
	}
      else {
	  for (i = OUTBINS - 1; i >= 0; i--)
	    if (*(FRONTIER + i)) {
		advance(i);
		done = FALSE;
	      }
	}
      odd = !odd;
      /* we reverse direction each time through so as not to
	 always favor lower-numbered lut points and let them
	 swallow up others */
    }
}

/* //////////////////////////////////////////////////////////////////////// */

void advance(i)
  int i;
{
  struct frontier	*newfront, *nfront, *front, *falloc();
  short		R, G, B;
  
  newfront = NULL;
  for (front = *(FRONTIER + i); front; front = front->next)
    for (R = front->r - 1; R <= front->r + 1; R++)
      if (R >= 0 && R < LEVELS)
	for (G = front->g - 1; G <= front->g + 1; G++)
	  if (G >= 0 && G < LEVELS)
	    for (B = front->b - 1; B <= front->b + 1; B++)
	      if ((B >= 0) && (B < LEVELS)
		  &&  (! *(*(*(mapped + R) + G) + B))) {
		  *(*(*(LUTMAP + R) + G) + B) = (u_char)i;
		  *(*(*(mapped + R) + G) + B) = TRUE;
		  nfront = falloc();
		  nfront->r = R;
		  nfront->g = G;
		  nfront->b = B;
		  nfront->next = newfront;
		  newfront = nfront;
		}
  *(FRONTIER + i) = newfront;
}


/* //////////////////////////////////////////////////////////////////////// */

static struct frontier *MEM;

void falloc_setup()
{
  if ((MEM = (struct frontier *)
       calloc(COLORS, sizeof(struct frontier))) == NULL)
    error("can't allocate core");
}

/* //////////////////////////////////////////////////////////////////////// */

struct frontier *falloc()
{
  static int index = 0;
  
  if (index >= COLORS)
    error("error in frontier allocation");
  
  return(MEM + index++);
}

/* //////////////////////////////////////////////////////////////////////// */

void error(message)
  char *message;
{
  fprintf(stderr, "%s: %s\n", Progname, message);
  exit(1);
}
