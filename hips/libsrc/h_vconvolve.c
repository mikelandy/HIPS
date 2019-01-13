/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_vconvolve.c - 1D vertical convolution
 *
 * (***important note***, h_vconvolve cross-correlates with the mask rather than
 * convolves; in other words, the rows and columns are not first reflected;
 * for the usual mirror symmetric masks this poses no complications).  Offset
 * designates the pixel of the mask which overlays a given input image pixel
 * in order to compute the cross-correlation corresponding to the
 * corresponding output image pixel.  The cross-correlation extends the edges
 * of the subimage whenever the mask extends beyond the subimage edges.
 *
 * Note that the earlier mask values are applied to earlier image
 * values.  Thus, the interpretation of the mask orientation depends on the
 * definition of ULORIG.  If ULORIG is defined (images have their origin at
 * the upper-left), then the first mask element is topmost relative to the
 * image.  Otherwise, the last mask element is topmost relative to the image.
 * Otherwise stated, a ULORIG coordinate system applies to masks as well as to
 * images.
 *
 * input pixel format (integer mask):  INT
 * input pixel format (float mask):  FLOAT
 * output pixel format: same as input
 *
 * Based on HIPS-1 dog: Yoav Cohen - 12/12/82
 * HIPS-2 - msl - 7/16/91
 */

#include <hipl_format.h>

int h_vconvolve(hdi,hdo,mask,nmask,offset)

struct header *hdi,*hdo;
int *mask;		/* if input is PFFLOAT, this is treated as `float *' */
int nmask,offset;

{
	switch(hdi->pixel_format) {
	case PFINT:	return(h_vconvolve_i(hdi,hdo,mask,nmask,offset));
	case PFFLOAT:	return(h_vconvolve_f(hdi,hdo,(float *) mask,nmask,
				offset));
	default:	return(perr(HE_FMTSUBR,"h_vconvolve",
				hformatname(hdi->pixel_format)));
	}
}

int h_vconvolve_i(hdi,hdo,mask,nmask,offset)

struct header *hdi,*hdo;
int *mask,nmask,offset;

{
	return(h_vconvolve_I((int *) hdi->firstpix,(int *) hdo->firstpix,
	    hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,mask,nmask,offset));
}

int h_vconvolve_f(hdi,hdo,mask,nmask,offset)

struct header *hdi,*hdo;
float *mask;
int nmask,offset;

{
	return(h_vconvolve_F((float *) hdi->firstpix,(float *) hdo->firstpix,
	    hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,mask,nmask,offset));
}

int h_vconvolve_I(imagei,imageo,nr,nc,nlpi,nlpo,mask,nmask,offset)

int *imagei,*imageo,nr,nc,nlpi,nlpo,*mask,nmask,offset;

{
	int rm1,*op,nexo,tbound,bbound,bb1,ir,ic,sum,tweight,*mp,nmlp1;
	int bweight,jr,i,rm1c,jrind,jrind0,nexi,*ip;

	rm1 = nr-1;
	op = imageo;
	nexo = nlpo - nc;
	nexi = nlpi - nc;
	tbound = offset;
	if (tbound>rm1)
		tbound = nr;
	else if (tbound<0)
		tbound = 0;
	bbound = nr - (nmask - offset);
	if (bbound>rm1)
		bbound = rm1;
	bb1 = bbound + 1;
	if (bb1 < 0)
		bb1 = 0;
	if (bb1 < tbound)
		bb1 = tbound;
	rm1c = rm1*nlpi;

	jrind0 = -offset*nlpi;
	for (ir=0;ir<tbound;ir++) {
		for (ic=0;ic<nc;ic++) {
			sum = 0; tweight = 0; bweight = 0; jrind = jrind0 + ic;
			for (i=0,jr=ir-offset;i<nmask;i++,jr++,jrind+=nlpi) {
				if (jr<=0)
					tweight += mask[i];
				else if (jr>=rm1)
					bweight += mask[i];
				else
					sum += mask[i]*imagei[jrind];
			}
			*op++ = sum + imagei[ic]*tweight +
				imagei[ic+rm1c]*bweight;
		}
		op += nexo;
		jrind0 += nlpi;
	}
	nmlp1 = nmask*nlpi - 1;
	ip = imagei + (tbound - offset)*nlpi;
	for (ir=tbound;ir<=bbound;ir++) {
		for (ic=0;ic<nc;ic++) {
			sum = 0; mp = mask;
			for (i=0;i<nmask;i++,ip+=nlpi)
				sum += *mp++ * *ip;
			*op++ = sum;
			ip -= nmlp1;
		}
		op += nexo;
		ip += nexi;
	}
	jrind0 = (bb1 - offset)*nlpi;
	for (ir=bb1;ir<nr;ir++) {
		for (ic=0;ic<nc;ic++) {
			sum = 0; bweight = 0; jrind = jrind0 + ic;
			for (i=0,jr=ir-offset;i<nmask;i++,jr++,jrind+=nlpi) {
				if (jr>=rm1)
					bweight += mask[i];
				else
					sum += mask[i]*imagei[jrind];
			}
			*op++ = sum + imagei[rm1c+ic]*bweight;
		}
		op += nexo;
		jrind0 += nlpi;
	}
	return(HIPS_OK);
}

int h_vconvolve_F(imagei,imageo,nr,nc,nlpi,nlpo,mask,nmask,offset)

float *imagei,*imageo,*mask;
int nr,nc,nlpi,nlpo,nmask,offset;

{
	float *op,sum,tweight,bweight,*mp,*ip;
	int rm1,nexo,tbound,bbound,bb1,ir,ic,jr,i,rm1c,jrind,jrind0,nmlp1;
	int nexi;

	rm1 = nr-1;
	op = imageo;
	nexo = nlpo - nc;
	nexi = nlpi - nc;
	tbound = offset;
	if (tbound>rm1)
		tbound = nr;
	else if (tbound<0)
		tbound = 0;
	bbound = nr - (nmask - offset);
	if (bbound>rm1)
		bbound = rm1;
	bb1 = bbound + 1;
	if (bb1 < 0)
		bb1 = 0;
	if (bb1 < tbound)
		bb1 = tbound;
	rm1c = rm1*nlpi;

	jrind0 = -offset*nlpi;
	for (ir=0;ir<tbound;ir++) {
		for (ic=0;ic<nc;ic++) {
			sum = 0; tweight = 0; bweight = 0; jrind = jrind0 + ic;
			for (i=0,jr=ir-offset;i<nmask;i++,jr++,jrind+=nlpi) {
				if (jr<=0)
					tweight += mask[i];
				else if (jr>=rm1)
					bweight += mask[i];
				else
					sum += mask[i]*imagei[jrind];
			}
			*op++ = sum + imagei[ic]*tweight +
				imagei[ic+rm1c]*bweight;
		}
		op += nexo;
		jrind0 += nlpi;
	}
	nmlp1 = nmask*nlpi - 1;
	ip = imagei + (tbound - offset)*nlpi;
	for (ir=tbound;ir<=bbound;ir++) {
		for (ic=0;ic<nc;ic++) {
			sum = 0; mp = mask;
			for (i=0;i<nmask;i++,ip+=nlpi)
				sum += *mp++ * *ip;
			*op++ = sum;
			ip -= nmlp1;
		}
		op += nexo;
		ip += nexi;
	}
	jrind0 = (bb1 - offset)*nlpi;
	for (ir=bb1;ir<nr;ir++) {
		for (ic=0;ic<nc;ic++) {
			sum = 0; bweight = 0; jrind = jrind0 + ic;
			for (i=0,jr=ir-offset;i<nmask;i++,jr++,jrind+=nlpi) {
				if (jr>=rm1)
					bweight += mask[i];
				else
					sum += mask[i]*imagei[jrind];
			}
			*op++ = sum + imagei[rm1c+ic]*bweight;
		}
		op += nexo;
		jrind0 += nlpi;
	}
	return(HIPS_OK);
}
