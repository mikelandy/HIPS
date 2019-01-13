static char *SccsId = "%W%      %G%";

/*      Copyright (c) 1991 The Turing Institute 

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * cshift - shift circularlly each image in the input sequence.
 *
 * usage:  cshift [-x srow] [-y scol]
 *
 * where
 *       srow (scol):  starting row (column) that will be shifted to
 *                     the 0th row (column), and defaults to rows/2,
 *                     (cols/2) where rows (cols) is the number of rows
 *                     (columns) of the image.
 * This module handles images of byte, short, int, float, double, complex,
 * and double complex formats.
 *
 * to load:	cc -o cshift cshift.c -lhipsa -lhips
 *
 * Jin Zhengping - 17/7/87
 * Rewritten by Jin Zhengping - 31 August 1991
 */

#include <hipl_format.h>
#include <stdio.h>
int types[] = {PFBYTE,PFSHORT,PFINT,PFFLOAT,PFDOUBLE,PFCOMPLEX,PFDBLCOM,LASTTYPE};
static Flag_Format flagfmt[] =
{
	{"x",
		{LASTFLAG},
		1,
		{{PTINT,"-1","srow"},LASTPARAMETER}},
	{"y",
		{LASTFLAG},
		1,
		{{PTINT,"-1","scol"},LASTPARAMETER}},
	LASTFLAG
};

int main(argc,argv)
int argc;
char **argv;
{
	struct          header hd,hdp,hdo;
	int             method,f,fr;
	Filename        filename;
	FILE            *fp;
	int		scol, srow ;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&srow,&scol,FFONE,&filename);
	fp=hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	if(srow==-1)
		srow = hd.orows-hd.orows/2 ;
	if(scol==-1)
		scol = hd.ocols-hd.ocols/2 ;
	if (srow<0||srow>hd.orows||scol<0||scol>hd.ocols) 
		perr(HE_MSG,"shift amount ??\n");
	method=fset_conversion(&hd,&hdp,types,filename);
	dup_headern(&hdp,&hdo);
	alloc_image(&hdo);
	write_headeru2(&hd,&hdo,argc,argv,hips_convback);
	fr = hdp.num_frame;
	for (f=0;f<fr;f++)
	{
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_cshift(&hdp,&hdo,srow,scol);
		write_imagec(&hd,&hdo,method,hips_convback,f);
	}
	return(0);
}
