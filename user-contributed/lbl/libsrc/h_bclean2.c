
/*  bclean.c                                      Brian Tierney,  LBL  3/90
 *
 *  removes small 8-connected objects from binary images.
 *
 *  Note: only recursive version has been converted to HIPS2, should
 *   also convert the non-recursive version, because it is faster. -bt
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
 *            Imaging and Distributed Computing Group
 *            email: bltierney@lbl.gov
 *
 *  converted to HIPS2
*/

#define Calloc(x,y) (y *)calloc((unsigned)(x), sizeof(y))

#include <stdio.h>
#include <hipl_format.h>

extern int min_size;

byte     *image;
byte     *grid;

int       y_begin;
int       y_end;
int       x_begin;
int       x_end;
int       ocol;
void get_size();

int       count;

int      *str_x;
int      *str_y;

int h_bclean(hdo)
    struct header *hdo;
{
    int       x, y;
    int       i;

    if (hdo->pixel_format != PFBYTE)
    	return(perr(HE_FMTSUBR,"h__bclean2",hformatname(hdo->pixel_format)));
    image = hdo->image;
    grid = Calloc(hdo->numpix, byte);

    y_begin = hdo->frow;
    y_end = y_begin + hdo->rows;

    x_begin = hdo->fcol;
    x_end = x_begin + hdo->cols;

    ocol = hdo->ocols;

    str_x = Calloc(min_size, int);
    str_y = Calloc(min_size, int);

    for (y = y_begin; y < y_end; y++) {
	for (x = x_begin; x < x_end; x++) {
	    if (grid[y * ocol + x] == 0 && image[y * ocol + x] > 0) {
		count = 0;
		get_size(x, y);

		if (count < min_size)	/* clear a connected area	 */
		    for (i = 0; i < count; i++)
			image[str_y[i] * ocol + str_x[i]] = 0;
	    }
	}
    }
    return(HIPS_OK);
}				/* end of  h_bclean (hdo)	 */

void get_size(x, y)
    int       x, y;
{
    int       ax, ay;

    grid[y * ocol + x] = 1;

    if (count < min_size) {
	str_x[count] = x;
	str_y[count] = y;
	count++;
    }
    ay = y + 1;
    if (ay < y_end && grid[ay * ocol + x] == 0 && image[ay * ocol + x] > 0)
	get_size(x, ay);

    ay = y - 1;
    if (ay >= y_begin && grid[ay * ocol + x] == 0 && image[ay * ocol + x] > 0)
	get_size(x, ay);

    ax = x + 1;
    if (ax < x_end && grid[y * ocol + ax] == 0 && image[y * ocol + ax] > 0)
	get_size(ax, y);

    ax = x - 1;
    if (ax >= x_begin && grid[y * ocol + ax] == 0 && image[y * ocol + ax] > 0)
	get_size(ax, y);

    /* diagonals */
    ax = x + 1;
    ay = y + 1;
    if (ay < y_end && ax < x_end && grid[ay * ocol + ax] == 0 && 
	image[ay * ocol + ax] > 0)
	get_size(ax, ay);

    ax = x + 1;
    ay = y - 1;
    if (ay >= y_begin && ax < x_end && grid[ay * ocol + ax] == 0 && 
	image[ay * ocol + ax] > 0)
	get_size(ax, ay);

    ax = x - 1;
    ay = y + 1;
    if (ay < y_end && ax >= x_begin && grid[ay * ocol + ax] == 0 && 
	image[ay * ocol + ax] > 0)
	get_size(ax, ay);

    ax = x - 1;
    ay = y - 1;
    if (ay >= y_begin && ax >= x_begin && grid[ay * ocol + ax] == 0 && 
	image[ay * ocol + ax] > 0)
	get_size(ax, ay);

}				/* end of  get_size (x, y)	 */
