/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_and.c - subroutines to compute the logical AND of two images
 *
 * pixel formats: MSBF, LSBF, BYTE
 *
 * Michael Landy - 7/4/91
 */

#include <hipl_format.h>

int h_and(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	switch(hdi1->pixel_format) {
	case PFMSBF:	return(h_and_mp(hdi1,hdi2,hdo));
	case PFLSBF:	return(h_and_lp(hdi1,hdi2,hdo));
	case PFBYTE:	return(h_and_b(hdi1,hdi2,hdo));
	default:	return(perr(HE_FMTSUBR,"h_and",
				hformatname(hdi1->pixel_format)));
	}
}

int h_and_mp(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_and_MP(hdi1->firstpix,hdi2->firstpix,hdo->firstpix,hdi1->rows,
		hdi1->cols,hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_and_lp(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_and_LP(hdi1->firstpix,hdi2->firstpix,hdo->firstpix,hdi1->rows,
		hdi1->cols,hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_and_b(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_and_B(hdi1->firstpix,hdi2->firstpix,hdo->firstpix,hdi1->rows,
		hdi1->cols,hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_and_MP(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

byte *imagei1,*imagei2,*imageo;
int nr,nc,nlpi1,nlpi2,nlpo;

{
	register int i,j,nexi1,nexi2,nexo,ncb;
	register byte *pi1,*pi2,*po;
	byte omask,nmask;

	ncb = (nc+7)/8;
	nexi1 = ((nlpi1+7)/8) - ncb;
	nexi2 = ((nlpi2+7)/8) - ncb;
	nexo = ((nlpo+7)/8) - ncb;
	pi1 = imagei1;
	pi2 = imagei2;
	po = imageo;
	if ((nc % 8) == 0) {
		for (i=0;i<nr;i++) {
			for (j=0;j<ncb;j++)
				*po++ = *pi1++ & *pi2++;
			pi1 += nexi1;
			pi2 += nexi2;
			po += nexo;
		}
	}
	else {
		omask = (0200 >> ((nc%8) - 1)) - 1;
		nmask = ~omask;
		ncb--;
		for (i=0;i<nr;i++) {
			for (j=0;j<ncb;j++)
				*po++ = *pi1++ & *pi2++;
			*po = (*po & omask) | ((*pi1++ & *pi2++) & nmask);
			po++;
			pi1 += nexi1;
			pi2 += nexi2;
			po += nexo;
		}
	}
	return(HIPS_OK);
}

int h_and_LP(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

byte *imagei1,*imagei2,*imageo;
int nr,nc,nlpi1,nlpi2,nlpo;

{
	register int i,j,nexi1,nexi2,nexo,ncb;
	register byte *pi1,*pi2,*po;
	byte omask,nmask;

	ncb = (nc+7)/8;
	nexi1 = ((nlpi1+7)/8) - ncb;
	nexi2 = ((nlpi2+7)/8) - ncb;
	nexo = ((nlpo+7)/8) - ncb;
	pi1 = imagei1;
	pi2 = imagei2;
	po = imageo;
	if ((nc % 8) == 0) {
		for (i=0;i<nr;i++) {
			for (j=0;j<ncb;j++)
				*po++ = *pi1++ & *pi2++;
			pi1 += nexi1;
			pi2 += nexi2;
			po += nexo;
		}
	}
	else {
		nmask = (01 << (nc%8)) - 1;
		omask = ~nmask;
		ncb--;
		for (i=0;i<nr;i++) {
			for (j=0;j<ncb;j++)
				*po++ = *pi1++ & *pi2++;
			*po = (*po & omask) | ((*pi1++ & *pi2++) & nmask);
			po++;
			pi1 += nexi1;
			pi2 += nexi2;
			po += nexo;
		}
	}
	return(HIPS_OK);
}

int h_and_B(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

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
			*po++ = *pi1++ & *pi2++;
		pi1 += nexi1;
		pi2 += nexi2;
		po += nexo;
	}
	return(HIPS_OK);
}
