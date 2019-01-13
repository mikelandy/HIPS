/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_col3tocol3.c - conversions from 3-color to another 3-color format
 *
 * Michael Landy - 5/20/93
 */

#include <hipl_format.h>

int h_30to3(),h_03to3(),h_3to3_flip(),h_30to3_flip(),h_03to3_flip();
int h_3to30(),h_03to30(),h_3to30_flip(),h_30to30_flip(),h_03to30_flip();
int h_3to03(),h_30to03(),h_3to03_flip(),h_30to03_flip(),h_03to03_flip();

int h_torgb(hdi,hdo)

struct header *hdi,*hdo;

{
	switch(hdi->pixel_format) {
	case PFRGBZ:	h_rgbztorgb(hdi,hdo); break;
	case PFZRGB:	h_zrgbtorgb(hdi,hdo); break;
	case PFBGR:	h_bgrtorgb(hdi,hdo); break;
	case PFBGRZ:	h_bgrztorgb(hdi,hdo); break;
	case PFZBGR:	h_zbgrtorgb(hdi,hdo); break;
	default:	return(perr(HE_FMTSUBR,"h_torgb",
				hformatname(hdi->pixel_format)));
	}
	return(HIPS_OK);
}

int h_torgbz(hdi,hdo)

struct header *hdi,*hdo;

{
	switch(hdi->pixel_format) {
	case PFRGB:	h_rgbtorgbz(hdi,hdo); break;
	case PFZRGB:	h_zrgbtorgbz(hdi,hdo); break;
	case PFBGR:	h_bgrtorgbz(hdi,hdo); break;
	case PFBGRZ:	h_bgrztorgbz(hdi,hdo); break;
	case PFZBGR:	h_zbgrtorgbz(hdi,hdo); break;
	default:	return(perr(HE_FMTSUBR,"h_torgbz",
				hformatname(hdi->pixel_format)));
	}
	return(HIPS_OK);
}

int h_tozrgb(hdi,hdo)

struct header *hdi,*hdo;

{
	switch(hdi->pixel_format) {
	case PFRGB:	h_rgbtozrgb(hdi,hdo); break;
	case PFRGBZ:	h_rgbztozrgb(hdi,hdo); break;
	case PFBGR:	h_bgrtozrgb(hdi,hdo); break;
	case PFBGRZ:	h_bgrztozrgb(hdi,hdo); break;
	case PFZBGR:	h_zbgrtozrgb(hdi,hdo); break;
	default:	return(perr(HE_FMTSUBR,"h_tozrgb",
				hformatname(hdi->pixel_format)));
	}
	return(HIPS_OK);
}

int h_tobgr(hdi,hdo)

struct header *hdi,*hdo;

{
	switch(hdi->pixel_format) {
	case PFRGB:	h_rgbtobgr(hdi,hdo); break;
	case PFRGBZ:	h_rgbztobgr(hdi,hdo); break;
	case PFZRGB:	h_zrgbtobgr(hdi,hdo); break;
	case PFBGRZ:	h_bgrztobgr(hdi,hdo); break;
	case PFZBGR:	h_zbgrtobgr(hdi,hdo); break;
	default:	return(perr(HE_FMTSUBR,"h_tobgr",
				hformatname(hdi->pixel_format)));
	}
	return(HIPS_OK);
}

int h_tobgrz(hdi,hdo)

struct header *hdi,*hdo;

{
	switch(hdi->pixel_format) {
	case PFRGB:	h_rgbtobgrz(hdi,hdo); break;
	case PFRGBZ:	h_rgbztobgrz(hdi,hdo); break;
	case PFZRGB:	h_zrgbtobgrz(hdi,hdo); break;
	case PFBGR:	h_bgrtobgrz(hdi,hdo); break;
	case PFZBGR:	h_zbgrtobgrz(hdi,hdo); break;
	default:	return(perr(HE_FMTSUBR,"h_tobgrz",
				hformatname(hdi->pixel_format)));
	}
	return(HIPS_OK);
}

int h_tozbgr(hdi,hdo)

struct header *hdi,*hdo;

{
	switch(hdi->pixel_format) {
	case PFRGB:	h_rgbtozbgr(hdi,hdo); break;
	case PFRGBZ:	h_rgbztozbgr(hdi,hdo); break;
	case PFZRGB:	h_zrgbtozbgr(hdi,hdo); break;
	case PFBGR:	h_bgrtozbgr(hdi,hdo); break;
	case PFBGRZ:	h_bgrztozbgr(hdi,hdo); break;
	default:	return(perr(HE_FMTSUBR,"h_tozbgr",
				hformatname(hdi->pixel_format)));
	}
	return(HIPS_OK);
}

int h_rgbztorgb(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_30to3(hdi,hdo));
}

int h_zrgbtorgb(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_03to3(hdi,hdo));
}

int h_bgrtorgb(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_3to3_flip(hdi,hdo));
}

int h_bgrztorgb(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_30to3_flip(hdi,hdo));
}

int h_zbgrtorgb(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_03to3_flip(hdi,hdo));
}

int h_rgbtorgbz(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_3to30(hdi,hdo));
}

int h_zrgbtorgbz(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_03to30(hdi,hdo));
}

int h_bgrtorgbz(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_3to30_flip(hdi,hdo));
}

int h_bgrztorgbz(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_30to30_flip(hdi,hdo));
}

int h_zbgrtorgbz(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_03to30_flip(hdi,hdo));
}

int h_rgbtozrgb(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_3to03(hdi,hdo));
}

int h_rgbztozrgb(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_30to03(hdi,hdo));
}

int h_bgrtozrgb(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_3to03_flip(hdi,hdo));
}

int h_bgrztozrgb(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_30to03_flip(hdi,hdo));
}

int h_zbgrtozrgb(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_03to03_flip(hdi,hdo));
}

int h_rgbtobgr(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_3to3_flip(hdi,hdo));
}

int h_rgbztobgr(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_30to3_flip(hdi,hdo));
}

int h_zrgbtobgr(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_03to3_flip(hdi,hdo));
}

int h_bgrztobgr(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_30to3(hdi,hdo));
}

int h_zbgrtobgr(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_03to3(hdi,hdo));
}

int h_rgbtobgrz(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_3to30_flip(hdi,hdo));
}

int h_rgbztobgrz(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_30to30_flip(hdi,hdo));
}

int h_zrgbtobgrz(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_03to30_flip(hdi,hdo));
}

int h_bgrtobgrz(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_3to30(hdi,hdo));
}

int h_zbgrtobgrz(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_03to30(hdi,hdo));
}

int h_rgbtozbgr(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_3to03_flip(hdi,hdo));
}

int h_rgbztozbgr(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_30to03_flip(hdi,hdo));
}

int h_zrgbtozbgr(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_03to03_flip(hdi,hdo));
}

int h_bgrtozbgr(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_3to03(hdi,hdo));
}

int h_bgrztozbgr(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_30to03(hdi,hdo));
}

int h_30to3(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	byte *pi,*po;

	pi = hdi->image;
	po = hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++) {
		*po++ = *pi++;
		*po++ = *pi++;
		*po++ = *pi++;
		pi++;
	}
	return(HIPS_OK);
}

int h_03to3(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	byte *pi,*po;

	pi = hdi->image;
	po = hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++) {
		pi++;
		*po++ = *pi++;
		*po++ = *pi++;
		*po++ = *pi++;
	}
	return(HIPS_OK);
}

int h_3to3_flip(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	byte *pi,*po;

	pi = hdi->image;
	po = hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++) {
		*po++ = pi[2];
		*po++ = pi[1];
		*po++ = pi[0];
		pi += 3;
	}
	return(HIPS_OK);
}

int h_30to3_flip(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	byte *pi,*po;

	pi = hdi->image;
	po = hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++) {
		*po++ = pi[2];
		*po++ = pi[1];
		*po++ = pi[0];
		pi += 4;
	}
	return(HIPS_OK);
}

int h_03to3_flip(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	byte *pi,*po;

	pi = hdi->image + 1;
	po = hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++) {
		*po++ = pi[2];
		*po++ = pi[1];
		*po++ = pi[0];
		pi += 4;
	}
	return(HIPS_OK);
}

int h_3to30(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	byte *pi,*po;

	pi = hdi->image;
	po = hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++) {
		*po++ = *pi++;
		*po++ = *pi++;
		*po++ = *pi++;
		*po++ = 0;
	}
	return(HIPS_OK);
}

int h_03to30(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	byte *pi,*po;

	pi = hdi->image + 1;
	po = hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++) {
		*po++ = *pi++;
		*po++ = *pi++;
		*po++ = *pi++;
		*po++ = 0;
		pi++;
	}
	return(HIPS_OK);
}

int h_3to30_flip(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	byte *pi,*po;

	pi = hdi->image;
	po = hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++) {
		*po++ = pi[2];
		*po++ = pi[1];
		*po++ = pi[0];
		*po++ = 0;
		pi += 3;
	}
	return(HIPS_OK);
}

int h_30to30_flip(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	byte *pi,*po;

	pi = hdi->image;
	po = hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++) {
		*po++ = pi[2];
		*po++ = pi[1];
		*po++ = pi[0];
		*po++ = 0;
		pi += 4;
	}
	return(HIPS_OK);
}

int h_03to30_flip(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	byte *pi,*po;

	pi = hdi->image + 1;
	po = hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++) {
		*po++ = pi[2];
		*po++ = pi[1];
		*po++ = pi[0];
		*po++ = 0;
		pi += 4;
	}
	return(HIPS_OK);
}

int h_3to03(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	byte *pi,*po;

	pi = hdi->image;
	po = hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++) {
		*po++ = 0;
		*po++ = *pi++;
		*po++ = *pi++;
		*po++ = *pi++;
	}
	return(HIPS_OK);
}

int h_30to03(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	byte *pi,*po;

	pi = hdi->image;
	po = hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++) {
		*po++ = 0;
		*po++ = *pi++;
		*po++ = *pi++;
		*po++ = *pi++;
		pi++;
	}
	return(HIPS_OK);
}

int h_3to03_flip(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	byte *pi,*po;

	pi = hdi->image;
	po = hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++) {
		*po++ = 0;
		*po++ = pi[2];
		*po++ = pi[1];
		*po++ = pi[0];
		pi += 3;
	}
	return(HIPS_OK);
}

int h_30to03_flip(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	byte *pi,*po;

	pi = hdi->image;
	po = hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++) {
		*po++ = 0;
		*po++ = pi[2];
		*po++ = pi[1];
		*po++ = pi[0];
		pi += 4;
	}
	return(HIPS_OK);
}

int h_03to03_flip(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,np;
	byte *pi,*po;

	pi = hdi->image + 1;
	po = hdo->image;
	np = hdi->numpix;
	for (i=0;i<np;i++) {
		*po++ = 0;
		*po++ = pi[2];
		*po++ = pi[1];
		*po++ = pi[0];
		pi += 4;
	}
	return(HIPS_OK);
}
