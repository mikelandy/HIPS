/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_maxhisto.c - subroutines to compute the maximum bin of a histogram
 *
 * pixel formats: HISTOGRAM
 *
 * Michael Landy - 7/2/91
 */

#include <hipl_format.h>

int h_maxhisto(histo)

struct hips_histo *histo;

{
	return(h_Maxhisto(histo->histo,histo->nbins));
}

int h_Maxhisto(histo,nbins)

int *histo,nbins;

{
	int nb2,max,*hp,i;

	nb2 = nbins+2;
	hp = histo;
	max = *hp++;
	for (i=1;i<nb2;i++) {
		if (*hp > max)
			max = *hp;
		hp++;
	}
	return(max);
}
