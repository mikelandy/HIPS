/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_shift.c - subroutines to logically shift image pixels
 *
 * pixel formats: BYTE, INT
 *
 * Michael Landy - 1/11/91
 */

#include <hipl_format.h>

int h_shift(hdi,hdo,shift)

struct header *hdi,*hdo;
int shift;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	return(h_shift_b(hdi,hdo,shift));
	case PFINT:	return(h_shift_i(hdi,hdo,shift));
	default:	return(perr(HE_FMTSUBR,"h_shift",
				hformatname(hdi->pixel_format)));
	}
}

int h_shift_b(hdi,hdo,shift)

struct header *hdi,*hdo;
int shift;

{
	return(h_shift_B(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
		hdi->ocols,hdo->ocols,shift));
}

int h_shift_i(hdi,hdo,shift)

struct header *hdi,*hdo;
int shift;

{
	return(h_shift_I((int *) hdi->firstpix,(int *) hdo->firstpix,hdi->rows,
		hdi->cols,hdi->ocols,hdo->ocols,shift));
}

int h_shift_B(imagei,imageo,nr,nc,nlpi,nlpo,shift)

byte *imagei,*imageo;
int nr,nc,nlpi,nlpo,shift;

{
	register int i,j,nexi,nexo;
	register byte *pi,*po;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	switch (shift) {	/* handle common cases efficiently */
	case 1:
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++)
				*po++ = *pi++ << 1;
			pi += nexi;
			po += nexo;
		}
		break;
	case 2:
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++)
				*po++ = *pi++ << 2;
			pi += nexi;
			po += nexo;
		}
		break;
	case -1:
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++)
				*po++ = *pi++ >> 1;
			pi += nexi;
			po += nexo;
		}
		break;
	case -2:
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++)
				*po++ = *pi++ >> 2;
			pi += nexi;
			po += nexo;
		}
		break;
	default:
		if (shift > 0) {
			for (i=0;i<nr;i++) {
				for (j=0;j<nc;j++)
					*po++ = *pi++ << shift;
				pi += nexi;
				po += nexo;
			}
		}
		else {
			shift = -shift;
			for (i=0;i<nr;i++) {
				for (j=0;j<nc;j++)
					*po++ = *pi++ >> shift;
				pi += nexi;
				po += nexo;
			}
		}
		break;
	}
	return(HIPS_OK);
}

int h_shift_I(imagei,imageo,nr,nc,nlpi,nlpo,shift)

int *imagei,*imageo;
int nr,nc,nlpi,nlpo,shift;

{
	register int i,j,nexi,nexo;
	register int *pi,*po;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	switch (shift) {	/* handle common cases efficiently */
	case 1:
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++)
				*po++ = *pi++ << 1;
			pi += nexi;
			po += nexo;
		}
		break;
	case 2:
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++)
				*po++ = *pi++ << 2;
			pi += nexi;
			po += nexo;
		}
		break;
	case -1:
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++)
				*po++ = *pi++ >> 1;
			pi += nexi;
			po += nexo;
		}
		break;
	case -2:
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++)
				*po++ = *pi++ >> 2;
			pi += nexi;
			po += nexo;
		}
		break;
	default:
		if (shift > 0) {
			for (i=0;i<nr;i++) {
				for (j=0;j<nc;j++)
					*po++ = *pi++ << shift;
				pi += nexi;
				po += nexo;
			}
		}
		else {
			shift = -shift;
			for (i=0;i<nr;i++) {
				for (j=0;j<nc;j++)
					*po++ = *pi++ >> shift;
				pi += nexi;
				po += nexo;
			}
		}
		break;
	}
	return(HIPS_OK);
}
