/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * inv.dct.c - inverse discrete cosine transform
 *
 * usage: inv.dct [-w] < iseq > oseq
 *
 * inv.dct converts from the discrete cosine transform domain to the image 
 * domain.  By default, the transform is computed on the ROI and
 * only the ROI is output.  If -w is specified, the output image is the entire
 * image with only the ROI replaced by its inverse transform.
 *
 * The program does not require square input pictures, but the linear
 * dimensions must both be powers of 2.
 *
 * To load: cc -o dct dct.c  -lhipsh -lhips -lm
 *
 * Michael Landy - 3/9/93
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"w",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFFLOAT,PFDOUBLE,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp,hdw;
	int method,fr,f;
	Filename filename;
	FILE *fp;
	h_boolean wflag;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&wflag,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	if (wflag)
		write_headeru(&hdp,argc,argv);
	else {
		dup_headern(&hdp,&hdw);
		setsize(&hdw,hdw.rows,hdw.cols);
		write_headeru(&hdw,argc,argv);
	}
	fr = hdp.num_frame;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_dctinv(&hdp,&hdp);
		if (wflag)
			write_image(&hdp,f);
		else
			write_roi(&hdp,f);
	}
	return(0);
}
