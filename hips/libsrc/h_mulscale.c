/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_mulscale.c - multiply by a fixed scale factor
 *
 * input pixel formats: BYTE, SHORT, INT, FLOAT, DOUBLE
 * output pixel format: same as input
 *
 * Computes opix = ipix*b
 *
 * Michael Landy (HIPS 2) - 7/7/91
 */

#include <hipl_format.h>
#define MAXSHORT 32768

static h_boolean balloc = FALSE;
static byte *byte_lut;
static byte byte_b;
static h_boolean salloc = FALSE;
static short *short_lut;
static short short_b;
int h_mulscale_Bs(),h_mulscale_Bf(),h_mulscale_Ss(),h_mulscale_Sf();

int h_mulscale(hdi,hdo,b)

struct header *hdi,*hdo;
Pixelval *b;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	return(h_mulscale_b(hdi,hdo,b));
	case PFSHORT:	return(h_mulscale_s(hdi,hdo,b));
	case PFINT:	return(h_mulscale_i(hdi,hdo,b));
	case PFFLOAT:	return(h_mulscale_f(hdi,hdo,b));
	case PFDOUBLE:	return(h_mulscale_d(hdi,hdo,b));
	default:	return(perr(HE_FMTSUBR,"h_mulscale",
				hformatname(hdi->pixel_format)));
	}
}

int h_mulscale_b(hdi,hdo,b)

struct header *hdi,*hdo;
Pixelval *b;

{
	return(h_mulscale_B(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
		hdi->ocols,hdo->ocols,b->v_byte));
}

int h_mulscale_s(hdi,hdo,b)

struct header *hdi,*hdo;
Pixelval *b;

{
	return(h_mulscale_S((short *) hdi->firstpix,(short *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,b->v_short));
}

int h_mulscale_i(hdi,hdo,b)

struct header *hdi,*hdo;
Pixelval *b;

{
	return(h_mulscale_I((int *) hdi->firstpix,(int *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,b->v_int));
}

int h_mulscale_f(hdi,hdo,b)

struct header *hdi,*hdo;
Pixelval *b;

{
	return(h_mulscale_F((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,b->v_float));
}

int h_mulscale_d(hdi,hdo,b)

struct header *hdi,*hdo;
Pixelval *b;

{
	return(h_mulscale_D((double *) hdi->firstpix,(double *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,b->v_double));
}

int h_mulscale_B(imagei,imageo,nr,nc,nlpi,nlpo,b)

byte *imagei,*imageo;
int nr,nc,nlpi,nlpo;
byte b;

{
	if ((nr*nc > 256) || (balloc && byte_b == b))
		return(h_mulscale_Bf(imagei,imageo,nr,nc,nlpi,nlpo,b));
	else
		return(h_mulscale_Bs(imagei,imageo,nr,nc,nlpi,nlpo,b));
}

int h_mulscale_Bs(imagei,imageo,nr,nc,nlpi,nlpo,b)

byte *imagei,*imageo;
int nr,nc,nlpi,nlpo;
byte b;

{
	register int i,j,nexi,nexo,val,ib;
	register byte *pi,*po;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	ib = b;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			val = (*pi++) * ib;
			*po++ = (val < 0) ? 0 : ((val > 255) ? 255 : val);
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_mulscale_Bf(imagei,imageo,nr,nc,nlpi,nlpo,b)

byte *imagei,*imageo;
int nr,nc,nlpi,nlpo;
byte b;

{
	register int i,j,nexi,nexo,val,ib;
	register byte *pi,*po;

	if (!balloc) {
		if ((byte_lut = (byte *) memalloc(256,sizeof(byte)))
			== (byte *) HIPS_ERROR)
				return(HIPS_ERROR);
		balloc = TRUE;
		byte_b = b + 1; /* force computation */
	}
	if (byte_b != b) {
		ib = byte_b = b;
		po = byte_lut;
		for (i=0;i<256;i++) {
			val = i * ib;
			*po++ = (val < 0) ? 0 : ((val > 255) ? 255 : val);
		}
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

int h_mulscale_S(imagei,imageo,nr,nc,nlpi,nlpo,b)

short *imagei,*imageo;
int nr,nc,nlpi,nlpo;
short b;

{
	if ((nr*nc > MAXSHORT) || (salloc && short_b == b))
		return(h_mulscale_Sf(imagei,imageo,nr,nc,nlpi,nlpo,b));
	else
		return(h_mulscale_Ss(imagei,imageo,nr,nc,nlpi,nlpo,b));
}

int h_mulscale_Ss(imagei,imageo,nr,nc,nlpi,nlpo,b)

short *imagei,*imageo;
int nr,nc,nlpi,nlpo;
short b;

{
	register int i,j,nexi,nexo;
	register short *pi,*po;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*po++ = (*pi++)*b;
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_mulscale_Sf(imagei,imageo,nr,nc,nlpi,nlpo,b)

short *imagei,*imageo;
int nr,nc,nlpi,nlpo;
short b;

{
	register int i,j,nexi,nexo;
	register short *pi,*po;

	if (!salloc) {
		if ((short_lut = (short *) memalloc(MAXSHORT,sizeof(short)))
			== (short *) HIPS_ERROR)
				return(HIPS_ERROR);
		salloc = TRUE;
		short_b = b + 1; /* force computation */
	}
	if (short_b != b) {
		short_b = b;
		po = short_lut;
		for (i = 0; i < MAXSHORT; i++)
			*po++ = i*b;
	}
	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			if (*pi < 0)
				*po++ = (*pi++)*b;
			else
				*po++ = short_lut[*pi++];
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_mulscale_I(imagei,imageo,nr,nc,nlpi,nlpo,b)

int *imagei,*imageo;
int nr,nc,nlpi,nlpo;
int b;

{
	register int i,j,nexi,nexo;
	register int *pi,*po;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*po++ = (*pi++)*b;
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_mulscale_F(imagei,imageo,nr,nc,nlpi,nlpo,b)

float *imagei,*imageo;
int nr,nc,nlpi,nlpo;
float b;

{
	register int i,j,nexi,nexo;
	register float *pi,*po;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*po++ = (*pi++)*b;
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_mulscale_D(imagei,imageo,nr,nc,nlpi,nlpo,b)

double *imagei,*imageo;
int nr,nc,nlpi,nlpo;
double b;

{
	register int i,j,nexi,nexo;
	register double *pi,*po;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*po++ = (*pi++)*b;
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}
