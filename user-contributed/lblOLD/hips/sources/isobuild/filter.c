
/* filter.c                Brian Tierney, LBL
 *
 *  for use with the isobuild program
 *
 *  These routines implement a 3D diffusion filter, as mentioned in the
 *   paper "3D Segmentation of MR Images of the Head Using Probability
 *   and Connectivity", by H. Cline, WE Lorenson, R, Kikinis, and F. Jolesz
 *   Journal of Computer Assisted Tomography, Nov 1990.
 *
 *  The use of this filter removes isolated voxels and reduces the
 *  effect of noise, however there is a loss of resulution.
 */

/* $Id: filter.c,v 1.3 1992/01/31 02:05:45 tierney Exp $ */

/* $Log: filter.c,v $
 * Revision 1.3  1992/01/31  02:05:45  tierney
 * *** empty log message ***
 *
 * Revision 1.2  1991/12/19  01:42:20  davidr
 * added RCS identification markers
 * */

static char rcsid[] = "$Id: filter.c,v 1.3 1992/01/31 02:05:45 tierney Exp $" ;

#include "isobuild.h"

static Grid_type **smoothed_slice;

/***************************************************************/

smooth_slice(slice)
    int       slice;
{
    register int r, c;
    Grid_type **alloc_2d_grid_array();

    if (slice == SZ)
	smoothed_slice = alloc_2d_grid_array(ydim, xdim);

    if (slice % 10 == 0)
	fprintf(stderr, "Smoothing slice %d...\n", slice);

    for (r = 0; r < ydim; r++)
	for (c = 0; c < xdim; c++)
	    diffusion_filter(c, r, slice);

    memcpy((char *) grid[slice][0], (char *) smoothed_slice[0], xdim * ydim);

#ifdef FDEBUG
    show_slice(slice);
#endif

    if (slice == EZ) {
	free_2d_grid_array(smoothed_slice);
	smoothed_slice = NULL;
    }
}

/***************************************************************/

diffusion_filter(x, y, z)
    int       x, y, z;
{

    int       g1, g2, g3, g4, g5, g6;
    float     asquared, tval1, tval2;

    if (x < xmax)
	g1 = (int) grid[z][y][x + 1];
    else
	g1 = (int) grid[z][y][x];

    if (x > 0)
	g2 = (int) grid[z][y][x - 1];
    else
	g2 = (int) grid[z][y][x];

    if (y < ymax)
	g3 = (int) grid[z][y + 1][x];
    else
	g3 = (int) grid[z][y][x];

    if (y > 0)
	g4 = (int) grid[z][y - 1][x];
    else
	g4 = (int) grid[z][y][x];

    if (z < zmax)
	g5 = (int) grid[z + 1][y][x];
    else
	g5 = (int) grid[z][y][x];

    if (z > 0)
	g6 = (int) grid[z - 1][y][x];
    else
	g6 = (int) grid[z][y][x];

/* #define NEED_ASPECT */

#ifdef NEED_ASPECT   /* this doesnt seem to help  -blt */

#define ASPECT .5		/* ratio of slice thickness to pixel size */
    asquared = ASPECT * ASPECT;
    tval1 = ((asquared * (g1 + g2 + g3 + g4)) + (g5 + g6) /
	     (4 + (8 * asquared)));
    tval2 = (tval1 / 2.) + tval1;

#else
    tval1 = (float) (g1 + g2 + g3 + g4 + g5 + g6) / 12.;
    tval2 = (tval1 / 2.) + tval1;

#endif
    if (tval2 > .5)
	smoothed_slice[y][x] = 1;
    else
	smoothed_slice[y][x] = 0;
}


/***************************************************************/
show_slice(slice)
{				/* for debugging  */
    register int c, r;

    fprintf(stderr, "\n frame #: %d \n", slice);
    for (r = 0; r < ydim; r++) {
	for (c = 0; c < xdim; c++)
	    fprintf(stderr, "%3d", grid[slice][r][c]);
	fprintf(stderr, "\n");
    }
}
