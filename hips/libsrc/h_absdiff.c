/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_absdiff.c - subroutines to compute the absolute value of the difference
 *			of two images
 *
 * pixel formats: BYTE, SHORT, INT, FLOAT, DOUBLE
 *
 * Michael Landy - 7/3/91
 */

#include <hipl_format.h>

int h_absdiff(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	switch(hdi1->pixel_format) {
	case PFBYTE:	return(h_absdiff_b(hdi1,hdi2,hdo));
	case PFSHORT:	return(h_absdiff_s(hdi1,hdi2,hdo));
	case PFINT:	return(h_absdiff_i(hdi1,hdi2,hdo));
	case PFFLOAT:	return(h_absdiff_f(hdi1,hdi2,hdo));
	case PFDOUBLE:	return(h_absdiff_d(hdi1,hdi2,hdo));
	default:	return(perr(HE_FMTSUBR,"h_absdiff",
				hformatname(hdi1->pixel_format)));
	}
}

int h_absdiff_b(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_absdiff_B(hdi1->firstpix,hdi2->firstpix,hdo->firstpix,
		hdi1->rows,hdi1->cols,hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_absdiff_s(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_absdiff_S((short *) hdi1->firstpix,(short *) hdi2->firstpix,
		(short *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_absdiff_i(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_absdiff_I((int *) hdi1->firstpix,(int *) hdi2->firstpix,
		(int *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_absdiff_f(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_absdiff_F((float *) hdi1->firstpix,(float *) hdi2->firstpix,
		(float *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_absdiff_d(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_absdiff_D((double *) hdi1->firstpix,(double *) hdi2->firstpix,
		(double *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_absdiff_B(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

byte *imagei1,*imagei2,*imageo;
int nr,nc,nlpi1,nlpi2,nlpo;

{
	register int i,j,nexi1,nexi2,nexo,diff;
	register byte *pi1,*pi2,*po;

	nexi1 = nlpi1-nc;
	nexi2 = nlpi2-nc;
	nexo = nlpo-nc;
	pi1 = imagei1;
	pi2 = imagei2;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			diff = ((int) *pi1++) - *pi2++;
			*po++ = (diff < 0) ? -diff : diff;
		}
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_absdiff_S(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

short *imagei1,*imagei2,*imageo;
int nr,nc,nlpi1,nlpi2,nlpo;

{
	register int i,j,nexi1,nexi2,nexo;
	register short *pi1,*pi2,*po,diff;

	nexi1 = nlpi1-nc;
	nexi2 = nlpi2-nc;
	nexo = nlpo-nc;
	pi1 = imagei1;
	pi2 = imagei2;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			diff = *pi1++ - *pi2++;
			*po++ = (diff < 0) ? -diff : diff;
		}
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_absdiff_I(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

int *imagei1,*imagei2,*imageo;
int nr,nc,nlpi1,nlpi2,nlpo;

{
	register int i,j,nexi1,nexi2,nexo;
	register int *pi1,*pi2,*po,diff;

	nexi1 = nlpi1-nc;
	nexi2 = nlpi2-nc;
	nexo = nlpo-nc;
	pi1 = imagei1;
	pi2 = imagei2;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			diff = *pi1++ - *pi2++;
			*po++ = (diff < 0) ? -diff : diff;
		}
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_absdiff_F(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

float *imagei1,*imagei2,*imageo;
int nr,nc,nlpi1,nlpi2,nlpo;

{
	register int i,j,nexi1,nexi2,nexo;
	register float *pi1,*pi2,*po,diff;

	nexi1 = nlpi1-nc;
	nexi2 = nlpi2-nc;
	nexo = nlpo-nc;
	pi1 = imagei1;
	pi2 = imagei2;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			diff = *pi1++ - *pi2++;
			*po++ = (diff < 0) ? -diff : diff;
		}
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_absdiff_D(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

double *imagei1,*imagei2,*imageo;
int nr,nc,nlpi1,nlpi2,nlpo;

{
	register int i,j,nexi1,nexi2,nexo;
	register double *pi1,*pi2,*po,diff;

	nexi1 = nlpi1-nc;
	nexi2 = nlpi2-nc;
	nexo = nlpo-nc;
	pi1 = imagei1;
	pi2 = imagei2;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			diff = *pi1++ - *pi2++;
			*po++ = (diff < 0) ? -diff : diff;
		}
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}
