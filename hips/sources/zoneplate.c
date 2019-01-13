/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * zoneplate - make a sinusoidal zoneplate image
 *
 * usage:	zoneplate [-s rows [cols]] [-f frequency] [-S]
 *
 * Zoneplate computes an image which is useful in testing image processing
 * algorithms (especially filters).  The image consists of a series of
 * concentric sinusoidal modulations where the local spatial frequency
 * increases linearly from the center of the image to the edges.  The center
 * of the image may be in either sin phase (-S) or cosine phase (the
 * default).  The size of the image may be specified with -s (default is 512
 * by 512, and the number of columns defaults to the number of rows).  The
 * local spatial frequency at the edges of the image may be specified in
 * cycles per image width (default is the maximum Nyquist
 * frequency in the image).  The resulting image is in floating point format.
 *
 * to load:	cc -o zoneplate zoneplate.c -lhipsh -lhips
 *
 * Michael Landy 4/8/89
 * HIPS 2 - msl - 7/5/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
    {"s",{LASTFLAG},1,{{PTINT,"512","rows"},{PTINT,"-1","cols"},LASTPARAMETER}},
    {"f",{LASTFLAG},1,{{PTDOUBLE,"-1","frequency"},LASTPARAMETER}},
    {"S",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
    LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	int rows,cols;
	double freq;
	h_boolean Sflag;
	struct header hd;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&rows,&cols,&freq,&Sflag,FFNONE);
	if (cols == -1)
		cols = rows;
	if (freq == -1)
		freq = ((rows > cols) ? cols : rows)/2.;
	init_header(&hd,"","",1,"",rows,cols,PFFLOAT,1,"");
	write_headeru(&hd,argc,argv);
	alloc_image(&hd);
	h_zoneplate(&hd,freq,Sflag);
	write_image(&hd);
	return(0);
}
