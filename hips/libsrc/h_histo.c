/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_histo.c - subroutines to compute the histogram of an image
 *
 * For complex and double complex images, the histogram boundaries are floating
 * point and double, respectively, and the complex magnitude is histogrammed.
 *
 * Note: these routines to not clear the histogram first - use h_clearhisto
 * for that purpose.
 *
 * pixel formats: BYTE, SBYTE, SHORT, USHORT, INT, UINT, FLOAT, DOUBLE,
 *	COMPLEX, DBLCOM
 *
 * Michael Landy - 6/17/91
 */

#include <hipl_format.h>
#include <math.h>

int h_histo(hd,histogram,nzflag,count)

struct header *hd;
struct hips_histo *histogram;
h_boolean nzflag;
int *count;

{
	switch(hd->pixel_format) {
	case PFBYTE:	return(h_histo_b(hd,histogram,nzflag,count));
	case PFSBYTE:	return(h_histo_sb(hd,histogram,nzflag,count));
	case PFSHORT:	return(h_histo_s(hd,histogram,nzflag,count));
	case PFUSHORT:	return(h_histo_us(hd,histogram,nzflag,count));
	case PFINT:	return(h_histo_i(hd,histogram,nzflag,count));
	case PFUINT:	return(h_histo_ui(hd,histogram,nzflag,count));
	case PFFLOAT:	return(h_histo_f(hd,histogram,nzflag,count));
	case PFDOUBLE:	return(h_histo_d(hd,histogram,nzflag,count));
	case PFCOMPLEX:	return(h_histo_c(hd,histogram,nzflag,count));
	case PFDBLCOM:	return(h_histo_dc(hd,histogram,nzflag,count));
	default:	return(perr(HE_FMTSUBR,"h_histo",
				hformatname(hd->pixel_format)));
	}
}

int h_histo_b(hd,histogram,nzflag,count)

struct header *hd;
struct hips_histo *histogram;
h_boolean nzflag;
int *count;

{
	return(h_histo_B(hd->firstpix,hd->rows,hd->cols,hd->ocols,
		histogram->nbins,histogram->histo,histogram->minbin.v_byte,
		histogram->binwidth.v_byte,nzflag,count));
}

int h_histo_sb(hd,histogram,nzflag,count)

struct header *hd;
struct hips_histo *histogram;
h_boolean nzflag;
int *count;

{
	return(h_histo_SB((sbyte *) hd->firstpix,hd->rows,hd->cols,hd->ocols,
		histogram->nbins,histogram->histo,histogram->minbin.v_sbyte,
		histogram->binwidth.v_sbyte,nzflag,count));
}

int h_histo_s(hd,histogram,nzflag,count)

struct header *hd;
struct hips_histo *histogram;
h_boolean nzflag;
int *count;

{
	return(h_histo_S((short *) hd->firstpix,hd->rows,hd->cols,hd->ocols,
		histogram->nbins,histogram->histo,histogram->minbin.v_short,
		histogram->binwidth.v_short,nzflag,count));
}

int h_histo_us(hd,histogram,nzflag,count)

struct header *hd;
struct hips_histo *histogram;
h_boolean nzflag;
int *count;

{
	return(h_histo_US((h_ushort *) hd->firstpix,hd->rows,hd->cols,hd->ocols,
		histogram->nbins,histogram->histo,histogram->minbin.v_ushort,
		histogram->binwidth.v_ushort,nzflag,count));
}

int h_histo_i(hd,histogram,nzflag,count)

struct header *hd;
struct hips_histo *histogram;
h_boolean nzflag;
int *count;

{
	return(h_histo_I((int *) hd->firstpix,hd->rows,hd->cols,hd->ocols,
		histogram->nbins,histogram->histo,histogram->minbin.v_int,
		histogram->binwidth.v_int,nzflag,count));
}

int h_histo_ui(hd,histogram,nzflag,count)

struct header *hd;
struct hips_histo *histogram;
h_boolean nzflag;
int *count;

{
	return(h_histo_UI((h_uint *) hd->firstpix,hd->rows,hd->cols,hd->ocols,
		histogram->nbins,histogram->histo,histogram->minbin.v_uint,
		histogram->binwidth.v_uint,nzflag,count));
}

int h_histo_f(hd,histogram,nzflag,count)

struct header *hd;
struct hips_histo *histogram;
h_boolean nzflag;
int *count;

{
	return(h_histo_F((float *) hd->firstpix,hd->rows,hd->cols,hd->ocols,
		histogram->nbins,histogram->histo,histogram->minbin.v_float,
		histogram->binwidth.v_float,nzflag,count));
}

int h_histo_d(hd,histogram,nzflag,count)

struct header *hd;
struct hips_histo *histogram;
h_boolean nzflag;
int *count;

{
	return(h_histo_D((double *) hd->firstpix,hd->rows,hd->cols,hd->ocols,
		histogram->nbins,histogram->histo,histogram->minbin.v_double,
		histogram->binwidth.v_double,nzflag,count));
}

int h_histo_c(hd,histogram,nzflag,count)

struct header *hd;
struct hips_histo *histogram;
h_boolean nzflag;
int *count;

{
	return(h_histo_C((float *) hd->firstpix,hd->rows,hd->cols,hd->ocols,
		histogram->nbins,histogram->histo,histogram->minbin.v_float,
		histogram->binwidth.v_float,nzflag,count));
}

int h_histo_dc(hd,histogram,nzflag,count)

struct header *hd;
struct hips_histo *histogram;
h_boolean nzflag;
int *count;

{
	return(h_histo_DC((double *) hd->firstpix,hd->rows,hd->cols,hd->ocols,
		histogram->nbins,histogram->histo,histogram->minbin.v_double,
		histogram->binwidth.v_double,nzflag,count));
}

int h_histo_B(image,nr,nc,nlp,nbins,histo,min,width,nzflag,count)

byte *image,min,width;
int nr,nc,nlp,nbins,*histo;
h_boolean nzflag;
int *count;

{
	register int i,j,nex,bin,cnt;
	register byte *p;

	cnt = 0;
	nex = nlp-nc;
	p = image;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			if (nzflag && *p == 0) {
				p++;
				continue;
			}
			cnt++;
			bin = (int) ((*p++ - min)/width);
			if (bin < 0)
				histo[0]++;
			else if (bin >= nbins)
				histo[nbins+1]++;
			else
				histo[bin+1]++;
		}
		p += nex;
	}
	*count = cnt;
	return(HIPS_OK);
}

int h_histo_SB(image,nr,nc,nlp,nbins,histo,min,width,nzflag,count)

sbyte *image,min,width;
int nr,nc,nlp,nbins,*histo;
h_boolean nzflag;
int *count;

{
	register int i,j,nex,bin,cnt;
	register sbyte *p;

	cnt = 0;
	nex = nlp-nc;
	p = image;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			if (nzflag && *p == 0) {
				p++;
				continue;
			}
			cnt++;
			bin = (int) ((*p++ - min)/width);
			if (bin < 0)
				histo[0]++;
			else if (bin >= nbins)
				histo[nbins+1]++;
			else
				histo[bin+1]++;
		}
		p += nex;
	}
	*count = cnt;
	return(HIPS_OK);
}

int h_histo_S(image,nr,nc,nlp,nbins,histo,min,width,nzflag,count)

short *image,min,width;
int nr,nc,nlp,nbins,*histo;
h_boolean nzflag;
int *count;

{
	register int i,j,nex,bin,cnt;
	register short *p;

	cnt = 0;
	nex = nlp-nc;
	p = image;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			if (nzflag && *p == 0) {
				p++;
				continue;
			}
			cnt++;
			bin = (int) ((*p++ - min)/width);
			if (bin < 0)
				histo[0]++;
			else if (bin >= nbins)
				histo[nbins+1]++;
			else
				histo[bin+1]++;
		}
		p += nex;
	}
	*count = cnt;
	return(HIPS_OK);
}

int h_histo_US(image,nr,nc,nlp,nbins,histo,min,width,nzflag,count)

h_ushort *image,min,width;
int nr,nc,nlp,nbins,*histo;
h_boolean nzflag;
int *count;

{
	register int i,j,nex,bin,cnt;
	register h_ushort *p;

	cnt = 0;
	nex = nlp-nc;
	p = image;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			if (nzflag && *p == 0) {
				p++;
				continue;
			}
			cnt++;
			bin = (int) ((*p++ - min)/width);
			if (bin < 0)
				histo[0]++;
			else if (bin >= nbins)
				histo[nbins+1]++;
			else
				histo[bin+1]++;
		}
		p += nex;
	}
	*count = cnt;
	return(HIPS_OK);
}

int h_histo_I(image,nr,nc,nlp,nbins,histo,min,width,nzflag,count)

int *image,min,width;
int nr,nc,nlp,nbins,*histo;
h_boolean nzflag;
int *count;

{
	register int i,j,nex,bin,cnt;
	register int *p;

	cnt = 0;
	nex = nlp-nc;
	p = image;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			if (nzflag && *p == 0) {
				p++;
				continue;
			}
			cnt++;
			bin = (int) ((*p++ - min)/width);
			if (bin < 0)
				histo[0]++;
			else if (bin >= nbins)
				histo[nbins+1]++;
			else
				histo[bin+1]++;
		}
		p += nex;
	}
	*count = cnt;
	return(HIPS_OK);
}

int h_histo_UI(image,nr,nc,nlp,nbins,histo,min,width,nzflag,count)

h_uint *image,min,width;
int nr,nc,nlp,nbins,*histo;
h_boolean nzflag;
int *count;

{
	register int i,j,nex,bin,cnt;
	register h_uint *p;

	cnt = 0;
	nex = nlp-nc;
	p = image;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			if (nzflag && *p == 0) {
				p++;
				continue;
			}
			cnt++;
			bin = (int) ((*p++ - min)/width);
			if (bin < 0)
				histo[0]++;
			else if (bin >= nbins)
				histo[nbins+1]++;
			else
				histo[bin+1]++;
		}
		p += nex;
	}
	*count = cnt;
	return(HIPS_OK);
}

int h_histo_F(image,nr,nc,nlp,nbins,histo,min,width,nzflag,count)

float *image,min,width;
int nr,nc,nlp,nbins,*histo;
h_boolean nzflag;
int *count;

{
	register int i,j,nex,bin,cnt;
	register float *p;

	cnt = 0;
	nex = nlp-nc;
	p = image;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			if (nzflag && *p == 0) {
				p++;
				continue;
			}
			cnt++;
			bin = (int) ((*p++ - min)/width);
			if (bin < 0)
				histo[0]++;
			else if (bin >= nbins)
				histo[nbins+1]++;
			else
				histo[bin+1]++;
		}
		p += nex;
	}
	*count = cnt;
	return(HIPS_OK);
}

int h_histo_D(image,nr,nc,nlp,nbins,histo,min,width,nzflag,count)

double *image,min,width;
int nr,nc,nlp,nbins,*histo;
h_boolean nzflag;
int *count;

{
	register int i,j,nex,bin,cnt;
	register double *p;

	cnt = 0;
	nex = nlp-nc;
	p = image;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			if (nzflag && *p == 0) {
				p++;
				continue;
			}
			cnt++;
			bin = (int) ((*p++ - min)/width);
			if (bin < 0)
				histo[0]++;
			else if (bin >= nbins)
				histo[nbins+1]++;
			else
				histo[bin+1]++;
		}
		p += nex;
	}
	*count = cnt;
	return(HIPS_OK);
}

int h_histo_C(image,nr,nc,nlp,nbins,histo,min,width,nzflag,count)

float *image,min,width;
int nr,nc,nlp,nbins,*histo;
h_boolean nzflag;
int *count;

{
	register int i,j,nex,bin,cnt;
	register float *p,sqmagn;

	cnt = 0;
	nex = 2*(nlp-nc);
	p = image;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			sqmagn = p[0]*p[0]+p[1]*p[1];
			if (nzflag && sqmagn == 0) {
				p += 2;
				continue;
			}
			cnt++;
			bin = (int) ((sqrt((double) sqmagn) - min)/width);
			p += 2;
			if (bin < 0)
				histo[0]++;
			else if (bin >= nbins)
				histo[nbins+1]++;
			else
				histo[bin+1]++;
		}
		p += nex;
	}
	*count = cnt;
	return(HIPS_OK);
}

int h_histo_DC(image,nr,nc,nlp,nbins,histo,min,width,nzflag,count)

double *image,min,width;
int nr,nc,nlp,nbins,*histo;
h_boolean nzflag;
int *count;

{
	register int i,j,nex,bin,cnt;
	register double *p,sqmagn;

	cnt = 0;
	nex = 2*(nlp-nc);
	p = image;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			sqmagn = p[0]*p[0]+p[1]*p[1];
			if (nzflag && sqmagn == 0) {
				p += 2;
				continue;
			}
			cnt++;
			bin = (int) ((sqrt(sqmagn) - min)/width);
			p += 2;
			if (bin < 0)
				histo[0]++;
			else if (bin >= nbins)
				histo[nbins+1]++;
			else
				histo[bin+1]++;
		}
		p += nex;
	}
	*count = cnt;
	return(HIPS_OK);
}
