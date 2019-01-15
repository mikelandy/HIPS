
/* addmask.c                                 Brian Tierney, LBL   4/90
 *
 *   usage:   addmask [-n] mask_image < image > new_image
 *
 *   creates new image which is the input image where the mask
 *    value is > 0, and zero everywhere else.
 *
 *  Works with data types: Byte, short, int, float, and complex.
 *
 *  converted to HIPS2:  Felix Huang
 */

/*   This program is copyright (C) 1990, Regents  of  the
University  of  California.   Anyone may reproduce this software,
in whole or in part, provided that:
(1)  Any copy  or  redistribution  must  show  the
     Regents  of  the  University of California, through its
     Lawrence Berkeley Laboratory, as the source,  and  must
     include this notice;
(2)  Any use of this software must reference this  distribu-
     tion,  state that the software copyright is held by the
     Regents of the University of California, and  that  the
     software is used by their permission.

     It is acknowledged that the U.S. Government has  rights
to this software under  Contract DE-AC03-765F00098 between the U.S.
Department of Energy and the University of California.

     This software is provided as a professional  academic  contribu-
tion  for  joint exchange.  Thus it is experimental, is pro-
vided ``as is'', with no warranties of any kind  whatsoever,
no  support,  promise  of updates, or printed documentation.
Bug reports or fixes may be sent to the author, who may or may
not act on them as he desires.
*/

/*   Author:  Brian L. Tierney
 *            Lawrence Berkeley Laboratory
 *            Imaging Technologies Group
 *            email: bltierney@lbl.gov
*/

#include <stdio.h>
#include <sys/types.h>
#include <math.h>

#include <hipl_format.h>

extern h_boolean neg;

extern struct header m_hd2;	/* mask image header             */

int       m_ocol;

int       nrow;
int       ncol;
int       i_ocol;

byte     *bmask;		/* binary mask	 */

h_addmask(hdi, hdo)
    struct header *hdi, *hdo;
{
    m_ocol = m_hd2.ocols;	/* mask column difference  */

    nrow = hdi->rows;
    ncol = hdi->cols;
    i_ocol = hdi->ocols;

    bmask = m_hd2.image + hdi->frow * m_hd2.ocols + hdi->fcol;

    switch (hdi->pixel_format) {
    case PFBYTE:
	return h_addm_b(hdi->firstpix, hdo->firstpix);
    case PFSHORT:
	return h_addm_s((short *) hdi->firstpix,
			(short *) hdo->firstpix);
    case PFINT:
	return h_addm_i((int *) hdi->firstpix,
			(int *) hdo->firstpix);
    case PFFLOAT:
	return h_addm_f((float *) hdi->firstpix,
			(float *) hdo->firstpix);
    case PFCOMPLEX:
	return h_addm_c((float *) hdi->firstpix,
			(float *) hdo->firstpix);
    }

}				/* end of h_addmask (hdi, hdo)	 */

h_addm_b(imagei, imageo)
    byte     *imagei;
    byte     *imageo;
{
    int       x, y;

    for (y = 0; y < nrow; y++, imagei = imagei + i_ocol,
	 imageo = imageo + i_ocol,
	 bmask = bmask + m_ocol) {
	for (x = 0; x < ncol; x++) {
	    if ((neg == FALSE && bmask[x] != 0) ||
		(neg == TRUE && bmask[x] == 0))
		imageo[x] = imagei[x];
	    else
		imageo[x] = 0;
	}
    }

}				/* end of  h_addm_b ( imagei, imageo )	 */


h_addm_s(imagei, imageo)
    short    *imagei;
    short    *imageo;
{
    int       x, y;

    for (y = 0; y < nrow; y++, imagei = imagei + i_ocol,
	 imageo = imageo + i_ocol,
	 bmask = bmask + m_ocol) {
	for (x = 0; x < ncol; x++) {
	    if ((neg == FALSE && bmask[x] != 0) ||
		(neg == TRUE && bmask[x] == 0))
		imageo[x] = imagei[x];
	    else
		imageo[x] = 0;
	}
    }

}				/* end of  h_addm_s ( imagei, imageo )	 */


h_addm_i(imagei, imageo)
    int      *imagei;
    int      *imageo;
{
    int       x, y;

    for (y = 0; y < nrow; y++, imagei = imagei + i_ocol,
	 imageo = imageo + i_ocol,
	 bmask = bmask + m_ocol) {
	for (x = 0; x < ncol; x++) {
	    if ((neg == FALSE && bmask[x] != 0) ||
		(neg == TRUE && bmask[x] == 0))
		imageo[x] = imagei[x];
	    else
		imageo[x] = 0;
	}
    }

}				/* end of  h_addm_i ( imagei, imageo )	 */


h_addm_f(imagei, imageo)
    float    *imagei;
    float    *imageo;
{
    int       x, y;

    for (y = 0; y < nrow; y++, imagei = imagei + i_ocol,
	 imageo = imageo + i_ocol,
	 bmask = bmask + m_ocol) {
	for (x = 0; x < ncol; x++) {
	    if ((neg == FALSE && bmask[x] != 0) ||
		(neg == TRUE && bmask[x] == 0))
		imageo[x] = imagei[x];
	    else
		imageo[x] = 0;
	}
    }

}				/* end of  h_addm_f ( imagei, imageo )	 */


h_addm_c(imagei, imageo)
    float    *imagei;
    float    *imageo;
{
    int       x, y;
    int       j;

    for (y = 0; y < nrow; y++, imagei = imagei + 2 * i_ocol,
	 imageo = imageo + 2 * i_ocol,
	 bmask = bmask + m_ocol) {
	for (x = 0; x < ncol; x++) {
	    j = 2 * x;
	    if ((neg == FALSE && bmask[x] != 0) ||
		(neg == TRUE && bmask[x] == 0)) {
		imageo[j] = imagei[j];
		imageo[j + 1] = imagei[j + 1];
	    } else {
		imageo[j] = 0;
		imageo[j + 1] = 0;
	    }
	}
    }

}				/* end of  h_addm_c ( imagei, imageo )	 */
