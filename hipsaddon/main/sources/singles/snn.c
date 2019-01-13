static char *SccsId = "%W%      %G%";

/*      Copyright (c) 1991 The Turing Institute

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * snn - apply a symmetric-nearest-neighbour filter to each image
 *       of the input sequence
 *
 * usage:   snn [-d] [-s size]
 *
 * where 
 *
 *
 *     -d        specifies the labelling method involved. With -d "median
 *               value method" is used while without it "mean value method" 
 *               is used.
 *
 *    size       is an integer which specifies the width of the window 
 *               masked on every pixel when the pixel is processed. It 
 *               should be an odd number, and defaults to 3 which means
 *               a 3 x 3 window. If an even number is specified, it will
 *               be reduced by 1 automatically.
 *
 *
 * to load:	cc -o snn snn.c -lhips -lhips
 *
 * Jin Zhengping -16/10/86 
 * Rewritten by Jin Zhengping - 31 August 1991
 */

#include <stdio.h>
#include <hipl_format.h>
int types[] = {PFBYTE,LASTTYPE};
static Flag_Format flagfmt[] =
{
	{"d",
		{LASTFLAG},
		0,
		{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
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
	int             size,med;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&med,&size,FFONE,&filename);
	fp=hfopenr(filename);
	if (size < 3) 
		perr(HE_MSG, "unreasonable window width specified.") ;
	fread_hdr_a(fp,&hd,filename);
	method=fset_conversion(&hd,&hdp,types,filename);
	dup_headern(&hdp,&hdo);
	alloc_image(&hdo);
	write_headeru2(&hd,&hdo,argc,argv,hips_convback);
	fr = hdp.num_frame;
	for (f=0;f<fr;f++)
	{
	        fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_snn(&hdp,&hdo,med,size);
		write_imagec(&hd,&hdo,method,hips_convback,f);
	}
	return(0);
}
