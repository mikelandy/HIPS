/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_seqord.c - subroutines to reorder a Walsh transform in sequency order
 *
 * pixel formats: INT, FLOAT
 *
 * Yoav Cohen - 2/18/82
 * HIPS 2 - msl - 8/11/91
 * fixed pointer error in h_seqord and h_invseqord - rld 11/01/08
 */

#include <hipl_format.h>
#include <math.h>

int invorder();

int h_seqord(hdi,hdo)

struct header *hdi,*hdo;

{
	switch(hdi->pixel_format) {
	case PFINT:	return(h_seqord_i(hdi,hdo));
	case PFFLOAT:	return(h_seqord_f(hdi,hdo));
	default:	return(perr(HE_FMTSUBR,"h_seqord",
				hformatname(hdi->pixel_format)));
	}
}

int h_seqord_i(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_seqord_I((int *) hdi->firstpix,(int *) hdo->firstpix,hdi->rows,
		hdi->cols,hdi->ocols,hdo->ocols));
}

int h_seqord_f(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_seqord_F((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols));
}

int h_seqord_I(imagei,imageo,nr,nc,nlpi,nlpo)

int *imagei,*imageo,nr,nc,nlpi,nlpo;

{
	int i,j,*po,nexo;

	po = imageo;
	nexo = nlpo - nc;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*po++ = imagei[invorder(i,nr)*nlpi + invorder(j,nc)];
		po += nexo;
	}
	return(HIPS_OK);
}

int h_seqord_F(imagei,imageo,nr,nc,nlpi,nlpo)

float *imagei,*imageo;
int nr,nc,nlpi,nlpo;

{
	int i,j,nexo;
	float *po;

	po = imageo;
	nexo = nlpo - nc;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*po++ = imagei[invorder(i,nr)*nlpi + invorder(j,nc)];
		po += nexo;
	}
	return(HIPS_OK);
}

int h_invseqord(hdi,hdo)

struct header *hdi,*hdo;

{
	switch(hdi->pixel_format) {
	case PFINT:	return(h_invseqord_i(hdi,hdo));
	case PFFLOAT:	return(h_invseqord_f(hdi,hdo));
	default:	return(perr(HE_FMTSUBR,"h_invseqord",
				hformatname(hdi->pixel_format)));
	}
}

int h_invseqord_i(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_invseqord_I((int *) hdi->firstpix,(int *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols));
}

int h_invseqord_f(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_invseqord_F((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols));
}

int h_invseqord_I(imagei,imageo,nr,nc,nlpi,nlpo)

int *imagei,*imageo,nr,nc,nlpi,nlpo;

{
	int i,j,*pi,nexi;

	pi = imagei;
	nexi = nlpi - nc;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			imageo[invorder(i,nr)*nlpo + invorder(j,nc)] = *pi++;
		pi += nexi;
	}
	return(HIPS_OK);
}

int h_invseqord_F(imagei,imageo,nr,nc,nlpi,nlpo)

float *imagei,*imageo;
int nr,nc,nlpi,nlpo;

{
	int i,j,nexi;
	float *pi;

	pi = imagei;
	nexi = nlpi - nc;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			imageo[invorder(i,nr)*nlpo + invorder(j,nc)] = *pi++;
		pi += nexi;
	}
	return(HIPS_OK);
}

/*
 * Given an index in an ordered vector of length "len"
 * return the index in an un-ordered vector.
 */

int invorder(index,len)

int index,len;

{
	int l2;

	if (len <= 2)
		return(index);
	l2 = len>>1;
	if (index<l2)
		return(invorder(index,l2));
	index = invorder(index-l2,l2);
	return(len-1-index);
}
