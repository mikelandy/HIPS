/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_correl.c - subroutines to cross-correlate two images
 *
 * Increasing pixel positions in the output correspond to increases shifts of
 * the second image over the first.  The user supplies the initial shift used
 * to produce the first output pixel.
 *
 * pixel formats: FLOAT
 *
 * Michael Landy - 4/8/89
 * Hips 2 - msl - 8/10/91
 */

#include <hipl_format.h>

int h_correl(hdi1,hdi2,hdo,dr0,dc0)

struct header *hdi1,*hdi2,*hdo;
int dr0,dc0;

{
	switch(hdi1->pixel_format) {
	case PFFLOAT:	return(h_correl_f(hdi1,hdi2,hdo,dr0,dc0));
	default:	return(perr(HE_FMTSUBR,"h_correl",
				hformatname(hdi1->pixel_format)));
	}
}

int h_correl_f(hdi1,hdi2,hdo,dr0,dc0)

struct header *hdi1,*hdi2,*hdo;
int dr0,dc0;

{
	return(h_correl_F((float *) hdi1->firstpix,(float *) hdi2->firstpix,
		(float *) hdo->firstpix,hdi1->rows,hdi1->cols,hdi2->rows,
		hdi2->cols,hdo->rows,hdo->cols,
		hdi1->ocols,hdi2->ocols,hdo->ocols,dr0,dc0));
}

int h_correl_F(imagei1,imagei2,imageo,nr1,nc1,nr2,nc2,nro,nco,nlpi1,nlpi2,nlpo,
	dr0,dc0)

float *imagei1,*imagei2,*imageo;
int nr1,nc1,nr2,nc2,nro,nco,nlpi1,nlpi2,nlpo,dr0,dc0;

{
	register int i,j,nexi1,nexi2,nexo,dr,dc,r,c;
	int firstrow1,firstcol1,firstrow2,firstcol2,roverlap,coverlap;
	float *pi1,*pi2,*po;
	double sum;

	nexo = nlpo-nco;
	po = imageo;
	dr = dr0;
	for (r=0;r<nro;r++,dr++) {
		if (dr >= nr1 || -dr >= nr2) {	/* no overlap this row */
			for (c=0;c<nco;c++)
				*po++ = 0.;
			po += nexo;
			continue;
		}
		if (dr > 0) {
			firstrow1 = dr;
			firstrow2 = 0;
			roverlap = (nr1 - dr) < nr2 ? (nr1 - dr) : nr2;
		}
		else {
			firstrow1 = 0;
			firstrow2 = -dr;
			roverlap = (nr2 + dr) < nr1 ? (nr2 + dr) : nr1;
		}
		dc = dc0;
		for (c=0;c<nco;c++,dc++) {
			if (dc >= nc1 || -dc >= nc2) {
				*po++ = 0.;
				continue;
			}
			if (dc > 0) {
			    firstcol1 = dc;
			    firstcol2 = 0;
			    coverlap = (nc1 - dc) < nc2 ? (nc1 - dc) : nc2;
			}
			else {
			    firstcol1 = 0;
			    firstcol2 = -dc;
			    coverlap = (nc2 + dc) < nc1 ? (nc2 + dc) : nc1;
			}
			sum = 0.;
			pi1 = imagei1 + firstrow1*nlpi1 + firstcol1;
			pi2 = imagei2 + firstrow2*nlpi2 + firstcol2;
			nexi1 = nlpi1 - coverlap;
			nexi2 = nlpi2 - coverlap;
			for (i=0;i<roverlap;i++) {
				for (j=0;j<coverlap;j++) {
					sum += *pi1++ * *pi2++;
				}
				pi1 += nexi1;
				pi2 += nexi2;
			}
			*po++ = sum;
		}
		po += nexo;
	}
	return(HIPS_OK);
}
