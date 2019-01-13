/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_invfourtr.c - subroutines to compute an inverse Fourier transform
 *
 * The transform is computed `in-place'.
 *
 * pixel formats: COMPLEX, DOUBLE
 *
 * Michael Landy - 7/10/91
 */

#include <hipl_format.h>
#include <math.h>

int h_invfourtr(hd)

struct header *hd;

{
	switch(hd->pixel_format) {
	case PFCOMPLEX:	return(h_invfourtr_c(hd));
	case PFDBLCOM:	return(h_invfourtr_dc(hd));
	default:	return(perr(HE_FMTSUBR,"h_invfourtr",
				hformatname(hd->pixel_format)));
	}
}

int h_invfourtr_c(hd)

struct header *hd;

{
	return(h_invfourtr_C((h_complex *) hd->firstpix,hd->rows,hd->cols,
		hd->ocols));
}

int h_invfourtr_dc(hd)

struct header *hd;

{
	return(h_invfourtr_DC((h_dblcom *) hd->firstpix,hd->rows,hd->cols,
		hd->ocols));
}

int h_invfourtr_C(image,nr,nc,nlp)

h_complex *image;
int nr,nc,nlp;

{
	int i,j,logrows,logcols,nex;
	h_complex *p;

	logrows = logcols = -1;
	i = 0; j = 1;
	while (1) {
		if (j == nr)
			logrows = i;
		if (j == nc)
			logcols = i;
		if (j >= nr && j >= nc)
			break;
		i++; j <<= 1;
	}
	if (logrows == -1 || logcols == -1)
		return(perr(HE_POW2));
	nex = nlp-nc;
	p = image;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			p[0][1] = - p[0][1];
			p++;
		}
		p += nex;
	}
	if (h_fft2dgen_c(image,logrows,logcols,nlp) == HIPS_ERROR)
		return(HIPS_ERROR);
	p = image;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			p[0][1] = - p[0][1];
			p++;
		}
		p += nex;
	}
	return(HIPS_OK);
}

int h_invfourtr_DC(image,nr,nc,nlp)

h_dblcom *image;
int nr,nc,nlp;

{
	int i,j,logrows,logcols,nex;
	h_dblcom *p;

	logrows = logcols = -1;
	i = 0; j = 1;
	while (1) {
		if (j == nr)
			logrows = i;
		if (j == nc)
			logcols = i;
		if (j >= nr && j >= nc)
			break;
		i++; j <<= 1;
	}
	if (logrows == -1 || logcols == -1)
		return(perr(HE_POW2));
	nex = nlp-nc;
	p = image;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			p[0][1] = - p[0][1];
			p++;
		}
		p += nex;
	}
	if (h_fft2dgen_dc(image,logrows,logcols,nlp) == HIPS_ERROR)
		return(HIPS_ERROR);
	p = image;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			p[0][1] = - p[0][1];
			p++;
		}
		p += nex;
	}
	return(HIPS_OK);
}
