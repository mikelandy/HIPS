/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_histoeq.c - subroutines to compute a mapping for histogram equalization
 *
 * Michael Landy; Lin, Shou-Tsung  - 12/4/85
 * HIPS 2 - msl - 8/6/91
 */

#include <hipl_format.h>
#include <math.h>

int h_histoeq(histogram,count,map)

struct hips_histo *histogram;
int count;
byte *map;

{
	switch(histogram->pixel_format) {
	case PFBYTE:	return(h_histoeq_b(histogram,count,map));
	default:	return(perr(HE_FMTSUBR,"h_histoeq",
				hformatname(histogram->pixel_format)));
	}
}

int h_histoeq_b(histogram,count,map)

struct hips_histo *histogram;
int count;
byte *map;

{
	return(h_histoeq_B(histogram->nbins,histogram->histo,count,map));
}

int h_histoeq_B(nbins,histo,count,map)

byte *map;
int nbins,*histo,count;

{
	int z,r,incr;
	float havg,hint;

	havg = (float) count/nbins;

	r = 0;
	hint = 0;
	for (z=0;z<nbins;z++) {
		hint += histo[z+1];
		incr= (int) hint/havg;
		map[z] = r + (incr/2);
		r += incr;
		hint -= incr*havg;
	};
	return(HIPS_OK);
}
