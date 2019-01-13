/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * neg - takes photographic negative of
 * input image by inverting bits. Input image
 * is either byte, short, integer, single-bit/bit packed, or 
 * floating.
 *
 * usage:	neg <iseq >oseq
 *
 * pixel formats handled directly: MSBF, LSBF, BYTE, SHORT, INT, FLOAT
 *
 * to load:	cc -o neg neg.c -lhipsh -lhips
 *
 * Mike Landy - 5/27/82
 * Hips 2 - msl - 1/6/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {LASTFLAG};
int types[] = {PFMSBF,PFLSBF,PFBYTE,PFSHORT,PFINT,PFFLOAT,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp;
	int method,fr,f;
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	write_headeru2(&hd,&hdp,argc,argv,hips_convback);
	fr = hdp.num_frame;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_neg(&hdp,&hdp);
		write_imagec(&hd,&hdp,method,hips_convback,f);
	}
	return(0);
}
