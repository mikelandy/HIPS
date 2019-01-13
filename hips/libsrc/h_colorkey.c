/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_colorkey.c - color keying
 *
 * H_colorkey performs the equivalent of a color keying operation.  All images
 * should be the same size.  For each pixel, the control image pixel is used
 * to choose which input image pixel will be used in the output image.  A
 * control value of 1 chooses the first input image (note that this is
 * different from the usual C convention), 2 chooses the 2nd, and so on.  If
 * bflag is TRUE, control pixel values which correspond to no input image
 * (i.e. values less than 1 or greater than nimage) result in an output pixel
 * value of hips_lchar.  If bflag is FALSE, control pixel values less than 1
 * choose the first image, and control pixel values greater than nimage choose
 * the last image.
 *
 * pixel formats: BYTE control -> BYTE image,
 *		  BYTE control -> INT image,
 *		  BYTE control -> FLOAT image,
 *		  INT control -> BYTE image,
 *		  INT control -> INT image,
 *		  INT control -> FLOAT image,
 *		  INTPYR control -> INTPYR image,
 *		  INTPYR control -> FLOATPYR image,
 *
 * Mike Landy - 3/11/89
 * Hips 2 - msl - 8/13/91
 */

#include <hipl_format.h>

static int savenimage = -1;
static byte **Iimage;
static int *Inlp;
static int saven_b = -1;
static byte **pib;
static int saven_i = -1;
static int **pii;
static int saven_f = -1;
static float **pif;
int h_colorkargs(),h_colorkargsp();

int h_colorkey(hdc,nimage,hdi,hdo,bflag)

struct header *hdc,**hdi,*hdo;
int nimage;
h_boolean bflag;

{
	switch(hdc->pixel_format) {
	case PFBYTE:	if (hdi[0]->pixel_format == PFBYTE)
				return(h_colorkey_bb(hdc,nimage,hdi,hdo,
					bflag));
			else if (hdi[0]->pixel_format == PFINT)
				return(h_colorkey_bi(hdc,nimage,hdi,hdo,
					bflag));
			else if (hdi[0]->pixel_format == PFFLOAT)
				return(h_colorkey_bf(hdc,nimage,hdi,hdo,
					bflag));
			else
				return(perr(HE_FMT2SUBR,"h_colorkey",
					hformatname(hdc->pixel_format),
					hformatname(hdi[0]->pixel_format)));
	case PFINT:	if (hdi[0]->pixel_format == PFBYTE)
				return(h_colorkey_ib(hdc,nimage,hdi,hdo,
					bflag));
			else if (hdi[0]->pixel_format == PFINT)
				return(h_colorkey_ii(hdc,nimage,hdi,hdo,
					bflag));
			else if (hdi[0]->pixel_format == PFFLOAT)
				return(h_colorkey_if(hdc,nimage,hdi,hdo,
					bflag));
			else
				return(perr(HE_FMT2SUBR,"h_colorkey",
					hformatname(hdc->pixel_format),
					hformatname(hdi[0]->pixel_format)));
	case PFINTPYR:	if (hdi[0]->pixel_format == PFINTPYR)
				return(h_colorkey_ipip(hdc,nimage,hdi,hdo,
					bflag));
			else if (hdi[0]->pixel_format == PFFLOATPYR)
				return(h_colorkey_ipfp(hdc,nimage,hdi,hdo,
					bflag));
			else
				return(perr(HE_FMT2SUBR,"h_colorkey",
					hformatname(hdc->pixel_format),
					hformatname(hdi[0]->pixel_format)));
	default:	return(perr(HE_FMTSUBR,"h_colorkey",
				hformatname(hdc->pixel_format)));
	}
}

int h_colorkey_bb(hdc,nimage,hdi,hdo,bflag)

struct header *hdc,**hdi,*hdo;
int nimage;
h_boolean bflag;

{
	if (h_colorkargs(nimage,hdi) == HIPS_ERROR)
		return(HIPS_ERROR);
	return(h_colorkey_BB(hdc->firstpix,nimage,Iimage,hdo->firstpix,
		hdc->rows,hdc->cols,hdc->ocols,Inlp,hdo->ocols,bflag));
}

int h_colorkey_bi(hdc,nimage,hdi,hdo,bflag)

struct header *hdc,**hdi,*hdo;
int nimage;
h_boolean bflag;

{
	if (h_colorkargs(nimage,hdi) == HIPS_ERROR)
		return(HIPS_ERROR);
	return(h_colorkey_BI(hdc->firstpix,nimage,(int **) Iimage,
		(int *) hdo->firstpix,hdc->rows,hdc->cols,hdc->ocols,
		Inlp,hdo->ocols,bflag));
}

int h_colorkey_bf(hdc,nimage,hdi,hdo,bflag)

struct header *hdc,**hdi,*hdo;
int nimage;
h_boolean bflag;

{
	if (h_colorkargs(nimage,hdi) == HIPS_ERROR)
		return(HIPS_ERROR);
	return(h_colorkey_BF(hdc->firstpix,nimage,(float **) Iimage,
		(float *) hdo->firstpix,hdc->rows,hdc->cols,hdc->ocols,
		Inlp,hdo->ocols,bflag));
}

int h_colorkey_ib(hdc,nimage,hdi,hdo,bflag)

struct header *hdc,**hdi,*hdo;
int nimage;
h_boolean bflag;

{
	if (h_colorkargs(nimage,hdi) == HIPS_ERROR)
		return(HIPS_ERROR);
	return(h_colorkey_IB((int *) hdc->firstpix,nimage,Iimage,hdo->firstpix,
		hdc->rows,hdc->cols,hdc->ocols,Inlp,hdo->ocols,bflag));
}

int h_colorkey_ii(hdc,nimage,hdi,hdo,bflag)

struct header *hdc,**hdi,*hdo;
int nimage;
h_boolean bflag;

{
	if (h_colorkargs(nimage,hdi) == HIPS_ERROR)
		return(HIPS_ERROR);
	return(h_colorkey_II((int *) hdc->firstpix,nimage,(int **) Iimage,
		(int *) hdo->firstpix,hdc->rows,hdc->cols,hdc->ocols,
		Inlp,hdo->ocols,bflag));
}

int h_colorkey_if(hdc,nimage,hdi,hdo,bflag)

struct header *hdc,**hdi,*hdo;
int nimage;
h_boolean bflag;

{
	if (h_colorkargs(nimage,hdi) == HIPS_ERROR)
		return(HIPS_ERROR);
	return(h_colorkey_IF((int *) hdc->firstpix,nimage,(float **) Iimage,
		(float *) hdo->firstpix,hdc->rows,hdc->cols,hdc->ocols,
		Inlp,hdo->ocols,bflag));
}

int h_colorkey_ipip(hdc,nimage,hdi,hdo,bflag)

struct header *hdc,**hdi,*hdo;
int nimage;
h_boolean bflag;

{
	if (h_colorkargsp(nimage,hdi) == HIPS_ERROR)
		return(HIPS_ERROR);
	return(h_colorkey_II((int *) hdc->image,nimage,(int **) Iimage,
		(int *) hdo->image,1,hdc->numpix,hdc->numpix,Inlp,hdc->numpix,
		bflag));
}

int h_colorkey_ipfp(hdc,nimage,hdi,hdo,bflag)

struct header *hdc,**hdi,*hdo;
int nimage;
h_boolean bflag;

{
	if (h_colorkargsp(nimage,hdi) == HIPS_ERROR)
		return(HIPS_ERROR);
	return(h_colorkey_IF((int *) hdc->image,nimage,(float **) Iimage,
		(float *) hdo->image,1,hdc->numpix,hdc->numpix,Inlp,
		hdc->numpix,bflag));
}

int h_colorkey_BB(imagec,nimage,imagei,imageo,nr,nc,nlpc,nlpi,nlpo,bflag)

byte *imagec,**imagei,*imageo;
int nimage,nr,nc,nlpc,*nlpi,nlpo;
h_boolean bflag;

{
	byte *pc,*po;
	int i,j,k,nexc,nexo,cval;

	if (saven_b < nimage) {
		if (saven_b > 0)
			free(pib);
		if ((pib = (byte **) memalloc(nimage,sizeof(byte *))) ==
			(byte **) HIPS_ERROR)
				return(HIPS_ERROR);
		saven_b = nimage;
	}
	pc = imagec;
	for (k=0;k<nimage;k++)
		pib[k] = imagei[k];
	po = imageo;
	nexc = nlpc - nc;
	nexo = nlpo - nc;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			cval = *pc++;
			if (cval < 1) {
				if (bflag)
					*po++ = hips_lchar;
				else
					*po++ = *(pib[0]);
			}
			else if (cval > nimage) {
				if (bflag)
					*po++ = hips_lchar;
				else
					*po++ = *(pib[nimage-1]);
			}
			else
				*po++ = *(pib[cval-1]);
			for (k=0;k<nimage;k++)
				pib[k]++;
		}
		pc += nexc;
		po += nexo;
		for (k=0;k<nimage;k++)
			pib[k] += nlpi[k] - nc;
	}
	return(HIPS_OK);
}

int h_colorkey_BI(imagec,nimage,imagei,imageo,nr,nc,nlpc,nlpi,nlpo,bflag)

byte *imagec;
int **imagei,*imageo,nimage,nr,nc,nlpc,*nlpi,nlpo;
h_boolean bflag;

{
	byte *pc;
	int *po,i,j,k,nexc,nexo,cval;

	if (saven_i < nimage) {
		if (saven_i > 0)
			free(pii);
		if ((pii = (int **) memalloc(nimage,sizeof(int *))) ==
			(int **) HIPS_ERROR)
				return(HIPS_ERROR);
		saven_i = nimage;
	}
	pc = imagec;
	for (k=0;k<nimage;k++)
		pii[k] = imagei[k];
	po = imageo;
	nexc = nlpc - nc;
	nexo = nlpo - nc;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			cval = *pc++;
			if (cval < 1) {
				if (bflag)
					*po++ = hips_lchar;
				else
					*po++ = *(pii[0]);
			}
			else if (cval > nimage) {
				if (bflag)
					*po++ = hips_lchar;
				else
					*po++ = *(pii[nimage-1]);
			}
			else
				*po++ = *(pii[cval-1]);
			for (k=0;k<nimage;k++)
				pii[k]++;
		}
		pc += nexc;
		for (k=0;k<nimage;k++)
			pii[k] += nlpi[k] - nc;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_colorkey_BF(imagec,nimage,imagei,imageo,nr,nc,nlpc,nlpi,nlpo,bflag)

byte *imagec;
float **imagei,*imageo;
int nimage,nr,nc,nlpc,*nlpi,nlpo;
h_boolean bflag;

{
	byte *pc;
	float *po;
	int i,j,k,nexc,nexo,cval;

	if (saven_f < nimage) {
		if (saven_f > 0)
			free(pif);
		if ((pif = (float **) memalloc(nimage,sizeof(float *))) ==
			(float **) HIPS_ERROR)
				return(HIPS_ERROR);
		saven_f = nimage;
	}
	pc = imagec;
	for (k=0;k<nimage;k++)
		pif[k] = imagei[k];
	po = imageo;
	nexc = nlpc - nc;
	nexo = nlpo - nc;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			cval = *pc++;
			if (cval < 1) {
				if (bflag)
					*po++ = hips_lchar;
				else
					*po++ = *(pif[0]);
			}
			else if (cval > nimage) {
				if (bflag)
					*po++ = hips_lchar;
				else
					*po++ = *(pif[nimage-1]);
			}
			else
				*po++ = *(pif[cval-1]);
			for (k=0;k<nimage;k++)
				pif[k]++;
		}
		pc += nexc;
		for (k=0;k<nimage;k++)
			pif[k] += nlpi[k] - nc;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_colorkey_IB(imagec,nimage,imagei,imageo,nr,nc,nlpc,nlpi,nlpo,bflag)

int *imagec;
byte **imagei,*imageo;
int nimage,nr,nc,nlpc,*nlpi,nlpo;
h_boolean bflag;

{
	byte *po;
	int *pc,i,j,k,nexc,nexo,cval;

	if (saven_b < nimage) {
		if (saven_b > 0)
			free(pib);
		if ((pib = (byte **) memalloc(nimage,sizeof(byte *))) ==
			(byte **) HIPS_ERROR)
				return(HIPS_ERROR);
		saven_b = nimage;
	}
	pc = imagec;
	for (k=0;k<nimage;k++)
		pib[k] = imagei[k];
	po = imageo;
	nexc = nlpc - nc;
	nexo = nlpo - nc;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			cval = *pc++;
			if (cval < 1) {
				if (bflag)
					*po++ = hips_lchar;
				else
					*po++ = *(pib[0]);
			}
			else if (cval > nimage) {
				if (bflag)
					*po++ = hips_lchar;
				else
					*po++ = *(pib[nimage-1]);
			}
			else
				*po++ = *(pib[cval-1]);
			for (k=0;k<nimage;k++)
				pib[k]++;
		}
		pc += nexc;
		po += nexo;
		for (k=0;k<nimage;k++)
			pib[k] += nlpi[k] - nc;
	}
	return(HIPS_OK);
}

int h_colorkey_II(imagec,nimage,imagei,imageo,nr,nc,nlpc,nlpi,nlpo,bflag)

int *imagec,**imagei,*imageo,nimage,nr,nc,nlpc,*nlpi,nlpo;
h_boolean bflag;

{
	int *pc,*po,i,j,k,nexc,nexo,cval;

	if (saven_i < nimage) {
		if (saven_i > 0)
			free(pii);
		if ((pii = (int **) memalloc(nimage,sizeof(int *))) ==
			(int **) HIPS_ERROR)
				return(HIPS_ERROR);
		saven_i = nimage;
	}
	pc = imagec;
	for (k=0;k<nimage;k++)
		pii[k] = imagei[k];
	po = imageo;
	nexc = nlpc - nc;
	nexo = nlpo - nc;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			cval = *pc++;
			if (cval < 1) {
				if (bflag)
					*po++ = hips_lchar;
				else
					*po++ = *(pii[0]);
			}
			else if (cval > nimage) {
				if (bflag)
					*po++ = hips_lchar;
				else
					*po++ = *(pii[nimage-1]);
			}
			else
				*po++ = *(pii[cval-1]);
			for (k=0;k<nimage;k++)
				pii[k]++;
		}
		pc += nexc;
		for (k=0;k<nimage;k++)
			pii[k] += nlpi[k] - nc;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_colorkey_IF(imagec,nimage,imagei,imageo,nr,nc,nlpc,nlpi,nlpo,bflag)

int *imagec;
float **imagei,*imageo;
int nimage,nr,nc,nlpc,*nlpi,nlpo;
h_boolean bflag;

{
	int *pc;
	float *po;
	int i,j,k,nexc,nexo,cval;

	if (saven_f < nimage) {
		if (saven_f > 0)
			free(pif);
		if ((pif = (float **) memalloc(nimage,sizeof(float *))) ==
			(float **) HIPS_ERROR)
				return(HIPS_ERROR);
		saven_f = nimage;
	}
	pc = imagec;
	for (k=0;k<nimage;k++)
		pif[k] = imagei[k];
	po = imageo;
	nexc = nlpc - nc;
	nexo = nlpo - nc;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			cval = *pc++;
			if (cval < 1) {
				if (bflag)
					*po++ = hips_lchar;
				else
					*po++ = *(pif[0]);
			}
			else if (cval > nimage) {
				if (bflag)
					*po++ = hips_lchar;
				else
					*po++ = *(pif[nimage-1]);
			}
			else
				*po++ = *(pif[cval-1]);
			for (k=0;k<nimage;k++)
				pif[k]++;
		}
		pc += nexc;
		for (k=0;k<nimage;k++)
			pif[k] += nlpi[k] - nc;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_colorkargs(nimage,hdi)

int nimage;
struct header **hdi;

{
	int i;

	if (savenimage < nimage) {
		if (savenimage > 0) {
			free(Iimage);
			free(Inlp);
		}
		if ((Iimage = (byte **) memalloc(nimage,sizeof(int *))) ==
			(byte **) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((Inlp = (int *) memalloc(nimage,sizeof(int))) ==
			(int *) HIPS_ERROR)
				return(HIPS_ERROR);
		savenimage = nimage;
	}
	for (i=0;i<nimage;i++) {
		Iimage[i] = hdi[i]->firstpix;
		Inlp[i] = hdi[i]->ocols;
	}
	return(HIPS_OK);
}

int h_colorkargsp(nimage,hdi)

int nimage;
struct header **hdi;

{
	int i;

	if (savenimage < nimage) {
		if (savenimage > 0) {
			free(Iimage);
			free(Inlp);
		}
		if ((Iimage = (byte **) memalloc(nimage,sizeof(int *))) ==
			(byte **) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((Inlp = (int *) memalloc(nimage,sizeof(int))) ==
			(int *) HIPS_ERROR)
				return(HIPS_ERROR);
		savenimage = nimage;
	}
	for (i=0;i<nimage;i++) {
		Iimage[i] = hdi[i]->image;
		Inlp[i] = hdi[i]->numpix;
	}
	return(HIPS_OK);
}
