/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * alloc_histo.c - HIPS image histogram allocation
 *
 * Michael Landy 6/17/91
 */

#include <hipl_format.h>

int alloc_histo(histo,min,max,nbins,format)

struct hips_histo *histo;
Pixelval *min,*max;
int nbins,format;

{
	int i;

	if (histo->histodealloc && histo->nbins != nbins) {
		free(histo->histo);
		histo->histodealloc = 0;
	}
	if (!(histo->histodealloc))
		if ((histo->histo = (int *) memalloc(nbins+2,sizeof(int))) ==
			(int *) HIPS_ERROR)
				return(HIPS_ERROR);
	histo->nbins = nbins;
	histo->sizehist = (nbins+2)*sizeof(int);
	histo->histodealloc = TRUE;
	histo->pixel_format = format;
	switch (format) {
	case PFBYTE:	histo->minbin.v_byte = min->v_byte;
			i = (1 + max->v_byte - min->v_byte)/nbins;
			if (i >= 256)
				i = 255;
			histo->binwidth.v_byte = (i == 0) ? 1 : i;
			break;
	case PFSBYTE:	histo->minbin.v_sbyte = min->v_sbyte;
			i = (1 + max->v_sbyte - min->v_sbyte)/nbins;
			histo->binwidth.v_sbyte = (i == 0) ? 1 : i;
			break;
	case PFSHORT:	histo->minbin.v_short = min->v_short;
			i = (1 + max->v_short - min->v_short)/nbins;
			histo->binwidth.v_short = (i == 0) ? 1 : i;
			break;
	case PFUSHORT:	histo->minbin.v_ushort = min->v_ushort;
			i = (1 + max->v_ushort - min->v_ushort)/nbins;
			histo->binwidth.v_ushort = (i == 0) ? 1 : i;
			break;
	case PFINT:	histo->minbin.v_int = min->v_int;
			i = (1 + max->v_int - min->v_int)/nbins;
			histo->binwidth.v_int = (i == 0) ? 1 : i;
			break;
	case PFUINT:	histo->minbin.v_uint = min->v_uint;
			i = (1 + max->v_uint - min->v_uint)/nbins;
			histo->binwidth.v_uint = (i == 0) ? 1 : i;
			break;
	case PFFLOAT:
	case PFCOMPLEX:	histo->minbin.v_float = min->v_float;
			histo->binwidth.v_float =
				(max->v_float - min->v_float)/nbins;
			break;
	case PFDOUBLE:
	case PFDBLCOM:	histo->minbin.v_double = min->v_double;
			histo->binwidth.v_double =
				(max->v_double - min->v_double)/nbins;
			break;
	default:	return(perr(HE_FMTSUBR,"alloc_histo",
				hformatname(format)));
	}
	return(HIPS_OK);
}

int alloc_histobins(histo)

struct hips_histo *histo;

{
	if (histo->histodealloc) {
		free(histo->histo);
		histo->histodealloc = 0;
	}
	if (!(histo->histodealloc))
		if ((histo->histo = (int *)
		    memalloc(histo->nbins+2,sizeof(int))) ==
			(int *) HIPS_ERROR)
				return(HIPS_ERROR);
	histo->sizehist = (histo->nbins+2)*sizeof(int);
	histo->histodealloc = TRUE;
	return(HIPS_OK);
}
