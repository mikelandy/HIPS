/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * setroi.c - set image region-of-interest
 *
 * Michael Landy - 1/3/91
 */

#include <hipl_format.h>

int setroi(hd,fr,fc,nr,nc)

struct header *hd;
int fr,fc,nr,nc;

{
	int fcb,cb;

	if (fr<0 || fc<0 || (fr+nr)>hd->orows || (fc+nc)>hd->ocols)
		return(perr(HE_ROI,fr,fc,nr,nc));
	if ((hd->pixel_format == PFMSBF || hd->pixel_format == PFLSBF)
	    && (fc % 8 != 0))
		return(perr(HE_ROI8,"setroi"));
	hd->rows = nr;
	hd->cols = nc;
	hd->frow = fr;
	hd->fcol = fc;
	if (hd->pixel_format == PFMSBF || hd->pixel_format == PFLSBF) {
		fcb = fc/8;
		cb = (hd->ocols + 7)/8;
		hd->firstpix = hd->image + ((cb * fr) + fcb);
	}
	else
		hd->firstpix = hd->image +
			((hd->ocols * fr) + fc) * hd->sizepix;
	return(HIPS_OK);
}

int setroi2(hd,roi)

struct header *hd;
struct hips_roi *roi;

{
	return(setroi(hd,roi->frow,roi->fcol,roi->rows,roi->cols));
}

int getroi(hd,roi)

struct header *hd;
struct hips_roi *roi;

{
	roi->frow = hd->frow;
	roi->fcol = hd->fcol;
	roi->rows = hd->rows;
	roi->cols = hd->cols;
	return(HIPS_OK);
}

int clearroi(hd)

struct header *hd;

{
	hd->rows = hd->orows;
	hd->cols = hd->ocols;
	hd->frow = hd->fcol = 0;
	hd->firstpix = hd->image;
	return(HIPS_OK);
}
