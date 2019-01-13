/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_invert.c - invert a frame (reflect about a horizontal axis)
 *
 * pixel formats: MSBF, LSBF, BYTE, INT, FLOAT
 *
 * Michael Landy - 6/23/91
 */

#include <hipl_format.h>

int h_invert(hdi,hdo)

struct header *hdi,*hdo;
{
	switch(hdi->pixel_format) {
	case PFMSBF:	return(h_invert_mp(hdi,hdo));
	case PFLSBF:	return(h_invert_lp(hdi,hdo));
	case PFBYTE:	return(h_invert_b(hdi,hdo));
	case PFINT:	return(h_invert_i(hdi,hdo));
	case PFFLOAT:	return(h_invert_f(hdi,hdo));
	default:	return(perr(HE_FMTSUBR,"h_invert",
				hformatname(hdi->pixel_format)));
	}
}

int h_invert_lp(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_invert_LP(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
		hdi->ocols,hdo->ocols));
}

int h_invert_mp(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_invert_MP(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
		hdi->ocols,hdo->ocols));
}

int h_invert_b(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_invert_B(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
		hdi->ocols,hdo->ocols));
}

int h_invert_i(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_invert_I((int *) hdi->firstpix,(int *) hdo->firstpix,hdi->rows,
		hdi->cols,hdi->ocols,hdo->ocols));
}

int h_invert_f(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_invert_F((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols));
}

int h_invert_MP(imagei,imageo,nr,nc,nlpi,nlpo)

byte *imagei,*imageo;
int nr,nc,nlpi,nlpo;

{
	register int i,j;
	register byte *pi1,*pi2,*po1,*po2,tmp;
	int ncb,nr2,nexi1,nexi2,nexo1,nexo2,omask,nmask;

	nr2 = nr/2;
	ncb = (nc+7)/8;
	nexi1 = ((nlpi+7)/8) - ncb;
	nexi2 = -(((nlpi+7)/8) + ncb);
	nexo1 = ((nlpo+7)/8) - ncb;
	nexo2 = -(((nlpo+7)/8) + ncb);
	pi1 = imagei;
	pi2 = imagei + (nr-1)*((nlpi+7)/8);
	po1 = imageo;
	po2 = imageo + (nr-1)*((nlpo+7)/8);
	if ((nc % 8) == 0) {
		for (i=0;i<nr2;i++) {
			for (j=0;j<ncb;j++) {
				tmp = *pi1++;
				*po1++ = *pi2++;
				*po2++ = tmp;
			}
			pi1 += nexi1;
			pi2 += nexi2;
			po1 += nexo1;
			po2 += nexo2;
		}
		if (nr & 01) {
			for (j=0;j<ncb;j++)
				*po1++ = *pi2++;
		}
	}
	else {
		omask = (0200 >> ((nc%8) - 1)) - 1;
		nmask = ~omask;
		ncb--;
		for (i=0;i<nr2;i++) {
			for (j=0;j<ncb;j++) {
				tmp = *pi1++;
				*po1++ = *pi2++;
				*po2++ = tmp;
			}
			tmp = *pi1++;
			*po1 = (*po1 & omask) | (*pi2++ & nmask);
			*po2 = (*po2 & omask) | (tmp & nmask);
			po1++; po2++;
			pi1 += nexi1;
			pi2 += nexi2;
			po1 += nexo1;
			po2 += nexo2;
		}
		if (nr & 01) {
			for (j=0;j<ncb;j++)
				*po1++ = *pi2++;
			*po1 = (*po1 & omask) | (*pi2 & nmask);
		}

	}
	return(HIPS_OK);
}

int h_invert_LP(imagei,imageo,nr,nc,nlpi,nlpo)

byte *imagei,*imageo;
int nr,nc,nlpi,nlpo;

{
	register int i,j;
	register byte *pi1,*pi2,*po1,*po2,tmp;
	int ncb,nr2,nexi1,nexi2,nexo1,nexo2,omask,nmask;

	nr2 = nr/2;
	ncb = (nc+7)/8;
	nexi1 = ((nlpi+7)/8) - ncb;
	nexi2 = -(((nlpi+7)/8) + ncb);
	nexo1 = ((nlpo+7)/8) - ncb;
	nexo2 = -(((nlpo+7)/8) + ncb);
	pi1 = imagei;
	pi2 = imagei + (nr-1)*((nlpi+7)/8);
	po1 = imageo;
	po2 = imageo + (nr-1)*((nlpo+7)/8);
	if ((nc % 8) == 0) {
		for (i=0;i<nr2;i++) {
			for (j=0;j<ncb;j++) {
				tmp = *pi1++;
				*po1++ = *pi2++;
				*po2++ = tmp;
			}
			pi1 += nexi1;
			pi2 += nexi2;
			po1 += nexo1;
			po2 += nexo2;
		}
		if (nr & 01) {
			for (j=0;j<ncb;j++)
				*po1++ = *pi2++;
		}
	}
	else {
		nmask = (01 << (nc%8)) - 1;
		omask = ~nmask;
		ncb--;
		for (i=0;i<nr2;i++) {
			for (j=0;j<ncb;j++) {
				tmp = *pi1++;
				*po1++ = *pi2++;
				*po2++ = tmp;
			}
			tmp = *pi1++;
			*po1 = (*po1 & omask) | (*pi2++ & nmask);
			*po2 = (*po2 & omask) | (tmp & nmask);
			po1++; po2++;
			pi1 += nexi1;
			pi2 += nexi2;
			po1 += nexo1;
			po2 += nexo2;
		}
		if (nr & 01) {
			for (j=0;j<ncb;j++)
				*po1++ = *pi2++;
			*po1 = (*po1 & omask) | (*pi2 & nmask);
		}

	}
	return(HIPS_OK);
}

int h_invert_B(imagei,imageo,nr,nc,nlpi,nlpo)

byte *imagei,*imageo;
int nr,nc,nlpi,nlpo;

{
	register int i,j;
	register byte *pi1,*pi2,*po1,*po2,tmp;
	int nr2,nexi1,nexi2,nexo1,nexo2;

	nr2 = nr/2;
	nexi1 = nlpi-nc;
	nexi2 = -(nlpi+nc);
	nexo1 = nlpo-nc;
	nexo2 = -(nlpo+nc);
	pi1 = imagei;
	pi2 = imagei + (nr-1)*nlpi;
	po1 = imageo;
	po2 = imageo + (nr-1)*nlpo;
	for (i=0;i<nr2;i++) {
		for (j=0;j<nc;j++) {
			tmp = *pi1++;
			*po1++ = *pi2++;
			*po2++ = tmp;
		}
		pi1 += nexi1;
		pi2 += nexi2;
		po1 += nexo1;
		po2 += nexo2;
	}
	if (nr & 01) {
		for (j=0;j<nc;j++)
			*po1++ = *pi2++;
	}
	return(HIPS_OK);
}

int h_invert_I(imagei,imageo,nr,nc,nlpi,nlpo)

int *imagei,*imageo;
int nr,nc,nlpi,nlpo;

{
	register int i,j;
	register int *pi1,*pi2,*po1,*po2,tmp;
	int nr2,nexi1,nexi2,nexo1,nexo2;

	nr2 = nr/2;
	nexi1 = nlpi-nc;
	nexi2 = -(nlpi+nc);
	nexo1 = nlpo-nc;
	nexo2 = -(nlpo+nc);
	pi1 = imagei;
	pi2 = imagei + (nr-1)*nlpi;
	po1 = imageo;
	po2 = imageo + (nr-1)*nlpo;
	for (i=0;i<nr2;i++) {
		for (j=0;j<nc;j++) {
			tmp = *pi1++;
			*po1++ = *pi2++;
			*po2++ = tmp;
		}
		pi1 += nexi1;
		pi2 += nexi2;
		po1 += nexo1;
		po2 += nexo2;
	}
	if (nr & 01) {
		for (j=0;j<nc;j++)
			*po1++ = *pi2++;
	}
	return(HIPS_OK);
}

int h_invert_F(imagei,imageo,nr,nc,nlpi,nlpo)

float *imagei,*imageo;
int nr,nc,nlpi,nlpo;

{
	register int i,j;
	register float *pi1,*pi2,*po1,*po2,tmp;
	int nr2,nexi1,nexi2,nexo1,nexo2;

	nr2 = nr/2;
	nexi1 = nlpi-nc;
	nexi2 = -(nlpi+nc);
	nexo1 = nlpo-nc;
	nexo2 = -(nlpo+nc);
	pi1 = imagei;
	pi2 = imagei + (nr-1)*nlpi;
	po1 = imageo;
	po2 = imageo + (nr-1)*nlpo;
	for (i=0;i<nr2;i++) {
		for (j=0;j<nc;j++) {
			tmp = *pi1++;
			*po1++ = *pi2++;
			*po2++ = tmp;
		}
		pi1 += nexi1;
		pi2 += nexi2;
		po1 += nexo1;
		po2 += nexo2;
	}
	if (nr & 01) {
		for (j=0;j<nc;j++)
			*po1++ = *pi2++;
	}
	return(HIPS_OK);
}
