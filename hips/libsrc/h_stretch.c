/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_stretch.c - range stretching
 *
 * input pixel formats: BYTE, SHORT
 * output pixel formats: same as input
 *
 * Stretches the greyscale using two different power functions.  The parameter
 * bdry is used to split the range of pixel values into two parts:
 *	from 0 to max*bdry, and from max*bdry to max
 *
 * Then, pixels in the first range have a power function applied with exponent
 * expt1, and are then rescaled so that the boundary value is unchanged.
 * Similarly, pixels in the second range have a power function applied with
 * exponent expt2, and are then linearly rescaled so that the boundary and
 * maximum values are unchanged.  Values less than zero are set to zero.
 *
 * Yoav Cohen 2/19/82
 * exponentiation bug fixed - WEJohnston 9/89
 * added support for short images and modified to use look-up table for
 *	byte and short images: Brian Tierney, LBL 10/90
 * HIPS 2 - msl - 6/13/91
 */

#include <hipl_format.h>
#include <math.h>
#define MY_MAXSHORT 32768

static h_boolean balloc = FALSE;
static byte *byte_lut;
static double byte_d,byte_expt1,byte_expt2;
static byte byte_mval;
static h_boolean salloc = FALSE;
static short *short_lut;
static double short_d,short_expt1,short_expt2;
static short short_mval;
int h_stretch_Ss(),h_stretch_Sf();

int h_stretch(hdi,hdo,d,expt1,expt2,mval)

struct header *hdi,*hdo;
double d,expt1,expt2;
Pixelval *mval;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	return(h_stretch_b(hdi,hdo,d,expt1,expt2,mval));
	case PFSHORT:	return(h_stretch_s(hdi,hdo,d,expt1,expt2,mval));
	default:	return(perr(HE_FMTSUBR,"h_stretch",
				hformatname(hdi->pixel_format)));
	}
}

int h_stretch_b(hdi,hdo,d,expt1,expt2,mval)

struct header *hdi,*hdo;
double d,expt1,expt2;
Pixelval *mval;

{
	return(h_stretch_B(hdi->firstpix,(float *) hdo->firstpix,hdi->rows,
		hdi->cols,hdi->ocols,hdo->ocols,d,expt1,expt2,mval->v_byte));
}

int h_stretch_s(hdi,hdo,d,expt1,expt2,mval)

struct header *hdi,*hdo;
double d,expt1,expt2;
Pixelval *mval;

{
	return(h_stretch_S((short *) hdi->firstpix,(short *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,d,expt1,expt2,
		mval->v_short));
}

int h_stretch_B(imagei,imageo,nr,nc,nlpi,nlpo,d,expt1,expt2,mval)

byte *imagei,*imageo;
int nr,nc,nlpi,nlpo;
double d,expt1,expt2;
byte mval;

{
	register int i,j,nexi,nexo;
	register byte *pi;
	byte *po;
	int d1;
	double a,s1,s2,s3,dtmp,dstmp;

	if (!balloc) {
		if ((byte_lut = (byte *) memalloc(256,sizeof(byte)))
			== (byte *) HIPS_ERROR)
				return(HIPS_ERROR);
		byte_d = d + 1;	/* force computation */
		balloc = TRUE;
	}
	if (byte_d != d || byte_expt1 != expt1 || byte_expt2 != expt2
	    || byte_mval != mval) {
		byte_d = d;
		byte_expt1 = expt1;
		byte_expt2 = expt2;
		byte_mval = mval;
		d1 = d * mval;
		s1 = pow(mval * d, (1. - expt1));
		s2 = pow(mval * (1. - d), 1. - expt2);
		s3 = mval * d;
		po = byte_lut;
		*po++ = 0;
		for (i=1;i<256;i++) {
			dtmp = i;
			if (dtmp == 0.)
				a = 0.;
			else if (dtmp <= d1)
				a = s1*pow(dtmp,expt1);
			else {
				dstmp = dtmp - s3;
				if (dstmp < 0)
					a = s3;
				else
					a = s3 + pow(dstmp,expt2)*s2;
			}
			*po++ = (byte) (a + 0.5);
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

int h_stretch_S(imagei,imageo,nr,nc,nlpi,nlpo,d,expt1,expt2,mval)

short *imagei,*imageo;
int nr,nc,nlpi,nlpo;
double d,expt1,expt2;
short mval;

{
	if (nr*nc < MY_MAXSHORT &&
	    (!(salloc && short_d == d && short_expt1 == expt1 &&
	    short_expt2 == expt2 && short_mval == mval)))
		return(h_stretch_Ss(imagei,imageo,nr,nc,nlpi,nlpo,d,expt1,
			expt2,mval));
	else
		return(h_stretch_Sf(imagei,imageo,nr,nc,nlpi,nlpo,d,expt1,
			expt2,mval));
}

int h_stretch_Sf(imagei,imageo,nr,nc,nlpi,nlpo,d,expt1,expt2,mval)

short *imagei,*imageo;
int nr,nc,nlpi,nlpo;
double d,expt1,expt2;
short mval;

{
	register int i,j,nexi,nexo;
	register short *pi;
	short *po;
	int d1;
	double a,s1,s2,s3,dtmp,dstmp;

	if (!salloc) {
		if ((short_lut = (short *) memalloc(MY_MAXSHORT,sizeof(short)))
			== (short *) HIPS_ERROR)
				return(HIPS_ERROR);
		short_d = d + 1;	/* force computation */
		salloc = TRUE;
	}
	if (short_d != d || short_expt1 != expt1 || short_expt2 != expt2
	    || short_mval != mval) {
		short_d = d;
		short_expt1 = expt1;
		short_expt2 = expt2;
		short_mval = mval;
		d1 = d * mval;
		s1 = pow(mval * d, (1. - expt1));
		s2 = pow(mval * (1. - d), 1. - expt2);
		s3 = mval * d;
		po = short_lut;
		*po++ = 0;
		for (i=1;i<MY_MAXSHORT;i++) {
			dtmp = i;
			if (dtmp == 0.)
				a = 0.;
			else if (dtmp <= d1)
				a = s1*pow(dtmp,expt1);
			else {
				dstmp = dtmp - s3;
				if (dstmp < 0)
					a = s3;
				else
					a = s3 + pow(dstmp,expt2)*s2;
			}
			*po++ = (short) (a + 0.5);
		}
	}
	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			if (*pi < 0) {
				*po++ = 0;
				pi++;
			}
			else
				*po++ = short_lut[*pi++];
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_stretch_Ss(imagei,imageo,nr,nc,nlpi,nlpo,d,expt1,expt2,mval)

short *imagei,*imageo;
int nr,nc,nlpi,nlpo;
double d,expt1,expt2;
short mval;

{
	register int i,j,nexi,nexo;
	register short *pi;
	short *po;
	int d1;
	double a,s1,s2,s3,dtmp,dstmp;

	d1 = d * mval;
	s1 = pow(mval * d, (1. - expt1));
	s2 = pow(mval * (1. - d), 1. - expt2);
	s3 = mval * d;
	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			dtmp = *pi++;
			if (dtmp <= 0)
				*po++ = 0;
			else {
				if (dtmp <= d1)
					a = s1*pow(dtmp,expt1);
				else {
					dstmp = dtmp - s3;
					if (dstmp < 0)
						a = s3;
					else
						a = s3 + pow(dstmp,expt2)*s2;
				}
				*po++ = (short) (a + 0.5);
			}
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}
