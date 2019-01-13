/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_fourtr.c - subroutines to compute a Fourier transform
 *
 * The transform is computed `in-place'.
 *
 * pixel formats: COMPLEX, DBLCOM
 *
 * Michael Landy - 7/10/91
 */

#include <hipl_format.h>
#include <math.h>

int h_fourtr(hd)

struct header *hd;

{
	switch(hd->pixel_format) {
	case PFCOMPLEX:	return(h_fourtr_c(hd));
	case PFDBLCOM:	return(h_fourtr_dc(hd));
	default:	return(perr(HE_FMTSUBR,"h_fourtr",
				hformatname(hd->pixel_format)));
	}
}

int h_fourtr_c(hd)

struct header *hd;

{
	return(h_fourtr_C((h_complex *) hd->firstpix,hd->rows,hd->cols,
		hd->ocols));
}

int h_fourtr_dc(hd)

struct header *hd;

{
	return(h_fourtr_DC((h_dblcom *) hd->firstpix,hd->rows,hd->cols,
		hd->ocols));
}

int h_fourtr_C(image,nr,nc,nlp)

h_complex *image;
int nr,nc,nlp;

{
	int i,j,logrows,logcols;

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
	return(h_fft2dgen_c(image,logrows,logcols,nlp));
}

int h_fourtr_DC(image,nr,nc,nlp)

h_dblcom *image;
int nr,nc,nlp;

{
	int i,j,logrows,logcols;

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
	return(h_fft2dgen_dc(image,logrows,logcols,nlp));
}

/* fft -- fast fourier transform, adapted from
 * Gonzalez & Wintz p.87.
 *
 * No division by N is performed.
 *
 * Calling sequence:
 *
 * h_fft_c(vec,loglen) - performs a 1-dimensional fft
 *
 * h_fft2d_c(vec,loglen,nlp) - performs a 2-dimensional fft where loglen is the
 *				log of the length of a side of the array and
 *				nlp is the number of values per stored row
 *				(for ROI)
 *
 * h_fft2dgen_c(vec,logrows,logcols,nlp) - same for nonsquare arrays
 *
 * h_fftn_c(vec,loglen,skip) - performs a 1D fft on every skip-th entry
 *
 * h_complex vec;
 * int loglen,logrows,logcols,nlp,skip;
 *
 * h_fft_dc, h_fft2d_dc, h_fft2dgen_dc and h_fftn_dc are the double complex
 * versions.
 *
 * modified for HIPS 2 - msl - 7/10/91
 */

#define pi 3.14159265358979

static float *cconstr,*cconsti;
static int cstoresize = 0, cconstsize = 0;

int h_fft_c(vec,loglen)

h_complex *vec;
int loglen;

{
	return(h_fftn_c(vec,loglen,1));
}

int h_fft2d_c(vec,loglen,nlp)

h_complex *vec;
int loglen,nlp;

{
	int i,j,size;

	size = 1<<loglen;
	for (i=0,j=0;i<size;i++,j+=nlp)
		if (h_fftn_c(vec+j,loglen,1) == HIPS_ERROR)
			return(HIPS_ERROR);
	for (i=0;i<size;i++)
		if (h_fftn_c(vec+i,loglen,nlp) == HIPS_ERROR)
			return(HIPS_ERROR);
	return(HIPS_OK);
}

int h_fft2dgen_c(vec,logrows,logcols,nlp)

h_complex *vec;
int logrows,logcols,nlp;

{
	int i,j,rows,cols;

	rows = 1<<logrows;
	cols = 1<<logcols;
	for (i=0,j=0;i<rows;i++,j+=nlp)
		if (h_fftn_c(vec+j,logcols,1) == HIPS_ERROR)
			return(HIPS_ERROR);
	for (i=0;i<cols;i++)
		if (h_fftn_c(vec+i,logrows,nlp) == HIPS_ERROR)
			return(HIPS_ERROR);
	return(HIPS_OK);
}

int h_fftn_c(vec,loglen,nskip)

h_complex *vec;
int loglen,nskip;

{
	int n,nv2,nm1,i,j,k,l,le,le1,c,nle;
	h_complex *veci,*vecj;
	float t,wr,wi,tr,ti;


	if (loglen==0)
		return(HIPS_OK);
	n = 1<<loglen;
	nv2 = n>>1;
	nm1 = n-1;
	j = 0;
	if (cstoresize < nv2) {
		if (cstoresize) {
			free(cconstr);
			free(cconsti);
		}
		if ((cconstr = (float *) memalloc(nv2,sizeof(float))) ==
			(float *) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((cconsti = (float *) memalloc(nv2,sizeof(float))) ==
			(float *) HIPS_ERROR)
				return(HIPS_ERROR);
		cstoresize = nv2;
	}
	if (cconstsize != nv2) {
		cconstsize = nv2;
		wr =  cos((double) 2*pi/n);
		wi = -sin((double) 2*pi/n);
		cconstr[0] = 1.;
		cconsti[0] = 0.;
		for (i=1;i<nv2;i++) {
			cconstr[i] = wr*cconstr[i-1] - wi*cconsti[i-1];
			cconsti[i] = wr*cconsti[i-1] + wi*cconstr[i-1];
		}
	}
	for (i=0;i<nm1;i++) {
		if (i<j) {
			veci = vec + i*nskip;
			vecj = vec + j*nskip;
			t = vecj[0][0]; vecj[0][0] = veci[0][0]; veci[0][0] = t;
			t = vecj[0][1]; vecj[0][1] = veci[0][1]; veci[0][1] = t;
		}
		k = nv2;
		while (k <= j) {
			j -= k;
			k >>= 1;
		}
		j += k;
	}
	le = 1;
	for (l=0;l<loglen;l++) {
		le1 = le;
		le += le;
		c = 0;
		nle = n/le;
		for (j=0;j<le1;j++) {
			for (i=j;i<n;i+=le) {
				if(i+le1>=n)
					return(perr(HE_FFTI,"h_fftn_c",i+le1));
				veci = vec + i*nskip;
				vecj = vec + (i+le1)*nskip;
			  	if (c==0) {
					tr = vecj[0][0];
					ti = vecj[0][1];
				}
				else {
				    tr = vecj[0][0]*cconstr[c] -
					vecj[0][1]*cconsti[c];
				    ti = vecj[0][0]*cconsti[c] +
					vecj[0][1]*cconstr[c];
				}
				vecj[0][0] = veci[0][0] - tr;
				vecj[0][1] = veci[0][1] - ti;

				veci[0][0] += tr;
				veci[0][1] += ti;
			}
			c += nle;
		}
	}
/** Division by n
	for (i=0;i<n;i++) {
		vec[i*nskip][0] /= n;
		vec[i*nskip][1] /= n;
	}
****************/
	return(HIPS_OK);
}

static double *dcconstr,*dcconsti;
static int dcstoresize = 0, dcconstsize = 0;

int h_fft_dc(vec,loglen)

h_dblcom *vec;
int loglen;

{
	return(h_fftn_dc(vec,loglen,1));
}

int h_fft2d_dc(vec,loglen,nlp)

h_dblcom *vec;
int loglen,nlp;

{
	int i,j,size;

	size = 1<<loglen;
	for (i=0,j=0;i<size;i++,j+=nlp)
		if (h_fftn_dc(vec+j,loglen,1) == HIPS_ERROR)
			return(HIPS_ERROR);
	for (i=0;i<size;i++)
		if (h_fftn_dc(vec+i,loglen,nlp) == HIPS_ERROR)
			return(HIPS_ERROR);
	return(HIPS_OK);
}

int h_fft2dgen_dc(vec,logrows,logcols,nlp)

h_dblcom *vec;
int logrows,logcols,nlp;

{
	int i,j,rows,cols;

	rows = 1<<logrows;
	cols = 1<<logcols;
	for (i=0,j=0;i<rows;i++,j+=nlp)
		if (h_fftn_dc(vec+j,logcols,1) == HIPS_ERROR)
			return(HIPS_ERROR);
	for (i=0;i<cols;i++)
		if (h_fftn_dc(vec+i,logrows,nlp) == HIPS_ERROR)
			return(HIPS_ERROR);
	return(HIPS_OK);
}

int h_fftn_dc(vec,loglen,nskip)

h_dblcom *vec;
int loglen,nskip;

{
	int n,nv2,nm1,i,j,k,l,le,le1,c,nle;
	h_dblcom *veci,*vecj;
	double t,wr,wi,tr,ti;


	if (loglen==0)
		return(HIPS_OK);
	n = 1<<loglen;
	nv2 = n>>1;
	nm1 = n-1;
	j = 0;
	if (dcstoresize < nv2) {
		if (dcstoresize) {
			free(dcconstr);
			free(dcconsti);
		}
		if ((dcconstr = (double *) memalloc(nv2,sizeof(double))) ==
			(double *) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((dcconsti = (double *) memalloc(nv2,sizeof(double))) ==
			(double *) HIPS_ERROR)
				return(HIPS_ERROR);
		dcstoresize = nv2;
	}
	if (dcconstsize != nv2) {
		dcconstsize = nv2;
		wr =  cos((double) 2*pi/n);
		wi = -sin((double) 2*pi/n);
		dcconstr[0] = 1.;
		dcconsti[0] = 0.;
		for (i=1;i<nv2;i++) {
			dcconstr[i] = wr*dcconstr[i-1] - wi*dcconsti[i-1];
			dcconsti[i] = wr*dcconsti[i-1] + wi*dcconstr[i-1];
		}
	}
	for (i=0;i<nm1;i++) {
		if (i<j) {
			veci = vec + i*nskip;
			vecj = vec + j*nskip;
			t = vecj[0][0]; vecj[0][0] = veci[0][0]; veci[0][0] = t;
			t = vecj[0][1]; vecj[0][1] = veci[0][1]; veci[0][1] = t;
		}
		k = nv2;
		while (k <= j) {
			j -= k;
			k >>= 1;
		}
		j += k;
	}
	le = 1;
	for (l=0;l<loglen;l++) {
		le1 = le;
		le += le;
		c = 0;
		nle = n/le;
		for (j=0;j<le1;j++) {
			for (i=j;i<n;i+=le) {
				if(i+le1>=n)
					return(perr(HE_FFTI,"h_fftn_dc",i+le1));
				veci = vec + i*nskip;
				vecj = vec + (i+le1)*nskip;
			  	if (c==0) {
					tr = vecj[0][0];
					ti = vecj[0][1];
				}
				else {
				    tr = vecj[0][0]*dcconstr[c] -
					vecj[0][1]*dcconsti[c];
				    ti = vecj[0][0]*dcconsti[c] +
					vecj[0][1]*dcconstr[c];
				}
				vecj[0][0] = veci[0][0] - tr;
				vecj[0][1] = veci[0][1] - ti;

				veci[0][0] += tr;
				veci[0][1] += ti;
			}
			c += nle;
		}
	}
/** Division by n
	for (i=0;i<n;i++) {
		vec[i*nskip][0] /= n;
		vec[i*nskip][1] /= n;
	}
****************/
	return(HIPS_OK);
}
