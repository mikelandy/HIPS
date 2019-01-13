/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/* reflect.c - reflect a frame about a vertical axis
 *
 * usage: reflect < frame > reflected_frame
 *
 * to load:	cc -o reflect reflect.c -lhipsh -lhips -lm
 *
 * HIPS 2 - msl - 6/21/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {LASTFLAG};
int types[] = {PFBYTE,PFINT,PFFLOAT,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	int f,fr,method;
	struct header hd,hdp,hdo;
	struct hips_roi roi;
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	dup_headern(&hdp,&hdo);
	getroi(&hdo,&roi);
	hd.fcol = hdo.fcol = hdo.ocols - (roi.fcol + roi.cols);
	alloc_image(&hdo);
	write_headeru2(&hd,&hdo,argc,argv,hips_convback);
	clearroi(&hdp);
	clearroi(&hdo);
	fr = hdp.num_frame;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_reflect(&hdp,&hdo);
		write_imagec(&hd,&hdo,method,hips_convback,f);
	}
	return(0);
}
