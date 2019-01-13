/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_zoneplate.c - subroutines to fill an image with a zoneplate image
 *
 * pixel formats: FLOAT
 *
 * Michael Landy - 4/8/89
 * HIPS 2 - Michael Landy - 7/5/91
 */

#include <hipl_format.h>
#include <math.h>

int h_zoneplate(hd,freq,sinphase)

struct header *hd;
double freq;
h_boolean sinphase;

{
	switch(hd->pixel_format) {
	case PFFLOAT:	return(h_zoneplate_f(hd,freq,sinphase));
	default:	return(perr(HE_FMTSUBR,"h_zoneplate",
				hformatname(hd->pixel_format)));
	}
}

int h_zoneplate_f(hd,freq,sinphase)

struct header *hd;
double freq;
h_boolean sinphase;

{
	return(h_zoneplate_F((float *) hd->firstpix,hd->rows,hd->cols,
		hd->ocols,freq,sinphase));
}

int h_zoneplate_F(image,nr,nc,nlp,freq,sinphase)

float *image;
int nr,nc,nlp;
double freq;
h_boolean sinphase;

{
	register int i,j,nex;
	register float *p;
	float nr2,nc2,y,y2,fact,x;

	nr2 = nr/2;
	nc2 = nc/2;
	fact = 3.1415926 * freq / 2.;
	nex = nlp-nc;
	p = image;
	for (i=0;i<nr;i++) {
		y = (i/nr2) - 1.;
		y2 = y*y;
		for (j=0;j<nc;j++) {
			x = (j/nc2) - 1.;
			if (sinphase)
				*p++ = sin((double) fact*(y2 + x*x));
			else
				*p++ = cos((double) fact*(y2 + x*x));
		}
		p += nex;
	}
	return(HIPS_OK);
}
