/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_rgb.c - conversions to/from RGB formats
 *
 * Michael Landy - 8/14/91
 */

#include <hipl_format.h>

int h_btorgb(hdr,hdg,hdb,hdo)

struct header *hdr,*hdg,*hdb,*hdo;

{
	int i,np;
	byte *pr,*pg,*pb,*po;

	pr = hdr->image;
	pg = hdg->image;
	pb = hdb->image;
	po = hdo->image;
	np = hdr->numpix;
	for (i=0;i<np;i++) {
		*po++ = *pr++;
		*po++ = *pg++;
		*po++ = *pb++;
	}
	return(HIPS_OK);
}

int h_btorgbz(hdr,hdg,hdb,hdo)

struct header *hdr,*hdg,*hdb,*hdo;

{
	int i,np;
	byte *pr,*pg,*pb,*po;

	pr = hdr->image;
	pg = hdg->image;
	pb = hdb->image;
	po = hdo->image;
	np = hdr->numpix;
	for (i=0;i<np;i++) {
		*po++ = *pr++;
		*po++ = *pg++;
		*po++ = *pb++;
		*po++ = 0;
	}
	return(HIPS_OK);
}

int h_rgbtob(hdi,hdr,hdg,hdb)

struct header *hdi,*hdr,*hdg,*hdb;

{
	int i,np;
	byte *pi,*pr,*pg,*pb;

	pi = hdi->image;
	pr = hdr->image;
	pg = hdg->image;
	pb = hdb->image;
	np = hdr->numpix;
	for (i=0;i<np;i++) {
		*pr++ = *pi++;
		*pg++ = *pi++;
		*pb++ = *pi++;
	}
	return(HIPS_OK);
}

int h_rgbztob(hdi,hdr,hdg,hdb)

struct header *hdi,*hdr,*hdg,*hdb;

{
	int i,np;
	byte *pi,*pr,*pg,*pb;

	pi = hdi->image;
	pr = hdr->image;
	pg = hdg->image;
	pb = hdb->image;
	np = hdr->numpix;
	for (i=0;i<np;i++) {
		*pr++ = *pi++;
		*pg++ = *pi++;
		*pb++ = *pi++;
		pi++;
	}
	return(HIPS_OK);
}

int h_rgbtob2(hdi,hdo,color)

struct header *hdi,*hdo;
char *color;

{
	int i,np;
	byte *pi,*po;

	po = hdo->image;
	np = hdi->numpix;
	if (strcmp(color,"r") == 0) {
		pi = hdi->image;
		for (i=0;i<np;i++) {
			*po++ = *pi;
			*pi += 3;
		}
	}
	else if (strcmp(color,"g") == 0) {
		pi = hdi->image + 1;
		for (i=0;i<np;i++) {
			*po++ = *pi;
			*pi += 3;
		}
	}
	else if (strcmp(color,"b") == 0) {
		pi = hdi->image + 2;
		for (i=0;i<np;i++) {
			*po++ = *pi;
			*pi += 3;
		}
	}
	else
		return(perr(HE_COLSPEC,"h_rgbtob2",color));
	return(HIPS_OK);
}

int h_rgbztob2(hdi,hdo,color)

struct header *hdi,*hdo;
char *color;

{
	int i,np;
	byte *pi,*po;

	po = hdo->image;
	np = hdi->numpix;
	if (strcmp(color,"r") == 0) {
		pi = hdi->image;
		for (i=0;i<np;i++) {
			*po++ = *pi;
			*pi += 4;
		}
	}
	else if (strcmp(color,"g") == 0) {
		pi = hdi->image + 1;
		for (i=0;i<np;i++) {
			*po++ = *pi;
			*pi += 4;
		}
	}
	else if (strcmp(color,"b") == 0) {
		pi = hdi->image + 2;
		for (i=0;i<np;i++) {
			*po++ = *pi;
			*pi += 4;
		}
	}
	else
		return(perr(HE_COLSPEC,"h_rgbtob2",color));
	return(HIPS_OK);
}
