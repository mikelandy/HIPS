/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_transpose.c - subroutines to reflect an image about the main diagonal
 *
 * pixel formats: BYTE, INT, FLOAT
 *
 * Michael Landy - 6/23/91
 */

#include <hipl_format.h>

int h_transpose(hdi,hdo)

struct header *hdi,*hdo;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	return(h_transpose_b(hdi,hdo));
	case PFINT:	return(h_transpose_i(hdi,hdo));
	case PFFLOAT:	return(h_transpose_f(hdi,hdo));
	default:	return(perr(HE_FMTSUBR,"h_transpose",
				hformatname(hdi->pixel_format)));
	}
}

int h_transpose_b(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_transpose_B(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
		hdi->ocols,hdo->ocols));
}

int h_transpose_i(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_transpose_I((int *) hdi->firstpix,(int *) hdo->firstpix,hdi->rows,
		hdi->cols,hdi->ocols,hdo->ocols));
}

int h_transpose_f(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_transpose_F((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols));
}

int h_transpose_B(imagei,imageo,nr,nc,nlpi,nlpo)

byte *imagei,*imageo;
int nr,nc,nlpi,nlpo;

{
	register int i,j,nexi,nexo,incr;
	register byte *pi,*po;

	nexi = nlpi-nc;
	pi = imagei;
#ifdef ULORIG
	po = imageo;
	incr = nlpo;
	nexo = 1-nc*nlpo;
#else
	po = imageo + (nlpo*nc) + nr - 1;;
	incr = -nlpo;
	nexo = nc*nlpo-1;
#endif
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			*po = *pi++;
			po += incr;
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_transpose_I(imagei,imageo,nr,nc,nlpi,nlpo)

int *imagei,*imageo;
int nr,nc,nlpi,nlpo;

{
	register int i,j,nexi,nexo,incr;
	register int *pi,*po;

	nexi = nlpi-nc;
	pi = imagei;
#ifdef ULORIG
	po = imageo;
	incr = nlpo;
	nexo = 1-nc*nlpo;
#else
	po = imageo + (nlpo*nc) + nr - 1;;
	incr = -nlpo;
	nexo = nc*nlpo-1;
#endif
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			*po = *pi++;
			po += incr;
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_transpose_F(imagei,imageo,nr,nc,nlpi,nlpo)

float *imagei,*imageo;
int nr,nc,nlpi,nlpo;

{
	register int i,j,nexi,nexo,incr;
	register float *pi,*po;

	nexi = nlpi-nc;
	pi = imagei;
#ifdef ULORIG
	po = imageo;
	incr = nlpo;
	nexo = 1-nc*nlpo;
#else
	po = imageo + (nlpo*nc) + nr - 1;;
	incr = -nlpo;
	nexo = nc*nlpo-1;
#endif
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			*po = *pi++;
			po += incr;
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}
