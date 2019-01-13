/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_divscale.c - divide by a fixed scale factor
 *
 * input pixel formats: SHORT, INT->INT, INT->BYTE, INT->FLOAT, FLOAT, COMPLEX
 * output pixel format: same as input (except for INT->BYTE and INT->FLOAT)
 *
 * Computes opix = ipix/b
 *
 * for complex images, the scale factor is floating point; for double complex
 * images the scale factor is double; for int->byte, the scale factor is an
 * integer, for int->float the scale factor is a float
 *
 * Michael Landy (HIPS 2) - 6/29/91
 */

#include <hipl_format.h>
#define MAXSHORT 32768

static h_boolean salloc = FALSE;
static short *short_lut;
static short short_b;
int h_divscale_Ss(),h_divscale_Sf();

int h_divscale(hdi,hdo,b)

struct header *hdi,*hdo;
Pixelval *b;

{
	switch(hdi->pixel_format) {
	case PFSHORT:	return(h_divscale_s(hdi,hdo,b));
	case PFINT:	if (hdo->pixel_format == PFBYTE)
				return(h_divscale_ib(hdi,hdo,b));
			else if (hdo->pixel_format == PFINT)
				return(h_divscale_i(hdi,hdo,b));
			else if (hdo->pixel_format == PFFLOAT)
				return(h_divscale_if(hdi,hdo,b));
			else
				return(perr(HE_FMT2SUBR,"h_divscale",
					hformatname(hdi->pixel_format),
					hformatname(hdo->pixel_format)));
	case PFFLOAT:	return(h_divscale_f(hdi,hdo,b));
	case PFCOMPLEX:	return(h_divscale_c(hdi,hdo,b));
	case PFDBLCOM:	return(h_divscale_dc(hdi,hdo,b));
	default:	return(perr(HE_FMTSUBR,"h_divscale",
				hformatname(hdi->pixel_format)));
	}
}

int h_divscale_s(hdi,hdo,b)

struct header *hdi,*hdo;
Pixelval *b;

{
	return(h_divscale_S((short *) hdi->firstpix,(short *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,b->v_short));
}

int h_divscale_ib(hdi,hdo,b)

struct header *hdi,*hdo;
Pixelval *b;

{
	return(h_divscale_IB((int *) hdi->firstpix,(byte *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,b->v_int));
}

int h_divscale_i(hdi,hdo,b)

struct header *hdi,*hdo;
Pixelval *b;

{
	return(h_divscale_I((int *) hdi->firstpix,(int *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,b->v_int));
}

int h_divscale_if(hdi,hdo,b)

struct header *hdi,*hdo;
Pixelval *b;

{
	return(h_divscale_IF((int *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,b->v_float));
}

int h_divscale_f(hdi,hdo,b)

struct header *hdi,*hdo;
Pixelval *b;

{
	return(h_divscale_F((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,b->v_float));
}

int h_divscale_c(hdi,hdo,b)

struct header *hdi,*hdo;
Pixelval *b;

{
	return(h_divscale_C((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,b->v_float));
}

int h_divscale_dc(hdi,hdo,b)

struct header *hdi,*hdo;
Pixelval *b;

{
	return(h_divscale_DC((double *) hdi->firstpix,(double *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,b->v_double));
}

int h_divscale_S(imagei,imageo,nr,nc,nlpi,nlpo,b)

short *imagei,*imageo;
int nr,nc,nlpi,nlpo;
short b;

{
	if ((nr*nc > MAXSHORT) || (salloc && short_b == b))
		return(h_divscale_Sf(imagei,imageo,nr,nc,nlpi,nlpo,b));
	else
		return(h_divscale_Ss(imagei,imageo,nr,nc,nlpi,nlpo,b));
}

int h_divscale_Ss(imagei,imageo,nr,nc,nlpi,nlpo,b)

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
			*po++ = (*pi++)/b;
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_divscale_Sf(imagei,imageo,nr,nc,nlpi,nlpo,b)

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
			*po++ = i/b;
	}
	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			if (*pi < 0)
				*po++ = (*pi++)/b;
			else
				*po++ = short_lut[*pi++];
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_divscale_IB(imagei,imageo,nr,nc,nlpi,nlpo,b)

int *imagei;
byte *imageo;
int nr,nc,nlpi,nlpo;
int b;

{
	register int i,j,nexi,nexo;
	register int *pi;
	register byte *po;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*po++ = (*pi++)/b;
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_divscale_I(imagei,imageo,nr,nc,nlpi,nlpo,b)

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
			*po++ = (*pi++)/b;
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_divscale_IF(imagei,imageo,nr,nc,nlpi,nlpo,b)

int *imagei;
float *imageo;
int nr,nc,nlpi,nlpo;
float b;

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
			*po++ = (*pi++)/b;
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_divscale_F(imagei,imageo,nr,nc,nlpi,nlpo,b)

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
			*po++ = (*pi++)/b;
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_divscale_C(imagei,imageo,nr,nc,nlpi,nlpo,b)

float *imagei,*imageo;
int nr,nc,nlpi,nlpo;
float b;

{
	register int i,j,nexi,nexo;
	register float *pi,*po;

	nexi = 2*(nlpi-nc);
	nexo = 2*(nlpo-nc);
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			*po++ = (*pi++)/b;
			*po++ = (*pi++)/b;
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_divscale_DC(imagei,imageo,nr,nc,nlpi,nlpo,b)

double *imagei,*imageo;
int nr,nc,nlpi,nlpo;
double b;

{
	register int i,j,nexi,nexo;
	register double *pi,*po;

	nexi = 2*(nlpi-nc);
	nexo = 2*(nlpo-nc);
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			*po++ = (*pi++)/b;
			*po++ = (*pi++)/b;
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}
