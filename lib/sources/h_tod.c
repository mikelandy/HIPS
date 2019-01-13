/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_tod.c - conversions to double pixel format
 *
 * Michael Landy - 1/4/91
 */

#include <hipl_format.h>
#include <math.h>

extern int hips_cplxtor;

int h_tod(hdi,hdo)

struct header *hdi,*hdo;

{
	switch(hdi->pixel_format) {
	case PFINT:	h_itod(hdi,hdo); break;
	case PFFLOAT:	h_ftod(hdi,hdo); break;
	case PFCOMPLEX:	if (h_ctod(hdi,hdo) == HIPS_ERROR)
				return(HIPS_ERROR);
			break;
	case PFDBLCOM:	if (h_dctod(hdi,hdo) == HIPS_ERROR)
				return(HIPS_ERROR);
			break;
	default:	return(perr(HE_FMTSUBR,"h_tod",
				hformatname(hdi->pixel_format)));
	}
	return(HIPS_OK);
}

int h_itod(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	int *pi;
	double *po;

	hips_lclip = hips_hclip = 0;
	pi = (int *) hdi->image;
	po = (double *) hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++)
		*po++ = *pi++;
	return(HIPS_OK);
}
 
int h_ftod(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	float *pi;
	double *po;

	hips_lclip = hips_hclip = 0;
	pi = (float *) hdi->image;
	po = (double *) hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++)
		*po++ = *pi++;
	return(HIPS_OK);
}
 
int h_ctod(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	float *pi,ireal,iimag;
	double *po;

	hips_lclip = hips_hclip = 0;
	pi = (float *) hdi->image;
	po = (double *) hdo->image;
	np = hdi->numpix;
	switch(hips_cplxtor) {
	case CPLX_MAG:
		for (i=0;i<np;i++) {
			ireal = *pi++;
			iimag = *pi++;
			*po++ = sqrt((double)
				ireal*ireal+iimag*iimag);
		}
		break;
	case CPLX_REAL:
		for (i=0;i<np;i++) {
			*po++ = *pi++;
			pi++;
		}
		break;
	case CPLX_IMAG:
		for (i=0;i<np;i++) {
			pi++;
			*po++ = *pi++;
		}
		break;
	case CPLX_PHASE:
		for (i=0;i<np;i++) {
			ireal = *pi++;
			iimag = *pi++;
			if (ireal == 0. && iimag == 0.)
				*po++ = 0.;
			else
				*po++ = atan2((double) iimag,(double) ireal);
		}
		break;
	default:
		return(perr(HE_CTORTP,"h_ctod",hips_cplxtor));
	}
	return(HIPS_OK);
}
 
int h_dctod(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	double *pi,ireal,iimag;
	double *po;

	hips_lclip = hips_hclip = 0;
	pi = (double *) hdi->image;
	po = (double *) hdo->image;
	np = hdi->numpix;
	switch(hips_cplxtor) {
	case CPLX_MAG:
		for (i=0;i<np;i++) {
			ireal = *pi++;
			iimag = *pi++;
			*po++ = sqrt(ireal*ireal+iimag*iimag);
		}
		break;
	case CPLX_REAL:
		for (i=0;i<np;i++) {
			*po++ = *pi++;
			pi++;
		}
		break;
	case CPLX_IMAG:
		for (i=0;i<np;i++) {
			pi++;
			*po++ = *pi++;
		}
		break;
	case CPLX_PHASE:
		for (i=0;i<np;i++) {
			ireal = *pi++;
			iimag = *pi++;
			if (ireal == 0. && iimag == 0.)
				*po++ = 0.;
			else
				*po++ = atan2(iimag,ireal);
		}
		break;
	default:
		return(perr(HE_CTORTP,"h_dctod",hips_cplxtor));
	}
	return(HIPS_OK);
}
