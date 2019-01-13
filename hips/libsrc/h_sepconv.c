/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_sepconv.c - 2D separable convolution
 *
 * h_sepconv performs 2D separable convolution by first convolving with a 1D
 * horizontal mask, and then convolving with a 1D vertical mask.
 * (***important note***, h_sepconv cross-correlates with the masks rather than
 * convolves; in other words, the rows and columns are not first reflected;
 * for the usual mirror symmetric masks this poses no complications).  The
 * offsets designates the pixel of the masks which overlays a given input image
 * pixel in order to compute the cross-correlation corresponding to the
 * corresponding output image pixel.  The cross-correlation extends the edges
 * of the subimage whenever the mask extends beyond the subimage edges.
 *
 * Note that the earlier mask values are applied to earlier image
 * values.  Thus, the interpretation of the vertical mask orientation depends
 * on the definition of ULORIG.  If ULORIG is defined (images have their origin
 * at the upper-left), then the first mask element is topmost relative to the
 * image.  Otherwise, the last mask element is topmost relative to the image.
 * Otherwise stated, a ULORIG coordinate system applies to masks as well as to
 * images.
 *
 * input pixel format (integer masks):  INT
 * input pixel format (float masks):  FLOAT
 * output pixel format: same as input
 *
 * Based on HIPS-1 dog: Yoav Cohen - 12/12/82
 * HIPS-2 - msl - 7/16/91
 */

#include <hipl_format.h>

int h_sepconv(hdi,hdt,hdo,maskh,nmaskh,offseth,maskv,nmaskv,offsetv)

struct header *hdi,*hdt,*hdo;
int *maskh,*maskv;	/* if input is PFFLOAT, this is treated as `float *' */
int nmaskh,offseth,nmaskv,offsetv;

{
	switch(hdi->pixel_format) {
	case PFINT:	return(h_sepconv_i(hdi,hdt,hdo,maskh,nmaskh,offseth,
					maskv,nmaskv,offsetv));
	case PFFLOAT:	return(h_sepconv_f(hdi,hdt,hdo,(float *) maskh,nmaskh,
				offseth,(float *) maskv,nmaskv,offsetv));
	default:	return(perr(HE_FMTSUBR,"h_sepconv",
				hformatname(hdi->pixel_format)));
	}
}

int h_sepconv_i(hdi,hdt,hdo,maskh,nmaskh,offseth,maskv,nmaskv,offsetv)

struct header *hdi,*hdt,*hdo;
int *maskh,*maskv;
int nmaskh,offseth,nmaskv,offsetv;

{
	if (h_hconvolve_I((int *) hdi->firstpix,(int *) hdt->firstpix,
	    hdi->rows,hdi->cols,hdi->ocols,hdt->ocols,maskh,nmaskh,offseth) ==
	    HIPS_ERROR)
		return(HIPS_ERROR);
	return(h_vconvolve_I((int *) hdt->firstpix,(int *) hdo->firstpix,
	    hdt->rows,hdt->cols,hdt->ocols,hdo->ocols,maskv,nmaskv,offsetv));
}

int h_sepconv_f(hdi,hdt,hdo,maskh,nmaskh,offseth,maskv,nmaskv,offsetv)

struct header *hdi,*hdt,*hdo;
float *maskh,*maskv;
int nmaskh,offseth,nmaskv,offsetv;

{
	if (h_hconvolve_F((float *) hdi->firstpix,(float *) hdt->firstpix,
	    hdi->rows,hdi->cols,hdi->ocols,hdt->ocols,maskh,nmaskh,offseth) ==
	    HIPS_ERROR)
		return(HIPS_ERROR);
	return(h_vconvolve_F((int *) hdt->firstpix,(float *) hdo->firstpix,
	    hdt->rows,hdt->cols,hdt->ocols,hdo->ocols,maskv,nmaskv,offsetv));
}
