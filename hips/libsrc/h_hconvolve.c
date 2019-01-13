/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_hconvolve.c - 1D horizontal convolution
 *
 * (***important note***, h_hconvolve cross-correlates with the mask rather than
 * convolves; in other words, the rows and columns are not first reflected;
 * for the usual mirror symmetric masks this poses no complications).  Offset
 * designates the pixel of the mask which overlays a given input image pixel
 * in order to compute the cross-correlation corresponding to the
 * corresponding output image pixel.  The cross-correlation extends the edges
 * of the subimage whenever the mask extends beyond the subimage edges.
 *
 * input pixel format (integer mask):  INT
 * input pixel format (float mask):  FLOAT
 * output pixel format: same as input
 *
 * Based on HIPS-1 dog: Yoav Cohen - 12/12/82
 * HIPS-2 - msl - 7/16/91
 */

#include <hipl_format.h>

int h_hconvolve(hdi,hdo,mask,nmask,offset)

struct header *hdi,*hdo;
int *mask;		/* if input is PFFLOAT, this is treated as `float *' */
int nmask,offset;

{
	switch(hdi->pixel_format) {
	case PFINT:	return(h_hconvolve_i(hdi,hdo,mask,nmask,offset));
	case PFFLOAT:	return(h_hconvolve_f(hdi,hdo,(float *) mask,nmask,
				offset));
	default:	return(perr(HE_FMTSUBR,"h_hconvolve",
				hformatname(hdi->pixel_format)));
	}
}

int h_hconvolve_i(hdi,hdo,mask,nmask,offset)

struct header *hdi,*hdo;
int *mask,nmask,offset;

{
	return(h_hconvolve_I((int *) hdi->firstpix,(int *) hdo->firstpix,
	    hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,mask,nmask,offset));
}

int h_hconvolve_f(hdi,hdo,mask,nmask,offset)

struct header *hdi,*hdo;
float *mask;
int nmask,offset;

{
	return(h_hconvolve_F((float *) hdi->firstpix,(float *) hdo->firstpix,
	    hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,mask,nmask,offset));
}

int h_hconvolve_I(imagei,imageo,nr,nc,nlpi,nlpo,mask,nmask,offset)

int *imagei,*imageo,nr,nc,nlpi,nlpo,*mask,nmask,offset;

{
	int cm1,*op,nexo,lbound,rbound,rb1,ir,irc,lpix,rpix,ic,sum,lweight;
	int rweight,jc,i,nm1,*pm,*ip;

	cm1 = nc-1;
	op = imageo;
	nexo = nlpo - nc;
	lbound = offset;
	if (lbound>cm1)
		lbound = nc;
	else if (lbound<0)
		lbound = 0;
	rbound = nc - (nmask - offset);
	if (rbound>cm1)
		rbound = cm1;
	rb1 = rbound + 1;
	if (rb1 < 0)
		rb1 = 0;
	if (rb1 < lbound)
		rb1 = lbound;
	nm1 = nmask-1;

	for (ir=0,irc=0;ir<nr;ir++,irc+=nlpi) {
		lpix=imagei[irc]; rpix=imagei[irc+cm1];
		for (ic=0;ic<lbound;ic++) {
			sum = 0; lweight = 0; rweight = 0; jc = ic-offset;
			for (i=0;i<nmask;i++,jc++) {
				if (jc<=0)
					lweight += mask[i];
				else if (jc>=cm1)
					rweight += mask[i];
				else
					sum += mask[i]*imagei[irc+jc];
			}
			*op++ = sum + lpix*lweight + rpix*rweight;
		}
		ip = imagei + irc + lbound - offset;
		for (ic=lbound;ic<=rbound;ic++) {
			sum = 0; pm = mask;
			for (i=0;i<nmask;i++)
				sum += *pm++ * *ip++;
			*op++ = sum; ip -= nm1;
		}
		for (ic=rb1;ic<nc;ic++) {
			sum = 0; rweight = 0; jc = ic-offset;
			for (i=0;i<nmask;i++,jc++) {
				if (jc>=cm1)
					rweight += mask[i];
				else
					sum += mask[i]*imagei[irc+jc];
			}
			*op++ = sum + rpix*rweight;
		}
		op += nexo;
	}
	return(HIPS_OK);
}

int h_hconvolve_F(imagei,imageo,nr,nc,nlpi,nlpo,mask,nmask,offset)

float *imagei,*imageo,*mask;
int nr,nc,nlpi,nlpo,nmask,offset;

{
	float *op,lpix,rpix,sum,lweight,rweight,*pm,*ip;
	int cm1,nexo,lbound,rbound,rb1,ir,irc,ic,jc,i,nm1;

	cm1 = nc-1;
	op = imageo;
	nexo = nlpo - nc;
	lbound = offset;
	if (lbound>cm1)
		lbound = nc;
	else if (lbound<0)
		lbound = 0;
	rbound = nc - (nmask - offset);
	if (rbound>cm1)
		rbound = cm1;
	rb1 = rbound + 1;
	if (rb1 < 0)
		rb1 = 0;
	if (rb1 < lbound)
		rb1 = lbound;
	nm1 = nmask-1;

	for (ir=0,irc=0;ir<nr;ir++,irc+=nlpi) {
		lpix=imagei[irc]; rpix=imagei[irc+cm1];
		for (ic=0;ic<lbound;ic++) {
			sum = 0; lweight = 0; rweight = 0; jc = ic-offset;
			for (i=0;i<nmask;i++,jc++) {
				if (jc<=0)
					lweight += mask[i];
				else if (jc>=cm1)
					rweight += mask[i];
				else
					sum += mask[i]*imagei[irc+jc];
			}
			*op++ = sum + lpix*lweight + rpix*rweight;
		}
		ip = imagei + irc + lbound - offset;
		for (ic=lbound;ic<=rbound;ic++) {
			sum = 0; pm = mask;
			for (i=0;i<nmask;i++)
				sum += *pm++ * *ip++;
			*op++ = sum; ip -= nm1;
		}
		for (ic=rb1;ic<nc;ic++) {
			sum = 0; rweight = 0; jc = ic-offset;
			for (i=0;i<nmask;i++,jc++) {
				if (jc>=cm1)
					rweight += mask[i];
				else
					sum += mask[i]*imagei[irc+jc];
			}
			*op++ = sum + rpix*rweight;
		}
		op += nexo;
	}
	return(HIPS_OK);
}
