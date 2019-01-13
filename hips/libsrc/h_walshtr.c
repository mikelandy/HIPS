/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_walshtr.c - subroutines to compute a Walsh transform
 *
 * The transform is computed `in-place'.  Regions-of-interest are ignored.
 * Both image dimensions must be powers of 2.
 *
 * pixel formats: INT, FLOAT
 *
 * Yoav Cohen - 2/18/82
 * HIPS 2 - msl - 8/11/91
 */

#include <hipl_format.h>
#include <math.h>

int h_walshtr(hd)

struct header *hd;

{
	switch(hd->pixel_format) {
	case PFINT:	return(h_walshtr_i(hd));
	case PFFLOAT:	return(h_walshtr_f(hd));
	default:	return(perr(HE_FMTSUBR,"h_walshtr",
				hformatname(hd->pixel_format)));
	}
}

int h_walshtr_i(hd)

struct header *hd;

{
	return(h_walshtr_I((int *) hd->image,hd->orows,hd->ocols));
}

int h_walshtr_f(hd)

struct header *hd;

{
	return(h_walshtr_F((float *) hd->image,hd->orows,hd->ocols));
}

int h_walshtr_I(image,nr,nc)

int *image;
int nr,nc;

{
	int i,j,logrows,logcols;

	logrows = logcols = -1;
	i = 0; j = 1;
	while (1) {
		if (j == nr)
			logrows = i;
		else if (j > nr)
			break;
		if (j == nc)
			logcols = i;
		else if (j > nc)
			break;
		if (logrows >= 0 && logcols >= 0)
			break;
		i++; j <<= 1;
	}
	if (logrows == -1 || logcols == -1)
		return(perr(HE_POW2));
	return(h_fwt_i(image,logrows+logcols));
}

int h_walshtr_F(image,nr,nc)

float *image;
int nr,nc;

{
	int i,j,logrows,logcols;

	logrows = logcols = -1;
	i = 0; j = 1;
	while (1) {
		if (j == nr)
			logrows = i;
		else if (j > nr)
			break;
		if (j == nc)
			logcols = i;
		else if (j > nc)
			break;
		if (logrows >= 0 && logcols >= 0)
			break;
		i++; j <<= 1;
	}
	if (logrows == -1 || logcols == -1)
		return(perr(HE_POW2));
	return(h_fwt_f(image,logrows+logcols));
}

/*
 * fwt -- fast walsh transform, adapted from Gonzalez & Wintz p.95.
 *
 * note that the coefficients are not divided by N.
 * The program will not overflow as long as
 * loglen+bits per pixel <= 31.
 *
 * h_fwt_i(vec,loglen)	- integer version
 * h_fwt_f(vec,loglen)	- floating point version
 */

int h_fwt_i(vec,loglen)

int *vec,loglen;

{
	int t,n,nv2,nm1,i,j,k,l,le,le1;
	int *veci,*vecj;

	n = 1<<loglen;
	nv2 = n >> 1;
	nm1 = n-1;
	j = 1;
	for (i=1;i<=nm1;i++) {
		if (i<j) {
			veci = vec+i-1;
			vecj = vec+j-1;
			t = (*vecj);
			*vecj = (*veci);
			*veci = t;
		}
		k = nv2;
		while (k<j) {
			j -= k;
			k >>= 1;
		}
		j += k;
	}
	le = 1;
	for (l=1;l<=loglen;l++) {
		le1 = le;
		le += le;
		for (j=0;j<le1;j++)
		    for (i=j;i<n;i+=le) {
			veci = vec+i;
			vecj = veci+le1;
			t = (*vecj);
			*vecj = (*veci-t);
			*veci += t;
		    }
	}
	return(HIPS_OK);
}

int h_fwt_f(vec,loglen)

float *vec;
int loglen;

{
	int n,nv2,nm1,i,j,k,l,le,le1;
	float t,*veci,*vecj;

	n = 1<<loglen;
	nv2 = n >> 1;
	nm1 = n-1;
	j = 1;
	for (i=1;i<=nm1;i++) {
		if (i<j) {
			veci = vec+i-1;
			vecj = vec+j-1;
			t = (*vecj);
			*vecj = (*veci);
			*veci = t;
		}
		k = nv2;
		while (k<j) {
			j -= k;
			k >>= 1;
		}
		j += k;
	}
	le = 1;
	for (l=1;l<=loglen;l++) {
		le1 = le;
		le += le;
		for (j=0;j<le1;j++)
			for (i=j;i<n;i+=le) {
				veci = vec+i;
				vecj = veci+le1;
				t = (*vecj);
				*vecj = (*veci-t);
				*veci+=t;
			}
	}
	return(HIPS_OK);
}
