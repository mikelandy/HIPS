/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * seehist - display histogram files as readable Ascii text
 *
 * usage:	disphist [-z] [-p] <hist
 *
 * The -z flag suppresses printing of empty bins.
 * The -p flag pipes the histograms to the standard output.
 * 
 * to load:	cc -o seehist seehist.c -lhipsh -lhips -lm
 *
 * Michael Landy - 8/17/82
 */

#include <stdio.h>
#include <hipl_format.h>
int types[] = {PFHIST,LASTTYPE};
static Flag_Format flagfmt[] = {
    {"z",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
    {"p",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
    LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd;
	struct hips_histo histo;
	Filename filename;
	FILE *fp;
	h_boolean pflag,zflag;
	int i1,i2,i,nb,fr,f;
	float f1,f2;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&zflag,&pflag,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_cpfa(fp,&hd,types,filename);
	if (pflag)
		write_header(&hd);
	hdr_to_histo(&hd,&histo);
	alloc_histobins(&histo);
	fr = hd.num_frame;
	switch (histo.pixel_format) {
	case PFBYTE:	i1 = histo.minbin.v_byte;
			i2 = histo.binwidth.v_byte;
			break;
	case PFSBYTE:	i1 = histo.minbin.v_sbyte;
			i2 = histo.binwidth.v_sbyte;
			break;
	case PFSHORT:	i1 = histo.minbin.v_short;
			i2 = histo.binwidth.v_short;
			break;
	case PFUSHORT:	i1 = histo.minbin.v_ushort;
			i2 = histo.binwidth.v_ushort;
			break;
	case PFINT:	i1 = histo.minbin.v_int;
			i2 = histo.binwidth.v_int;
			break;
	case PFUINT:	i1 = histo.minbin.v_uint;
			i2 = histo.binwidth.v_uint;
			break;
	case PFFLOAT:
	case PFCOMPLEX:	f1 = histo.minbin.v_float;
			f2 = histo.binwidth.v_float;
			break;
	case PFDOUBLE:
	case PFDBLCOM:	f1 = histo.minbin.v_double;
			f2 = histo.binwidth.v_double;
			break;
	default:	perr(HE_FMT,hformatname(histo.pixel_format));
	}
	fprintf(stderr,"region size was %d x %d\n",hd.rows,hd.cols);
	fprintf(stderr,"image format was %s\n",hformatname(histo.pixel_format));
	nb = histo.nbins;
	if (histo.pixel_format == PFFLOAT ||
	    histo.pixel_format == PFCOMPLEX ||
	    histo.pixel_format == PFDOUBLE ||
	    histo.pixel_format == PFDBLCOM)
		fprintf(stderr,
			"left edge: %f; bin width: %f; number of bins: %d",
			f1,f2,nb);
	else
		fprintf(stderr,
			"left edge: %d; bin width: %d; number of bins: %d",
			i1,i2,nb);
	for (f=0;f<fr;f++) {
		fread_histo(fp,&histo,f,filename);
		if (pflag)
			write_histo(&histo,f);
		fprintf(stderr,"\nFrame %d\n\n",f);
		if (histo.pixel_format == PFFLOAT ||
		    histo.pixel_format == PFCOMPLEX ||
		    histo.pixel_format == PFDOUBLE ||
		    histo.pixel_format == PFDBLCOM)
			fprintf(stderr,"min  <=  pix  <  max\t\tcount\n\n");
		else
			fprintf(stderr,"min  <=  pix  <  max\tcount\n\n");
		for (i=0;i<=nb+1;i++) {
			if (histo.histo[i] || !zflag) {
				if (i == 0) {
					if (histo.pixel_format == PFFLOAT ||
					    histo.pixel_format == PFCOMPLEX ||
					    histo.pixel_format == PFDOUBLE ||
					    histo.pixel_format == PFDBLCOM)
						fprintf(stderr,
							"-infin\t%f",f1);
					else
						fprintf(stderr,
							"-infin\t\t%d",i1);
				}
				else if (i == nb+1) {
					if (histo.pixel_format == PFFLOAT ||
					    histo.pixel_format == PFCOMPLEX ||
					    histo.pixel_format == PFDOUBLE ||
					    histo.pixel_format == PFDBLCOM)
						fprintf(stderr,
							"%f\t+infin",
							f1+nb*f2);
					else
						fprintf(stderr,
							"%d\t\t+infin",
							i1+nb*i2);
				}
				else {
					if (histo.pixel_format == PFFLOAT ||
					    histo.pixel_format == PFCOMPLEX ||
					    histo.pixel_format == PFDOUBLE ||
					    histo.pixel_format == PFDBLCOM)
						fprintf(stderr,
							"%f\t%f",
							f1+(i-1)*f2,
							f1+i*f2);
					else
						fprintf(stderr,
							"%d\t\t%d",
							i1+(i-1)*i2,
							i1+i*i2);
				}
				fprintf(stderr,"\t%d\n",histo.histo[i]);
			}
		}
	}
	return(0);
}
