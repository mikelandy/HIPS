/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_div.c - subroutines to divide two images
 *
 * pixel formats: BYTE, SHORT, INT, FLOAT, FLOAT/COMPLEX, DOUBLE,
 *			DOUBLE/DBLCOM, COMPLEX, COMPLEX/FLOAT, DBLCOM,
 *			DBLCOM/DOUBLE, INTPYR, FLOATPYR
 *
 * Michael Landy - 7/3/91
 */

#include <hipl_format.h>

int h_div(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	switch(hdi1->pixel_format) {
	case PFBYTE:	return(h_div_b(hdi1,hdi2,hdo));
	case PFSHORT:	return(h_div_s(hdi1,hdi2,hdo));
	case PFINT:	return(h_div_i(hdi1,hdi2,hdo));
	case PFFLOAT:	if (hdi2->pixel_format == PFFLOAT)
				return(h_div_f(hdi1,hdi2,hdo));
			else if (hdi2->pixel_format == PFCOMPLEX)
				return(h_div_fc(hdi1,hdi2,hdo));
			else
				return(perr(HE_FMT3SUBR,"h_div",
					hformatname(hdi1->pixel_format),
					hformatname(hdi2->pixel_format),
					hformatname(hdo->pixel_format)));
	case PFDOUBLE:	if (hdi2->pixel_format == PFDOUBLE)
				return(h_div_d(hdi1,hdi2,hdo));
			else if (hdi2->pixel_format == PFDBLCOM)
				return(h_div_ddc(hdi1,hdi2,hdo));
			else
				return(perr(HE_FMT3SUBR,"h_div",
					hformatname(hdi1->pixel_format),
					hformatname(hdi2->pixel_format),
					hformatname(hdo->pixel_format)));
	case PFCOMPLEX:	if (hdi2->pixel_format == PFFLOAT)
				return(h_div_cf(hdi1,hdi2,hdo));
			else if (hdi2->pixel_format == PFCOMPLEX)
				return(h_div_c(hdi1,hdi2,hdo));
			else
				return(perr(HE_FMT3SUBR,"h_div",
					hformatname(hdi1->pixel_format),
					hformatname(hdi2->pixel_format),
					hformatname(hdo->pixel_format)));
	case PFDBLCOM:	if (hdi2->pixel_format == PFDOUBLE)
				return(h_div_dcd(hdi1,hdi2,hdo));
			else if (hdi2->pixel_format == PFDBLCOM)
				return(h_div_dc(hdi1,hdi2,hdo));
			else
				return(perr(HE_FMT3SUBR,"h_div",
					hformatname(hdi1->pixel_format),
					hformatname(hdi2->pixel_format),
					hformatname(hdo->pixel_format)));
	case PFINTPYR:	return(h_div_ip(hdi1,hdi2,hdo));
	case PFFLOATPYR:return(h_div_fp(hdi1,hdi2,hdo));
	default:	return(perr(HE_FMTSUBR,"h_div",
				hformatname(hdi1->pixel_format)));
	}
}

int h_div_b(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_div_B(hdi1->firstpix,hdi2->firstpix,hdo->firstpix,hdi1->rows,
		hdi1->cols,hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_div_s(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_div_S((short *) hdi1->firstpix,(short *) hdi2->firstpix,
		(short *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_div_i(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_div_I((int *) hdi1->firstpix,(int *) hdi2->firstpix,
		(int *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_div_f(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_div_F((float *) hdi1->firstpix,(float *) hdi2->firstpix,
		(float *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_div_fc(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_div_FC((float *) hdi1->firstpix,(float *) hdi2->firstpix,
		(float *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_div_d(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_div_D((double *) hdi1->firstpix,(double *) hdi2->firstpix,
		(double *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_div_ddc(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_div_DDC((double *) hdi1->firstpix,(double *) hdi2->firstpix,
		(double *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_div_c(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_div_C((float *) hdi1->firstpix,(float *) hdi2->firstpix,
		(float *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_div_cf(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_div_CF((float *) hdi1->firstpix,(float *) hdi2->firstpix,
		(float *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_div_dc(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_div_DC((double *) hdi1->firstpix,(double *) hdi2->firstpix,
		(double *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_div_dcd(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_div_DCD((double *) hdi1->firstpix,(double *) hdi2->firstpix,
		(double *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_div_ip(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_div_I((int *) hdi1->image,(int *) hdi2->image,
		(int *) hdo->image,1,hdi1->numpix,
		hdi1->numpix,hdi1->numpix,hdi1->numpix));
}

int h_div_fp(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_div_F((float *) hdi1->image,(float *) hdi2->image,
		(float *) hdo->image,1,hdi1->numpix,
		hdi1->numpix,hdi1->numpix,hdi1->numpix));
}

int h_div_B(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

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
	hips_zdiv = 0;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			if (*pi2 == 0) {
				hips_zdiv++;
				*po++ = 255;
				pi1++; pi2++;
			}
			else
				*po++ = *pi1++ / *pi2++;
		}
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_div_S(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

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
	hips_zdiv = 0;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			if (*pi2 == 0) {
				hips_zdiv++;
				*po++ = 32767;
				pi1++; pi2++;
			}
			else
				*po++ = *pi1++ / *pi2++;
		}
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_div_I(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

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
	hips_zdiv = 0;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			if (*pi2 == 0) {
				hips_zdiv++;
				*po++ = 10000000;
				pi1++; pi2++;
			}
			else
				*po++ = *pi1++ / *pi2++;
		}
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_div_F(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

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
	hips_zdiv = 0;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			if (*pi2 == 0) {
				hips_zdiv++;
				*po++ = 10000000;
				pi1++; pi2++;
			}
			else
				*po++ = *pi1++ / *pi2++;
		}
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_div_FC(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

float *imagei1,*imagei2,*imageo;
int nr,nc,nlpi1,nlpi2,nlpo;

{
	register int i,j,nexi1,nexi2,nexo;
	register float *pi1,*pi2,*po,a,c,d,denom;

	nexi1 = nlpi1-nc;
	nexi2 = 2*(nlpi2-nc);
	nexo = 2*(nlpo-nc);
	pi1 = imagei1;
	pi2 = imagei2;
	po = imageo;
	hips_zdiv = 0;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			a = *pi1++;
			c = *pi2++;
			d = *pi2++;
			denom = c*c + d*d;
			if (denom == 0) {
				hips_zdiv++;
				*po++ = 10000000;
				*po++ = 10000000;
			}
			else {
				*po++ = a*c/denom;
				*po++ = -a*d/denom;
			}
		}
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_div_D(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

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
	hips_zdiv = 0;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			if (*pi2 == 0) {
				hips_zdiv++;
				*po++ = 10000000;
				pi1++; pi2++;
			}
			else
				*po++ = *pi1++ / *pi2++;
		}
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_div_DDC(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

double *imagei1,*imagei2,*imageo;
int nr,nc,nlpi1,nlpi2,nlpo;

{
	register int i,j,nexi1,nexi2,nexo;
	register double *pi1,*pi2,*po,a,c,d,denom;

	nexi1 = nlpi1-nc;
	nexi2 = 2*(nlpi2-nc);
	nexo = 2*(nlpo-nc);
	pi1 = imagei1;
	pi2 = imagei2;
	po = imageo;
	hips_zdiv = 0;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			a = *pi1++;
			c = *pi2++;
			d = *pi2++;
			denom = c*c + d*d;
			if (denom == 0) {
				hips_zdiv++;
				*po++ = 10000000;
				*po++ = 10000000;
			}
			else {
				*po++ = a*c/denom;
				*po++ = -a*d/denom;
			}
		}
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_div_C(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

float *imagei1,*imagei2,*imageo;
int nr,nc,nlpi1,nlpi2,nlpo;

{
	register int i,j,nexi1,nexi2,nexo;
	register float *pi1,*pi2,*po,a,b,c,d,denom;

	nexi1 = 2*(nlpi1-nc);
	nexi2 = 2*(nlpi2-nc);
	nexo = 2*(nlpo-nc);
	pi1 = imagei1;
	pi2 = imagei2;
	po = imageo;
	hips_zdiv = 0;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			a = *pi1++;
			b = *pi1++;
			c = *pi2++;
			d = *pi2++;
			denom = c*c + d*d;
			if (denom == 0) {
				hips_zdiv++;
				*po++ = 10000000;
				*po++ = 10000000;
			}
			else {
				*po++ = (a*c + b*d)/denom;
				*po++ = (b*c - a*d)/denom;
			}
		}
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_div_CF(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

float *imagei1,*imagei2,*imageo;
int nr,nc,nlpi1,nlpi2,nlpo;

{
	register int i,j,nexi1,nexi2,nexo;
	register float *pi1,*pi2,*po;

	nexi1 = 2*(nlpi1-nc);
	nexi2 = nlpi2-nc;
	nexo = 2*(nlpo-nc);
	pi1 = imagei1;
	pi2 = imagei2;
	po = imageo;
	hips_zdiv = 0;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			if (*pi2 == 0) {
				hips_zdiv++;
				*po++ = 10000000;
				*po++ = 10000000;
				pi1 += 2; pi2++;
			}
			else {
				*po++ = *pi1++ / *pi2;
				*po++ = *pi1++ / *pi2++;
			}
		}
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_div_DC(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

double *imagei1,*imagei2,*imageo;
int nr,nc,nlpi1,nlpi2,nlpo;

{
	register int i,j,nexi1,nexi2,nexo;
	register double *pi1,*pi2,*po,a,b,c,d,denom;

	nexi1 = 2*(nlpi1-nc);
	nexi2 = 2*(nlpi2-nc);
	nexo = 2*(nlpo-nc);
	pi1 = imagei1;
	pi2 = imagei2;
	po = imageo;
	hips_zdiv = 0;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			a = *pi1++;
			b = *pi1++;
			c = *pi2++;
			d = *pi2++;
			denom = c*c + d*d;
			if (denom == 0) {
				hips_zdiv++;
				*po++ = 10000000;
				*po++ = 10000000;
			}
			else {
				*po++ = (a*c + b*d)/denom;
				*po++ = (b*c - a*d)/denom;
			}
		}
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_div_DCD(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

double *imagei1,*imagei2,*imageo;
int nr,nc,nlpi1,nlpi2,nlpo;

{
	register int i,j,nexi1,nexi2,nexo;
	register double *pi1,*pi2,*po;

	nexi1 = 2*(nlpi1-nc);
	nexi2 = nlpi2-nc;
	nexo = 2*(nlpo-nc);
	pi1 = imagei1;
	pi2 = imagei2;
	po = imageo;
	hips_zdiv = 0;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			if (*pi2 == 0) {
				hips_zdiv++;
				*po++ = 10000000;
				*po++ = 10000000;
				pi1 += 2; pi2++;
			}
			else {
				*po++ = *pi1++ / *pi2;
				*po++ = *pi1++ / *pi2++;
			}
		}
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}
