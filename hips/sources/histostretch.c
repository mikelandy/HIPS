/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * histostretch.c - stretch contrast by ignoring outlier pixel values
 *
 * usage: histostretch [-p lowpercent [highpercent]] [-m newmin [newmax]] [-n]
 *
 * Histostretch will take a portion of the pixel value range of an image and
 * linearly map it to a specified range of pixel values.  The user specifies
 * the input range by giving the percentage of pixels which should fall below
 * the bottom range value and the percentage of pixels which should fall above
 * the high range value.  Once these cutoffs are known, the image will be
 * scaled linearly such that the bottom input range value maps to newmin and
 * the top value to newmax.  The defaults are 1 (1 percent) for both
 * percentages, 0 for newmin and 255 for newmax.  Thus, the default behavior
 * is to set the bottom 1% of pixels to 0, set the top 1% of pixels to 255,
 * and linearly stretch the contrast of the rest to fill the full range from
 * 0 to 255.  If the -n switch is specified, all of the zero value pixels are
 * disregarded in computing the percentage thresholds.
 *
 * Formats: BYTE
 *
 * to load:	cc -o histostretch histostretch.c -lhipsh -lhips -lm
 *
 * Michael Landy - 9/3/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"p",{LASTFLAG},1,{{PTDOUBLE,"1","lowpercent"},
		{PTDOUBLE,"1","highpercent"},LASTPARAMETER}},
	{"m",{LASTFLAG},1,{{PTINT,"0","newmin"},
		{PTINT,"255","newmax"},LASTPARAMETER}},
	{"n",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp;
	int method,fr,f,count,i,j,k,nbins,klow,khigh,newmin,newmax;
	Filename filename;
	FILE *fp;
	h_boolean nzflag;
	double lowpercent,highpercent;
	float lb,lc;
	struct hips_histo histo;
	Pixelval min,max;
	byte lut[256];
	char msg[200];

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&lowpercent,&highpercent,&newmin,&newmax,
		&nzflag,FFONE,&filename);
	if (lowpercent<0. || lowpercent>100. || highpercent<0. ||
		highpercent>100.)
			perr(HE_MSG,"bad percentage specified");
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	write_headeru2(&hd,&hdp,argc,argv,hips_convback);
	fr = hdp.num_frame;
	min.v_byte = 0;
	max.v_byte = 255;
	nbins = 256;
	histo.histodealloc = FALSE;
	alloc_histo(&histo,&min,&max,nbins,hdp.pixel_format);
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_clearhisto(&histo);
		h_histo(&hdp,&histo,nzflag,&count);
		j = (lowpercent * count) / 100.;
		k = 0;
		for (i=1;i<=nbins;i++) {
			k += histo.histo[i];
			if (k>=j) break;
		}
		klow = i;
		j = (highpercent * count) / 100.;
		k = 0;
		for (i=nbins;i>0;i--) {
			k += histo.histo[i];
			if (k>=j) break;
		}
		khigh = i;
		if (klow >= khigh) {
			sprintf(msg,
		"frame %d: zero contrast range, trying to scale entire range",
			f);
			perr(HE_IMSG,msg);
			j = 0;
			k = 0;
			for (i=1;i<=nbins;i++) {
				k += histo.histo[i];
				if (k>=j) break;
			}
			klow = i;
			k = 0;
			for (i=nbins;i>0;i--) {
				k += histo.histo[i];
				if (k>=j) break;
			}
			khigh = i;
			if (klow >= khigh) {
				perr(HE_IMSG,
					"zero contrast image");
				klow = 0; khigh = 1;
			}
		}
		lb = ((float) (newmax-newmin))/(khigh-klow);
		lc = newmin - (lb*klow);
		for (i=0;i<256;i++) {
			j = lb*i + lc + 0.5;
			lut[i] = (j <= 0) ? 0 : ((j >= 255) ? 255 : j);
		}
		h_applylut(&hdp,&hdp,256,lut);
		write_imagec(&hd,&hdp,method,hips_convback,f);
	}
	return(0);
}
