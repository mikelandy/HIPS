static char *SccsId = "%W%      %G%";

/*      Copyright (c) 1991 The Turing Institute

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * extend - double the size of each image in the input sequence by
 *          filling the extended area with 0, the boundary values of
 *          that image, or the opposite half image as if the image
 *          is first extended by repeating itself and then wrapped
 *          by half the image size. (Used for FFT.)
 *
 * usage:	extend [-b | -w]
 *
 *     -b:       a flag specifying that boundary values are used
 *               to fill the extended area, otherwise 0 is used
 *               if -w is not set.
 *
 *     -w:       a flag specifying that wrapping is to be applied to
 *               fill the extended area, otherwise 0 is used
 *               if -b is not set.
 *
 * The module handles images of byte, float, or double format.
 *
 * to load:	cc -o extend extend.c -lhipsa -lhips
 *
 * Jin Zhengping - 17/7/87
 * Rewritten by Jin Zhengping - 31 August 1991
 */

#include <hipl_format.h>
#include <stdio.h>
int types[] = {PFBYTE,PFFLOAT,PFDOUBLE,LASTTYPE};
static Flag_Format flagfmt[] =
{
	{"b",
		{"w",LASTFLAG},
		0,
		{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"w",
		{"b",LASTFLAG},
		0,
		{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
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
	h_boolean		boundary,wrapping;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&boundary,&wrapping,FFONE,&filename);
	fp=hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method=fset_conversion(&hd,&hdp,types,filename);
	dup_headern(&hdp,&hdo);
	setsize(&hdo,hd.orows*2,hd.ocols*2); 
	alloc_image(&hdo);
	write_headeru2(&hd,&hdo,argc,argv,hips_convback);
	fr = hdp.num_frame;
	for (f=0;f<fr;f++)
	{
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_extend(&hdp,&hdo,boundary,wrapping);
		write_imagec(&hd,&hdo,method,hips_convback,f);
	}
	return(0);
}
