/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_shuffleadd.c - subroutines to shuffle one image and add it to another
 *
 * The rows and columns of the second image are permuted randomly and added
 * to the first image.
 *
 * pixel formats: FLOAT, BYTE+SHORT->BYTE
 *
 * Yoav Cohen 3/15/82
 * modified for float images: Mike Landy 10/19/83
 * HIPS 2 - msl - 8/5/91
 */

#include <hipl_format.h>

int shuffle();

int h_shuffleadd(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	switch(hdi1->pixel_format) {
	case PFBYTE:	if (hdi2->pixel_format == PFSHORT &&
			    hdo->pixel_format == PFBYTE)
				return(h_shuffleadd_bsb(hdi1,hdi2,hdo));
			else
				return(perr(HE_FMT3SUBR,"h_shuffleadd",
					hformatname(hdi1->pixel_format),
					hformatname(hdi2->pixel_format),
					hformatname(hdo->pixel_format)));
	case PFFLOAT:	return(h_shuffleadd_f(hdi1,hdi2,hdo));
	default:	return(perr(HE_FMTSUBR,"h_shuffleadd",
				hformatname(hdi1->pixel_format)));
	}
}

int h_shuffleadd_bsb(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_shuffleadd_BSB(hdi1->firstpix,(short *) hdi2->firstpix,
		hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

int h_shuffleadd_f(hdi1,hdi2,hdo)

struct header *hdi1,*hdi2,*hdo;

{
	return(h_shuffleadd_F((float *) hdi1->firstpix,(float *) hdi2->firstpix,
		(float *) hdo->firstpix,hdi1->rows,hdi1->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols));
}

static int savenr = -1;
static int savenc = -1;
static int *pr,*pc;

int h_shuffleadd_BSB(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

byte *imagei1,*imageo;
short *imagei2;
int nr,nc,nlpi1,nlpi2,nlpo;

{
	register int i,j,nexi1,nexo,val,*pt;
	register byte *pi1,*po;
	register short *pi2;

	if (savenr != nr) {
		if (savenr > 0)
			free(pr);
		if ((pr = (int *) memalloc(nr,sizeof(int))) ==
			(int *) HIPS_ERROR)
				return(HIPS_ERROR);
		pt=pr;
		for (i=0;i<nr;i++)
			*pt++ = i;
		savenr = nr;
	}
	if (savenc != nc) {
		if (savenc > 0)
			free(pc);
		if ((pc = (int *) memalloc(nc,sizeof(int))) ==
			(int *) HIPS_ERROR)
				return(HIPS_ERROR);
		pt=pc;
		for (i=0;i<nc;i++)
			*pt++ = i;
		savenc = nc;
	}
	shuffle(pr,nr);
	shuffle(pc,nc);
	nexi1 = nlpi1-nc;
	nexo = nlpo-nc;
	pi1 = imagei1;
	po = imageo;
	hips_lclip = hips_hclip = 0;
	for (i=0;i<nr;i++) {
		pi2 = imagei2 + (*(pr+i))*nlpi2;
		for (j=0;j<nc;j++) {
			val = *pi1++ + *(pi2 + *(pc+j));
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
		po += nexo;
	}
	return(HIPS_OK);
}

int h_shuffleadd_F(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo)

float *imagei1,*imagei2,*imageo;
int nr,nc,nlpi1,nlpi2,nlpo;

{
	register int i,j,nexi1,nexo,*pt;
	register float *pi1,*pi2,*po;

	if (savenr != nr) {
		if (savenr > 0)
			free(pr);
		if ((pr = (int *) memalloc(nr,sizeof(int))) ==
			(int *) HIPS_ERROR)
				return(HIPS_ERROR);
		pt=pr;
		for (i=0;i<nr;i++)
			*pt++ = i;
		savenr = nr;
	}
	if (savenc != nc) {
		if (savenc > 0)
			free(pc);
		if ((pc = (int *) memalloc(nc,sizeof(int))) ==
			(int *) HIPS_ERROR)
				return(HIPS_ERROR);
		pt=pc;
		for (i=0;i<nc;i++)
			*pt++ = i;
		savenc = nc;
	}
	shuffle(pr,nr);
	shuffle(pc,nc);
	nexi1 = nlpi1-nc;
	nexo = nlpo-nc;
	pi1 = imagei1;
	po = imageo;
	hips_lclip = hips_hclip = 0;
	for (i=0;i<nr;i++) {
		pi2 = imagei2 + (*(pr+i))*nlpi2;
		for (j=0;j<nc;j++)
			*po++ = *pi1++ + *(pi2 + *(pc+j));
		pi1 += nexi1;
		po += nexo;
	}
	return(HIPS_OK);
}

int shuffle(p,n)

int *p;
int n;

{
	int nm1,i,k,m,t;
	
	nm1=n-1;
	for(i=0,m=n;i<nm1;i++,m--) {
		k = H__RANDOM()%m + i;
		t = *(p+k);
		*(p+k) = *(p+i);
		*(p+i) = t;
	}
	return(HIPS_OK);
}
