/* iso_march.c    routine for computing the marching cubes triangles
		  and edge list */

/*  for use with the isobuild program
 *
 * by Brian Tierney, LBL  12/90
 *            Lawrence Berkeley Laboratory
 *            Imaging Technologies Group
 *            email: bltierney@lbl.gov
 *
*/

/* $Id: iso_march.c,v 1.6 1992/01/31 02:05:45 tierney Exp $ */

/* $Log: iso_march.c,v $
 * Revision 1.6  1992/01/31  02:05:45  tierney
 * y
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

static char rcsid[] = "$Id: iso_march.c,v 1.6 1992/01/31 02:05:45 tierney Exp $";

#include "isobuild.h"
#include "cell_table.h"

/************ Temporary Globals *****************/

float     DATA1, DATA2, DATA3, DATA4, DATA5, DATA6, DATA7, DATA8;

/**************************** iso_surface ****************************/

int
iso_march(startx, starty, endx, endy, slice)
    int       startx, starty, endx, endy, slice;
{
    register int x, y, xdim1, ydim1;
    int       index;
    int       npolys = 0;
    CUBE_TRIANGLES **tmpslice;
    float     crossings[13][3];

    for (y = 0; y < 3; y++)	/* initialize triangles array */
	for (x = 0; x < 13; x++)
	    crossings[x][y] = 0.;

    xdim1 = endx - 1;
    ydim1 = endy - 1;

    for (y = starty; y < ydim1; y++) {
	for (x = startx; x < xdim1; x++) {
	    index = calc_index_and_temps(x, y, slice);

	    if (index > 0) {	/* index of 0 means no triangles */

		if ((get_cell_verts(index, (float) x, (float) y,
				    (float) slice, (float) TVAL,
				    crossings)) < 0)
		    return (-1);
		npolys += get_cell_polys(index, crossings, x, y, slice);
	    }
	}
    }
    if (DUP_CHECK) {
	tmpslice = currslice;
	currslice = prevslice;
	prevslice = tmpslice;
    }
    if (VERBOSE2)
	fprintf(stderr, "Slice %d: %d polygons \n", slice, npolys);

    return (npolys);
}

/********************************************************/
int
iso_march_block(xloc, yloc, zloc, width, height,
		sx, sy, ex, ey)
    int       xloc, yloc, zloc, width, height;
    int       sx, sy, ex, ey;
{
    register int i, j, x, y;
    int       index, npolys = 0;
    float     crossings[13][3];
    CUBE_TRIANGLES **tmpslice;

    for (y = 0; y < 3; y++)	/* initialize triangles array */
	for (x = 0; x < 13; x++)
	    crossings[x][y] = 0.;

    y = yloc;
    for (i = 0; i < height; i++) {
	x = xloc;
	for (j = 0; j < width; j++) {

	    if (x >= sx && y >= sy && x < ex && y < ey) {

		index = calc_index_and_temps(x, y, zloc);

		if (index > 0) {/* index of 0 means no triangles */
		    if (DUP_CHECK) {	/* initialize */
			currslice[y][x].edge_index = -1;
			currslice[y][x].num_edges = 0;
		    }
		    if ((get_cell_verts(index, (float) x, (float) y,
					(float) zloc, (float) TVAL,
					crossings)) < 0)
			return (-1);
		    npolys += get_cell_polys(index, crossings, x, y, zloc);
		}
	    }
	    x++;
	}
	y++;
    }
    if (DUP_CHECK) {
	tmpslice = currslice;
	currslice = prevslice;
	prevslice = tmpslice;
    }
    return (npolys);
}

/************************* calc_index_and_temps ****************************/
int
calc_index_and_temps(x1, y1, z1)
    int       x1, y1, z1;

/* This subroutine calculates the index into the cell_table,
 * and creates some global temporary variables (for speed).
 */
{
    int       x2, y2, z2, index = 0;
    Grid_type grid_cube[8];
    float     nval;

    x2 = x1 + 1;
    y2 = y1 + 1;
    z2 = z1 + 1;

    grid_cube[0] = grid[z1][y1][x1];
    grid_cube[1] = grid[z1][y1][x2];
    grid_cube[2] = grid[z1][y2][x2];
    grid_cube[3] = grid[z1][y2][x1];
    grid_cube[4] = grid[z2][y1][x1];
    grid_cube[5] = grid[z2][y1][x2];
    grid_cube[6] = grid[z2][y2][x2];
    grid_cube[7] = grid[z2][y2][x1];


    if (grid_cube[0] == 1)
	index++;
    if (grid_cube[1] == 1)
	index += 2;
    if (grid_cube[2] == 1)
	index += 4;
    if (grid_cube[3] == 1)
	index += 8;

    if (grid_cube[4] == 1)
	index += 16;
    if (grid_cube[5] == 1)
	index += 32;
    if (grid_cube[6] == 1)
	index += 64;
    if (grid_cube[7] == 1)
	index += 128;

    /* if index = 0 or 255, then the surface does not intersect this voxel */
    if (index == 0 || index == 255)
	return (0);

    DATA1 = (float) data[z1][y1][x1];
    DATA2 = (float) data[z1][y1][x2];
    DATA3 = (float) data[z1][y2][x2];
    DATA4 = (float) data[z1][y2][x1];

    DATA5 = (float) data[z2][y1][x1];
    DATA6 = (float) data[z2][y1][x2];
    DATA7 = (float) data[z2][y2][x2];
    DATA8 = (float) data[z2][y2][x1];



    if (SEG_METHOD > 0 || SMALLER_BOX == 1 || SMOOTH_GRID == 1) {
	/* create false value N % from the thresh value */
	nval = (float) (TVAL - ((data_max - data_min) * .1));

	if (DATA1 > TVAL && grid_cube[0] <= 0)
	    DATA1 = nval;

	if (DATA2 > TVAL && grid_cube[1] <= 0)
	    DATA2 = nval;

	if (DATA3 > TVAL && grid_cube[2] <= 0)
	    DATA3 = nval;

	if (DATA4 > TVAL && grid_cube[3] <= 0)
	    DATA4 = nval;

	if (DATA5 > TVAL && grid_cube[4] <= 0)
	    DATA5 = nval;

	if (DATA6 > TVAL && grid_cube[5] <= 0)
	    DATA6 = nval;

	if (DATA7 > TVAL && grid_cube[6] <= 0)
	    DATA7 = nval;

	if (DATA8 > TVAL && grid_cube[7] <= 0)
	    DATA8 = nval;
    }

    return (index);
}

/**************************** get_cell_verts ****************************/

int
get_cell_verts(index, x1, y1, z1, threshold, crossings)
    int       index;
    float     x1, y1, z1;
    float     threshold;
    float     crossings[13][3];
{
/* This routine computes the vertex locations */

    register int i;
    float     x2, y2, z2, val;
    int       nedges;
    int       crnt_edge;

#define linterp(d1,d2,t,x) ((t-d1) / ((d2-d1)) + x)

    x2 = x1 + 1.;
    y2 = y1 + 1.;
    z2 = z1 + 1.;

    nedges = cell_table[index].nedges;
    for (i = 0; i < nedges; i++) {
	crnt_edge = cell_table[index].edges[i];
	switch (crnt_edge) {
	case 1:
	    if (DATA2 == DATA1)
		val = x1;
	    else {
		val = linterp(DATA1, DATA2, threshold, x1);
	    }
	    crossings[1][0] = val;
	    crossings[1][1] = y1;
	    crossings[1][2] = z1;
	    break;
	case 2:
	    if (DATA3 == DATA2)
		val = y1;
	    else {
		val = linterp(DATA2, DATA3, threshold, y1);
	    }
	    crossings[2][1] = val;
	    crossings[2][0] = x2;
	    crossings[2][2] = z1;
	    break;

	case 3:
	    if (DATA3 == DATA4)
		val = x1;
	    else {
		val = linterp(DATA4, DATA3, threshold, x1);
	    }
	    crossings[3][0] = val;
	    crossings[3][1] = y2;
	    crossings[3][2] = z1;
	    break;

	case 4:
	    if (DATA4 == DATA1)
		val = y1;
	    else {
		val = linterp(DATA1, DATA4, threshold, y1);
	    }
	    crossings[4][1] = val;
	    crossings[4][0] = x1;
	    crossings[4][2] = z1;
	    break;
	case 5:
	    if (DATA6 == DATA5)
		val = x1;
	    else {
		val = linterp(DATA5, DATA6, threshold, x1);
	    }
	    crossings[5][0] = val;
	    crossings[5][1] = y1;
	    crossings[5][2] = z2;
	    break;

	case 6:
	    if (DATA7 == DATA6)
		val = y1;
	    else {
		val = linterp(DATA6, DATA7, threshold, y1);
	    }
	    crossings[6][1] = val;
	    crossings[6][0] = x2;
	    crossings[6][2] = z2;
	    break;

	case 7:
	    if (DATA7 == DATA8)
		val = x1;
	    else {
		val = linterp(DATA8, DATA7, threshold, x1);
	    }
	    crossings[7][0] = val;
	    crossings[7][1] = y2;
	    crossings[7][2] = z2;
	    break;

	case 8:
	    if (DATA8 == DATA5)
		val = y1;
	    else {
		val = linterp(DATA5, DATA8, threshold, y1);
	    }
	    crossings[8][1] = val;
	    crossings[8][0] = x1;
	    crossings[8][2] = z2;
	    break;

	case 9:
	    if (DATA5 == DATA1)
		val = z1;
	    else {
		val = linterp(DATA1, DATA5, threshold, z1);
	    }
	    crossings[9][2] = val;
	    crossings[9][1] = y1;
	    crossings[9][0] = x1;
	    break;

	case 10:
	    if (DATA6 == DATA2)
		val = z1;
	    else {
		val = linterp(DATA2, DATA6, threshold, z1);
	    }
	    crossings[10][2] = val;
	    crossings[10][1] = y1;
	    crossings[10][0] = x2;
	    break;

	case 11:
	    if (DATA8 == DATA4)
		val = z1;
	    else {
		val = linterp(DATA4, DATA8, threshold, z1);
	    }
	    crossings[11][2] = val;
	    crossings[11][1] = y2;
	    crossings[11][0] = x1;
	    break;

	case 12:
	    if (DATA7 == DATA3)
		val = z1;
	    else {
		val = linterp(DATA3, DATA7, threshold, z1);
	    }
	    crossings[12][2] = val;
	    crossings[12][1] = y2;
	    crossings[12][0] = x2;
	    break;

	}			/* end switch */
    }				/* end for */

#define TEST_FOR_NEG_VALUES

#ifdef TEST_FOR_NEG_VALUES
    for (i = 0; i < 13; i++) {
	if (crossings[i][0] < 0) {
	    fprintf(stderr,
		    "Neg val(%f) at crossing[%d][0], from loc %d,%d,%d, index=%d, thresh=%d \n",
		    crossings[i][0], i, (int) x1, (int) y1, (int) z1, index, (int) threshold);
	    fprintf(stderr, " DATA values: %f,%f,%f,%f,%f,%f,%f,%f \n",
		    DATA1, DATA2, DATA3, DATA4, DATA5, DATA6, DATA7, DATA8);
	    /* crossings[i][0] = 0.; */
	}
	if (crossings[i][1] < 0) {
	    fprintf(stderr,
		    "Neg val(%f) at crossing[%d][0], from loc %d,%d,%d, index=%d, thresh=%d \n",
		    crossings[i][1], i, (int) x1, (int) y1, (int) z1, index, (int) threshold);
	    fprintf(stderr, " DATA values: %f,%f,%f,%f,%f,%f,%f,%f \n",
		    DATA1, DATA2, DATA3, DATA4, DATA5, DATA6, DATA7, DATA8);
	    /* crossings[i][1] = 0.; */
	}
	if (crossings[i][2] < 0) {
	    fprintf(stderr,
		    "Neg val(%f) at crossing[%d][0], from loc %d,%d,%d, index=%d, thresh=%d \n",
		    crossings[i][2], i, (int) x1, (int) y1, (int) z1, index, (int) threshold);
	    fprintf(stderr, " DATA values: %f,%f,%f,%f,%f,%f,%f,%f \n",
		    DATA1, DATA2, DATA3, DATA4, DATA5, DATA6, DATA7, DATA8);
	    /* crossings[i][2] = 0.; */
	}
    }
#endif

    for (i = 0; i < 13; i++)
	if (crossings[i][0] < 0 || crossings[i][1] < 0 || crossings[i][2] < 0) {
	    Error("negative vertex detected");
	}
    return (0);
}

/**************************** get_cell_polys ****************************/

int
get_cell_polys(index, crossings, x, y, z)
    int       index;
    float     crossings[13][3];
    int       x, y, z;
/* This subroutine will calculate the polygons */
{
    register int num_polys, poly_cnt;
    register int poly, idx;
    float    *p1, *p2, *p3;
    void      add_polygon();

    num_polys = cell_table[index].npolys;	/* possible values: 0 to 10 */

    idx = poly_cnt = 0;
    for (poly = 0; poly < num_polys; poly++) {

	p1 = &crossings[cell_table[index].polys[idx++]][0];
	p2 = &crossings[cell_table[index].polys[idx++]][0];
	p3 = &crossings[cell_table[index].polys[idx++]][0];

/* #define SHOW_TRIANGLES  */
#ifdef SHOW_TRIANGLES
	fprintf(stderr, "triangle from data cube at (%d,%d,%d): \n", x, y, z);
	fprintf(stderr, "  (%.3f,%.3f,%.3f) (%.3f,%.3f,%.3f) (%.3f, %.3f,%.3f) \n",
	     p1[0], p1[1], p1[2], p2[0], p2[1], p2[2], p3[0], p3[1], p3[2]);
#endif

#define CHECK_FOR_DEGENERATE_TRIANGLES

#ifdef CHECK_FOR_DEGENERATE_TRIANGLES
	/*
	 * degenerate triangles are produced when one of the corners of the
	 * cube equals the threshold value, and some renderers don't handle
	 * degenerate triangles well
	 */

	if ((p1[0] == p2[0] && p1[1] == p2[1] && p1[2] == p2[2]) ||
	    (p1[0] == p3[0] && p1[1] == p3[1] && p1[2] == p3[2]) ||
	    (p2[0] == p3[0] && p2[1] == p3[1] && p2[2] == p3[2]));
	else {
#endif
	    /* p1,p2,p3 contain x,y,z values for each vertex of the triangle */
	    add_polygon(p1, p2, p3, x, y, z);
	    poly_cnt++;
#ifdef CHECK_FOR_DEGENERATE_TRIANGLES
	}
#endif
    }

    return (poly_cnt);
}

/********************************************************************/

/* if SMALL_CHUNKS is defined, output will be written whenever the
   buffer is full, otherwise the size of the buffer is increased
 */

VERT_PTR  vertex_list = NULL;
NORM_PTR  norm_list = NULL;
int      *conn_list = NULL;

#define OUTBUF_SIZE 50000

int       VERT_LIMIT = OUTBUF_SIZE;
int       CONN_LIMIT = OUTBUF_SIZE * 3;

#define VERT_INCR 30000		/* size to increase vertex lists when full */

static int num_vertices = 0, num_conn = 0;

/**************************** add_polygon ****************************/

void
add_polygon(p1, p2, p3, x, y, z)
    float    *p1, *p2, *p3;
    int       x, y, z;
{
    int       idx, conn1, conn2, conn3;
    NORMAL_VECT norm, calc_normal();
    int       size;

    if (vertex_list == NULL)
	vertex_list = (VERT_PTR) malloc(sizeof(VERTEX) * VERT_LIMIT);

    if (MARCH_NORMALS) {
	if (norm_list == NULL)
	    norm_list = (NORM_PTR) malloc(sizeof(NORMAL) * VERT_LIMIT);
    }
    if (DUP_CHECK && conn_list == NULL) {
	conn_list = (int *) malloc(sizeof(int) * CONN_LIMIT);
    }
    /* store the vertices */
    if (!DUP_CHECK || (idx = check_for_duplicate_vertex(p1, x, y, z)) < 0) {
	vertex_list[num_vertices].x = p1[0];
	vertex_list[num_vertices].y = p1[1];
	vertex_list[num_vertices].z = p1[2];

	if (MARCH_NORMALS) {
	    if (PRE_NORMALS) {
		norm_list[num_vertices].nx = normals[z][y][x].x;
		norm_list[num_vertices].ny = normals[z][y][x].y;
		norm_list[num_vertices].nz = normals[z][y][x].z;
	    } else {
		norm = calc_normal((int) (p1[0] + .5), (int) (p1[1] + .5),
				   (int) (p1[2] + .5));
		norm_list[num_vertices].nx = norm.x;
		norm_list[num_vertices].ny = norm.y;
		norm_list[num_vertices].nz = norm.z;
	    }
	}
	conn1 = num_vertices++;
    } else {
	conn1 = idx;
    }

    if (!DUP_CHECK || (idx = check_for_duplicate_vertex(p2, x, y, z)) < 0) {
	vertex_list[num_vertices].x = p2[0];
	vertex_list[num_vertices].y = p2[1];
	vertex_list[num_vertices].z = p2[2];

	if (MARCH_NORMALS) {
	    if (PRE_NORMALS) {
		norm_list[num_vertices].nx = normals[z][y][x].x;
		norm_list[num_vertices].ny = normals[z][y][x].y;
		norm_list[num_vertices].nz = normals[z][y][x].z;
	    } else {
		norm = calc_normal((int) (p2[0] + .5), (int) (p2[1] + .5),
				   (int) (p2[2] + .5));
		norm_list[num_vertices].nx = norm.x;
		norm_list[num_vertices].ny = norm.y;
		norm_list[num_vertices].nz = norm.z;
	    }
	}
	conn2 = num_vertices++;
    } else {
	conn2 = idx;
    }

    if (!DUP_CHECK || (idx = check_for_duplicate_vertex(p3, x, y, z)) < 0) {
	vertex_list[num_vertices].x = p3[0];
	vertex_list[num_vertices].y = p3[1];
	vertex_list[num_vertices].z = p3[2];

	if (MARCH_NORMALS) {
	    if (PRE_NORMALS) {
		norm_list[num_vertices].nx = normals[z][y][x].x;
		norm_list[num_vertices].ny = normals[z][y][x].y;
		norm_list[num_vertices].nz = normals[z][y][x].z;
	    } else {
		norm = calc_normal((int) (p3[0] + .5), (int) (p3[1] + .5),
				   (int) (p3[2] + .5));
		norm_list[num_vertices].nx = norm.x;
		norm_list[num_vertices].ny = norm.y;
		norm_list[num_vertices].nz = norm.z;
	    }
	}
	conn3 = num_vertices++;
    } else {
	conn3 = idx;
    }

    if (DUP_CHECK) {
	/* store the connnectity info info */
	if (currslice[y][x].edge_index == -1)
	    currslice[y][x].edge_index = num_conn;
	currslice[y][x].num_edges += 3;

	conn_list[num_conn++] = conn1;
	conn_list[num_conn++] = conn2;
	conn_list[num_conn++] = conn3;
    }
    if (SMALL_CHUNKS) {
	if (num_vertices + 4 >= VERT_LIMIT || num_conn + 3 >= CONN_LIMIT) {
	    dump_polys(1);	/* write polys found so far */

	    if (DUP_CHECK)
		init_dup_vertex_check_arrays();
	}
    } else {
	if (num_vertices + 4 >= VERT_LIMIT) {
	    /* get more space */
	    VERT_LIMIT += VERT_INCR;	/* add this much space */
	    size = VERT_LIMIT * sizeof(VERTEX);	/* size for malloc/realloc */
	    if ((vertex_list = (VERT_PTR) realloc((char *) vertex_list,
						  size)) == NULL) {
		Error("not enough memory to store vertices");
	    }
	    if (MARCH_NORMALS) {
		size = VERT_LIMIT * sizeof(NORMAL);	/* size for
							 * malloc/realloc */
		if ((norm_list = (NORM_PTR) realloc((char *) norm_list,
						    size)) == NULL) {
		    Error("not enough memory to store normals");
		}
	    }
	}
	if (DUP_CHECK && num_conn + 3 >= CONN_LIMIT) {
	    CONN_LIMIT += VERT_INCR * 3;	/* add this much space */
	    size = CONN_LIMIT * sizeof(int);
	    if ((conn_list = (int *) realloc((char *) conn_list,
					     size)) == NULL) {
		Error("not enough memory to store connectivity");
	    }
	}
    }
}

/***********  check for duplicate vertices  *******************************/
int
check_for_duplicate_vertex(vert, x, y, z)
    float     vert[3];		/* vertex location */
    int       x, y, z;		/* location in original data set */
{
    int       idx;


    /* first, check for duplicate vertices from the same cube */
    if ((idx = search_for_dup(vert, currslice[y][x].num_edges,
			      currslice[y][x].edge_index)) >= 0)
	return (idx);

    if (x > 0) {
	if ((idx = search_for_dup(vert, currslice[y][x - 1].num_edges,
				  currslice[y][x - 1].edge_index)) >= 0)
	    return (idx);
    }
    if (y > 0) {
	if ((idx = search_for_dup(vert, currslice[y - 1][x].num_edges,
				  currslice[y - 1][x].edge_index)) >= 0)
	    return (idx);
    }
    if (z > 0) {
	if ((idx = search_for_dup(vert, prevslice[y][x].num_edges,
				  prevslice[y][x].edge_index)) >= 0)
	    return (idx);
    }
    return (-1);
}

/***************************************************************/
int
search_for_dup(vert, num_to_check, conn_idx)
    float     vert[3];		/* vertex location */
    int       num_to_check;	/* number of vertices in this cube */
    int       conn_idx;		/* index into connectivity list */
{
    register int i, idx;
    static int dup_cnt = 0;

    if (conn_idx == -1)
	return (-1);

    if (num_to_check == -1) {	/* for testing */
	fprintf(stderr, "Found %d duplicate vertices \n", dup_cnt);
	dup_cnt = 0;
	return (-1);
    }
    for (i = 0; i < num_to_check; i++) {
	idx = conn_list[conn_idx];
	if (idx > num_vertices) {	/* seems to need this check, but I
					 * dont know why? */
	    fprintf(stderr, "Error checking dup vertex: conn_idx = %d, num_verts = %d \n",
		    conn_idx, num_vertices);
	    return (-1);
	}
	if (vertex_list[idx].x == vert[0] && vertex_list[idx].y == vert[1] &&
	    vertex_list[idx].z == vert[2]) {
	    dup_cnt++;
	    return (idx);
	}
	conn_idx++;
    }
    return (-1);		/* none found */
}

/**************************** dump_polys ****************************/
int
dump_polys(more)
    int       more;
{
    void      write_polys_to_socket();

    if (VERBOSE2)
	Status("Writing triangles to file/socket..");

    if (SERVER) {
	write_polys_to_socket(vertex_list, norm_list, conn_list,
			      num_vertices, num_conn, more);
    } else if (OUTPUT_TYPE == 0) {
	write_polys(vertex_list, norm_list, conn_list, num_vertices,
		    num_conn, more);
    } else if (OUTPUT_TYPE == 1) {
	write_polys_ascii(vertex_list, norm_list, conn_list,
			  num_vertices, num_conn);
    } else if (OUTPUT_TYPE == 2) {
	write_polys_movie_byu(vertex_list, conn_list, num_vertices, num_conn);
    } else if (OUTPUT_TYPE == 3) {
	write_polys_wavefront(vertex_list, norm_list, conn_list,
			      num_vertices, num_conn);
    } else if (OUTPUT_TYPE == 4) {
	write_polys_sunvision(vertex_list, norm_list, conn_list,
			      num_vertices, num_conn);
    }
    num_vertices = num_conn = 0;

    if (VERBOSE)
	search_for_dup(0, -1, 0);	/* prints diag message */

    return;
}
