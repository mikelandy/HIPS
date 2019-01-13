/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_rletob.c - subroutines to convert from run-length to byte format
 *
 * pixel formats: RLE
 *
 * Michael Landy - 8/15/94
 */

#include <hipl_format.h>

int h_rletob(hdi,hdo,storelen)

struct header *hdi,*hdo;
int storelen;

{
	return(h_rletoB(hdi->image,hdo->firstpix,hdo->rows,hdo->cols,
		hdo->ocols,storelen));
}

int h_rletoB(imagei,imageo,nr,nc,nlpo,storelen)

byte *imagei,*imageo;
int nr,nc,nlpo,storelen;

{
	register int i,j,k,nexo,runl,val;
	register byte *pi,*po;

	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		j = nc;
		while (j != 0) {
			if (--storelen < 0)
				return(perr(HE_SHORTBUF,"h_rletoB"));
			val = *pi++;
			if (val == 255) {
				if (--storelen < 0)
					return(perr(HE_SHORTBUF,"h_rletoB"));
				runl = *pi++;
				if (runl == 255) {
					*po++ = 255;
					j--;
				}
				else {
					if (--storelen < 0)
						return(perr(HE_SHORTBUF,
							"h_rletoB"));
					val = *pi++;
					runl += (val == 255) ? 2 : 4;
					if (runl > j)
						return(perr(HE_SHORTOBUF,
							"h_rletoB"));
					for (k=runl;k>0;k--)
						*po++ = val;
					j -= runl;
				}
			}
			else {
				*po++ = val;
				j--;
			}
		}
		po += nexo;
	}
	return(HIPS_OK);
}
