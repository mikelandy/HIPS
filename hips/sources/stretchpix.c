/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * stretchpix.c - stretch or compress the range of gray-levels
 *
 * usage: stretchpix [-p expt1 [ boundary [ expt2 ]]] [-m max | -f] < in > out
 *
 * Stretches the greyscale using two different power functions.  The parameter
 * boundary is used to split the range of pixel values into two parts:
 *
 *	from 0 to max*bdry, and from max*bdry to max
 *
 * Then, pixels in the first range have a power function applied with exponent
 * expt1, and are then rescaled so that the boundary value is unchanged.
 * Similarly, pixels in the second range have a power function applied with
 * exponent expt2, and are then linearly rescaled so that the boundary and
 * maximum values are unchanged.  Values less than zero are set to zero.
 *
 * The maximum value may either be specified (-m) or computed as the maximum
 * pixel value in the input frame (recomputed for each frame, -f).  For byte
 * images the default is `-m 255'.  For short images the default is -f.
 * Other defaults:  expt1:  2., boundary: .5, expt2: 1/expt1.
 *
 * to load: cc -o stretchpix stretchpix.c -lhipsh -lhips -lm
 *
 * Yoav Cohen 2/19/82
 * HIPS-2 - msl - 6/15/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"p",{LASTFLAG},1,{{PTDOUBLE,"2.","expt1"},{PTDOUBLE,".5","boundary"},
		{PTDOUBLE,"-999.","expt2"},LASTPARAMETER}},
	{"m",{"f",LASTFLAG},1,{{PTBOOLEAN,"TRUE"},{PTINT,"-999","maximum"},
		LASTPARAMETER}},
	{"f",{"m",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,PFSHORT,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp;
	int method,fr,f;
	double expt1,bdry,expt2;
	Pixelval mval;
	int max;
	h_boolean mflag,fflag;
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&expt1,&bdry,&expt2,&mflag,&max,&fflag
		,FFONE,&filename);
	if (expt2 == -999.)
		expt2 = 1./expt1;
	if (bdry <= 0 || bdry >= 1)
		perr(HE_MSG,"boundary must be between 0 and 1");
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	if (mflag && max == -999) {
		if (hdp.pixel_format == PFBYTE)
			mval.v_byte = 255;
		else {
			mflag = FALSE;
			fflag = TRUE;
		}
	}
	else if (mflag) {
		if (hdp.pixel_format == PFBYTE)
			mval.v_byte = max;
		else
			mval.v_short = max;
	}
	write_headeru2(&hd,&hdp,argc,argv,hips_convback);
	fr = hdp.num_frame;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		if (fflag)
			h_max(&hdp,&mval,FALSE);
		h_stretch(&hdp,&hdp,bdry,expt1,expt2,&mval);
		write_imagec(&hd,&hdp,method,hips_convback,f);
	}
	return(0);
}
