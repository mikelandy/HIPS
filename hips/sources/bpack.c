/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * bpack - convert to packed pixel format
 *
 * usage:	bpack [-P [M or L]] <iseq >oseq
 *
 * Convert from any pixel format to packed format.  If -P is not specified,
 * the system-wide default packing order is used (set when the software was
 * installed).  M and L specify most-significant-bit first and
 * least-significant-bit first order, respectively.
 *
 * to load:	cc -o bpack bpack.c -lhips -lm
 *
 * Mike Landy - 1/11/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {LASTFLAG};
int types[] = {PFMSBF,LASTTYPE};

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
	if (hips_packing == MSBF_PACKING)
		types[0] = PFMSBF;
	else
		types[0] = PFLSBF;
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	write_headeru(&hdp,argc,argv);
	fr = hdp.num_frame;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		write_image(&hdp,f);
	}
	return(0);
}
