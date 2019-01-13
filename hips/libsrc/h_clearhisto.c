/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_clearhisto.c - subroutines to zero a histogram
 *
 * Michael Landy - 6/30/91
 */

#include <hipl_format.h>
#include <math.h>

int h_clearhisto(histogram)

struct hips_histo *histogram;

{
	return(h_Clearhisto(histogram->nbins,histogram->histo));
}

int h_Clearhisto(nbins,histo)

int nbins,*histo;

{
	register int nb2,i,*ph;

	ph = histo;
	nb2 = nbins+2;
	for (i=0;i<=nb2;i++)
		*ph++ = 0;
	return(HIPS_OK);
}
