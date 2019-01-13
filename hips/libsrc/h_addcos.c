/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_addcos.c - subroutines to add a cosine function to an image
 *
 * Units:
 *	xf - horizontal frequency in cycles per frame width
 *	yf - vertical frequency in cycles per frame height
 *	phase - degrees of phase angle (0 = cosine phase at pixel (0,0),
 *					-90 = sine phase at that position)
 *	amplitude - multiplier (peak value)
 *
 * pixel formats: FLOAT
 *
 * Michael Landy - 8/11/91
 */

#include <hipl_format.h>
#include <math.h>

int h_addcos(hdi,hdo,xf,yf,phase,amplitude)

struct header *hdi,*hdo;
float xf,yf,phase,amplitude;

{
	switch(hdi->pixel_format) {
	case PFFLOAT:	return(h_addcos_f(hdi,hdo,xf,yf,phase,amplitude));
	default:	return(perr(HE_FMTSUBR,"h_addcos",
				hformatname(hdi->pixel_format)));
	}
}

int h_addcos_f(hdi,hdo,xf,yf,phase,amplitude)

struct header *hdi,*hdo;
float xf,yf,phase,amplitude;

{
	return(h_addcos_F((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,xf,yf,
		phase,amplitude));
}

int h_addcos_F(imagei,imageo,nr,nc,nlpi,nlpo,xf,yf,phase,amplitude)

float *imagei,*imageo;
int nr,nc,nlpi,nlpo;
float xf,yf,phase,amplitude;

{
	register int i,j,nexi,nexo;
	register float *pi,*po,ycos;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	xf = H_2PI*xf/nc;
	yf = H_2PI*yf/nr;
	phase /= H_180_PI;
	for (i=0;i<nr;i++) {
		ycos = phase + i*yf;
		for (j=0;j<nc;j++) {
			*po++ = *pi++ + amplitude*cos(j*xf + ycos);
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}
