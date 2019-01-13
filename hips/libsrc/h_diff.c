/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_diff.c - subroutines to subtract two images
 *
 * pixel formats: SHORT, INT, FLOAT, DOUBLE, COMPLEX, DBLCOM, INTPYR, FLOATPYR
 *			also:  INT-BYTE->INT
 *
 * Michael Landy - 7/3/91
 */

#include <hipl_format.h>

int h_diff(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	switch(hdi1->pixel_format) {
	case PFSHORT:	return(h_diff_s(hdi1,hdi2,hdo));
	case PFINT:	if (hdi2->pixel_format == PFINT)
				return(h_diff_i(hdi1,hdi2,hdo));
			else if (hdi2->pixel_format == PFBYTE &&
			    hdo->pixel_format == PFINT)
				return(h_diff_ibi(hdi1,hdi2,hdo));
			else
				return(perr(HE_FMT3SUBR,"h_diff",
					hformatname(hdi1->pixel_format),
					hformatname(hdi2->pixel_format),
					hformatname(hdo->pixel_format)));
	case PFFLOAT:	return(h_diff_f(hdi1,hdi2,hdo));
	case PFDOUBLE:	return(h_diff_d(hdi1,hdi2,hdo));
	case PFCOMPLEX:	return(h_diff_c(hdi1,hdi2,hdo));
	case PFDBLCOM:	return(h_diff_dc(hdi1,hdi2,hdo));
	case PFINTPYR:	return(h_diff_ip(hdi1,hdi2,hdo));
	case PFFLOATPYR:return(h_diff_fp(hdi1,hdi2,hdo));
	default:	return(perr(HE_FMTSUBR,"h_diff",
				hformatname(hdi1->pixel_format)));
	}
}

int h_diff_s(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_diff_S((short *) hdi1->firstpix,(short *) hdi2->firstpix,
		(short *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_diff_i(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_diff_I((int *) hdi1->firstpix,(int *) hdi2->firstpix,
		(int *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_diff_ibi(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_diff_IBI((int *) hdi1->firstpix,(byte *) hdi2->firstpix,
		(int *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_diff_f(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_diff_F((float *) hdi1->firstpix,(float *) hdi2->firstpix,
		(float *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_diff_d(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_diff_D((double *) hdi1->firstpix,(double *) hdi2->firstpix,
		(double *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_diff_c(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_diff_C((float *) hdi1->firstpix,(float *) hdi2->firstpix,
		(float *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_diff_dc(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_diff_DC((double *) hdi1->firstpix,(double *) hdi2->firstpix,
		(double *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_diff_ip(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_diff_I((int *) hdi1->image,(int *) hdi2->image,
		(int *) hdo->image,1,hdi1->numpix,
		hdi1->numpix,hdi1->numpix,hdi1->numpix));
}

int h_diff_fp(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_diff_F((float *) hdi1->image,(float *) hdi2->image,
		(float *) hdo->image,1,hdi1->numpix,
		hdi1->numpix,hdi1->numpix,hdi1->numpix));
}

int h_diff_S(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

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
		for (j=0;j<nc;j++)
			*po++ = *pi1++ - *pi2++;
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_diff_I(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

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
		for (j=0;j<nc;j++)
			*po++ = *pi1++ - *pi2++;
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_diff_IBI(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

byte *imagei2;
int *imagei1,*imageo;
int nr,nc,nlpi1,nlpi2,nlpo;

{
	register int i,j,nexi1,nexi2,nexo;
	register byte *pi2;
	register int *pi1,*po;

	nexi1 = nlpi1-nc;
	nexi2 = nlpi2-nc;
	nexo = nlpo-nc;
	pi1 = imagei1;
	pi2 = imagei2;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*po++ = *pi1++ - *pi2++;
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_diff_F(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

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
		for (j=0;j<nc;j++)
			*po++ = *pi1++ - *pi2++;
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_diff_D(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

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
		for (j=0;j<nc;j++)
			*po++ = *pi1++ - *pi2++;
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_diff_C(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

float *imagei1,*imagei2,*imageo;
int nr,nc,nlpi1,nlpi2,nlpo;

{
	register int i,j,nexi1,nexi2,nexo;
	register float *pi1,*pi2,*po;

	nexi1 = 2*(nlpi1-nc);
	nexi2 = 2*(nlpi2-nc);
	nexo = 2*(nlpo-nc);
	pi1 = imagei1;
	pi2 = imagei2;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			*po++ = *pi1++ - *pi2++;
			*po++ = *pi1++ - *pi2++;
		}
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_diff_DC(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

double *imagei1,*imagei2,*imageo;
int nr,nc,nlpi1,nlpi2,nlpo;

{
	register int i,j,nexi1,nexi2,nexo;
	register double *pi1,*pi2,*po;

	nexi1 = 2*(nlpi1-nc);
	nexi2 = 2*(nlpi2-nc);
	nexo = 2*(nlpo-nc);
	pi1 = imagei1;
	pi2 = imagei2;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			*po++ = *pi1++ - *pi2++;
			*po++ = *pi1++ - *pi2++;
		}
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}
