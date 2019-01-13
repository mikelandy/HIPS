static char *SccsId = "%W%      %G%";

/*      Copyright (c) 1991 The Turing Institute

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/* unoise - add random noise of uniform distribution to each image of
 *          the input sequence
 *		 
 * usage: unoise [-s sdev]
 *
 * where
 *    sdev:   sdev is the standard deviation and defaults to 1.0.
 *
 * to load: cc -o unoise unoise.c -lhipsa -lhips -lm
 *
 * Jin Zhengping 11/9/86
 * Rewritten by Jin Zhengping - 31 August 1991
 */

#include <hipl_format.h>
#include <stdio.h>

int types[] = {PFBYTE,PFFLOAT,LASTTYPE};
static Flag_Format flagfmt[] =
{
	{"s",
		{LASTFLAG},
		1,
		{{PTDOUBLE,"1.0","sdev"},LASTPARAMETER}},
	LASTFLAG
};

int main(argc,argv)

int	argc;
char	**argv;

{	
	struct          header hd,hdp;
	int             method,f,fr;
	Filename        filename;
	FILE            *fp;

	double		sdev;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&sdev,FFONE,&filename);
	if(sdev<=0.0)
		perr(HE_MSG,"sdev must be > 0");
	fp=hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method=fset_conversion(&hd,&hdp,types,filename);
	write_headeru2(&hd,&hdp,argc,argv,hips_convback);
	fr = hdp.num_frame;
	for (f=0;f<fr;f++)
	{
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_unoise(&hdp,sdev);
		write_imagec(&hd,&hdp,method,hips_convback,f);
	}
	return(0);
}
