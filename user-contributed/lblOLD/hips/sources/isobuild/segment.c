
/* segment.c                Brian Tierney, LBL
 *
 *  for use with the isobuild program
 *
 *  contains routines for segmentting 3D data, either by thresholding,
 *   or by a 3D connectivity method
 */

/* $Id: segment.c,v 1.6 1992/01/31 02:05:45 tierney Exp $ */

/* $Log: segment.c,v $
 * Revision 1.6  1992/01/31  02:05:45  tierney
 * *** empty log message ***
 *
 * Revision 1.5  1992/01/30  20:05:03  davidr
 * prior to Brian's changes
 *
 * Revision 1.4  1992/01/10  01:59:31  davidr
 * works with triserv now
 *
 * Revision 1.2  1991/12/19  01:42:20  davidr
 * added RCS identification markers
 * */

static char rcsid[] = "$Id: segment.c,v 1.6 1992/01/31 02:05:45 tierney Exp $" ;

#include "isobuild.h"

typedef struct s_item {		/* stack used to simulate recursion for
				 * segmentation */
    short     i, j, k;
}         STACK_ITEM;

int       stack_size;

STACK_ITEM *stack = NULL;
int       sp;			/* stack pointer */


/* a 'grid' is used to perform the segmentation. The meaning of the
 *  grid values are as follows:
 *     1 : thresholded object
 *     2 : edge of object  (if DO_BRIDGES is on )
 *     3 : flood filled object
 */

/* #define SEG_DEBUG */

/* useful globals */
int       min_size;

/***************************************************************/

threshold_block(binfo, thresh)
    BLOCK_INFO *binfo;
    Data_type thresh;
{
    register int i, j;
    int       x, y;

    y = binfo->yloc;
    for (j = 0; j <= binfo->height; j++) {
	x = binfo->xloc;
	for (i = 0; i <= binfo->width; i++) {

	    if (x < xdim && y < ydim) {
		if (binfo->dslice[y][x] >= thresh)
		    binfo->grid[y][x] = 1;
	    }
	    x++;
	}
	y++;
    }
}

/***************************************************************/
int
seg_slice_thresh(thresh, data2d, grid2d, sx, sy, ex, ey, slice)
    Data_type thresh;
    Data_type **data2d;
    Grid_type **grid2d;
    int       sx, sy, ex, ey, slice;
{
    /* simple thresholding to segment data */

    register int i, j;
    int       num_on = 0;

#ifdef SEG_DEBUG2
    fprintf(stderr, "segmenting slice %d \n", slice);
#endif

    for (j = sy; j < ey; j++)
	for (i = sx; i < ex; i++) {
	    if (grid2d[j][i] != 255) {	/* not input mask */
		if (data2d[j][i] >= thresh) {
		    grid2d[j][i] = 1;
		    num_on++;
		} else {
		    grid2d[j][i] = 0;
		}
	    }
	}

    if (VERBOSE2)
	fprintf(stderr, "%d voxels selected from slice %d \n", num_on, slice);

#ifdef SEG_DEBUG2
    Status("after thresholding...");
    show_grid_slice(grid2d, sx, sy, ex, ey);
#endif

    return (num_on);
}

/****************************************************************/
int
flood_fill_segmentation(thresh, bridge)
    Data_type thresh;
    int       bridge;
{
    int       rval, nvoxels, try_cnt,slice;
    void      init_grid();
    int       clear_background_grid();
    void      clear_background_image(), locate_surfaces(), show_slices();

    init_grid(0);

    if (VERBOSE)
	Status("Locating objects...");

    stack_size = identify_objects(thresh);	/* label objects '1' */

    if (SMOOTH_GRID) {
	for (slice=SZ; slice<EZ; slice++)
	    smooth_slice(slice);
    }

    fprintf(stderr, "%d voxels selected \n", stack_size);

    min_size = stack_size / 20;
    if (min_size > 4000)
	min_size = 4000;	/* dont let recursion go too deep */

    fprintf(stderr, "Looking for objects at least %d voxels in size \n",
	    min_size);

    alloc_stack(stack_size);	/* stack shouldn't be larger than the number
				 * of pixels found by identify_objects */

    locate_surfaces();		/* label edges of object '2' */

    try_cnt = 0;
    while ((rval = locate_object(bridge, try_cnt)) <= 0) {
	if (rval < 0)
	    return (-1);
	try_cnt++;
    }

    if (VERBOSE)
	Status("Zeroing non-objects...");

    nvoxels = clear_background_grid();	/* grid = 0 if grid != 3 */

    fprintf(stderr, "%d voxels selected after flood-fill segmentation \n",
	    nvoxels);

    cfree((char *) stack);

#ifdef SEG_DEBUG
    show_slices();
#endif

    return (thresh);
}

/***************************************************************/
int
locate_object(bridge, try_cnt)
    int       bridge, try_cnt;
{
    int       rval, pcnt;
    static int sx = 1, sy = 1, sz = 1;
    int       object_fill(), get_seed_guess1(), get_seed_guess2();
    void      reset_grid();

    if (try_cnt == 0) {
	rval = get_seed_guess1(&sx, &sy, &sz);
    } else {
	rval = get_seed_guess2(&sx, &sy, &sz);
    }
    if (rval < 0) {		/* didn't find an object */
	Status("Object not found.");
	return (-1);		/* give up */
    }
    fprintf(stderr, "Trying seed location: %d, %d, %d \n", sx, sy, sz);

    pcnt = object_fill(sx, sy, sz, bridge);

    if (pcnt < min_size) {
	fprintf(stderr, "Object at location %d,%d,%d too small (%d pixels) \n",
		sx, sy, sz, pcnt);
	reset_grid(sx, sy, sz);	/* reset before doing flood fill */
	if (sx < xmax)
	    sx++;
	if (sy < ymax)
	    sy++;
	if (sz < zmax)
	    sz++;
	return (0);		/* try again */
    } else {
	fprintf(stderr, "Slice %d: Object at location: (%d,%d);  %d pixels \n",
		sx, sy, sz, pcnt);
	return (1);		/* success */
    }
}

/***************************************************************/
void
reset_grid(c, r, f)
    int       c, r, f;
{
/* restore grid to original state before check_object_size routine
 *  by setting all locations marked 3 back to 1
 */

    sp = 0;			/* initialize stack pointer */

    push(-1, -1, -1);		/* null stack */
    do {

start:
	grid[f][r][c] = 1;

	if ((f < zmax) && (grid[f + 1][r][c] == 3)) {
	    push(c, r, f);
	    f++;
	    goto start;
	}
	if ((f > 0) && (grid[f - 1][r][c] == 3)) {
	    push(c, r, f);
	    f--;
	    goto start;
	}
	if ((r < ymax) && (grid[f][r + 1][c] == 3)) {
	    push(c, r, f);
	    r++;
	    goto start;
	}
	if ((r > 0) && (grid[f][r - 1][c] == 3)) {
	    push(c, r, f);
	    r--;
	    goto start;
	}
	if ((c < xmax) && (grid[f][r][c + 1] == 3)) {
	    push(c, r, f);
	    c++;
	    goto start;
	}
	if ((c > 0) && (grid[f][r][c - 1] == 3)) {
	    push(c, r, f);
	    c--;
	    goto start;
	}
	pop(&c, &r, &f);

    } while (f >= 0);		/* neg i indicates empty stack */

    if (sp != 0)
	Error("stack not empty.");

    return;
}

/***************************************************************/
void
show_slices()
{				/* for debugging  */
    register int c, r, f;

    for (f = 100; f < 101; f++) {
	fprintf(stderr, "\n frame #: %d \n", f);
	for (r = 0; r < ydim; r++) {
	    for (c = 0; c < xdim; c++)
		fprintf(stderr, "%3d", grid[f][r][c]);
	    fprintf(stderr, "\n");
	}
    }
}

/***************************************************************/
int
identify_objects(thresh_value)	/* labels objects with a '1' */
    Data_type thresh_value;
{
    int       num_on = 0, check_val;

    long      nvoxels;
    register int i;
    register Grid_type *gptr;
    register Data_type *dptr;

    gptr = **grid;
    dptr = **data;

    nvoxels = zdim * xdim * ydim;

    for (i = 0; i < nvoxels; i++) {
	check_val = (int) dptr[i];

	if (check_val < thresh_value)
	    gptr[i] = 0;
	else {
	    gptr[i] = 1;
	    num_on++;
	}
    }

    return (num_on);
}

/***************************************************************/
void
locate_surfaces()
{				/* mark all surfaces as 2 */
    register int c, r, f;

    /* label the surface points as 2 -- only checking 2-d slice */

    for (f = 0; f < zdim; f++)
	for (r = 0; r < ydim; r++)
	    for (c = 0; c < xdim; c++)
		if (grid[f][r][c] == 1) {
		    if ((r > 0 && grid[f][r - 1][c] == 0) ||
			(r < ymax && grid[f][r + 1][c] == 0) ||
			(c > 0 && grid[f][r][c - 1] == 0) ||
			(c < xmax && grid[f][r][c + 1] == 0)) {
			grid[f][r][c] = 2;
		    }
		}
}

/***************************************************************/
int
clear_background_grid()
 /*
  * at this point, all points in the desired object should be 3, so set any
  * other points to 0
  */
{
    register int i;
    long      nvoxels;
    register Grid_type *gptr;
    int       cnt = 0;

    gptr = **grid;
    nvoxels = zdim * xdim * ydim;

    for (i = 0; i < nvoxels; i++) {
	if (gptr[i] == 3) {
	    gptr[i] = 1;
	    cnt++;
	} else
	    gptr[i] = 0;
    }
    return (cnt);
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
    if ((r < ymax) && (grid[f][r + 1][c] == 2)) {
	grid[f][r + 1][c] = 3;
	cnt++;
    }
    if ((c > 0) && (grid[f][r][c - 1] == 2)) {
	grid[f][r][c - 1] = 3;
	cnt++;
    }
    if ((c < xmax) && (grid[f][r][c + 1] == 2)) {
	grid[f][r][c + 1] = 3;
	cnt++;
    }
    if (x_flag) {		/* also check x-slice */
	if ((f > 0) && (grid[f - 1][r][c] == 2)) {
	    grid[f - 1][r][c] = 3;
	    cnt++;
	}
	if ((f < zmax) && (grid[f + 1][r][c] == 2)) {
	    grid[f + 1][r][c] = 3;
	    cnt++;
	}
    }
    return (cnt);
}

/***************************************************************/

int
get_seed_guess1(sx, sy, sz)
    int      *sx, *sy, *sz;
{
/* looks for a good seed value starting near the center of the image */
    register int f, r, c;

    int       bx, by, bz;

    bx = (xdim / 2) - 2;	/* start near center of data set */
    by = (ydim / 2) - 2;
    bz = (zdim / 2);

    if (VERBOSE2)
	fprintf(stderr, "Guess1: Looking for seed loc starting at: %d,%d,%d \n",
		bx, by, bz);

    for (f = bz; f < zmax; f++)
	for (r = by; r < ymax; r++)
	    for (c = bx; c < xmax; c++)
		if (grid[f][r][c] == 1) {
		    if ((grid[f][r][c - 1] == 1) &&
			(grid[f][r][c + 1] == 1) &&
			(grid[f][r - 1][c] == 1) &&
			(grid[f][r + 1][c] == 1)
			&& (grid[f][r - 1][c - 1] == 1) &&
			(grid[f][r + 1][c + 1] == 1) &&
			(grid[f][r + 1][c - 1] == 1) &&
			(grid[f][r - 1][c + 1] == 1)) {
			*sx = c;
			*sy = r;
			*sz = f;
			return (0);
		    }
		}
    return (-1);
}

/***************************************************************/

int
get_seed_guess2(sx, sy, sz)
    int      *sx, *sy, *sz;
{
    /*
     * looks for a good seed value starting at the upper left corner of the
     * image
     */

    register int f, r, c;
    int       bx, by, bz;

    bx = *sx;
    by = *sy;
    bz = *sz;

    if (VERBOSE2)
	fprintf(stderr, "Guess2: Looking for seed loc starting at: %d,%d,%d \n",
		bx, by, bz);

    for (f = bz; f < zmax; f++)
	for (r = by; r < ymax; r++)
	    for (c = bx; c < zmax; c++)
		if (grid[f][r][c] == 1) {
		    if ((grid[f][r][c - 1] == 1) &&
			(grid[f][r][c + 1] == 1) &&
			(grid[f][r - 1][c] == 1) &&
			(grid[f][r + 1][c] == 1)
			&& (grid[f][r - 1][c - 1] == 1) &&
			(grid[f][r + 1][c + 1] == 1) &&
			(grid[f][r + 1][c - 1] == 1) &&
			(grid[f][r - 1][c + 1] == 1)
			) {
			*sx = c;
			*sy = r;
			*sz = f;
			return (0);
		    }
		}
    return (-1);
}


/***************************************************************/

void
init_grid(val)
    int       val;
 /* sets the entire grid to 'val' */
{
    int       nvoxels;

    nvoxels = ydim * xdim * zdim;
    if (VERBOSE)
	Status("Initializing grid.");

#ifdef OLD
    register int i;
    gptr = **grid;
    register Grid_type *gptr;

    for (i = 0; i < nvoxels; i++)
	gptr[i] = val;
#else
    memset((char *) grid[0][0], val, nvoxels);
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

    int       neighbors = 0;

    if ((r > 0) && (c > 0))
	if (grid[f][r - 1][c - 1] == 2)
	    neighbors++;
    if ((r > 0) && (c < xmax))
	if (grid[f][r - 1][c + 1] == 2)
	    neighbors++;
    if ((r < ymax) && (c > 0))
	if (grid[f][r + 1][c - 1] == 2)
	    neighbors++;
    if ((r < ymax) && (c < xmax))
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
	if ((f > 0) && (r < ymax))
	    if (grid[f - 1][r + 1][c] == 2)
		neighbors++;
	if ((f > 0) && (c < xmax))
	    if (grid[f - 1][r][c + 1] == 2)
		neighbors++;
	/* next slice */
	if ((f < zmax) && (r > 0))
	    if ((grid[f + 1][r - 1][c] == 2))
		neighbors++;
	if ((f < zmax) && (c > 0))
	    if ((grid[f + 1][r][c - 1] == 2))
		neighbors++;
	if ((f < zmax) && (r < ymax))
	    if ((grid[f + 1][r + 1][c] == 2))
		neighbors++;
	if ((f < zmax) && (c < xmax))
	    if ((grid[f + 1][r][c + 1] == 2))
		neighbors++;
    }
    return (neighbors);
}

/***************************************************************/
int
object_fill(c, r, f, bridges)	/* label all points within the item 1 */
    int       c, r, f, bridges;

/* this routine uses a stack to do a 3D recursive flood fill starting
   at location (c,r,f)  */
{
    void      sum_pixel();

    int       cnt = 0, skip = 0;/* count number of points in object */
    sp = 0;			/* initialize stack pointer */

    push(-1, -1, -1);		/* null stack */
    do {

start:
	grid[f][r][c] = 3;
	cnt++;
	if (bridges > 0 && cnt > 1) {	/* check strength of connecting
					 * bridges */

	    if (bridges == 1) {	/* 2D bridges */
		cnt += mark_edge_neighbors(c, r, f, 0);
		if (count_diag_neighbor_edges(c, r, f, 0) >= 2) {
		    skip++;
		    goto w_stop;
		}
	    }
	    if (bridges == 2) {	/* 3D weak bridges */
		cnt += mark_edge_neighbors(c, r, f, 1);
		if (count_diag_neighbor_edges(c, r, f, 1) >= 4) {
		    skip++;
		    goto w_stop;
		}
	    }
	    if (bridges == 3) {	/* 3D strong bridges */
		cnt += mark_edge_neighbors(c, r, f, 1);
		if (count_diag_neighbor_edges(c, r, f, 1) >= 2) {
		    skip++;
		    goto w_stop;
		}
	    }
	}
#ifdef NEED?
	else {
	    cnt += mark_edge_neighbors(c, r, f, 0);	/* include edges */
	}
#endif

	if ((f < zmax) && (grid[f + 1][r][c] == 1)) {
	    push(c, r, f);
	    f++;
	    goto start;
	}
	if ((f > 0) && (grid[f - 1][r][c] == 1)) {
	    push(c, r, f);
	    f--;
	    goto start;
	}
	if ((r < ymax) && (grid[f][r + 1][c] == 1)) {
	    push(c, r, f);
	    r++;
	    goto start;
	}
	if ((r > 0) && (grid[f][r - 1][c] == 1)) {
	    push(c, r, f);
	    r--;
	    goto start;
	}
	if ((c < xmax) && (grid[f][r][c + 1] == 1)) {
	    push(c, r, f);
	    c++;
	    goto start;
	}
	if ((c > 0) && (grid[f][r][c - 1] == 1)) {
	    push(c, r, f);
	    c--;
	    goto start;
	}
w_stop:
	pop(&c, &r, &f);

    } while (f >= 0);		/* neg i indicates empty stack */

    if (sp != 0)
	Error("stack not empty.");

    if (bridges)
	fprintf(stderr, " fill stopped %d times due to weak bridges \n", skip);

    return (cnt);
}

/***************************************************************/
push(i, j, k)			/* add location to stack */
    int       i, j, k;
{
    sp++;
    if (sp >= stack_size) {
	fprintf(stderr, "Stack was allocated %d slots \n", stack_size);
	Error("Recursive stack overflow!!");
    }
    stack[sp].i = i;
    stack[sp].j = j;
    stack[sp].k = k;
}

/***************************************************************/
pop(i, j, k)			/* remove item from stack */
    int      *i, *j, *k;
{
    *i = stack[sp].i;
    *j = stack[sp].j;
    *k = stack[sp].k;
    sp--;
}

/***************************************************************/
alloc_stack(st_size)		/* allocate stack for non-recursive
				 * flood-fill alg */
    int       st_size;
{
    if ((stack = Calloc(st_size, STACK_ITEM)) == NULL)
	perror("calloc: stack");
}
