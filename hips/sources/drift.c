/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * drift - translate a single frame with linear, integral velocity
 *
 * usage:	drift [-f frames] [-s x-shift [y-shift]] [-w] < frame > sequence
 *
 * Drift takes a single frame and drifts the pixels with a linear velocity.
 * The user can specify the number of output frames and the amount of shift
 * per frame.  Positive values of x-shift and y-shift move the image rightward
 * and upward, respectively.  If the number of frames is greater than one,
 * then the first output frame is identical to the input frame.  Otherwise
 * a single shift is applied.  The shifts default to 1 and 0, and the number
 * of output frames defaults to 1.  By default, the input region drifts, and
 * uncovered pixels are set to hips_lchar.  If -w (wrap-around) is specified,
 * pixels which drift off one edge of the region reappear at the opposite
 * edge.
 *
 * Pixel formats handled directly:  BYTE, SHORT, INT, FLOAT, DOUBLE, COMPLEX,
 *					DBLCOM
 *
 * to load:	cc -o drift drift.c -lhipsh -lhips
 *
 * Mike Landy - 6/6/86
 * HIPS 2 - msl - 7/9/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"f",{LASTFLAG},1,{{PTINT,"1","frames"},LASTPARAMETER}},
	{"s",{LASTFLAG},1,{{PTINT,"1","x-shift"},{PTINT,"0","y-shift"},
		LASTPARAMETER}},
	{"w",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,PFSHORT,PFINT,PFFLOAT,PFDOUBLE,PFCOMPLEX,PFDBLCOM,
		LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp1,hdp2;
	int method,f,ofr,xshift,yshift,currx,curry;
	Filename filename;
	FILE *fp;
	h_boolean wflag;
	struct hips_roi roi;
	h_boolean imagecopy = FALSE;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&ofr,&xshift,&yshift,&wflag,
		FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	if (hd.rows != hd.orows || hd.cols != hd.ocols)
		imagecopy = TRUE;
	if (ofr < 1)
		perr(HE_MSG,"output frames must be at least 1");
	if (hd.numcolor != 1)
		perr(HE_MSG,"Number of color planes must be 1, use subseq");
	method = fset_conversion(&hd,&hdp1,types,filename);
	if (hdp1.num_frame != 1)
		perr(HE_MSG,"Number of input frames must be 1, use subseq");
	hd.num_frame = hdp1.num_frame = ofr;
	write_headeru2(&hd,&hdp1,argc,argv,hips_convback);
	getroi(&hdp1,&roi);
	dup_headern(&hdp1,&hdp2);
	alloc_image(&hdp2);
	fread_imagec(fp,&hd,&hdp1,method,0,filename);
	if (imagecopy) {
		clearroi(&hdp1);
		clearroi(&hdp2);
		h_copy(&hdp1,&hdp2);
		setroi2(&hdp1,&roi);
		setroi2(&hdp2,&roi);
	}
	if (ofr == 1) {
		h_translate(&hdp1,&hdp2,xshift,yshift,wflag);
		write_imagec(&hd,&hdp2,method,hips_convback,0);
		return(0);
	}
	currx = curry = 0;
	write_imagec(&hd,&hdp1,method,hips_convback,0);
	for (f=1;f<ofr;f++) {
		currx += xshift;
		curry += yshift;
		h_translate(&hdp1,&hdp2,currx,curry,wflag);
		write_imagec(&hd,&hdp2,method,hips_convback,f);
	}
	return(0);
}
