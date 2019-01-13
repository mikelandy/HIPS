/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/* picinvert.c - reflect a frame about a horizontal axis
 *
 * usage: picinvert < frame > inverted_frame
 *
 * to load:	cc -o picinvert picinvert.c -lhipsh -lhips -lm
 *
 * HIPS 2 - msl - 6/23/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {LASTFLAG};
int types[] = {PFMSBF,PFLSBF,PFBYTE,PFINT,PFFLOAT,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	int f,fr,method;
	struct header hd,hdp;
	struct hips_roi roi;
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	getroi(&hdp,&roi);
	hd.frow = hdp.frow = hdp.orows - (roi.frow + roi.rows);
	write_headeru2(&hd,&hdp,argc,argv,hips_convback);
	clearroi(&hdp);
	fr = hdp.num_frame;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_invert(&hdp,&hdp);
		write_imagec(&hd,&hdp,method,hips_convback,f);
	}
	return(0);
}
