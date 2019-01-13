/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * stretch - stretch an image
 *
 * usage:	stretch [-f xfactor yfactor | -s rows columns] < in > out
 * 
 * Stretch changes the row and column dimensions of an image by stretching.
 * The algorithm is basically block averaging, where each pixel in the old
 * image is treated as square, and each pixel in the new image rectangular.
 * The new pixel's value is an average of the pixel's in the old image it
 * overlaps, weighted by the degree of overlap.  The user can either specify
 * the stretch factor for each dimension (the number of rows and columns will
 * be truncated to an integer value), using the -f switch, or can specify the
 * output image size, with the -s switch.
 *
 * to load:	cc -o stretch stretch.c -lhipsh -lhips -lm
 *
 * Mike Landy - 6/11/87, based on code by Lou Salkind
 * HIPS 2 - msl - 6/29/91
 */

#include <stdio.h>
#include <hipl_format.h>
int types[] = {PFBYTE,LASTTYPE};
static Flag_Format flagfmt[] = {
    {"f",{"s",LASTFLAG},2,{{PTBOOLEAN,"FALSE"},{PTDOUBLE,"0","xfactor"},
	{PTDOUBLE,"0","yfactor"},LASTPARAMETER}},
    {"s",{"f",LASTFLAG},2,{{PTBOOLEAN,"FALSE"},{PTINT,"0","rows"},
	{PTINT,"0","columns"},LASTPARAMETER}},
    LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	h_boolean fflag,sflag;
	double xfactor,yfactor;
	int nor,noc;
	int f,fr,method;
	struct header hd,hdp,hdo,hdcb;
	struct hips_roi roi;
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&fflag,&xfactor,&yfactor,&sflag,&nor,&noc,
		FFONE,&filename);
	if (!fflag && !sflag)
		perr(HE_MSG,"one of -s and -f must be specified");
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	getroi(&hd,&roi);
	clearroi(&hd);
	method = fset_conversion(&hd,&hdp,types,filename);
	if (fflag) {
		nor = hd.orows * yfactor;
		noc = hd.ocols * xfactor;
	}
	else {
		yfactor = ((float) nor)/hd.orows;
		xfactor = ((float) noc)/hd.ocols;
	}
	roi.frow *= yfactor;
	roi.fcol *= xfactor;
	roi.rows *= yfactor;
	roi.cols *= xfactor;
	dup_headern(&hdp,&hdo);
	setsize(&hdo,nor,noc);
	setroi2(&hdo,&roi);
	alloc_image(&hdo);
	if (hips_convback)
		setupconvback(&hd,&hdo,&hdcb);
	write_headeru2(&hdcb,&hdo,argc,argv,hips_convback);
	clearroi(&hdo);
	fr = hdp.num_frame;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_stretchimg(&hdp,&hdo);
		write_imagec(&hdcb,&hdo,method,hips_convback,f);
	}
	return(0);
}
