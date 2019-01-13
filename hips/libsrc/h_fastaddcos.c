/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_fastaddcos.c - subroutines to add a cosine function to an image quickly
 *
 * Units:
 *	xf - horizontal frequency in cycles per frame width
 *	yf - vertical frequency in cycles per frame height
 *	phase - degrees of phase angle (0 = cosine phase at pixel (0,0),
 *					-90 = sine phase at that position)
 *	amplitude - multiplier (peak value)
 *
 * The number of rows and columns must evenly divide 512.
 *
 * pixel formats: FLOAT
 *
 * Charlie Chubb - 12/5/86
 * HIPS 2 - msl - 8/12/91
 */

#include <hipl_format.h>
#include <trig512.h>

int h_fastaddcos(hdi,hdo,xf,yf,phase,amplitude)

struct header *hdi,*hdo;
int xf,yf;
float phase,amplitude;

{
	switch(hdi->pixel_format) {
	case PFFLOAT:	return(h_fastaddcos_f(hdi,hdo,xf,yf,phase,amplitude));
	default:	return(perr(HE_FMTSUBR,"h_fastaddcos",
				hformatname(hdi->pixel_format)));
	}
}

int h_fastaddcos_f(hdi,hdo,xf,yf,phase,amplitude)

struct header *hdi,*hdo;
int xf,yf;
float phase,amplitude;

{
	return(h_fastaddcos_F((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,xf,yf,
		phase,amplitude));
}

int h_fastaddcos_F(imagei,imageo,nr,nc,nlpi,nlpo,xf,yf,phase,amplitude)

float *imagei,*imageo;
int nr,nc,nlpi,nlpo,xf,yf;
float phase,amplitude;

{
	register int i,j,nexi,nexo,ph,ndx;
	register float *pi,*po;

	if (512%nc || 512%nr)
		return(perr(HE_POW2));
	yf *= 512/nr;
	xf *= 512/nc;
	ph = ((int)(phase*512./360.)) & 511;
	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	ndx = ph;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			*po++ = *pi++ + amplitude*COS(ndx);
			ndx += xf;
		}
		pi += nexi;
		po += nexo;
		ndx += yf;
	}
	return(HIPS_OK);
}
