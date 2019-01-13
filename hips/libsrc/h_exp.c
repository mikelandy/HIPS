/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_exp.c - exponential function
 *
 * input pixel formats: BYTE, SHORT, INT, FLOAT
 * output pixel formats: FLOAT (for others)
 *
 * Computes exp(pixel)
 *
 * Mike Landy - 4/25/89
 * HIPS 2 - msl - 6/13/91
 */

#include <hipl_format.h>
#include <math.h>

int h_exp(hdi,hdo)

struct header *hdi,*hdo;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	return(h_exp_b(hdi,hdo));
	case PFSHORT:	return(h_exp_s(hdi,hdo));
	case PFINT:	return(h_exp_i(hdi,hdo));
	case PFFLOAT:	return(h_exp_f(hdi,hdo));
	default:	return(perr(HE_FMTSUBR,"h_exp",
				hformatname(hdi->pixel_format)));
	}
}

int h_exp_b(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_exp_B(hdi->firstpix,(float *) hdo->firstpix,hdi->rows,
		hdi->cols,hdi->ocols,hdo->ocols));
}

int h_exp_s(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_exp_S((short *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols));
}

int h_exp_i(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_exp_I((int *) hdi->firstpix,(float *) hdo->firstpix,hdi->rows,
		hdi->cols,hdi->ocols,hdo->ocols));
}

int h_exp_f(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_exp_F((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols));
}

int h_exp_B(imagei,imageo,nr,nc,nlpi,nlpo)

byte *imagei;
float *imageo;
int nr,nc,nlpi,nlpo;

{
	register int i,j,nexi,nexo;
	register byte *pi;
	float *po;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*po++ = exp((double) *pi++);
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_exp_S(imagei,imageo,nr,nc,nlpi,nlpo)

short *imagei;
float *imageo;
int nr,nc,nlpi,nlpo;

{
	register int i,j,nexi,nexo;
	register short *pi;
	register float *po;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*po++ = exp((double) *pi++);
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_exp_I(imagei,imageo,nr,nc,nlpi,nlpo)

int *imagei;
float *imageo;
int nr,nc,nlpi,nlpo;

{
	register int i,j,nexi,nexo;
	register int *pi;
	register float *po;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*po++ = exp((double) *pi++);
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_exp_F(imagei,imageo,nr,nc,nlpi,nlpo)

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
		for (j=0;j<nc;j++)
			*po++ = exp((double) *pi++);
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}
