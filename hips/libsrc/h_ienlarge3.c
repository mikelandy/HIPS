/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_ienlarge3.c - subroutines to enlarge an image by trilinear interpolation
 *
 * pixel formats: BYTE, INT, FLOAT, COMPLEX
 *
 * Michael Landy - 1/12/91
 */

#include <hipl_format.h>

static int ienl3_xf = -1;
static int ienl3_yf = -1;
static int ienl3_tf = -1;
static h_boolean ienl3_allocated = FALSE;
static int ienl3_scale,*factors;
int ienl3_alloc();

int h_ienlarge3(hdi1,hdi2,hdo,xf,yf,tf,t)

struct header *hdi1,*hdi2,*hdo;
int xf,yf,tf,t;

{
	switch(hdi1->pixel_format) {
	case PFBYTE:	return(h_ienlarge3_b(hdi1,hdi2,hdo,xf,yf,tf,t));
	case PFINT:	return(h_ienlarge3_i(hdi1,hdi2,hdo,xf,yf,tf,t));
	case PFFLOAT:	return(h_ienlarge3_f(hdi1,hdi2,hdo,xf,yf,tf,t));
	case PFCOMPLEX:	return(h_ienlarge3_c(hdi1,hdi2,hdo,xf,yf,tf,t));
	default:	return(perr(HE_FMTSUBR,"h_ienlarge3",
				hformatname(hdi1->pixel_format)));
	}
}

int h_ienlarge3_b(hdi1,hdi2,hdo,xf,yf,tf,t)

struct header *hdi1,*hdi2,*hdo;
int xf,yf,tf,t;

{
	return(h_ienlarge3_B(hdi1->firstpix,hdi2->firstpix,hdo->firstpix,
		hdi1->rows,hdi1->cols,hdi1->ocols,hdi2->ocols,hdo->ocols,
		xf,yf,tf,t));
}

int h_ienlarge3_i(hdi1,hdi2,hdo,xf,yf,tf,t)

struct header *hdi1,*hdi2,*hdo;
int xf,yf,tf,t;

{
	return(h_ienlarge3_I((int *) hdi1->firstpix,(int *) hdi2->firstpix,
		(int *) hdo->firstpix,hdi1->rows,hdi1->cols,hdi1->ocols,
		hdi2->ocols,hdo->ocols,xf,yf,tf,t));
}

int h_ienlarge3_f(hdi1,hdi2,hdo,xf,yf,tf,t)

struct header *hdi1,*hdi2,*hdo;
int xf,yf,tf,t;

{
	return(h_ienlarge3_F((float *) hdi1->firstpix,(float *) hdi2->firstpix,
		(float *) hdo->firstpix,hdi1->rows,hdi1->cols,hdi1->ocols,
		hdi2->ocols,hdo->ocols,xf,yf,tf,t));
}

int h_ienlarge3_c(hdi1,hdi2,hdo,xf,yf,tf,t)

struct header *hdi1,*hdi2,*hdo;
int xf,yf,tf,t;

{
	return(h_ienlarge3_C((float *) hdi1->firstpix,(float *) hdi2->firstpix,
		(float *) hdo->firstpix,hdi1->rows,hdi1->cols,hdi1->ocols,
		hdi2->ocols,hdo->ocols,xf,yf,tf,t));
}

int h_ienlarge3_B(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo,xf,yf,tf,t)

byte *imagei1,*imagei2,*imageo;
int nr,nc,nlpi1,nlpi2,nlpo,xf,yf,tf,t;

{
	register int i,ii,j,jj,*fp,oval;
	register byte *pi1,*pi2,*po;
	int val,valr,vald,valrd,valn,valnr,valnd,valnrd,nexo,halfscl;

	if (ienl3_alloc(xf,yf,tf) == HIPS_ERROR)
		return(HIPS_ERROR);
	pi1 = imagei1;
	pi2 = imagei2;
	po = imageo;
	nexo = nlpo-(nc*xf);
	halfscl = ienl3_scale/2;
	for (i=nr;i;i--) {
	    for (ii=0;ii<yf;ii++) {
		for (j=nc;j;j--) {
			val = *pi1;
			valr = (j==1) ? val : *(pi1+1);
			vald = (i==1) ? val : *(pi1+nlpi1);
			valrd = (i==1) ? valr :
				((j==1) ? vald :
					*(pi1+nlpi1+1));
			valn = *pi2;
			valnr = (j==1) ? val : *(pi2+1);
			valnd = (i==1) ? val : *(pi2+nlpi2);
			valnrd = (i==1) ? valr :
				((j==1) ? vald :
					*(pi2+nlpi2+1));
			fp = factors + (ii*xf+t*xf*yf)*8;
			for (jj=0;jj<xf;jj++) {
				oval = val * *fp++;
				oval += valr * *fp++;
				oval += vald * *fp++;
				oval += valrd * *fp++;
				oval += valn * *fp++;
				oval += valnr * *fp++;
				oval += valnd * *fp++;
				oval += valnrd * *fp++;
				*po++ = (oval+halfscl)/ienl3_scale;
			}
			pi1++;
			pi2++;
		}
		pi1 -= nc;
		pi2 -= nc;
		po += nexo;
	    }
	    pi1 += nlpi1;
	    pi2 += nlpi2;
	}
	return(HIPS_OK);
}

int h_ienlarge3_I(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo,xf,yf,tf,t)

int *imagei1,*imagei2,*imageo;
int nr,nc,nlpi1,nlpi2,nlpo,xf,yf,tf,t;

{
	register int i,ii,j,jj,*fp,oval;
	register int *pi1,*pi2,*po;
	int val,valr,vald,valrd,valn,valnr,valnd,valnrd,nexo,halfscl;

	if (ienl3_alloc(xf,yf,tf) == HIPS_ERROR)
		return(HIPS_ERROR);
	pi1 = imagei1;
	pi2 = imagei2;
	po = imageo;
	nexo = nlpo-(nc*xf);
	halfscl = ienl3_scale/2;
	for (i=nr;i;i--) {
	    for (ii=0;ii<yf;ii++) {
		for (j=nc;j;j--) {
			val = *pi1;
			valr = (j==1) ? val : *(pi1+1);
			vald = (i==1) ? val : *(pi1+nlpi1);
			valrd = (i==1) ? valr :
				((j==1) ? vald :
					*(pi1+nlpi1+1));
			valn = *pi2;
			valnr = (j==1) ? val : *(pi2+1);
			valnd = (i==1) ? val : *(pi2+nlpi2);
			valnrd = (i==1) ? valr :
				((j==1) ? vald :
					*(pi2+nlpi2+1));
			fp = factors + (ii*xf+t*xf*yf)*8;
			for (jj=0;jj<xf;jj++) {
				oval = val * *fp++;
				oval += valr * *fp++;
				oval += vald * *fp++;
				oval += valrd * *fp++;
				oval += valn * *fp++;
				oval += valnr * *fp++;
				oval += valnd * *fp++;
				oval += valnrd * *fp++;
				*po++ = (oval+halfscl)/ienl3_scale;
			}
			pi1++;
			pi2++;
		}
		pi1 -= nc;
		pi2 -= nc;
		po += nexo;
	    }
	    pi1 += nlpi1;
	    pi2 += nlpi2;
	}
	return(HIPS_OK);
}

int h_ienlarge3_F(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo,xf,yf,tf,t)

float *imagei1,*imagei2,*imageo;
int nr,nc,nlpi1,nlpi2,nlpo,xf,yf,tf,t;

{
	register int i,ii,j,jj,*fp;
	register float oval;
	register float *pi1,*pi2,*po;
	float val,valr,vald,valrd,valn,valnr,valnd,valnrd;
	int nexo;

	if (ienl3_alloc(xf,yf,tf) == HIPS_ERROR)
		return(HIPS_ERROR);
	pi1 = imagei1;
	pi2 = imagei2;
	po = imageo;
	nexo = nlpo-(nc*xf);
	for (i=nr;i;i--) {
	    for (ii=0;ii<yf;ii++) {
		for (j=nc;j;j--) {
			val = *pi1;
			valr = (j==1) ? val : *(pi1+1);
			vald = (i==1) ? val : *(pi1+nlpi1);
			valrd = (i==1) ? valr :
				((j==1) ? vald :
					*(pi1+nlpi1+1));
			valn = *pi2;
			valnr = (j==1) ? val : *(pi2+1);
			valnd = (i==1) ? val : *(pi2+nlpi2);
			valnrd = (i==1) ? valr :
				((j==1) ? vald :
					*(pi2+nlpi2+1));
			fp = factors + (ii*xf+t*xf*yf)*8;
			for (jj=0;jj<xf;jj++) {
				oval = val * *fp++;
				oval += valr * *fp++;
				oval += vald * *fp++;
				oval += valrd * *fp++;
				oval += valn * *fp++;
				oval += valnr * *fp++;
				oval += valnd * *fp++;
				oval += valnrd * *fp++;
				*po++ = oval/ienl3_scale;
			}
			pi1++;
			pi2++;
		}
		pi1 -= nc;
		pi2 -= nc;
		po += nexo;
	    }
	    pi1 += nlpi1;
	    pi2 += nlpi2;
	}
	return(HIPS_OK);
}

int h_ienlarge3_C(imagei1,imagei2,imageo,nr,nc,nlpi1,nlpi2,nlpo,xf,yf,tf,t)

float *imagei1,*imagei2,*imageo;
int nr,nc,nlpi1,nlpi2,nlpo,xf,yf,tf,t;

{
	register int i,ii,j,jj,*fp;
	register float *pi1,*pi2,*po,oval;
	float val[2],valr[2],vald[2],valrd[2],valn[2],valnr[2],valnd[2];
	float valnrd[2];
	int nexo,nlpi12,nlpi22;

	if (ienl3_alloc(xf,yf,tf) == HIPS_ERROR)
		return(HIPS_ERROR);
	pi1 = imagei1;
	pi2 = imagei2;
	nlpi12 = nlpi1*2;
	nlpi22 = nlpi2*2;
	po = imageo;
	nexo = 2*(nlpo-(nc*xf));
	for (i=nr;i;i--) {
	    for (ii=0;ii<yf;ii++) {
		for (j=nc;j;j--) {
			val[0] = *pi1;
			val[1] = *(pi1+1);
			valn[0] = *pi2;
			valn[1] = *(pi2+1);
			if (j == 1) {
				valr[0] = val[0];
				valr[1] = val[1];
				valnr[0] = valn[0];
				valnr[1] = valn[1];
			}
			else {
				valr[0] = *(pi1+2);
				valr[1] = *(pi1+3);
				valnr[0] = *(pi2+2);
				valnr[1] = *(pi2+3);
			}
			if (i == 1) {
				vald[0] = val[0];
				vald[1] = val[1];
				valnd[0] = valn[0];
				valnd[1] = valn[1];
			}
			else {
				vald[0] = *(pi1+nlpi12);
				vald[1] = *(pi1+nlpi12+1);
				valnd[0] = *(pi2+nlpi22);
				valnd[1] = *(pi2+nlpi22+1);
			}
			if (i == 1) {
				valrd[0] = valr[0];
				valrd[1] = valr[1];
				valnrd[0] = valnr[0];
				valnrd[1] = valnr[1];
			}
			else if (j == 1) {
				valrd[0] = vald[0];
				valrd[1] = vald[1];
				valnrd[0] = valnd[0];
				valnrd[1] = valnd[1];
			}
			else {
				valrd[0] = *(pi1+nlpi12+2);
				valrd[1] = *(pi1+nlpi12+3);
				valnrd[0] = *(pi2+nlpi22+2);
				valnrd[1] = *(pi2+nlpi22+3);
			}
			fp = factors + (ii*xf+t*xf*yf)*8;
			for (jj=0;jj<xf;jj++) {
				oval = val[0] * *fp++;
				oval += valr[0] * *fp++;
				oval += vald [0]* *fp++;
				oval += valrd[0] * *fp++;
				oval += valn[0] * *fp++;
				oval += valnr[0] * *fp++;
				oval += valnd[0] * *fp++;
				oval += valnrd[0] * *fp++;
				*po++ = oval/ienl3_scale;
				fp -= 8;
				oval = val[1] * *fp++;
				oval += valr[1] * *fp++;
				oval += vald [1]* *fp++;
				oval += valrd[1] * *fp++;
				oval += valn[1] * *fp++;
				oval += valnr[1] * *fp++;
				oval += valnd[1] * *fp++;
				oval += valnrd[1] * *fp++;
				*po++ = oval/ienl3_scale;
			}
			pi1 += 2;
			pi2 += 2;
		}
		pi1 -= 2*nc;
		pi2 -= 2*nc;
		po += nexo;
	    }
	    pi1 += 2*nlpi1;
	    pi2 += 2*nlpi2;
	}
	return(HIPS_OK);
}

int ienl3_alloc(xf,yf,tf)

int xf,yf,tf;

{
	register int i,j,t,*fp;

	ienl3_scale = xf*yf*tf;
	if (ienl3_allocated) {
		if (xf != ienl3_xf || yf != ienl3_yf || tf != ienl3_tf) {
			free(factors);
			if ((factors = (int *)
				memalloc(8*ienl3_scale,sizeof(int)))
				== (int *) HIPS_ERROR)
					return(HIPS_ERROR);
		}
	}
	else {
		ienl3_allocated = TRUE;
		if ((factors = (int *) memalloc(8*ienl3_scale,sizeof(int)))
			== (int *) HIPS_ERROR)
				return(HIPS_ERROR);
	}
	ienl3_xf = xf;
	ienl3_yf = yf;
	ienl3_tf = tf;
	fp = factors;
	for (t=0;t<tf;t++) {
		for (i=0;i<yf;i++) {
			for (j=0;j<xf;j++) {
				*fp++ = (xf-j)*(yf-i)*(tf-t);
				*fp++ = (j)*(yf-i)*(tf-t);
				*fp++ = (xf-j)*(i)*(tf-t);
				*fp++ = (j)*(i)*(tf-t);
				*fp++ = (xf-j)*(yf-i)*(t);
				*fp++ = (j)*(yf-i)*(t);
				*fp++ = (xf-j)*(i)*(t);
				*fp++ = (j)*(i)*(t);
			}
		}
	}
	return(HIPS_OK);
}
