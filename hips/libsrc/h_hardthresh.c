/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_hardthresh.c - subroutines to apply a `hard threshold'
 *
 * Pixels below a designated threshold are set to hips_lchar and those above
 * are set to hips_hchar.  For complex images the designated threshold is a
 * float and is compared with pixel magnitudes.
 *
 * pixel formats: BYTE, INT, FLOAT, COMPLEX
 *
 * Michael Landy - 6/17/91
 */

#include <hipl_format.h>

int h_hardthresh(hdi,hdo,thresh)

struct header *hdi,*hdo;
Pixelval *thresh;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	return(h_hardthresh_b(hdi,hdo,thresh));
	case PFINT:	return(h_hardthresh_i(hdi,hdo,thresh));
	case PFFLOAT:	return(h_hardthresh_f(hdi,hdo,thresh));
	case PFCOMPLEX:	return(h_hardthresh_c(hdi,hdo,thresh));
	default:	return(perr(HE_FMTSUBR,"h_hardthresh",
				hformatname(hdi->pixel_format)));
	}
}

int h_hardthresh_b(hdi,hdo,thresh)

struct header *hdi,*hdo;
Pixelval *thresh;

{
	return(h_hardthresh_B(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
		hdi->ocols,hdo->ocols,thresh->v_byte));
}

int h_hardthresh_i(hdi,hdo,thresh)

struct header *hdi,*hdo;
Pixelval *thresh;

{
	return(h_hardthresh_I((int *) hdi->firstpix,(int *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,thresh->v_int));
}

int h_hardthresh_f(hdi,hdo,thresh)

struct header *hdi,*hdo;
Pixelval *thresh;

{
	return(h_hardthresh_F((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,thresh->v_float));
}

int h_hardthresh_c(hdi,hdo,thresh)

struct header *hdi,*hdo;
Pixelval *thresh;

{
	return(h_hardthresh_C((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,thresh->v_float));
}

int h_hardthresh_B(imagei,imageo,nr,nc,nlpi,nlpo,thresh)

byte *imagei,*imageo,thresh;
int nr,nc,nlpi,nlpo;

{
	register int i,j,nexi,nexo;
	register byte *pi,*po;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*po++ = (*pi++ >= thresh) ? hips_hchar : hips_lchar;
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_hardthresh_I(imagei,imageo,nr,nc,nlpi,nlpo,thresh)

int *imagei,*imageo;
int nr,nc,nlpi,nlpo,thresh;

{
	register int i,j,nexi,nexo;
	register int *pi,*po;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*po++ = (*pi++ >= thresh) ? hips_hchar : hips_lchar;
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_hardthresh_F(imagei,imageo,nr,nc,nlpi,nlpo,thresh)

float *imagei,*imageo,thresh;
int nr,nc,nlpi,nlpo;

{
	register int i,j,nexi,nexo;
	register float *pi,*po;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*po++ = (*pi++ >= thresh) ? hips_hchar : hips_lchar;
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_hardthresh_C(imagei,imageo,nr,nc,nlpi,nlpo,thresh)

float *imagei,*imageo,thresh;
int nr,nc,nlpi,nlpo;

{
	register int i,j,nexi,nexo;
	register float *pi,*po,sqthresh,sqmagn;

	nexi = 2*(nlpi-nc);
	nexo = 2*(nlpo-nc);
	pi = imagei;
	po = imageo;
	sqthresh = thresh*thresh;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			sqmagn = (pi[0] * pi[0]) + (pi[1] * pi[1]);
			*po++ = (sqmagn >= sqthresh) ? hips_hchar : hips_lchar;
			*po++ = 0;
			pi += 2;
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}
