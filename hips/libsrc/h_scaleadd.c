/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_scaleadd.c - subroutines to add a scaled version of one image to a 2nd
 *
 * pixel formats: FLOAT
 *
 * Michael Landy - 8/26/91
 */

#include <hipl_format.h>

int h_scaleadd(hdi,hdo,s)

struct header *hdi,*hdo;
float s;

{
	switch(hdi->pixel_format) {
	case PFFLOAT:	return(h_scaleadd_f(hdi,hdo,s));
	default:	return(perr(HE_FMTSUBR,"h_scaleadd",
				hformatname(hdi->pixel_format)));
	}
}

int h_scaleadd_f(hdi,hdo,s)

struct header *hdi,*hdo;
float s;

{
	return(h_scaleadd_F((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,s));
}

int h_scaleadd_F(imagei,imageo,nr,nc,nlpi,nlpo,s)

float *imagei,*imageo;
int nr,nc,nlpi,nlpo;
float s;

{
	register int i,j,nexi,nexo;
	register float *pi,*po;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*po++ += *pi++ * s;
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}
