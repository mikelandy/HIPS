/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_power.c - raise to a power, and if PFBYTE then normalize
 *
 * input pixel formats: BYTE, SHORT, INT, FLOAT
 * output pixel formats: BYTE (for BYTE input), FLOAT (for others)
 *
 * For byte images, pixels are renormalized to lie between 0 and 255.  For
 * short, integer and float images, the output is a float image and no
 * renormalization is performed.
 *
 * Yoav Cohen 2/16/82
 * added int/float - Mike Landy - 3/16/89
 * modified to use look-up table for byte and short images:
 *     Brian Tierney, LBL 10/90
 * HIPS 2 - msl - 1/10/91
 */

#include <hipl_format.h>
#include <math.h>
#define MY_MAXSHORT 32768

static h_boolean balloc = FALSE;
static byte *byte_lut;
static double byte_power;
static h_boolean salloc = FALSE;
static float *short_lut;
static double short_power;
int h_power_Bs(),h_power_Bf(),h_power_Ss(),h_power_Sf();

int h_power(hdi,hdo,power)

struct header *hdi,*hdo;
double power;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	return(h_power_b(hdi,hdo,power));
	case PFSHORT:	return(h_power_s(hdi,hdo,power));
	case PFINT:	return(h_power_i(hdi,hdo,power));
	case PFFLOAT:	return(h_power_f(hdi,hdo,power));
	default:	return(perr(HE_FMTSUBR,"h_power",
				hformatname(hdi->pixel_format)));
	}
}

int h_power_b(hdi,hdo,power)

struct header *hdi,*hdo;
double power;

{
	return(h_power_B(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
		hdi->ocols,hdo->ocols,power));
}

int h_power_s(hdi,hdo,power)

struct header *hdi,*hdo;
double power;

{
	return(h_power_S((short *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,power));
}

int h_power_i(hdi,hdo,power)

struct header *hdi,*hdo;
double power;

{
	return(h_power_I((int *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,power));
}

int h_power_f(hdi,hdo,power)

struct header *hdi,*hdo;
double power;

{
	return(h_power_F((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,power));
}

int h_power_B(imagei,imageo,nr,nc,nlpi,nlpo,power)

byte *imagei,*imageo;
int nr,nc,nlpi,nlpo;
double power;

{
	if ((nr*nc > 256) || (balloc && power==byte_power))
		return(h_power_Bf(imagei,imageo,nr,nc,nlpi,nlpo,power));
	else
		return(h_power_Bs(imagei,imageo,nr,nc,nlpi,nlpo,power));
}

int h_power_Bs(imagei,imageo,nr,nc,nlpi,nlpo,power)

byte *imagei,*imageo;
int nr,nc,nlpi,nlpo;
double power;

{
	register int i,j,nexi,nexo;
	register byte *pi,*po;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			if (*pi == 0) {
				*po++ = 0;
				pi++;
			}
			else
				*po++ = (byte) (255. *
				    (pow((double) *pi++/255.,power)) + 0.5);
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_power_Bf(imagei,imageo,nr,nc,nlpi,nlpo,power)

byte *imagei,*imageo;
int nr,nc,nlpi,nlpo;
double power;

{
	register int i,j,nexi,nexo;
	register byte *pi,*po;

	if (!balloc) {
		if ((byte_lut = (byte *) memalloc(256,sizeof(byte)))
			== (byte *) HIPS_ERROR)
				return(HIPS_ERROR);
		balloc = TRUE;
		byte_power = power+1; /* force computation */
	}
	if (byte_power != power) {
		byte_power = power;
		po = byte_lut;
		*po++ = 0.;
		for (i=1;i<256;i++)
			*po++ = (byte) (255. *
			    (pow((double) i/255.,power)) + 0.5);
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

int h_power_S(imagei,imageo,nr,nc,nlpi,nlpo,power)

short *imagei;
float *imageo;
int nr,nc,nlpi,nlpo;
double power;

{
	if ((nr*nc > MY_MAXSHORT) || (salloc && short_power==power))
		return(h_power_Sf(imagei,imageo,nr,nc,nlpi,nlpo,power));
	else
		return(h_power_Ss(imagei,imageo,nr,nc,nlpi,nlpo,power));
}

int h_power_Ss(imagei,imageo,nr,nc,nlpi,nlpo,power)

short *imagei;
float *imageo;
int nr,nc,nlpi,nlpo;
double power;

{
	register int i,j,nexi,nexo;
	register short *pi;
	register float *po;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			if (*pi == 0) {
				*po++ = 0.;
				pi++;
			}
			else
				*po++ = pow((double) *pi++,power);
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_power_Sf(imagei,imageo,nr,nc,nlpi,nlpo,power)

short *imagei;
float *imageo;
int nr,nc,nlpi,nlpo;
double power;

{
	register int i,j,nexi,nexo;
	register short *pi;
	register float *po;

	if (!salloc) {
		if ((short_lut = (float *) memalloc(MY_MAXSHORT,sizeof(float)))
			== (float *) HIPS_ERROR)
				return(HIPS_ERROR);
		salloc = TRUE;
		short_power = power + 1; /* force computation */
	}
	if (short_power != power) {
		short_power = power;
		po = short_lut;
		*po++ = 0.;
		for (i = 1; i < MY_MAXSHORT; i++)
			*po++ = (float) (pow((double) i,power));
	}
	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			if (*pi >= 0)
				*po++ = short_lut[*pi++];
			else
				*po++ = pow((double) *pi++,power);
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_power_I(imagei,imageo,nr,nc,nlpi,nlpo,power)

int *imagei;
float *imageo;
int nr,nc,nlpi,nlpo;
double power;

{
	register int i,j,nexi,nexo;
	register int *pi;
	register float *po;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			if (*pi == 0) {
				*po++ = 0.;
				pi++;
			}
			else
				*po++ = pow((double) *pi++,power);
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_power_F(imagei,imageo,nr,nc,nlpi,nlpo,power)

float *imagei,*imageo;
int nr,nc,nlpi,nlpo;
double power;

{
	register int i,j,nexi,nexo;
	register float *pi,*po;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			if (*pi == 0.) {
				*po++ = 0.;
				pi++;
			}
			else
				*po++ = pow((double) *pi++,power);
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}
