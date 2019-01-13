/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_col3tob.c - conversions from 3-color to byte pixel format
 *
 * Michael Landy - 5/20/93
 */

#include <hipl_format.h>

int h_col3tob(hdi,hdo,fr)

struct header *hdi,*hdo;
int fr;

{
	switch(hdi->pixel_format) {
	case PFRGB:	h_rgbtob_1(hdi,hdo,fr); break;
	case PFRGBZ:	h_rgbztob_1(hdi,hdo,fr); break;
	case PFZRGB:	h_zrgbtob_1(hdi,hdo,fr); break;
	case PFBGR:	h_bgrtob_1(hdi,hdo,fr); break;
	case PFBGRZ:	h_bgrztob_1(hdi,hdo,fr); break;
	case PFZBGR:	h_zbgrtob_1(hdi,hdo,fr); break;
	default:	return(perr(HE_FMTSUBR,"h_col3tob",
				hformatname(hdi->pixel_format)));
	}
	return(HIPS_OK);
}

int h_rgbtob_1(hdi,hdo,fr)

struct header *hdi,*hdo;
int fr;

{
	return(h_3to1_b((hdi->image) + (fr%3),(hdo->image),
		(hdi->orows)*(hdi->ocols)));
}

int h_rgbztob_1(hdi,hdo,fr)

struct header *hdi,*hdo;
int fr;

{
	return(h_4to1_b((hdi->image) + (fr%3),(hdo->image),
		(hdi->orows)*(hdi->ocols)));
}

int h_zrgbtob_1(hdi,hdo,fr)

struct header *hdi,*hdo;
int fr;

{
	return(h_4to1_b((hdi->image) + (fr%3) + 1,(hdo->image),
		(hdi->orows)*(hdi->ocols)));
}

int h_bgrtob_1(hdi,hdo,fr)

struct header *hdi,*hdo;
int fr;

{
	return(h_3to1_b((hdi->image) + 2 - (fr%3),(hdo->image),
		(hdi->orows)*(hdi->ocols)));
}

int h_bgrztob_1(hdi,hdo,fr)

struct header *hdi,*hdo;
int fr;

{
	return(h_4to1_b((hdi->image) + 2 - (fr%3),(hdo->image),
		(hdi->orows)*(hdi->ocols)));
}

int h_zbgrtob_1(hdi,hdo,fr)

struct header *hdi,*hdo;
int fr;

{
	return(h_4to1_b((hdi->image) + 3 - (fr%3),(hdo->image),
		(hdi->orows)*(hdi->ocols)));
}

int h_3to1_b(pi,po,np)

byte *pi,*po;
int np;

{
	int i;

	for (i=0;i<np;i++) {
		*po++ = *pi;
		pi += 3;
	}
	return(HIPS_OK);
}

int h_4to1_b(pi,po,np)

byte *pi,*po;
int np;

{
	int i;

	for (i=0;i<np;i++) {
		*po++ = *pi;
		pi += 4;
	}
	return(HIPS_OK);
}
