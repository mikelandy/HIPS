/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_neg.c - subroutines to take the negative of an image
 *
 * pixel formats: MSBF, LSBF, BYTE, SHORT, INT, FLOAT
 *
 * Michael Landy - 1/6/91
 */

#include <hipl_format.h>

int h_neg(hdi,hdo)

struct header *hdi,*hdo;

{
	switch(hdi->pixel_format) {
	case PFMSBF:	return(h_neg_mp(hdi,hdo));
	case PFLSBF:	return(h_neg_lp(hdi,hdo));
	case PFBYTE:	return(h_neg_b(hdi,hdo));
	case PFSHORT:	return(h_neg_s(hdi,hdo));
	case PFINT:	return(h_neg_i(hdi,hdo));
	case PFFLOAT:	return(h_neg_f(hdi,hdo));
	default:	return(perr(HE_FMTSUBR,"h_neg",
				hformatname(hdi->pixel_format)));
	}
}

int h_neg_mp(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_neg_MP(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
		hdi->ocols,hdo->ocols));
}

int h_neg_lp(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_neg_LP(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
		hdi->ocols,hdo->ocols));
}

int h_neg_b(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_neg_B(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
		hdi->ocols,hdo->ocols));
}

int h_neg_s(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_neg_S((short *) hdi->firstpix,(short *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols));
}

int h_neg_i(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_neg_I((int *) hdi->firstpix,(int *) hdo->firstpix,hdi->rows,
		hdi->cols,hdi->ocols,hdo->ocols));
}

int h_neg_f(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_neg_F((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols));
}

int h_neg_MP(imagei,imageo,nr,nc,nlpi,nlpo)

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
				*po++ = *pi++ ^ 0377;
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
				*po++ = *pi++ ^ 0377;
			*po = (*po & omask) | ((*pi++ ^ 0377) & nmask);
			po++;
			pi += nexi;
			po += nexo;
		}
	}
	return(HIPS_OK);
}

int h_neg_LP(imagei,imageo,nr,nc,nlpi,nlpo)

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
				*po++ = *pi++ ^ 0377;
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
				*po++ = *pi++ ^ 0377;
			*po = (*po & omask) | ((*pi++ ^ 0377) & nmask);
			po++;
			pi += nexi;
			po += nexo;
		}
	}
	return(HIPS_OK);
}

int h_neg_B(imagei,imageo,nr,nc,nlpi,nlpo)

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
			*po++ = *pi++ ^ 0377;
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_neg_S(imagei,imageo,nr,nc,nlpi,nlpo)

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
			*po++ = - *pi++;
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_neg_I(imagei,imageo,nr,nc,nlpi,nlpo)

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
			*po++ = - *pi++;
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_neg_F(imagei,imageo,nr,nc,nlpi,nlpo)

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
			*po++ = - *pi++;
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}
