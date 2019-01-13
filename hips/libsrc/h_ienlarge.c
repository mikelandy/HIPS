/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_ienlarge.c - subroutines to enlarge an image by bilinear interpolation
 *
 * pixel formats: BYTE, INT, FLOAT, COMPLEX
 *
 * Michael Landy - 1/12/91
 */

#include <hipl_format.h>

static int ienl_xf = -1;
static int ienl_yf = -1;
static h_boolean ienl_allocated = FALSE;
static int ienl_scale,*factors;
int ienl_alloc();

int h_ienlarge(hdi,hdo,xf,yf)

struct header *hdi,*hdo;
int xf,yf;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	return(h_ienlarge_b(hdi,hdo,xf,yf));
	case PFINT:	return(h_ienlarge_i(hdi,hdo,xf,yf));
	case PFFLOAT:	return(h_ienlarge_f(hdi,hdo,xf,yf));
	case PFCOMPLEX:	return(h_ienlarge_c(hdi,hdo,xf,yf));
	default:	return(perr(HE_FMTSUBR,"h_ienlarge",
				hformatname(hdi->pixel_format)));
	}
}

int h_ienlarge_b(hdi,hdo,xf,yf)

struct header *hdi,*hdo;
int xf,yf;

{
	return(h_ienlarge_B(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
		hdi->ocols,hdo->ocols,xf,yf));
}

int h_ienlarge_i(hdi,hdo,xf,yf)

struct header *hdi,*hdo;
int xf,yf;

{
	return(h_ienlarge_I((int *) hdi->firstpix,(int *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,xf,yf));
}

int h_ienlarge_f(hdi,hdo,xf,yf)

struct header *hdi,*hdo;
int xf,yf;

{
	return(h_ienlarge_F((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,xf,yf));
}

int h_ienlarge_c(hdi,hdo,xf,yf)

struct header *hdi,*hdo;
int xf,yf;

{
	return(h_ienlarge_C((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,xf,yf));
}

int h_ienlarge_B(imagei,imageo,nr,nc,nlpi,nlpo,xf,yf)

byte *imagei,*imageo;
int nr,nc,nlpi,nlpo,xf,yf;

{
	register int i,ii,j,jj,*fp,oval;
	register byte *pi,*po;
	int val,valr,vald,valrd,nexo,halfscl;

	if (ienl_alloc(xf,yf) == HIPS_ERROR)
		return(HIPS_ERROR);
	pi = imagei;
	po = imageo;
	nexo = nlpo-(nc*xf);
	halfscl = ienl_scale/2;
	for (i=nr;i;i--) {
	    for (ii=0;ii<yf;ii++) {
		for (j=nc;j;j--) {
			val = *pi;
			valr = (j==1) ? val : *(pi+1);
			vald = (i==1) ? val : *(pi+nlpi);
			valrd = (i==1) ? valr :
				((j==1) ? vald :
					*(pi+nlpi+1));
			fp = factors + ii*xf*4;
			for (jj=0;jj<xf;jj++) {
				oval = val * *fp++;
				oval += valr * *fp++;
				oval += vald * *fp++;
				oval += valrd * *fp++;
				*po++ = (oval+halfscl)/ienl_scale;
			}
			pi++;
		}
		pi -= nc;
		po += nexo;
	    }
	    pi += nlpi;
	}
	return(HIPS_OK);
}

int h_ienlarge_I(imagei,imageo,nr,nc,nlpi,nlpo,xf,yf)

int *imagei,*imageo;
int nr,nc,nlpi,nlpo,xf,yf;

{
	register int i,ii,j,jj,*fp,oval;
	register int *pi,*po;
	int val,valr,vald,valrd,nexo,halfscl;

	if (ienl_alloc(xf,yf) == HIPS_ERROR)
		return(HIPS_ERROR);
	pi = imagei;
	po = imageo;
	nexo = nlpo-(nc*xf);
	halfscl = ienl_scale/2;
	for (i=nr;i;i--) {
	    for (ii=0;ii<yf;ii++) {
		for (j=nc;j;j--) {
			val = *pi;
			valr = (j==1) ? val : *(pi+1);
			vald = (i==1) ? val : *(pi+nlpi);
			valrd = (i==1) ? valr :
				((j==1) ? vald :
					*(pi+nlpi+1));
			fp = factors + ii*xf*4;
			for (jj=0;jj<xf;jj++) {
				oval = val * *fp++;
				oval += valr * *fp++;
				oval += vald * *fp++;
				oval += valrd * *fp++;
				*po++ = (oval+halfscl)/ienl_scale;
			}
			pi++;
		}
		pi -= nc;
		po += nexo;
	    }
	    pi += nlpi;
	}
	return(HIPS_OK);
}

int h_ienlarge_F(imagei,imageo,nr,nc,nlpi,nlpo,xf,yf)

float *imagei,*imageo;
int nr,nc,nlpi,nlpo,xf,yf;

{
	register int i,ii,j,jj,*fp;
	register float *pi,*po,oval;
	float val,valr,vald,valrd;
	int nexo;

	if (ienl_alloc(xf,yf) == HIPS_ERROR)
		return(HIPS_ERROR);
	pi = imagei;
	po = imageo;
	nexo = nlpo-(nc*xf);
	for (i=nr;i;i--) {
	    for (ii=0;ii<yf;ii++) {
		for (j=nc;j;j--) {
			val = *pi;
			valr = (j==1) ? val : *(pi+1);
			vald = (i==1) ? val : *(pi+nlpi);
			valrd = (i==1) ? valr :
				((j==1) ? vald :
					*(pi+nlpi+1));
			fp = factors + ii*xf*4;
			for (jj=0;jj<xf;jj++) {
				oval = val * *fp++;
				oval += valr * *fp++;
				oval += vald * *fp++;
				oval += valrd * *fp++;
				*po++ = oval/ienl_scale;
			}
			pi++;
		}
		pi -= nc;
		po += nexo;
	    }
	    pi += nlpi;
	}
	return(HIPS_OK);
}

int h_ienlarge_C(imagei,imageo,nr,nc,nlpi,nlpo,xf,yf)

float *imagei,*imageo;
int nr,nc,nlpi,nlpo,xf,yf;

{
	register int i,ii,j,jj,*fp;
	register float *pi,*po,oval;
	float val[2],valr[2],vald[2],valrd[2];
	int nlpi2,nexo;

	if (ienl_alloc(xf,yf) == HIPS_ERROR)
		return(HIPS_ERROR);
	pi = imagei;
	po = imageo;
	nexo = 2*(nlpo-(nc*xf));
	nlpi2 = nlpi*2;
	for (i=nr;i;i--) {
	    for (ii=0;ii<yf;ii++) {
		for (j=nc;j;j--) {
			val[0] = *pi;
			val[1] = *(pi+1);
			if (j == 1) {
				valr[0] = val[0];
				valr[1] = val[1];
			}
			else {
				valr[0] = *(pi+2);
				valr[1] = *(pi+3);
			}
			if (i == 1) {
				vald[0] = val[0];
				vald[1] = val[1];
			}
			else {
				vald[0] = *(pi+nlpi2);
				vald[1] = *(pi+nlpi2+1);
			}
			if (i == 1) {
				valrd[0] = valr[0];
				valrd[1] = valr[1];
			}
			else if (j == 1) {
				valrd[0] = vald[0];
				valrd[1] = vald[1];
			}
			else {
				valrd[0] = *(pi+nlpi2+2);
				valrd[1] = *(pi+nlpi2+3);
			}
			fp = factors + ii*xf*4;
			for (jj=0;jj<xf;jj++) {
				oval = val[0] * *fp++;
				oval += valr[0] * *fp++;
				oval += vald[0] * *fp++;
				oval += valrd[0] * *fp++;
				*po++ = oval/ienl_scale;
				fp -= 4;
				oval = val[1] * *fp++;
				oval += valr[1] * *fp++;
				oval += vald[1] * *fp++;
				oval += valrd[1] * *fp++;
				*po++ = oval/ienl_scale;
			}
			pi += 2;
		}
		pi -= 2*nc;
		po += nexo;
	    }
	    pi += 2*nlpi;
	}
	return(HIPS_OK);
}

int ienl_alloc(xf,yf)

int xf,yf;

{
	register int i,j,*fp;

	ienl_scale = xf*yf;
	if (ienl_allocated) {
		if (xf != ienl_xf || yf != ienl_yf) {
			free(factors);
			if ((factors = (int *)
				memalloc(4*ienl_scale,sizeof(int)))
				== (int *) HIPS_ERROR)
					return(HIPS_ERROR);
		}
	}
	else {
		ienl_allocated = TRUE;
		if ((factors = (int *) memalloc(4*ienl_scale,sizeof(int)))
			== (int *) HIPS_ERROR)
				return(HIPS_ERROR);
	}
	ienl_xf = xf;
	ienl_yf = yf;
	fp = factors;
	for (i=0;i<yf;i++) {
		for (j=0;j<xf;j++) {
			*fp++ = (xf-j)*(yf-i);
			*fp++ = (j)*(yf-i);
			*fp++ = (xf-j)*(i);
			*fp++ = (j)*(i);
		}
	}
	return(HIPS_OK);
}
