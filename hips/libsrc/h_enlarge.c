/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_enlarge.c - subroutines to enlarge an image by pixel replication
 *
 * pixel formats: BYTE, INT, FLOAT, COMPLEX
 *
 * Michael Landy - 1/12/91
 */

#include <hipl_format.h>

int h_enlarge(hdi,hdo,xf,yf)

struct header *hdi,*hdo;
int xf,yf;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	return(h_enlarge_b(hdi,hdo,xf,yf));
	case PFINT:	return(h_enlarge_i(hdi,hdo,xf,yf));
	case PFFLOAT:	return(h_enlarge_f(hdi,hdo,xf,yf));
	case PFCOMPLEX:	return(h_enlarge_c(hdi,hdo,xf,yf));
	default:	return(perr(HE_FMTSUBR,"h_enlarge",
				hformatname(hdi->pixel_format)));
	}
}

int h_enlarge_b(hdi,hdo,xf,yf)

struct header *hdi,*hdo;
int xf,yf;

{
	return(h_enlarge_B(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
		hdi->ocols,hdo->ocols,xf,yf));
}

int h_enlarge_i(hdi,hdo,xf,yf)

struct header *hdi,*hdo;
int xf,yf;

{
	return(h_enlarge_I((int *) hdi->firstpix,(int *) hdo->firstpix,hdi->rows,
		hdi->cols,hdi->ocols,hdo->ocols,xf,yf));
}

int h_enlarge_f(hdi,hdo,xf,yf)

struct header *hdi,*hdo;
int xf,yf;

{
	return(h_enlarge_F((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,xf,yf));
}

int h_enlarge_c(hdi,hdo,xf,yf)

struct header *hdi,*hdo;
int xf,yf;

{
	return(h_enlarge_C((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,xf,yf));
}

int h_enlarge_B(imagei,imageo,nr,nc,nlpi,nlpo,xf,yf)

byte *imagei,*imageo;
int nr,nc,nlpi,nlpo,xf,yf;

{
	register int i,ii,j,jj,nexo;
	register byte *pi,*po;

	pi = imagei;
	po = imageo;
	nexo = nlpo-(nc*xf);
	for (i=0;i<nr;i++) {
	    for (ii=0;ii<yf;ii++) {
		for (j=0;j<nc;j++) {
		    for (jj=0;jj<xf;jj++)
			*po++ = *pi;
		    pi++;
		}
		pi -= nc;
		po += nexo;
	    }
	    pi += nlpi;
	}
	return(HIPS_OK);
}

int h_enlarge_I(imagei,imageo,nr,nc,nlpi,nlpo,xf,yf)

int *imagei,*imageo;
int nr,nc,nlpi,nlpo,xf,yf;

{
	register int i,ii,j,jj,nexo;
	register int *pi,*po;

	pi = imagei;
	po = imageo;
	nexo = nlpo-(nc*xf);
	for (i=0;i<nr;i++) {
	    for (ii=0;ii<yf;ii++) {
		for (j=0;j<nc;j++) {
		    for (jj=0;jj<xf;jj++)
			*po++ = *pi;
		    pi++;
		}
		pi -= nc;
		po += nexo;
	    }
	    pi += nlpi;
	}
	return(HIPS_OK);
}

int h_enlarge_F(imagei,imageo,nr,nc,nlpi,nlpo,xf,yf)

float *imagei,*imageo;
int nr,nc,nlpi,nlpo,xf,yf;

{
	register int i,ii,j,jj,nexo;
	register float *pi,*po;

	pi = imagei;
	po = imageo;
	nexo = nlpo-(nc*xf);
	for (i=0;i<nr;i++) {
	    for (ii=0;ii<yf;ii++) {
		for (j=0;j<nc;j++) {
		    for (jj=0;jj<xf;jj++)
			*po++ = *pi;
		    pi++;
		}
		pi -= nc;
		po += nexo;
	    }
	    pi += nlpi;
	}
	return(HIPS_OK);
}

int h_enlarge_C(imagei,imageo,nr,nc,nlpi,nlpo,xf,yf)

float *imagei,*imageo;
int nr,nc,nlpi,nlpo,xf,yf;

{
	register int i,ii,j,jj,nexo;
	register float *pi,*po;

	pi = imagei;
	po = imageo;
	nexo = 2*(nlpo-(nc*xf));
	for (i=0;i<nr;i++) {
	    for (ii=0;ii<yf;ii++) {
		for (j=0;j<nc;j++) {
		    for (jj=0;jj<xf;jj++) {
			*po++ = pi[0];
			*po++ = pi[1];
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
