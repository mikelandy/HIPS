/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * halftone - halftones using the Floyd-Steinberg algorithm
 *
 * usage:	halftone [-l lower] [-u upper] [-r] [-p a b c d] [-P M | L]
 *
 * Halftone converts an 8-bit sequence to a bi-level sequence using the
 * Floyd-Steinberg error diffusion algorithm.  The input sequence is converted
 * if necessary to byte format, and the output halftone is bit packed.
 * The values below `lower' are treated as black, and above
 * `upper' are treated as white.  The effective range is thus from `lower'
 * to `upper', which defaults to the entire range from 0 to 255.  The
 * algorithm is based on diffusing the error to adjacent pixels created when a
 * pixel is changed to 0 or 255.  The algorithm proceeds across each row
 * from left to right, one row at a time.  The error is parceled out to four
 * neighbors: `right', `below-left', `below', and `below-right', using
 * relative weights of the error a, b, c, and d, respectively (which default
 * to 7, 3, 5, and 1).  The weights may be specified (using the -p switch).
 * They are non-negative integers which must sum to 16.  Finally, the process
 * may be randomly dithered (switch -r), which uses a random threshold for
 * each pixel chosen uniformly across the pixel range.  The output is
 * bit-packed, and the user may specify which form of bit-packing using -M or
 * -L (the default is installation-dependent on MSBFVERSION).
 *
 * to load:	cc -o halftone halftone.c -lhipsh -lhips -lm
 *
 * Mike Landy - 7/28/87 (based on code by Lou Salkind and Jim Bergen)
 * HIPS 2 - msl - 1/8/91
 */

#include <stdio.h>
#include <hipl_format.h>

int types[] = {PFBYTE,LASTTYPE};
static Flag_Format flagfmt[] = {
    {"l",{LASTFLAG},1,{{PTINT,"0","lower"},LASTPARAMETER}},
    {"u",{LASTFLAG},1,{{PTINT,"255","upper"},LASTPARAMETER}},
    {"r",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
    {"p",{LASTFLAG},4,{{PTINT,"7","a"},{PTINT,"3","b"},{PTINT,"5","c"},
	{PTINT,"1","d"},LASTPARAMETER}},
    LASTFLAG};
char msg[] = "the -p parameters must be non-negative and sum to 16";

#define	RANDOMVALS	1201

int main(argc,argv)

int argc;
char **argv;

{
	int f,fr,method,lower,upper,alpha,beta,gamma,delta;
	h_boolean rflag;
	struct header hd,hdp,hdo;
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&lower,&upper,&rflag,&alpha,&beta,&gamma,
		&delta,FFONE,&filename);
	fp = hfopenr(filename);
	if (alpha<0 || beta<0 || gamma<0 || delta<0 ||
	    (alpha+beta+gamma+delta != 16))
		perr(HE_MSG,msg);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	dup_headern(&hdp,&hdo);
	setformat(&hdo,(hips_packing == MSBF_PACKING)?PFMSBF:PFLSBF);
	alloc_image(&hdo);
	write_headeru(&hdo,argc,argv);
	fr = hdp.num_frame;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_halftone2(&hdp,&hdp,lower,upper,rflag,alpha,beta,gamma,delta);
		if (hips_packing == MSBF_PACKING)
			h_btomp(&hdp,&hdo);
		else
			h_btolp(&hdp,&hdo);
		write_image(&hdo,f);
	}
	return(0);
}
