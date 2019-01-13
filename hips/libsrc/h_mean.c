/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_mean.c - subroutines to compute the mean of an image
 *
 * pixel formats: FLOAT
 *
 * Michael Landy - 8/7/91
 */

#include <hipl_format.h>

int h_mean(hd,mean,nzflag)

struct header *hd;
float *mean;
h_boolean nzflag;

{
	switch(hd->pixel_format) {
	case PFFLOAT:	return(h_mean_f(hd,mean,nzflag));
	default:	return(perr(HE_FMTSUBR,"h_mean",
				hformatname(hd->pixel_format)));
	}
}

int h_mean_f(hd,mean,nzflag)

struct header *hd;
float *mean;
h_boolean nzflag;

{
	float h_mean_F();

	*mean = h_mean_F((float *) hd->firstpix,hd->rows,hd->cols,hd->ocols,
			nzflag);
	return(HIPS_OK);
}

float h_mean_F(image,nr,nc,nlp,nzflag)

float *image;
int nr,nc,nlp;
h_boolean nzflag;

{
	register int i,j,nex,count;
	register float *p,sum;

	nex = nlp-nc;
	p = image;
	sum = 0;
	if (nzflag) {
		count = 0;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				if (*p != 0)
					sum += *p;
				else
					count++;
				p++;
			}
			p += nex;
		}
		if (count == nr*nc)
			return(0.);
		else
			return(sum/(nr*nc - count));
	}
	else {
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++)
				sum += *p++;
			p += nex;
		}
		return(sum/(nr*nc));
	}
}
