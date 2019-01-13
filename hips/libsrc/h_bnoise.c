/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_bnoise.c - add binomial noise to an image
 *
 * pixel formats: BYTE, INT, FLOAT
 *
 * Adds binomially distributed noise.  Each pixel is summed with a random
 * variable mulc*B + addc, where B is binomially distributed, and in
 * particular, equal to the sum of n independent Bernoullian random variables,
 * each taking the value 1 with probability p and 0 otherwise.  Thus the random
 * variable mulc*B + addc has expectation mulc*Np + addc -- so set
 * addc = -mulc*N*p to make the expectation 0 -- and variance mulc*mulc*Np(1-p).
 *
 * Charlie Chubb - 10/15/87
 * HIPS 2 - msl - 8/6/91
 */

#include <hipl_format.h>
#include <math.h>

#define	 MAXSIGNED	((int)(((unsigned)(~0)) >> 1))

static int *lookup;
static int lalloc = FALSE;
static int saven = -1;
static double savep = -1;
int compute_blookup();

int h_bnoise(hdi,hdo,n,p,addc,mulc)

struct header *hdi,*hdo;
int n;
double p;
Pixelval *addc,*mulc;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	return(h_bnoise_b(hdi,hdo,n,p,addc,mulc));
	case PFINT:	return(h_bnoise_i(hdi,hdo,n,p,addc,mulc));
	case PFFLOAT:	return(h_bnoise_f(hdi,hdo,n,p,addc,mulc));
	default:	return(perr(HE_FMTSUBR,"h_bnoise",
				hformatname(hdi->pixel_format)));
	}
}

int h_bnoise_b(hdi,hdo,n,p,addc,mulc)

struct header *hdi,*hdo;
int n;
double p;
Pixelval *addc,*mulc;

{
	return(h_bnoise_B(hdi->firstpix,hdo->firstpix,hdi->rows,
		hdi->cols,hdi->ocols,hdo->ocols,n,p,addc->v_int,mulc->v_int));
}

int h_bnoise_i(hdi,hdo,n,p,addc,mulc)

struct header *hdi,*hdo;
int n;
double p;
Pixelval *addc,*mulc;

{
	return(h_bnoise_I((int *) hdi->firstpix,(int *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,n,p,addc->v_int,
		mulc->v_int));
}

int h_bnoise_f(hdi,hdo,n,p,addc,mulc)

struct header *hdi,*hdo;
int n;
double p;
Pixelval *addc,*mulc;

{
	return(h_bnoise_F((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,n,p,addc->v_float,
		mulc->v_float));
}

int h_bnoise_B(imagei,imageo,nr,nc,nlpi,nlpo,n,p,addc,mulc)

byte *imagei,*imageo;
int nr,nc,nlpi,nlpo,n,addc,mulc;
double p;

{
	register int i,j,nexi,nexo;
	register byte *pi,*po;
	int hi,lo,himid,lomid,rv,val;

	if (!lalloc || saven != n || savep != p)
		if (compute_blookup(n,p) == HIPS_ERROR)
			return(HIPS_ERROR);
	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	hips_lclip = hips_hclip = 0;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			rv = H__RANDOM();
			hi = n;
			lo = 0;
			while (hi != lo) {
				himid = (hi + lo + 1)/2;
				if (rv > lookup[himid])
					lo = himid;
				else {
					lomid = (hi + lo)/2;
					if (rv <= lookup[lomid])
						hi = lomid;
					else
						hi = lo = himid;
				}
			}
			val = *pi++ + mulc*hi + addc;
			if (val < 0) {
				*po++ = 0;
				hips_lclip++;
			}
			else if (val > 255) {
				*po++ = 255;
				hips_hclip++;
			}
			else
				*po++ = val;
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_bnoise_I(imagei,imageo,nr,nc,nlpi,nlpo,n,p,addc,mulc)

int *imagei,*imageo;
int nr,nc,nlpi,nlpo,n,addc,mulc;
double p;

{
	register int i,j,nexi,nexo;
	register int *pi,*po;
	int hi,lo,himid,lomid,rv;

	if (!lalloc || saven != n || savep != p)
		if (compute_blookup(n,p) == HIPS_ERROR)
			return(HIPS_ERROR);
	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			rv = H__RANDOM();
			hi = n;
			lo = 0;
			while (hi != lo) {
				himid = (hi + lo + 1)/2;
				if (rv > lookup[himid])
					lo = himid;
				else {
					lomid = (hi + lo)/2;
					if (rv <= lookup[lomid])
						hi = lomid;
					else
						hi = lo = himid;
				}
			}
			*po++ = *pi++ + mulc*hi + addc;
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_bnoise_F(imagei,imageo,nr,nc,nlpi,nlpo,n,p,addc,mulc)

float *imagei,*imageo,addc,mulc;
int nr,nc,nlpi,nlpo,n;
double p;

{
	register int i,j,nexi,nexo;
	register float *pi,*po;
	int hi,lo,himid,lomid,rv;

	if (!lalloc || saven != n || savep != p)
		if (compute_blookup(n,p) == HIPS_ERROR)
			return(HIPS_ERROR);
	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			rv = H__RANDOM();
			hi = n;
			lo = 0;
			while (hi != lo) {
				himid = (hi + lo + 1)/2;
				if (rv > lookup[himid])
					lo = himid;
				else {
					lomid = (hi + lo)/2;
					if (rv <= lookup[lomid])
						hi = lomid;
					else
						hi = lo = himid;
				}
			}
			*po++ = *pi++ + mulc*hi + addc;
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int compute_blookup(n,p)

int n;
double p;

{
	double *cum_ln,*prob,cum_prob,q;
	int k;

	if (lalloc && n != saven) {
		free(lookup);
		lalloc = FALSE;
	}
	q = 1-p;
	if ((cum_ln = (double *) hmalloc((n+1)*sizeof(double))) ==
		(double *) HIPS_ERROR)
			return(HIPS_ERROR);
	cum_ln[0] = 0.;
	for (k=1;k<=n;k++)
		cum_ln[k] = log((double)k) + cum_ln[k-1];
	if ((prob = (double *) hmalloc((n+1)*sizeof(double))) ==
		(double *) HIPS_ERROR)
			return(HIPS_ERROR);
	for (k=0;k<=n;k++)
		prob[k] = exp(
				((double)k) * log(p)
				+
				((double)(n-k)) * log(q)
				+
				cum_ln[n]
				-
				(
					cum_ln[k]
					+
					cum_ln[n-k]
				)
			);
	free(cum_ln);
	if (!lalloc)
		if ((lookup = (int *) hmalloc((n+1)*sizeof(int))) ==
			(int *) HIPS_ERROR)
				return(HIPS_ERROR);
	for (cum_prob=0.,k=0;k<=n;k++)
		lookup[k] = (int)((cum_prob+=prob[k]) * ((double)MAXSIGNED));
	free(prob);
	lalloc = TRUE;
	saven = n;
	savep = p;
	return(HIPS_OK);
}
