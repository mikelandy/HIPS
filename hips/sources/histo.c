/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * histo - compute grey-level histograms
 *
 * usage:	histo [-N numbins] [-m min max] [-c] [-n] <seq >hist
 *
 * where numbins is the number of bins (default: 256, or the number of
 * integers in the range if that is smaller than 256 (for the various integer
 * formats)), min is the left edge of the first bin (default: 0, for byte
 * sequences, and first frame minimum, for other formats), and max is the
 * right edge of the last bin (default: 255 for byte sequences, and first
 * frame maximum (plus a little for floating point formats) for other formats).
 * -c causes multiple frame sequences to collapse to a single histogram,
 * instead of a separate histogram being generated for each input frame. -n
 * causes zero-valued pixels to be ignored.  For complex and double complex
 * images, complex magnitudes are histogrammed.
 *
 * to load:	cc -o histo histo.c -lhipsh -lhips -lm
 *
 * Michael Landy - 12/14/82
 * HIPS 2 - msl - 6/30/90
 */

#include <stdio.h>
#include <hipl_format.h>
int types[] = {PFBYTE,PFSBYTE,PFSHORT,PFUSHORT,PFINT,PFUINT,PFFLOAT,PFDOUBLE,
	PFCOMPLEX,PFDBLCOM,LASTTYPE};
static Flag_Format flagfmt[] = {
    {"N",{LASTFLAG},1,{{PTBOOLEAN,"FALSE"},{PTINT,"256","numbins"},
	LASTPARAMETER}},
    {"m",{LASTFLAG},2,{{PTBOOLEAN,"FALSE"},{PTDOUBLE,"0","min"},
	{PTDOUBLE,"256","max"},LASTPARAMETER}},
    {"c",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
    {"n",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
    LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	h_boolean Nflag,mflag,cflag,nflag;
	int numbins,count,i;
	double minbin,maxbin;
	Pixelval min,max;
	int f,fr,method;
	struct header hd,hdp,hdo;
	struct hips_histo histo;
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&Nflag,&numbins,&mflag,&minbin,&maxbin,
		&cflag,&nflag,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	dup_headern(&hdp,&hdo);
	hdo.pixel_format = PFHIST;
	histo.histodealloc = FALSE;
	if (mflag) {
		switch(hdp.pixel_format) {
		case PFBYTE:	min.v_byte=minbin;max.v_byte=maxbin;break;
		case PFSBYTE:	min.v_sbyte=minbin;max.v_sbyte=maxbin;break;
		case PFSHORT:	min.v_short=minbin;max.v_short=maxbin;break;
		case PFUSHORT:	min.v_ushort=minbin;max.v_ushort=maxbin;break;
		case PFINT:	min.v_int=minbin;max.v_int=maxbin;break;
		case PFUINT:	min.v_uint=minbin;max.v_uint=maxbin;break;
		case PFFLOAT:
		case PFCOMPLEX:	min.v_float=minbin;max.v_float=maxbin;break;
		case PFDOUBLE:
		case PFDBLCOM:	min.v_double=minbin;max.v_double=maxbin;break;
		}
	}
	else if (hdp.pixel_format == PFBYTE) {
		mflag = TRUE;
		min.v_byte = 0;
		max.v_byte = 255;
	}
	fr = hdp.num_frame;
	if (cflag)
		hdo.num_frame = 1;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		if (!mflag && f==0) {
			h_minmax(&hdp,&min,&max,nflag);
			if (hdp.pixel_format == PFFLOAT ||
			    hdp.pixel_format == PFCOMPLEX)
				max.v_float +=
				    (max.v_float - min.v_float)*.00001;
			else if (hdp.pixel_format == PFDOUBLE ||
			    hdp.pixel_format == PFDBLCOM)
				max.v_double +=
				    (max.v_double - min.v_double)*.00001;
		}
		if (f == 0) {
			if (!Nflag) {
			    switch(hdp.pixel_format) {
			    case PFBYTE:   i=max.v_byte-min.v_byte;break;
			    case PFSBYTE:  i=max.v_sbyte-min.v_sbyte;break;
			    case PFSHORT:  i=max.v_short-min.v_short;break;
			    case PFUSHORT: i=max.v_ushort-min.v_ushort;break;
			    case PFINT:    i=max.v_int-min.v_int;break;
			    case PFUINT:   i=max.v_uint-min.v_uint;break;
			    case PFFLOAT:
			    case PFDOUBLE:
			    case PFCOMPLEX:
			    case PFDBLCOM: i=1000;break;
			    }
			    if (i+1 < 256)
				numbins = i+1;
			}
			alloc_histo(&histo,&min,&max,numbins,hdp.pixel_format);
			histo_to_hdr(&hdo,&histo);
			write_headeru(&hdo,argc,argv);
		}
		if (!cflag || f==0)
			h_clearhisto(&histo);
		h_histo(&hdp,&histo,nflag,&count);
		if (!cflag)
			write_histo(&histo,f);
	}
	if (cflag)
		write_histo(&histo,0);
	return(0);
}
