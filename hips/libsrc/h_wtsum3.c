/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_wtsum3.c - subroutines to compute a weighted sum of three images
 *
 * As for computing luminance or other transformations of color images.
 *
 * pixel formats: BYTE, SHORT, INT, FLOAT
 *
 * Michael Landy - 8/17/91
 */

#include <hipl_format.h>

int h_wtsum3(hdi1,hdi2,hdi3,hdo,wt1,wt2,wt3)

struct header *hdi1,*hdi2,*hdi3,*hdo;
float wt1,wt2,wt3;

{
	switch(hdi1->pixel_format) {
	case PFBYTE:	return(h_wtsum3_b(hdi1,hdi2,hdi3,hdo,wt1,wt2,wt3));
	case PFSHORT:	return(h_wtsum3_s(hdi1,hdi2,hdi3,hdo,wt1,wt2,wt3));
	case PFINT:	return(h_wtsum3_i(hdi1,hdi2,hdi3,hdo,wt1,wt2,wt3));
	case PFFLOAT:	return(h_wtsum3_f(hdi1,hdi2,hdi3,hdo,wt1,wt2,wt3));
	default:	return(perr(HE_FMTSUBR,"h_wtsum3",
				hformatname(hdi1->pixel_format)));
	}
}

int h_wtsum3_b(hdi1,hdi2,hdi3,hdo,wt1,wt2,wt3)

struct header *hdi1,*hdi2,*hdi3,*hdo;
float wt1,wt2,wt3;

{
	return(h_wtsum3_B(hdi1->firstpix,hdi2->firstpix,hdi3->firstpix,
		hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdi3->ocols,hdo->ocols,wt1,wt2,wt3));
}

int h_wtsum3_s(hdi1,hdi2,hdi3,hdo,wt1,wt2,wt3)

struct header *hdi1,*hdi2,*hdi3,*hdo;
float wt1,wt2,wt3;

{
	return(h_wtsum3_S((short *) hdi1->firstpix,(short *) hdi2->firstpix,
		(short *) hdi3->firstpix,
		(short *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdi3->ocols,hdo->ocols,wt1,wt2,wt3));
}

int h_wtsum3_i(hdi1,hdi2,hdi3,hdo,wt1,wt2,wt3)

struct header *hdi1,*hdi2,*hdi3,*hdo;
float wt1,wt2,wt3;

{
	return(h_wtsum3_I((int *) hdi1->firstpix,(int *) hdi2->firstpix,
		(int *) hdi3->firstpix,
		(int *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdi3->ocols,hdo->ocols,wt1,wt2,wt3));
}

int h_wtsum3_f(hdi1,hdi2,hdi3,hdo,wt1,wt2,wt3)

struct header *hdi1,*hdi2,*hdi3,*hdo;
float wt1,wt2,wt3;

{
	return(h_wtsum3_F((float *) hdi1->firstpix,(float *) hdi2->firstpix,
		(float *) hdi3->firstpix,
		(float *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdi3->ocols,hdo->ocols,wt1,wt2,wt3));
}

int h_wtsum3_B(imagei1,imagei2,imagei3,imageo,nr,nc,nlpi1,nlpi2,nlpi3,nlpo,wt1,wt2,
	wt3)

byte *imagei1,*imagei2,*imagei3,*imageo;
int nr,nc,nlpi1,nlpi2,nlpi3,nlpo;
float wt1,wt2,wt3;

{
	register int i,j,nexi1,nexi2,nexi3,nexo,val;
	register byte *pi1,*pi2,*pi3,*po;

	nexi1 = nlpi1-nc;
	nexi2 = nlpi2-nc;
	nexi3 = nlpi3-nc;
	nexo = nlpo-nc;
	pi1 = imagei1;
	pi2 = imagei2;
	pi3 = imagei3;
	po = imageo;
	hips_lclip = hips_hclip = 0;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			val = (wt1 * *pi1++) + (wt2 * *pi2++) +
				(wt3 * *pi3++) + .5;
			if (val < 0) {
				hips_lclip++;
				*po++ = 0;
			}
			else if (val > 255) {
				hips_hclip++;
				*po++ = 255;
			}
			else
				*po++ = val;
		}
		pi1 += nexi1;
		pi2 += nexi2;
		pi3 += nexi3;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_wtsum3_S(imagei1,imagei2,imagei3,imageo,nr,nc,nlpi1,nlpi2,nlpi3,nlpo,wt1,wt2,
	wt3)

short *imagei1,*imagei2,*imagei3,*imageo;
int nr,nc,nlpi1,nlpi2,nlpi3,nlpo;
float wt1,wt2,wt3;

{
	register int i,j,nexi1,nexi2,nexi3,nexo;
	register short *pi1,*pi2,*pi3,*po;

	nexi1 = nlpi1-nc;
	nexi2 = nlpi2-nc;
	nexi3 = nlpi3-nc;
	nexo = nlpo-nc;
	pi1 = imagei1;
	pi2 = imagei2;
	pi3 = imagei3;
	po = imageo;
	hips_lclip = hips_hclip = 0;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*po++ = (wt1 * *pi1++) + (wt2 * *pi2++) +
				(wt3 * *pi3++) + .5;
		pi1 += nexi1;
		pi2 += nexi2;
		pi3 += nexi3;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_wtsum3_I(imagei1,imagei2,imagei3,imageo,nr,nc,nlpi1,nlpi2,nlpi3,nlpo,wt1,wt2,
	wt3)

int *imagei1,*imagei2,*imagei3,*imageo;
int nr,nc,nlpi1,nlpi2,nlpi3,nlpo;
float wt1,wt2,wt3;

{
	register int i,j,nexi1,nexi2,nexi3,nexo;
	register int *pi1,*pi2,*pi3,*po;

	nexi1 = nlpi1-nc;
	nexi2 = nlpi2-nc;
	nexi3 = nlpi3-nc;
	nexo = nlpo-nc;
	pi1 = imagei1;
	pi2 = imagei2;
	pi3 = imagei3;
	po = imageo;
	hips_lclip = hips_hclip = 0;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*po++ = (wt1 * *pi1++) + (wt2 * *pi2++) +
				(wt3 * *pi3++) + .5;
		pi1 += nexi1;
		pi2 += nexi2;
		pi3 += nexi3;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_wtsum3_F(imagei1,imagei2,imagei3,imageo,nr,nc,nlpi1,nlpi2,nlpi3,nlpo,wt1,wt2,
	wt3)

float *imagei1,*imagei2,*imagei3,*imageo;
int nr,nc,nlpi1,nlpi2,nlpi3,nlpo;
float wt1,wt2,wt3;

{
	register int i,j,nexi1,nexi2,nexi3,nexo;
	register float *pi1,*pi2,*pi3,*po;

	nexi1 = nlpi1-nc;
	nexi2 = nlpi2-nc;
	nexi3 = nlpi3-nc;
	nexo = nlpo-nc;
	pi1 = imagei1;
	pi2 = imagei2;
	pi3 = imagei3;
	po = imageo;
	hips_lclip = hips_hclip = 0;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*po++ = (wt1 * *pi1++) + (wt2 * *pi2++) +
				(wt3 * *pi3++);
		pi1 += nexi1;
		pi2 += nexi2;
		pi3 += nexi3;
		po += nexo;
	}
	return(HIPS_OK);
}
