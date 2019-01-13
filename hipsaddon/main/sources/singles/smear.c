static char *SccsId = "%W%      %G%";

/*      Copyright (c) 1991 The Turing Institute

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * smear - expand a one row image into a rectangular image by repeating that row.
 *
 * usage:	smear [-n rows]
 *
 * where 
 *
 *    rows     the number of the rows to be repeated, and defaults to the 
 *             width of the input image so that the output image will be
 *             square.
 *
 * to load:	cc -o smear smear.c -lhips
 *
 * Peter Mowforth & Jin Zhengping -13/1/86 
 * Rewritten by Jin Zhengping - 31 August 1991
 */

#include <stdio.h>
#include <hipl_format.h>
int types[] = {PFBYTE,LASTTYPE};
static Flag_Format flagfmt[] =
{
	{"n",
		{LASTFLAG},
		1,
		{{PTINT,"-1","rows"},LASTPARAMETER}},
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
	int             rows,cols;

	int		i,j;
	byte		*ip,*op;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&rows,FFONE,&filename);
	fp=hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	if (rows==-1)
		rows = hd.ocols;
	cols = hd.ocols;
	method=fset_conversion(&hd,&hdp,types,filename);
	dup_headern(&hdp,&hdo);
	setsize(&hdo,rows,cols);
	alloc_image(&hdo);
	write_headeru2(&hd,&hdo,argc,argv,hips_convback);
	fr = hdp.num_frame;
	for(f=0;f<fr;f++)
	{
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		for(ip=hdp.image,op=hdo.image,i=0;i<cols;i++,op++,ip++)
			for(j=0; j<rows*cols; j+=cols)
				*(op + j) = *ip;
		write_imagec(&hd,&hdo,method,hips_convback,f);
	}
	return(0);
}
