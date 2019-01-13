/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_btorle.c - subroutines to convert from byte to run-length format
 *
 * pixel formats: byte
 *
 * Michael Landy - 8/15/94
 */

#include <hipl_format.h>

int h_btorle(hdi,hdo,bufsize,storelen)

struct header *hdi,*hdo;
int bufsize,*storelen;

{
	return(h_Btorle(hdi->firstpix,hdo->image,hdi->rows,hdi->cols,
		hdi->ocols,bufsize,storelen));
}

int h_Btorle(imagei,imageo,nr,nc,nlpi,bufsize,storelen)

byte *imagei,*imageo;
int nr,nc,nlpi,bufsize,*storelen;

{
	register int i,j,nexi,runl,val,len,maxrun;
	register byte *pi,*pi2,*po;

	nexi = nlpi-nc;
	pi = imagei;
	po = imageo;
	len = 0;
	for (i=0;i<nr;i++) {
		j = nc;
		while (j != 0) {
			val = *pi;
			runl = 1;
			maxrun = (val == 255) ? 256 : 258;
			if (maxrun > j)
				maxrun = j;
			pi2 = pi;
			while (--maxrun > 0) {
				if (*++pi2 != val)
					break;
				runl++;
			}
			if (val == 255) {
				if (runl > 1) {
					if (len+3 > bufsize)
						return(perr(HE_BUF,"h_Btorle"));
					*po++ = 255;
					*po++ = runl - 2;
					*po++ = 255;
					len += 3;
					pi += runl;
					j -= runl;
				}
				else {
					if (len+2 > bufsize)
						return(perr(HE_BUF,"h_Btorle"));
					*po++ = 255;
					*po++ = 255;
					len += 2;
					pi++;
					j--;
				}
			}
			else {
				if (runl > 3) {
					if (len+3 > bufsize)
						return(perr(HE_BUF,"h_Btorle"));
					*po++ = 255;
					*po++ = runl - 4;
					*po++ = val;
					len += 3;
					pi += runl;
					j -= runl;
				}
				else {
					if (len+1 > bufsize)
						return(perr(HE_BUF,"h_Btorle"));
					*po++ = val;
					len += 1;
					pi++;
					j--;
				}
			}
		}
		pi += nexi;
	}
	*storelen = len;
	return(HIPS_OK);
}
