/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_avg.c - subroutines to compute a weighted average of two images
 *
 * pixel formats: BYTE, SHORT, INT, FLOAT
 *
 * Michael Landy - 7/8/91
 */

#include <hipl_format.h>

int h_avg(hdi1,hdi2,hdo,wt1,wt2)

struct header *hdi1,*hdi2,*hdo;
float wt1,wt2;

{
	switch(hdi1->pixel_format) {
	case PFBYTE:	return(h_avg_b(hdi1,hdi2,hdo,wt1,wt2));
	case PFSHORT:	return(h_avg_s(hdi1,hdi2,hdo,wt1,wt2));
	case PFINT:	return(h_avg_i(hdi1,hdi2,hdo,wt1,wt2));
	case PFFLOAT:	return(h_avg_f(hdi1,hdi2,hdo,wt1,wt2));
	default:	return(perr(HE_FMTSUBR,"h_avg",
				hformatname(hdi1->pixel_format)));
	}
}

int h_avg_b(hdi1,hdi2,hdo,wt1,wt2)

struct header *hdi1,*hdi2,*hdo;
float wt1,wt2;

{
	return(h_avg_B(hdi1->firstpix,hdi2->firstpix,
		hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols,wt1,wt2));
}

int h_avg_s(hdi1,hdi2,hdo,wt1,wt2)

struct header *hdi1,*hdi2,*hdo;
float wt1,wt2;

{
	return(h_avg_S((short *) hdi1->firstpix,(short *) hdi2->firstpix,
		(short *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols,wt1,wt2));
}

int h_avg_i(hdi1,hdi2,hdo,wt1,wt2)

struct header *hdi1,*hdi2,*hdo;
float wt1,wt2;

{
	return(h_avg_I((int *) hdi1->firstpix,(int *) hdi2->firstpix,
		(int *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols,wt1,wt2));
}

int h_avg_f(hdi1,hdi2,hdo,wt1,wt2)

struct header *hdi1,*hdi2,*hdo;
float wt1,wt2;

{
	return(h_avg_F((float *) hdi1->firstpix,(float *) hdi2->firstpix,
		(float *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols,wt1,wt2));
}

int h_avg_B(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo,wt1,wt2)

byte *imagei1,*imagei2,*imageo;
int nr,nc,nlpi1,nlpi2,nlpo;
float wt1,wt2;

{
	register int i,j,nexi1,nexi2,nexo;
	register byte *pi1,*pi2,*po;
	float wwt1,wwt2;

	if (wt1+wt2 == 0.)
		wwt1 = wwt2 = .5;
	else {
		wwt1 = wt1/(wt1+wt2);
		wwt2 = wt2/(wt1+wt2);
	}
	nexi1 = nlpi1-nc;
	nexi2 = nlpi2-nc;
	nexo = nlpo-nc;
	pi1 = imagei1;
	pi2 = imagei2;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*po++ = (wwt1 * *pi1++) + (wwt2 * *pi2++);
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_avg_S(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo,wt1,wt2)

short *imagei1,*imagei2,*imageo;
int nr,nc,nlpi1,nlpi2,nlpo;
float wt1,wt2;

{
	register int i,j,nexi1,nexi2,nexo;
	register short *pi1,*pi2,*po;
	float wwt1,wwt2;

	if (wt1+wt2 == 0.)
		wwt1 = wwt2 = .5;
	else {
		wwt1 = wt1/(wt1+wt2);
		wwt2 = wt2/(wt1+wt2);
	}
	nexi1 = nlpi1-nc;
	nexi2 = nlpi2-nc;
	nexo = nlpo-nc;
	pi1 = imagei1;
	pi2 = imagei2;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*po++ = (wwt1 * *pi1++) + (wwt2 * *pi2++);
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_avg_I(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo,wt1,wt2)

int *imagei1,*imagei2,*imageo;
int nr,nc,nlpi1,nlpi2,nlpo;
float wt1,wt2;

{
	register int i,j,nexi1,nexi2,nexo;
	register int *pi1,*pi2,*po;
	float wwt1,wwt2;

	if (wt1+wt2 == 0.)
		wwt1 = wwt2 = .5;
	else {
		wwt1 = wt1/(wt1+wt2);
		wwt2 = wt2/(wt1+wt2);
	}
	nexi1 = nlpi1-nc;
	nexi2 = nlpi2-nc;
	nexo = nlpo-nc;
	pi1 = imagei1;
	pi2 = imagei2;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*po++ = (wwt1 * *pi1++) + (wwt2 * *pi2++);
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_avg_F(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo,wt1,wt2)

float *imagei1,*imagei2,*imageo;
int nr,nc,nlpi1,nlpi2,nlpo;
float wt1,wt2;

{
	register int i,j,nexi1,nexi2,nexo;
	register float *pi1,*pi2,*po;
	float wwt1,wwt2;

	if (wt1+wt2 == 0.)
		wwt1 = wwt2 = .5;
	else {
		wwt1 = wt1/(wt1+wt2);
		wwt2 = wt2/(wt1+wt2);
	}
	nexi1 = nlpi1-nc;
	nexi2 = nlpi2-nc;
	nexo = nlpo-nc;
	pi1 = imagei1;
	pi2 = imagei2;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*po++ = (wwt1 * *pi1++) + (wwt2 * *pi2++);
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}
