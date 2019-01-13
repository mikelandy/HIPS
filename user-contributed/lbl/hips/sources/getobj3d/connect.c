
/* connect.c:   routines relating to the connectivity algorithm
                for getobj3d.c

    Brian Tierney,  LBL
*/

#include "getobj.h"

#define FASTER
/* FASTER changes routines which only access the data
   sequentially to use a faster method. The old method is retained
   because it is a bit more readable. */

/***************************************************************/
int
identify_objects(thresh_value)	/* labels objects with a '1' */
    int       thresh_value;
{
    int       num_on = 0, check_val;

#ifdef FASTER
    register int i;
    register u_char *bptr, *gptr;
    register u_short *sptr;
    register u_int *iptr;

    gptr = **grid;
    if (pix_format == PFBYTE)
	bptr = **image;
    else if (pix_format == PFSHORT)
	sptr = **sht_image;
    else
	iptr = **int_image;

    for (i = 0; i < nvoxels; i++) {
	if (pix_format == PFBYTE)
	    check_val = (int) bptr[i];
	else if (pix_format == PFSHORT)
	    check_val = (int) sptr[i];
	else
	    check_val = iptr[i];

	if (check_val < thresh_value)
	    gptr[i] = 0;
	else {
	    gptr[i] = 1;
	    num_on++;
	}
    }

#else				/* easier to read, about twice as slow  */
    register int c, r, f;

    /* change all data to 0 or 1, depending on the thresh value */
    for (f = 0; f < nframe; f++)
	for (r = 0; r < nrow; r++)
	    for (c = 0; c < ncol; c++) {
		if (pix_format == PFSHORT)
		    check_val = (int) sht_image[f][r][c];
		else if (pix_format == PFINT)
		    check_val = int_image[f][r][c];
		else
		    check_val = (int) image[f][r][c];

		if (check_val < thresh_value)
		    grid[f][r][c] = 0;
		else {
		    grid[f][r][c] = 1;
		    num_on++;
		}
	    }

#endif
    return (num_on);
}

/***************************************************************/
void
locate_surfaces()
{				/* mark all surfaces as 2 */
    register int c, r, f;

    /* label the surfacen points as 2 -- only checking 2-d slice */

    for (f = 0; f < nframe; f++)
	for (r = 0; r < nrow; r++)
	    for (c = 0; c < ncol; c++)
		if (grid[f][r][c] == 1) {
		    if ((r > 0 && grid[f][r - 1][c] == 0) ||
			(r < nrow - 1 && grid[f][r + 1][c] == 0) ||
			(c > 0 && grid[f][r][c - 1] == 0) ||
			(c < ncol - 1 && grid[f][r][c + 1] == 0)) {
			grid[f][r][c] = 2;
		    }
		}
}

/*********************************************************/
void
clear_background_image(bg_val)
    int       bg_val;
 /* sets the image pixels to zero based on the grid value */

{

#ifdef FASTER
    register int i;
    register u_char *bptr, *gptr;
    register u_short *sptr;
    register u_int *iptr;

    gptr = **grid;
    if (pix_format == PFBYTE)
	bptr = **image;
    else if (pix_format == PFSHORT)
	sptr = **sht_image;
    else
	iptr = **int_image;

    for (i = 0; i < nvoxels; i++) {
	if (gptr[i] == 0) {
	    if (pix_format == PFBYTE)
		bptr[i] = (u_char) bg_val;
	    else if (pix_format == PFSHORT)
		sptr[i] = (u_short) bg_val;
	    else
		iptr[i] = bg_val;
	}
    }

#else				/* easier to read, about twice as slow  */
    register int c, r, f;

    for (f = 0; f < nframe; f++)
	for (r = 0; r < nrow; r++)
	    for (c = 0; c < ncol; c++)
		if (grid[f][r][c] == 0) {
		    if (pix_format == PFBYTE)
			image[f][r][c] = (u_char) bg_val;
		    else if (pix_format == PFSHORT)
			sht_image[f][r][c] = (u_short) bg_val;
		    else
			int_image[f][r][c] = bg_val;
		}
#endif

}

/***************************************************************/
void
clear_background_grid()
 /*
  * at this point, all points in the desired object should be 3, so set any
  * other points to 0
  */
{

#ifdef FASTER
    register int i;
    register u_char *gptr;

    gptr = **grid;

    for (i = 0; i < nvoxels; i++) {
	if (gptr[i] == 3)
	    gptr[i] = 255;
	else
	    gptr[i] = 0;
    }

#else				/* easier to read, about twice as slow  */

    register int c, r, f;

    for (f = 0; f < nframe; f++)
	for (r = 0; r < nrow; r++)
	    for (c = 0; c < ncol; c++) {
		if (grid[f][r][c] == 3)
		    grid[f][r][c] = 255;
		else
		    grid[f][r][c] = 0;
	    }
#endif
}

/***************************************************************/
int
mark_edge_neighbors(c, r, f, x_flag)	/* if neighbors are an edge, mark as
					 * being part of object */
    register int c, r, f, x_flag;
{
    int       cnt = 0;

    if ((r > 0) && (grid[f][r - 1][c] == 2)) {
	grid[f][r - 1][c] = 3;
	cnt++;
    }
    if ((r < nrow - 1) && (grid[f][r + 1][c] == 2)) {
	grid[f][r + 1][c] = 3;
	cnt++;
    }
    if ((c > 0) && (grid[f][r][c - 1] == 2)) {
	grid[f][r][c - 1] = 3;
	cnt++;
    }
    if ((c < ncol - 1) && (grid[f][r][c + 1] == 2)) {
	grid[f][r][c + 1] = 3;
	cnt++;
    }
    if (x_flag) {		/* also check x-slice */
	if ((f > 0) && (grid[f - 1][r][c] == 2)) {
	    grid[f - 1][r][c] = 3;
	    cnt++;
	}
	if ((f < nframe - 1) && (grid[f + 1][r][c] == 2)) {
	    grid[f + 1][r][c] = 3;
	    cnt++;
	}
    }
    return (cnt);
}

/***************************************************************/
int
get_seed(sx, sy, sz)
    int      *sx, *sy, *sz;
/* looks for an object at the given seed point +-10 pixels:
 * Starts at given seed, first look up and right 10 pixels, then
 * looks down and left 10 pixels
*/
{
    register int c, r, f;

    int       bx, by, bz;

    fprintf(stderr, "\n Looking for object at (%d,%d,%d) \n", *sx, *sy, *sz);

    bx = *sx;
    by = *sy;
    bz = *sz;

    /* look at 4 edge neighbors to determine if good seed */
    for (f = bz; f < bz + 10; f++)
	for (r = by; r < by + 10; r++)
	    for (c = bx; c < bx + 10; c++)
		if (grid[f][r][c] == 1) {
		    if (count_neighbors(c, r, f) > 3) {
			*sx = c;
			*sy = r;
			*sz = f;
			return (0);
		    }
		}
    /* try again */
    for (f = bz - 10; f < bx; f++)
	for (r = by - 10; r < by; r++)
	    for (c = bx - 10; c < bz; c++)
		if (grid[f][r][c] == 1) {
		    if (count_neighbors(c, r, f) > 3) {
			*sx = c;
			*sy = r;
			*sz = f;
			return (0);
		    }
		}
    /* if get thru without returning */

    return (-1);
}

/***************************************************************/
int
get_seed_guess(sx, sy, sz)
    int      *sx, *sy, *sz;
 /*
  * if the user does not specify a seed value, then this routine located
  * reasonable guesses for seeds. If the -a (find all objects) flag is
  * specified, it begins the search from location (1,1,1). Otherwise it
  * starts near the center of the data set, and if it doesn't find anything,
  * then looks from location (1,1,1).
  */
{
/* returns a seed for an object larger than 8 pixels */
    register int c, r, f;

    static int bx = 0, by = 0, bz = 0;

    if (bx == 0 || by == 0 || bz == 0) {
	if (find_all)
	    bx = by = bz = 1;	/* start at upper corner */
	else {
	    bx = (ncol / 2) - 2;/* start near center of data set */
	    by = (nrow / 2) - 2;
	    bz = (nframe / 2);
	    if (bx < 1)
		bx = 1;
	    if (by < 1)
		by = 1;
	}
    }
    if (!find_all) {
	if (verbose) {
	    fprintf(stderr, "\n Looking for a seed value starting at: %d,%d,%d \n",
		    bx, by, bz);
	}
	for (f = bz; f < nframe - 1; f++)
	    for (r = by; r < nrow - 1; r++)
		for (c = bx; c < ncol - 1; c++)
		    if (grid[f][r][c] == 1) {
			if ((grid[f][r][c - 1] == 1) &&
			    (grid[f][r][c + 1] == 1) &&
			    (grid[f][r - 1][c] == 1) &&
			    (grid[f][r + 1][c] == 1) &&
			    (grid[f][r - 1][c - 1] == 1) &&
			    (grid[f][r + 1][c + 1] == 1) &&
			    (grid[f][r + 1][c - 1] == 1) &&
			    (grid[f][r - 1][c + 1] == 1)) {
			    *sx = c;
			    *sy = r;
			    *sz = f;
			    return (0);
			}
		    }
	bx = by = bz = 1;	/* didn't find a seed, so try again from
				 * upper corner */
    }
    if (verbose) {
	fprintf(stderr, "\n Looking for a seed value starting at: %d,%d,%d \n",
		bx, by, bz);
    }
    for (f = bz; f < nframe - 1; f++)
	for (r = by; r < nrow - 1; r++)
	    for (c = bx; c < ncol - 1; c++)
		if (grid[f][r][c] == 1) {
		    if ((grid[f][r][c - 1] == 1) &&
			(grid[f][r - 1][c] == 1) &&
			(grid[f][r][c + 1] == 1) &&
			(grid[f][r + 1][c] == 1) &&
			(grid[f][r - 1][c - 1] == 1) &&
			(grid[f][r + 1][c + 1] == 1) &&
			(grid[f][r + 1][c - 1] == 1) &&
			(grid[f][r - 1][c + 1] == 1)) {
			*sx = c;
			*sy = r;
			*sz = f;
			bx = c + 1;
			by = r;
			bz = f;
			return (0);
		    }
		}
    /* if get thru without returning */
    if (verbose)
	fprintf(stderr, " object not found \n");

    return (-1);
}

/***************************************************************/

void
init_grid(val)
    int       val;
 /* sets the entire grid to 'val' */
{

#ifdef FASTER
    register int i;
    register u_char *gptr;

    gptr = **grid;

    for (i = 0; i < nvoxels; i++)
	gptr[i] = val;

#else				/* easier to read, about twice as slow  */
    register int c, r, f;

    for (f = 0; f < nframe; f++)
	for (r = 0; r < nrow; r++)
	    for (c = 0; c < ncol; c++)
		grid[f][r][c] = val;
#endif
}

/***************************************************************/
int
count_diag_neighbor_edges(c, r, f, x_flag)
    register int c, r, f, x_flag;	/* max value returned is 12 */
{
    /*
     * NOTE: this routine only counts neighbors that are marked '2'. There
     * will also be some neighbors marked 0, but we don't want to count those
     * because when labeling the edges we only look at the 2-D slice.
     */

    int       neighbors = 0, tx, ty, tz;

    tx = ncol - 1;
    ty = nrow - 1;
    tz = nframe - 1;

    if ((r > 0) && (c > 0))
	if (grid[f][r - 1][c - 1] == 2)
	    neighbors++;
    if ((r > 0) && (c < tx))
	if (grid[f][r - 1][c + 1] == 2)
	    neighbors++;
    if ((r < ty) && (c > 0))
	if (grid[f][r + 1][c - 1] == 2)
	    neighbors++;
    if ((r < ty) && (c < tx))
	if (grid[f][r + 1][c + 1] == 2)
	    neighbors++;

    if (x_flag) {		/* count x-slices too */
	/* previous slice */
	if ((f > 0) && (r > 0))
	    if (grid[f - 1][r - 1][c] == 2)
		neighbors++;
	if ((f > 0) && (c > 0))
	    if (grid[f - 1][r][c - 1] == 2)
		neighbors++;
	if ((f > 0) && (r < ty))
	    if (grid[f - 1][r + 1][c] == 2)
		neighbors++;
	if ((f > 0) && (c < tx))
	    if (grid[f - 1][r][c + 1] == 2)
		neighbors++;

	/* next slice */
	if ((f < tz) && (r > 0))
	    if ((grid[f + 1][r - 1][c] == 2))
		neighbors++;
	if ((f < tz) && (c > 0))
	    if ((grid[f + 1][r][c - 1] == 2))
		neighbors++;
	if ((f < tz) && (r < ty))
	    if ((grid[f + 1][r + 1][c] == 2))
		neighbors++;
	if ((f < tz) && (c < tx))
	    if ((grid[f + 1][r][c + 1] == 2))
		neighbors++;
    }
    return (neighbors);
}

/***************************************************************/
int
count_neighbors(c, r, f)
    register int c, r, f;	/* max value returned is 6 */
{
    /*
     * counts number of primary neighbors, used by 'get_seed'
     */

    int       neighbors = 0, tx, ty, tz;

    tx = ncol - 1;
    ty = nrow - 1;
    tz = nframe - 1;

    if (f > 0)
	if (grid[f - 1][r][c] == 1)
	    neighbors++;
    if (f < tz)
	if (grid[f + 1][r][c] == 1)
	    neighbors++;

    if (r > 0)
	if (grid[f][r - 1][c] == 1)
	    neighbors++;
    if (r < ty)
	if (grid[f][r + 1][c] == 1)
	    neighbors++;

    if (c > 0)
	if (grid[f][r][c - 1] == 1)
	    neighbors++;
    if (c < tx)
	if (grid[f][r][c + 1] == 1)
	    neighbors++;

    return (neighbors);
}
