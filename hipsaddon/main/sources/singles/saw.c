static char *SccsId = "%W%      %G%";

/*      Copyright (c) 1991 The Turing Institute

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * saw - generate an image with saw-tooth like intensity profile
 *
 * usage:	saw [-d rows cols]
 *
 * where
 *    rows cols:  the number of rows and columns of the image generated
 *                and defaults to (128, 128).
 *
 * to load:	cc -o saw saw.c -lhips
 *
 * Jin Zhengping -1/17/85 
 * Rewritten by Jin Zhengping - 31 August 1991
 *
 */

#include <stdio.h>
#include <hipl_format.h>
int types[] = {PFBYTE,LASTTYPE};
static Flag_Format flagfmt[] =
{
	{"d",
		{LASTFLAG},
		2,
		{{PTINT,"128","rows"},{PTINT,"128","cols"},LASTPARAMETER}},
	LASTFLAG
};

#define BACKGROUND 20

int main(argc,argv)

int     argc;
char    **argv;

{
	struct header	hd;
	int		rows,cols,i,j,work,up;
	byte		*op;
	int		count=0;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&rows,&cols,FFNONE);
	if(rows < 0)
		perr(HE_MSG,"rows must be positive");
	if(rows > 512)
		fprintf(stderr,"%s: dimension rows could be too large %d\n", Progname, rows);
	if(cols < 0)
		perr(HE_MSG,"cols must be positive");
	if(cols > 512)
		fprintf(stderr,"%s: dimension cols could be too large %d\n", Progname, cols);

	init_header(&hd,"","saw-tooth",1,
		"",rows,cols,PFBYTE,1,"an image of saw-tooth profile.");
	alloc_image(&hd);
	write_headeru(&hd,argc,argv);
	while(count<cols)
	{
		up = (count>cols-20)? (cols-count): 20;
		for(i=0; i<up; i++)
		{
			work = i + BACKGROUND;
			op = hd.image+count++;
			for(j=0; j<rows; j++,op+=cols)
				*op = work;
		}
	}
	write_image(&hd);
	return(0);
}
