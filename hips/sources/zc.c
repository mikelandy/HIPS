/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * zc.c - find zero-crossings in a convolved sequence
 *
 * usage:	zc [-e error] [-p percent | -s] [-n] [-z] <iseq >oseq
 *
 * The -e option sets the value of the Laplacian which is considered to be
 * zero to "error". (A Laplacian of absolute value less than or equal to "error"
 * will be coded as zero.)  The "error" definition defaults to zero.  The -s
 * option outputs a floating point image which gives the "slope" at each zero
 * crossing.  This is not an actual slope, but a crude approximation.  If the -p
 * option is specified, the best "percent" pixels will be coded as
 * zero-crossings.  "Percent" defaults to 100%.  The program generally places
 * zero crossings where the image
 * is actually zero and abuts pixels of opposite signs, or at a
 * positive pixel which abuts a negative pixel.  The -n option places the zero
 * crossings at actual zeroes and at negative pixels which abut positive ones.
 * Lastly, if an actual zero (as defined by "error") abuts pixels of one sign
 * but not the other (as occurs at the edges of a broad area of zeroes), no
 * zero crossing is marked.  Such a pixel will be marked if -z is specified.
 * Note that zero crossings are marked as with hips_hchar (set with -UH) on
 * a background of hips_lchar (set with -UL).  With -s, the -CB switch is
 * effective.
 *
 * Input Formats: FLOAT
 * Output Formats: BYTE (for -p), FLOAT (for -s)
 *
 * to load:	cc -o zc zc.c -lhipsh -lhips -lm
 *
 * Yoav Cohen 7/6/82
 * YC modified 7/28/82, for use with the -p option.
 * YC modified 1/17/83, for INT input.
 * totally rewritten/extended - Mike Landy 8/14/84
 * HIPS 2 - msl - 6/21/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"e",{LASTFLAG},1,{{PTDOUBLE,"0","error"},LASTPARAMETER}},
	{"p",{"s",LASTFLAG},1,{{PTBOOLEAN,"TRUE"},{PTDOUBLE,"100","percent"},
		LASTPARAMETER}},
	{"s",{"p",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"n",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"z",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFFLOAT,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp,hdp2,hdo;
	int method,fr,f,count,i,j,k,nbins;
	Filename filename;
	FILE *fp;
	h_boolean pflag,sflag,nflag,zflag,imagecopy;
	double error,percent;
	Pixelval min,max,thresh;
	struct hips_histo histo;
	struct hips_roi roi;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&error,&pflag,&percent,&sflag,&nflag,
		&zflag,FFONE,&filename);
	if (error<0.)
		perr(HE_MSG,"error must be greater than 0");
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	dup_headern(&hdp,&hdp2);
	alloc_image(&hdp2);
	histo.histodealloc = FALSE;
	imagecopy = (hdp.rows != hdp.orows || hdp.cols != hdp.ocols) ?
		TRUE : FALSE;
	if (sflag)
		write_headeru2(&hd,&hdp,argc,argv,hips_convback);
	else {
		dup_headern(&hdp,&hdo);
		setformat(&hdo,PFBYTE);
		alloc_image(&hdo);
		write_headeru(&hdo,argc,argv);
	}
	fr = hdp.num_frame;
	getroi(&hdp,&roi);
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		if (imagecopy) {
			clearroi(&hdp);
			clearroi(&hdp2);
			h_copy(&hdp,&hdp2);
			setroi2(&hdp,&roi);
			setroi2(&hdp2,&roi);
		}
		h_zc(&hdp,&hdp2,error,nflag,zflag);
		if (sflag)
			write_imagec(&hd,&hdp2,method,hips_convback,f);
		else {
			h_minmax(&hdp2,&min,&max,TRUE);
			if (percent == 100.)
				thresh.v_float = min.v_float;
			else {
				nbins = 1000;
				max.v_float +=
					(max.v_float - min.v_float)*.00001;
				alloc_histo(&histo,&min,&max,nbins,
					hdp2.pixel_format);
				h_clearhisto(&histo);
				h_histo(&hdp2,&histo,TRUE,&count);
				j = (percent * count) / 100.;
				k = 0;
				for (i = histo.nbins;i>0;i--) {
					k += histo.histo[i];
					if (k>=j) break;
				}
				thresh.v_float = histo.minbin.v_float +
				    (histo.binwidth.v_float * (i-1));
			}
			h_hardthresh(&hdp2,&hdp2,&thresh);
			h_tob(&hdp2,&hdo);
			write_image(&hdo,f);
		}
	}
	return(0);
}
