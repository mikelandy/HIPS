/*
 *  greyrectx.c: display a hips image in a gfxtool and 
 *    print out coordinates of a box in the image.
 *  Authors:  Steven Tylock and Paul W. Palumbo
 *    SUNY at Buffalo Department of Computer Science
 *    January, 1987
 *    
 *  This program contains some original source from the hips 
 *    package.  Our original contributions are in the public domain.
 */

#include <hipl_format.h>
#include <usercore.h>
#include <math.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <pixrect/pixrect_hs.h>
#define XDEFAULT_VWSURF(ddname) {"", "", 0, ddname, 0, 256, "", 0, 0}
#define max(x,y) ((x)<(y) ? (y) : (x))
#define MASK8 0377

int pixwindd();
int cgpixwindd();
int bw2dd();
int cg2dd();
/*struct vwsurf surface = DEFAULT_VWSURF(pixwindd);*/
struct vwsurf surface = XDEFAULT_VWSURF(cgpixwindd);

char *bp;
short *sp;
int t=200,sr,sc,r,c,in_r,in_c;
int win_wid, win_hgt;
struct suncore_raster raster;
struct suncore_raster raster2;
char fname[256];
FILE *fp;
int factor;
int endit = TRUE;
char *Progname;

main(argc,argv)
int argc;
char *argv[];
{
  struct header hd;
  int i;
  unsigned char *outpic;
  unsigned char *reduce();

  Progname = strsave(*argv);
  for (i=1; i<argc; ++i) {
   if (argv[i][0] == '-') {
     switch (argv[i][1]) {
       case 't': t = atoi(argv[++i]);
                 break;
       default:  (void)fprintf(stderr,"bad option: -%c\n",argv[i][1]);
                 exit(4);
     }
   }
   else { (void)strcpy (fname,argv[i]);}
  }
  if (fname[0]  == '\0') fp = stdin;
  else if ((fp=fopen(fname,"r")) == NULL ) {
	(void)fprintf (stderr,"cannot open %s\n",fname); exit(0);}
  fread_header(fp,&hd,fname);
  (void)strcat(fname,".zipcode");
  in_r = hd.orows;
  in_c = hd.ocols;

  if ((bp = (char *) calloc(in_r,in_c)) == NULL) { 
    (void)fprintf(stderr,"bad calloc\n"); exit(1);
  }
  if (fread(bp,in_r*in_c,1,fp) != 1) {
        (void)fprintf(stderr,"bad read\n"); exit(2); }
  init_graphics();

  size_raster(&surface,0.0,1.0,0.0,1.0,&raster2);
  win_wid = raster2.width;
  win_hgt = raster2.height;
  outpic = reduce(bp,in_r,in_c,win_wid, &r, &c);

  sr = r;
/*  sc = (c+15) / 16;*/
  sc = (c+1) / 2;
  if ((sp = (short *) calloc(sr*sc,sizeof(short))) == NULL) {
    (void)fprintf(stderr,"bad calloc\n"); exit(1);
  }

for (;(endit ==TRUE);) {
  drawraster(sp,outpic,r,c);
  drawbox(bp,outpic,fname,hd);
  delete_retained_segment(1);
}

  deselect_view_surface(&surface);
  terminate_core();
}

drawraster(sp,outpic,sr,sc)
register int sr,sc;
unsigned char *outpic;
short *sp;
{
  register int ic, itc, isc, i, j;
  unsigned char *bptemp;
  short *sptemp;
  ic = (sc);
  sc = (sc+1) / 2;
  sptemp = sp;
  for (i=0; i< sr; ++i) {
    for (j=0; j< sc; ++j) { 
       *sptemp++ = 0;
    }
  }
  for (i=0; i< sr; ++i) {
    isc = i * sc;
    itc = i * ic;
    bptemp = outpic + itc;
    sptemp = sp + isc;
    for (j=0; j< ic; ++j) { 
      /* correct lookup table problem */
      if (*bptemp == 0377) *bptemp = 0;
      else if (*bptemp == 0) *bptemp = 0377;
      if (j&01) {
        *sptemp |= *bptemp;
      }
      else { 
        ++sptemp;
        *sptemp = (*bptemp << 8);
      }
      ++bptemp;
    }
  }

    create_retained_segment(1);
    raster.width = ic; raster.height = sr; raster.depth = 8; raster.bits = sp;
    put_raster(&raster);
    close_retained_segment(1);
}

drawbox(bp,outpic,filename,hd)
unsigned char *bp, *outpic;
char filename[];
struct header hd;
{
  float X1,Y1,X2,Y2,x1,x2,y1,y2,x3,y3;
  float box_wid, box_hgt;
  float box_offsetx;
  int cut_offsetx, cut_offsety;
  int cut_wid, cut_hgt;

  int button = 2;
  int button2 = 0;
  char code;

  set_echo(LOCATOR,1,1);
while (button == 2)
 {
  set_line_index(255);
  set_linestyle(SOLID);
  (void)fprintf(stderr,"Left for pixel value, middle for box corner, right to quit:\n");
  await_any_button_get_locator_2(999999999,1,&button,&x1,&y1);
  if (button == 3) { if (confirm()) exit(0); else button = 1; }
  while (button == 1)
   {int X, Y, offset;
    size_raster(&surface,0.0,1.0,0.0,1.0,&raster2);
    win_wid = raster2.width;
    win_hgt = raster2.height;
    map_ndc_to_world_2(x1,y1,&X1,&Y1);
    X = X1*win_wid;
    Y = r - Y1*win_hgt;
    offset = X + Y*c;
    if ((Y >= 0) && (X < c))
      (void)fprintf(stderr,"coordinate (%d, %d) has value: %d\n",X,Y,*(outpic + offset));
    else
      (void)fprintf(stderr,"coordinate (%d, %d) has value: <undefined>\n",X,Y);
    (void)fprintf(stderr,"Left for pixel value, middle for box corner, right to quit:\n");
    await_any_button_get_locator_2(999999999,1,&button,&x1,&y1);
    if (button == 3) { if (confirm()) exit(0); else button = 1; }
   } 
  set_linestyle(SOLID);
  set_echo_position(LOCATOR,1,x1,y1);
  set_echo(LOCATOR,1,6);
  (void)fprintf(stderr,"Any button for second box corner\n");
  await_any_button_get_locator_2(999999999,1,&button,&x2,&y2);

  if (x2 < x1) {x3=x1; x1=x2; x2=x3;}
  if (y2 < y1) {y3=y1; y1=y2; y2=y3;}
  map_ndc_to_world_2(x1,y1,&X1,&Y1);
  map_ndc_to_world_2(x2,y2,&X2,&Y2);

  create_retained_segment(2);

    set_rasterop(XORROP);
    set_linestyle(SOLID);
/*    set_linestyle(DOTTED); */
    move_abs_2(X1,Y1);
    line_abs_2(X2,Y1);
    line_abs_2(X2,Y2);
    line_abs_2(X1,Y2);
    line_abs_2(X1,Y1);
    move_abs_2(0.0,0.0);
    line_abs_2(0.0,1.0);
    line_abs_2(1.0,1.0);
    line_abs_2(1.0,0.0);
    line_abs_2(0.0,0.0);
  close_retained_segment(2);
  set_echo(LOCATOR,1,1);

    size_raster(&surface,0.0,1.0,0.0,1.0,&raster2);
    win_wid = raster2.width;
    win_hgt = raster2.height;
  box_wid = (X2 - X1) * win_wid;
  box_hgt = (Y2 - Y1) * win_hgt;
  cut_wid = (int) (box_wid + 0.5);
  cut_hgt = (int) (box_hgt + 0.5);
  box_offsetx = X1 * win_wid;
  cut_offsetx = (int)(box_offsetx + 0.5);
  cut_offsety = (int)(r - Y2*win_hgt + 0.5);
/*  (void)fprintf(stderr,"box width = %d box height = %d\n",
					cut_wid*factor,cut_hgt*factor); */
/*  (void)fprintf(stderr,"offset x = %d  offset y = %d\n",
					cut_offsetx*factor,cut_offsety*factor); */
  if ((cut_offsety <0) || (cut_offsetx > c) || ((cut_offsety + box_hgt) > r)
     || ((cut_offsetx + box_wid) > c))
     {
      (void)fprintf (stderr,"block is not contained in picture, please try again\n");
      delete_retained_segment(2);
      set_echo(LOCATOR,1,1);
      button = 2;
     }
  else {
    (void)fprintf(stderr,"Left button to set code, middle to redraw box, and right to print coordinates\n");
  await_any_button(999999999,&button);
  if (button == 1) { while (button == 1) {
      (void)fprintf(stderr,"Enter code: Left for City, Middle for State, Right for ZipCode\n");
      await_any_button(999999999,&button2);
      switch (button2) {
          case 1: code = 'C';
		  break;
          case 2: code = 'S';
		  break;
          case 3: code = 'Z';
		  break;
          default: break;
        }
      (void)fprintf(stderr,"code is %c.\n",code);
      (void)fprintf(stderr,"Left button to set code, middle to redraw box, and right to print coordinates\n");
      await_any_button(999999999,&button);
    } }
  if (button == 2) {
      delete_retained_segment(2);
      set_echo(LOCATOR,1,1);
      }
  if (button == 3) {
	(void)fprintf(stderr,"confirm box dimensions and code of: %c\n",code);
	if (confirm()) {
	  (void)printf ("%c ",code);
	  (void)printf ("%d %d %d %d\n", cut_hgt*factor,cut_wid*factor,cut_offsety*factor,cut_offsetx*factor);
/*        extract(cut_hgt*factor,cut_wid*factor,cut_offsety*factor,cut_offsetx*factor,bp,hd,filename); */
	  }
        delete_retained_segment(2);
	button = 2;
   }
  }
 }
}

extract(height,width,y_off,x_off,ifr,hd,filename)
int height,width,y_off,x_off;
unsigned char *ifr;
struct header hd;
char filename[];
{
        int r,c,nr,nc,ir,ic,or,oc;
        int lc,rc,br,tr,i,j;
        unsigned char *ip,*op;
	unsigned char *ofr;
        FILE *fd;
 
        (void)fprintf(stderr,"Height = %d, Width = %d, y_off = %d , x_off = %d\n",height,width,y_off,x_off);
        r = hd.orows;
        c = hd.ocols;
        nr=r/2; nc=c/2;
        nr=height; ir=(r - ((nr<0) ? -nr : nr))/2;
        nc=width; ic=(c - ((nc<0) ? -nc : nc))/2;
        ir=y_off;
        ic=x_off;
 
        lc=ic; br=ir;
        if(nr<0) {
                br=ir+nr+1;nr=(-nr);
        }
        if(nc<0) {
                lc=ic+nc+1;nc=(-nc);
        }
 
        rc=lc+nc-1;
        tr=br+nr-1;
        
        if(lc<0)lc=0; if(br<0)br=0;
        if(rc>=c)rc=c-1; if(tr>=r)tr=r-1;
 
        or=tr-br+1;
        oc=rc-lc+1;
 
        if(or<=0 || oc<=0 ) {
                 (void) sprintf(stderr,"extract: zero or negative dimension (or,oc) = (%d,%d).\n",or,oc);
                 error(stderr,0);
        }
 
        hd.orows=or; hd.ocols=oc;
        if ((ofr = (unsigned char *) calloc((unsigned)or*oc,sizeof(unsigned char))) == 0) {
                error("extract: can't allocate core\n",0);
        }
                 
        (void)fprintf(stderr,"filename = %s\n",filename);
        if ((fp=fopen(filename,"w")) == NULL ) {
              (void) sprintf (stderr,"cannot create %s\n",filename);
                error(stderr,0);
        }
        fwrite_header(fp,&hd,filename);
        
        if(hd.pixel_format==PFBYTE) {
                ip = ifr + lc + br*c;
                op = ofr;
                for(i=0;i<or;i++) {
                        for(j=0;j<oc;j++)
                                *op++ = *ip++;
                        ip += c-oc;
                }
                if (fwrite(ofr,or*oc*sizeof(unsigned char),1,fp)) != 1) {
                        error("extract: error during write\n",0);
                }
        }
        (void)close(fd);
        free(ofr);
}

/*
 *  reduce:  according to Landy et al. as in the Hips package
 **   usage: reduce [factor] < bigframe > smallframe
 **   default "factor": 2
 **   if "rows" or "cols" are not divisible by "factor", they are
 **   reduced to the highest number which is divisible by "factor".
  bp = reduce(bp,in_r,in_c,win_wid, &r, &c);
 */

unsigned char *
reduce (inpic,in_r,in_c,win_wid, rowsout, colsout)
unsigned char *inpic;
int in_r, in_c, win_wid;
int *rowsout, *colsout;
{
        register int i_fact_cols, f2, hf, sum, colsmf, factor_cols;
        register int i,j,k,mf;
        register unsigned char  *pin, *pout, *outpic ;

        factor = (max(in_r,in_c) + win_wid - 1) / win_wid;
        (*rowsout)=in_r/factor ;
        (*colsout)=in_c/factor ;
/*(void)fprintf(stderr,"in_r=%d in_c=%d win_wid=%d\n",in_r,in_c,win_wid); */
        (void)fprintf(stderr,"Factor = %d\n",factor);
        if (factor == 1) return(inpic);
        colsmf = (*colsout)*factor;
        factor_cols = factor* in_c;
        if (factor < 1 || factor > 512) {
                error("reduce: unreasonable factor\n",0);
        }

      if(!(outpic=(unsigned char *)calloc((unsigned)((*rowsout)*(*colsout)),sizeof(char)))){
                error("reduce: not enough core\n",0);
        }
        f2=factor*factor ;
        hf=f2/2 ;
        
        pout=outpic ;
        if (factor != 1) {
          for (i=0 ; i< (*rowsout) ; i++){
            i_fact_cols = i*factor_cols;
            for (mf=0 ; mf<colsmf ; mf += factor) {
                sum=hf ;
                pin=inpic + i_fact_cols + mf;
                for(j=0;j<factor;j++,pin+= in_c)
                  for (k=0 ; k<factor ; k++) sum+= ((*(pin+k)) & MASK8 ) ;
                *pout++ =(sum/f2) ;
            }     
          }
        }   
          
  return(outpic);
}

init_graphics()
{
  float red[256],green[256],blue[256];
  int i;

  if (initialize_core(BUFFERED,SYNCHRONOUS,TWOD)) exit(1);
  if (initialize_view_surface(&surface,FALSE)) exit(2);
  if (select_view_surface(&surface)) exit(3);
  size_raster(&surface,0.0,1.0,0.0,1.0,&raster2);
  win_wid = raster2.width;
  win_hgt = raster2.height;

  for (i=0; i<256; ++i) { blue[i] = green[i] = red[i] = ((float)i)/255; }
  blue[255] = green[255] = red[255] = 0.0;
  blue[0] = green[0] = red[0] = 0.0;

  set_rasterop(NORMAL);
  define_color_indices(&surface,0,255,red,green,blue);

  set_ndc_space_2(1.0,1.0);
  set_viewport_2(0.0,1.0,0.0,1.0);
  set_window(0.0,1.0,0.0,1.0);

  initialize_device(LOCATOR,1);
  initialize_device(BUTTON,1);
  initialize_device(BUTTON,2);
  initialize_device(BUTTON,3);
  set_echo_surface(LOCATOR,1,&surface);
 
}

error(string,ret)
char string[];
int ret;
{
        char ans[256];

   (void)fprintf(stderr,"%s",string);
   if (ret != 1) {
        (void)fprintf(stderr,"ERROR:  Hit return to abort!!");
        (void)gets(ans);
        exit(1);
   }
}

static
confirm()
{
   int 			 button = 4;
   int                   result;

   (void)fprintf (stderr,"Left button to confirm, Middle or Right to cancel\n");
   await_any_button(999999999,&button);
   switch (button)  {
         case 2:
         case 3:
            result = FALSE; 
            break;
         case 1:
            result = TRUE; 
            break;
         default:
            exit(1);
      }
   return result;
}
