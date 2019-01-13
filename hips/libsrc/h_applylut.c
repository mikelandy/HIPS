/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_applylut.c - subroutines to apply a lookup table to an image
 *
 * pixel formats: BYTE, SHORT, INT
 *
 * Michael Landy - 7/17/91
 */

#include <hipl_format.h>

int h_applylut(hdi,hdo,count,lut)

struct header *hdi,*hdo;
int count;
byte *lut;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	return(h_applylut_b(hdi,hdo,count,lut));
	case PFSHORT:	return(h_applylut_s(hdi,hdo,count,(short *) lut));
	case PFINT:	return(h_applylut_i(hdi,hdo,count,(int *) lut));
	default:	return(perr(HE_FMTSUBR,"h_applylut",
				hformatname(hdi->pixel_format)));
	}
}

int h_applylut_b(hdi,hdo,count,lut)

struct header *hdi,*hdo;
int count;
byte *lut;

{
	return(h_applylut_B(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
		hdi->ocols,hdo->ocols,count,lut));
}

int h_applylut_s(hdi,hdo,count,lut)

struct header *hdi,*hdo;
int count;
short *lut;

{
	return(h_applylut_S((short *) hdi->firstpix,(short *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,count,lut));
}

int h_applylut_i(hdi,hdo,count,lut)

struct header *hdi,*hdo;
int count,*lut;

{
	return(h_applylut_I((int *) hdi->firstpix,(int *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,count,lut));
}

int h_applylut_B(imagei,imageo,nr,nc,nlpi,nlpo,count,lut)

byte *imagei,*imageo;
int nr,nc,nlpi,nlpo,count;
byte *lut;

{
	register int i,j,nexi,nexo;
	register byte *pi,*po;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	hips_lclip = hips_hclip = 0;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			if (*pi >= count) {
				hips_hclip++;
				*po++ = lut[count-1];
				pi++;
			}
			else
				*po++ = lut[*pi++];
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_applylut_S(imagei,imageo,nr,nc,nlpi,nlpo,count,lut)

short *imagei,*imageo;
int nr,nc,nlpi,nlpo,count;
short *lut;

{
	register int i,j,nexi,nexo;
	register short *pi,*po;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	hips_lclip = hips_hclip = 0;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			if (*pi < 0) {
				hips_lclip++;
				*po++ = lut[0];
				pi++;
			}
			else if (*pi >= count) {
				hips_hclip++;
				*po++ = lut[count-1];
				pi++;
			}
			else
				*po++ = lut[*pi++];
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_applylut_I(imagei,imageo,nr,nc,nlpi,nlpo,count,lut)

int *imagei,*imageo;
int nr,nc,nlpi,nlpo,count;
int *lut;

{
	register int i,j,nexi,nexo;
	register int *pi,*po;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	hips_lclip = hips_hclip = 0;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			if (*pi < 0) {
				hips_lclip++;
				*po++ = lut[0];
				pi++;
			}
			else if (*pi >= count) {
				hips_hclip++;
				*po++ = lut[count-1];
				pi++;
			}
			else
				*po++ = lut[*pi++];
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}
