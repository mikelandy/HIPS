/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * dotdiff.c - halftones using an 8x8 dot diffusion matrix
 *
 * usage:	dotdiff <iseq >oseq
 *
 * Dotdiff converts an 8-bit sequence to a bi-level sequence using an 8 x 8
 * dot error diffusion matrix.  The input and output sequences are both
 * byte-formatted, although only values hips_lchar and hips_hchar are used
 * in the output image.
 *
 * pixel formats handled directly: BYTE
 *
 * to load:	cc -o dotdiff dotdiff.c -lhipsh -lhips
 *
 * Mike Landy - 7/13/89 (algorithm from Hong Min)
 * Hips 2 - msl - 8/10/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {LASTFLAG};
int types[] = {PFBYTE,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp,hdt;
	int method,fr,f;
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	write_headeru2(&hd,&hdp,argc,argv,hips_convback);
	dup_headern(&hdp,&hdt);
	setformat(&hdt,PFSHORT);
	setsize(&hdt,hdt.rows,hdt.cols);
	alloc_image(&hdt);
	fr = hdp.num_frame;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_tos(&hdp,&hdt);
		h_dotdiff(&hdp,&hdt,&hdp);
		write_imagec(&hd,&hdp,method,hips_convback,f);
	}
	return(0);
}
