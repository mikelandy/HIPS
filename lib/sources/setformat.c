/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * setformat.c - set image pixel format
 *
 * Michael Landy - 1/3/91
 */

#include <hipl_format.h>

int setformat(hd,pfmt)

struct header *hd;
int pfmt;

{
	hd->pixel_format = pfmt;
	if (pfmt == PFINTPYR || pfmt == PFFLOATPYR)
		return(perr(HE_SETFP));
	hd->sizepix = hsizepix(pfmt);
	hd->sizeimage = hd->numpix * hd->sizepix;
	if (pfmt == PFMSBF || pfmt == PFLSBF)
		hd->sizeimage = hd->orows * ((hd->ocols+7) / 8) * sizeof(byte);
	if ((pfmt == PFMSBF || pfmt == PFLSBF) && (hd->fcol % 8 != 0)) {
		clearroi(hd);
		return(perr(HE_ROI8C,"setformat"));
	}
	return(HIPS_OK);
}

int setpyrformat(hd,pfmt,toplev)

struct header *hd;
int pfmt,toplev;

{
	hd->pixel_format = pfmt;
	if (setparam(hd,"toplev",PFINT,1,toplev) == HIPS_ERROR)
		return(HIPS_ERROR);
	if (pfmt != PFINTPYR && pfmt != PFFLOATPYR)
		return(perr(HE_SETPF));
	hd->sizepix = hsizepix(pfmt);
	if ((hd->numpix = pyrnumpix(toplev,hd->rows,hd->cols)) == HIPS_ERROR)
		return(HIPS_ERROR);
	hd->sizeimage = hd->numpix * hd->sizepix;
	return(HIPS_OK);
}
