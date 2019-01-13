/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_col3toi.c - conversions from 3-color to integer pixel format
 *
 * Michael Landy - 5/20/93
 */

#include <hipl_format.h>

int h_3to1_i(),h_4to1_i();

int h_col3toi(hdi,hdo,fr)

struct header *hdi,*hdo;
int fr;

{
	switch(hdi->pixel_format) {
	case PFRGB:	h_rgbtoi_1(hdi,hdo,fr); break;
	case PFRGBZ:	h_rgbztoi_1(hdi,hdo,fr); break;
	case PFZRGB:	h_zrgbtoi_1(hdi,hdo,fr); break;
	case PFBGR:	h_bgrtoi_1(hdi,hdo,fr); break;
	case PFBGRZ:	h_bgrztoi_1(hdi,hdo,fr); break;
	case PFZBGR:	h_zbgrtoi_1(hdi,hdo,fr); break;
	default:	return(perr(HE_FMTSUBR,"h_col3toi",
				hformatname(hdi->pixel_format)));
	}
	return(HIPS_OK);
}

int h_rgbtoi_1(hdi,hdo,fr)

struct header *hdi,*hdo;
int fr;

{
	return(h_3to1_i((hdi->image) + (fr%3),((int *) hdo->image),
		(hdi->orows)*(hdi->ocols)));
}

int h_rgbztoi_1(hdi,hdo,fr)

struct header *hdi,*hdo;
int fr;

{
	return(h_4to1_i((hdi->image) + (fr%3),((int *) hdo->image),
		(hdi->orows)*(hdi->ocols)));
}

int h_zrgbtoi_1(hdi,hdo,fr)

struct header *hdi,*hdo;
int fr;

{
	return(h_4to1_i((hdi->image) + (fr%3) + 1,((int *) hdo->image),
		(hdi->orows)*(hdi->ocols)));
}

int h_bgrtoi_1(hdi,hdo,fr)

struct header *hdi,*hdo;
int fr;

{
	return(h_3to1_i((hdi->image) + 2 - (fr%3),((int *) hdo->image),
		(hdi->orows)*(hdi->ocols)));
}

int h_bgrztoi_1(hdi,hdo,fr)

struct header *hdi,*hdo;
int fr;

{
	return(h_4to1_i((hdi->image) + 2 - (fr%3),((int *) hdo->image),
		(hdi->orows)*(hdi->ocols)));
}

int h_zbgrtoi_1(hdi,hdo,fr)

struct header *hdi,*hdo;
int fr;

{
	return(h_4to1_i((hdi->image) + 3 - (fr%3),((int *) hdo->image),
		(hdi->orows)*(hdi->ocols)));
}

int h_3to1_i(pi,po,np)

byte *pi;
int *po;
int np;

{
	int i;

	for (i=0;i<np;i++) {
		*po++ = *pi;
		pi += 3;
	}
	return(HIPS_OK);
}

int h_4to1_i(pi,po,np)

byte *pi;
int *po;
int np;

{
	int i;

	for (i=0;i<np;i++) {
		*po++ = *pi;
		pi += 4;
	}
	return(HIPS_OK);
}
