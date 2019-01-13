static char *SccsId = "%W%      %G%";

/*      Copyright (c) 1991 The Turing Institute

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * mls - apply a maximum-likelihood-smoothing filter to each image
 *       of the input sequence
 *
 * usage:	mls [-p perc] [-s size [ssize]]
 *
 * where "perc" is the percentage (x 100) of the nearest neighbours
 * in the original window which have the closest grey levels to that
 * of the central pixel. "perc" defaults to 62. "size" is the
 * width of the window in which mls filtering is performed and it
 * defaults to 3. "Ssize" is the width of the windows which make
 * up the original window, to find the window with the greatest
 * concentration of the nearest neighbours. "Ssize" defaults to
 * (size + 1) / 2.
 *
 * to load:	cc -o mls mls.c -lhipsa -lhips
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
	{"p",
		{LASTFLAG},
		1,
		{{PTINT,"62","perc"},LASTPARAMETER}},
	{"s",
		{LASTFLAG},
		1,
		{{PTINT,"3","size"},{PTINT,"-1","ssize"},LASTPARAMETER}},
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
	int             k,size,ssize;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&k,&size,&ssize,FFONE,&filename);
	if(size<1)
		perr(HE_MSG, "unreasonable size specified.");
	if (k<0 || k>100)
		perr(HE_MSG, "unreasonable perc specified.");
	if (ssize==-1)
		ssize = (size + 1) / 2;
	if (ssize<1 || ssize>size)
		perr(HE_MSG, "unreasonable ssize specified.");
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
		h_mls(&hdp,&hdo,k,size,ssize);
		write_imagec(&hd,&hdo,method,hips_convback,f);
	}
	return(0);
}
