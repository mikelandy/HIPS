
/*  h_fill_holes.c                          Brian Tierney,  LBL  3/90
 *
 *   converted to HIPS2		    Felix Huang,    LBL  8/91
 *
 *  fills holes in a binary images
 *
 *  usage: fill_holes [-s NN] < infile > outfile
 *    where -s NN the size of holes to look for ( default = 2)
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
*/

#define Calloc(x,y) (y *)calloc((unsigned)(x), sizeof(y))

#include <stdio.h>
#include <hipl_format.h>

extern int hole_size;
extern h_boolean ends_only;

#define PVAL 255

void      look_for_holes(), fill_holes(), line_fill();

extern int nrow;
extern int ncol;
extern int i_ocol;

byte    **roi1, **roi2;

h_fill_holes(hdi, hdo)
    struct header *hdi, *hdo;
{
    i_to_r(hdi->firstpix);
    look_for_holes();
    r_to_o(hdo->firstpix);
}


/********************************************************************/
i_to_r(imagei)			/* input image to roi1	 */
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

r_to_o(imageo)			/* roi2 to output image	 */
    byte     *imageo;
{
    int       x, y;

    for (y = 0; y < nrow; y++, imageo = imageo + i_ocol)
	for (x = 0; x < ncol; x++)
	    imageo[x] = roi2[y][x];
}


/********************************************************************/
void
look_for_holes()
{
    register int i, j;

    for (i = hole_size; i < nrow - hole_size; i++)
	for (j = hole_size; j < ncol - hole_size; j++)
	    if (roi1[i][j] > 0) {	/* pixel is part of an object */
		roi2[i][j] = roi1[i][j];
		fill_holes(i, j);
	    }
}

/*************************************************************/
void
fill_holes(i, j)
    int       i, j;
{
    /*
     * NOTE: since we are scanning all pixels, we only need to look for holes
     * in 2 directions, the other 2 directions will be checked later when at
     * another pixel location
     */

    register int i1, j1, i2, j2, jj, ii;

    int       dist;

    dist = 2;			/* start 2 pixels away from point */

    while (dist <= hole_size + 1) {
	i1 = i - dist;
	j1 = j - dist;
	i2 = i + dist;
	j2 = j + dist;
	if (i1 < 0)
	    i1 = 0;
	if (j1 < 0)
	    j1 = 0;
	if (i2 > nrow - 1)
	    i2 = nrow - 1;
	if (j2 > ncol - 1)
	    j2 = ncol - 1;

	for (jj = j1; jj < j2; jj++) {
	    if (roi1[i2][jj] > 0)
		draw_line(j, i, jj, i2);
	}

	for (ii = i1; ii < i2; ii++) {
	    if (roi1[ii][j2] > 0)
		draw_line(j, i, j2, ii);
	}
	dist++;
    }
}

/*********************************************************************/
draw_line(x1, y1, x2, y2)
    int       x1, y1, x2, y2;
{
    int       a, b;

    if (ends_only == TRUE) {
	a = side_cnt(y1, x1);	/* check if 1st point is a end of a line */
	b = corner_cnt(y1, x1);
#ifdef TEST
	if (a > 1 || b > 1)	/* not an end */
#endif
	    if (a > 1 || b > 1 || (a == 1 && b == 1))	/* not an end */
		return;

	a = side_cnt(y2, x2);	/* check if 2nd point is a end of a line */
	b = corner_cnt(y2, x2);
#ifdef TEST
	if (a > 1 || b > 1)	/* not an end */
#endif
	    if (a > 1 || b > 1 || (a == 1 && b == 1))	/* not an end */
		return;
    }
    line_fill(roi2, x1, y1, x2, y2);
}

/*********************************************************************/

int
side_cnt(i, j)
    int       i, j;		/* current array location */
{
    int       cnt = 0;

    if (i > 0)
	if (roi1[i - 1][j] > 0)
	    cnt++;
    if (j > 0)
	if (roi1[i][j - 1] > 0)
	    cnt++;
    if (i < nrow - 1)
	if (roi1[i + 1][j] > 0)
	    cnt++;
    if (j < ncol - 1)
	if (roi1[i][j + 1] > 0)
	    cnt++;
    return (cnt);
}

/*********************************************************************/
int
corner_cnt(i, j)
    int       i, j;		/* current array location */
{
    int       cnt = 0;

    if (i > 0 && j > 0)
	if (roi1[i - 1][j - 1] > 0)
	    cnt++;
    if (i < nrow - 1 && j < ncol - 1)
	if (roi1[i + 1][j + 1] > 0)
	    cnt++;
    if (i < nrow - 1 && j > 0)
	if (roi1[i + 1][j - 1] > 0)
	    cnt++;
    if (i > 0 && j < ncol - 1)
	if (roi1[i - 1][j + 1] > 0)
	    cnt++;

    return (cnt);
}

/***************************************************************/
int
count_neighbors(c, r)
    register int c, r;		/* max value returned is 8 */
{
    int       neighbors = 0, tx, ty;

    tx = ncol - 1;
    ty = nrow - 1;

    if (r > 0)
	if (roi1[r - 1][c] > 0)
	    neighbors++;
    if (r < ty)
	if (roi1[r + 1][c] > 0)
	    neighbors++;

    if (c > 0)
	if (roi1[r][c - 1] > 0)
	    neighbors++;
    if (c < tx)
	if (roi1[r][c + 1] > 0)
	    neighbors++;

    if (r < ty && c < tx)
	if (roi1[r + 1][c + 1] > 0)
	    neighbors++;

    if (r > 0 && c > 0)
	if (roi1[r - 1][c - 1] > 0)
	    neighbors++;

    if (r > 0 && c < tx)
	if (roi1[r - 1][c + 1] > 0)
	    neighbors++;

    if (r < ty && c > 0)
	if (roi1[r + 1][c - 1] > 0)
	    neighbors++;

    return (neighbors);
}

/********************************************************************/
void
line_fill(buf, x1, y1, x2, y2)	/* Bresenhams's scan conversion algorithm */
    byte    **buf;
    int       x1, y1, x2, y2;
 /*
  * this code adapted from:   Digital Line Drawing by Paul Heckbert from
  * "Graphics Gems", Academic Press, 1990
  */
{
    int       d, x, y, ax, ay, sx, sy, dx, dy;

    /* absolute value of a */
#ifndef ABS
#define ABS(a)          (((a)<0) ? -(a) : (a))
#endif

    /* take binary sign of a, either -1, or 1 if >= 0 */
#define SGN(a)          (((a)<0) ? -1 : 1)

    if (x1 == x2 && y1 == y2) {
	/* single point, don 't need to scan convert */
	buf[y1][x1] = PVAL;
	return;
    }
    dx = x2 - x1;
    ax = ABS(dx) << 1;
    sx = SGN(dx);

    dy = y2 - y1;
    ay = ABS(dy) << 1;
    sy = SGN(dy);

    x = x1;
    y = y1;
    if (ax > ay) {		/* x dominant */
	d = ay - (ax >> 1);
	for (;;) {
	    buf[y][x] = PVAL;
	    if (x == x2)
		return;
	    if (d >= 0) {
		y += sy;
		d -= ax;
	    }
	    x += sx;
	    d += ay;
	}
    } else {			/* y dominant */

	d = ax - (ay >> 1);
	for (;;) {
	    buf[y][x] = PVAL;
	    if (y == y2)
		return;
	    if (d >= 0) {
		x += sx;
		d -= ay;
	    }
	    y += sy;
	    d += ax;
	}
    }
}
