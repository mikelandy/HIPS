/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_minp.c - subroutines to compute the minimum of two images pixel by pixel
 *
 * pixel formats: BYTE, SHORT, INT, FLOAT, DOUBLE, INTPYR, FLOATPYR
 *
 * Michael Landy - 8/17/91
 */

#include <hipl_format.h>

#define OURMIN(A,B)	(((A) < (B)) ? (A) : (B))

int h_minp(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	switch(hdi1->pixel_format) {
	case PFBYTE:	return(h_minp_b(hdi1,hdi2,hdo));
	case PFSHORT:	return(h_minp_s(hdi1,hdi2,hdo));
	case PFINT:	return(h_minp_i(hdi1,hdi2,hdo));
	case PFFLOAT:	return(h_minp_f(hdi1,hdi2,hdo));
	case PFDOUBLE:	return(h_minp_d(hdi1,hdi2,hdo));
	case PFINTPYR:	return(h_minp_ip(hdi1,hdi2,hdo));
	case PFFLOATPYR:return(h_minp_fp(hdi1,hdi2,hdo));
	default:	return(perr(HE_FMTSUBR,"h_minp",
				hformatname(hdi1->pixel_format)));
	}
}

int h_minp_b(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_minp_B(hdi1->firstpix,hdi2->firstpix,
		hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_minp_s(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_minp_S((short *) hdi1->firstpix,(short *) hdi2->firstpix,
		(short *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_minp_i(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_minp_I((int *) hdi1->firstpix,(int *) hdi2->firstpix,
		(int *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_minp_f(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_minp_F((float *) hdi1->firstpix,(float *) hdi2->firstpix,
		(float *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_minp_d(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_minp_D((double *) hdi1->firstpix,(double *) hdi2->firstpix,
		(double *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_minp_ip(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_minp_I((int *) hdi1->image,(int *) hdi2->image,
		(int *) hdo->image,1,hdi1->numpix,
		hdi1->numpix,hdi1->numpix,hdi1->numpix));
}

int h_minp_fp(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_minp_F((float *) hdi1->image,(float *) hdi2->image,
		(float *) hdo->image,1,hdi1->numpix,
		hdi1->numpix,hdi1->numpix,hdi1->numpix));
}

int h_minp_B(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

byte *imagei1,*imagei2,*imageo;
int nr,nc,nlpi1,nlpi2,nlpo;

{
	register int i,j,nexi1,nexi2,nexo;
	register byte *pi1,*pi2,*po;

	nexi1 = nlpi1-nc;
	nexi2 = nlpi2-nc;
	nexo = nlpo-nc;
	pi1 = imagei1;
	pi2 = imagei2;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			*po++ = OURMIN(*pi1,*pi2);
			pi1++;
			pi2++;
		}
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_minp_S(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

short *imagei1,*imagei2,*imageo;
int nr,nc,nlpi1,nlpi2,nlpo;

{
	register int i,j,nexi1,nexi2,nexo;
	register short *pi1,*pi2,*po;

	nexi1 = nlpi1-nc;
	nexi2 = nlpi2-nc;
	nexo = nlpo-nc;
	pi1 = imagei1;
	pi2 = imagei2;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			*po++ = OURMIN(*pi1,*pi2);
			pi1++;
			pi2++;
		}
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_minp_I(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

int *imagei1,*imagei2,*imageo;
int nr,nc,nlpi1,nlpi2,nlpo;

{
	register int i,j,nexi1,nexi2,nexo;
	register int *pi1,*pi2,*po;

	nexi1 = nlpi1-nc;
	nexi2 = nlpi2-nc;
	nexo = nlpo-nc;
	pi1 = imagei1;
	pi2 = imagei2;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			*po++ = OURMIN(*pi1,*pi2);
			pi1++;
			pi2++;
		}
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_minp_F(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

float *imagei1,*imagei2,*imageo;
int nr,nc,nlpi1,nlpi2,nlpo;

{
	register int i,j,nexi1,nexi2,nexo;
	register float *pi1,*pi2,*po;

	nexi1 = nlpi1-nc;
	nexi2 = nlpi2-nc;
	nexo = nlpo-nc;
	pi1 = imagei1;
	pi2 = imagei2;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			*po++ = OURMIN(*pi1,*pi2);
			pi1++;
			pi2++;
		}
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_minp_D(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

double *imagei1,*imagei2,*imageo;
int nr,nc,nlpi1,nlpi2,nlpo;

{
	register int i,j,nexi1,nexi2,nexo;
	register double *pi1,*pi2,*po;

	nexi1 = nlpi1-nc;
	nexi2 = nlpi2-nc;
	nexo = nlpo-nc;
	pi1 = imagei1;
	pi2 = imagei2;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			*po++ = OURMIN(*pi1,*pi2);
			pi1++;
			pi2++;
		}
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}
