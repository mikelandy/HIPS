/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_convolve.c - subroutines to perform a true 3D convolution on two images
 *
 * The first header is for the `input' sequence.  The buffer allocated for
 * that image contains only those frames which will be required for this
 * frame's computation (and hence the buffer length is sufficient only to hold
 * the number of frames in the `kernel' sequence.  The calling program reads
 * these as if into a circular queue, so the user specifies which frame in the
 * buffer corresponds to the first frame to use, the number to convolve, and
 * the first one in the `kernel' buffer to use.  The `kernel' buffer contains
 * the entire `kernel' sequence.  Regions of interest are supported, but are
 * a bad idea for space reasons.
 *
 * pixel formats: INT, FLOAT
 *
 * Michael Landy - 8/10/91
 */

#include <hipl_format.h>

int h_convolve(hdi,hdk,hdo,firsti,firstk,numf)

struct header *hdi,*hdk,*hdo;
int firsti,firstk,numf;

{
	switch(hdi->pixel_format) {
	case PFINT:	return(h_convolve_i(hdi,hdk,hdo,firsti,firstk,numf));
	case PFFLOAT:	return(h_convolve_f(hdi,hdk,hdo,firsti,firstk,numf));
	default:	return(perr(HE_FMTSUBR,"h_convolve",
				hformatname(hdi->pixel_format)));
	}
}

int h_convolve_i(hdi,hdk,hdo,firsti,firstk,numf)

struct header *hdi,*hdk,*hdo;
int firsti,firstk,numf;

{
	return(h_convolve_I((int *) hdi->firstpix,(int *) hdk->firstpix,
		(int *) hdo->firstpix,hdi->rows,hdi->cols,
		hdi->orows,hdi->ocols,hdk->rows,hdk->cols,hdk->orows,
		hdk->ocols,hdk->num_frame,hdo->ocols,firsti,firstk,numf));
}

int h_convolve_f(hdi,hdk,hdo,firsti,firstk,numf)

struct header *hdi,*hdk,*hdo;
int firsti,firstk,numf;

{
	return(h_convolve_F((float *) hdi->firstpix,(float *) hdk->firstpix,
		(float *) hdo->firstpix,hdi->rows,hdi->cols,
		hdi->orows,hdi->ocols,hdk->rows,hdk->cols,hdk->orows,
		hdk->ocols,hdk->num_frame,hdo->ocols,firsti,firstk,numf));
}

int h_convolve_I(imagei,imagek,imageo,nri,nci,tnri,nlpi,nrk,nck,tnrk,nlpk,nfrk,
	nlpo,firsti,firstk,numf)

int *imagei,*imagek,*imageo;
int nri,nci,tnri,nlpi,nrk,nck,tnrk,nlpk,nfrk,nlpo,firsti,firstk,numf;

{
	int i,j,k,l,f,nexi,nexk,nexo,nro,nco,npixi,npixk;
	int *pi,*pk,*po,*po2,*pk2,*pi2;
	h_boolean bigkernel;

	nro = nri + nrk - 1;
	nco = nci + nck - 1;
	h_setimage_I(imageo,nro,nco,nlpo,0);
	npixi = nlpi*tnri;
	npixk = nlpk*tnrk;
	bigkernel = npixk > npixi;
	nexk = nlpk - nck;
	nexo = bigkernel ? (nlpo - nck) : (nlpo - nci);
	nexi = nlpi - nci;
	for (f=0;f<numf;f++) {
		pi = imagei + ((firsti + f) % nfrk)*npixi;
		pk = imagek + (firstk + numf - f - 1)*npixk;
		if (bigkernel) {
			for (i=0;i<nri;i++) {
				po = imageo + i*nlpo;
				for (j=0;j<nci;j++) {
					po2 = po + j;
					pk2 = pk;
					for (k=0;k<nrk;k++) {
						for (l=0;l<nck;l++)
							*po2++ += *pi * *pk2++;
						pk2 += nexk;
						po2 += nexo;
					}
					pi++;
				}
				pi += nexi;
			}
		}
		else {
			for (i=0;i<nrk;i++) {
				po = imageo + i*nlpo;
				for (j=0;j<nck;j++) {
					po2 = po + j;
					pi2 = pi;
					for (k=0;k<nri;k++) {
						for (l=0;l<nci;l++)
							*po2++ += *pk * *pi2++;
						pi2 += nexi;
						po2 += nexo;
					}
					pk++;
				}
				pk += nexk;
			}
		}
	}
	return(HIPS_OK);
}

int h_convolve_F(imagei,imagek,imageo,nri,nci,tnri,nlpi,nrk,nck,tnrk,nlpk,nfrk,
	nlpo,firsti,firstk,numf)

float *imagei,*imagek,*imageo;
int nri,nci,tnri,nlpi,nrk,nck,tnrk,nlpk,nfrk,nlpo,firsti,firstk,numf;

{
	int i,j,k,l,f,nexi,nexk,nexo,nro,nco,npixi,npixk;
	float *pi,*pk,*po,*po2,*pk2,*pi2;
	h_boolean bigkernel;

	nro = nri + nrk - 1;
	nco = nci + nck - 1;
	h_setimage_F(imageo,nro,nco,nlpo,0);
	npixi = nlpi*tnri;
	npixk = nlpk*tnrk;
	bigkernel = npixk > npixi;
	nexk = nlpk - nck;
	nexo = bigkernel ? (nlpo - nck) : (nlpo - nci);
	nexi = nlpi - nci;
	for (f=0;f<numf;f++) {
		pi = imagei + ((firsti + f) % nfrk)*npixi;
		pk = imagek + (firstk + numf - f - 1)*npixk;
		if (bigkernel) {
			for (i=0;i<nri;i++) {
				po = imageo + i*nlpo;
				for (j=0;j<nci;j++) {
					po2 = po + j;
					pk2 = pk;
					for (k=0;k<nrk;k++) {
						for (l=0;l<nck;l++)
							*po2++ += *pi * *pk2++;
						pk2 += nexk;
						po2 += nexo;
					}
					pi++;
				}
				pi += nexi;
			}
		}
		else {
			for (i=0;i<nrk;i++) {
				po = imageo + i*nlpo;
				for (j=0;j<nck;j++) {
					po2 = po + j;
					pi2 = pi;
					for (k=0;k<nri;k++) {
						for (l=0;l<nci;l++)
							*po2++ += *pk * *pi2++;
						pi2 += nexi;
						po2 += nexo;
					}
					pk++;
				}
				pk += nexk;
			}
		}
	}
	return(HIPS_OK);
}
