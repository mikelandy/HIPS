/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * thresh.c - apply a threshold
 *
 * usage: thresh [-p percent | -t threshold] [-f] [-s | -S] [-n]
 *
 * where percent is the percent of points to be included above the threshold
 * for each frame. This threshold is recomputed each frame unless -f is
 * specified in which case it is only computed for the first frame, and then
 * is left fixed.  The number of above-threshold points may be slightly higher
 * than this percentage.  The -t switch allows a given fixed threshold to be
 * specified.  If no switch is specified, a movable threshold of 3% is applied.
 * If the -n switch is specified, all of the zero value pixels are disregarded
 * in computing the percentage threshold.  By default a hard threshold is
 * computed where pixels greater than or equal to the threshold are set to
 * hips_hchar, and those below are set to hips_lchar.  If -s is specified then
 * a soft threshold is applied where pixels below the threshold are set to
 * hips_lchar and those greater than or equal to the threshold are left
 * unchanged.  For complex images, thresholds refer to the complex magnitude
 * of image pixels.  If -s is specified then the output is in the same format
 * as the input.  The same goes for -S (which used a hard threshold).
 * However, if neither -s nor -S is specified, then a hard threshold is
 * applied, and the output is in byte format.  The -CB standard switch applies
 * only if -s or -S is specified.
 *
 * Formats: BYTE, INTEGER, FLOAT or COMPLEX
 *
 * to load:	cc -o thresh thresh.c -lhipsh -lhips -lm
 *
 * Michael Landy (Hips-2) - 6/17/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"p",{"t",LASTFLAG},1,{{PTBOOLEAN,"TRUE"},{PTDOUBLE,"3","percent"},
		LASTPARAMETER}},
	{"t",{"p",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},{PTDOUBLE,"-1.","threshold"},
		LASTPARAMETER}},
	{"f",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"s",{"S",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"S",{"s",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"n",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,PFINT,PFFLOAT,PFCOMPLEX,LASTTYPE};
char threshmsg[] = "\n\twarning: -n specified and threshold is <= 0";

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp,hdo,hdt;
	int method,fr,f,count,i,j,k,nbins;
	Filename filename;
	FILE *fp;
	h_boolean pflag,tflag,fflag,sflag,nzflag,Sflag,convbyte;
	double percent,threshval;
	Pixelval min,max,thresh;
	struct hips_histo histo;
	char msg[200];

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&pflag,&percent,&tflag,&threshval,&fflag,
		&sflag,&Sflag,&nzflag,FFONE,&filename);
	if (percent<=0. || percent>=100.)
		perr(HE_MSG,"bad percentage specified");
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	histo.histodealloc = FALSE;
	method = fset_conversion(&hd,&hdp,types,filename);
	if (tflag) {
		switch(hdp.pixel_format) {
		case PFBYTE:	i = threshval;
				if (((double) i) != threshval)
					thresh.v_byte = i+1;
				else
					thresh.v_byte = i;
				break;
		case PFINT:	i = threshval;
				if (((double) i) != threshval)
					thresh.v_int = i+1;
				else
					thresh.v_int = i;
				break;
		case PFFLOAT:
		case PFCOMPLEX:	thresh.v_float = threshval; break;
		}
	}
	if (sflag || Sflag) {
		convbyte = FALSE;
		write_headeru2(&hd,&hdp,argc,argv,hips_convback);
	}
	else if (hdp.pixel_format == PFBYTE) {
		convbyte = FALSE;
		write_headeru(&hdp,argc,argv);
	}
	else {
		convbyte = TRUE;
		dup_headern(&hdp,&hdo);
		setformat(&hdo,PFBYTE);
		alloc_image(&hdo);
		if (hdp.pixel_format == PFCOMPLEX) {
			dup_headern(&hdp,&hdt);
			setformat(&hdt,PFINT);
			alloc_image(&hdt);
		}
		write_headeru(&hdo,argc,argv);
	}
	fr = hdp.num_frame;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		if (pflag && (f == 0 || !fflag)) {
			if (hdp.pixel_format != PFBYTE) {
				h_minmax(&hdp,&min,&max,nzflag);
				nbins = 1000;
				if (hdp.pixel_format != PFINT) {
					max.v_float +=
					    (max.v_float - min.v_float)*.00001;
				}
			}
			else {
				min.v_byte = 0;
				max.v_byte = 255;
				nbins = 256;
			}
			alloc_histo(&histo,&min,&max,nbins,hdp.pixel_format);
			h_clearhisto(&histo);
			h_histo(&hdp,&histo,nzflag,&count);
			j = (percent * count) / 100.;
			k = 0;
			for (i = histo.nbins;i>0;i--) {
				k += histo.histo[i];
				if (k>=j) break;
			}
			switch (hdp.pixel_format) {
			case PFBYTE:	thresh.v_byte = i-1;
					sprintf(msg,"frame %d threshold at %d",
						f,i-1);
					if (nzflag && i==1)
						strcat(msg,threshmsg);
					break;
			case PFINT:	thresh.v_int = histo.minbin.v_int +
						(histo.binwidth.v_int * (i-1));
					sprintf(msg,"frame %d threshold at %d",
						f,thresh.v_int);
					if (nzflag && thresh.v_int <= 0)
						strcat(msg,threshmsg);
					break;
			case PFFLOAT:
			case PFCOMPLEX:	thresh.v_float = histo.minbin.v_float +
					    (histo.binwidth.v_float * (i-1));
					sprintf(msg,"frame %d threshold at %f",
						f,thresh.v_float);
					if (nzflag && thresh.v_float <= 0)
						strcat(msg,threshmsg);
					break;
			}
			perr(HE_IMSG,msg);
		}
		if (sflag)
			h_softthresh(&hdp,&hdp,&thresh);
		else
			h_hardthresh(&hdp,&hdp,&thresh);
		if (sflag || Sflag)
			write_imagec(&hd,&hdp,method,hips_convback,f);
		else if (convbyte) {
			if (hdp.pixel_format == PFCOMPLEX) {
				h_toi(&hdp,&hdt);
				h_tob(&hdt,&hdo);
			}
			else
				h_tob(&hdp,&hdo);
			write_image(&hdo,f);
		}
		else
			write_image(&hdp,f);
	}
	return(0);
}
