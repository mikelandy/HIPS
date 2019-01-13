static char *SccsId = "%W%      %G%";

/*      Copyright (c) 1991 The Turing Institute

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * mark - mark a row or column in each image of the input sequence
 *
 * usage:	mark [-l line] [-v]
 *
 * where "v" is an option for marking vertical line in the image and 
 * defaults to marking horizontal line. "line" specifies the "line"-th
 * row or column is to be marked.  "Line" defaults to half of the
 * number of rows or columns of the image.
 *
 * to load:	cc -o mark mark.c -lhips
 *
 * Peter Mowforth & Jin Zhengping -8/5/85 
 * Rewritten by Jin Zhengping - 31 August 1991
 */

#include <stdio.h>
#include <hipl_format.h>
int types[] = {PFBYTE,LASTTYPE};
static Flag_Format flagfmt[] =
{
	{"l",
		{LASTFLAG},
		1,
		{{PTINT,"-1","line"},LASTPARAMETER}},
	{"v",
		{LASTFLAG},
		0,
		{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG
};

#define WHITE 0377
#define BLACK 0000

int main(argc,argv)

int     argc;
char    **argv;

{
	struct          header hd,hdp;
	int             method,f,fr;
	Filename        filename;
	FILE            *fp;

	h_boolean		verline;
	int		line,i;
	byte		*ifrp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&line,&verline,FFONE,&filename);
	fp=hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	if(line==-1)
		line = ((verline==TRUE)? hd.ocols: hd.orows)/2;
	if((verline==TRUE && (line<0 || line>= hd.ocols))
	|| (verline==FALSE && (line<0 || line>= hd.orows)))
		perr(HE_MSG,"unreasonable line specified.");
	method=fset_conversion(&hd,&hdp,types,filename);
	write_headeru2(&hd,&hdp,argc,argv,hips_convback);
	fr = hdp.num_frame;
	for (f=0;f<fr;f++)
	{
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		if (verline==TRUE) {
			for(i=1,ifrp=hdp.image+line; i<hd.orows; i+=2)
			{
				*ifrp = BLACK;
				ifrp += hd.ocols;
				*ifrp = WHITE;
				ifrp += hd.ocols;
			}
		} else
		{
			for(i=1,ifrp=hdp.image+line*hd.ocols; i<hd.ocols; i+=2)
			{
				*ifrp++ = BLACK;
				*ifrp++ = WHITE;
			}
		}
		write_imagec(&hd,&hdp,method,hips_convback,f);
	}
	return(0) ;
}
