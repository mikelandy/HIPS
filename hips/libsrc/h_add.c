/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_add.c - subroutines to add two images
 *
 * pixel formats: SHORT, INT, FLOAT, DOUBLE, COMPLEX, DBLCOM, INTPYR, FLOATPYR
 *			also:  BYTE+INT->INT, BYTE+SHORT->BYTE
 *
 * Michael Landy - 1/14/91
 */

#include <hipl_format.h>

int h_add(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	switch(hdi1->pixel_format) {
	case PFBYTE:	if (hdi2->pixel_format == PFINT &&
			    hdo->pixel_format == PFINT)
				return(h_add_bii(hdi1,hdi2,hdo));
			else if (hdi2->pixel_format == PFSHORT &&
			    hdo->pixel_format == PFBYTE)
				return(h_add_bsb(hdi1,hdi2,hdo));
			else
				return(perr(HE_FMT3SUBR,"h_add",
					hformatname(hdi1->pixel_format),
					hformatname(hdi2->pixel_format),
					hformatname(hdo->pixel_format)));
	case PFSHORT:	return(h_add_s(hdi1,hdi2,hdo));
	case PFINT:	return(h_add_i(hdi1,hdi2,hdo));
	case PFFLOAT:	return(h_add_f(hdi1,hdi2,hdo));
	case PFDOUBLE:	return(h_add_d(hdi1,hdi2,hdo));
	case PFCOMPLEX:	return(h_add_c(hdi1,hdi2,hdo));
	case PFDBLCOM:	return(h_add_dc(hdi1,hdi2,hdo));
	case PFINTPYR:	return(h_add_ip(hdi1,hdi2,hdo));
	case PFFLOATPYR:return(h_add_fp(hdi1,hdi2,hdo));
	default:	return(perr(HE_FMTSUBR,"h_add",
				hformatname(hdi1->pixel_format)));
	}
}

int h_add_bii(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_add_BII(hdi1->firstpix,(int *) hdi2->firstpix,
		(int *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_add_bsb(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_add_BSB(hdi1->firstpix,(short *) hdi2->firstpix,
		hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_add_s(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_add_S((short *) hdi1->firstpix,(short *) hdi2->firstpix,
		(short *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_add_i(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_add_I((int *) hdi1->firstpix,(int *) hdi2->firstpix,
		(int *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_add_f(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_add_F((float *) hdi1->firstpix,(float *) hdi2->firstpix,
		(float *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_add_d(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_add_D((double *) hdi1->firstpix,(double *) hdi2->firstpix,
		(double *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_add_c(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_add_C((float *) hdi1->firstpix,(float *) hdi2->firstpix,
		(float *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_add_dc(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_add_DC((double *) hdi1->firstpix,(double *) hdi2->firstpix,
		(double *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_add_ip(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_add_I((int *) hdi1->image,(int *) hdi2->image,
		(int *) hdo->image,1,hdi1->numpix,
		hdi1->numpix,hdi1->numpix,hdi1->numpix));
}

int h_add_fp(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_add_F((float *) hdi1->image,(float *) hdi2->image,
		(float *) hdo->image,1,hdi1->numpix,
		hdi1->numpix,hdi1->numpix,hdi1->numpix));
}

int h_add_BII(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

byte *imagei1;
int *imagei2,*imageo;
int nr,nc,nlpi1,nlpi2,nlpo;

{
	register int i,j,nexi1,nexi2,nexo;
	register byte *pi1;
	register int *pi2,*po;

	nexi1 = nlpi1-nc;
	nexi2 = nlpi2-nc;
	nexo = nlpo-nc;
	pi1 = imagei1;
	pi2 = imagei2;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*po++ = *pi1++ + *pi2++;
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_add_BSB(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

byte *imagei1,*imageo;
short *imagei2;
int nr,nc,nlpi1,nlpi2,nlpo;

{
	register int i,j,nexi1,nexi2,nexo,val;
	register byte *pi1,*po;
	register short *pi2;

	nexi1 = nlpi1-nc;
	nexi2 = nlpi2-nc;
	nexo = nlpo-nc;
	pi1 = imagei1;
	pi2 = imagei2;
	po = imageo;
	hips_lclip = hips_hclip = 0;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			val = *pi1++ + *pi2++;
			if (val < 0) {
				*po++ = 0;
				hips_lclip++;
			}
			else if (val > 255) {
				*po++ = 255;
				hips_hclip++;
			}
			else
				*po++ = val;
		}
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_add_S(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

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
			*po++ = *pi1++ + *pi2++;
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_add_I(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

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
			*po++ = *pi1++ + *pi2++;
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_add_F(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

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
			*po++ = *pi1++ + *pi2++;
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_add_D(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

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
			*po++ = *pi1++ + *pi2++;
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_add_C(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

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
			*po++ = *pi1++ + *pi2++;
			*po++ = *pi1++ + *pi2++;
		}
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_add_DC(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

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
			*po++ = *pi1++ + *pi2++;
			*po++ = *pi1++ + *pi2++;
		}
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}
