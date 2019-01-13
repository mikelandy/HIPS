/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_dither.c - halftones using an 8x8 dither matrix
 *
 * H_dither converts an 8-bit sequence to a bi-level sequence using an 8 x 8
 * dither matrix.  The input and output sequences are both byte-formatted,
 * although only values hips_lchar and hips_hchar are used in the output image.
 *
 * pixel formats: BYTE
 *
 * Mike Landy - 7/13/89
 * HIPS 2 - msl - 8/8/91
 */

#include <hipl_format.h>

static int dithermat[8][8] = {
	  2, 130,  34, 162,  10, 138,  42, 170,
	194,  66, 226,  98, 202,  74, 234, 106,
	 50, 178,  18, 146,  58, 186,  26, 154,
	242, 114, 210,  82, 250, 122, 218,  90,
	 14, 142,  46, 174,   6, 134,  38, 166,
	206,  78, 238, 110, 198,  70, 230, 102,
	 62, 190,  30, 158,  54, 182,  22, 150,
	254, 126, 222,  94, 246, 118, 214,  86 };

int h_dither(hdi,hdo)

struct header *hdi,*hdo;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	return(h_dither_b(hdi,hdo));
	default:	return(perr(HE_FMTSUBR,"h_dither",
				hformatname(hdi->pixel_format)));
	}
}

int h_dither_b(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_dither_B(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
	    hdi->ocols,hdo->ocols));
}

int h_dither_B(imagei,imageo,nr,nc,nlpi,nlpo)

byte *imagei,*imageo;
int nr,nc,nlpi,nlpo;

{
	byte *pi,*po;
	int nexi,nexo,i,j;

	nexi = nlpi - nc;
	nexo = nlpo - nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for(j=0;j<nc;j++){
			if (*pi++ >= dithermat[i%8][j%8])
				*po++ = hips_hchar;
			else
				*po++ = hips_lchar;
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}
