static char *SccsId = "%W%      %G%";

/*      Copyright (c) 1991 The Turing Institute

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * mean - apply a mean filter to each image of the input sequence 
 *
 * usage:	mean [-s size]
 *
 * where "size" is the width of the window in which the mean averaging
 * is operated, and defaults to 3.
 *
 * to load:	cc -o mean mean.c -lhipsa -lhips
 *
 * Peter Mowforth & Jin Zhengping - 8/5/85
 * Rewritten by Jin Zhengping - 31 August 1991
 *
 */

#include <stdio.h>
#include <hipl_format.h>
int types[] = {PFBYTE,LASTTYPE};
static Flag_Format flagfmt[] =
{
	{"s",
		{LASTFLAG},
		1,
		{{PTINT,"3","size"},LASTPARAMETER}},
	LASTFLAG
};

int main(argc,argv)

int     argc;
char    **argv;

{
	struct          header hd,hdp,hdo;
	int             method,f,fr;
	Filename        filename;
	FILE            *fp;
	int             size;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&size,FFONE,&filename);
	if(size<1)
		perr(HE_MSG, "unreasonable size specified.");
	fp=hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method=fset_conversion(&hd,&hdp,types,filename);
	dup_headern(&hdp,&hdo);
	alloc_image(&hdo);
	write_headeru2(&hd,&hdo,argc,argv,hips_convback);

	fr = hdp.num_frame;
	for (f=0;f<fr;f++)
	{
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_meanfilt(&hdp,&hdo,size);
		write_imagec(&hd,&hdo,method,hips_convback,f);
	}
	return(0) ;
}
