/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_sample.c - subroutines to subsample images
 *
 * pixel formats: BYTE, SHORT, INT, FLOAT, DOUBLE, COMPLEX, DBLCOM
 *
 * Michael Landy - 3/8/94
 */

#include <hipl_format.h>

int h_sample(hdi,hdo,ratex,ratey,offsetx,offsety)

struct header *hdi,*hdo;
int ratex,ratey,offsetx,offsety;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	return(h_sample_b(hdi,hdo,ratex,ratey,offsetx,offsety));
	case PFSHORT:	return(h_sample_s(hdi,hdo,ratex,ratey,offsetx,offsety));
	case PFINT:	return(h_sample_i(hdi,hdo,ratex,ratey,offsetx,offsety));
	case PFFLOAT:	return(h_sample_f(hdi,hdo,ratex,ratey,offsetx,offsety));
	case PFDOUBLE:	return(h_sample_d(hdi,hdo,ratex,ratey,offsetx,offsety));
	case PFCOMPLEX:	return(h_sample_c(hdi,hdo,ratex,ratey,offsetx,offsety));
	case PFDBLCOM:	return(h_sample_dc(hdi,hdo,ratex,ratey,offsetx,
				offsety));
	default:	return(perr(HE_FMTSUBR,"h_sample",
				hformatname(hdi->pixel_format)));
	}
}

int h_sample_b(hdi,hdo,ratex,ratey,offsetx,offsety)

struct header *hdi,*hdo;
int ratex,ratey,offsetx,offsety;

{
	return(h_sample_B(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
		hdi->ocols,hdo->ocols,ratex,ratey,offsetx,offsety));
}

int h_sample_s(hdi,hdo,ratex,ratey,offsetx,offsety)

struct header *hdi,*hdo;
int ratex,ratey,offsetx,offsety;

{
	return(h_sample_S((short *) hdi->firstpix,(short *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,ratex,
		ratey,offsetx,offsety));
}

int h_sample_i(hdi,hdo,ratex,ratey,offsetx,offsety)

struct header *hdi,*hdo;
int ratex,ratey,offsetx,offsety;

{
	return(h_sample_I((int *) hdi->firstpix,(int *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,ratex,ratey,
		offsetx,offsety));
}

int h_sample_f(hdi,hdo,ratex,ratey,offsetx,offsety)

struct header *hdi,*hdo;
int ratex,ratey,offsetx,offsety;

{
	return(h_sample_F((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,ratex,ratey,
		offsetx,offsety));
}

int h_sample_d(hdi,hdo,ratex,ratey,offsetx,offsety)

struct header *hdi,*hdo;
int ratex,ratey,offsetx,offsety;

{
	return(h_sample_D((double *) hdi->firstpix,(double *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,ratex,ratey,
		offsetx,offsety));
}

int h_sample_c(hdi,hdo,ratex,ratey,offsetx,offsety)

struct header *hdi,*hdo;
int ratex,ratey,offsetx,offsety;

{
	return(h_sample_C((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,ratex,ratey,
		offsetx,offsety));
}

int h_sample_dc(hdi,hdo,ratex,ratey,offsetx,offsety)

struct header *hdi,*hdo;
int ratex,ratey,offsetx,offsety;

{
	return(h_sample_DC((double *) hdi->firstpix,(double *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,ratex,ratey,
		offsetx,offsety));
}

int h_sample_B(imagei,imageo,nr,nc,nlpi,nlpo,ratex,ratey,offsetx,offsety)

byte *imagei,*imageo;
int nr,nc,nlpi,nlpo,ratex,ratey,offsetx,offsety;

{
	register int i,j,nexi,nexo,nco,nro;
	register byte *pi,*po;

	nco = (nc + ratex - (1 + offsetx))/ratex;
	nro = (nr + ratey - (1 + offsety))/ratey;
	nexi = ratey*nlpi - ratex*nco;
	nexo = nlpo - nco;
	pi = imagei + offsety*nlpi + offsetx;
	po = imageo;
	for (i=0;i<nro;i++) {
		for (j=0;j<nco;j++) {
			*po++ = *pi;
			pi += ratex;
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_sample_S(imagei,imageo,nr,nc,nlpi,nlpo,ratex,ratey,offsetx,offsety)

short *imagei,*imageo;
int nr,nc,nlpi,nlpo,ratex,ratey,offsetx,offsety;

{
	register int i,j,nexi,nexo,nco,nro;
	register short *pi,*po;

	nco = (nc + ratex - (1 + offsetx))/ratex;
	nro = (nr + ratey - (1 + offsety))/ratey;
	nexi = ratey*nlpi - ratex*nco;
	nexo = nlpo - nco;
	pi = imagei + offsety*nlpi + offsetx;
	po = imageo;
	for (i=0;i<nro;i++) {
		for (j=0;j<nco;j++) {
			*po++ = *pi;
			pi += ratex;
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_sample_I(imagei,imageo,nr,nc,nlpi,nlpo,ratex,ratey,offsetx,offsety)

int *imagei,*imageo;
int nr,nc,nlpi,nlpo,ratex,ratey,offsetx,offsety;

{
	register int i,j,nexi,nexo,nco,nro;
	register int *pi,*po;

	nco = (nc + ratex - (1 + offsetx))/ratex;
	nro = (nr + ratey - (1 + offsety))/ratey;
	nexi = ratey*nlpi - ratex*nco;
	nexo = nlpo - nco;
	pi = imagei + offsety*nlpi + offsetx;
	po = imageo;
	for (i=0;i<nro;i++) {
		for (j=0;j<nco;j++) {
			*po++ = *pi;
			pi += ratex;
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_sample_F(imagei,imageo,nr,nc,nlpi,nlpo,ratex,ratey,offsetx,offsety)

float *imagei,*imageo;
int nr,nc,nlpi,nlpo,ratex,ratey,offsetx,offsety;

{
	register int i,j,nexi,nexo,nco,nro;
	register float *pi,*po;

	nco = (nc + ratex - (1 + offsetx))/ratex;
	nro = (nr + ratey - (1 + offsety))/ratey;
	nexi = ratey*nlpi - ratex*nco;
	nexo = nlpo - nco;
	pi = imagei + offsety*nlpi + offsetx;
	po = imageo;
	for (i=0;i<nro;i++) {
		for (j=0;j<nco;j++) {
			*po++ = *pi;
			pi += ratex;
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_sample_D(imagei,imageo,nr,nc,nlpi,nlpo,ratex,ratey,offsetx,offsety)

double *imagei,*imageo;
int nr,nc,nlpi,nlpo,ratex,ratey,offsetx,offsety;

{
	register int i,j,nexi,nexo,nco,nro;
	register double *pi,*po;

	nco = (nc + ratex - (1 + offsetx))/ratex;
	nro = (nr + ratey - (1 + offsety))/ratey;
	nexi = ratey*nlpi - ratex*nco;
	nexo = nlpo - nco;
	pi = imagei + offsety*nlpi + offsetx;
	po = imageo;
	for (i=0;i<nro;i++) {
		for (j=0;j<nco;j++) {
			*po++ = *pi;
			pi += ratex;
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_sample_C(imagei,imageo,nr,nc,nlpi,nlpo,ratex,ratey,offsetx,offsety)

float *imagei,*imageo;
int nr,nc,nlpi,nlpo,ratex,ratey,offsetx,offsety;

{
	register int i,j,nexi,nexo,nco,nro;
	register float *pi,*po;

	nco = (nc + ratex - (1 + offsetx))/ratex;
	nro = (nr + ratey - (1 + offsety))/ratey;
	nexi = 2*(ratey*nlpi - ratex*nco);
	nexo = 2*(nlpo - nco);
	pi = imagei + offsety*nlpi + offsetx;
	po = imageo;
	ratex *= 2;
	for (i=0;i<nro;i++) {
		for (j=0;j<nco;j++) {
			*po++ = *pi;
			*po++ = *(pi+1);
			pi += ratex;
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_sample_DC(imagei,imageo,nr,nc,nlpi,nlpo,ratex,ratey,offsetx,offsety)

double *imagei,*imageo;
int nr,nc,nlpi,nlpo,ratex,ratey,offsetx,offsety;

{
	register int i,j,nexi,nexo,nco,nro;
	register double *pi,*po;

	nco = (nc + ratex - (1 + offsetx))/ratex;
	nro = (nr + ratey - (1 + offsety))/ratey;
	nexi = 2*(ratey*nlpi - ratex*nco);
	nexo = 2*(nlpo - nco);
	pi = imagei + offsety*nlpi + offsetx;
	po = imageo;
	ratex *= 2;
	for (i=0;i<nro;i++) {
		for (j=0;j<nco;j++) {
			*po++ = *pi;
			*po++ = *(pi+1);
			pi += ratex;
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}
