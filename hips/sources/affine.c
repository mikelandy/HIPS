/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * affine - Warp an image using an affine transformation
 *
 * usage:	affine [-ll x y] [-lr x y] [-ul x y] [-ur x y] [-s nr nc] [-C]
 * 
 * Affine warps an image using an affine transformation.  The image is 
 * considered to have a coordinate system with the origin at the lower-left
 * corner, x horizontal, and y vertical.  The rightmost column is treated as
 * having x=1.0, and the top row as having y=1.0.  The user specifies to which
 * coordinate points up to three of the four image corners map in the output
 * image:  the lower left (-ll), the lower-right (-lr), the upper-left (-ul),
 * and the upper-right (-ur).  Specification of three of the four mappings
 * uniquely defines an affine map (involving translation, rotation, 
 * scaling, and shear).  The user-specified arguments need not be confined
 * to the interval [0,1], and clipping will be performed appropriately.  If
 * -C is specified, the user-specified arguments are in column and row numbers
 * of the output image, rather than as fractions of each. The default mappings
 * depend on the number of arguments specified:
 *	0	- identity mapping
 *	1	- translation only
 *	2	- translation, scaling, and rotation
 *	3	- full affine transformation
 *	4	- error (try "polywarp" instead)
 *
 * For each output pixel, the corresponding input coordinates are computed.
 * If that input point is outside of the image, then the output pixel is
 * set to the background value (set by the standard switch -UL, the default is
 * zero).  Otherwise, the value is bilinearly interpolated between the
 * surrounding four pixels in the input.  The output image size may be
 * specified using -s, and defaults to the input image size.
 *
 * to load:	cc -o affine affine.c -lhipsh -lhips -lm
 *
 * Mike Landy - 8/7/88
 * HIPS 2 - msl - 6/29/91
 */

#include <stdio.h>
#include <hipl_format.h>
int types[] = {PFBYTE,LASTTYPE};
static Flag_Format flagfmt[] = {
    {"ll",{LASTFLAG},2,{{PTBOOLEAN,"FALSE"},{PTDOUBLE,"0","x"},
	{PTDOUBLE,"0","y"},LASTPARAMETER}},
    {"lr",{LASTFLAG},2,{{PTBOOLEAN,"FALSE"},{PTDOUBLE,"0","x"},
	{PTDOUBLE,"0","y"},LASTPARAMETER}},
    {"ul",{LASTFLAG},2,{{PTBOOLEAN,"FALSE"},{PTDOUBLE,"0","x"},
	{PTDOUBLE,"0","y"},LASTPARAMETER}},
    {"ur",{LASTFLAG},2,{{PTBOOLEAN,"FALSE"},{PTDOUBLE,"0","x"},
	{PTDOUBLE,"0","y"},LASTPARAMETER}},
    {"s",{LASTFLAG},2,{{PTBOOLEAN,"FALSE"},{PTINT,"0","rows"},
	{PTINT,"0","columns"},LASTPARAMETER}},
    {"C",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
    LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	h_boolean llflag,lrflag,ulflag,urflag,sflag,Cflag;
	double llx,lly,lrx,lry,ulx,uly,urx,ury;
	float A,B,C,a,b,c,determinant,dx,dy,x,y;
	int nor,noc;
	int f,fr,method;
	struct header hd,hdp,hdo,hdcb;
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&llflag,&llx,&lly,&lrflag,&lrx,&lry,
		&ulflag,&ulx,&uly,&urflag,&urx,&ury,&sflag,&nor,&noc,&Cflag,
		FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	clearroi(&hd);
	method = fset_conversion(&hd,&hdp,types,filename);
	if (!sflag) {
		nor = hd.orows;
		noc = hd.ocols;
	}
	if (Cflag) {
		llx /= (noc-1);
		lly /= (nor-1);
		lrx /= (noc-1);
		lry /= (nor-1);
		ulx /= (noc-1);
		uly /= (nor-1);
		urx /= (noc-1);
		ury /= (nor-1);
	}
	if (!urflag) {
	    if (!ulflag) {
		if (!lrflag) {
		    if (!llflag) {
			llx = 0;
			lly = 0;
			lrx = 1;
			lry = 0;
			ulx = 0;
			uly = 1;
		    }
		    else {
			lrx = llx+1;
			lry = lly;
			ulx = llx;
			uly = lly+1;
		    }
		}
		else {
		    if (!llflag) {
			llx = lrx-1;
			lly = lry;
			ulx = lrx-1;
			uly = lry+1;
		    }
		    else {
			dx = lrx-llx;
			dy = lry-lly;
			ulx = llx-dy;
			uly = lly+dx;
		    }
		}
	    }
	    else {
		if (!lrflag) {
		    if (!llflag) {
			llx = ulx;
			lly = uly-1;
			lrx = ulx+1;
			lry = uly-1;
		    }
		    else {
			dx = ulx-llx;
			dy = uly-lly;
			lrx = llx+dy;
			lry = lly-dx;
		    }
		}
		else {
		    if (!llflag) {
			x = ulx-lrx;
			y = uly-lry;
			dx = (x+y)/2.;
			dy = (y-x)/2.;
			llx = lrx-dy;
			lly = lry+dx;
		    }
		    else {
			/* nothing to do */
		    }
		}
	    }
	}
	else {
	    if (!ulflag) {
		if (!lrflag) {
		    if (!llflag) {
			llx = urx-1;
			lly = ury-1;
			lrx = urx;
			lry = ury-1;
			ulx = urx-1;
			uly = ury;
		    }
		    else {
			x = urx-llx;
			y = ury-lly;
			dx = (x+y)/2.;
			dy = (y-x)/2.;
			lrx = llx+dx;
			lry = lly+dy;
			ulx = llx-dy;
			uly = lly+dx;
		    }
		}
		else {
		    if (!llflag) {
			dx = urx-lrx;
			dy = ury-lry;
			llx = lrx-dy;
			lly = lry+dx;
			ulx = urx-dy;
			uly = ury+dx;
		    }
		    else {
			ulx = llx+(urx-lrx);
			uly = lly+(ury-lry);
		    }
		}
	    }
	    else {
		if (!lrflag) {
		    if (!llflag) {
			dx = urx-ulx;
			dy = ury-uly;
			llx = ulx+dy;
			lly = uly-dx;
			lrx = urx+dy;
			lry = ury-dx;
		    }
		    else {
			lrx = llx+(urx-ulx);
			lry = lly+(ury-uly);
		    }
		}
		else {
		    if (!llflag) {
			llx = lrx-(urx-ulx);
			lly = lry-(ury-uly);
		    }
		    else
			perr(HE_MSG,
			    "can't specify all four corners, try polywarp");
		}
	    }
	}
	determinant = llx*lry + lly*ulx + lrx*uly
		- (llx*uly + lly*lrx + lry*ulx);
	if (determinant == 0)
		perr(HE_MSG,"specified affine mapping is not invertible");
	A = (uly - lly)/determinant;
	B = (llx - ulx)/determinant;
	C = (lly*ulx - llx*uly)/determinant;
	a = (lly - lry)/determinant;
	b = (lrx - llx)/determinant;
	c = (llx*lry - lly*lrx)/determinant;
	dup_headern(&hdp,&hdo);
	setsize(&hdo,nor,noc);
	alloc_image(&hdo);
	if (hips_convback)
		setupconvback(&hd,&hdo,&hdcb);
	write_headeru2(&hdcb,&hdo,argc,argv,hips_convback);
	fr = hdp.num_frame;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_affine(&hdp,&hdo,A,B,C,a,b,c);
		write_imagec(&hdcb,&hdo,method,hips_convback,f);
	}
	return(0);
}
