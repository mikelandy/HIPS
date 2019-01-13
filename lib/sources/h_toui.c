/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_toui.c - conversions to unsigned int pixel format
 *
 * Michael Landy - 1/4/91
 */

#include <hipl_format.h>

int h_toui(hdi,hdo)

struct header *hdi,*hdo;

{
	switch(hdi->pixel_format) {
	case PFINT:	h_itoui(hdi,hdo); break;
	default:	return(perr(HE_FMTSUBR,"h_toui",
				hformatname(hdi->pixel_format)));
	}
	return(HIPS_OK);
}

int h_itoui(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	int *pi;
	h_uint *po;

	hips_lclip = hips_hclip = 0;
	pi = (int *) hdi->image;
	po = (h_uint *) hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++)
		*po++ = *pi++;
	return(HIPS_OK);
}
