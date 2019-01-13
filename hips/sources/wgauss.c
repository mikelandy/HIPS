/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * wgauss - window a sequence with a 3-dimensional Gaussian envelope
 *
 * usage: wgauss [-x xsigma] [-y ysigma] [-t tsigma] [-m tmu ymu xmu] <isq >osq
 *
 * If any standard deviation is omitted, it is treated as infinite.  If any
 * mean is omitted, it defaults to the center of the image sequence for that
 * particular dimension.
 *
 * pixel formats handled directly: FLOAT
 *
 * to load:	cc -o wgauss wgauss.c -lhipsh -lhips
 *
 * Hips 2 - msl - 8/10/91
 */

#include <stdio.h>
#include <hipl_format.h>
#include <math.h>

static Flag_Format flagfmt[] = {
	{"x",{LASTFLAG},1,{{PTDOUBLE,"-1.","xsigma"},LASTPARAMETER}},
	{"y",{LASTFLAG},1,{{PTDOUBLE,"-1.","ysigma"},LASTPARAMETER}},
	{"t",{LASTFLAG},1,{{PTDOUBLE,"-1.","tsigma"},LASTPARAMETER}},
	{"m",{LASTFLAG},3,{{PTDOUBLE,"-999.","tmu"},{PTDOUBLE,"-999.","ymu"},
		{PTDOUBLE,"-999.","xmu"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFFLOAT,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp;
	int method,fr,f,nc,c;
	Filename filename;
	FILE *fp;
	double xsigma,ysigma,tsigma,xmu,ymu,tmu,factor,twosigmasq,diff;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&xsigma,&ysigma,&tsigma,&tmu,&ymu,&xmu,
		FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	write_headeru2(&hd,&hdp,argc,argv,hips_convback);
	nc = hdp.numcolor;
	fr = hdp.num_frame / nc;
	if (tmu == -999.)
		tmu = ((double) fr - 1)/2.;
	if (ymu == -999.)
		ymu = ((double) hdp.rows - 1)/2.;
	if (xmu == -999.)
		xmu = ((double) hdp.cols - 1)/2.;
	twosigmasq = 2.*tsigma*tsigma;
	if (tsigma < 0.)
		factor = 1.;
	for (f=0;f<fr;f++) {
		for (c=0;c<nc;c++) {
			fread_imagec(fp,&hd,&hdp,method,3*f+c,filename);
			diff = f - tmu;
			if (tsigma >= 0.)
				factor = exp(-diff*diff/twosigmasq);
			h_wgauss(&hdp,&hdp,ymu,xmu,ysigma,xsigma,factor);
			write_imagec(&hd,&hdp,method,hips_convback,3*f+c);
		}
	}
	return(0);
}
