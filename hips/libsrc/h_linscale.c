/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_linscale.c - linear scaling
 *
 * input pixel formats: BYTE, SHORT, INT, FLOAT
 * output pixel format: FLOAT
 *
 * Computes opix = b*ipix+c
 *
 * Michael Landy (HIPS 2) - 6/19/91
 */

#include <hipl_format.h>
#define MAXSHORT 32768

static h_boolean balloc = FALSE;
static float *byte_lut;
static float byte_b,byte_c;
static h_boolean salloc = FALSE;
static float *short_lut;
static float short_b,short_c;
int h_linscale_Bs(),h_linscale_Bf(),h_linscale_Ss(),h_linscale_Sf();

int h_linscale(hdi,hdo,b,c)

struct header *hdi,*hdo;
float b,c;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	return(h_linscale_b(hdi,hdo,b,c));
	case PFSHORT:	return(h_linscale_s(hdi,hdo,b,c));
	case PFINT:	return(h_linscale_i(hdi,hdo,b,c));
	case PFFLOAT:	return(h_linscale_f(hdi,hdo,b,c));
	default:	return(perr(HE_FMTSUBR,"h_linscale",
				hformatname(hdi->pixel_format)));
	}
}

int h_linscale_b(hdi,hdo,b,c)

struct header *hdi,*hdo;
float b,c;

{
	return(h_linscale_B(hdi->firstpix,(float *) hdo->firstpix,hdi->rows,
		hdi->cols,hdi->ocols,hdo->ocols,b,c));
}

int h_linscale_s(hdi,hdo,b,c)

struct header *hdi,*hdo;
float b,c;

{
	return(h_linscale_S((short *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,b,c));
}

int h_linscale_i(hdi,hdo,b,c)

struct header *hdi,*hdo;
float b,c;

{
	return(h_linscale_I((int *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,b,c));
}

int h_linscale_f(hdi,hdo,b,c)

struct header *hdi,*hdo;
float b,c;

{
	return(h_linscale_F((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,b,c));
}

int h_linscale_B(imagei,imageo,nr,nc,nlpi,nlpo,b,c)

byte *imagei;
float *imageo;
int nr,nc,nlpi,nlpo;
float b,c;

{
	if ((nr*nc > 256) || (balloc && byte_b == b && byte_c == c))
		return(h_linscale_Bf(imagei,imageo,nr,nc,nlpi,nlpo,b,c));
	else
		return(h_linscale_Bs(imagei,imageo,nr,nc,nlpi,nlpo,b,c));
}

int h_linscale_Bs(imagei,imageo,nr,nc,nlpi,nlpo,b,c)

byte *imagei;
float *imageo;
int nr,nc,nlpi,nlpo;
float b,c;

{
	register int i,j,nexi,nexo;
	register byte *pi;
	float *po;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*po++ = b * (*pi++) + c;
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_linscale_Bf(imagei,imageo,nr,nc,nlpi,nlpo,b,c)

byte *imagei;
float *imageo;
int nr,nc,nlpi,nlpo;
float b,c;

{
	register int i,j,nexi,nexo;
	register byte *pi;
	float *po;

	if (!balloc) {
		if ((byte_lut = (float *) memalloc(256,sizeof(float)))
			== (float *) HIPS_ERROR)
				return(HIPS_ERROR);
		balloc = TRUE;
		byte_b = b + 1; /* force computation */
	}
	if (byte_b != b || byte_c != c) {
		byte_b = b;
		byte_c = c;
		po = byte_lut;
		for (i=0;i<256;i++)
			*po++ = b * i + c;
	}
	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*po++ = byte_lut[*pi++];
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_linscale_S(imagei,imageo,nr,nc,nlpi,nlpo,b,c)

short *imagei;
float *imageo;
int nr,nc,nlpi,nlpo;
float b,c;

{
	if ((nr*nc > MAXSHORT) || (salloc && short_b == b && short_c == c))
		return(h_linscale_Sf(imagei,imageo,nr,nc,nlpi,nlpo,b,c));
	else
		return(h_linscale_Ss(imagei,imageo,nr,nc,nlpi,nlpo,b,c));
}

int h_linscale_Ss(imagei,imageo,nr,nc,nlpi,nlpo,b,c)

short *imagei;
float *imageo;
int nr,nc,nlpi,nlpo;
float b,c;

{
	register int i,j,nexi,nexo;
	register short *pi;
	register float *po;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*po++ = b * (*pi++) + c;
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_linscale_Sf(imagei,imageo,nr,nc,nlpi,nlpo,b,c)

short *imagei;
float *imageo;
int nr,nc,nlpi,nlpo;
float b,c;

{
	register int i,j,nexi,nexo;
	register short *pi;
	register float *po;

	if (!salloc) {
		if ((short_lut = (float *) memalloc(MAXSHORT,sizeof(float)))
			== (float *) HIPS_ERROR)
				return(HIPS_ERROR);
		salloc = TRUE;
		short_b = b + 1; /* force computation */
	}
	if (short_b != b || short_c != c) {
		short_b = b;
		short_c = c;
		po = short_lut;
		for (i = 0; i < MAXSHORT; i++)
			*po++ = b * i + c;
	}
	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			if (*pi < 0)
				*po++ = b * (*pi++) + c;
			else
				*po++ = short_lut[*pi++];
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_linscale_I(imagei,imageo,nr,nc,nlpi,nlpo,b,c)

int *imagei;
float *imageo;
int nr,nc,nlpi,nlpo;
float b,c;

{
	register int i,j,nexi,nexo;
	register int *pi;
	register float *po;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*po++ = b * (*pi++) + c;
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_linscale_F(imagei,imageo,nr,nc,nlpi,nlpo,b,c)

float *imagei,*imageo;
int nr,nc,nlpi,nlpo;
float b,c;

{
	register int i,j,nexi,nexo;
	register float *pi,*po;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*po++ = b * (*pi++) + c;
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}
