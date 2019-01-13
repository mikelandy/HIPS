/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_tob.c - conversions to byte pixel format
 *
 * Michael Landy - 1/4/91
 */

#include <hipl_format.h>

extern byte hips_lchar,hips_hchar;

int h_tob(hdi,hdo)

struct header *hdi,*hdo;

{
	switch(hdi->pixel_format) {
	case PFMSBF:	h_mptob(hdi,hdo); break;
	case PFLSBF:	h_lptob(hdi,hdo); break;
	case PFSHORT:	h_stob(hdi,hdo); break;
	case PFINT:	h_itob(hdi,hdo); break;
	case PFFLOAT:	h_ftob(hdi,hdo); break;
	default:	return(perr(HE_FMTSUBR,"h_tob",
				hformatname(hdi->pixel_format)));
	}
	return(HIPS_OK);
}

int h_mptob(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,j,nr,nc,bit;
	byte *pi;
	byte *po;

	hips_lclip = hips_hclip = 0;
	pi = hdi->image;
	po = hdo->image;
	nr = hdi->orows;
	nc = hdi->ocols;
	for (i=0;i<nr;i++) {
		bit = 0;
		for (j=0;j<nc;j++) {
			*po++ = ((*pi & (0200>>bit))!=0)
				? hips_hchar : hips_lchar;
			if (++bit == 8) {
				bit = 0;
				pi++;
			}
		}
		if (bit != 0)
			pi++;
	}
	return(HIPS_OK);
}

int h_lptob(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,j,nr,nc,bit;
	byte *pi;
	byte *po;

	hips_lclip = hips_hclip = 0;
	pi = hdi->image;
	po = hdo->image;
	nr = hdi->orows;
	nc = hdi->ocols;
	for (i=0;i<nr;i++) {
		bit = 0;
		for (j=0;j<nc;j++) {
			*po++ = ((*pi & (01<<bit))!=0)
				? hips_hchar : hips_lchar;
			if (++bit == 8) {
				bit = 0;
				pi++;
			}
		}
		if (bit != 0)
			pi++;
	}
	return(HIPS_OK);
}

int h_stob(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	short *pi;
	byte *po;

	hips_lclip = hips_hclip = 0;
	pi = (short *) hdi->image;
	po = hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++) {
		if (*pi < 0) {
			*po++ = 0;
			hips_lclip++;
		}
		else if (*pi > 255) {
			*po++ = 255;
			hips_hclip++;
		}
		else
			*po++ = *pi;
		pi++;
	}
	return(HIPS_OK);
}

int h_itob(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	int *pi;
	byte *po;

	hips_lclip = hips_hclip = 0;
	pi = (int *) hdi->image;
	po = hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++) {
		if (*pi < 0) {
			hips_lclip++;
			*po++ = 0;
		}
		else if (*pi > 255) {
			hips_hclip++;
			*po++ = 255;
		}
		else
			*po++ = *pi;
		pi++;
	}
	return(HIPS_OK);
}

int h_ftob(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np,val;
	float *pi;
	byte *po;

	hips_lclip = hips_hclip = 0;
	pi = (float *) hdi->image;
	po = hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++) {
		val = *pi++ + .5;
		if (val < 0) {
			hips_lclip++;
			*po++ = 0;
		}
		else if (val > 255) {
			hips_hclip++;
			*po++ = 255;
		}
		else
			*po++ = val;
	}
	return(HIPS_OK);
}
