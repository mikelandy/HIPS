/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/* pictranspose.c - reflect a frame about the main diagonal
 *
 * usage: pictranspose < frame > transposed_frame
 *
 * Pictranspose reflects a frame about the upper-left to lower-right diagonal.
 *
 * to load:	cc -o pictranspose pictranspose.c -lhipsh -lhips -lm
 *
 * HIPS 2 - Michael Landy - 6/23/91
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
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	dup_headern(&hdp,&hdo);
	setsize(&hdo,hdp.ocols,hdp.orows);
	hd.rows = hdo.rows = hdp.cols;
	hd.cols = hdo.cols = hdp.rows;
#ifdef ULORIG
	hd.frow = hdo.frow = hdp.fcol;
	hd.fcol = hdo.fcol = hdp.frow;
#else
	hd.frow = hdo.frow = hdp.ocols - (hdp.fcol + hdp.cols);
	hd.fcol = hdo.fcol = hdp.orows - (hdp.frow + hdp.rows);
#endif
	alloc_image(&hdo);
	write_headeru2(&hd,&hdo,argc,argv,hips_convback);
	clearroi(&hdp);
	clearroi(&hdo);
	fr = hdp.num_frame;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_transpose(&hdp,&hdo);
		write_imagec(&hd,&hdo,method,hips_convback,f);
	}
	return(0);
}
