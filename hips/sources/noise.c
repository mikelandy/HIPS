/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * noise.c - simulate a noisy digital communication channel
 *		 
 * usage: noise [-p p-error [random-seed [bits-per-pixel]]] < inframe > outframe
 *
 * defaults: p-error: .001
 *	     random-seed: 1
 *	     bits-per-pixel: 8
 *
 * This program randomly flips bits with probability p.  Only the low-order
 * bits-per-pixel bits are subject to flipping.
 *
 * to load:	cc -o noise noise.c -lhipsh -lhips -lm
 *
 * Yoav Cohen 3/15/82
 * sped-up: Mike Landy 9/18/88
 * HIPS 2 - msl - 8/5/91
 */

#include <stdio.h>
#include <hipl_format.h>

int types[] = {PFBYTE,LASTTYPE};
static Flag_Format flagfmt[] = {
	{"p",{LASTFLAG},1,{{PTDOUBLE,".001","p-error"},{PTINT,"1","seed"},
		{PTINT,"8","bits-per-pixel"},LASTPARAMETER}},
	LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	int f,fr,method,counter,seed,bpp;
	double p;
	struct header hd,hdp;
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&p,&seed,&bpp,FFONE,&filename);
	if (bpp < 1 || bpp > 8)
		perr(HE_MSG,"bits-per-pixel must be between 1 and 8");
	if (p>=1.0 || p<=0.0)
		perr(HE_MSG,"p(error) must be 0>p>1");
	H__SRANDOM(seed);
	counter = -1;
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	write_headeru2(&hd,&hdp,argc,argv,hips_convback);
	fr = hdp.num_frame;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_noise(&hdp,&hdp,p,&counter,bpp);
		write_imagec(&hd,&hdp,method,hips_convback,f);
	}
	return(0);
}
