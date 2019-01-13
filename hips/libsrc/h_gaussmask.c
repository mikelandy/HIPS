/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_gaussmask.c - subroutine to compute a 1D Gaussian mask
 *
 * gauss_mask creates a one-dimensional gaussian mask of standard deviation
 * "sigma" and stores the result in a mask consisting of "nmask" pixels. 
 * The value of the mask are scaled so that the mask pixels sum to one.  The
 * function returns the scale factor required to do that.  The "precision"
 * argument (a positive integer, set to one if not positive), determines on
 * how many intervals each entry of the mask is computed.  If precision equals
 * one, each mask entry is computed by considering only the midpoint of the
 * interval that mask entry represents.  Otherwise, "precision" equally spaced
 * points are computed and averaged to derive each array entry.
 *
 * Yoav Cohen, 12/10/82
 * HIPS 2 - Michael Landy - 7/16/91
 */

#include <hipl_format.h>
#include <math.h>

double h_gaussmask(sigma,nmask,maskarr,precision)

double sigma;
float  *maskarr;
int nmask,precision;

{
	int i,j,i2,n2;
	double const1,tssq,x,sum,scale;
	double xx,estimate,deltax,xshift;
	h_boolean odd;

	if (sigma == 0.0)
		return((double) perr(HE_SIGMA,"h_gaussmask"));
	if (precision<1)
		precision = 1;
	n2 = nmask/2;
	odd =  (nmask%2 == 1);
	const1 = 1./(sigma*sqrt(2.0*H_PI));
	tssq = 1./(2*sigma*sigma);
	deltax = 1.0/precision;
	xshift = deltax/2. - 1/2.;

	for(i=n2,i2=odd?n2:n2-1;i<nmask;i++,i2--) {
		x = i-n2;
		if (!odd)
			x += .5;
		xx = x+xshift;
		estimate = 0;
		for (j=0;j<precision;j++) {
			estimate += const1*exp(-xx*xx*tssq);
			xx += deltax;
		}
		estimate /= precision;
		maskarr[i] = maskarr[i2] = estimate;
	}
	sum = 0.0;
	for(i=0;i<nmask;i++)
		sum += maskarr[i];
	scale = 1./sum;
	for(i=0;i<nmask;i++)
		maskarr[i] *= scale;
	return(scale);
}
