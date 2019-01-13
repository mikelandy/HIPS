/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_log.c - natural logarithm
 *
 * input pixel formats: BYTE, SHORT, INT, FLOAT
 * output pixel formats: FLOAT (for others)
 *
 * Computes log(pixel+offset), and sets log(x)=-999999 if x <= 0.
 *
 * Mike Landy - 5/10/82
 * Mike Landy - 5/17/85 - added float input
 * Charles Carman - 12/11/87 - added short input
 * HIPS 2 - msl - 6/13/91
 */

#include <hipl_format.h>
#include <math.h>
#define MY_MAXSHORT 32768

static h_boolean balloc = FALSE;
static float *byte_lut;
static double byte_offset;
static h_boolean salloc = FALSE;
static float *short_lut;
static double short_offset;
int h_log_Bs(),h_log_Bf(),h_log_Ss(),h_log_Sf(),h_log_Is(),h_log_If();

int h_log(hdi,hdo,offset)

struct header *hdi,*hdo;
double offset;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	return(h_log_b(hdi,hdo,offset));
	case PFSHORT:	return(h_log_s(hdi,hdo,offset));
	case PFINT:	return(h_log_i(hdi,hdo,offset));
	case PFFLOAT:	return(h_log_f(hdi,hdo,offset));
	default:	return(perr(HE_FMTSUBR,"h_log",
				hformatname(hdi->pixel_format)));
	}
}

int h_log_b(hdi,hdo,offset)

struct header *hdi,*hdo;
double offset;

{
	return(h_log_B(hdi->firstpix,(float *) hdo->firstpix,hdi->rows,
		hdi->cols,hdi->ocols,hdo->ocols,offset));
}

int h_log_s(hdi,hdo,offset)

struct header *hdi,*hdo;
double offset;

{
	return(h_log_S((short *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,offset));
}

int h_log_i(hdi,hdo,offset)

struct header *hdi,*hdo;
double offset;

{
	return(h_log_I((int *) hdi->firstpix,(float *) hdo->firstpix,hdi->rows,
		hdi->cols,hdi->ocols,hdo->ocols,offset));
}

int h_log_f(hdi,hdo,offset)

struct header *hdi,*hdo;
double offset;

{
	return(h_log_F((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,offset));
}

int h_log_B(imagei,imageo,nr,nc,nlpi,nlpo,offset)

byte *imagei;
float *imageo;
int nr,nc,nlpi,nlpo;
double offset;

{
	if ((nr*nc > 256) || (balloc && (byte_offset == offset)))
		return(h_log_Bf(imagei,imageo,nr,nc,nlpi,nlpo,offset));
	else
		return(h_log_Bs(imagei,imageo,nr,nc,nlpi,nlpo,offset));
}

int h_log_Bs(imagei,imageo,nr,nc,nlpi,nlpo,offset)

byte *imagei;
float *imageo;
int nr,nc,nlpi,nlpo;
double offset;

{
	register int i,j,nexi,nexo;
	register byte *pi;
	float *po;
	double x;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			x = *pi++ + offset;
			if (x <= 0)
				*po++ = -999999.;
			else
				*po++ = log(x);
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_log_Bf(imagei,imageo,nr,nc,nlpi,nlpo,offset)

byte *imagei;
float *imageo;
int nr,nc,nlpi,nlpo;
double offset;

{
	register int i,j,nexi,nexo;
	register byte *pi;
	float *po;
	double x;

	if (!balloc) {
		if ((byte_lut = (float *) memalloc(256,sizeof(float)))
			== (float *) HIPS_ERROR)
				return(HIPS_ERROR);
		byte_offset = offset + 1;	/* force computation */
		balloc = TRUE;
	}
	if (byte_offset != offset) {
		byte_offset = offset;
		po = byte_lut;
		for (i=0;i<256;i++) {
			x = offset + i;
			if (x <= 0)
				*po++ = -999999.;
			else
				*po++ = log(x);
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

int h_log_S(imagei,imageo,nr,nc,nlpi,nlpo,offset)

short *imagei;
float *imageo;
int nr,nc,nlpi,nlpo;
double offset;

{
	if ((nr*nc > MY_MAXSHORT) || (salloc && (short_offset == offset)))
		return(h_log_Sf(imagei,imageo,nr,nc,nlpi,nlpo,offset));
	else
		return(h_log_Ss(imagei,imageo,nr,nc,nlpi,nlpo,offset));
}

int h_log_Ss(imagei,imageo,nr,nc,nlpi,nlpo,offset)

short *imagei;
float *imageo;
int nr,nc,nlpi,nlpo;
double offset;

{
	register int i,j,nexi,nexo;
	register short *pi;
	register float *po;
	double x;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			x = *pi++ + offset;
			if (x <= 0)
				*po++ = -999999.;
			else
				*po++ = log(x);
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_log_Sf(imagei,imageo,nr,nc,nlpi,nlpo,offset)

short *imagei;
float *imageo;
int nr,nc,nlpi,nlpo;
double offset;

{
	register int i,j,nexi,nexo;
	register short *pi;
	register float *po;
	double x;

	if (!salloc) {
		if ((short_lut = (float *) memalloc(MY_MAXSHORT,sizeof(float)))
			== (float *) HIPS_ERROR)
				return(HIPS_ERROR);
		short_offset = offset + 1;	/* force computation */
		salloc = TRUE;
	}
	if (short_offset != offset) {
		short_offset = offset;
		po = short_lut;
		for (i=0;i<MY_MAXSHORT;i++) {
			x = offset + i;
			if (x <= 0)
				*po++ = -999999.;
			else
				*po++ = log(x);
		}
	}
	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			if (*pi >= 0)
				*po++ = short_lut[*pi++];
			else {
				x = *pi++ + offset;
				if (x <= 0)
					*po++ = -999999.;
				else
					*po++ = log(x);
			}
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_log_I(imagei,imageo,nr,nc,nlpi,nlpo,offset)

int *imagei;
float *imageo;
int nr,nc,nlpi,nlpo;
double offset;

{
	if (salloc && (short_offset == offset))
		return(h_log_If(imagei,imageo,nr,nc,nlpi,nlpo,offset));
	else
		return(h_log_Is(imagei,imageo,nr,nc,nlpi,nlpo,offset));
}

int h_log_Is(imagei,imageo,nr,nc,nlpi,nlpo,offset)

int *imagei;
float *imageo;
int nr,nc,nlpi,nlpo;
double offset;

{
	register int i,j,nexi,nexo;
	register int *pi;
	register float *po;
	double x;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			x = offset + *pi++;
			if (x <= 0)
				*po++ = -999999.;
			else
				*po++ = log(x);
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_log_If(imagei,imageo,nr,nc,nlpi,nlpo,offset)

int *imagei;
float *imageo;
int nr,nc,nlpi,nlpo;
double offset;

{
	register int i,j,nexi,nexo;
	register int *pi;
	register float *po;
	double x;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			if (*pi <= 0) {
				*po++ = -999999.;
				pi++;
			}
			else if (*pi >= MY_MAXSHORT) {
				x = *pi++ + offset;
				if (x <= 0)
					*po++ = -999999.;
				else
					*po++ = log(x);
			}
			else
				*po++ = short_lut[*pi++];
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_log_F(imagei,imageo,nr,nc,nlpi,nlpo,offset)

float *imagei,*imageo;
int nr,nc,nlpi,nlpo;
double offset;

{
	register int i,j,nexi,nexo;
	register float *pi,*po;
	double x;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			x = *pi++ + offset;
			if (x <= 0)
				*po++ = -999999.;
			else
				*po++ = log(x);
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}
