/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * bclean.c - remove small 8-connected components
 *
 * usage:	bclean [-s size] <iseq >oseq
 *
 * bclean is used to delete 8-connected components in a white-on-black
 * or thinned and categorized image of size less than or equal to a given
 * amount.  Size defaults to 1.
 *
 * to load:	cc -o bclean bclean.c -lhipsh -lhips -lm
 *
 * Mike Landy - 11/24/82
 * HIPS 2 - msl - 8/8/91
 */

#include <stdio.h>
#include <hipl_format.h>

int types[] = {PFBYTE,LASTTYPE};
static Flag_Format flagfmt[] = {
    {"s",{LASTFLAG},1,{{PTINT,"1","size"},LASTPARAMETER}},
    LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	int f,fr,method,size;
	struct header hd,hdp;
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&size,FFONE,&filename);
	fp = hfopenr(filename);
	if (size < 1)
		perr(HE_MSG,"size must be at least 1");
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	write_headeru2(&hd,&hdp,argc,argv,hips_convback);
	fr = hdp.num_frame;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_bclean(&hdp,size);
		write_imagec(&hd,&hdp,method,hips_convback,f);
	}
	return(0);
}
