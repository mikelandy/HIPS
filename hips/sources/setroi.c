/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * setroi.c - set the region-of-interest
 *
 * Usage:	setroi -s frow fcol nrow ncol < frame > roiframe
 * 		setroi -l frow fcol lrow lcol < frame > roiframe
 *
 * In the first form, the user specifies the first row and column and the
 * number of rows and columns in the region-of-interest.  In the second form
 * the user specifies the last row and column in place of the size of the 
 * region-of-interest.
 *
 * Load:	cc -o setroi setroi.c -lhips
 *
 * Michael Landy - 1/8/91
 */

#include <hipl_format.h>
#include <stdio.h>

static Flag_Format flagfmt[] = {
    {"s",{"l",LASTFLAG},4,{{PTBOOLEAN,"FALSE"},{PTINT,"-1","frow"},
	{PTINT,"-1","fcol"},{PTINT,"-1","nrow"},{PTINT,"-1","ncol"},
	LASTPARAMETER}},
    {"l",{"s",LASTFLAG},4,{{PTBOOLEAN,"FALSE"},{PTINT,"-1","frow"},
	{PTINT,"-1","fcol"},{PTINT,"-1","lrow"},{PTINT,"-1","lcol"},
	LASTPARAMETER}},
    LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd;
	int c,i,frow,fcol,lrow,lcol,nrow,ncol,*fmts,fmtssize;
	hsize_t currsize;
	h_boolean sflag,lflag;
	Filename filename;
	FILE *fp;


	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&sflag,&frow,&fcol,&nrow,&ncol,
		&lflag,&frow,&fcol,&lrow,&lcol,FFONE,&filename);
	if (!sflag && !lflag)
		perr(HE_MSG,"either -l or -s must be specified");
	if (lflag) {
		nrow = (lrow-frow)+1;
		ncol = (lcol-fcol)+1;
	}
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	setroi(&hd,frow,fcol,nrow,ncol);
	write_headeru(&hd,argc,argv);
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
