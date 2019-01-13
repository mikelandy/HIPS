/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_max.c - subroutines to compute the maximum of an image
 *
 * pixel formats: BYTE, SBYTE, SHORT, USHORT, INT, UINT, FLOAT, DOUBLE
 *
 * Michael Landy - 6/13/91
 */

#include <hipl_format.h>

int h_max(hd,maxval,nzflag)

struct header *hd;
Pixelval *maxval;
h_boolean nzflag;

{
	switch(hd->pixel_format) {
	case PFBYTE:	return(h_max_b(hd,maxval,nzflag));
	case PFSBYTE:	return(h_max_sb(hd,maxval,nzflag));
	case PFSHORT:	return(h_max_s(hd,maxval,nzflag));
	case PFUSHORT:	return(h_max_us(hd,maxval,nzflag));
	case PFINT:	return(h_max_i(hd,maxval,nzflag));
	case PFUINT:	return(h_max_ui(hd,maxval,nzflag));
	case PFFLOAT:	return(h_max_f(hd,maxval,nzflag));
	case PFDOUBLE:	return(h_max_d(hd,maxval,nzflag));
	default:	return(perr(HE_FMTSUBR,"h_max",
				hformatname(hd->pixel_format)));
	}
}

int h_max_b(hd,maxval,nzflag)

struct header *hd;
Pixelval *maxval;
h_boolean nzflag;

{
	byte h_max_B();

	maxval->v_byte =
		h_max_B((byte *) hd->firstpix,hd->rows,hd->cols,hd->ocols,
			nzflag);
	return(HIPS_OK);
}

int h_max_sb(hd,maxval,nzflag)

struct header *hd;
Pixelval *maxval;
h_boolean nzflag;

{
	sbyte h_max_SB();

	maxval->v_sbyte =
		h_max_SB((sbyte *) hd->firstpix,hd->rows,hd->cols,hd->ocols,
			nzflag);
	return(HIPS_OK);
}

int h_max_s(hd,maxval,nzflag)

struct header *hd;
Pixelval *maxval;
h_boolean nzflag;

{
	short h_max_S();

	maxval->v_short =
		h_max_S((short *) hd->firstpix,hd->rows,hd->cols,hd->ocols,
			nzflag);
	return(HIPS_OK);
}

int h_max_us(hd,maxval,nzflag)

struct header *hd;
Pixelval *maxval;
h_boolean nzflag;

{
	h_ushort h_max_US();

	maxval->v_ushort =
		h_max_US((h_ushort *) hd->firstpix,hd->rows,hd->cols,hd->ocols,
			nzflag);
	return(HIPS_OK);
}

int h_max_i(hd,maxval,nzflag)

struct header *hd;
Pixelval *maxval;
h_boolean nzflag;

{
	int h_max_I();

	maxval->v_int =
		h_max_I((int *) hd->firstpix,hd->rows,hd->cols,hd->ocols,
			nzflag);
	return(HIPS_OK);
}

int h_max_ui(hd,maxval,nzflag)

struct header *hd;
Pixelval *maxval;
h_boolean nzflag;

{
	h_uint h_max_UI();

	maxval->v_uint =
		h_max_UI((h_uint *) hd->firstpix,hd->rows,hd->cols,hd->ocols,
			nzflag);
	return(HIPS_OK);
}

int h_max_f(hd,maxval,nzflag)

struct header *hd;
Pixelval *maxval;
h_boolean nzflag;

{
	float h_max_F();

	maxval->v_float =
		h_max_F((float *) hd->firstpix,hd->rows,hd->cols,hd->ocols,
			nzflag);
	return(HIPS_OK);
}

int h_max_d(hd,maxval,nzflag)

struct header *hd;
Pixelval *maxval;
h_boolean nzflag;

{
	double h_max_D();

	maxval->v_double =
		h_max_D((double *) hd->firstpix,hd->rows,hd->cols,hd->ocols,
			nzflag);
	return(HIPS_OK);
}

byte h_max_B(image,nr,nc,nlp,nzflag)

byte *image;
int nr,nc,nlp;
h_boolean nzflag;

{
	register int i,j,nex;
	register byte *p,max;
	h_boolean notyet;

	nex = nlp-nc;
	p = image;
	if (nzflag) {
		max = 0;
		notyet = TRUE;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				if (*p != 0) {
					if (notyet) {
						notyet = FALSE;
						max = *p;
					}
					else if (*p > max)
						max = *p;
				}
				p++;
			}
			p += nex;
		}
	}
	else {
		max = *p;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				if (*p > max)
					max = *p;
				p++;
			}
			p += nex;
		}
	}
	return(max);
}

sbyte h_max_SB(image,nr,nc,nlp,nzflag)

sbyte *image;
int nr,nc,nlp;
h_boolean nzflag;

{
	register int i,j,nex;
	register sbyte *p,max;
	h_boolean notyet;

	nex = nlp-nc;
	p = image;
	if (nzflag) {
		max = 0;
		notyet = TRUE;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				if (*p != 0) {
					if (notyet) {
						notyet = FALSE;
						max = *p;
					}
					else if (*p > max)
						max = *p;
				}
				p++;
			}
			p += nex;
		}
	}
	else {
		max = *p;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				if (*p > max)
					max = *p;
				p++;
			}
			p += nex;
		}
	}
	return(max);
}

short h_max_S(image,nr,nc,nlp,nzflag)

short *image;
int nr,nc,nlp;
h_boolean nzflag;

{
	register int i,j,nex;
	register short *p,max;
	h_boolean notyet;

	nex = nlp-nc;
	p = image;
	if (nzflag) {
		max = 0;
		notyet = TRUE;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				if (*p != 0) {
					if (notyet) {
						notyet = FALSE;
						max = *p;
					}
					else if (*p > max)
						max = *p;
				}
				p++;
			}
			p += nex;
		}
	}
	else {
		max = *p;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				if (*p > max)
					max = *p;
				p++;
			}
			p += nex;
		}
	}
	return(max);
}

h_ushort h_max_US(image,nr,nc,nlp,nzflag)

h_ushort *image;
int nr,nc,nlp;
h_boolean nzflag;

{
	register int i,j,nex;
	register h_ushort *p,max;
	h_boolean notyet;

	nex = nlp-nc;
	p = image;
	if (nzflag) {
		max = 0;
		notyet = TRUE;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				if (*p != 0) {
					if (notyet) {
						notyet = FALSE;
						max = *p;
					}
					else if (*p > max)
						max = *p;
				}
				p++;
			}
			p += nex;
		}
	}
	else {
		max = *p;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				if (*p > max)
					max = *p;
				p++;
			}
			p += nex;
		}
	}
	return(max);
}

int h_max_I(image,nr,nc,nlp,nzflag)

int *image;
int nr,nc,nlp;
h_boolean nzflag;

{
	register int i,j,nex;
	register int *p,max;
	h_boolean notyet;

	nex = nlp-nc;
	p = image;
	if (nzflag) {
		max = 0;
		notyet = TRUE;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				if (*p != 0) {
					if (notyet) {
						notyet = FALSE;
						max = *p;
					}
					else if (*p > max)
						max = *p;
				}
				p++;
			}
			p += nex;
		}
	}
	else {
		max = *p;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				if (*p > max)
					max = *p;
				p++;
			}
			p += nex;
		}
	}
	return(max);
}

h_uint h_max_UI(image,nr,nc,nlp,nzflag)

h_uint *image;
int nr,nc,nlp;
h_boolean nzflag;

{
	register int i,j,nex;
	register h_uint *p,max;
	h_boolean notyet;

	nex = nlp-nc;
	p = image;
	if (nzflag) {
		max = 0;
		notyet = TRUE;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				if (*p != 0) {
					if (notyet) {
						notyet = FALSE;
						max = *p;
					}
					else if (*p > max)
						max = *p;
				}
				p++;
			}
			p += nex;
		}
	}
	else {
		max = *p;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				if (*p > max)
					max = *p;
				p++;
			}
			p += nex;
		}
	}
	return(max);
}

float h_max_F(image,nr,nc,nlp,nzflag)

float *image;
int nr,nc,nlp;
h_boolean nzflag;

{
	register int i,j,nex;
	register float *p,max;
	h_boolean notyet;

	nex = nlp-nc;
	p = image;
	if (nzflag) {
		max = 0;
		notyet = TRUE;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				if (*p != 0) {
					if (notyet) {
						notyet = FALSE;
						max = *p;
					}
					else if (*p > max)
						max = *p;
				}
				p++;
			}
			p += nex;
		}
	}
	else {
		max = *p;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				if (*p > max)
					max = *p;
				p++;
			}
			p += nex;
		}
	}
	return(max);
}

double h_max_D(image,nr,nc,nlp,nzflag)

double *image;
int nr,nc,nlp;
h_boolean nzflag;

{
	register int i,j,nex;
	register double *p,max;
	h_boolean notyet;

	nex = nlp-nc;
	p = image;
	if (nzflag) {
		max = 0;
		notyet = TRUE;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				if (*p != 0) {
					if (notyet) {
						notyet = FALSE;
						max = *p;
					}
					else if (*p > max)
						max = *p;
				}
				p++;
			}
			p += nex;
		}
	}
	else {
		max = *p;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				if (*p > max)
					max = *p;
				p++;
			}
			p += nex;
		}
	}
	return(max);
}
