/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_affine.c - Warp an image using an affine transformation
 *
 * h_affine warps an image using an affine transformation.
 * For each output pixel, the corresponding input coordinates are computed
 * according to the formulae:
 *
 *	input-x = A*x + B*y + C
 *	input-y = a*x + b*y + c
 *
 * where (x,y) is the output pixel position in a coordinate system where the
 * lower-left corner of the subimage is (0,0), and the upper-right corner is
 * (1,1).  If the computed input position is outside of the input subimage,
 * then the output pixel is set to the background value (hips_lchar).
 * Otherwise, the value is bilinearly interpolated between the surrounding
 * four pixels in the input.
 *
 * pixel formats: BYTE
 *
 * Mike Landy - 8/7/88
 * HIPS 2 - msl - 6/29/91
 */

#include <hipl_format.h>

int h_affine(hdi,hdo,A,B,C,a,b,c)

struct header *hdi,*hdo;
float A,B,C,a,b,c;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	return(h_affine_b(hdi,hdo,A,B,C,a,b,c));
	default:	return(perr(HE_FMTSUBR,"h_affine",
				hformatname(hdi->pixel_format)));
	}
}

int h_affine_b(hdi,hdo,A,B,C,a,b,c)

struct header *hdi,*hdo;
float A,B,C,a,b,c;

{
	return(h_affine_B(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
		hdi->ocols,hdo->rows,hdo->cols,hdo->ocols,A,B,C,a,b,c));
}

int h_affine_B(imagei,imageo,nr,nc,nlpi,nor,noc,nlpo,A,B,C,a,b,c)

byte *imagei,*imageo;
int nr,nc,nlpi,nor,noc,nlpo;
float A,B,C,a,b,c;

{
	register int i,j;
	byte *op;
	int nr1,nc1,ix,iy,ix1,iy1,nor1,noc1,nexo;
	float x,y,dx,dy,px,py,xprime,yprime,vll,vlr,vul,vur,v;

	nexo = nlpo-noc;
	nor1 = nor - 1;
	noc1 = noc - 1;
	nr1 = nr - 1;
	nc1 = nc - 1;
	op = imageo;
	for (i=0;i<nor;i++) {
#ifdef ULORIG
		yprime = 1. - (((float) i)/nor1);
#else
		yprime = ((float) i)/nor1;
#endif
		for (j=0;j<noc;j++) {
			xprime = ((float) j)/noc1;
			x = A*xprime + B*yprime + C;
			y = a*xprime + b*yprime + c;
			if (x < 0 || y < 0 || x > 1 || y > 1) {
				*op++ = hips_lchar;
				continue;
			}
			px = x*nc1;
			ix = px;
			ix1 = ix + 1;
			dx = px - ix;
#ifdef ULORIG
			py = (1.-y)*nr1;
#else
			py = y*nr1;
#endif
			iy = py;
			iy1 = iy + 1;
			dy = py - iy;
			if (px >= nc1)
				ix1 = ix;
			if (py >= nr1)
				iy1 = iy;
			vll = imagei[iy*nlpi + ix];
			vlr = imagei[iy*nlpi + ix1];
			vul = imagei[iy1*nlpi + ix];
			vur = imagei[iy1*nlpi + ix1];
			v = vll*(1-dx)*(1-dy) +
			    vlr*dx*(1-dy) + vul*(1-dx)*dy + vur*dx*dy;
			*op++ = v + 0.5;
		}
		op += nexo;
	}
	return(HIPS_OK);
}
