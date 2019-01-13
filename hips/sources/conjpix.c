/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * conjpix - takes the complex conjugate of pixel values
 *
 * formats handled directly: complex, double complex
 *
 * usage:	conjpix <iseq >oseq
 *
 * to load:	cc -o conjpix conjpix.c -lhipsh -lhips -lm
 *
 * Mike Landy - 3/14/95
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {LASTFLAG};
int types[] = {PFCOMPLEX,PFDBLCOM,LASTTYPE};

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
	write_headeru(&hdp,argc,argv);
	fr = hdp.num_frame;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_conj(&hdp,&hdp);
		write_image(&hdp,f);
	}
	return(0);
}
