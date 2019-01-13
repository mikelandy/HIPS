/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_extremum.c - apply a extremum filter to an image
 *
 * where size is the length of the side of the neighborhood in which the
 * extremum is computed.
 *
 * pixel formats: BYTE
 *
 * Mike Landy - 5/28/82
 * HIPS 2 - msl - 6/16/91
 */

#include <hipl_format.h>

int h_extremum(hdi,hdo,size)

struct header *hdi,*hdo;
int size;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	return(h_extremum_b(hdi,hdo,size));
	default:	return(perr(HE_FMTSUBR,"h_extremum",
				hformatname(hdi->pixel_format)));
	}
}

int h_extremum_b(hdi,hdo,size)

struct header *hdi,*hdo;
int size;

{
	return(h_extremum_B(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
	    hdi->ocols,hdo->ocols,size));
}

int h_extremum_B(imagei,imageo,nr,nc,nlpi,nlpo,size)

byte *imagei,*imageo;
int nr,nc,nlpi,nlpo,size;

{
	int	i,j,k,sizesq,min,max,ir,ic;
	int	minus,plus,ii,jj,top,bot,left,right,nexi,nexo;
	byte	*ip,*op,*nnp;

	sizesq = size*size;
	plus = size / 2;
	minus = plus - size + 1;
	top = -minus;
	bot = nr - plus;
	left = -minus;
	right = nc - plus;
	ip = imagei;
	op = imageo;
	nexi = nlpi-nc;
	nexo = nlpo-nc;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			max = -1; min = 1000;
			if (i<top || i>=bot || j<left || j>=right) {
				for (ii=minus;ii<=plus;ii++)
					for (jj=minus;jj<=plus;jj++) {
					    ir = i + ii;
					    ic = j + jj;
					    ir = ir<0?0:(ir>=nr)?nr-1:ir;
					    ic = ic<0?0:(ic>=nc)?nc-1:ic;
					    k = imagei[ir*nlpi+ic];
					    if (k < min) min = k;
					    if (k > max) max = k;
					}
			}
			else {
				nnp = ip + minus*nlpi + minus;
				for (ii=minus;ii<=plus;ii++) {
					for (jj=minus;jj<=plus;jj++) {
						k = *nnp++;
						if (k < min) min = k;
						if (k > max) max = k;
					}
					nnp += nlpi - size;
				}
			}
			k = (abs(*ip-min) < abs(*ip-max)) ? min : max;
			ip++;
			*op++ = k;
		}
		ip += nexi;
		op += nexo;
	}
	return(HIPS_OK);
}
