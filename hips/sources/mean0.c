/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * mean0 - subtract the mean from an image so that it has a mean of zero
 *
 * usage:	mean0 <iseq >oseq
 *
 * pixel formats handled directly: FLOAT
 *
 * to load:	cc -o mean0 mean0.c -lhipsh -lhips
 *
 * Hips 2 - msl - 8/7/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {LASTFLAG};
int types[] = {PFFLOAT,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp;
	int method,fr,f;
	Filename filename;
	FILE *fp;
	float mean;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	write_headeru2(&hd,&hdp,argc,argv,hips_convback);
	fr = hdp.num_frame;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_mean(&hdp,&mean,FALSE);
		h_linscale(&hdp,&hdp,1.,-mean);
		write_imagec(&hd,&hdp,method,hips_convback,f);
	}
	return(0);
}
