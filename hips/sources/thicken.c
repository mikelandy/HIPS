/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * thicken.c - thicken a white-on-black image
 *
 * usage:	thicken
 *
 * A pixel is set to hips_hchar if either it, its lower, right, or lower-right
 * neighbors is nonzero.  For LLORIG, it is set if it, its upper, right or
 * upper-right neighbors is nonzero.  If not set, it is cleared to
 * hips_lchar.
 *
 * to load:	cc -o thicken thicken.c -lhipsh -lhips -lm
 *
 * Mike Landy - 12/20/82
 * HIPS 2 - msl - 8/4/91
 */

#include <stdio.h>
#include <hipl_format.h>

int types[] = {PFBYTE,LASTTYPE};
static Flag_Format flagfmt[] = {LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	int f,fr,method;
	struct header hd,hdp;
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	write_headeru2(&hd,&hdp,argc,argv,hips_convback);
	fr = hdp.num_frame;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_thicken(&hdp,&hdp);
		write_imagec(&hd,&hdp,method,hips_convback,f);
	}
	return(0);
}
