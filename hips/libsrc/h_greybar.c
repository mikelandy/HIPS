/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_greybar.c - subroutines to fill an image with grey bars ramped in value
 *
 * pixel formats: BYTE
 *
 * Michael Landy - 8/16/91
 */

#include <hipl_format.h>

int h_greybar(hd,width,low,step)

struct header *hd;
int width;
float low,step;

{
	switch(hd->pixel_format) {
	case PFBYTE:	return(h_greybar_b(hd,width,low,step));
	default:	return(perr(HE_FMTSUBR,"h_greybar",
				hformatname(hd->pixel_format)));
	}
}

int h_greybar_b(hd,width,low,step)

struct header *hd;
int width;
float low,step;

{
	return(h_greybar_B(hd->firstpix,hd->rows,hd->cols,hd->ocols,
		width,low,step));
}

int h_greybar_B(image,nr,nc,nlp,width,low,step)

byte *image;
int nr,nc,nlp,width;
float low,step;

{
	int i,j,k,lim,g,nex,nb;
	register byte *p;

	nex = nlp-nc;
	p = image;
	nb = (nc + width - 1) / width;
	for (i=0;i<nr;i++) {
		for (j=0;j<nb;j++) {
			lim = (nc-j*width) >= width ? width : (nc-j*width);
			g = low + j*step + .5;
			for (k=0;k<lim;k++)
				*p++ = g;
		}
		p += nex;
	}
	return(HIPS_OK);
}
