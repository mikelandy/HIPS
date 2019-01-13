/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * read_roi.c - read an image region-of-interest
 *
 * Michael Landy - 6/23/91
 */

#include <stdio.h>
#include <hipl_format.h>

int read_roi(hd,fr)

struct header *hd;
int fr;

{
	return(fread_roi(stdin,hd,fr,"<stdin>"));
}

int fread_roi(fp,hd,fr,fname)

FILE *fp;
struct header *hd;
int fr;
Filename fname;

{
	int linebytes,olinebytes,i;
	byte *p;

	p = hd->firstpix;
	if (hd->pixel_format == PFMSBF || hd->pixel_format == PFLSBF) {
		linebytes = (hd->cols + 7)/8;
		olinebytes = (hd->ocols + 7)/8;
	}
	else {
		linebytes = hd->cols * hd->sizepix;
		olinebytes = hd->ocols * hd->sizepix;
	}
	for (i=0;i<hd->rows;i++) {
		if (fread(p,linebytes,1,fp) != 1)
			return(perr(HE_READFRFILE,fr,fname));
		p += olinebytes;
	}
	return(HIPS_OK);
}
