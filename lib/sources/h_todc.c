/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_todc.c - conversions to double complex pixel format
 *
 * Michael Landy - 1/4/91
 */

#include <hipl_format.h>

extern int hips_rtocplx;

int h_todc(hdi,hdo)

struct header *hdi,*hdo;

{
	switch(hdi->pixel_format) {
	case PFINT:	if (h_itodc(hdi,hdo) == HIPS_ERROR)
				return(HIPS_ERROR);
			break;
	case PFFLOAT:	if (h_ftodc(hdi,hdo) == HIPS_ERROR)
				return(HIPS_ERROR);
			break;
	case PFDOUBLE:	if (h_dtodc(hdi,hdo) == HIPS_ERROR)
				return(HIPS_ERROR);
			break;
	case PFCOMPLEX:	h_ctodc(hdi,hdo); break;
	default:	return(perr(HE_FMTSUBR,"h_todc",
				hformatname(hdi->pixel_format)));
	}
	return(HIPS_OK);
}

int h_itodc(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	int *pi;
	double *po;

	hips_lclip = hips_hclip = 0;
	pi = (int *) hdi->image;
	po = (double *) hdo->image;
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
		return(perr(HE_RTOCTP,"h_itodc",hips_rtocplx));
	}
	return(HIPS_OK);
}

int h_ftodc(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	float *pi;
	double *po;

	hips_lclip = hips_hclip = 0;
	pi = (float *) hdi->image;
	po = (double *) hdo->image;
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
		return(perr(HE_RTOCTP,"h_ftodc",hips_rtocplx));
	}
	return(HIPS_OK);
}

int h_dtodc(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	double *pi;
	double *po;

	hips_lclip = hips_hclip = 0;
	pi = (double *) hdi->image;
	po = (double *) hdo->image;
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
		return(perr(HE_RTOCTP,"h_dtodc",hips_rtocplx));
	}
	return(HIPS_OK);
}

int h_ctodc(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	float *pi;
	double *po;

	hips_lclip = hips_hclip = 0;
	pi = (float *) hdi->image;
	po = (double *) hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++) {
		*po++ = *pi++;
		*po++ = *pi++;
	}
	return(HIPS_OK);
}
