/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_rot90.c - subroutines to rotate an image by 90 degrees
 *
 * If dirflag is TRUE then rotate counterclockwise, else clockwise.
 *
 * pixel formats: BYTE, INT, FLOAT
 *
 * Michael Landy - 1/11/91
 */

#include <hipl_format.h>

int h_rot90(hdi,hdo,dirflag)

struct header *hdi,*hdo;
h_boolean dirflag;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	return(h_rot90_b(hdi,hdo,dirflag));
	case PFINT:	return(h_rot90_i(hdi,hdo,dirflag));
	case PFFLOAT:	return(h_rot90_f(hdi,hdo,dirflag));
	default:	return(perr(HE_FMTSUBR,"h_rot90",
				hformatname(hdi->pixel_format)));
	}
}

int h_rot90_b(hdi,hdo,dirflag)

struct header *hdi,*hdo;
h_boolean dirflag;

{
	return(h_rot90_B(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
		hdi->ocols,hdo->ocols,dirflag));
}

int h_rot90_i(hdi,hdo,dirflag)

struct header *hdi,*hdo;
h_boolean dirflag;

{
	return(h_rot90_I((int *) hdi->firstpix,(int *) hdo->firstpix,hdi->rows,
		hdi->cols,hdi->ocols,hdo->ocols,dirflag));
}

int h_rot90_f(hdi,hdo,dirflag)

struct header *hdi,*hdo;
h_boolean dirflag;

{
	return(h_rot90_F((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,dirflag));
}

int h_rot90_B(imagei,imageo,nr,nc,nlpi,nlpo,dirflag)

byte *imagei,*imageo;
int nr,nc,nlpi,nlpo;
h_boolean dirflag;

{
	register int i,j,nexi,nexo,incr;
	register byte *pi,*po;

	nexi = nlpi-nc;
	pi = imagei;
#ifdef ULORIG
	if (!dirflag) {	/* clockwise */
#else
	if (dirflag) {	/* counter-clockwise */
#endif
		po = imageo + nr - 1;
		incr = nlpo;
		nexo = -(1 + nlpo*nc);
	}
	else {
		po = imageo + (nc-1)*nlpo;
		incr = -nlpo;
		nexo = 1 + nlpo*nc;
	}
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			*po = *pi++;
			po += incr;
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_rot90_I(imagei,imageo,nr,nc,nlpi,nlpo,dirflag)

int *imagei,*imageo;
int nr,nc,nlpi,nlpo;
h_boolean dirflag;

{
	register int i,j,nexi,nexo,incr;
	register int *pi,*po;

	nexi = nlpi-nc;
	pi = imagei;
#ifdef ULORIG
	if (!dirflag) {	/* clockwise */
#else
	if (dirflag) {	/* counter-clockwise */
#endif
		po = imageo + nr - 1;
		incr = nlpo;
		nexo = -(1 + nlpo*nc);
	}
	else {
		po = imageo + (nc-1)*nlpo;
		incr = -nlpo;
		nexo = 1 + nlpo*nc;
	}
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			*po = *pi++;
			po += incr;
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_rot90_F(imagei,imageo,nr,nc,nlpi,nlpo,dirflag)

float *imagei,*imageo;
int nr,nc,nlpi,nlpo;
h_boolean dirflag;

{
	register int i,j,nexi,nexo,incr;
	register float *pi,*po;

	nexi = nlpi-nc;
	pi = imagei;
#ifdef ULORIG
	if (!dirflag) {	/* clockwise */
#else
	if (dirflag) {	/* counter-clockwise */
#endif
		po = imageo + nr - 1;
		incr = nlpo;
		nexo = -(1 + nlpo*nc);
	}
	else {
		po = imageo + (nc-1)*nlpo;
		incr = -nlpo;
		nexo = 1 + nlpo*nc;
	}
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			*po = *pi++;
			po += incr;
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}
