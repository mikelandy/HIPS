/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * autodiff - compute absolute value of differences of successive frames
 *
 * usage:	autodiff [-f factor] <iseq >oseq
 *
 * Computes factor*|frame[i]-frame[i+1]|
 * For byte images the results are clipped at 0 and 255.  Factor defaults to 1.
 *
 * pixel formats handled directly: BYTE, SHORT, INT, FLOAT, DOUBLE
 *
 * to load:	cc -o autodiff autodiff.c -lhipsh -lhips
 *
 * Yoav Cohen 7/9/82
 * Hips 2 - msl - 7/7/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"f",{LASTFLAG},1,{{PTDOUBLE,"1","factor"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,PFSHORT,PFINT,PFFLOAT,PFDOUBLE,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp,hdp2,*ph1,*ph2,*ptmp;
	int method,fr,f;
	Filename filename;
	FILE *fp;
	double factor;
	Pixelval scale;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&factor,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	if (hdp.numcolor != 1)
		perr(HE_MSG,"Number of color planes must be 1, use subseq");
	if (hgetdepth(&hdp) != 1)
		perr(HE_MSG,"Number of depth planes must be 1, use subseq");
	fr = hd.num_frame = hdp.num_frame = hd.num_frame - 1;
	if (fr <= 0)
		perr(HE_MSG,"number of frames must be at least two");
	switch(hdp.pixel_format) {
	case PFBYTE:	scale.v_byte = factor; break;
	case PFSHORT:	scale.v_short = factor; break;
	case PFINT:	scale.v_int = factor; break;
	case PFFLOAT:	scale.v_float = factor; break;
	case PFDOUBLE:	scale.v_double = factor; break;
	}
	dup_headern(&hdp,&hdp2);
	alloc_image(&hdp2);
	write_headeru2(&hd,&hdp,argc,argv,hips_convback);
	fread_imagec(fp,&hd,&hdp,method,0,filename);
	ph1 = &hdp;
	ph2 = &hdp2;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,ph2,method,f+1,filename);
		h_absdiff(ph1,ph2,ph1);
		h_mulscale(ph1,ph1,&scale);
		write_imagec(&hd,ph1,method,hips_convback,f);
		ptmp=ph1; ph1=ph2; ph2=ptmp;
	}
	return(0);
}
