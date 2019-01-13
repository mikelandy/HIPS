/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_checkers.c - subroutines to fill an image with a checkerboard
 *
 * pixel formats: BYTE
 *
 * Michael Landy/Yoav Cohen - 5/5/82
 * HIPS 2 - Michael Landy - 7/5/91
 */

#include <hipl_format.h>

int h_checkers(hd,highflag)

struct header *hd;
h_boolean highflag;

{
	switch(hd->pixel_format) {
	case PFBYTE:	return(h_checkers_b(hd,highflag));
	default:	return(perr(HE_FMTSUBR,"h_checkers",
				hformatname(hd->pixel_format)));
	}
}

int h_checkers_b(hd,highflag)

struct header *hd;
h_boolean highflag;

{
	return(h_checkers_B(hd->firstpix,hd->rows,hd->cols,hd->ocols,highflag));
}

int h_checkers_B(image,nr,nc,nlp,highflag)

byte *image;
h_boolean highflag;
int nr,nc,nlp;

{
	register int i,j,nex;
	register byte *p,gl1,gl2,gltmp;

	nex = nlp-nc;
	p = image;
	if (highflag) {
		gl1 = hips_hchar;
		gl2 = hips_lchar;
	}
	else {
		gl2 = hips_hchar;
		gl1 = hips_lchar;
	}
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*p++ = (j & 01) ? gl2 : gl1;
		gltmp = gl1; gl1 = gl2; gl2 = gltmp;
		p += nex;
	}
	return(HIPS_OK);
}
