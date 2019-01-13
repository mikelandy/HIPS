/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_col1tocol3.c - conversions from single pixel to 3-color format
 *
 * Michael Landy - 5/20/93
 */

#include <hipl_format.h>

int h_col1torgb(hdi,hdo,fr)

struct header *hdi,*hdo;
int fr;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	h_btorgb_1(hdi,hdo,fr); break;
	case PFINT:	h_itorgb_1(hdi,hdo,fr); break;
	default:	return(perr(HE_FMTSUBR,"h_col1torgb",
				hformatname(hdi->pixel_format)));
	}
	return(HIPS_OK);
}

int h_col1torgbz(hdi,hdo,fr)

struct header *hdi,*hdo;
int fr;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	h_btorgbz_1(hdi,hdo,fr); break;
	case PFINT:	h_itorgbz_1(hdi,hdo,fr); break;
	default:	return(perr(HE_FMTSUBR,"h_col1torgbz",
				hformatname(hdi->pixel_format)));
	}
	return(HIPS_OK);
}

int h_col1tozrgb(hdi,hdo,fr)

struct header *hdi,*hdo;
int fr;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	h_btozrgb_1(hdi,hdo,fr); break;
	case PFINT:	h_itozrgb_1(hdi,hdo,fr); break;
	default:	return(perr(HE_FMTSUBR,"h_col1tozrgb",
				hformatname(hdi->pixel_format)));
	}
	return(HIPS_OK);
}

int h_col1tobgr(hdi,hdo,fr)

struct header *hdi,*hdo;
int fr;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	h_btobgr_1(hdi,hdo,fr); break;
	case PFINT:	h_itobgr_1(hdi,hdo,fr); break;
	default:	return(perr(HE_FMTSUBR,"h_col1tobgr",
				hformatname(hdi->pixel_format)));
	}
	return(HIPS_OK);
}

int h_col1tobgrz(hdi,hdo,fr)

struct header *hdi,*hdo;
int fr;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	h_btobgrz_1(hdi,hdo,fr); break;
	case PFINT:	h_itobgrz_1(hdi,hdo,fr); break;
	default:	return(perr(HE_FMTSUBR,"h_col1tobgrz",
				hformatname(hdi->pixel_format)));
	}
	return(HIPS_OK);
}

int h_col1tozbgr(hdi,hdo,fr)

struct header *hdi,*hdo;
int fr;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	h_btozbgr_1(hdi,hdo,fr); break;
	case PFINT:	h_itozbgr_1(hdi,hdo,fr); break;
	default:	return(perr(HE_FMTSUBR,"h_col1tozbgr",
				hformatname(hdi->pixel_format)));
	}
	return(HIPS_OK);
}

int h_btorgb_1(hdi,hdo,fr)

struct header *hdi,*hdo;
int fr;

{
	return(h_1to3_b(hdi->image,(hdo->image) + (fr%3),
		(hdi->orows)*(hdi->ocols)));
}

int h_itorgb_1(hdi,hdo,fr)

struct header *hdi,*hdo;
int fr;

{
	return(h_1to3_i((int *) hdi->image,(hdo->image) + (fr%3),
		(hdi->orows)*(hdi->ocols)));
}

int h_btorgbz_1(hdi,hdo,fr)

struct header *hdi,*hdo;
int fr;

{
	return(h_1to4_b(hdi->image,(hdo->image) + (fr%3),
		(hdi->orows)*(hdi->ocols)));
}

int h_itorgbz_1(hdi,hdo,fr)

struct header *hdi,*hdo;
int fr;

{
	return(h_1to4_i((int *) hdi->image,(hdo->image) + (fr%3),
		(hdi->orows)*(hdi->ocols)));
}

int h_btozrgb_1(hdi,hdo,fr)

struct header *hdi,*hdo;
int fr;

{
	return(h_1to4_b(hdi->image,(hdo->image) + (fr%3) + 1,
		(hdi->orows)*(hdi->ocols)));
}

int h_itozrgb_1(hdi,hdo,fr)

struct header *hdi,*hdo;
int fr;

{
	return(h_1to4_i((int *) hdi->image,(hdo->image) + (fr%3) + 1,
		(hdi->orows)*(hdi->ocols)));
}

int h_btobgr_1(hdi,hdo,fr)

struct header *hdi,*hdo;
int fr;

{
	return(h_1to3_b(hdi->image,(hdo->image) + 2 - (fr%3),
		(hdi->orows)*(hdi->ocols)));
}

int h_itobgr_1(hdi,hdo,fr)

struct header *hdi,*hdo;
int fr;

{
	return(h_1to3_i((int *) hdi->image,(hdo->image) + 2 - (fr%3),
		(hdi->orows)*(hdi->ocols)));
}

int h_btobgrz_1(hdi,hdo,fr)

struct header *hdi,*hdo;
int fr;

{
	return(h_1to4_b(hdi->image,(hdo->image) + 2 - (fr%3),
		(hdi->orows)*(hdi->ocols)));
}

int h_itobgrz_1(hdi,hdo,fr)

struct header *hdi,*hdo;
int fr;

{
	return(h_1to4_i((int *) hdi->image,(hdo->image) + 2 - (fr%3),
		(hdi->orows)*(hdi->ocols)));
}

int h_btozbgr_1(hdi,hdo,fr)

struct header *hdi,*hdo;
int fr;

{
	return(h_1to4_b(hdi->image,(hdo->image) + 3 - (fr%3),
		(hdi->orows)*(hdi->ocols)));
}

int h_itozbgr_1(hdi,hdo,fr)

struct header *hdi,*hdo;
int fr;

{
	return(h_1to4_i((int *) hdi->image,(hdo->image) + 3 - (fr%3),
		(hdi->orows)*(hdi->ocols)));
}

int h_1to3_b(pi,po,np)

byte *pi,*po;
int np;

{
	int i;

	for (i=0;i<np;i++) {
		*po = *pi++;
		po += 3;
	}
	return(HIPS_OK);
}

int h_1to3_i(pi,po,np)

int *pi;
byte *po;
int np;

{
	int i;

	hips_lclip = hips_hclip = 0;
	for (i=0;i<np;i++) {
		if (*pi < 0) {
			hips_lclip++;
			*po = 0;
			pi++;
		}
		else if (*pi > 255) {
			hips_hclip++;
			*po = 255;
			pi++;
		}
		else
			*po = *pi++;
		po += 3;
	}
	return(HIPS_OK);
}

int h_1to4_b(pi,po,np)

byte *pi,*po;
int np;

{
	int i;

	for (i=0;i<np;i++) {
		*po = *pi++;
		po += 4;
	}
	return(HIPS_OK);
}

int h_1to4_i(pi,po,np)

int *pi;
byte *po;
int np;

{
	int i;

	hips_lclip = hips_hclip = 0;
	for (i=0;i<np;i++) {
		if (*pi < 0) {
			hips_lclip++;
			*po = 0;
			pi++;
		}
		else if (*pi > 255) {
			hips_hclip++;
			*po = 255;
			pi++;
		}
		else
			*po = *pi++;
		po += 4;
	}
	return(HIPS_OK);
}
