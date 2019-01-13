/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * strobe - average groups of successive frames
 *
 * usage:	strobe [-b batch-length]
 *
 * Strobe averages batches of frames pixel-by-corresponding-pixel, for
 * subsequences of length "batch-length".  Batch-length defaults to the
 * length of the sequence, resulting in a single frame output which is the
 * average of all input frames.  The number of frames output is the number of
 * frames input divided by the batch-length, with excess frames discarded.
 *
 * pixel formats handled directly: BYTE, SHORT, INT, FLOAT
 *
 * to load:	cc -o strobe strobe.c -lhipsh -lhips
 *
 * Michael Landy - 8/1/84
 * Hips 2 - msl - 7/7/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"b",{LASTFLAG},1,{{PTINT,"-1","batch-length"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,PFSHORT,PFINT,PFFLOAT,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp,hdsum;
	int method,fr,f,batch,i;
	Filename filename;
	FILE *fp;
	Pixelval scale;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&batch,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	if (batch < 0)
		batch = hd.num_frame;
	if (batch < 2)
		perr(HE_MSG,"batch length must be at least two");
	fr = hd.num_frame = hd.num_frame/batch;
	if (fr <= 0)
	    perr(HE_MSG,"number of frames must be at least the batch length");
	method = fset_conversion(&hd,&hdp,types,filename);
	if (hdp.numcolor != 1)
		perr(HE_MSG,"Number of color planes must be 1, use subseq");
	if (hgetdepth(&hdp) != 1)
		perr(HE_MSG,"Number of depth planes must be 1, use subseq");
	switch(hdp.pixel_format) {
	case PFBYTE:	scale.v_int = batch; break;
	case PFSHORT:	scale.v_short = batch; break;
	case PFINT:	scale.v_int = batch; break;
	case PFFLOAT:	scale.v_float = batch; break;
	}
	dup_headern(&hdp,&hdsum);
	if (hdp.pixel_format == PFBYTE)
		setformat(&hdsum,PFINT);
	alloc_image(&hdsum);
	write_headeru2(&hd,&hdp,argc,argv,hips_convback);
	clearroi(&hdp);
	clearroi(&hdsum);
	for (f=0;f<fr;f++) {
		if (hdp.pixel_format == PFBYTE) {
			fread_imagec(fp,&hd,&hdp,method,f,filename);
			h_toi(&hdp,&hdsum);
		}
		else
			fread_imagec(fp,&hd,&hdsum,method,f,filename);
		for (i=1;i<batch;i++) {
			fread_imagec(fp,&hd,&hdp,method,f,filename);
			h_add(&hdp,&hdsum,&hdsum);
		}
		h_divscale(&hdsum,&hdp,&scale);
		write_imagec(&hd,&hdp,method,hips_convback,f);
	}
	return(0);
}
