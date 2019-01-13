/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_addgabor.c - subroutines to add a Gabor function to an image
 *
 * Units:
 *	xf - horizontal frequency in cycles per frame width
 *	yf - vertical frequency in cycles per frame height
 *	xm - horizontal mean pixel position
 *	ym - vertical mean pixel position
 *	xs - horizontal standard deviation in pixels
 *	ys - vertical standard deviation in pixels
 *	phase - degrees of phase angle (0 = cosine phase at the mean,
 *					-90 = sine phase at the mean)
 *	amplitude - multiplier (peak value if in cosine phase), this also
 *		allows the calling program to factor in the effects of a
 *		temporal Gaussian
 *
 * pixel formats: FLOAT
 *
 * Michael Landy - 8/11/91
 */

#include <hipl_format.h>
#include <math.h>

int h_addgabor(hdi,hdo,xm,ym,xf,yf,xs,ys,phase,amplitude)

struct header *hdi,*hdo;
float xm,ym,xf,yf,xs,ys,phase,amplitude;

{
	switch(hdi->pixel_format) {
	case PFFLOAT:	return(h_addgabor_f(hdi,hdo,xm,ym,xf,yf,xs,ys,phase,
				amplitude));
	default:	return(perr(HE_FMTSUBR,"h_addgabor",
				hformatname(hdi->pixel_format)));
	}
}

int h_addgabor_f(hdi,hdo,xm,ym,xf,yf,xs,ys,phase,amplitude)

struct header *hdi,*hdo;
float xm,ym,xf,yf,xs,ys,phase,amplitude;

{
	return(h_addgabor_F((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,xm,ym,xf,yf,xs,ys,
		phase,amplitude));
}

int h_addgabor_F(imagei,imageo,nr,nc,nlpi,nlpo,xm,ym,xf,yf,xs,ys,phase,amplitude)

float *imagei,*imageo;
int nr,nc,nlpi,nlpo;
float xm,ym,xf,yf,xs,ys,phase,amplitude;

{
	register int i,j,nexi,nexo;
	register float *pi,*po,xssq,yssq,x,y,yexp,ycos;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	xf = H_2PI*xf/nc;
	yf = H_2PI*yf/nr;
	xssq = 2.*xs*xs;
	yssq = 2.*ys*ys;
	phase /= H_180_PI;
	for (i=0;i<nr;i++) {
		y = i - ym;
		yexp = y*y/yssq;
		ycos = phase + y*yf;
		for (j=0;j<nc;j++) {
			x = j - xm;
			*po++ = *pi++ +
				amplitude*exp(-((x*x/xssq)+yexp))
				* cos(x*xf + ycos);
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}
