
/* file_io.c               -Brian Tierney,  LBL   12/90  */

/* This file contains code for read HIPS files to be used with isovis,
   as well as the routine to output ASCII and binary 
   formated vertex/triangle lists
 */

/* $Id: file_io.c,v 1.4 1992/01/31 02:05:45 tierney Exp $ */

/* $Log: file_io.c,v $
 * Revision 1.4  1992/01/31  02:05:45  tierney
 * *** empty log message ***
 *
 * Revision 1.3  1992/01/30  20:05:03  davidr
 * prior to Brian's changes
 *
 * Revision 1.2  1991/12/19  01:42:20  davidr
 * added RCS identification markers
 * */

static char rcsid[] = "$Id: file_io.c,v 1.4 1992/01/31 02:05:45 tierney Exp $" ;

#include "isobuild.h"

static struct header hd;

/**************************** get_data ****************************/

int
get_size(fp)
    FILE     *fp;
/* This subroutine will read in the data file. */
{
    /* read HIPS header */
    fread_header(fp, &hd, IN_FNAME);

#ifndef FLOAT_INPUT
    if (hd.pixel_format != PFBYTE) {
	Status("Error: pixel format must be byte.");
	return (-1);
    }
#else
    if (hd.pixel_format != PFFLOAT && hd.pixel_format != PFSHORT &&
	hd.pixel_format != PFINT && hd.pixel_format != PFBYTE) {
	Status("Error: pixel format must be float, int, short, or byte.");
	return (-1);
    }
#endif

    zdim = hd.num_frame;
    ydim = hd.orows;
    xdim = hd.ocols;

    /* setup globals */
    xmax = xdim - 1;
    ymax = ydim - 1;
    zmax = zdim - 1;

    if (VERBOSE)
	fprintf(stderr, "%s: data set size xdim=%d ydim=%d zdim=%d\n",
		MY_NAME, xdim, ydim, zdim);

    return (0);
}

/***************************************************************/
int
read_hips_data(fp, in_data, size)
    FILE     *fp;
    Data_type ***in_data;
    int       size;
{

#ifndef FLOAT_INPUT
    fprintf(stderr, "%s: Reading data file %s ... \n", MY_NAME, IN_FNAME);

    if (Fread(in_data[0][0], sizeof(Data_type), size, fp) != size) {
	perror("\n error reading file\n");
	return (-1);
    }
#else
    register int i;
    unsigned char cval;
    short     sval;
    int       ival;

    fprintf(stderr, "%s: Reading data file %s ... \n", MY_NAME, IN_FNAME);

    if (hd.pixel_format == PFFLOAT) {
	if (Fread(in_data[0][0], sizeof(float), size, fp) != size) {
	    perror("\n error reading file\n");
	    return (-1);
	}
    } else {
	Status("Converting input data set to floating point...");
	if (hd.pixel_format == PFBYTE) {
	    for (i = 0; i < size; i++) {
		if (Fread(&cval, sizeof(unsigned char), 1, fp) != 1) {
		    perror("\n error reading file\n");
		    exit(-1);
		}
		in_data[0][0][i] = (float) cval;
	    }
	} else if (hd.pixel_format == PFSHORT) {
	    for (i = 0; i < size; i++) {
		if (Fread(&sval, sizeof(short), 1, fp) != 1) {
		    perror("\n error reading file\n");
		    exit(-1);
		}
		in_data[0][0][i] = (float) sval;
	    }
	} else if (hd.pixel_format == PFINT) {
	    for (i = 0; i < size; i++) {
		if (Fread(&ival, sizeof(int), 1, fp) != 1) {
		    perror("\n error reading file\n");
		    exit(-1);
		}
		in_data[0][0][i] = (float) ival;
	    }
	}
	Status("Conversion done.");
    }
#endif

    return (0);
}

/***************************************************************/
int
read_mask_file(size)		/* load an input segmentation mask */
    int       size;
{
    Grid_type *grid_ptr;
    register int i;

    if (fread_header(in_mask_fp, &hd, IN_MASK_FNAME) != 0) {
	Status("Error reading hips header.");
	return (-1);
    }
    if (hd.pixel_format != PFBYTE) {
	Status("Error: mask pixel format must be byte.");
	return (-1);
    }
    if (hd.num_frame != zdim || hd.orows != ydim || hd.ocols != xdim) {
	Status("Error: mask and data must be the same size.");
	return (-1);
    }
    fprintf(stderr, "%s: Reading mask file %s ... \n", MY_NAME, IN_MASK_FNAME);

    if (Fread(grid[0][0], sizeof(Grid_type), size, in_mask_fp) != size) {
	perror("error reading file");
	return (-1);
    }

    /* iso_march expects mask to contain 0's and 1's only */
    grid_ptr = grid[0][0];
    for (i = 0; i < xdim * ydim * zdim; i++) {
	if (grid_ptr[i] > 0)
	    grid_ptr[i] = 1;
    }
    return (1);
}

/***************************************************************/
int
write_points(point_list, npoints, more)
    POINT_PTR point_list;
    int       npoints, more;
{
    int       h_buf[4];
    static int tot_points = 0;

/*
 * Header: more flag:  1 = MAY be more vertices to follow
 * npoints: number of vertices
*/
    if (tot_points == 0) {	/* write size header first time */
	h_buf[0] = xdim;
	h_buf[1] = ydim;
	h_buf[2] = zdim;
	h_buf[3] = 0;
	if (fwrite((char *) h_buf, sizeof(int), 4, poly_fp) != 4)
	    perror("error writing header");
    }
    tot_points += npoints;
    if (!more) {
	fprintf(stderr, "Writing %d points.\n", tot_points);
	tot_points = 0;
    }
    if (fwrite(&more, sizeof(int), 1, poly_fp) != 1)
	perror("error writing more");
    if (fwrite(&npoints, sizeof(int), 1, poly_fp) != 1)
	perror("error writing npoints");

    if (npoints <= 0)
	return 0;

    if (fwrite((char *) point_list, sizeof(POINT), npoints, poly_fp) != npoints)
	perror("error writing output_buf");

    return 0;
}

/***************************************************************/
int
write_polys(vert_list, norm_list, conn_list, nverts, nconn, more)
    VERT_PTR  vert_list;
    NORM_PTR  norm_list;
    int      *conn_list;
    int       nverts, nconn, more;
{
    int       norm_flag;
    int       h_buf[4];
    static int tot_verts = 0;

/*
 * Header: more flag:  1 = MAY be more vertices to follow
 * npoints: number of vertices
*/
    if (tot_verts == 0) {	/* write size header first time */
	h_buf[0] = xdim;
	h_buf[1] = ydim;
	h_buf[2] = zdim;
	h_buf[3] = 0;
	if (fwrite((char *) h_buf, sizeof(int), 4, poly_fp) != 4)
	    perror("error writing header");
    }

    tot_verts += nverts;
    if (!more) {
	fprintf(stderr, "Writing %d vertices.\n", tot_verts);
	tot_verts = 0;
    }
    if (fwrite(&more, sizeof(int), 1, poly_fp) != 1)
	perror("error writing more flag");
    if (fwrite(&nverts, sizeof(int), 1, poly_fp) != 1)
	perror("error writing nverts");
    if (fwrite(&nconn, sizeof(int), 1, poly_fp) != 1)
	perror("error writing nconn");
    if (MARCH_NORMALS)
	norm_flag = 1;
    else
	norm_flag = 0;
    if (fwrite(&norm_flag, sizeof(int), 1, poly_fp) != 1)
	perror("error writing nconn");

    if (nverts <= 0)
	return 0;

    /* write all vertices */
    if (fwrite((char *) vert_list, sizeof(VERTEX), nverts, poly_fp) != nverts)
	perror("error writing vertex list");

    if (MARCH_NORMALS) {
	/* write all normals */
	if (fwrite((char *) norm_list, sizeof(NORMAL), nverts, poly_fp) != nverts)
	    perror("error writing normal list");
    }
    if (nconn > 0) {
	/* write connectivity list */
	if (fwrite((char *) conn_list, sizeof(int), nconn, poly_fp) != nconn)
	    perror("error writing connectivity list");
    }
    return 0;
}

/***************************************************************/
int
write_polys_movie_byu(vert_list, conn_list, nverts, nconn)
    VERT_PTR  vert_list;
    int      *conn_list;
    int       nverts, nconn;
{
    int       val, i, npolys;

/* movie BYU header
    1: number of parts or objects
    2: number of vertices
    3: number of polygons
    4: number of edges

  move BYU data
    1: parts array (starting and ending locations of objects)
    2: vertices data (x,y,z order, floating point)
    3: edge data, negative # to indicate end of polygon, type int
 */
    fprintf(stderr, "Writing movie BYU file (%d vertices) ... \n", nverts);

    val = 1;			/* only 1 object */
    npolys = (nconn / 3);	/* this is the number of triangles */
    fprintf(poly_fp, "%d %d %d %d \n", val, nverts, npolys, nconn);

    /*
     * parts array just constists of 2 numbers, the upper and lower bounds of
     * the vertex list (in this case: 1 and # polygons )
     */
    fprintf(poly_fp, "%d %d \n", val, npolys);

    if (nverts <= 0)
	return 0;

    /* write all vertices */
    for (i = 0; i < nverts; i++) {
	fprintf(poly_fp, "%f %f %f \n", vert_list[i].x, vert_list[i].y,
		vert_list[i].z);
    }

    i = 0;
    while (i < nconn) {
	/*
	 * for byu format, indices start at 1, and last value for each
	 * polygon is negative
	 */
	fprintf(poly_fp, "%d %d %d \n", conn_list[i++] + 1, conn_list[i++] + 1,
		-(conn_list[i++] + 1));
    }

    return 0;
}

/***************************************************************/
int
write_polys_sunvision(vert_list, norm_list, conn_list, nverts, nconn)
    VERT_PTR  vert_list;
    NORM_PTR  norm_list;
    int      *conn_list;
    int       nverts, nconn;
{
    /* this routine written by Wing Nip, LBL */

    int       i;
    float     min_in, max_in;
    float     min_out = -1.0;
    float     max_out = 1.0;
    float     scale;
    float     x, y, z;

    /* normalize vertices between -1.0 & 1.0 */
    max_in = -1.0;
    min_in = vert_list[0].x;	/* arbitrary assign a min value */

    for (i = 0; i < nverts; i++) {  /* find min and max values */
	if (vert_list[i].x > max_in)
	    max_in = vert_list[i].x;
	if (vert_list[i].y > max_in)
	    max_in = vert_list[i].y;
	if (vert_list[i].z > max_in)
	    max_in = vert_list[i].z;
	if (vert_list[i].x < min_in)
	    min_in = vert_list[i].x;
	if (vert_list[i].y < min_in)
	    min_in = vert_list[i].y;
	if (vert_list[i].z < min_in)
	    min_in = vert_list[i].z;
    }
    scale = (max_out - min_out) / (max_in - min_in);
    fprintf(stderr, "Writing sunvision vff file(%d vertices) ... \n", nverts);
    fprintf(poly_fp, "ncaa \n");
    fprintf(poly_fp, "type=vertices; \n");
    fprintf(poly_fp, "components=x y z normal_x normal_y normal_z; \n");
    fprintf(poly_fp, "{ \n");

    /* write all vertices */
    for (i = 0; i < nverts; i++) {
	fprintf(poly_fp, "     {");
	x = (vert_list[i].x - min_in) * scale + min_out;
	y = (vert_list[i].y - min_in) * scale + min_out;
	z = (vert_list[i].z - min_in) * scale + min_out;

	fprintf(poly_fp, "%f, %f, %f", x, y, z);

	/* normal */
	if (MARCH_NORMALS) {
	    fprintf(poly_fp, " %f, %f, %f,", (float) norm_list[i].nx,
		    (float) norm_list[i].ny, (float) norm_list[i].nz);
	} else {
	    fprintf(poly_fp, " 1.0, 1.0, 1.0");  /* flat shading */
	}
	fprintf(poly_fp, "},\n");
    }

    fprintf(poly_fp, "}\n\n");
    fprintf(poly_fp, "type=connectivity;\n");
    fprintf(poly_fp, "components=variable.i;\n");
    fprintf(poly_fp, "{\n");
    i = 0;
    while (i < nconn) {
	fprintf(poly_fp, "     {");
	fprintf(poly_fp, "%d, %d, %d,}, \n", conn_list[i++],
		conn_list[i++], conn_list[i++]);
    }
    fprintf(poly_fp, "}\n");
    return 0;
}

/***************************************************************/
int
write_polys_wavefront(vert_list, norm_list, conn_list, nverts, nconn)
    VERT_PTR  vert_list;
    NORM_PTR  norm_list;
    int      *conn_list;
    int       nverts, nconn;
{
    int       i, f1, f2, f3;

    fprintf(stderr, "Writing wavefront formatted file (%d vertices) ... \n",
	    nverts);

    fprintf(poly_fp, "g %s \n", IN_FNAME);	/* group name */

    if (nverts <= 0)
	return 0;

    /* write all vertices */
    for (i = 0; i < nverts; i++) {
	fprintf(poly_fp, "v %f %f %f \n", vert_list[i].x, vert_list[i].y,
		vert_list[i].z);
    }
    if (MARCH_NORMALS) {
	/* write all normals */
	for (i = 0; i < nverts; i++) {
	    fprintf(poly_fp, "vn: %f %f %f \n",
		    (float) norm_list[i].nx, (float) norm_list[i].ny,
		    (float) norm_list[i].nz);
	}
    }
    i = 0;
    while (i < nconn) {
	f1 = conn_list[i++] + 1;
	f2 = conn_list[i++] + 1;
	f3 = conn_list[i++] + 1;

	if (MARCH_NORMALS)
	    fprintf(poly_fp, "f %d//%d %d//%d %d//%d \n",
		    f1, f1, f2, f2, f3, f3);
	else
	    fprintf(poly_fp, "f %d %d %d \n", f1, f2, f3);
    }
    fprintf(poly_fp, "\n");

    return 0;
}

/****************************************************************/
int
write_points_ascii(point_list, npoints)
    POINT_PTR point_list;
    int       npoints;
/* This subroutine writes out an ascii file of the polygons */
{
    register int i;

    fprintf(poly_fp, "num points: %d \n", npoints);

    /* write out all of the vertices */
    for (i = 0; i < npoints; i++) {
	if (fprintf(poly_fp, "vert: %.3f %.3f %.3f \n",
		    (float) point_list[i].x, (float) point_list[i].y,
		    (float) point_list[i].z) == -1) {
	    Error("writing to ascii point file.");
	}
	if (fprintf(poly_fp, "   norm: %.3f %.3f %.3f \n",
		    (float) point_list[i].nx, (float) point_list[i].ny,
		    (float) point_list[i].nz) == -1) {
	    Error("writing to ascii point file.");
	}
    }

    return 0;
}

/****************************************************************/
int
write_polys_ascii(vert_list, norm_list, conn_list, nverts, nconn)
    VERT_PTR  vert_list;
    NORM_PTR  norm_list;
    int      *conn_list;
    int       nverts, nconn;
/* This subroutine writes out an ascii file of the polygons */
{
    register int i;

    fprintf(poly_fp, "Num verts: %d,  num_edges: %d \n", nverts, nconn);

    /* write out all of the vertices */
    for (i = 0; i < nverts; i++) {
	if (fprintf(poly_fp, "vert: %.3f %.3f %.3f \n",
		    (float) vert_list[i].x, (float) vert_list[i].y,
		    (float) vert_list[i].z) == -1) {
	    Error("writing to ascii polygon file.");
	}
	if (MARCH_NORMALS) {
	    if (fprintf(poly_fp, "   norm: %.3f %.3f %.3f \n",
			(float) norm_list[i].nx, (float) norm_list[i].ny,
			(float) norm_list[i].nz) == -1) {
		Error("writing to ascii polygon file.");
	    }
	}
    }

    i = 0;
    while (i < nconn) {
	if (fprintf(poly_fp, "edge: %d %d %d \n",
		    conn_list[i++], conn_list[i++],
		    conn_list[i++]) == -1) {
	    Error("writing to ascii polygon file.");
	}
    }

    return 0;
}

/**************************************************************/
void
exit_handler()
{
    if (SERVER)
	fprintf(stderr, "Caught signal from %s: Exiting... \n\n", MY_NAME);
    else
	fprintf(stderr, "%s: Exiting... \n\n", MY_NAME);

    if (grid != NULL)
	free_3d_grid_array(grid);

    if (data != NULL)
	free_3d_data_array(data);

    if (normals != NULL)
	free_3d_normal_array(normals);

    if (block_info_array != NULL)
	free_block_info_array(block_info_array);

    exit(-1);
}
