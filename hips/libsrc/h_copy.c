/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_copy.c - subroutines to copy an image
 *
 * pixel formats: MSBF, LSBF, SBYTE, BYTE, USHORT, SHORT, UINT, INT, FLOAT,
 *			DOUBLE, COMPLEX, DBLCOM
 *
 * Michael Landy - 1/10/91
 */

#include <hipl_format.h>

int h_copy(hdi,hdo)

struct header *hdi,*hdo;

{
	switch(hdi->pixel_format) {
	case PFMSBF:	return(h_copy_mp(hdi,hdo));
	case PFLSBF:	return(h_copy_lp(hdi,hdo));
	case PFSBYTE:
	case PFBYTE:	return(h_copy_b(hdi,hdo));
	case PFUSHORT:
	case PFSHORT:	return(h_copy_s(hdi,hdo));
	case PFUINT:
	case PFINT:	return(h_copy_i(hdi,hdo));
	case PFFLOAT:	return(h_copy_f(hdi,hdo));
	case PFDOUBLE:	return(h_copy_d(hdi,hdo));
	case PFCOMPLEX:	return(h_copy_c(hdi,hdo));
	case PFDBLCOM:	return(h_copy_dc(hdi,hdo));
	default:	return(perr(HE_FMTSUBR,"h_copy",
				hformatname(hdi->pixel_format)));
	}
}

int h_copy_mp(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_copy_MP(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
		hdi->ocols,hdo->ocols));
}

int h_copy_lp(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_copy_LP(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
		hdi->ocols,hdo->ocols));
}

int h_copy_b(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_copy_B(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
		hdi->ocols,hdo->ocols));
}

int h_copy_s(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_copy_S((short *) hdi->firstpix,(short *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols));
}

int h_copy_i(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_copy_I((int *) hdi->firstpix,(int *) hdo->firstpix,hdi->rows,
		hdi->cols,hdi->ocols,hdo->ocols));
}

int h_copy_f(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_copy_F((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols));
}

int h_copy_d(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_copy_D((double *) hdi->firstpix,(double *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols));
}

int h_copy_c(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_copy_C((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols));
}

int h_copy_dc(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_copy_DC((double *) hdi->firstpix,(double *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols));
}

int h_copy_MP(imagei,imageo,nr,nc,nlpi,nlpo)

byte *imagei,*imageo;
int nr,nc,nlpi,nlpo;

{
	register int i,j,nexi,nexo,ncb;
	register byte *pi,*po;
	byte omask,nmask;

	ncb = (nc+7)/8;
	nexi = ((nlpi+7)/8) - ncb;
	nexo = ((nlpo+7)/8) - ncb;
	pi = imagei;
	po = imageo;
	if ((nc % 8) == 0) {
		for (i=0;i<nr;i++) {
			for (j=0;j<ncb;j++)
				*po++ = *pi++;
			pi += nexi;
			po += nexo;
		}
	}
	else {
		omask = (0200 >> ((nc%8) - 1)) - 1;
		nmask = ~omask;
		ncb--;
		for (i=0;i<nr;i++) {
			for (j=0;j<ncb;j++)
				*po++ = *pi++;
			*po = (*po & omask) | (*pi++ & nmask);
			po++;
			pi += nexi;
			po += nexo;
		}
	}
	return(HIPS_OK);
}

int h_copy_LP(imagei,imageo,nr,nc,nlpi,nlpo)

byte *imagei,*imageo;
int nr,nc,nlpi,nlpo;

{
	register int i,j,nexi,nexo,ncb;
	register byte *pi,*po;
	byte omask,nmask;

	ncb = (nc+7)/8;
	nexi = ((nlpi+7)/8) - ncb;
	nexo = ((nlpo+7)/8) - ncb;
	pi = imagei;
	po = imageo;
	if ((nc % 8) == 0) {
		for (i=0;i<nr;i++) {
			for (j=0;j<ncb;j++)
				*po++ = *pi++;
			pi += nexi;
			po += nexo;
		}
	}
	else {
		nmask = (01 << (nc%8)) - 1;
		omask = ~nmask;
		ncb--;
		for (i=0;i<nr;i++) {
			for (j=0;j<ncb;j++)
				*po++ = *pi++;
			*po = (*po & omask) | (*pi++ & nmask);
			po++;
			pi += nexi;
			po += nexo;
		}
	}
	return(HIPS_OK);
}

int h_copy_B(imagei,imageo,nr,nc,nlpi,nlpo)

byte *imagei,*imageo;
int nr,nc,nlpi,nlpo;

{
	register int i,j,nexi,nexo;
	register byte *pi,*po;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*po++ = *pi++;
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_copy_S(imagei,imageo,nr,nc,nlpi,nlpo)

short *imagei,*imageo;
int nr,nc,nlpi,nlpo;

{
	register int i,j,nexi,nexo;
	register short *pi,*po;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*po++ = *pi++;
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_copy_I(imagei,imageo,nr,nc,nlpi,nlpo)

int *imagei,*imageo;
int nr,nc,nlpi,nlpo;

{
	register int i,j,nexi,nexo;
	register int *pi,*po;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*po++ = *pi++;
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_copy_F(imagei,imageo,nr,nc,nlpi,nlpo)

float *imagei,*imageo;
int nr,nc,nlpi,nlpo;

{
	register int i,j,nexi,nexo;
	register float *pi,*po;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*po++ = *pi++;
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_copy_D(imagei,imageo,nr,nc,nlpi,nlpo)

double *imagei,*imageo;
int nr,nc,nlpi,nlpo;

{
	register int i,j,nexi,nexo;
	register double *pi,*po;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*po++ = *pi++;
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_copy_C(imagei,imageo,nr,nc,nlpi,nlpo)

float *imagei,*imageo;
int nr,nc,nlpi,nlpo;

{
	register int i,j,nexi,nexo;
	register float *pi,*po;

	nexi = 2*(nlpi-nc);
	nexo = 2*(nlpo-nc);
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			*po++ = *pi++;
			*po++ = *pi++;
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_copy_DC(imagei,imageo,nr,nc,nlpi,nlpo)

double *imagei,*imageo;
int nr,nc,nlpi,nlpo;

{
	register int i,j,nexi,nexo;
	register double *pi,*po;

	nexi = 2*(nlpi-nc);
	nexo = 2*(nlpo-nc);
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			*po++ = *pi++;
			*po++ = *pi++;
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}
