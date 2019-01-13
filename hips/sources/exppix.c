/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * exppix - takes exponential of the input image
 *
 * usage: logimg < iseq > oseq
 *
 * logimg computes log(pixel+offset).  offset defaults to 1.  log(x) is set
 * to -999999 if x <= 0.
 *
 * to load: cc -o exppix exppix.c -lhipsh -lhips -lm
 *
 * Input image: byte, short, int, or float format
 * output image: floating point
 *
 * Mike Landy - 4/25/89
 * Hips 2 - msl - 6/13/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {LASTFLAG};
int types[] = {PFBYTE,PFSHORT,PFINT,PFFLOAT,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp,hdo;
	int method,fr,f;
	Filename filename;
	FILE *fp;
	h_boolean imagecopy;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	imagecopy = FALSE;
	if (hdp.pixel_format != PFFLOAT) {
		if (hdp.rows != hdp.orows || hdp.cols != hdp.ocols)
			imagecopy = TRUE;
		dup_headern(&hdp,&hdo);
		setformat(&hdo,PFFLOAT);
		alloc_image(&hdo);
	}
	else	/* PFFLOAT */
		dup_header(&hdp,&hdo);
	write_headeru(&hdo,argc,argv);
	fr = hdp.num_frame;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		if (imagecopy)
			h_tof(&hdp,&hdo);
		h_exp(&hdp,&hdo);
		write_image(&hdo,f);
	}
	return(0);
}
