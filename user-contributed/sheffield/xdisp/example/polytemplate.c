/* 

This is an example C HIPS program which reads an image in byte format with
polygons of interest (*.poi) in the header and sets the points in each 
polygon to 255. X clip boxes are used round each polygon to reduce the amount
of processing required for each polygon.

NOTE: there is a clash between the X definition of Bool and
the HIPS definition, so an edited hips_basic.h should be used.

To build this program:

cc -O -I$OPENWINHOME/include -I$HINCS -c polytemplate.c
cc -o polytemplate polytemplate.o -L$OPENWINHOME/lib -lX11 -lhips -lm

where HINCS is the name of the location of HIPS header include files.

*/

#include <stdio.h>
#include <stdlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <hipl_format.h>

typedef struct polygon_t {
	int			no_of_sides;
	XPoint			*sides;
	XRectangle		clip_box;
	Region			region;
	struct polygon_t 	*next;
		} polygon_t;

static Flag_Format flagfmt[] = {LASTFLAG};
int types[] = {PFMSBF,PFLSBF,PFBYTE,LASTTYPE};

struct extpar *grepparam();

main(argc,argv)

int argc;
char **argv;

{
  struct header hd,hdp;
  int method,fr,f;
  Filename filename;
  FILE *fp;
  int *pars;
  int ct;
  struct extpar *xp;
  int i,j;
  polygon_t *poly;
  polygon_t *polygon_list = NULL;
  unsigned char *image;
  int nc;
  int r,c,w,h;

    Progname = strsave(*argv);
    parseargs(argc,argv,flagfmt,FFONE,&filename);
    fp = hfopenr(filename);
    fread_hdr_a(fp,&hd,filename);
    method = fset_conversion(&hd,&hdp,types,filename);
    write_headeru2(&hd,&hdp,argc,argv,hips_convback);
    fr = hd.num_frame;

    /* read in polygons from header */

    while((xp = grepparam(&hd,".poi")) != (struct extpar *)NULLPAR) {
	getparam(&hd,xp->name,PFINT,&ct,&pars);
	poly = (polygon_t *)malloc(sizeof(polygon_t));
	poly->next = polygon_list;
	polygon_list = poly;
	poly->no_of_sides = ct/2;
	poly->sides = (XPoint *)malloc(poly->no_of_sides*sizeof(XPoint));
	for (i = 0; i < ct; i+= 2) {
	    poly->sides[i/2].y = pars[i];
	    poly->sides[i/2].x = pars[i+1];
	    }
	poly->region = 
		XPolygonRegion(poly->sides,poly->no_of_sides,EvenOddRule);
	XClipBox(poly->region,&poly->clip_box);
	clearparam(&hd,xp->name);
	}

    /* process each frame */

    for (f=0;f<fr;f++) {
	fread_imagec(fp,&hd,&hdp,method,f,filename);
	image = (unsigned char *)hdp.firstpix;
	nc = hdp.ocols;
	poly = polygon_list;
    	while (poly != NULL) {
	    r = poly->clip_box.y;
	    c = poly->clip_box.x;
	    w = poly->clip_box.width;
	    h = poly->clip_box.height;
	    for (i = r; i < r+h; i++)
	        for (j = c; j < c+w; j++)
		    if (XPointInRegion(poly->region,j,i)) 
			image[i*nc+j] = 255;
	    poly = poly->next;
	    }
	write_imagec(&hd,&hdp,method,hips_convback,f);
        }

    return(0);
}


struct extpar *grepparam(hd,name)
  struct header *hd;
  char *name;
{
    struct extpar *xp;
    xp = hd->params;
    while (xp != NULLPAR) {
	if (strstr(xp->name,name) != NULL)
	    return(xp);
	xp = xp->nextp;
	}
    return(NULLPAR);
}

