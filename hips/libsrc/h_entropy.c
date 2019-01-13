/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_entropy.c - subroutines to compute the entropy of an image
 *
 * If pairflag is set, entropy is computed across pairs of pixels
 * (horizontal neighbors).
 *
 * Yoav Cohen - 9/20/82
 * HIPS 2 - Michael Landy - 7/5/91
 */

#include <hipl_format.h>
#include <math.h>

double h_entropy(table,count,pairflag)

int *table,count;
h_boolean pairflag;

{
	register int i,ntable,*pt;
	double entropy,prob,dn;

	if (pairflag)
		ntable = 1<<16;
	else
		ntable = 1<<8;
	entropy=0;
	dn=count;
	pt = table;
	for(i=0;i<ntable;i++) {
		prob = *pt++ / dn;
		if (prob!=0)
			entropy += prob*log(prob);
	}
	entropy /= -log((double) 2.);
	return(entropy);
}
