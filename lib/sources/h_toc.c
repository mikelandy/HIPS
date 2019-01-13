/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_toc.c - conversions to complex pixel format
 *
 * Michael Landy - 1/4/91
 */

#include <hipl_format.h>

extern int hips_rtocplx;

int h_toc(hdi,hdo)

struct header *hdi,*hdo;

{
	switch(hdi->pixel_format) {
	case PFINT:	if (h_itoc(hdi,hdo) == HIPS_ERROR)
				return(HIPS_ERROR);
			break;
	case PFFLOAT:	if (h_ftoc(hdi,hdo) == HIPS_ERROR)
				return(HIPS_ERROR);
			break;
	case PFDOUBLE:	if (h_dtoc(hdi,hdo) == HIPS_ERROR)
				return(HIPS_ERROR);
			break;
	case PFDBLCOM:	h_dctoc(hdi,hdo); break;
	default:	return(perr(HE_FMTSUBR,"h_toc",
				hformatname(hdi->pixel_format)));
	}
	return(HIPS_OK);
}

int h_itoc(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	int *pi;
	float *po;

	hips_lclip = hips_hclip = 0;
	pi = (int *) hdi->image;
	po = (float *) hdo->image;
	np = hdi->numpix;
	switch(hips_rtocplx) {
	case CPLX_RVI0:
		for (i=0;i<np;i++) {
			*po++ = *pi++;
			*po++ = 0;
		}
		break;
	case CPLX_R0IV:
		for (i=0;i<np;i++) {
			*po++ = 0;
			*po++ = *pi++;
		}
		break;
	case CPLX_RVIV:
		for (i=0;i<np;i++) {
			*po++ = *pi;
			*po++ = *pi++;
		}
		break;
	default:
		return(perr(HE_RTOCTP,"h_itoc",hips_rtocplx));
	}
	return(HIPS_OK);
}

int h_ftoc(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	float *pi;
	float *po;

	hips_lclip = hips_hclip = 0;
	pi = (float *) hdi->image;
	po = (float *) hdo->image;
	np = hdi->numpix;
	switch(hips_rtocplx) {
	case CPLX_RVI0:
		for (i=0;i<np;i++) {
			*po++ = *pi++;
			*po++ = 0;
		}
		break;
	case CPLX_R0IV:
		for (i=0;i<np;i++) {
			*po++ = 0;
			*po++ = *pi++;
		}
		break;
	case CPLX_RVIV:
		for (i=0;i<np;i++) {
			*po++ = *pi;
			*po++ = *pi++;
		}
		break;
	default:
		return(perr(HE_RTOCTP,"h_ftoc",hips_rtocplx));
	}
	return(HIPS_OK);
}

int h_dtoc(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	double *pi;
	float *po;

	hips_lclip = hips_hclip = 0;
	pi = (double *) hdi->image;
	po = (float *) hdo->image;
	np = hdi->numpix;
	switch(hips_rtocplx) {
	case CPLX_RVI0:
		for (i=0;i<np;i++) {
				*po++ = *pi++;
				*po++ = 0;
		}
		break;
	case CPLX_R0IV:
		for (i=0;i<np;i++) {
			*po++ = 0;
			*po++ = *pi++;
		}
		break;
	case CPLX_RVIV:
		for (i=0;i<np;i++) {
			*po++ = *pi;
			*po++ = *pi++;
		}
		break;
	default:
		return(perr(HE_RTOCTP,"h_dtoc",hips_rtocplx));
	}
	return(HIPS_OK);
}

int h_dctoc(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	double *pi;
	float *po;

	hips_lclip = hips_hclip = 0;
	pi = (double *) hdi->image;
	po = (float *) hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++) {
		*po++ = *pi++;
		*po++ = *pi++;
	}
	return(HIPS_OK);
}
