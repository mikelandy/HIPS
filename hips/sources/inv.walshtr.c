/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * inv.walshtr.c - perform the inverse Walsh transform
 *
 * usage: inv.walshtr [-o] < iseq > oseq
 *
 * This program computes a fast Walsh transform.  If -o is specified, the
 * input coefficients are assumed to be in sequency order.
 *
 * The program does not require square input pictures, but the linear
 * dimensions must both be powers of 2.
 *
 * input pixel formats: INT, FLOAT
 *
 * To load: cc -o inv.walshtr inv.walshtr.c  -lhipsh -lhips -lm
 *
 * Yoav Cohen 2/18/82
 * HIPS 2 - msl - 8/11/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"o",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFINT,PFFLOAT,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp,hdo;
	int method,fr,f;
	Filename filename;
	FILE *fp;
	h_boolean oflag;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&oflag,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	clearroi(&hd);
	method = fset_conversion(&hd,&hdp,types,filename);
	write_headeru(&hdp,argc,argv);
	if (oflag) {
		dup_headern(&hdp,&hdo);
		alloc_image(&hdo);
	}
	fr = hdp.num_frame;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_walshtr(&hdp);
		if (oflag) {
			h_invseqord(&hdp,&hdo);
			write_image(&hdo,f);
		}
		else
			write_image(&hdp,f);
	}
	return(0);
}
