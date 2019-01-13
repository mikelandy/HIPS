static char *SccsId = "%W%      %G%";

/*      Copyright (c) 1991 The Turing Institute

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * ellipse - generate an ellipse line drawing image
 *
 * usage:	ellipse [-c cenr cenc] [-d rows cols] [-r radr radc]
 *
 * where  
 *
 *    cenr cenc:  the row and column position of the center of the 
 *                ellipse and defaults to the middle of the image.
 *
 *    rows cols:  the number of rows and columns of the image generated
 *                and defaults to (128, 128).
 *
 *    radr radc:  the vertical and horizontal radiuses of the ellipse
 *                and defaults to the value as large as the ellipse thus
 *                drawn can be contained totally within the image.
 *
 *
 * to load:	cc -o ellipse ellipse.c -lhipsa -lhips -lm
 *
 * Jin Zhengping -12/6/85 
 * Jin Zhengping -14/2/86 Ed.2 
 * Rewritten by Jin Zhengping - 31 August 1991
 */

#include <stdio.h>
#include <hipl_format.h>

int types[] = {PFBYTE,LASTTYPE};
static Flag_Format flagfmt[] =
{
	{"c",
		{LASTFLAG},
		2,
		{{PTINT,"0","cenr"},{PTINT,"0","cenc"},LASTPARAMETER}},
	{"d",
		{LASTFLAG},
		2,
		{{PTINT,"128","rows"},{PTINT,"128","cols"},LASTPARAMETER}},
	{"r",
		{LASTFLAG},
		2,
		{{PTDOUBLE,"0","radr"},{PTDOUBLE,"0","radc"},LASTPARAMETER}},
	LASTFLAG
};

int main(argc,argv)

int     argc;
char    **argv;

{
	struct		header hd;
	double		radr,radc;
	int		rows,cols,cenc,cenr;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&cenr,&cenc,&rows,&cols,&radr,&radc,FFNONE);
	if(rows < 0)
		perr(HE_MSG,"rows must be positive");
	if(rows > 512)
		fprintf(stderr,"%s: dimension rows could be too large %d\n", Progname, rows);
	if(cols < 0)
		perr(HE_MSG,"cols must be positive");
	if(cols > 512)
		fprintf(stderr,"%s: dimension cols could be too large %d\n", Progname, cols);

	if(!cenr) cenr = rows/2;
	if(!cenc) cenc = cols/2;
	if (cenr<0 || cenr>=rows)
		perr(HE_MSG, "unreasonable centeral rows-position cenr specified");
	if (cenc<0 || cenc>=cols)
		perr(HE_MSG, "unreasonable centeral cols-position cenc specified");
	if(!radr) 
		radr = (2*cenr<rows)? (cenr-1): (rows-cenr-1);
	if (radr<0 || radr>(cenr-1) || radr>(rows-cenr-1))
		perr(HE_MSG, "unreasonable rows-dimension radius radr specified");
	if(!radc) 
		radc = (2*cenc<cols)? (cenc-1): (cols-cenc-1);
	if (radc<0 || radc>(cenc-1) || radc>(cols-cenc-1))
		perr(HE_MSG, "unreasonable cols-dimension radius radc specified");

	init_header(&hd,"","ellipse",1,
                    "",rows,cols,PFBYTE,1,"a line drawing ellipse.");
	alloc_image(&hd);
	write_headeru(&hd,argc,argv);
	h_ellipse(&hd,cenr,cenc,radr,radc);
	write_image(&hd);
	return(0);
}
