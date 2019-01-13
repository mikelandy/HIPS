/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * bnoise.c - add binomial noise to an image
 *
 * usage: bnoise [-s seed] [-n N] [-p p] [-a addconst] [-m mulconst]
 *
 * Bnoise adds jointly independent binomially distributed noise to each pixel
 * in an image sequence.  The algorithm executes in Pix*(log N) time,
 * for Pix the number of pixels in the sequence, and N the number of
 * Bernoullians added to obtain each binomial.
 *
 * Bnoise adds to each pixel of inseq a random variable mulconst*B + addconst,
 * where B is binomially distributed, and in particular, equal to the sum of N
 * independent Bernoullian random variables, each taking the value 1 with
 * probability p and 0 otherwise.  Thus the random variable mulconst*B +
 * addconst has expectation mulconst*Np + addconst -- so set addconst =
 * -mulconst*N*p to make the expectation 0 -- and
 * variance mulconst*mulconst*Np(1-p).
 *
 * For byte and integer images, addconst and mulconst are clipped to be
 * integers.
 *
 * Defaults: seed=1, N=1, p=.5, addconst=-mulconst*N*p, mulconst=2.
 *
 * pixel formats handled directly: BYTE, INT, FLOAT
 *
 * to load:	cc -o bnoise bnoise.c -lhipsh -lhips -lm
 *
 * Charlie Chubb - 10/15/87
 * Hips 2 - msl - 8/6/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"p",{LASTFLAG},1,{{PTDOUBLE,".5","prob"},LASTPARAMETER}},
	{"n",{LASTFLAG},1,{{PTINT,"1","N"},LASTPARAMETER}},
	{"a",{LASTFLAG},1,{{PTDOUBLE,"999999.","addconst"},LASTPARAMETER}},
	{"m",{LASTFLAG},1,{{PTDOUBLE,"2.","mulconst"},LASTPARAMETER}},
	{"s",{LASTFLAG},1,{{PTINT,"1","seed"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,PFINT,PFFLOAT,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp;
	int method,fr,f,seed,tlclip,thclip,N;
	Filename filename;
	FILE *fp;
	double p,addc,mulc;
	Pixelval aval,mval;
	char msg[100];

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&p,&N,&addc,&mulc,&seed,FFONE,&filename);
	if (p > 1. || p < 0.)
		perr(HE_MSG,"p must be in the interval [0,1]");
	if (addc == 999999.)
		addc = -mulc*N*p;
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	write_headeru2(&hd,&hdp,argc,argv,hips_convback);
	switch (hdp.pixel_format) {
	case PFBYTE:
	case PFINT:	aval.v_int = addc; mval.v_int = mulc; break;
	case PFFLOAT:	aval.v_float = addc; mval.v_float = mulc; break;
	}
	fprintf(stderr,"%s: N = %d, p = %f, q = %f,",Progname,N,p,1-p);
	if (hdp.pixel_format == PFFLOAT)
		fprintf(stderr,"mulconst = %f, addconst = %f, seed = %d\n",
					mulc,addc,seed);
	else
		fprintf(stderr,"mulconst = %d, addconst = %d, seed = %d\n",
					(int) mulc,(int) addc,seed);
	fr = hdp.num_frame;
	H__SRANDOM(seed);
	tlclip = thclip = 0;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		hips_lclip = hips_hclip = 0;
		h_bnoise(&hdp,&hdp,N,p,&aval,&mval);
		thclip += hips_hclip;
		tlclip += hips_lclip;
		write_imagec(&hd,&hdp,method,hips_convback,f);
	}
	if (thclip || tlclip) {
		sprintf(msg,"total of %d underflows and %d overflows detected",
			tlclip,thclip);
		perr(HE_IMSG,msg);
	}
	return(0);
}
