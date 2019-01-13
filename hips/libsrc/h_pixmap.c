/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_pixmap.c - subroutines to apply a pixel mapping table
 *
 * pixel formats: BYTE
 *
 * Michael Landy - 8/7/91
 */

#include <hipl_format.h>

int h_pixmap(hdi,hdo,map)

struct header *hdi,*hdo;
byte *map;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	return(h_pixmap_b(hdi,hdo,map));
	default:	return(perr(HE_FMTSUBR,"h_pixmap",
				hformatname(hdi->pixel_format)));
	}
}

int h_pixmap_b(hdi,hdo,map)

struct header *hdi,*hdo;
byte *map;

{
	return(h_pixmap_B(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
		hdi->ocols,hdo->ocols,map));
}

int h_pixmap_B(imagei,imageo,nr,nc,nlpi,nlpo,map)

byte *imagei,*imageo,*map;
int nr,nc,nlpi,nlpo;

{
	register int i,j,nexi,nexo;
	register byte *pi,*po;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*po++ = map[*pi++];
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}
