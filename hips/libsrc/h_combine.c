/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_combine.c - subroutines to combine two real images into one complex image
 *
 * The input images are either real and imaginary parts, or if phasemagflag is
 * TRUE, then phase and magnitude.
 *
 * pixel formats: FLOAT->COMPLEX, DOUBLE->DBLCOM
 *
 * Michael Landy - 8/9/91
 */

#include <hipl_format.h>
#include <math.h>

int h_combine(hdi1,hdi2,hdo,phasemagflag)

struct header *hdi1,*hdi2,*hdo;
h_boolean phasemagflag;

{
	switch(hdi1->pixel_format) {
	case PFFLOAT:	return(h_combine_f(hdi1,hdi2,hdo,phasemagflag));
	case PFDOUBLE:	return(h_combine_d(hdi1,hdi2,hdo,phasemagflag));
	default:	return(perr(HE_FMTSUBR,"h_combine",
				hformatname(hdi1->pixel_format)));
	}
}

int h_combine_f(hdi1,hdi2,hdo,phasemagflag)

struct header *hdi1,*hdi2,*hdo;
h_boolean phasemagflag;

{
	return(h_combine_F((float *) hdi1->firstpix,(float *) hdi2->firstpix,
		(float *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols,phasemagflag));
}

int h_combine_d(hdi1,hdi2,hdo,phasemagflag)

struct header *hdi1,*hdi2,*hdo;
h_boolean phasemagflag;

{
	return(h_combine_D((double *) hdi1->firstpix,(double *) hdi2->firstpix,
		(double *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols,phasemagflag));
}

int h_combine_F(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo,phasemagflag)

float *imagei1,*imagei2,*imageo;
int nr,nc,nlpi1,nlpi2,nlpo;
h_boolean phasemagflag;

{
	register int i,j,nexi1,nexi2,nexo;
	register float *pi1,*pi2,*po;

	nexi1 = nlpi1-nc;
	nexi2 = nlpi2-nc;
	nexo = 2*(nlpo-nc);
	pi1 = imagei1;
	pi2 = imagei2;
	po = imageo;
	if (phasemagflag) {
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				*po++ = *pi2 * cos((double) *pi1);
				*po++ = *pi2++ * sin((double) *pi1++);
			}
			pi1 += nexi1;
			pi2 += nexi2;
			po += nexo;
		}
	}
	else {
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				*po++ = *pi1++;
				*po++ = *pi2++;
			}
			pi1 += nexi1;
			pi2 += nexi2;
			po += nexo;
		}
	}
	return(HIPS_OK);
}

int h_combine_D(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo,phasemagflag)

double *imagei1,*imagei2,*imageo;
int nr,nc,nlpi1,nlpi2,nlpo;
h_boolean phasemagflag;

{
	register int i,j,nexi1,nexi2,nexo;
	register double *pi1,*pi2,*po;

	nexi1 = nlpi1-nc;
	nexi2 = nlpi2-nc;
	nexo = 2*(nlpo-nc);
	pi1 = imagei1;
	pi2 = imagei2;
	po = imageo;
	if (phasemagflag) {
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				*po++ = *pi2 * cos((double) *pi1);
				*po++ = *pi2++ * sin((double) *pi1++);
			}
			pi1 += nexi1;
			pi2 += nexi2;
			po += nexo;
		}
	}
	else {
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				*po++ = *pi1++;
				*po++ = *pi2++;
			}
			pi1 += nexi1;
			pi2 += nexi2;
			po += nexo;
		}
	}
	return(HIPS_OK);
}
