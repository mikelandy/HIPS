/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * powerpix.c - raise to a power
 *
 * usage: powerpix [-p power] < frame > new_frame
 *
 * default power:  .5
 *
 * For byte images, pixels are renormalized to lie between 0 and 255.  For
 * short, integer and float images, the output is a float image and no
 * renormalization is performed.
 *
 * to load: cc -o powerpix powerpix.c -lhipsh -lhips -lm
 *
 * Yoav Cohen 2/16/82
 * added int/float - Mike Landy - 3/16/89
 *
 * modified to use look-up table for byte and short images:
 *     Brian Tierney, LBL 10/90
 * Hips 2 - msl - 1/10/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"p",{LASTFLAG},1,{{PTDOUBLE,".5","power"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,PFSHORT,PFINT,PFFLOAT,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp,hdo;
	int method,fr,f;
	double power;
	Filename filename;
	FILE *fp;
	h_boolean imagecopy;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&power,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	imagecopy = FALSE;
	if (hdp.pixel_format == PFINT || hdp.pixel_format == PFSHORT) {
		if (hdp.rows != hdp.orows || hdp.cols != hdp.ocols)
			imagecopy = TRUE;
		dup_headern(&hdp,&hdo);
		setformat(&hdo,PFFLOAT);
		alloc_image(&hdo);
	}
	else	/* PFBYTE or PFFLOAT */
		dup_header(&hdp,&hdo);
	write_headeru(&hdo,argc,argv);
	fr = hdp.num_frame;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		if (imagecopy)
			h_tof(&hdp,&hdo);
		h_power(&hdp,&hdo,power);
		write_image(&hdo,f);
	}
	return(0);
}
