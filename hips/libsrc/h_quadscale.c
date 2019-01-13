/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_quadscale.c - quadratic scaling
 *
 * input pixel formats: BYTE, SHORT, INT, FLOAT
 * output pixel format: FLOAT
 *
 * Computes opix = a*ipix*ipix+b*ipix+c
 *
 * Michael Landy (HIPS 2) - 6/19/91
 */

#include <hipl_format.h>
#define MAXSHORT 32768

static h_boolean balloc = FALSE;
static float *byte_lut;
static float byte_a,byte_b,byte_c;
static h_boolean salloc = FALSE;
static float *short_lut;
static float short_a,short_b,short_c;
int h_quadscale_Bs(),h_quadscale_Bf(),h_quadscale_Ss(),h_quadscale_Sf();

int h_quadscale(hdi,hdo,a,b,c)

struct header *hdi,*hdo;
float a,b,c;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	return(h_quadscale_b(hdi,hdo,a,b,c));
	case PFSHORT:	return(h_quadscale_s(hdi,hdo,a,b,c));
	case PFINT:	return(h_quadscale_i(hdi,hdo,a,b,c));
	case PFFLOAT:	return(h_quadscale_f(hdi,hdo,a,b,c));
	default:	return(perr(HE_FMTSUBR,"h_quadscale",
				hformatname(hdi->pixel_format)));
	}
}

int h_quadscale_b(hdi,hdo,a,b,c)

struct header *hdi,*hdo;
float a,b,c;

{
	return(h_quadscale_B(hdi->firstpix,(float *) hdo->firstpix,hdi->rows,
		hdi->cols,hdi->ocols,hdo->ocols,a,b,c));
}

int h_quadscale_s(hdi,hdo,a,b,c)

struct header *hdi,*hdo;
float a,b,c;

{
	return(h_quadscale_S((short *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,a,b,c));
}

int h_quadscale_i(hdi,hdo,a,b,c)

struct header *hdi,*hdo;
float a,b,c;

{
	return(h_quadscale_I((int *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,a,b,c));
}

int h_quadscale_f(hdi,hdo,a,b,c)

struct header *hdi,*hdo;
float a,b,c;

{
	return(h_quadscale_F((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,a,b,c));
}

int h_quadscale_B(imagei,imageo,nr,nc,nlpi,nlpo,a,b,c)

byte *imagei;
float *imageo;
int nr,nc,nlpi,nlpo;
float a,b,c;

{
	if ((nr*nc > 256) || (balloc && byte_a == a && byte_b == b
	    && byte_c == c))
		return(h_quadscale_Bf(imagei,imageo,nr,nc,nlpi,nlpo,a,b,c));
	else
		return(h_quadscale_Bs(imagei,imageo,nr,nc,nlpi,nlpo,a,b,c));
}

int h_quadscale_Bs(imagei,imageo,nr,nc,nlpi,nlpo,a,b,c)

byte *imagei;
float *imageo;
int nr,nc,nlpi,nlpo;
float a,b,c;

{
	register int i,j,nexi,nexo;
	register byte *pi,val;
	float *po;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			val = *pi++;
			*po++ = a*val*val + b*val + c;
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_quadscale_Bf(imagei,imageo,nr,nc,nlpi,nlpo,a,b,c)

byte *imagei;
float *imageo;
int nr,nc,nlpi,nlpo;
float a,b,c;

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
	if (byte_a != a || byte_b != b || byte_c != c) {
		byte_a = a;
		byte_b = b;
		byte_c = c;
		po = byte_lut;
		for (i=0;i<256;i++)
			*po++ = a*i*i + b*i + c;
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

int h_quadscale_S(imagei,imageo,nr,nc,nlpi,nlpo,a,b,c)

short *imagei;
float *imageo;
int nr,nc,nlpi,nlpo;
float a,b,c;

{
	if ((nr*nc > MAXSHORT) || (salloc && short_a == a && short_b == b
	    && short_c == c))
		return(h_quadscale_Sf(imagei,imageo,nr,nc,nlpi,nlpo,a,b,c));
	else
		return(h_quadscale_Ss(imagei,imageo,nr,nc,nlpi,nlpo,a,b,c));
}

int h_quadscale_Ss(imagei,imageo,nr,nc,nlpi,nlpo,a,b,c)

short *imagei;
float *imageo;
int nr,nc,nlpi,nlpo;
float a,b,c;

{
	register int i,j,nexi,nexo;
	register short *pi,val;
	register float *po;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			val = *pi++;
			*po++ = a*val*val + b*val + c;
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_quadscale_Sf(imagei,imageo,nr,nc,nlpi,nlpo,a,b,c)

short *imagei;
float *imageo;
int nr,nc,nlpi,nlpo;
float a,b,c;

{
	register int i,j,nexi,nexo;
	register short *pi,val;
	register float *po;

	if (!salloc) {
		if ((short_lut = (float *) memalloc(MAXSHORT,sizeof(float)))
			== (float *) HIPS_ERROR)
				return(HIPS_ERROR);
		salloc = TRUE;
		short_b = b + 1; /* force computation */
	}
	if (short_a != a || short_b != b || short_c != c) {
		short_a = a;
		short_b = b;
		short_c = c;
		po = short_lut;
		for (i = 0; i < MAXSHORT; i++)
			*po++ = a*i*i + b*i + c;
	}
	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			val = *pi++;
			if (val < 0)
				*po++ = a*val*val + b*val + c;
			else
				*po++ = short_lut[val];
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_quadscale_I(imagei,imageo,nr,nc,nlpi,nlpo,a,b,c)

int *imagei;
float *imageo;
int nr,nc,nlpi,nlpo;
float a,b,c;

{
	register int i,j,nexi,nexo;
	register int *pi,val;
	register float *po;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			val = *pi++;
			*po++ = a*val*val + b*val + c;
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_quadscale_F(imagei,imageo,nr,nc,nlpi,nlpo,a,b,c)

float *imagei,*imageo;
int nr,nc,nlpi,nlpo;
float a,b,c;

{
	register int i,j,nexi,nexo;
	register float *pi,*po,val;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			val = *pi++;
			*po++ = a*val*val + b*val + c;
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}
