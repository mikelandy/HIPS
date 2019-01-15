

/*   bthin.c                            Brian Tierney,  LBL  3/90
 *
 *  program to this binary object to a single pixel width line
 *
 *   Author:  Brian L. Tierney
 *            Lawrence Berkeley Laboratory
 *            Imaging and Distributed Computing Group
 *            email: bltierney@lbl.gov
 *
 *  converted to HIPS2
 *            Felix K. Huang
 *            Lawrence Berkeley Laboratory
 *            Imaging and Distributed Computing Group
 *            email: fhuang@george.lbl.gov
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

#define Calloc(x,y) (y *)calloc((unsigned)(x), sizeof(y))

#include <stdio.h>
#include <hipl_format.h>

extern int pval;	/* new pixel value for thinned output objects       */

extern int nrow;
extern int ncol;
extern int i_ocol;

byte    **roi1, **roi2;
void i_to_r(),r_to_o(),thin_binary_image();

int h_bthin(hdi, hdo)
    struct header *hdi, *hdo;
{
    if (hdi->pixel_format != PFBYTE)
    	return(perr(HE_FMTSUBR,"h_thin",hformatname(hdi->pixel_format)));
    if (hdo->pixel_format != PFBYTE)
    	return(perr(HE_FMTSUBR,"h_thin",hformatname(hdo->pixel_format)));
    i_to_r(hdi->firstpix);
    thin_binary_image();
    r_to_o(hdo->firstpix);
    return(HIPS_OK);
}


/********************************************************************/
void i_to_r(imagei)			/* input image to roi1	 */
    byte     *imagei;
{
    int       x, y;

    roi1 = Calloc(nrow, byte *);
    for (y = 0; y < nrow; y++)
	roi1[y] = Calloc(ncol, byte);

    roi2 = Calloc(nrow, byte *);
    for (y = 0; y < nrow; y++)
	roi2[y] = Calloc(ncol, byte);

    for (y = 0; y < nrow; y++, imagei = imagei + i_ocol)
	for (x = 0; x < ncol; x++)
	    roi1[y][x] = imagei[x];
}

void r_to_o(imageo)			/* roi2 to output image	 */
    byte     *imageo;
{
    int       x, y;

    for (y = 0; y < nrow; y++, imageo = imageo + i_ocol)
	for (x = 0; x < ncol; x++)
	    imageo[x] = roi2[y][x];
}


/********************************************************************/
void thin_binary_image()
{
    register int i, j, i1, j1, i2, j2, ni, nj, di, dj;

    for (i = 0; i < nrow; i++) {
	for (j = 0; j < ncol; j++) {

	    if (roi1[i][j] > 0) {	/* pixel is part of an object */
		i1 = i2 = i;
		j1 = j2 = j;

		/* find 4 points for edge of the circle  */

		while (i1 < nrow && roi1[i1][j] > 0)
		    i1++;

		while (i2 > 0 && roi1[i2][j] > 0)
		    i2--;

		while (j1 < ncol && roi1[i][j1] > 0)
		    j1++;

		while (j2 > 0 && roi1[i][j2] > 0)
		    j2--;


		di = i1 - i2;
		dj = j1 - j2;

		/* calc new center based on smaller diameter */
		if (di <= 1 || dj <= 1) {
		    ni = i;
		    nj = j;
		    roi2[ni][nj] = pval;
		} else if (di > dj) {
		    ni = i;
		    nj = (j1 + j2) / 2;
		    roi2[ni][nj] = pval;
		} else if (di < dj) {
		    ni = (i1 + i2) / 2;
		    nj = j;
		    roi2[ni][nj] = pval;
		} else {	/* d1 == dj, do both */
		    ni = i;
		    nj = (j1 + j2) / 2;
		    roi2[ni][nj] = pval;
		    ni = (i1 + i2) / 2;
		    nj = j;
		    roi2[ni][nj] = pval;
		}
	    }			/* if */
	}			/* for j */
    }				/* for i */
}
