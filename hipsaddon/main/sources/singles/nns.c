static char *SccsId = "%W%      %G%";

/*	Copyright (c) 1991 The Turing Institute 

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * nns - apply a nearest-neighbour-smoothing filter to each image of
 *       the input sequence
 *
 * usage:	nns [-s size [k]]
 *
 * Nns applies a nearest-neighbour-smoothing filter to the input
 * sequence. Size is the window size in which 
 * nns is computed and defaults to 3. k is the number of nearest 
 * neighbours to be averaged and defaults to size*(size+1)/2-1.
 * This filter works on byte images.
 *
 * to load:	cc -o nns nns.c -lhipsa -lhips
 *
 * Peter Mowforth and Jin Zhengping - 8/5/85
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
		{{PTINT,"3","size"},{PTINT,"-1","k"},LASTPARAMETER}},
	LASTFLAG
};

int main(argc,argv)

int	argc;
char	**argv;

{
	struct		header hd,hdp,hdo;
	int		method,f,fr;
	Filename	filename;
	FILE		*fp;
	int		size,k;


	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&size,&k,FFONE,&filename);
	fp=hfopenr(filename);
	if (size <= 1)
		perr(HE_MSG,"size must be greater than 1");
	if (size > 10)
		perr(HE_MSG,"large size specified - this will be slow");
	if (k == -1)
		k = size * (size+1) / 2 - 1;
	if (k < 1 || k >= size*size)
		perr(HE_MSG,"k must be greater than 0 and less than size*size+1");
	fread_hdr_a(fp,&hd,filename);
	method=fset_conversion(&hd,&hdp,types,filename);
	dup_headern(&hdp,&hdo);
	alloc_image(&hdo);
	write_headeru2(&hd,&hdo,argc,argv,hips_convback);
	fr = hdp.num_frame;
	for (f=0;f<fr;f++)
	{
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_nns(&hdp,&hdo,size,k);
		write_imagec(&hd,&hdo,method,hips_convback,f);
	}
	return(0);
}
