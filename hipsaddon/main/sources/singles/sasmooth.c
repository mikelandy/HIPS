static char *SccsId = "%W%      %G%";

/*      Copyright (c) 1991 The Turing Institute

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * sasmooth - apply a selective-averaging-smoothing filter to each image of
 *            the input sequence
 *
 * usage:	sasmooth [-s size] [-t threshold]
 *
 * where "size" is the size of the window in which sas is computed and
 * defaults to 3. threshold is a threshold and defaults to 2.
 *
 * to load:	cc -o sasmooth sasmooth.c -lhipsa -lhips 
 *
 * Peter Mowforth & Jin Zhengping - 8/5/85
 * Rewritten by Jin Zhengping - 31 August 1991
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
	{"t",
		{LASTFLAG},
		1,
		{{PTINT,"2","threshold"},LASTPARAMETER}},
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
	int             size,t;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&size,&t,FFONE,&filename);
	fp=hfopenr(filename);
	if (size<1)
		perr(HE_MSG,"size must be greater than 1");
	if (size>21)
		perr(HE_MSG,"large size specified - this will be slow");
	if (t<0 || t>256)
		perr(HE_MSG,"unreasonable threshold specified");

	fread_hdr_a(fp,&hd,filename);
	method=fset_conversion(&hd,&hdp,types,filename);
	dup_headern(&hdp,&hdo);
	alloc_image(&hdo);
	write_headeru2(&hd,&hdo,argc,argv,hips_convback);
	fr = hdp.num_frame;
	for (f=0;f<fr;f++)
	{
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_sasmooth(&hdp,&hdo,size,t);
		write_imagec(&hd,&hdo,method,hips_convback,f);
	}
	return(0) ;
}
