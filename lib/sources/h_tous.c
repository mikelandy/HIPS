/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_tous.c - conversions to unsigned short pixel format
 *
 * Michael Landy - 1/4/91
 */

#include <hipl_format.h>

int h_tous(hdi,hdo)

struct header *hdi,*hdo;

{
	switch(hdi->pixel_format) {
	case PFINT:	h_itous(hdi,hdo); break;
	default:	return(perr(HE_FMTSUBR,"h_tous",
				hformatname(hdi->pixel_format)));
	}
	return(HIPS_OK);
}

int h_itous(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	int *pi;
	h_ushort *po;

	hips_lclip = hips_hclip = 0;
	pi = (int *) hdi->image;
	po = (h_ushort *) hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++)
		*po++ = *pi++;
	return(HIPS_OK);
}
