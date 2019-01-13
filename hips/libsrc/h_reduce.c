/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_reduce.c - subroutines to reduce an image by pixel averaging
 *
 * pixel formats: BYTE->INT, SHORT, INT, FLOAT, COMPLEX
 *
 * Rewritten by Michael Landy - 11/5/87
 * HIPS 2 - Michael Landy - 6/29/91
 */

#include <hipl_format.h>

int h_reduce(hdi,hdo,xf,yf)

struct header *hdi,*hdo;
int xf,yf;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	if (hdo->pixel_format == PFINT)
				return(h_reduce_bi(hdi,hdo,xf,yf));
			else
				return(perr(HE_FMT2SUBR,"h_reduce",
				    hformatname(hdi->pixel_format),
				    hformatname(hdo->pixel_format)));
	case PFSHORT:	return(h_reduce_s(hdi,hdo,xf,yf));
	case PFINT:	return(h_reduce_i(hdi,hdo,xf,yf));
	case PFFLOAT:	return(h_reduce_f(hdi,hdo,xf,yf));
	case PFCOMPLEX:	return(h_reduce_c(hdi,hdo,xf,yf));
	default:	return(perr(HE_FMTSUBR,"h_reduce",
				hformatname(hdi->pixel_format)));
	}
}

int h_reduce_bi(hdi,hdo,xf,yf)

struct header *hdi,*hdo;
int xf,yf;

{
	return(h_reduce_BI(hdi->firstpix,(int *) hdo->firstpix,hdi->rows,
		hdi->cols,hdi->ocols,hdo->ocols,xf,yf));
}

int h_reduce_s(hdi,hdo,xf,yf)

struct header *hdi,*hdo;
int xf,yf;

{
	return(h_reduce_S((short *) hdi->firstpix,(short *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,xf,yf));
}

int h_reduce_i(hdi,hdo,xf,yf)

struct header *hdi,*hdo;
int xf,yf;

{
	return(h_reduce_I((int *) hdi->firstpix,(int *) hdo->firstpix,hdi->rows,
		hdi->cols,hdi->ocols,hdo->ocols,xf,yf));
}

int h_reduce_f(hdi,hdo,xf,yf)

struct header *hdi,*hdo;
int xf,yf;

{
	return(h_reduce_F((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,xf,yf));
}

int h_reduce_c(hdi,hdo,xf,yf)

struct header *hdi,*hdo;
int xf,yf;

{
	return(h_reduce_C((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,xf,yf));
}

int h_reduce_BI(imagei,imageo,nr,nc,nlpi,nlpo,xf,yf)

byte *imagei;
int *imageo;
int nr,nc,nlpi,nlpo,xf,yf;

{
	int nor,noc;
	register int i,ii,j,jj,nexi;
	register byte *pi;
	register int *po;

	nor = nr/yf;
	noc = nc/xf;
	pi = imagei;
	po = imageo;
	nexi = nlpi-nc;
	for (i=0;i<nor;i++) {
	    for (ii=0;ii<yf;ii++) {
		for (j=0;j<noc;j++) {
		    for (jj=0;jj<xf;jj++)
			*po += *pi++;
		    po++;
		}
		pi += nexi;
		po -= noc;
	    }
	    po += nlpo;
	}
	return(HIPS_OK);
}

int h_reduce_S(imagei,imageo,nr,nc,nlpi,nlpo,xf,yf)

short *imagei,*imageo;
int nr,nc,nlpi,nlpo,xf,yf;

{
	int nor,noc;
	register int i,ii,j,jj,nexi;
	register short *pi,*po;

	nor = nr/yf;
	noc = nc/xf;
	pi = imagei;
	po = imageo;
	nexi = nlpi-nc;
	for (i=0;i<nor;i++) {
	    for (ii=0;ii<yf;ii++) {
		for (j=0;j<noc;j++) {
		    for (jj=0;jj<xf;jj++)
			*po += *pi++;
		    po++;
		}
		pi += nexi;
		po -= noc;
	    }
	    po += nlpo;
	}
	return(HIPS_OK);
}

int h_reduce_I(imagei,imageo,nr,nc,nlpi,nlpo,xf,yf)

int *imagei,*imageo;
int nr,nc,nlpi,nlpo,xf,yf;

{
	int nor,noc;
	register int i,ii,j,jj,nexi;
	register int *pi,*po;

	nor = nr/yf;
	noc = nc/xf;
	pi = imagei;
	po = imageo;
	nexi = nlpi-nc;
	for (i=0;i<nor;i++) {
	    for (ii=0;ii<yf;ii++) {
		for (j=0;j<noc;j++) {
		    for (jj=0;jj<xf;jj++)
			*po += *pi++;
		    po++;
		}
		pi += nexi;
		po -= noc;
	    }
	    po += nlpo;
	}
	return(HIPS_OK);
}

int h_reduce_F(imagei,imageo,nr,nc,nlpi,nlpo,xf,yf)

float *imagei,*imageo;
int nr,nc,nlpi,nlpo,xf,yf;

{
	int nor,noc;
	register int i,ii,j,jj,nexi;
	register float *pi,*po;

	nor = nr/yf;
	noc = nc/xf;
	pi = imagei;
	po = imageo;
	nexi = nlpi-nc;
	for (i=0;i<nor;i++) {
	    for (ii=0;ii<yf;ii++) {
		for (j=0;j<noc;j++) {
		    for (jj=0;jj<xf;jj++)
			*po += *pi++;
		    po++;
		}
		pi += nexi;
		po -= noc;
	    }
	    po += nlpo;
	}
	return(HIPS_OK);
}

int h_reduce_C(imagei,imageo,nr,nc,nlpi,nlpo,xf,yf)

float *imagei,*imageo;
int nr,nc,nlpi,nlpo,xf,yf;

{
	int nor,noc;
	register int i,ii,j,jj,nexi;
	register float *pi,*po;

	nor = nr/yf;
	noc = nc/xf;
	pi = imagei;
	po = imageo;
	nexi = 2*(nlpi-nc);
	for (i=0;i<nor;i++) {
	    for (ii=0;ii<yf;ii++) {
		for (j=0;j<noc;j++) {
		    for (jj=0;jj<xf;jj++) {
			po[0] += *pi++;
			po[1] += *pi++;
		    }
		    po += 2;
		}
		pi += nexi;
		po -= 2*noc;
	    }
	    po += 2*nlpo;
	}
	return(HIPS_OK);
}
