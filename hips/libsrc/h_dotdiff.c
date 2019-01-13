/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_dotdiff.c - subroutines to halftone using an 8x8 dot diffusion matrix
 *
 * H_dotdiff converts an 8-bit sequence to a bi-level
 * sequence using an 8 x 8 dot error diffusion matrix.  Note that the error
 * diffusion is computed based on replacing pixels with 0 or 255, although
 * pixels are actually replaced with hips_lchar and hips_hchar.  Also note
 * that a short-format temporary image is destroyed in the process.
 *
 * pixel formats: BYTE
 *
 * Mike Landy - 7/13/89 (algorithm from Hong Min)
 * HIPS 2 - msl - 8/10/91
 */

#include <hipl_format.h>

int	dotdiff[8][8] = {
	34, 48, 40, 32, 29, 15, 23, 31,
	42, 58, 56, 53, 21,  5,  7, 10,
	50, 62, 61, 45, 13,  1,  2, 18,
	38, 46, 54, 37, 25, 17,  9, 26,
	28, 14, 22, 30, 35, 49, 41, 33,
	20,  4,  6, 11, 43, 59, 57, 52,
	12,  0,  3, 19, 51, 63, 60, 44,
	24, 16,  8, 27, 39, 47, 55, 36 };

int	revmat[64] = {
	49, 21, 22, 50, 41, 13, 42, 14,
	58, 30, 15, 43, 48, 20, 33,  5, 
	57, 29, 23, 51, 40, 12, 34,  6,
	56, 28, 31, 59, 32,  4, 35,  7,
	 3, 39,  0, 36, 63, 27, 24, 60,
	 2, 38,  8, 44, 55, 19, 25, 61,
	 1, 37, 16, 52, 47, 11, 26, 62,
	10, 46,  9, 45, 54, 18, 17, 53 };
int weight();

int h_dotdiff(hdi,hdt,hdo)

struct header *hdi,*hdt,*hdo;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	return(h_dotdiff_b(hdi,hdt,hdo));
	default:	return(perr(HE_FMTSUBR,"h_dotdiff",
				hformatname(hdi->pixel_format)));
	}
}


int h_dotdiff_b(hdi,hdt,hdo)

struct header *hdi,*hdt,*hdo;

{
	return(h_dotdiff_S(hdi->firstpix,(short *) hdt->firstpix,hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdt->ocols,hdo->ocols));
}

int h_dotdiff_S(imagei,imaget,imageo,nr,nc,nlpi,nlpt,nlpo)

byte *imagei,*imageo;
short *imaget;
int nr,nc,nlpi,nlpt,nlpo;

{
	int i,j,offsett,k,err,u,v,w,nexi,next;
	short *pt;
	byte *pi;

	nexi = nlpi - nc;
	next = nlpt - nc;
	pi = imagei;
	pt = imaget;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*pt++ = *pi++;
		pi += nexi;
		pt += next;
	}
	for (k=0; k<64; k++) {
	    for (i=revmat[k]/8; i<nr; i+=8)
		for(j=revmat[k]%8; j<nc; j+=8) {
		    offsett = i*nlpt+j;
		    if (imaget[offsett] < 128) {
			imageo[i*nlpo+j] = hips_lchar;
			err = imaget[offsett];
		    }
		    else {
			imageo[i*nlpo+j] = hips_hchar;
			err = imaget[offsett] - 255;
		    }
		    w = 0;
		    for(u=i-1; u<=i+1; u++)
			for(v=j-1; v<=j+1; v++)
			    if ((u>=0) && (v>=0) && (u<nr) && (v<nc)
				&& (dotdiff[u%8][v%8] > k))
				    w += weight(u-i, v-j);
		    if (w > 0) {
			for(u=i-1; u<=i+1; u++)
			    for(v=j-1; v<=j+1; v++)
				if ((u>=0)&&(v>=0)&&(u<nr)&&(v<nc)
				    &&(dotdiff[u%8][v%8]>k))
					imaget[u*nlpt+v] +=
					    err * weight(u-i, v-j) / w;
		    }
		}
	}
	return(HIPS_OK);
}

int weight(x,y)

register int x,y;

{
	return(3 - x*x - y*y);
}
