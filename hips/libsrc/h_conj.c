/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_conj.c - subroutines to compute the complex conjugate
 *
 * pixel formats: COMPLEX, DBLCOM
 *
 * Michael Landy - 3/14/95
 */

#include <hipl_format.h>
#include <math.h>

int h_conj(hdi,hdo)

struct header *hdi,*hdo;

{
	switch(hdi->pixel_format) {
	case PFCOMPLEX:	return(h_conj_c(hdi,hdo));
	case PFDBLCOM:	return(h_conj_dc(hdi,hdo));
	default:	return(perr(HE_FMTSUBR,"h_conj",
				hformatname(hdi->pixel_format)));
	}
}

int h_conj_c(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_conj_C((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols));
}

int h_conj_dc(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_conj_DC((double *) hdi->firstpix,(double *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols));
}

int h_conj_C(imagei,imageo,nr,nc,nlpi,nlpo)

float *imagei,*imageo;
int nr,nc,nlpi,nlpo;

{
	register int i,j,nexi,nexo;
	register float *pi,*po;

	nexi = 2*(nlpi-nc);
	nexo = 2*(nlpo-nc);
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			*po++ = *pi++;
			*po++ = - *pi++;
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_conj_DC(imagei,imageo,nr,nc,nlpi,nlpo)

double *imagei,*imageo;
int nr,nc,nlpi,nlpo;

{
	register int i,j,nexi,nexo;
	register double *pi,*po;

	nexi = 2*(nlpi-nc);
	nexo = 2*(nlpo-nc);
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			*po++ = *pi++;
			*po++ = - *pi++;
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}
