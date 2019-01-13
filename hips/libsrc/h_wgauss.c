/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_wgauss.c - Window an image with a 2-dimensional Gaussian
 *
 * multiplies an input image by a 2-dimensional Gaussian with mean
 * (rowmu,colmu) and standard deviation (rowsigma,colsigma).  The resulting
 * window has a peak value of factor (allowing one to factor in a temporal
 * Gaussian).  Negative values of either sigma are taken to indicate an
 * infinite Gaussian (identically 1).
 *
 * pixel formats: FLOAT
 *
 * Michael Landy - 8/10/91
 */

#include <hipl_format.h>
#include <math.h>

static double *rowmult,*colmult,saverowsigma,savecolsigma,savecolmu,saverowmu;
static int wralloc = FALSE;
static int wcalloc = FALSE;
static int savenr = -1;
static int savenc = -1;

int h_wgauss(hdi,hdo,rowmu,colmu,rowsigma,colsigma,factor)

struct header *hdi,*hdo;
double rowmu,colmu,rowsigma,colsigma,factor;

{
	switch(hdi->pixel_format) {
	case PFFLOAT:	return(h_wgauss_f(hdi,hdo,rowmu,colmu,rowsigma,
				colsigma,factor));
	default:	return(perr(HE_FMTSUBR,"h_wgauss",
				hformatname(hdi->pixel_format)));
	}
}

int h_wgauss_f(hdi,hdo,rowmu,colmu,rowsigma,colsigma,factor)

struct header *hdi,*hdo;
double rowmu,colmu,rowsigma,colsigma,factor;

{
	return(h_wgauss_F((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,rowmu,colmu,rowsigma,
		colsigma,factor));
}

int h_wgauss_F(imagei,imageo,nr,nc,nlpi,nlpo,rowmu,colmu,rowsigma,colsigma,factor)

float *imagei,*imageo;
int nr,nc,nlpi,nlpo;
double rowmu,colmu,rowsigma,colsigma,factor;

{
	int i,j,nexi,nexo;
	double *p,diff,twosigmasq,mult;
	float *pi,*po;

	if (wralloc && savenr != nr) {
		free(rowmult);
		wralloc = FALSE;
	}
	if (!wralloc) {
		if ((rowmult = (double *) memalloc(nr,sizeof(double))) ==
			(double *) HIPS_ERROR)
				return(HIPS_ERROR);
		wralloc = TRUE;
		savenr = nr;
		saverowsigma = rowsigma + 1.;  /* force computation */
	}
	if (saverowsigma != rowsigma || saverowmu != rowmu) {
		p = rowmult;
		if (rowsigma < 0) {
			for (i=0;i<nr;i++)
				*p++ = 1.;
		}
		else {
			twosigmasq = 2.*rowsigma*rowsigma;
			for (i=0;i<nr;i++) {
				diff = rowmu - i;
				*p++ = exp(-diff*diff/twosigmasq);
			}
		}
		saverowsigma = rowsigma;
		saverowmu = rowmu;
	}
	if (wcalloc && savenc != nc) {
		free(colmult);
		wcalloc = FALSE;
	}
	if (!wcalloc) {
		if ((colmult = (double *) memalloc(nc,sizeof(double))) ==
			(double *) HIPS_ERROR)
				return(HIPS_ERROR);
		wcalloc = TRUE;
		savenc = nc;
		savecolsigma = colsigma + 1.;  /* force computation */
	}
	if (savecolsigma != colsigma || savecolmu != colmu) {
		p = colmult;
		if (colsigma < 0) {
			for (i=0;i<nc;i++)
				*p++ = 1.;
		}
		else {
			twosigmasq = 2.*colsigma*colsigma;
			for (i=0;i<nc;i++) {
				diff = colmu - i;
				*p++ = exp(-diff*diff/twosigmasq);
			}
		}
		saverowsigma = rowsigma;
		saverowmu = rowmu;
	}
	nexi = nlpi - nc;
	nexo = nlpo - nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		mult = factor * rowmult[i];
		p = colmult;
		for (j=0;j<nc;j++)
			*po++ = *pi++ * *p++ * mult;
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

