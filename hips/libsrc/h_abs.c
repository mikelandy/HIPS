/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_abs.c - subroutines to take the absolute value of pixels
 *
 * pixel formats: INT, FLOAT
 *
 * Michael Landy - 1/10/91
 */

#include <hipl_format.h>

int h_abs(hdi,hdo)

struct header *hdi,*hdo;

{
	switch(hdi->pixel_format) {
	case PFINT:	return(h_abs_i(hdi,hdo));
	case PFFLOAT:	return(h_abs_f(hdi,hdo));
	default:	return(perr(HE_FMTSUBR,"h_abs",
				hformatname(hdi->pixel_format)));
	}
}

int h_abs_i(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_abs_I((int *) hdi->firstpix,(int *) hdo->firstpix,hdi->rows,
		hdi->cols,hdi->ocols,hdo->ocols));
}

int h_abs_f(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_abs_F((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols));
}

int h_abs_I(imagei,imageo,nr,nc,nlpi,nlpo)

int *imagei,*imageo;
int nr,nc,nlpi,nlpo;

{
	register int i,j,nexi,nexo;
	register int *pi,*po;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			if (*pi < 0)
				*po++ = - *pi++;
			else
				*po++ = *pi++;
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_abs_F(imagei,imageo,nr,nc,nlpi,nlpo)

float *imagei,*imageo;
int nr,nc,nlpi,nlpo;

{
	register int i,j,nexi,nexo;
	register float *pi,*po;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			if (*pi < 0)
				*po++ = - *pi++;
			else
				*po++ = *pi++;
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}
