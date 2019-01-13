/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * shiftpix.c - shift "factor" bits to the right or left each pixel of a frame
 *
 * usage: shiftpix [-s shiftfactor] < inframe > outframe
 *
 * default factor: -2
 *
 * to load: cc -o shiftpix shiftpix.c -lhipsh -lhips -lm
 *
 * Yoav Cohen 2/16/82
 * modified 2/23/82 by YC to take care of multiple frames.
 * Hips 2 - msl - 1/11/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"s",{LASTFLAG},1,{{PTINT,"-2","shiftfactor"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,PFINT,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp;
	int method,fr,f,factor;
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&factor,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	write_headeru2(&hd,&hdp,argc,argv,hips_convback);
	fr = hdp.num_frame;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_shift(&hdp,&hdp,factor);
		write_imagec(&hd,&hdp,method,hips_convback,f);
	}
	return(0);
}
