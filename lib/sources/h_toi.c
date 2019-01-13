/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_toi.c - conversions to int pixel format
 *
 * Michael Landy - 1/4/91
 */

#include <hipl_format.h>
#include <math.h>

extern int hips_cplxtor;
extern byte hips_lchar,hips_hchar;

int h_toi(hdi,hdo)

struct header *hdi,*hdo;

{
	switch(hdi->pixel_format) {
	case PFMSBF:	h_mptoi(hdi,hdo); break;
	case PFLSBF:	h_lptoi(hdi,hdo); break;
	case PFBYTE:	h_btoi(hdi,hdo); break;
	case PFSBYTE:	h_sbtoi(hdi,hdo); break;
	case PFUSHORT:	h_ustoi(hdi,hdo); break;
	case PFSHORT:	h_stoi(hdi,hdo); break;
	case PFUINT:	h_uitoi(hdi,hdo); break;
	case PFFLOATPYR:
	case PFFLOAT:	h_ftoi(hdi,hdo); break;
	case PFDOUBLE:	h_dtoi(hdi,hdo); break;
	case PFCOMPLEX:	if (h_ctoi(hdi,hdo) == HIPS_ERROR)
				return(HIPS_ERROR);
			break;
	case PFDBLCOM:	if (h_dctoi(hdi,hdo) == HIPS_ERROR)
				return(HIPS_ERROR);
			break;
	default:	return(perr(HE_FMTSUBR,"h_toi",
				hformatname(hdi->pixel_format)));
	}
	return(HIPS_OK);
}

int h_mptoi(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,j,nr,nc,bit;
	byte *pi;
	int *po;

	hips_lclip = hips_hclip = 0;
	pi = hdi->image;
	po = (int *) hdo->image;
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

int h_lptoi(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,j,nr,nc,bit;
	byte *pi;
	int *po;

	hips_lclip = hips_hclip = 0;
	pi = hdi->image;
	po = (int *) hdo->image;
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

int h_btoi(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	byte *pi;
	int *po;

	hips_lclip = hips_hclip = 0;
	pi = hdi->image;
	po = (int *) hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++)
		*po++ = *pi++;
	return(HIPS_OK);
}

int h_sbtoi(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	sbyte *pi;
	int *po;

	hips_lclip = hips_hclip = 0;
	pi = (sbyte *) hdi->image;
	po = (int *) hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++)
		*po++ = *pi++;
	return(HIPS_OK);
}

int h_ustoi(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	h_ushort *pi;
	int *po;

	hips_lclip = hips_hclip = 0;
	pi = (h_ushort *) hdi->image;
	po = (int *) hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++)
		*po++ = *pi++;
	return(HIPS_OK);
}

int h_stoi(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	short *pi;
	int *po;

	hips_lclip = hips_hclip = 0;
	pi = (short *) hdi->image;
	po = (int *) hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++)
		*po++ = *pi++;
	return(HIPS_OK);
}

int h_uitoi(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	h_uint *pi;
	int *po;

	hips_lclip = hips_hclip = 0;
	pi = (h_uint *) hdi->image;
	po = (int *) hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++)
		*po++ = *pi++;
	return(HIPS_OK);
}

int h_ftoi(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np,val;
	float *pi;
	int *po;

	hips_lclip = hips_hclip = 0;
	pi = (float *) hdi->image;
	po = (int *) hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++) {
		*po++ = (*pi < 0) ? (*pi -.5) : (*pi +.5);
		pi++;
	}
	return(HIPS_OK);
}

int h_dtoi(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np,val;
	double *pi;
	int *po;

	hips_lclip = hips_hclip = 0;
	pi = (double *) hdi->image;
	po = (int *) hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++) {
		*po++ = (*pi < 0) ? (*pi -.5) : (*pi +.5);
		pi++;
	}
	return(HIPS_OK);
}

int h_ctoi(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	float *pi,ireal,iimag,val;
	int *po;

	hips_lclip = hips_hclip = 0;
	pi = (float *) hdi->image;
	po = (int *) hdo->image;
	np = hdi->numpix;
	switch(hips_cplxtor) {
	case CPLX_MAG:
		for (i=0;i<np;i++) {
			ireal = *pi++;
			iimag = *pi++;
			val = (float) sqrt((double)
				ireal*ireal+iimag*iimag);
			*po++ = (val < 0) ? (val -.5) : (val +.5);
		}
		break;
	case CPLX_REAL:
		for (i=0;i<np;i++) {
			val = *pi++;
			pi++;
			*po++ = (val < 0) ? (val -.5) : (val +.5);
		}
		break;
	case CPLX_IMAG:
		for (i=0;i<np;i++) {
			pi++;
			val = *pi++;
			*po++ = (val < 0) ? (val -.5) : (val +.5);
		}
		break;
	case CPLX_PHASE:
		for (i=0;i<np;i++) {
			ireal = *pi++;
			iimag = *pi++;
			if (ireal == 0. && iimag == 0.)
				*po++ = 0;
			else {
				val = (float) atan2((double) iimag,
							(double) ireal);
				*po++ = (val < 0) ? (val -.5) : (val +.5);
			}
		}
		break;
	default:
		return(perr(HE_CTORTP,"h_ctoi",hips_cplxtor));
	}
	return(HIPS_OK);
}

int h_dctoi(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	double *pi,ireal,iimag,val;
	int *po;

	hips_lclip = hips_hclip = 0;
	pi = (double *) hdi->image;
	po = (int *) hdo->image;
	np = hdi->numpix;
	switch(hips_cplxtor) {
	case CPLX_MAG:
		for (i=0;i<np;i++) {
			ireal = *pi++;
			iimag = *pi++;
			val = (float) sqrt(ireal*ireal+iimag*iimag);
			*po++ = (val < 0) ? (val -.5) : (val +.5);
		}
		break;
	case CPLX_REAL:
		for (i=0;i<np;i++) {
			val = *pi++;
			pi++;
			*po++ = (val < 0) ? (val -.5) : (val +.5);
		}
		break;
	case CPLX_IMAG:
		for (i=0;i<np;i++) {
			pi++;
			val = *pi++;
			*po++ = (val < 0) ? (val -.5) : (val +.5);
		}
		break;
	case CPLX_PHASE:
		for (i=0;i<np;i++) {
			ireal = *pi++;
			iimag = *pi++;
			if (ireal == 0. && iimag == 0.)
				*po++ = 0;
			else {
				val = (float) atan2(iimag,ireal);
				*po++ = (val < 0) ? (val -.5) : (val +.5);
			}
		}
		break;
	default:
		return(perr(HE_CTORTP,"h_dctoi",hips_cplxtor));
	}
	return(HIPS_OK);
}
