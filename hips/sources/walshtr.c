/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * walshtr.c - perform the Walsh transform
 *
 * usage: walshtr [-o] < iseq > oseq
 *
 * This program computes a fast Walsh transform.  If -o is specified, the
 * coefficients are output in sequency order.  The coefficients are
 * normalized.
 *
 * The program does not require square input pictures, but the linear
 * dimensions must both be powers of 2.
 *
 * input pixel formats: INT, FLOAT
 * output pixel format: FLOAT
 *
 * To load: cc -o walshtr walshtr.c  -lhipsh -lhips -lm
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
	struct header hd,hdp,hdt,hdo;
	int method,fr,f;
	Filename filename;
	FILE *fp;
	h_boolean oflag;
	Pixelval p;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&oflag,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	clearroi(&hd);
	method = fset_conversion(&hd,&hdp,types,filename);
	dup_header(&hdp,&hdt);
	if (hdp.pixel_format == PFINT) {
		setformat(&hdt,PFFLOAT);
		alloc_image(&hdt);
	}
	write_headeru(&hdt,argc,argv);
	if (oflag) {
		dup_headern(&hdt,&hdo);
		alloc_image(&hdo);
	}
	p.v_float = hd.orows * hd.ocols;
	fr = hdp.num_frame;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_walshtr(&hdp);
		h_divscale(&hdp,&hdt,&p);
		if (oflag) {
			h_seqord(&hdt,&hdo);
			write_image(&hdo,f);
		}
		else
			write_image(&hdt,f);
	}
	return(0);
}
