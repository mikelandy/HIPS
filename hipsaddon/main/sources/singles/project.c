static char *SccsId = "%W%      %G%";

/*	Copyright (c) 1991 The Turing Institute

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * project - project each image in the input sequence onto a tilted
 *           plane
 *
 * usage:	project [-a angle] [-d rows cols] [-l distance] [-s scale]
 *
 *  angle       "angle" is a double-floating point value for the degree of the
 *              angle subtended by the projecting plane and the image plane.
 *              The projecting plane only rotates about the horizontal axis
 *              of the image plane. It defaults to 10.0.
 *
 *  rows,cols   (rows,cols) is a integer pair specifying the size of the
 *              output image. It defaults to the size of the input image.
 *
 *  distance    "distance" is a double-floating poin value for the distance from 
 *              perspective focus point to the centre of the image. It
 *              defaults to half of the height of the image.
 *
 *  scale       "scale" is a double-floating poin value for the scale by which the
 *              image is magnified. It defaults to 1.0.
 *
 *  Note: any part of the projected image that lie outside the output
 *  image will be clipped.
 *
 *
 * to load:	cc -o project project.c -lhipsa -lhips -lm
 *
 * Jin Zhengping -8/3/89 
 * Rewritten by Jin Zhengping - 31 August 1991
 *
 */

#include <stdio.h>
#include <hipl_format.h>
#include <math.h>

#define WARNINGSIZE	512 
#define MY_PI              3.1415927

int types[] = {PFBYTE,LASTTYPE};
static Flag_Format flagfmt[] =
{
	{"a",
		{LASTFLAG},
		1,
		{{PTDOUBLE,"10.0","angle"},LASTPARAMETER}},
	{"d",
		{LASTFLAG},
		2,
		{{PTINT,"-1","rows"},{PTINT,"-1","cols"},LASTPARAMETER}},
	{"l",
		{LASTFLAG},
		1,
		{{PTDOUBLE,"-1.0","distance"},LASTPARAMETER}},
	{"s",
		{LASTFLAG},
		1,
		{{PTDOUBLE,"1.0","scale"},LASTPARAMETER}},
	LASTFLAG
};


int main(argc,argv)

int	argc;
char	**argv ;

{
	struct          header hd,hdp,hdo;
	int             method,f,fr;
	Filename        filename;
	FILE            *fp;
	double		distance,angle,scale;
	int		rowsn,colsn;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,
		&angle,&rowsn,&colsn,&distance,&scale,
		FFONE,&filename);
	fp=hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);

	angle -= floor(angle/360.0)*360 ;
	if (angle>90.0)
		perr(HE_MSG,"reasonable angle specified?");

	if(distance==-1.0)
		distance = hd.orows/2;
	if(distance<0.0)
		distance = -distance ;

	if (scale<=0.0)
		perr(HE_MSG,"reasonable scale specified?");

        if (rowsn==-1)
	{
		double	cosl=cos(angle*MY_PI/180.0);
		double	tanl=tan(angle*MY_PI/180.0);
		double	tmpf=1.0/((1.0/scale)-tanl*hd.orows/distance);
		rowsn = hd.orows*tmpf/cosl;
		colsn = hd.ocols*tmpf;
	}
        if(rowsn<=0 || colsn <= 0)
		perr(HE_MSG,"reasonable image size specified?");
	if(rowsn > WARNINGSIZE || colsn > WARNINGSIZE)
	{
		fprintf(stderr,"\n\nWarning: the size of the output image is larger than %d.\n",
			       WARNINGSIZE) ;
		fprintf(stderr,"         rows = %d, columns = %d\n\n", rowsn, colsn) ;
		fprintf(stderr,"         Reasonable angle, distance and/or scale specified?\n") ;
		fprintf(stderr,"         angle = %.1f, distance = %.1f, scale = %.1f\n\n",
			       angle,distance,scale) ;
	}

	method=fset_conversion(&hd,&hdp,types,filename);
	dup_headern(&hdp,&hdo);
	setsize(&hdo,rowsn,colsn);
	alloc_image(&hdo);
	write_headeru2(&hd,&hdo,argc,argv,hips_convback);
	fr = hdp.num_frame;
	for (f=0;f<fr;f++)
	{
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_project(&hdp,&hdo,angle,distance,scale);
		write_imagec(&hd,&hdo,method,hips_convback,f);
	}
	return(0);
}
