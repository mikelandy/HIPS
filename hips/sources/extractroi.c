/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * extractroi.c - extract the region-of-interest from a sequence
 *
 * Usage:	extractroi < frame
 *
 * Load:	cc -o extractroi extractroi.c -lhips
 *
 * Michael Landy - 1/16/91
 */

#include <hipl_format.h>
#include <stdio.h>

static Flag_Format flagfmt[] = {LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdo;
	int i,*fmts,fmtssize;
	hsize_t currsize;
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	dup_headern(&hd,&hdo);
	setsize(&hdo,hdo.rows,hdo.cols);
	write_headeru(&hdo,argc,argv);
	if (hd.pixel_format != PFMIXED) {
		for (i=0;i<hd.num_frame;i++) {
			fread_image(fp,&hd,i,filename);
			write_roi(&hd,i);
		}
	}
	else {
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
			write_roi(&hd,i);
		}
	}
	return(0);
}
