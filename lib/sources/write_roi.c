/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * write_roi.c - write an image region-of-interest
 *
 * Michael Landy - 1/16/91
 */

#include <stdio.h>
#include <hipl_format.h>

int write_roi(hd,fr)

struct header *hd;
int fr;

{
	return(fwrite_roi(stdout,hd,fr,"<stdout>"));
}

int fwrite_roi(fp,hd,fr,fname)

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
		if (fwrite(p,linebytes,1,fp) != 1)
			return(perr(HE_WRITEFRFILE,fr,fname));
		p += olinebytes;
	}
	return(HIPS_OK);
}
