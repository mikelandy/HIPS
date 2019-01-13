
/* normal.c              -Brian Tierney, LBL        1/91  */

/*  for use with isobuild:  computes normal vectors
 *
 *  There are 2 methods: 1 is to compute all normals ahead of time
 *  and store them in an array.  This method is MUCH faster for the
 *  server version, but uses LOTS of memory.  The other method is
 *  to compute them as needed.
 *
 *  2 types of normal vectors are supported, and which type is controlled
 *  by some #defines in isobuild.h
 *
 *  These types are:
 *     1) default: normals will be in the range -1.0 to 1.0
 *
 *     2) #define SHORT_NORMALS :  normals will be in the range
 *		 -32767 to 32767 (short ints)  (or equal to the
 *			range of the input data if input is type float)
 *
 *    It is not allowed to define both of these at the same time!
 */

/* $Id: normal.c,v 1.4 1992/01/31 02:05:45 tierney Exp $ */

/* $Log: normal.c,v $
 * Revision 1.4  1992/01/31  02:05:45  tierney
 * *** empty log message ***
 *
 * Revision 1.3  1992/01/30  20:05:03  davidr
 * prior to Brian's changes
 *
 * Revision 1.2  1991/12/19  01:42:20  davidr
 * added RCS identification markers
 * */

static char rcsid[] = "$Id: normal.c,v 1.4 1992/01/31 02:05:45 tierney Exp $" ;

#include "isobuild.h"


/********************************************************************/
void
compute_normal_array()
{				/* pre-compute all normals */
    register int i, j, k;
    NORMAL_VECT calc_normal();

    Status("Pre-Computing normals..");

    memset(gradient_curr_slice[0], 0, sizeof(FLOAT_VECT) * xdim * ydim);
    memset(gradient_next_slice[0], 0, sizeof(FLOAT_VECT) * xdim * ydim);

    for (i = 0; i < zmax; i++) {
	if (i % 10 == 0)
	    fprintf(stderr, "Computing normals: slice %d...\n", i);

	for (j = 0; j < ymax; j++)
	    for (k = 0; k < xmax; k++)
		normals[i][j][k] = calc_normal(k, j, i);
    }

    Status("Normals computed.");
}

/**************************** calc_normal ****************************/

NORMAL_VECT
calc_normal(x, y, z)
    int       x, y, z;		/* location in original data set */
{
    static int prev_z = 0;
    FLOAT_VECT **tmp_ptr, fvect, get_gradient();
    NORMAL_VECT norm;		/* normals (returned by this routine) */
    NORMAL_VECT normalize_vect();
    FLOAT_VECT compute_cube_center_normal();
    NORMAL_VECT normal_roundoff();


    if (DIVIDING && DO_CUBE_CENTER_NORMALS) {
	if (z != prev_z) {
	    tmp_ptr = gradient_curr_slice;
	    gradient_curr_slice = gradient_next_slice;
	    gradient_next_slice = tmp_ptr;
	    memset(gradient_next_slice[0], 0, sizeof(FLOAT_VECT) * xdim * ydim);
	    prev_z = z;
	}
	/* only compute gradient if it hasn't already been computed */
	/*
	 * Actually: this method will re-compute the gradient if the previous
	 * gradient value was zero, but this doesnt happen often
	 */
	if (gradient_curr_slice[y][x].z == 0.)
	    gradient_curr_slice[y][x] = get_gradient(x, y, z);
	if (gradient_curr_slice[y][x + 1].z == 0.)
	    gradient_curr_slice[y][x + 1] = get_gradient(x + 1, y, z);
	if (gradient_curr_slice[y + 1][x].z == 0.)
	    gradient_curr_slice[y + 1][x] = get_gradient(x, y + 1, z);
	if (gradient_curr_slice[y + 1][x + 1].z == 0.)
	    gradient_curr_slice[y + 1][x + 1] = get_gradient(x + 1, y + 1, z);
	/* compute values for the next slice */
	if (gradient_next_slice[y][x].z == 0.)
	    gradient_next_slice[y][x] = get_gradient(x, y, z + 1);
	if (gradient_next_slice[y][x + 1].z == 0.)
	    gradient_next_slice[y][x + 1] = get_gradient(x + 1, y, z + 1);
	/* these 2 will have never been computed already */
	gradient_next_slice[y + 1][x] = get_gradient(x, y + 1, z + 1);
	gradient_next_slice[y + 1][x + 1] = get_gradient(x + 1, y + 1, z + 1);

	fvect = compute_cube_center_normal(x, y, z);

    } else {
	fvect = get_gradient(x, y, z);
    }

    norm = normalize_vect(fvect);

    return (norm);
}

/******************************************************/
NORMAL_VECT
normalize_vect(vect)
    FLOAT_VECT vect;
{
    double    sum, mag;
    NORMAL_VECT norm;

    sum = (vect.x * vect.x) + (vect.y * vect.y) + (vect.z * vect.z);
    mag = sqrt(sum + 0.0000001);

#ifdef SHORT_NORMALS
    norm.x = (short) ((vect.x / mag) * 32767.0);
    norm.y = (short) ((vect.y / mag) * 32767.0);
    norm.z = (short) ((vect.z / mag) * 32767.0);
#else
    norm.x = (Norm_type) (vect.x / mag);
    norm.y = (Norm_type) (vect.y / mag);
    norm.z = (Norm_type) (vect.z / mag);
#endif

    return (norm);
}

/******************************************************************/
FLOAT_VECT
get_gradient(x, y, z)
    int       x, y, z;
/* This routine computes the gradient at location x,y,z */
{
    /* compute gradients using center differences */

    FLOAT_VECT grad;
    Data_type d1, d2, val;

/*#define THIN_OBJ_CHECK doesn't really seem to help */

#define AVERAGE(d1,d2) (((float)d1 - (float) d2 ) * .5)
#define MINABS(A,B)  ((fabs(A)) < (fabs(B)) ? (A) : (B))

#ifdef FLOAT_INPUT
    if (x == 0)			/* use edge value instead */
	grad.x = (data[z][y][x + 1] - data[z][y][x]) / 2.;
    else if (x >= xmax)
	grad.x = (data[z][y][x] - data[z][y][x - 1]) / 2.;
    else
	grad.x = (data[z][y][x + 1] - data[z][y][x - 1]) / 2.;

    if (y == 0)
	grad.y = (data[z][y + 1][x] - data[z][y][x]) / 2.;
    else if (y >= ymax)
	grad.y = (data[z][y][x] - data[z][y - 1][x]) / 2.;
    else
	grad.y = (data[z][y + 1][x] - data[z][y - 1][x]) / 2.;

    if (z == 0)
	tmpz = (data[z + 1][y][x] - data[z][y][x]) / 2.;
    else if (z >= zmax)
	tmpz = (data[z][y][x] - data[z - 1][y][x]) / 2.;
    else
	tmpz = (data[z + 1][y][x] - data[z - 1][y][x]) / 2.;

#else

    /* cast data to float to maintain more precision */
    val = data[z][y][x];

    if (x == 0) {
	d1 = data[z][y][x + 1];
	d2 = val;
    } else if (x >= xmax) {
	d1 = val;
	d2 = data[z][y][x - 1];
    } else {
	d1 = data[z][y][x + 1];
	d2 = data[z][y][x - 1];
    }
#ifdef THIN_OBJ_CHECK
    if ((d1 > val && d2 > val) || (d1 < val && d2 < val)) {
	/* checks for 'thin' objects and computes gradient differently */
	grad.x = MINABS(AVERAGE(d1, val), AVERAGE(d2, val));
    } else
#endif
	grad.x = AVERAGE(d1, d2);

    if (y == 0) {
	d1 = data[z][y + 1][x];
	d2 = val;
    } else if (y >= ymax) {
	d1 = val;
	d2 = data[z][y - 1][x];
    } else {
	d1 = data[z][y + 1][x];
	d2 = data[z][y - 1][x];
    }
#ifdef THIN_OBJ_CHECK
    if ((d1 > val && d2 > val) || (d1 < val && d2 < val)) {
	grad.y = MINABS(AVERAGE(d1, val), AVERAGE(d2, val));
    } else
#endif
	grad.y = AVERAGE(d1, d2);

    if (z == 0) {
	d1 = data[z + 1][y][x];
	d2 = val;
    } else if (z >= zmax) {
	d1 = val;
	d2 = data[z - 1][y][x];
    } else {
	d1 = data[z + 1][y][x];
	d2 = data[z - 1][y][x];
    }
#ifdef THIN_OBJ_CHECK
    if ((d1 > val && d2 > val) || (d1 < val && d2 < val)) {
	grad.z = MINABS(AVERAGE(d1, val), AVERAGE(d2, val));
    } else
#endif
	grad.z = AVERAGE(d1, d2);

#endif

    return (grad);
}

/********************************************************************/
FLOAT_VECT
compute_cube_center_normal(x, y, z)
    int       x, y, z;
{
    FLOAT_VECT norm_vect;

    if (z < zmax) {
	norm_vect.x = (.125 * (gradient_curr_slice[y][x].x +
			       gradient_curr_slice[y][x + 1].x +
			       gradient_curr_slice[y + 1][x].x +
			       gradient_curr_slice[y + 1][x + 1].x +
			       gradient_next_slice[y][x].x +
			       gradient_next_slice[y][x + 1].x +
			       gradient_next_slice[y + 1][x].x +
			       gradient_next_slice[y + 1][x + 1].x));

	norm_vect.y = (.125 * (gradient_curr_slice[y][x].y +
			       gradient_curr_slice[y][x + 1].y +
			       gradient_curr_slice[y + 1][x].y +
			       gradient_curr_slice[y + 1][x + 1].y +
			       gradient_next_slice[y][x].y +
			       gradient_next_slice[y][x + 1].y +
			       gradient_next_slice[y + 1][x].y +
			       gradient_next_slice[y + 1][x + 1].y));

	norm_vect.z = (.125 * (gradient_curr_slice[y][x].z +
			       gradient_curr_slice[y][x + 1].z +
			       gradient_curr_slice[y + 1][x].z +
			       gradient_curr_slice[y + 1][x + 1].z +
			       gradient_next_slice[y][x].z +
			       gradient_next_slice[y][x + 1].z +
			       gradient_next_slice[y + 1][x].z +
			       gradient_next_slice[y + 1][x + 1].z));
    } else {
	norm_vect.x = (.25 * (gradient_curr_slice[y][x].x +
			      gradient_curr_slice[y][x + 1].x +
			      gradient_curr_slice[y + 1][x].x +
			      gradient_curr_slice[y + 1][x + 1].x));

	norm_vect.y = (.25 * (gradient_curr_slice[y][x].y +
			      gradient_curr_slice[y][x + 1].y +
			      gradient_curr_slice[y + 1][x].y +
			      gradient_curr_slice[y + 1][x + 1].y));

	norm_vect.z = (.25 * (gradient_curr_slice[y][x].z +
			      gradient_curr_slice[y][x + 1].z +
			      gradient_curr_slice[y + 1][x].z +
			      gradient_curr_slice[y + 1][x + 1].z));
    }
    return (norm_vect);
}

/********************************************************************/
FLOAT_VECT
compute_cube_center_normal_old(x, y, z)
    int       x, y, z;
{
    FLOAT_VECT norm_vect;

    if (z < zmax) {
	norm_vect.x = (.125 * (normals[z][y][x].x + normals[z][y][x + 1].x +
		       normals[z][y + 1][x].x + normals[z][y + 1][x + 1].x +
		       normals[z + 1][y][x].x + normals[z + 1][y][x + 1].x +
	      normals[z + 1][y + 1][x].x + normals[z + 1][y + 1][x + 1].x));

	norm_vect.y = (.125 * (normals[z][y][x].y + normals[z][y][x + 1].y +
		       normals[z][y + 1][x].y + normals[z][y + 1][x + 1].y +
		       normals[z + 1][y][x].y + normals[z + 1][y][x + 1].y +
	      normals[z + 1][y + 1][x].y + normals[z + 1][y + 1][x + 1].y));

	norm_vect.z = (.125 * (normals[z][y][x].z + normals[z][y][x + 1].z +
		       normals[z][y + 1][x].z + normals[z][y + 1][x + 1].z +
		       normals[z + 1][y][x].z + normals[z + 1][y][x + 1].z +
	      normals[z + 1][y + 1][x].z + normals[z + 1][y + 1][x + 1].z));
    } else {
	norm_vect.x = (.25 * (normals[z][y][x].x + normals[z][y][x + 1].x +
		      normals[z][y + 1][x].x + normals[z][y + 1][x + 1].x));

	norm_vect.y = (.25 * (normals[z][y][x].y + normals[z][y][x + 1].y +
		      normals[z][y + 1][x].y + normals[z][y + 1][x + 1].y));

	norm_vect.z = (.25 * (normals[z][y][x].z + normals[z][y][x + 1].z +
		      normals[z][y + 1][x].z + normals[z][y + 1][x + 1].z));
    }
    return (norm_vect);
}


/***********************************************************/

norm_debug(x, y, z)
    int       x, y, z;
{
    FLOAT_VECT grad1, grad2, grad3, grad4, grad5, grad6, grad7, grad8;
    NORMAL_VECT norm_vect;
    NORMAL_VECT normalize_vect();
    FLOAT_VECT get_gradient();
    int       sx, sy, sz;

    sx = x;
    sy = y;
    sz = z;

    /**************/
    fprintf(stderr, "This point: (%d,%d,%d) = %d \n", x, y, z, data[z][y][x]);
    fprintf(stderr, "     (%d,%d,%d) = %d ; (%d,%d,%d) = %d \n",
	    x + 1, y, z, data[z][y][x + 1], x - 1, y, z, data[z][y][x - 1]);
    fprintf(stderr, "     (%d,%d,%d) = %d ; (%d,%d,%d) = %d \n",
	    x, y + 1, z, data[z][y + 1][x], x, y - 1, z, data[z][y - 1][x]);
    fprintf(stderr, "     (%d,%d,%d) = %d ; (%d,%d,%d) = %d \n",
	    x, y, z + 1, data[z + 1][y][x], x, y, z - 1, data[z - 1][y][x]);
    grad1 = get_gradient(x, y, z);
    fprintf(stderr, "  gradient: gx = %f, gy = %f, gz = %f \n",
	    grad1.x, grad1.y, grad1.z);

    /**************/
    x = sx + 1;
    y = sy;
    fprintf(stderr, "next point, this slice: (%d,%d,%d) = %d \n", x, y, z,
	    data[z][y][x]);
    fprintf(stderr, "     (%d,%d,%d) = %d ; (%d,%d,%d) = %d \n",
	    x + 1, y, z, data[z][y][x + 1], x - 1, y, z, data[z][y][x - 1]);
    fprintf(stderr, "     (%d,%d,%d) = %d ; (%d,%d,%d) = %d \n",
	    x, y + 1, z, data[z][y + 1][x], x, y - 1, z, data[z][y - 1][x]);
    fprintf(stderr, "     (%d,%d,%d) = %d ; (%d,%d,%d) = %d \n",
	    x, y, z + 1, data[z + 1][y][x], x, y, z - 1, data[z - 1][y][x]);
    grad2 = get_gradient(x, y, z);
    fprintf(stderr, "  gradient: gx = %f, gy = %f, gz = %f \n",
	    grad2.x, grad2.y, grad2.z);

    /**************/
    x = sx;
    y = sy + 1;
    fprintf(stderr, "next point, this slice: (%d,%d,%d) = %d \n", x, y, z,
	    data[z][y][x]);
    fprintf(stderr, "     (%d,%d,%d) = %d ; (%d,%d,%d) = %d \n",
	    x + 1, y, z, data[z][y][x + 1], x - 1, y, z, data[z][y][x - 1]);
    fprintf(stderr, "     (%d,%d,%d) = %d ; (%d,%d,%d) = %d \n",
	    x, y + 1, z, data[z][y + 1][x], x, y - 1, z, data[z][y - 1][x]);
    fprintf(stderr, "     (%d,%d,%d) = %d ; (%d,%d,%d) = %d \n",
	    x, y, z + 1, data[z + 1][y][x], x, y, z - 1, data[z - 1][y][x]);
    grad3 = get_gradient(x, y, z);
    fprintf(stderr, "  gradient: gx = %f, gy = %f, gz = %f \n",
	    grad3.x, grad3.y, grad3.z);

    /**************/
    x = sx + 1;
    y = sy + 1;
    fprintf(stderr, "next point, this slice: (%d,%d,%d) = %d \n", x, y, z,
	    data[z][y][x]);
    fprintf(stderr, "     (%d,%d,%d) = %d ; (%d,%d,%d) = %d \n",
	    x + 1, y, z, data[z][y][x + 1], x - 1, y, z, data[z][y][x - 1]);
    fprintf(stderr, "     (%d,%d,%d) = %d ; (%d,%d,%d) = %d \n",
	    x, y + 1, z, data[z][y + 1][x], x, y - 1, z, data[z][y - 1][x]);
    fprintf(stderr, "     (%d,%d,%d) = %d ; (%d,%d,%d) = %d \n",
	    x, y, z + 1, data[z + 1][y][x], x, y, z - 1, data[z - 1][y][x]);
    grad4 = get_gradient(x, y, z);
    fprintf(stderr, "  gradient: gx = %f, gy = %f, gz = %f \n",
	    grad4.x, grad4.y, grad4.z);


    /**************************************/
    z = sz + 1;
    x = sx;
    y = sy;
    fprintf(stderr, "This point, next slice: (%d,%d,%d) = %d \n",
	    x, y, z, data[z][y][x]);
    fprintf(stderr, "     (%d,%d,%d) = %d ; (%d,%d,%d) = %d \n",
	    x + 1, y, z, data[z][y][x + 1], x - 1, y, z, data[z][y][x - 1]);
    fprintf(stderr, "     (%d,%d,%d) = %d ; (%d,%d,%d) = %d \n",
	    x, y + 1, z, data[z][y + 1][x], x, y - 1, z, data[z][y - 1][x]);
    fprintf(stderr, "     (%d,%d,%d) = %d ; (%d,%d,%d) = %d \n",
	    x, y, z + 1, data[z + 1][y][x], x, y, z - 1, data[z - 1][y][x]);
    grad5 = get_gradient(x, y, z);
    fprintf(stderr, "  gradient: gx = %f, gy = %f, gz = %f \n",
	    grad5.x, grad5.y, grad5.z);

    /**************/
    x = sx + 1;
    y = sy;
    fprintf(stderr, "next point, next slice: (%d,%d,%d) = %d \n", x, y, z,
	    data[z][y][x]);
    fprintf(stderr, "     (%d,%d,%d) = %d ; (%d,%d,%d) = %d \n",
	    x + 1, y, z, data[z][y][x + 1], x - 1, y, z, data[z][y][x - 1]);
    fprintf(stderr, "     (%d,%d,%d) = %d ; (%d,%d,%d) = %d \n",
	    x, y + 1, z, data[z][y + 1][x], x, y - 1, z, data[z][y - 1][x]);
    fprintf(stderr, "     (%d,%d,%d) = %d ; (%d,%d,%d) = %d \n",
	    x, y, z + 1, data[z + 1][y][x], x, y, z - 1, data[z - 1][y][x]);
    grad6 = get_gradient(x, y, z);
    fprintf(stderr, "  gradient: gx = %f, gy = %f, gz = %f \n",
	    grad6.x, grad6.y, grad6.z);

    /**************/
    x = sx;
    y = sy + 1;
    fprintf(stderr, "next point, next slice: (%d,%d,%d) = %d \n", x, y, z,
	    data[z][y][x]);
    fprintf(stderr, "     (%d,%d,%d) = %d ; (%d,%d,%d) = %d \n",
	    x + 1, y, z, data[z][y][x + 1], x - 1, y, z, data[z][y][x - 1]);
    fprintf(stderr, "     (%d,%d,%d) = %d ; (%d,%d,%d) = %d \n",
	    x, y + 1, z, data[z][y + 1][x], x, y - 1, z, data[z][y - 1][x]);
    fprintf(stderr, "     (%d,%d,%d) = %d ; (%d,%d,%d) = %d \n",
	    x, y, z + 1, data[z + 1][y][x], x, y, z - 1, data[z - 1][y][x]);
    grad7 = get_gradient(x, y, z);
    fprintf(stderr, "  gradient: gx = %f, gy = %f, gz = %f \n",
	    grad7.x, grad7.y, grad7.z);

    /**************/
    x = sx + 1;
    y = sy + 1;
    fprintf(stderr, "next point, next slice: (%d,%d,%d) = %d \n", x, y, z,
	    data[z][y][x]);
    fprintf(stderr, "     (%d,%d,%d) = %d ; (%d,%d,%d) = %d \n",
	    x + 1, y, z, data[z][y][x + 1], x - 1, y, z, data[z][y][x - 1]);
    fprintf(stderr, "     (%d,%d,%d) = %d ; (%d,%d,%d) = %d \n",
	    x, y + 1, z, data[z][y + 1][x], x, y - 1, z, data[z][y - 1][x]);
    fprintf(stderr, "     (%d,%d,%d) = %d ; (%d,%d,%d) = %d \n",
	    x, y, z + 1, data[z + 1][y][x], x, y, z - 1, data[z - 1][y][x]);
    grad8 = get_gradient(x, y, z);
    fprintf(stderr, "  gradient: gx = %f, gy = %f, gz = %f \n",
	    grad8.x, grad8.y, grad8.z);

    norm_vect.x = .125 * (grad1.x + grad2.x + grad3.x + grad4.x + grad5.x +
			  grad6.x + grad7.x + grad8.x);
    norm_vect.y = .125 * (grad1.y + grad2.y + grad3.y + grad4.y + grad5.y +
			  grad6.y + grad7.y + grad8.y);
    norm_vect.z = .125 * (grad1.z + grad2.z + grad3.z + grad4.z + grad5.z +
			  grad6.z + grad7.z + grad8.z);

    fprintf(stderr, "Normal for location (%d,%d,%d) = (%f,%f,%f) \n",
	    sx, sy, sz, norm_vect.x, norm_vect.y, norm_vect.z);

    norm_vect = normalize_vect(norm_vect);
    fprintf(stderr, "Normalized: location (%d,%d,%d) = (%f,%f,%f) \n",
	    sx, sy, sz, norm_vect.x, norm_vect.y, norm_vect.z);
}
