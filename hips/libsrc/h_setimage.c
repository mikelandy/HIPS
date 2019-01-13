/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_setimage.c - subroutines to set all image pixels to a single value
 *
 * pixel formats: MSBF, LSBF, BYTE, SBYTE, SHORT, USHORT, INT, UINT, FLOAT,
 *			DOUBLE, COMPLEX, DOUBLECOMPLEX
 *
 * Michael Landy - 6/23/91
 */

#include <hipl_format.h>

int h_setimage(hd,val)

struct header *hd;
Pixelval *val;

{
	switch(hd->pixel_format) {
	case PFMSBF:	return(h_setimage_mp(hd,val));
	case PFLSBF:	return(h_setimage_lp(hd,val));
	case PFBYTE:	return(h_setimage_b(hd,val));
	case PFSBYTE:	return(h_setimage_sb(hd,val));
	case PFSHORT:	return(h_setimage_s(hd,val));
	case PFUSHORT:	return(h_setimage_us(hd,val));
	case PFINT:	return(h_setimage_i(hd,val));
	case PFUINT:	return(h_setimage_ui(hd,val));
	case PFFLOAT:	return(h_setimage_f(hd,val));
	case PFDOUBLE:	return(h_setimage_d(hd,val));
	case PFCOMPLEX:	return(h_setimage_c(hd,val));
	case PFDBLCOM:	return(h_setimage_dc(hd,val));
	default:	return(perr(HE_FMTSUBR,"h_setimage",
				hformatname(hd->pixel_format)));
	}
}

int h_setimage_mp(hd,val)

struct header *hd;
Pixelval *val;

{
	return(h_setimage_MP(hd->firstpix,hd->rows,hd->cols,hd->ocols,
		val->v_byte));
}

int h_setimage_lp(hd,val)

struct header *hd;
Pixelval *val;

{
	return(h_setimage_LP(hd->firstpix,hd->rows,hd->cols,hd->ocols,
		val->v_byte));
}

int h_setimage_b(hd,val)

struct header *hd;
Pixelval *val;

{
	return(h_setimage_B(hd->firstpix,hd->rows,hd->cols,hd->ocols,
		val->v_byte));
}

int h_setimage_sb(hd,val)

struct header *hd;
Pixelval *val;

{
	return(h_setimage_SB((sbyte *) hd->firstpix,hd->rows,hd->cols,hd->ocols,
		val->v_sbyte));
}

int h_setimage_s(hd,val)

struct header *hd;
Pixelval *val;

{
	return(h_setimage_S((short *) hd->firstpix,hd->rows,hd->cols,hd->ocols,
		val->v_short));
}

int h_setimage_us(hd,val)

struct header *hd;
Pixelval *val;

{
	return(h_setimage_US((h_ushort *) hd->firstpix,hd->rows,hd->cols,
		hd->ocols,val->v_ushort));
}

int h_setimage_i(hd,val)

struct header *hd;
Pixelval *val;

{
	return(h_setimage_I((int *) hd->firstpix,hd->rows,hd->cols,hd->ocols,
		val->v_int));
}

int h_setimage_ui(hd,val)

struct header *hd;
Pixelval *val;

{
	return(h_setimage_UI((h_uint *) hd->firstpix,hd->rows,hd->cols,
		hd->ocols,val->v_uint));
}

int h_setimage_f(hd,val)

struct header *hd;
Pixelval *val;

{
	return(h_setimage_F((float *) hd->firstpix,hd->rows,hd->cols,hd->ocols,
		val->v_float));
}

int h_setimage_d(hd,val)

struct header *hd;
Pixelval *val;

{
	return(h_setimage_D((double *) hd->firstpix,hd->rows,hd->cols,hd->ocols,
		val->v_double));
}

int h_setimage_c(hd,val)

struct header *hd;
Pixelval *val;

{
	return(h_setimage_C((float *) hd->firstpix,hd->rows,hd->cols,hd->ocols,
		val->v_complex));
}

int h_setimage_dc(hd,val)

struct header *hd;
Pixelval *val;

{
	return(h_setimage_DC((double *) hd->firstpix,hd->rows,hd->cols,
		hd->ocols,val->v_dblcom));
}

int h_setimage_MP(image,nr,nc,nlp,val)

byte *image,val;
int nr,nc,nlp;

{
	register int i,j,nex,ncb;
	register byte *p;
	byte omask,nmask;

	ncb = (nc+7)/8;
	nex = ((nlp+7)/8) - ncb;
	p = image;
	if ((nc % 8) == 0) {
		for (i=0;i<nr;i++) {
			for (j=0;j<ncb;j++)
				*p++ = val;
			p += nex;
		}
	}
	else {
		omask = (0200 >> ((nc%8) - 1)) - 1;
		nmask = ~omask;
		ncb--;
		for (i=0;i<nr;i++) {
			for (j=0;j<ncb;j++)
				*p++ = val;
			*p = (*p & omask) | (val & nmask);
			p++;
			p += nex;
		}
	}
	return(HIPS_OK);
}

int h_setimage_LP(image,nr,nc,nlp,val)

byte *image,val;
int nr,nc,nlp;

{
	register int i,j,nex,ncb;
	register byte *p;
	byte omask,nmask;

	ncb = (nc+7)/8;
	nex = ((nlp+7)/8) - ncb;
	p = image;
	if ((nc % 8) == 0) {
		for (i=0;i<nr;i++) {
			for (j=0;j<ncb;j++)
				*p++ = val;
			p += nex;
		}
	}
	else {
		nmask = (01 << (nc%8)) - 1;
		omask = ~nmask;
		ncb--;
		for (i=0;i<nr;i++) {
			for (j=0;j<ncb;j++)
				*p++ = val;
			*p = (*p & omask) | (val & nmask);
			p++;
			p += nex;
		}
	}
	return(HIPS_OK);
}

int h_setimage_B(image,nr,nc,nlp,val)

byte *image,val;
int nr,nc,nlp;

{
	register int i,j,nex;
	register byte *p;

	nex = nlp-nc;
	p = image;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*p++ = val;
		p += nex;
	}
	return(HIPS_OK);
}

int h_setimage_SB(image,nr,nc,nlp,val)

sbyte *image,val;
int nr,nc,nlp;

{
	register int i,j,nex;
	register sbyte *p;

	nex = nlp-nc;
	p = image;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*p++ = val;
		p += nex;
	}
	return(HIPS_OK);
}

int h_setimage_S(image,nr,nc,nlp,val)

short *image,val;
int nr,nc,nlp;

{
	register int i,j,nex;
	register short *p;

	nex = nlp-nc;
	p = image;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*p++ = val;
		p += nex;
	}
	return(HIPS_OK);
}

int h_setimage_US(image,nr,nc,nlp,val)

h_ushort *image,val;
int nr,nc,nlp;

{
	register int i,j,nex;
	register h_ushort *p;

	nex = nlp-nc;
	p = image;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*p++ = val;
		p += nex;
	}
	return(HIPS_OK);
}

int h_setimage_I(image,nr,nc,nlp,val)

int *image,val;
int nr,nc,nlp;

{
	register int i,j,nex;
	register int *p;

	nex = nlp-nc;
	p = image;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*p++ = val;
		p += nex;
	}
	return(HIPS_OK);
}

int h_setimage_UI(image,nr,nc,nlp,val)

h_uint *image,val;
int nr,nc,nlp;

{
	register int i,j,nex;
	register h_uint *p;

	nex = nlp-nc;
	p = image;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*p++ = val;
		p += nex;
	}
	return(HIPS_OK);
}

int h_setimage_F(image,nr,nc,nlp,val)

float *image,val;
int nr,nc,nlp;

{
	register int i,j,nex;
	register float *p;

	nex = nlp-nc;
	p = image;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*p++ = val;
		p += nex;
	}
	return(HIPS_OK);
}

int h_setimage_D(image,nr,nc,nlp,val)

double *image,val;
int nr,nc,nlp;

{
	register int i,j,nex;
	register double *p;

	nex = nlp-nc;
	p = image;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*p++ = val;
		p += nex;
	}
	return(HIPS_OK);
}

int h_setimage_C(image,nr,nc,nlp,val)

float *image,*val;
int nr,nc,nlp;

{
	register int i,j,nex;
	register float *p;

	nex = 2*(nlp-nc);
	p = image;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			*p++ = val[0];
			*p++ = val[1];
		}
		p += nex;
	}
	return(HIPS_OK);
}

int h_setimage_DC(image,nr,nc,nlp,val)

double *image,*val;
int nr,nc,nlp;

{
	register int i,j,nex;
	register double *p;

	nex = nlp-nc;
	p = image;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			*p++ = val[0];
			*p++ = val[1];
		}
		p += nex;
	}
	return(HIPS_OK);
}
