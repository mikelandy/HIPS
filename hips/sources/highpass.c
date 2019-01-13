/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * highpass.c - apply a highpass filter to a sequence
 *
 * usage: highpass  [-b | -r] [-d metric] [-f filterno | -p highcut [highorder]]
 *				[-I | -B | -E] [-s | <inseq] >outseq
 *
 * highpass calculates a highpass filter MTF and either outputs that MTF as
 * a floating point image (with -s), or multiplies that MTF by an input
 * image sequence.  The user may specify one of a standard set of highpass
 * filters (with -f), or specify the precise filter desired (using -p for the
 * filter parameters, and -I, -B or -E to specify which filtering method:
 * Ideal, Butterworth or Exponential, which defaults to Ideal).  Filters are
 * parameterized by the cutoff frequency, and for Butterworth and Exponential
 * filters, by the order of the filter.
 *
 * The input sequence must be of images in the Fourier domain (represented
 * in complex or double complex format), or of spectra (represented in float or
 * double format), e.g. as output by fourtr(1).  If the -s option is specified
 * no input is expected, and the program outputs the MTF of the filter in float
 * format.  The MTF is plotted on a 128X128 picture.  Of course, the program
 * may be used with a uniform frame input which consists entirely of 1's in
 * order to compute a general MTF in any of the four formats.
 * Option -d specifies the distance metric used to compute frequency (the
 * Minkowski exponent, which defaults to 2, for a Euclidean metric, which
 * results in an isotropic filter).
 *
 * Option -f specifies a standard filter number according to the following
 * list (the default filter is number 1 if neither -f nor -p is specified):
 *
 *     number	method	 	cutoff	order
 *
 *	1	Ideal	  	.333
 *	2	Butterworth: 	.333    1 
 *	3	Exponential:    .333	1
 *	4	Ideal	  	.5
 *	5	Butterworth:    .5	1
 *	6	Exponential:	.5	1
 *	7	Ideal	  	.667
 *	8	Butterworth:	.667	1
 *	9	Exponential:	.667	1
 *
 * Three filter types have been implemented: Ideal, Butterworth and Exponential.
 * The cutoff frequency is expressed as a fraction of the frequency range.
 * The frequency range is controlled by the options -b or -r.  If neither is`
 * specified, then frequency "1.0" is controlled
 * by (half) the number of columns, and the rows are treated with the same
 * distance metric, so that non-square pictures may have a different range of
 * frequencies in the rows and columns.  Option -r specifies frequency "1.0"
 * by the number of rows rather than columns.  Option -b specifies both, so
 * that the distance metric for non-square pictures is different for rows and
 * columns, adjusted so that both (number of rows/2) and (number of columns/2)
 * are treated as frequency 1.0.
 *
 * To load: cc -o highpass highpass.c  -lhipsh -lhips -lm
 *
 * Yoav Cohen 5/21/82
 * modified by Mike Landy 7/13/84
 * HIPS 2 - msl - 8/9/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"b",{"r",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"r",{"b",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"d",{LASTFLAG},1,{{PTDOUBLE,"2","metric"},LASTPARAMETER}},
	{"f",{"p",LASTFLAG},1,{{PTINT,"1","filternumber"},LASTPARAMETER}},
	{"p",{"f",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},{PTDOUBLE,"0","highcut"},
		{PTINT,"1","order"},LASTPARAMETER}},
	{"I",{"E","B",LASTFLAG},0,{{PTBOOLEAN,"TRUE"},LASTPARAMETER}},
	{"E",{"I","B",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"B",{"I","E",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"s",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFFLOAT,PFDOUBLE,PFCOMPLEX,PFDBLCOM,LASTTYPE};

#define NPARAMS 2
#define NFILTERS 9

int methods[NFILTERS] =
	{FILTMETHOD_IDEAL,
	FILTMETHOD_BUTTERWORTH,
	FILTMETHOD_EXPONENTIAL,
	FILTMETHOD_IDEAL,
	FILTMETHOD_BUTTERWORTH,
	FILTMETHOD_EXPONENTIAL,
	FILTMETHOD_IDEAL,
	FILTMETHOD_BUTTERWORTH,
	FILTMETHOD_EXPONENTIAL};

double highcuts[NFILTERS] = {
		.333,
		.333,
		.333,
		.50,
		.50,
		.50,
		.667,
		.667,
		.667};

int orders[NFILTERS] = {
		0,
		1,
		1,
		0,
		1,
		1,
		0,
		1,
		1};


int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp;
	int method,fr,f,filterno,order;
	double metric,highcut;
	Filename filename;
	FILE *fp;
	h_boolean bflag,rflag,pflag,Iflag,Eflag,Bflag,sflag;
	struct hips_filter filt;
	Pixelval val;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&bflag,&rflag,&metric,&filterno,&pflag,
		&highcut,&order,&Iflag,&Eflag,&Bflag,&sflag,FFONE,&filename);
	if (sflag) {
		init_header(&hd,"","",1,"",128,128,PFFLOAT,1,"");
		alloc_image(&hd);
		dup_header(&hd,&hdp);
		val.v_float = 1.;
		h_setimage(&hd,&val);
		write_headeru(&hdp,argc,argv);
	}
	else {
		fp = hfopenr(filename);
		fread_hdr_a(fp,&hd,filename);
		method = fset_conversion(&hd,&hdp,types,filename);
		write_headeru2(&hd,&hdp,argc,argv,hips_convback);
	}
	if (pflag) {
		filt.method = Iflag ? FILTMETHOD_IDEAL :
		    (Bflag ? FILTMETHOD_BUTTERWORTH : FILTMETHOD_EXPONENTIAL);
		filt.highcut = highcut;
		filt.highorder = order;
	}
	else {
		filterno--;
		if (filterno < 0 || filterno >= NFILTERS)
			perr(HE_MSG,"invalid filter number");
		filt.method = methods[filterno];
		filt.highcut = highcuts[filterno];
		filt.highorder = orders[filterno];
	}
	filt.disttype = bflag ? FILTDIST_BOTH : (rflag ? FILTDIST_ROW :
		FILTDIST_COL);
	filt.ftype = FILTTYPE_HIGHPASS;
	filt.dmetric = metric;
	fr = hdp.num_frame;
	for (f=0;f<fr;f++) {
		if (!sflag)
			fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_filter(&hdp,&hdp,&filt);
		write_imagec(&hd,&hdp,method,hips_convback,f);
	}
	return(0);
}
