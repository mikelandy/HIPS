static char *SccsId = "%W%      %G%";

/*	Copyright (c) 1989 The Turing Institute

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * mandelbrot.c - generate a subset of a mandelbrot set 
 *
 * usage:	mandelbrot [-a lb rb] [-b ub bb] [-d rows cols]
 *                         [-e eps] [-n ite]
 *
 *    lb,rb       (lb,rb) are reals specifying the horizontal boundaries
 *                of the subset, and defaults to (-0.750,-0.746).
 *
 *    ub,bb       (ub,bb) are reals specifying the vertical boundaries
 *                of the subset, and defaults to (0.0986,0.1014).
 *
 *    rows,cols   (rows,cols) are integers specifying the size of the output
 *                image, and defaults to (256,341).
 *
 *    eps         eps is a real specifying the torelance to the trucation error,
 *                and defaults to 0.01.
 *
 *    ite         ite is an integer to specify the maximum number of iterations,
 *                and defaults to 300.
 *
 *
 * to load:	cc -o mandelbrot mandelbrot.c -lhips -lm
 *
 * Jin Zhengping -8/3/89 
 * Rewritten by Jin Zhengping - 31 August 1991
 */

#include <stdio.h>
#include <math.h>
#include <hipl_format.h>
int types[] = {PFBYTE,LASTTYPE};
static Flag_Format flagfmt[] =
{
	{"a",
		{LASTFLAG},
		2,
		{{PTDOUBLE,"-0.750","lb"},{PTDOUBLE,"-0.746","rb"},LASTPARAMETER}},
	{"b",
		{LASTFLAG},
		2,
		{{PTDOUBLE,"0.0986","ub"},{PTDOUBLE,"0.1014","bb"},LASTPARAMETER}},
	{"d",
		{LASTFLAG},
		2,
		{{PTINT,"256","rows"},{PTINT,"341","cols"},LASTPARAMETER}},
	{"e",
		{LASTFLAG},
		1,
		{{PTDOUBLE,"0.01","eps"},LASTPARAMETER}},
	{"n",
		{LASTFLAG},
		1,
		{{PTINT,"300","ite"},LASTPARAMETER}},
	LASTFLAG
};
#define BACKGROUND 0


int main(argc,argv)

int     argc;
char    **argv;

{
	struct header	hd;
	int		i,j,rows,cols;
	byte		*ofrp;

	double		a,b,al,ar,bl,br;
	double		stepa,stepb,x,y,x2,y2,tmpDb,eps;
	int		maxnum,num;
	float		scale;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&al,&ar,&bl,&br,&rows,&cols,&eps,&maxnum,FFNONE);
	if (rows <= 0 || cols <= 0)
		perr(HE_MSG, "reasonable image size specified?") ;

	init_header(&hd,"","mandelbrot",1,
		"",rows,cols,PFBYTE,1,"a mandelbrot set.");
	alloc_image(&hd);
	write_headeru(&hd,argc,argv);

	stepa = (ar - al) / cols ;
	stepb = (br - bl) / rows ;
	scale = 255.0 / maxnum ;

	ofrp = hd.image ;
	for(b=br,i=0; i<rows; i++,b-=stepb)
	    for(a=al,j=0; j<cols; j++,a+=stepa,ofrp++)
	    {
		*ofrp = BACKGROUND ;
		x = a ; y = b ;
		num = 0 ;
		while(num++<maxnum)
		{
			x2 = x*x ; y2 = y*y ;
			y = 2.0 * x * y + b ;
			x = x2 - y2 + a ;

			tmpDb = sqrt(x2 + y2) ;
			if(tmpDb>=2.0)
				if((tmpDb - floor(tmpDb)) < eps)
				{
					*ofrp = num * scale ;
					num = maxnum ;
				}
		}
	    }
	write_image(&hd);

	return(0) ;
}
