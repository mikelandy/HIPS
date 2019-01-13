/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * abdou.c - Abdou's edge fitting procedure
 *
 * usage:	abdou [-s size] <iseq >oseq
 *
 * Where size is the length of a side of the nonoverlapping domains in which the
 * algorithm operates.  The program is an implementation of
 * the edge fitting algorithm described in I. E. Abdou's doctoral thesis,
 * "Quantitative Methods of Edge Detection", published by the USC Image
 * Processing Institute as USCIPI Report 830.
 * The output gives either zero, or the signal-to-noise ratio
 * for each edge pixel.  All computations are done with pixels scaled by
 * size*size in order for integer calculations to be exact.  This yields the
 * same signal-to-noise ratio as nonscaled pixels would.
 * Size defaults to 7.
 *
 * to load:	cc -o abdou abdou.c -lhipsh -lhips -lm
 *
 * Mike Landy 7/12/82
 * HIPS 2 - msl - 8/7/91
 */

#include <stdio.h>
#include <hipl_format.h>

int types[] = {PFBYTE,LASTTYPE};
static Flag_Format flagfmt[] = {
    {"s",{LASTFLAG},1,{{PTINT,"7","size"},LASTPARAMETER}},
    LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	int f,fr,method,size;
	struct header hd,hdp,hdo;
	Filename filename;
	FILE *fp;
	h_boolean imagecopy;
	struct hips_roi roi;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&size,FFONE,&filename);
	fp = hfopenr(filename);
	if (size < 3 || size > 15 || size%2 == 0)
		perr(HE_MSG,"size must be an odd number between 3 and 15");
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	dup_headern(&hdp,&hdo);
	setformat(&hdo,PFFLOAT);
	alloc_image(&hdo);
	write_headeru(&hdo,argc,argv);
	fr = hdp.num_frame;
	imagecopy = (hdp.rows != hdp.orows || hdp.cols != hdp.ocols) ?
		TRUE : FALSE;
	getroi(&hdp,&roi);
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		if (imagecopy) {
			clearroi(&hdp);
			clearroi(&hdo);
			h_tof(&hdp,&hdo);
			setroi2(&hdp,&roi);
			setroi2(&hdo,&roi);
		}
		h_abdou(&hdp,&hdo,size);
		write_image(&hdo,f);
	}
	return(0);
}
