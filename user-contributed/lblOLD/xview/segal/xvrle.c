/* xvrle.c      Distribution 1.0   91/5/11   Extensions to XV */

/*   The Extensions to XV system is copyright (C) 1988-1991 Regents of the
University of California.  Anyone may reproduce ``Extensions to XV'',
the software in this distribution, in whole or in part, pro-
vided that:

(1)  Any copy or redistribution of Extensions to XV must show the
     Regents  of  the  University of California, through its
     Lawrence Berkeley Laboratory, as the source,  and  must
     include this notice;

(2)  Any use of this software must reference this  distribu-
     tion,  state that the software copyright is held by the
     Regents of the University of California, and  that  the
     software is used by their permission.

     It is acknowledged that the U.S. Government has  rights
in Extensions to XV under  Contract DE-AC03-765F00098 between
the U. S. Department of Energy and the University of California.

     Extensions to XV is provided as a professional academic contribu-
tion  for  joint exchange.  Thus it is experimental, is pro-
vided ``as is'', with no warranties of any kind  whatsoever,
no  support,  promise  of updates, or printed documentation.
The Regents of the University of California  shall  have  no
liability  with respect to the infringement of copyrights by
Extensions to XV, or any part thereof. */

/* Extensions to XV includes the following source code:
	xvcmap.c
	xvgam.c
	xvhips.c
	xvhisto.c
	xvmap.c
	xvrgb.c
	xvrle.c
*/

/* By Bryan Skene (skene@lbl.george.gov)
 * Lawrence Berkeley Laboratory
 * May, 1991
 */

/*
 * xvrle.c - xv load and save routines for images in the RLE format
 * Callable functions:
 *	LoadRLE(fname)
 *	WriteRLE(file_id_no, format)
 *
 */

#include "xv.h"
#include "rle.h"

#ifdef USE_STDLIB_H
#include <stdlib.h>
#else

#ifdef USE_STRING_H
#include <string.h>
#else
#include <strings.h>
#define strrchr rindex
#endif

#ifdef VOID_STAR
extern void *malloc();
#else
extern char *malloc();
#endif
extern void free();

#endif /* USE_STDLIB_H */

#ifdef __STDC__
static void BuildCmap(void);
#else
static void BuildCmap();
#endif

#define VPRINTF if (verbose || header) fprintf

/* Utah type definitions */
static rle_map *rle_colormap;

int rle_visual;

FILE	*fpin;
int	maplen;
int	width, height;
int	verbose = 0, header = 0;
char	*Progname;


/**********************************/
void read_rle_header()
{
/* remember to set fpin before calling this routine */
	int	i;

	rle_dflt_hdr.rle_file = fpin;
	rle_get_setup(&rle_dflt_hdr);

	width = rle_dflt_hdr.xmax - rle_dflt_hdr.xmin + 1;
	height = rle_dflt_hdr.ymax - rle_dflt_hdr.ymin + 1;
	VPRINTF(stderr, "Image size: %dx%d\n", width, height);

   if (rle_dflt_hdr.ncolors == 1 && rle_dflt_hdr.ncmap == 3) {
      rle_visual = PSEUDOCOLOR;
      rle_colormap = rle_dflt_hdr.cmap;
      maplen = (1 << rle_dflt_hdr.cmaplen);
      VPRINTF(stderr, "Mapped color image with a map of length %d.\n", maplen);
      }
   else if (rle_dflt_hdr.ncolors == 3 && rle_dflt_hdr.ncmap == 0) {
      rle_visual = DIRECTCOLOR;
      VPRINTF(stderr, "24 bit color image, no colormap.\n");
      }
   else if (rle_dflt_hdr.ncolors == 3 && rle_dflt_hdr.ncmap == 3) {
      rle_visual = TRUECOLOR;
      rle_colormap = rle_dflt_hdr.cmap;
      maplen = (1 << rle_dflt_hdr.cmaplen);
      VPRINTF(stderr, "24 bit color image with color map of length %d\n" ,maplen);
      }
   else if (rle_dflt_hdr.ncolors == 1 && rle_dflt_hdr.ncmap == 0) {
      rle_visual = GRAYSCALE;
      VPRINTF(stderr, "Grayscale image.\n");
      }
   else {
      fprintf(stderr,
              "ncolors = %d, ncmap = %d, I don't know how to handle this!\n",
              rle_dflt_hdr.ncolors, rle_dflt_hdr.ncmap);
      exit(-1);
      }
   if (rle_dflt_hdr.alpha == 0)
      VPRINTF(stderr, "No alpha channel.\n");
   else if (rle_dflt_hdr.alpha == 1)
      VPRINTF(stderr, "Alpha channel exists!\n");
   else {
      fprintf(stderr, "alpha = %d, I don't know how to handle this!\n",
              rle_dflt_hdr.alpha);
      exit(-1);
      }
   switch (rle_dflt_hdr.background) {
      case 0:
         VPRINTF(stderr, "Use all pixels, ignore background color.");
         break;
      case 1:
         VPRINTF(stderr,
                  "Use only non-background pixels, ignore background color.");
         break;
      case 2:
         VPRINTF(stderr,
        "Use only non-background pixels, clear to background color (default).");
         break;
      default:
         VPRINTF(stderr, "Unknown background flag!\n");
         break;
      }
   for (i = 0; i < rle_dflt_hdr.ncolors; i++)
      VPRINTF(stderr, " %d", rle_dflt_hdr.bg_color[i]);
   if (rle_dflt_hdr.ncolors == 1 && rle_dflt_hdr.ncmap == 3) {
      VPRINTF(stderr, " (%d %d %d)\n",
              rle_dflt_hdr.cmap[rle_dflt_hdr.bg_color[0]]>>8,
              rle_dflt_hdr.cmap[rle_dflt_hdr.bg_color[0]+256]>>8,
              rle_dflt_hdr.cmap[rle_dflt_hdr.bg_color[0]+512]>>8);
      }
   else if (rle_dflt_hdr.ncolors == 3 && rle_dflt_hdr.ncmap == 3) {
      VPRINTF(stderr, " (%d %d %d)\n",
              rle_dflt_hdr.cmap[rle_dflt_hdr.bg_color[0]]>>8,
              rle_dflt_hdr.cmap[rle_dflt_hdr.bg_color[1]+256]>>8,
              rle_dflt_hdr.cmap[rle_dflt_hdr.bg_color[2]+512]>>8);
      }
   else
      VPRINTF(stderr, "\n");
   if (rle_dflt_hdr.comments)
      for (i = 0; rle_dflt_hdr.comments[i] != NULL; i++)
         VPRINTF(stderr, "%s\n", rle_dflt_hdr.comments[i]);
}


/******************************************/
int LoadRLE(fname)
char *fname;
{
	rle_pixel	**scanline;
	byte		rval, gval, bval;
	byte		*pI;
	register int	i, scan, x, y;
	int		numcolors, cmaplength;
	char		colortype[60];

	fprintf(stderr, "LoadRLE:image ... loading\n");
	fpin = rle_open_f(cmd_name(argv_save), fname, "r" );
	read_rle_header();
	if (header)
		exit(0);

	vpr"LoadRLE:rle_visual(%d) width = %d, height = %d\n", rle_visual, width, height);
	/* Deal with the global XV variables */
	pWIDE = width;
	pHIGH = height;
	pic = (byte *) malloc(pWIDE * pHIGH, sizeof(byte));

	/* rle allocation */
	if(!(rle_row_alloc(&rle_dflt_hdr, &scanline) == 0)) {
		fprintf(stderr, "rle row allocation failed!\n");
		exit(-1);
	}

	/* if there's a cmap, load it */
	if(rle_dflt_hdr.ncmap != 0) {
		/* then there's a li'l old colr map here! */
		vpr"LoadRLE:ncmap = %d, cmaplen = %d\n", rle_dflt_hdr.ncmap, rle_dflt_hdr.cmaplen);
		numcolors = cmaplength = (0x1 << rle_dflt_hdr.cmaplen);
		vpr"LoadRLE:reading colormap with %d entries\n", cmaplength);
		for(i = 0; i < cmaplength; i++) {
			r[i] = rle_colormap[i] >> 8;
			if(rle_dflt_hdr.ncmap > 1) g[i] = rle_colormap[i + 256] >> 8;
			if(rle_dflt_hdr.ncmap > 2) b[i] = rle_colormap[i + 512] >> 8;
			if(rle_dflt_hdr.ncmap > 1) vpr"LoadRLE:g[%d] = %d, ", i, g[i]);
			if(rle_dflt_hdr.ncmap > 2) vpr"LoadRLE:b[%d] = %d\n", i, b[i]);
		}
	}
	else {
		for(i = 0; i < 256; i++) /* we are dealing with gray-scale */
			r[i] = g[i] = b[i] = i;
		numcolors = 256;
	}

	if(rle_visual == TRUECOLOR || rle_visual == DIRECTCOLOR) {
		vpr"LoadRLE:malloc-ing pic24 ...\n");
		p24WIDE = width * 3;
		p24HIGH = height;
		free(pic24);
		pic24 = (byte *) malloc(p24WIDE * p24HIGH, sizeof(byte));
		if (!pic24) FatalError("couldn't malloc 'pic24'");
		pI = pic24 + p24HIGH * p24WIDE - p24WIDE;
	}
	else pI = pic + height * width - width;

	/* loop through the scan lines */
	vpr"LoadRLE:looping through the scan lines\n");
	for(scan = 0; scan < height; scan++) {
		SetISTR(ISTR_INFO, "Reading scan line %3d/%3d", scan, height);
		y = rle_getrow(&rle_dflt_hdr, scanline);
		switch(rle_visual) {
		case GRAYSCALE:		/* 8 bits without colormap */
			vpr"LoadRLE:getting gray-scale scan line %d\n", scan);
			for(x = 0; x < width; x++) {
				*pI++ = rval = scanline[0][x];
				gval = scanline[0][x];
				bval = scanline[0][x];
			}
			pI = pI - 2 * width;
			break;

		case TRUECOLOR:		/* 24 bits with colormap */
			vpr"LoadRLE:getting true-color scan line %d\n", scan);
			for(x = 0; x < width; x++) {
				*pI++ = rval = rle_colormap[scanline[0][x]] >> 8;
				*pI++ = gval = rle_colormap[scanline[1][x] + 256] >> 8;
				*pI++ = bval = rle_colormap[scanline[2][x] + 512] >> 8;
			}
			pI = pI - 2 * p24WIDE;
			break;

		case DIRECTCOLOR: 	/* 24 bits without colormap */
			vpr"LoadRLE:getting direct-color scan line %d\n", scan);
			for(x = 0; x < width; x++) {
				*pI++ = rval = scanline[0][x];
				*pI++ = gval = scanline[1][x];
				*pI++ = bval = scanline[2][x];
			}
			pI = pI - 2 * p24WIDE;
			break;

		case PSEUDOCOLOR:	/* 8 bits with colormap */
			vpr"LoadRLE:getting pseudocolor scan line %d\n", scan);
			for(x = 0; x < width; x++, pI++) {
				rval = rle_colormap[scanline[0][x]] >> 8;
				gval = rle_colormap[scanline[0][x] + 256] >> 8;
				bval = rle_colormap[scanline[0][x] + 512] >> 8;
				/* Do stuff with r, g, b like above */
				*pI = rval;
			}
			pI = pI - 2 * width;
			break;

		default:
			fprintf(stderr, "LoadRLE:rle_visual unrecognized\n");
			break;
		}
	}

	i = 0;
	if(rle_visual == TRUECOLOR || rle_visual == DIRECTCOLOR) {
		vpr"numcolors = %3d\n", numcolors);

		i = Conv24to8(pic24, width, height, numcolors);
	}

	fclose(fpin);

	switch(rle_visual) {
	case GRAYSCALE :
		sprintf(colortype, "Gray Scale");
		break;

	case TRUECOLOR :
		sprintf(colortype, "True Color");
		break;
		
	case DIRECTCOLOR :
		sprintf(colortype, "Direct Color");
		break;
		
	case PSEUDOCOLOR :
		sprintf(colortype, "Pseudo Color");
		break;
		
	default :
		break;
	}

	SetISTR(ISTR_FORMAT, "RLE %s, %d color channel(s) in %d stored colormap(s).", colortype, rle_dflt_hdr.ncolors, rle_dflt_hdr.ncmap);


	if(!i) fprintf(stderr, "LoadRLE:image ... obtained\n");
	return(i);
}


/********************************************/
int WriteRLE(fpout, format)
FILE *fpout;
int format;
{
/* Writes an RLE file in the loaded file format (could be easily extended to
   allow changes between GRAYSCALE, TRUECOLOR, DIRECTCOLOR, PSEUDOCOLOR -
   just add some selectively active buttons to dirW that determine certain
   values in the rle_dflt_hdr (like ncmap, ncolors, etc.))
   NOTE: will only work if the original image was RLE - this could be changed
   too.  I don't have time now.		- Bryan Skene
*/
	int i, x, y, index, rval, gval, bval;
	int offset_from_bottom;
	rle_pixel **scandata;

	/* all we will change in the header is the file pointer ...
	   see above comment - could change more here to give flexibility
	*/
	if(format == 1) { /* Gray Scale Requested */
		rle_dflt_hdr.ncolors = 1;
		rle_dflt_hdr.ncmap = 0;
		rle_dflt_hdr.cmap = NULL;
		rle_dflt_hdr.cmaplen = 0;
	}
	else if(format == 3) { /* True Color Requested */
		rle_dflt_hdr.ncolors = 3;
		rle_dflt_hdr.ncmap = 3; /* 3 color channel maps */
		if(rle_dflt_hdr.cmap == NULL) {
			BuildCmap();
			rle_dflt_hdr.cmap = rle_colormap;
			rle_dflt_hdr.cmaplen = 8;
		}
	}
	else if(format == 4) { /* Direct Color Requested */
		rle_dflt_hdr.ncolors = 3;
		rle_dflt_hdr.ncmap = 0; /* no color channel maps */
		rle_dflt_hdr.cmap = NULL;
		rle_dflt_hdr.cmaplen = 0;
	}

	rle_dflt_hdr.rle_file = fpout;

        /* rle allocation */
	if(!(rle_row_alloc(&rle_dflt_hdr, &scandata) == 0)) {
		fprintf(stderr, "rle row allocation failed!\n");
		exit(-1);
	}

	/* Do the Writing */
	vpr"WriteRLE: Writing the data\n");
	rle_put_setup(&rle_dflt_hdr);

	if(rle_visual == DIRECTCOLOR || rle_visual == TRUECOLOR) {
		vpr"WriteRLE: Original image was 24 bit ...\n");
	}

	/* visit all the triples in the original 24 bit colormap */
	for(y = 0; y < p24HIGH; y++) {
		for(x = 0; x < p24WIDE/3; x++) {
			offset_from_bottom = p24WIDE * (p24HIGH - y - 1);
			rval = pic24[offset_from_bottom + 3*x + 0];
			gval = pic24[offset_from_bottom + 3*x + 1];
			bval = pic24[offset_from_bottom + 3*x + 2];
			
			index = epic[eWIDE * (eHIGH - y - 1) + x];

			rval += r[index] - rorg[index];
			gval += g[index] - gorg[index];
			bval += b[index] - borg[index];

			RANGE(rval, 0, 255);
			RANGE(gval, 0, 255);
			RANGE(bval, 0, 255);

			switch(format) {
			case 1 :
				scandata[RLE_RED][x] = MONO(rval, gval, bval);
				break;

			case 3 :
			case 4 :
				scandata[RLE_RED][x] = rval;
				scandata[RLE_GREEN][x] = gval;
				scandata[RLE_BLUE][x] = bval;
				break;

			default :
				break;
			}
		}
		/* write the scan line */
		SetISTR(ISTR_INFO, "Writing scan line %3d/%3d", y, p24HIGH);
		rle_putrow(scandata, rle_dflt_hdr.xmax, &rle_dflt_hdr);
	}
	rle_puteof(&rle_dflt_hdr);
	vpr"WriteRLE: Done writing\n");
	return (0);
}


/********************************************/
static void BuildCmap()
{
	unsigned short i;

	rle_colormap = (unsigned short *) calloc(3 * 256, sizeof(unsigned short));

	for(i = 0; i < 256; i++) {
		rle_colormap[i      ] = (unsigned short) i << 8;
		rle_colormap[i + 256] = (unsigned short) i << 8;
		rle_colormap[i + 512] = (unsigned short) i << 8;
	}
}


/********************************************/


/********************************************/


/********************************************/


/********************************************/


/********************************************/
