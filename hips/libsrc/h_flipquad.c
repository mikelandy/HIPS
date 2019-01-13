/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_flipquad.c - swap opposite quadrants of an image
 *
 * pixel formats: BYTE, FLOAT, DOUBLE
 *
 * Michael Landy - 7/10/91
 */

#include <hipl_format.h>

int h_flipquad(hdi,hdo)

struct header *hdi,*hdo;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	return(h_flipquad_b(hdi,hdo));
	case PFFLOAT:	return(h_flipquad_f(hdi,hdo));
	case PFDOUBLE:	return(h_flipquad_d(hdi,hdo));
	default:	return(perr(HE_FMTSUBR,"h_flipquad",
				hformatname(hdi->pixel_format)));
	}
}

int h_flipquad_b(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_flipquad_B(hdi->firstpix,hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols));
}

int h_flipquad_f(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_flipquad_F((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols));
}

int h_flipquad_d(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_flipquad_D((double *) hdi->firstpix,(double *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols));
}

int h_flipquad_B(imagei,imageo,nr,nc,nlpi,nlpo)

byte *imagei,*imageo;
int nr,nc,nlpi,nlpo;

{
	register int nr2,nc2,i,j,nexi,nexo;
	register byte *pi1,*pi2,*pi3,*pi4,*po1,*po2,*po3,*po4,t;

	if (nr == 1 && nc == 1) {
		*imageo = *imagei;
		return(HIPS_OK);
	}
	if (nr == 1 && nc%2 == 0) {
		nc2 = nc/2;
		pi1 = imagei;
		pi2 = imagei + nc2;
		po1 = imageo;
		po2 = imageo + nc2;
		for (j=0;j<nc2;j++) {
			t = *pi1++;
			*po1++ = *pi2++;
			*po2++ = t;
		}
		return(HIPS_OK);
	}
	if (nc == 1 && nr%2 == 0) {
		nr2 = nr/2;
		pi1 = imagei;
		pi2 = imagei + nr2*nlpi;
		po1 = imageo;
		po2 = imageo + nr2*nlpo;
		for (j=0;j<nr2;j++) {
			t = *pi1;
			*po1 = *pi2;
			*po2 = t;
			pi1 += nlpi;
			pi2 += nlpi;
			po1 += nlpo;
			po2 += nlpo;
		}
		return(HIPS_OK);
	}
	if (nr%2 || nc%2)
		return(perr(HE_MULT2));
	nr2 = nr/2;
	nc2 = nc/2;
	nexi = nlpi-nc2;
	nexo = nlpo-nc2;
	pi1 = imagei;
	pi2 = imagei + nc2;
	pi3 = imagei + nr2*nlpi;
	pi4 = imagei + nr2*nlpi + nc2;
	po1 = imageo;
	po2 = imageo + nc2;
	po3 = imageo + nr2*nlpo;
	po4 = imageo + nr2*nlpo + nc2;
	for (i=0;i<nr2;i++) {
		for (j=0;j<nc2;j++) {
			t = *pi1++;
			*po1++ = *pi4++;
			*po4++ = t;
			t = *pi2++;
			*po2++ = *pi3++;
			*po3++ = t;
		}
		pi1 += nexi;
		pi2 += nexi;
		pi3 += nexi;
		pi4 += nexi;
		po1 += nexo;
		po2 += nexo;
		po3 += nexo;
		po4 += nexo;
	}
	return(HIPS_OK);
}

int h_flipquad_F(imagei,imageo,nr,nc,nlpi,nlpo)

float *imagei,*imageo;
int nr,nc,nlpi,nlpo;

{
	register int nr2,nc2,i,j,nexi,nexo;
	register float *pi1,*pi2,*pi3,*pi4,*po1,*po2,*po3,*po4,t;

	if (nr == 1 && nc == 1) {
		*imageo = *imagei;
		return(HIPS_OK);
	}
	if (nr == 1 && nc%2 == 0) {
		nc2 = nc/2;
		pi1 = imagei;
		pi2 = imagei + nc2;
		po1 = imageo;
		po2 = imageo + nc2;
		for (j=0;j<nc2;j++) {
			t = *pi1++;
			*po1++ = *pi2++;
			*po2++ = t;
		}
		return(HIPS_OK);
	}
	if (nc == 1 && nr%2 == 0) {
		nr2 = nr/2;
		pi1 = imagei;
		pi2 = imagei + nr2*nlpi;
		po1 = imageo;
		po2 = imageo + nr2*nlpo;
		for (j=0;j<nr2;j++) {
			t = *pi1;
			*po1 = *pi2;
			*po2 = t;
			pi1 += nlpi;
			pi2 += nlpi;
			po1 += nlpo;
			po2 += nlpo;
		}
		return(HIPS_OK);
	}
	if (nr%2 || nc%2)
		return(perr(HE_MULT2));
	nr2 = nr/2;
	nc2 = nc/2;
	nexi = nlpi-nc2;
	nexo = nlpo-nc2;
	pi1 = imagei;
	pi2 = imagei + nc2;
	pi3 = imagei + nr2*nlpi;
	pi4 = imagei + nr2*nlpi + nc2;
	po1 = imageo;
	po2 = imageo + nc2;
	po3 = imageo + nr2*nlpo;
	po4 = imageo + nr2*nlpo + nc2;
	for (i=0;i<nr2;i++) {
		for (j=0;j<nc2;j++) {
			t = *pi1++;
			*po1++ = *pi4++;
			*po4++ = t;
			t = *pi2++;
			*po2++ = *pi3++;
			*po3++ = t;
		}
		pi1 += nexi;
		pi2 += nexi;
		pi3 += nexi;
		pi4 += nexi;
		po1 += nexo;
		po2 += nexo;
		po3 += nexo;
		po4 += nexo;
	}
	return(HIPS_OK);
}

int h_flipquad_D(imagei,imageo,nr,nc,nlpi,nlpo)

double *imagei,*imageo;
int nr,nc,nlpi,nlpo;

{
	register int nr2,nc2,i,j,nexi,nexo;
	register double *pi1,*pi2,*pi3,*pi4,*po1,*po2,*po3,*po4,t;

	if (nr == 1 && nc == 1) {
		*imageo = *imagei;
		return(HIPS_OK);
	}
	if (nr == 1 && nc%2 == 0) {
		nc2 = nc/2;
		pi1 = imagei;
		pi2 = imagei + nc2;
		po1 = imageo;
		po2 = imageo + nc2;
		for (j=0;j<nc2;j++) {
			t = *pi1++;
			*po1++ = *pi2++;
			*po2++ = t;
		}
		return(HIPS_OK);
	}
	if (nc == 1 && nr%2 == 0) {
		nr2 = nr/2;
		pi1 = imagei;
		pi2 = imagei + nr2*nlpi;
		po1 = imageo;
		po2 = imageo + nr2*nlpo;
		for (j=0;j<nr2;j++) {
			t = *pi1;
			*po1 = *pi2;
			*po2 = t;
			pi1 += nlpi;
			pi2 += nlpi;
			po1 += nlpo;
			po2 += nlpo;
		}
		return(HIPS_OK);
	}
	if (nr%2 || nc%2)
		return(perr(HE_MULT2));
	nr2 = nr/2;
	nc2 = nc/2;
	nexi = nlpi-nc2;
	nexo = nlpo-nc2;
	pi1 = imagei;
	pi2 = imagei + nc2;
	pi3 = imagei + nr2*nlpi;
	pi4 = imagei + nr2*nlpi + nc2;
	po1 = imageo;
	po2 = imageo + nc2;
	po3 = imageo + nr2*nlpo;
	po4 = imageo + nr2*nlpo + nc2;
	for (i=0;i<nr2;i++) {
		for (j=0;j<nc2;j++) {
			t = *pi1++;
			*po1++ = *pi4++;
			*po4++ = t;
			t = *pi2++;
			*po2++ = *pi3++;
			*po3++ = t;
		}
		pi1 += nexi;
		pi2 += nexi;
		pi3 += nexi;
		pi4 += nexi;
		po1 += nexo;
		po2 += nexo;
		po3 += nexo;
		po4 += nexo;
	}
	return(HIPS_OK);
}
