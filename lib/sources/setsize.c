/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * setsize.c - set image size
 *
 * Michael Landy - 1/3/91
 */

#include <hipl_format.h>

int setsize(hd,r,c)

struct header *hd;
int r,c;

{
	int toplev,one=1;

	hd->orows = hd->rows = r;
	hd->ocols = hd->cols = c;
	hd->frow = hd->fcol = 0;
	hd->numpix = r*c;
	if (hd->pixel_format == PFINTPYR || hd->pixel_format == PFFLOATPYR) {
		if (getparam(hd,"toplev",PFINT,&one,&toplev) == HIPS_ERROR)
			return(HIPS_ERROR);
		hd->numpix = pyrnumpix(toplev,hd->rows,hd->cols);
	}
	hd->sizeimage = hd->numpix * hd->sizepix;
	if (hd->pixel_format == PFMSBF || hd->pixel_format == PFLSBF)
		hd->sizeimage = hd->orows * ((hd->ocols+7) / 8) * sizeof(byte);
	return(HIPS_OK);
}
