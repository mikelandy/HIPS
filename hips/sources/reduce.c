/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * reduce - reduce a sequence by pixel averaging
 *
 * usage:	reduce [-h hfactor] [-v vfactor] [-t tfactor]
 * 		reduce [-s [spacefactor]] [-t tfactor]
 * 
 * Reduce reduces the input sequence vertically by vfactor, horizontally by 
 * hfactor and temporally by tfactor.  The spatial factors default to 1, and 
 * tfactor defaults to 1.  In the second calling sequence form (which is the
 * overall default), the user specifies spacefactor, which is applied in both
 * spatial dimensions, and defaults to 2.  This filter works on byte, short,
 * integer, float, and complex images.
 *
 * to load:	cc -o reduce reduce.c -lhipsh -lhips -lm
 *
 * Rewritten by Michael Landy - 11/5/87
 * HIPS 2 - msl - 6/29/91
 */

#include <stdio.h>
#include <hipl_format.h>
int types[] = {PFBYTE,PFSHORT,PFINT,PFFLOAT,PFCOMPLEX,LASTTYPE};
static Flag_Format flagfmt[] = {
    {"h",{"s",LASTFLAG},1,{{PTINT,"1","hfactor"},LASTPARAMETER}},
    {"v",{"s",LASTFLAG},1,{{PTINT,"1","vfactor"},LASTPARAMETER}},
    {"t",{LASTFLAG},1,{{PTINT,"1","tfactor"},LASTPARAMETER}},
    {"s",{"h","v",LASTFLAG},0,{{PTBOOLEAN,"TRUE"},{PTINT,"2","spacefactor"},
	LASTPARAMETER}},
    LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	h_boolean sflag;
	int xfactor,yfactor,tfactor,sfactor;
	int f,fr,i,method;
	struct header hd,hdp1,hdp2,hdo,hdcb;
	struct hips_roi roi;
	Filename filename;
	FILE *fp;
	Pixelval zero,scale;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&xfactor,&yfactor,&tfactor,&sflag,&sfactor,
		FFONE,&filename);
	fp = hfopenr(filename);
	if (sflag)
		xfactor = yfactor = sfactor;
	fread_hdr_a(fp,&hd,filename);
	getroi(&hd,&roi);
	roi.frow /= yfactor;
	roi.fcol /= xfactor;
	roi.rows /= yfactor;
	roi.cols /= xfactor;
	clearroi(&hd);
	method = fset_conversion(&hd,&hdp1,types,filename);
	dup_headern(&hdp1,&hdp2);
	setsize(&hdp2,hdp1.orows/yfactor,hdp1.ocols/xfactor);
	if (tfactor > 1 && hdp1.numcolor != 1)
		perr(HE_MSG,"can't reduce color images in time");
	hdp2.num_frame /= tfactor;
	if (hdp1.pixel_format == PFBYTE) {
		setformat(&hdp2,PFINT);
		alloc_image(&hdp2);
		dup_headern(&hdp2,&hdo);
		setformat(&hdo,PFBYTE);
		alloc_image(&hdo);
	}
	else {
		alloc_image(&hdp2);
		dup_header(&hdp2,&hdo);
	}
	setroi2(&hdo,&roi);
	if (hips_convback)
		setupconvback(&hd,&hdo,&hdcb);
	write_headeru2(&hdcb,&hdo,argc,argv,hips_convback);
	clearroi(&hdo);
	hdp1.rows = hdp2.orows * yfactor;
	hdp1.cols = hdp2.ocols * xfactor;
	switch(hdp2.pixel_format) {
	case PFSHORT:	zero.v_short = 0;
			scale.v_short = xfactor*yfactor*tfactor;
			break;
	case PFINT:	zero.v_int = 0;
			scale.v_int = xfactor*yfactor*tfactor;
			break;
	case PFFLOAT:
	case PFCOMPLEX:	zero.v_float = 0;
			scale.v_float = xfactor*yfactor*tfactor;
			break;
	}
	fr = hdp2.num_frame;
	for (f=0;f<fr;f++) {
		h_setimage(&hdp2,&zero);
		for (i=0;i<tfactor;i++) {
			fread_imagec(fp,&hd,&hdp1,method,f,filename);
			h_reduce(&hdp1,&hdp2,xfactor,yfactor);
		}
		h_divscale(&hdp2,&hdo,&scale);
		write_imagec(&hdcb,&hdo,method,hips_convback,f);
	}
	return(0);
}
