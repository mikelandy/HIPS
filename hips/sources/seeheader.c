/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * seeheader.c - print the header of a frame sequence
 *
 * Usage:	seeheader [-p] [-a] < frame
 *
 * Load:	cc -o seeheader seeheader.c -lhips
 *
 * Michael Landy - 2/4/82
 * HIPS 2 - msl - 1/6/91
 * PFMIXED - msl - 11/12/92
 *
 * The -p option allows seeheader to be used in a pipe, sending the
 * original sequence to the standard output, and the text output to stderr.
 * The -a flag causes entire extended parameter arrays to be printed.  By
 * default at most 5 values are printed.
 */

#include <hipl_format.h>
#include <stdio.h>

static Flag_Format flagfmt[] = {
	{"p",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"a",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd;
	int c,i,*fmts,fmtssize;
	h_boolean pflag,aflag;
	Filename filename;
	FILE *fp;
	hsize_t currsize;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&pflag,&aflag,FFONE,&filename);
	fp = hfopenr(filename);
	if (pflag)
		fread_hdr_a(fp,&hd,filename);
	else
		fread_header(fp,&hd,filename);
	fprintf(stderr,"%s",formatheaderc(&hd,aflag));
	if (!pflag)
		return(0);
	write_header(&hd);
	if (hd.sizeimage) {
		for (i=0;i<hd.num_frame;i++) {
			fread_image(fp,&hd,i,filename);
			write_image(&hd,i);
		}
	}
	else if (hd.pixel_format == PFMIXED) {
		fmtssize = hd.num_frame;
		getparam(&hd,"formats",PFINT,&fmtssize,&fmts);
		if (fmtssize != hd.num_frame)
			perr(HE_FMTSLEN,filename);
		setformat(&hd,fmts[0]);
		alloc_image(&hd);
		currsize = hd.sizeimage;
		for (i=0;i<hd.num_frame;i++) {
			setformat(&hd,fmts[i]);
			if (hd.sizeimage > currsize) {
				free(hd.image);
				alloc_image(&hd);
				currsize = hd.sizeimage;
			}
			fread_image(fp,&hd,i,filename);
			write_image(&hd,i);
		}
	}
	else {
		while ((c=getc(fp)) != EOF) putchar(c);
	}
	return(0);
}
