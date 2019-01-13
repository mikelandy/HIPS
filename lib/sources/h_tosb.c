/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_tosb.c - conversions to signed byte pixel format
 *
 * Michael Landy - 1/4/91
 */

#include <hipl_format.h>

int h_tosb(hdi,hdo)

struct header *hdi,*hdo;

{
	switch(hdi->pixel_format) {
	case PFSHORT:	h_stosb(hdi,hdo); break;
	case PFINT:	h_itosb(hdi,hdo); break;
	default:	return(perr(HE_FMTSUBR,"h_tosb",
				hformatname(hdi->pixel_format)));
	}
	return(HIPS_OK);
}

int h_stosb(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	short *pi;
	sbyte *po;

	hips_lclip = hips_hclip = 0;
	pi = (short *) hdi->image;
	po = (sbyte *) hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++) {
		if (*pi > 127) {
			hips_hclip++;
			*po++ = 127;
		}
		else if (*pi < -128) {
			hips_lclip++;
			*po++ = -128;
		}
		else
			*po++ = *pi;
		pi++;
	}
	return(HIPS_OK);
}

int h_itosb(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	int *pi;
	sbyte *po;

	hips_lclip = hips_hclip = 0;
	pi = (int *) hdi->image;
	po = (sbyte *) hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++) {
		if (*pi > 127) {
			hips_hclip++;
			*po++ = 127;
		}
		else if (*pi < -128) {
			hips_lclip++;
			*po++ = -128;
		}
		else
			*po++ = *pi;
		pi++;
	}
	return(HIPS_OK);
}
