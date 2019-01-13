/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * alloc_image.c - HIPL image allocation
 *
 * Michael Landy 1/3/91
 */

#include <hipl_format.h>

int alloc_image(hd)

struct header *hd;

{
	int fcb,cb;

	if (hd->sizeimage == 0) {
		hd->imdealloc = FALSE;
		return(HIPS_OK);
	}
	if ((hd->image = hmalloc(hd->sizeimage)) == (byte *) HIPS_ERROR)
		return(HIPS_ERROR);
	if (hd->pixel_format == PFMSBF || hd->pixel_format == PFLSBF) {
		fcb = hd->fcol/8;
		cb = (hd->ocols + 7)/8;
		hd->firstpix = hd->image + ((cb * hd->frow) + fcb);
	}
	else
		hd->firstpix = hd->image +
			((hd->ocols * hd->frow) + hd->fcol) * hd->sizepix;
	hd->imdealloc = TRUE;
	return(HIPS_OK);
}

int alloc_imagez(hd)

struct header *hd;

{
	int fcb,cb;

	if (hd->sizeimage == 0) {
		hd->imdealloc = FALSE;
		return(HIPS_OK);
	}
	if ((hd->image = halloc(hd->sizeimage,1)) == (byte *) HIPS_ERROR)
		return(HIPS_ERROR);
	if (hd->pixel_format == PFMSBF || hd->pixel_format == PFLSBF) {
		fcb = hd->fcol/8;
		cb = (hd->ocols + 7)/8;
		hd->firstpix = hd->image + ((cb * hd->frow) + fcb);
	}
	else
		hd->firstpix = hd->image +
			((hd->ocols * hd->frow) + hd->fcol) * hd->sizepix;
	hd->imdealloc = TRUE;
	return(HIPS_OK);
}

int free_image(hd)

struct header *hd;

{
	if (hd->imdealloc)
		free(hd->image);
	hd->image = (byte *) 0;
	hd->imdealloc = FALSE;
	return(HIPS_OK);
}
