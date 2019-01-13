/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_rot180.c - subroutines to rotate an image by 180 degrees
 *
 * pixel formats: BYTE, INT, FLOAT
 *
 * Michael Landy - 6/21/91
 */

#include <hipl_format.h>

int h_rot180(hdi,hdo)

struct header *hdi,*hdo;
{
	switch(hdi->pixel_format) {
	case PFBYTE:	return(h_rot180_b(hdi,hdo));
	case PFINT:	return(h_rot180_i(hdi,hdo));
	case PFFLOAT:	return(h_rot180_f(hdi,hdo));
	default:	return(perr(HE_FMTSUBR,"h_rot180",
				hformatname(hdi->pixel_format)));
	}
}

int h_rot180_b(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_rot180_B(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
		hdi->ocols,hdo->ocols));
}

int h_rot180_i(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_rot180_I((int *) hdi->firstpix,(int *) hdo->firstpix,hdi->rows,
		hdi->cols,hdi->ocols,hdo->ocols));
}

int h_rot180_f(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_rot180_F((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols));
}

int h_rot180_B(imagei,imageo,nr,nc,nlpi,nlpo)

byte *imagei,*imageo;
int nr,nc,nlpi,nlpo;

{
	register int i,j,nexi,nexo;
	register byte *pi,*po;

	nexi = nlpi-nc;
	pi = imagei;
	po = imageo + (nr-1)*nlpo + (nc-1);
	nexo = -(nlpo-nc);
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*po-- = *pi++;
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_rot180_I(imagei,imageo,nr,nc,nlpi,nlpo)

int *imagei,*imageo;
int nr,nc,nlpi,nlpo;

{
	register int i,j,nexi,nexo;
	register int *pi,*po;

	nexi = nlpi-nc;
	pi = imagei;
	po = imageo + (nr-1)*nlpo + (nc-1);
	nexo = -(nlpo-nc);
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*po-- = *pi++;
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_rot180_F(imagei,imageo,nr,nc,nlpi,nlpo)

float *imagei,*imageo;
int nr,nc,nlpi,nlpo;

{
	register int i,j,nexi,nexo;
	register float *pi,*po;

	nexi = nlpi-nc;
	pi = imagei;
	po = imageo + (nr-1)*nlpo + (nc-1);
	nexo = -(nlpo-nc);
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*po-- = *pi++;
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}
