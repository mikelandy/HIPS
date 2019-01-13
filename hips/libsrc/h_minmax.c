/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_minmax.c - subroutines to compute the minimum and maximum of an image
 *
 * For complex and double complex images, the values returned are floating
 * point and double, respectively, and give the minimum and maximum complex
 * magnitude.
 *
 * pixel formats: BYTE, SBYTE, SHORT, USHORT, INT, UINT, FLOAT, DOUBLE,
 *	COMPLEX, DBLCOM
 *
 * Michael Landy - 6/17/91
 */

#include <hipl_format.h>
#include <math.h>

int h_minmax(hd,minval,maxval,nzflag)

struct header *hd;
Pixelval *minval,*maxval;
h_boolean nzflag;

{
	switch(hd->pixel_format) {
	case PFBYTE:	return(h_minmax_b(hd,minval,maxval,nzflag));
	case PFSBYTE:	return(h_minmax_sb(hd,minval,maxval,nzflag));
	case PFSHORT:	return(h_minmax_s(hd,minval,maxval,nzflag));
	case PFUSHORT:	return(h_minmax_us(hd,minval,maxval,nzflag));
	case PFINT:	return(h_minmax_i(hd,minval,maxval,nzflag));
	case PFUINT:	return(h_minmax_ui(hd,minval,maxval,nzflag));
	case PFFLOAT:	return(h_minmax_f(hd,minval,maxval,nzflag));
	case PFDOUBLE:	return(h_minmax_d(hd,minval,maxval,nzflag));
	case PFCOMPLEX:	return(h_minmax_c(hd,minval,maxval,nzflag));
	case PFDBLCOM:	return(h_minmax_dc(hd,minval,maxval,nzflag));
	default:	return(perr(HE_FMTSUBR,"h_minmax",
				hformatname(hd->pixel_format)));
	}
}

int h_minmax_b(hd,minval,maxval,nzflag)

struct header *hd;
Pixelval *minval,*maxval;
h_boolean nzflag;

{
	byte min,max;

	h_minmax_B((byte *) hd->firstpix,hd->rows,hd->cols,hd->ocols,&min,&max,
		nzflag);
	minval->v_byte = min;
	maxval->v_byte = max;
	return(HIPS_OK);
}

int h_minmax_sb(hd,minval,maxval,nzflag)

struct header *hd;
Pixelval *minval,*maxval;
h_boolean nzflag;

{
	sbyte min,max;

	h_minmax_SB((sbyte *) hd->firstpix,hd->rows,hd->cols,hd->ocols,&min,
		&max,nzflag);
	minval->v_sbyte = min;
	maxval->v_sbyte = max;
	return(HIPS_OK);
}

int h_minmax_s(hd,minval,maxval,nzflag)

struct header *hd;
Pixelval *minval,*maxval;
h_boolean nzflag;

{
	short min,max;

	h_minmax_S((short *) hd->firstpix,hd->rows,hd->cols,hd->ocols,&min,
		&max,nzflag);
	minval->v_short = min;
	maxval->v_short = max;
	return(HIPS_OK);
}

int h_minmax_us(hd,minval,maxval,nzflag)

struct header *hd;
Pixelval *minval,*maxval;
h_boolean nzflag;

{
	h_ushort min,max;

	h_minmax_US((h_ushort *) hd->firstpix,hd->rows,hd->cols,hd->ocols,&min,
		&max,nzflag);
	minval->v_ushort = min;
	maxval->v_ushort = max;
	return(HIPS_OK);
}

int h_minmax_i(hd,minval,maxval,nzflag)

struct header *hd;
Pixelval *minval,*maxval;
h_boolean nzflag;

{
	int min,max;

	h_minmax_I((int *) hd->firstpix,hd->rows,hd->cols,hd->ocols,&min,&max,
		nzflag);
	minval->v_int = min;
	maxval->v_int = max;
	return(HIPS_OK);
}

int h_minmax_ui(hd,minval,maxval,nzflag)

struct header *hd;
Pixelval *minval,*maxval;
h_boolean nzflag;

{
	h_uint min,max;

	h_minmax_UI((h_uint *) hd->firstpix,hd->rows,hd->cols,hd->ocols,&min,
		&max,nzflag);
	minval->v_uint = min;
	maxval->v_uint = max;
	return(HIPS_OK);
}

int h_minmax_f(hd,minval,maxval,nzflag)

struct header *hd;
Pixelval *minval,*maxval;
h_boolean nzflag;

{
	float min,max;

	h_minmax_F((float *) hd->firstpix,hd->rows,hd->cols,hd->ocols,&min,
		&max,nzflag);
	minval->v_float = min;
	maxval->v_float = max;
	return(HIPS_OK);
}

int h_minmax_d(hd,minval,maxval,nzflag)

struct header *hd;
Pixelval *minval,*maxval;
h_boolean nzflag;

{
	double min,max;

	h_minmax_D((double *) hd->firstpix,hd->rows,hd->cols,hd->ocols,&min,
		&max,nzflag);
	minval->v_double = min;
	maxval->v_double = max;
	return(HIPS_OK);
}

int h_minmax_c(hd,minval,maxval,nzflag)

struct header *hd;
Pixelval *minval,*maxval;
h_boolean nzflag;

{
	float min,max;

	h_minmax_C((float *) hd->firstpix,hd->rows,hd->cols,hd->ocols,&min,
		&max,nzflag);
	minval->v_float = min;
	maxval->v_float = max;
	return(HIPS_OK);
}

int h_minmax_dc(hd,minval,maxval,nzflag)

struct header *hd;
Pixelval *minval,*maxval;
h_boolean nzflag;

{
	double min,max;

	h_minmax_DC((double *) hd->firstpix,hd->rows,hd->cols,hd->ocols,&min,
		&max,nzflag);
	minval->v_double = min;
	maxval->v_double = max;
	return(HIPS_OK);
}

int h_minmax_B(image,nr,nc,nlp,rmin,rmax,nzflag)

byte *image,*rmin,*rmax;
int nr,nc,nlp;
h_boolean nzflag;

{
	register int i,j,nex;
	register byte *p,min,max;
	h_boolean notyet;

	nex = nlp-nc;
	p = image;
	if (nzflag) {
		min = max = 0;
		notyet = TRUE;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				if (*p != 0) {
					if (notyet) {
						notyet = FALSE;
						min = max = *p;
					}
					else if (*p < min)
						min = *p;
					else if (*p > max)
						max = *p;
				}
				p++;
			}
			p += nex;
		}
	}
	else {
		min = max = *p;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				if (*p < min)
					min = *p;
				else if (*p > max)
					max = *p;
				p++;
			}
			p += nex;
		}
	}
	*rmin = min;
	*rmax = max;
	return(HIPS_OK);
}

int h_minmax_SB(image,nr,nc,nlp,rmin,rmax,nzflag)

sbyte *image,*rmin,*rmax;
int nr,nc,nlp;
h_boolean nzflag;

{
	register int i,j,nex;
	register sbyte *p,min,max;
	h_boolean notyet;

	nex = nlp-nc;
	p = image;
	if (nzflag) {
		min = max = 0;
		notyet = TRUE;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				if (*p != 0) {
					if (notyet) {
						notyet = FALSE;
						min = max = *p;
					}
					else if (*p < min)
						min = *p;
					else if (*p > max)
						max = *p;
				}
				p++;
			}
			p += nex;
		}
	}
	else {
		min = max = *p;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				if (*p < min)
					min = *p;
				else if (*p > max)
					max = *p;
				p++;
			}
			p += nex;
		}
	}
	*rmin = min;
	*rmax = max;
	return(HIPS_OK);
}

int h_minmax_S(image,nr,nc,nlp,rmin,rmax,nzflag)

short *image,*rmin,*rmax;
int nr,nc,nlp;
h_boolean nzflag;

{
	register int i,j,nex;
	register short *p,min,max;
	h_boolean notyet;

	nex = nlp-nc;
	p = image;
	if (nzflag) {
		min = max = 0;
		notyet = TRUE;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				if (*p != 0) {
					if (notyet) {
						notyet = FALSE;
						min = max = *p;
					}
					else if (*p < min)
						min = *p;
					else if (*p > max)
						max = *p;
				}
				p++;
			}
			p += nex;
		}
	}
	else {
		min = max = *p;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				if (*p < min)
					min = *p;
				else if (*p > max)
					max = *p;
				p++;
			}
			p += nex;
		}
	}
	*rmin = min;
	*rmax = max;
	return(HIPS_OK);
}

int h_minmax_US(image,nr,nc,nlp,rmin,rmax,nzflag)

h_ushort *image,*rmin,*rmax;
int nr,nc,nlp;
h_boolean nzflag;

{
	register int i,j,nex;
	register h_ushort *p,min,max;
	h_boolean notyet;

	nex = nlp-nc;
	p = image;
	if (nzflag) {
		min = max = 0;
		notyet = TRUE;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				if (*p != 0) {
					if (notyet) {
						notyet = FALSE;
						min = max = *p;
					}
					else if (*p < min)
						min = *p;
					else if (*p > max)
						max = *p;
				}
				p++;
			}
			p += nex;
		}
	}
	else {
		min = max = *p;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				if (*p < min)
					min = *p;
				else if (*p > max)
					max = *p;
				p++;
			}
			p += nex;
		}
	}
	*rmin = min;
	*rmax = max;
	return(HIPS_OK);
}

int h_minmax_I(image,nr,nc,nlp,rmin,rmax,nzflag)

int *image,*rmin,*rmax;
int nr,nc,nlp;
h_boolean nzflag;

{
	register int i,j,nex;
	register int *p,min,max;
	h_boolean notyet;

	nex = nlp-nc;
	p = image;
	if (nzflag) {
		min = max = 0;
		notyet = TRUE;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				if (*p != 0) {
					if (notyet) {
						notyet = FALSE;
						min = max = *p;
					}
					else if (*p < min)
						min = *p;
					else if (*p > max)
						max = *p;
				}
				p++;
			}
			p += nex;
		}
	}
	else {
		min = max = *p;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				if (*p < min)
					min = *p;
				else if (*p > max)
					max = *p;
				p++;
			}
			p += nex;
		}
	}
	*rmin = min;
	*rmax = max;
	return(HIPS_OK);
}

int h_minmax_UI(image,nr,nc,nlp,rmin,rmax,nzflag)

h_uint *image,*rmin,*rmax;
int nr,nc,nlp;
h_boolean nzflag;

{
	register int i,j,nex;
	register h_uint *p,min,max;
	h_boolean notyet;

	nex = nlp-nc;
	p = image;
	if (nzflag) {
		min = max = 0;
		notyet = TRUE;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				if (*p != 0) {
					if (notyet) {
						notyet = FALSE;
						min = max = *p;
					}
					else if (*p < min)
						min = *p;
					else if (*p > max)
						max = *p;
				}
				p++;
			}
			p += nex;
		}
	}
	else {
		min = max = *p;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				if (*p < min)
					min = *p;
				else if (*p > max)
					max = *p;
				p++;
			}
			p += nex;
		}
	}
	*rmin = min;
	*rmax = max;
	return(HIPS_OK);
}

int h_minmax_F(image,nr,nc,nlp,rmin,rmax,nzflag)

float *image,*rmin,*rmax;
int nr,nc,nlp;
h_boolean nzflag;

{
	register int i,j,nex;
	register float *p,min,max;
	h_boolean notyet;

	nex = nlp-nc;
	p = image;
	if (nzflag) {
		min = max = 0;
		notyet = TRUE;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				if (*p != 0) {
					if (notyet) {
						notyet = FALSE;
						min = max = *p;
					}
					else if (*p < min)
						min = *p;
					else if (*p > max)
						max = *p;
				}
				p++;
			}
			p += nex;
		}
	}
	else {
		min = max = *p;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				if (*p < min)
					min = *p;
				else if (*p > max)
					max = *p;
				p++;
			}
			p += nex;
		}
	}
	*rmin = min;
	*rmax = max;
	return(HIPS_OK);
}

int h_minmax_D(image,nr,nc,nlp,rmin,rmax,nzflag)

double *image,*rmin,*rmax;
int nr,nc,nlp;
h_boolean nzflag;

{
	register int i,j,nex;
	register double *p,min,max;
	h_boolean notyet;

	nex = nlp-nc;
	p = image;
	if (nzflag) {
		min = max = 0;
		notyet = TRUE;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				if (*p != 0) {
					if (notyet) {
						notyet = FALSE;
						min = max = *p;
					}
					else if (*p < min)
						min = *p;
					else if (*p > max)
						max = *p;
				}
				p++;
			}
			p += nex;
		}
	}
	else {
		min = max = *p;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				if (*p < min)
					min = *p;
				else if (*p > max)
					max = *p;
				p++;
			}
			p += nex;
		}
	}
	*rmin = min;
	*rmax = max;
	return(HIPS_OK);
}

int h_minmax_C(image,nr,nc,nlp,rmin,rmax,nzflag)

float *image,*rmin,*rmax;
int nr,nc,nlp;
h_boolean nzflag;

{
	register int i,j,nex;
	register float *p,min,max,sqmagn;
	h_boolean notyet;

	nex = 2*(nlp-nc);
	p = image;
	if (nzflag) {
		min = max = 0;
		notyet = TRUE;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				sqmagn = (p[0] * p[0]) + (p[1] * p[1]);
				if (sqmagn != 0) {
					if (notyet) {
						notyet = FALSE;
						min = max = sqmagn;
					}
					else if (sqmagn < min)
						min = sqmagn;
					else if (sqmagn > max)
						max = sqmagn;
				}
				p += 2;
			}
			p += nex;
		}
	}
	else {
		min = max = (p[0] * p[0]) + (p[1] * p[1]);
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				sqmagn = (p[0] * p[0]) + (p[1] * p[1]);
				if (sqmagn < min)
					min = sqmagn;
				else if (sqmagn > max)
					max = sqmagn;
				p += 2;
			}
			p += nex;
		}
	}
	*rmin = (float) sqrt((double) min);
	*rmax = (float) sqrt((double) max);
	return(HIPS_OK);
}

int h_minmax_DC(image,nr,nc,nlp,rmin,rmax,nzflag)

double *image,*rmin,*rmax;
int nr,nc,nlp;
h_boolean nzflag;

{
	register int i,j,nex;
	register double *p,min,max,sqmagn;
	h_boolean notyet;

	nex = 2*(nlp-nc);
	p = image;
	if (nzflag) {
		min = max = 0;
		notyet = TRUE;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				sqmagn = (p[0] * p[0]) + (p[1] * p[1]);
				if (sqmagn != 0) {
					if (notyet) {
						notyet = FALSE;
						min = max = sqmagn;
					}
					else if (sqmagn < min)
						min = sqmagn;
					else if (sqmagn > max)
						max = sqmagn;
				}
				p += 2;
			}
			p += nex;
		}
	}
	else {
		min = max = (p[0] * p[0]) + (p[1] * p[1]);
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				sqmagn = (p[0] * p[0]) + (p[1] * p[1]);
				if (sqmagn < min)
					min = sqmagn;
				else if (sqmagn > max)
					max = sqmagn;
				p += 2;
			}
			p += nex;
		}
	}
	*rmin = (float) sqrt((double) min);
	*rmax = (float) sqrt((double) max);
	return(HIPS_OK);
}
