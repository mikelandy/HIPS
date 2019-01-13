static char *SccsId = "%W%      %G%";

/*      Copyright (c) 1991 The Turing Institute

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * framing - paint a thin frame around the image 
 *
 * usage:	framing [-f frameval]
 *
 *  where
 *      frameval:   the gray level for the frame and defaults to 0.
 *
 * to load:	cc -o framing framing.c -lhipsa -lhips
 *
 * Jin Zhengping - 16/12/87
 * Rewritten by Jin Zhengping - 31 August 1991
 */

#include <stdio.h>
#include <hipl_format.h>
int types[] = {PFBYTE,LASTTYPE};
static Flag_Format flagfmt[] =
{
	{"f",
		{LASTFLAG},
		1,
		{{PTINT,"0","frameval"},LASTPARAMETER}},
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
	int		frameval;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&frameval,FFONE,&filename);
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
		h_framing(&hdp,&hdo,frameval);
		write_imagec(&hd,&hdo,method,hips_convback,f);
	}
	return(0) ;
}
