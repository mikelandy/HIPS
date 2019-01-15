
/* iso_divide.c    routine for computing the vertex locations  */

/*  for use with the isobuild program
 *
 * modified by Brian Tierney, LBL  12/90
 *            Lawrence Berkeley Laboratory
 *            Imaging Technologies Group
 *            email: bltierney@lbl.gov
 *
 */

/* $Id: iso_divide.c,v 1.3 1992/01/31 02:05:45 tierney Exp $ */

/* $Log: iso_divide.c,v $
 * Revision 1.3  1992/01/31  02:05:45  tierney
 * *** empty log message ***
 *
 * Revision 1.2  1991/12/19  01:42:20  davidr
 * added RCS identification markers
 * */

static char rcsid[] = "$Id: iso_divide.c,v 1.3 1992/01/31 02:05:45 tierney Exp $" ;

#include "isobuild.h"

/**************************** iso_surface ****************************/

int
iso_divide(startx, starty, endx, endy, slice)
    int       startx, starty, endx, endy, slice;
{
    register int x, y;
    int       index, ly, lx;
    int       npoints = 0;
    int       calc_index_and_temps();

    lx = endx - 1;
    ly = endy - 1;

    for (y = starty; y < ly; y++)
	for (x = startx; x < lx; x++) {
	    index = calc_index(x, y, slice);

	    if (index > 0) {
		add_point(x, y, slice);
		npoints++;
	    }
	}

    if (VERBOSE2)
	fprintf(stderr, "slice %d:  %d points \n", slice, npoints);

    return (npoints);
}

/********************************************************/
int
iso_divide_block(xloc, yloc, zloc, width, height, gb1, gb2, sx, sy, ex, ey)
    int       xloc, yloc, zloc, width, height;
    Grid_type **gb1, **gb2;
    int       sx, sy, ex, ey;
{
    register int i, j, xloc2, yloc2;
    int       npoints = 0, index;

    yloc2 = yloc;

    for (i = 0; i < height; i++) {
	xloc2 = xloc;
	for (j = 0; j < width; j++) {

	    if (xloc2 >= sx && yloc2 >= sy && xloc2 < ex && yloc2 < ey) {

		index = calc_index_block(xloc2, yloc2, gb1, gb2);

		if (index > 0) {/* index of 0 means no triangles */
		    add_point(xloc2, yloc2, zloc);
		    npoints++;
		}
	    }
	    xloc2++;
	}
	yloc2++;
    }

    return (npoints);
}

/********************************************************************/

int
calc_index_block(x1, y1, grid_b1, grid_b2)
    int       x1, y1;
    Grid_type **grid_b1, **grid_b2;

/* This subroutine calculates the index into the cell_table
 */
{
    int       x2, y2, index = 0;

    x2 = x1 + 1;
    y2 = y1 + 1;


    if (grid_b1[y1][x1] == 1)
	index++;
    if (grid_b1[y1][x2] == 1)
	index += 2;
    if (grid_b1[y2][x2] == 1)
	index += 4;
    if (grid_b1[y2][x1] == 1)
	index += 8;

    if (grid_b2[y1][x1] == 1)
	index += 16;
    if (grid_b2[y1][x2] == 1)
	index += 32;
    if (grid_b2[y2][x2] == 1)
	index += 64;
    if (grid_b2[y2][x1] == 1)
	index += 128;


    /* if index = 0 or 255, then the surface does not intersect this voxel */
    if (index == 0 || index == 255)
	return (0);

    return (index);
}

/*******************************************************************/
int
calc_index(x1, y1, z1)
    int       x1, y1, z1;
{
    int       x2, y2, z2, index = 0;

    x2 = x1 + 1;
    y2 = y1 + 1;
    z2 = z1 + 1;

    if (grid[z1][y1][x1] == 1)
	index++;
    if (grid[z1][y1][x2] == 1)
	index += 2;
    if (grid[z1][y2][x2] == 1)
	index += 4;
    if (grid[z1][y2][x1] == 1)
	index += 8;

    if (grid[z2][y1][x1] == 1)
	index += 16;
    if (grid[z2][y1][x2] == 1)
	index += 32;
    if (grid[z2][y2][x2] == 1)
	index += 64;
    if (grid[z2][y2][x1] == 1)
	index += 128;

    /* if index = 0 or 255, then the surface does not intersect this voxel */
    if (index == 0 || index == 255)
	return (0);

    return (index);
}

/********************************************************************/

POINT_PTR point_list = NULL;
#define BUF_SIZE 100001
int       POINT_LIMIT = BUF_SIZE;
#define POINT_INCR 60000	/* size to increase vertex lists when full */

static int num_points = 0;

/**************************** add_polygon ****************************/

int
add_point(x, y, z)
    int       x, y, z;
{
    NORMAL_VECT norm;
    NORMAL_VECT      calc_normal();

    int       size;

    if (point_list == NULL) {
	point_list = (POINT_PTR) malloc(sizeof(POINT) * POINT_LIMIT);
    }

    /* store the vertices */
    point_list[num_points].x = (u_char) x;	/* x of first point */
    point_list[num_points].y = (u_char) y;	/* y of first point */
    point_list[num_points].z = (u_char) z;	/* z of first point */

    if (PRE_NORMALS) {
	point_list[num_points].nx = normals[z][y][x].x;
	point_list[num_points].ny = normals[z][y][x].y;
	point_list[num_points].nz = normals[z][y][x].z;
    } else {
	norm = calc_normal(x, y, z);
	point_list[num_points].nx = norm.x;
	point_list[num_points].ny = norm.y;
	point_list[num_points].nz = norm.z;
    }

    num_points++;

    if (SMALL_CHUNKS) {
	if (num_points + 3 >= POINT_LIMIT) {
	    dump_points(1);	/* write points found so far */
	}
    } else {
	if (num_points + 3 >= POINT_LIMIT) {
	    /* get more space */
	    POINT_LIMIT += POINT_INCR;	/* add this much space */
	    size = POINT_LIMIT * sizeof(POINT);	/* size for malloc/realloc */
	    if ((point_list = (POINT_PTR) realloc((char *) point_list,
						  size)) == NULL) {
		Error("not enough memory to store vertices");
	    }
	}
    }
    return 0;
}

/*********************************************************************/
int
dump_points(more)
    int       more;
{
    void      write_points_to_socket();

    if (SERVER) {
	write_points_to_socket(point_list, num_points, more);
    } else if (OUTPUT_TYPE == 0) {
	write_points(point_list, num_points, more);
    } else if (OUTPUT_TYPE == 1) {
	write_points_ascii(point_list, num_points);
    }
    num_points = 0;
    return;
}
