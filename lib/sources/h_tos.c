/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_tos.c - conversions to short pixel format
 *
 * Michael Landy - 1/4/91
 */

#include <hipl_format.h>

int h_tos(hdi,hdo)

struct header *hdi,*hdo;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	h_btos(hdi,hdo); break;
	case PFSBYTE:	h_sbtos(hdi,hdo); break;
	case PFINT:	h_itos(hdi,hdo); break;
	case PFFLOAT:	h_ftos(hdi,hdo); break;
	default:	return(perr(HE_FMTSUBR,"h_tos",
				hformatname(hdi->pixel_format)));
	}
	return(HIPS_OK);
}

int h_btos(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	byte *pi;
	short *po;

	hips_lclip = hips_hclip = 0;
	pi = hdi->image;
	po = (short *) hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++)
		*po++ = *pi++;
	return(HIPS_OK);
}

int h_sbtos(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	sbyte *pi;
	short *po;

	hips_lclip = hips_hclip = 0;
	pi = (sbyte *) hdi->image;
	po = (short *) hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++)
		*po++ = *pi++;
	return(HIPS_OK);
}

int h_itos(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	int *pi;
	short *po;

	hips_lclip = hips_hclip = 0;
	pi = (int *) hdi->image;
	po = (short *) hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++)
		*po++ = *pi++;
	return(HIPS_OK);
}

int h_ftos(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	float *pi;
	short *po;

	hips_lclip = hips_hclip = 0;
	pi = (float *) hdi->image;
	po = (short *) hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++) {
		*po++ = (*pi >= 0) ? (*pi + .5) : (*pi - .5);
		pi++;
	}
	return(HIPS_OK);
}
