/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_tof.c - conversions to float pixel format
 *
 * Michael Landy - 1/4/91
 */

#include <hipl_format.h>
#include <math.h>

extern int hips_cplxtor;

int h_tof(hdi,hdo)

struct header *hdi,*hdo;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	h_btof(hdi,hdo); break;
	case PFSHORT:	h_stof(hdi,hdo); break;
	case PFINTPYR:
	case PFINT:	h_itof(hdi,hdo); break;
	case PFDOUBLE:	h_dtof(hdi,hdo); break;
	case PFCOMPLEX:	if (h_ctof(hdi,hdo) == HIPS_ERROR)
				return(HIPS_ERROR);
			break;
	case PFDBLCOM:	if (h_dctof(hdi,hdo) == HIPS_ERROR)
				return(HIPS_ERROR);
			break;
	default:	return(perr(HE_FMTSUBR,"h_tof",
				hformatname(hdi->pixel_format)));
	}
	return(HIPS_OK);
}

int h_btof(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	byte *pi;
	float *po;

	hips_lclip = hips_hclip = 0;
	pi = hdi->image;
	po = (float *) hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++)
		*po++ = *pi++;
	return(HIPS_OK);
}

int h_stof(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	short *pi;
	float *po;

	hips_lclip = hips_hclip = 0;
	pi = (short *) hdi->image;
	po = (float *) hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++)
		*po++ = *pi++;
	return(HIPS_OK);
}

int h_itof(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	int *pi;
	float *po;

	hips_lclip = hips_hclip = 0;
	pi = (int *) hdi->image;
	po = (float *) hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++)
		*po++ = *pi++;
	return(HIPS_OK);
}
 
int h_dtof(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	double *pi;
	float *po;

	hips_lclip = hips_hclip = 0;
	pi = (double *) hdi->image;
	po = (float *) hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++)
		*po++ = *pi++;
	return(HIPS_OK);
}
 
int h_ctof(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	float *pi,ireal,iimag;
	float *po;

	hips_lclip = hips_hclip = 0;
	pi = (float *) hdi->image;
	po = (float *) hdo->image;
	np = hdi->numpix;
	switch(hips_cplxtor) {
	case CPLX_MAG:
		for (i=0;i<np;i++) {
			ireal = *pi++;
			iimag = *pi++;
			*po++ = (float) sqrt((double)
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
				*po++ = (float) atan2((double) iimag,
							(double) ireal);
		}
		break;
	default:
		return(perr(HE_CTORTP,"h_ctof",hips_cplxtor));
	}
	return(HIPS_OK);
}
 
int h_dctof(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	double *pi,ireal,iimag;
	float *po;

	hips_lclip = hips_hclip = 0;
	pi = (double *) hdi->image;
	po = (float *) hdo->image;
	np = hdi->numpix;
	switch(hips_cplxtor) {
	case CPLX_MAG:
		for (i=0;i<np;i++) {
			ireal = *pi++;
			iimag = *pi++;
			*po++ = (float) sqrt(ireal*ireal+iimag*iimag);
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
				*po++ = (float) atan2(iimag,ireal);
		}
		break;
	default:
		return(perr(HE_CTORTP,"h_dctof",hips_cplxtor));
	}
	return(HIPS_OK);
}
