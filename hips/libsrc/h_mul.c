/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_mul.c - subroutines to multiply two images
 *
 * pixel formats: BYTE, SHORT, INT, FLOAT, FLOAT*COMPLEX, DOUBLE,
 *			DOUBLE*DBLCOM, COMPLEX, DBLCOM, INTPYR, FLOATPYR
 *
 * Michael Landy - 7/3/91
 */

#include <hipl_format.h>

int h_mul(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	switch(hdi1->pixel_format) {
	case PFBYTE:	return(h_mul_b(hdi1,hdi2,hdo));
	case PFSHORT:	return(h_mul_s(hdi1,hdi2,hdo));
	case PFINT:	return(h_mul_i(hdi1,hdi2,hdo));
	case PFFLOAT:	if (hdi2->pixel_format == PFFLOAT)
				return(h_mul_f(hdi1,hdi2,hdo));
			else if (hdi2->pixel_format == PFCOMPLEX)
				return(h_mul_fc(hdi1,hdi2,hdo));
			else
				return(perr(HE_FMT3SUBR,"h_mul",
					hformatname(hdi1->pixel_format),
					hformatname(hdi2->pixel_format),
					hformatname(hdo->pixel_format)));
	case PFDOUBLE:	if (hdi2->pixel_format == PFDOUBLE)
				return(h_mul_d(hdi1,hdi2,hdo));
			else if (hdi2->pixel_format == PFDBLCOM)
				return(h_mul_ddc(hdi1,hdi2,hdo));
			else
				return(perr(HE_FMT3SUBR,"h_mul",
					hformatname(hdi1->pixel_format),
					hformatname(hdi2->pixel_format),
					hformatname(hdo->pixel_format)));
	case PFCOMPLEX:	if (hdi2->pixel_format == PFFLOAT)
				return(h_mul_fc(hdi2,hdi1,hdo));
			else if (hdi2->pixel_format == PFCOMPLEX)
				return(h_mul_c(hdi1,hdi2,hdo));
			else
				return(perr(HE_FMT3SUBR,"h_mul",
					hformatname(hdi1->pixel_format),
					hformatname(hdi2->pixel_format),
					hformatname(hdo->pixel_format)));
	case PFDBLCOM:	if (hdi2->pixel_format == PFDOUBLE)
				return(h_mul_ddc(hdi2,hdi1,hdo));
			else if (hdi2->pixel_format == PFDBLCOM)
				return(h_mul_dc(hdi1,hdi2,hdo));
			else
				return(perr(HE_FMT3SUBR,"h_mul",
					hformatname(hdi1->pixel_format),
					hformatname(hdi2->pixel_format),
					hformatname(hdo->pixel_format)));
	case PFINTPYR:	return(h_mul_ip(hdi1,hdi2,hdo));
	case PFFLOATPYR:return(h_mul_fp(hdi1,hdi2,hdo));
	default:	return(perr(HE_FMTSUBR,"h_mul",
				hformatname(hdi1->pixel_format)));
	}
}

int h_mul_b(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_mul_B(hdi1->firstpix,hdi2->firstpix,hdo->firstpix,hdi1->rows,
		hdi1->cols,hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_mul_s(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_mul_S((short *) hdi1->firstpix,(short *) hdi2->firstpix,
		(short *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_mul_i(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_mul_I((int *) hdi1->firstpix,(int *) hdi2->firstpix,
		(int *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_mul_f(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_mul_F((float *) hdi1->firstpix,(float *) hdi2->firstpix,
		(float *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_mul_fc(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_mul_FC((float *) hdi1->firstpix,(float *) hdi2->firstpix,
		(float *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_mul_d(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_mul_D((double *) hdi1->firstpix,(double *) hdi2->firstpix,
		(double *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_mul_ddc(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_mul_DDC((double *) hdi1->firstpix,(double *) hdi2->firstpix,
		(double *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_mul_c(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_mul_C((float *) hdi1->firstpix,(float *) hdi2->firstpix,
		(float *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_mul_dc(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_mul_DC((double *) hdi1->firstpix,(double *) hdi2->firstpix,
		(double *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_mul_ip(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_mul_I((int *) hdi1->image,(int *) hdi2->image,
		(int *) hdo->image,1,hdi1->numpix,
		hdi1->numpix,hdi1->numpix,hdi1->numpix));
}

int h_mul_fp(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_mul_F((float *) hdi1->image,(float *) hdi2->image,
		(float *) hdo->image,1,hdi1->numpix,
		hdi1->numpix,hdi1->numpix,hdi1->numpix));
}

int h_mul_B(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

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
		for (j=0;j<nc;j++)
			*po++ = *pi1++ * *pi2++;
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_mul_S(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

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
			*po++ = *pi1++ * *pi2++;
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_mul_I(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

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
			*po++ = *pi1++ * *pi2++;
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_mul_F(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

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
			*po++ = *pi1++ * *pi2++;
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_mul_FC(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

float *imagei1,*imagei2,*imageo;
int nr,nc,nlpi1,nlpi2,nlpo;

{
	register int i,j,nexi1,nexi2,nexo;
	register float *pi1,*pi2,*po;

	nexi1 = nlpi1-nc;
	nexi2 = 2*(nlpi2-nc);
	nexo = 2*(nlpo-nc);
	pi1 = imagei1;
	pi2 = imagei2;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			*po++ = *pi1 * *pi2++;
			*po++ = *pi1++ * *pi2++;
		}
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_mul_D(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

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
			*po++ = *pi1++ * *pi2++;
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_mul_DDC(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

double *imagei1,*imagei2,*imageo;
int nr,nc,nlpi1,nlpi2,nlpo;

{
	register int i,j,nexi1,nexi2,nexo;
	register double *pi1,*pi2,*po;

	nexi1 = nlpi1-nc;
	nexi2 = 2*(nlpi2-nc);
	nexo = 2*(nlpo-nc);
	pi1 = imagei1;
	pi2 = imagei2;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			*po++ = *pi1 * *pi2++;
			*po++ = *pi1++ * *pi2++;
		}
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_mul_C(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

float *imagei1,*imagei2,*imageo;
int nr,nc,nlpi1,nlpi2,nlpo;

{
	register int i,j,nexi1,nexi2,nexo;
	register float *pi1,*pi2,*po,t1,t2;

	nexi1 = 2*(nlpi1-nc);
	nexi2 = 2*(nlpi2-nc);
	nexo = 2*(nlpo-nc);
	pi1 = imagei1;
	pi2 = imagei2;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			t1 = (pi1[0] * pi2[0]) - (pi1[1] * pi2[1]);
			t2 = (pi1[0] * pi2[1]) + (pi1[1] * pi2[0]);
			*po++ = t1;
			*po++ = t2;
			pi1 += 2;
			pi2 += 2;
		}
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_mul_DC(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

double *imagei1,*imagei2,*imageo;
int nr,nc,nlpi1,nlpi2,nlpo;

{
	register int i,j,nexi1,nexi2,nexo;
	register double *pi1,*pi2,*po,t1,t2;

	nexi1 = 2*(nlpi1-nc);
	nexi2 = 2*(nlpi2-nc);
	nexo = 2*(nlpo-nc);
	pi1 = imagei1;
	pi2 = imagei2;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			t1 = (pi1[0] * pi2[0]) - (pi1[1] * pi2[1]);
			t2 = (pi1[0] * pi2[1]) + (pi1[1] * pi2[0]);
			*po++ = t1;
			*po++ = t2;
			pi1 += 2;
			pi2 += 2;
		}
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}
