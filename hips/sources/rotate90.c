/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/* rotate90.c - rotate a frame by 90 degrees
 *
 * usage: rotate90 [-l] < frame > rotated_frame
 *
 * The -l switch rotates to the left (counterclockwise), otherwise the
 * frames are rotated to the right (clockwise).
 *
 * to load:	cc -o rotate90 rotate90.c -lhipsh -lhips -lm
 *
 * HIPS 2 - msl - 1/11/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"l",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,PFINT,PFFLOAT,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	int f,fr,method;
	h_boolean lflag;
	struct header hd,hdp,hdo;
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&lflag,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	dup_headern(&hdp,&hdo);
	setsize(&hdo,hdp.ocols,hdp.orows);
#ifdef ULORIG
	if (!lflag) {
#else
	if (lflag) {
#endif
		hd.frow = hdo.frow = hdp.fcol;
		hd.fcol = hdo.fcol = hdp.orows - (hdp.frow + hdp.rows);
		hd.rows = hdo.rows = hdp.cols;
		hd.cols = hdo.cols = hdp.rows;
	}
	else {
		hd.frow = hdo.frow = hdp.ocols - (hdp.fcol + hdp.cols);
		hd.fcol = hdo.fcol = hdp.frow;
		hd.rows = hdo.rows = hdp.cols;
		hd.cols = hdo.cols = hdp.rows;
	}
	alloc_image(&hdo);
	write_headeru2(&hd,&hdo,argc,argv,hips_convback);
	clearroi(&hdp);
	clearroi(&hdo);
	fr = hdp.num_frame;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_rot90(&hdp,&hdo,lflag);
		write_imagec(&hd,&hdo,method,hips_convback,f);
	}
	return(0);
}
