static char *SccsId = "%W%      %G%";

/*      Copyright (c) 1991 The Turing Institute

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * rotate - rotate each image in the input sequence about its central pixel
 *
 * usage:	rotate [-a angle] [-d rows cols]
 *
 *    angle       "angle" is a double-floating point value for the degree of
 *                angle to be rotated. It defaults to 90.0.
 *
 *    rows,cols   (rows,cols) is a pair of integers specifying the size
 *                of the output image. It defaults to the size of the
 *                input image.
 *
 *  Note: any part of the rotated image that lie outside the output
 *  image will be clipped.
 *
 * to load:	cc -o rotate rotate.c -lhipsa -lhips -lm
 *
 * Jin Zhengping -8/3/89 
 * Rewritten by Jin Zhengping - 31 August 1991
 * added FLOAT - Mike Landy - 23 June 1998
 */

#include <stdio.h>
#include <math.h>
#include <hipl_format.h>
int types[] = {PFBYTE,PFFLOAT,LASTTYPE};
static Flag_Format flagfmt[] =
{
        {"a",
                {LASTFLAG},
                1,
                {{PTDOUBLE,"90.0","angle"},LASTPARAMETER}},
        {"d",
                {LASTFLAG},
                2,
                {{PTINT,"-1","rows"},{PTINT,"-1","cols"},LASTPARAMETER}},
        LASTFLAG
};


int main(argc,argv)

int     argc;
char    **argv ;
 
{
	struct          header hd,hdp,hdo;
	int             method,f,fr;
	Filename        filename;
	FILE            *fp;
	double		angle;
	int     	rowsn,colsn;

	byte		interpolation();

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&angle,&rowsn,&colsn,FFONE,&filename);
	fp=hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	angle -= floor(angle/360.0)*360 ;
	if (rowsn==-1)
	{
		rowsn = hd.orows ;
		colsn = hd.ocols ;
	}
	if (rowsn <= 0 || colsn <= 0)
		perr(HE_MSG,"reasonable image size specified?");
	method=fset_conversion(&hd,&hdp,types,filename);
	dup_headern(&hdp,&hdo);
	setsize(&hdo,rowsn,colsn);
	alloc_image(&hdo);
	write_headeru2(&hd,&hdo,argc,argv,hips_convback);
	fr = hdp.num_frame;
	for (f=0;f<fr;f++)
	{
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_rotate(&hdp,&hdo,angle);
		write_imagec(&hd,&hdo,method,hips_convback,f);
	}
	return(0);
}
