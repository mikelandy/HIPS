/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * median - apply a median filter to an image
 *
 * usage:	median [-s size]
 *
 * where size is the length of the side of the neighborhood in which the
 * median is computed.  Size defaults to 3.
 *
 * to load:	cc -o median median.c -lhipsh -lhips -lm
 *
 * Mike Landy - 5/28/82
 * median algorithm replaced <Pierre Landau 1/6/87>
 * HIPS 2 - msl - 6/16/91
 */

#include <stdio.h>
#include <hipl_format.h>

int types[] = {PFBYTE,LASTTYPE};
static Flag_Format flagfmt[] = {
    {"s",{LASTFLAG},1,{{PTINT,"3","size"},LASTPARAMETER}},
    LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	int f,fr,method,size;
	struct header hd,hdp,hdo;
	Filename filename;
	FILE *fp;
	h_boolean imagecopy;
	struct hips_roi roi;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&size,FFONE,&filename);
	fp = hfopenr(filename);
	if (size <= 1)
		perr(HE_MSG,"size must be greater than 1");
	if (size > 10)
		perr(HE_IMSG,"Large size specified - this will be slow");
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	dup_headern(&hdp,&hdo);
	alloc_image(&hdo);
	write_headeru2(&hd,&hdp,argc,argv,hips_convback);
	fr = hdp.num_frame;
	imagecopy = (hdp.rows != hdp.orows || hdp.cols != hdp.ocols) ?
		TRUE : FALSE;
	getroi(&hdp,&roi);
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		if (imagecopy) {
			clearroi(&hdp);
			clearroi(&hdo);
			h_copy(&hdp,&hdo);
			setroi2(&hdp,&roi);
			setroi2(&hdo,&roi);
		}
		h_median(&hdp,&hdo,size);
		write_imagec(&hd,&hdo,method,hips_convback,f);
	}
	return(0);
}
