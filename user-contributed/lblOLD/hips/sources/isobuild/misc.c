
/* misc.c     Brian Tierney */

/* for use with the isobuild program:
 * routines for array allocation / deallocation / initilization, etc
 *
 *  3d arrays are all allocated as arrays of pointer to arrays or pointers
 *  to arrays of data.  This takes a bit more memory, but array element
 *  access is somewhat faster because there is no additions/multiplication
 *  used in the access, only indirection.  -BT
 */

/* $Id: misc.c,v 1.3 1992/01/31 02:05:45 tierney Exp $ */

/* $Log: misc.c,v $
 * Revision 1.3  1992/01/31  02:05:45  tierney
 * y
 *
 * Revision 1.2  1991/12/19  01:42:20  davidr
 * added RCS identification markers
 * */

static char rcsid[] = "$Id: misc.c,v 1.3 1992/01/31 02:05:45 tierney Exp $" ;

#include "isobuild.h"

/**************************** get_max_min ****************************/

void
get_max_min(size, max, min)
    int       size;
    Data_type *max, *min;
/* This subroutine finds the maximum & minimum data values */
{

    register int i;
    register Data_type *dptr;
    Data_type lmin, lmax;	/* local min and max variable */

    dptr = data[0][0];

    lmax = lmin = dptr[0];
    for (i = 0; i < size; i++) {
	if (dptr[i] > lmax)
	    lmax = dptr[i];
	else if (dptr[i] < lmin)
	    lmin = dptr[i];
    }

    *max = lmax;
    *min = lmin;
    return;
}

/**************************************************/

Data_type ***
alloc_3d_data_array(nx, ny, nz)
    int       nx, ny, nz;
{
    register int i, j;

    Data_type ***array;

    /* allocate 3-d array for input image data */

    /* allocate 2 arrays of pointers */
    if ((array = Calloc(nx, Data_type **)) == NULL)
	perror("calloc error: data array ");
    if ((array[0] = Calloc(nx * ny, Data_type *)) == NULL)
	perror("calloc error: data array ");

    /* allocate array for data */
    if ((array[0][0] = Calloc(nx * ny * nz, Data_type)) == NULL)
	perror("calloc error: data array ");

    /* initialize pointer arrays */
    for (i = 1; i < ny; i++)
	array[0][i] = **array + (nz * i);
    for (i = 1; i < nx; i++) {
	array[i] = *array + (ny * i);
	array[i][0] = **array + (ny * nz * i);
	for (j = 1; j < ny; j++)/* initialize pointer array */
	    array[i][j] = array[i][0] + (nz * j);
    }
    return (array);
}

/***********************************************************/
Grid_type ***
alloc_3d_grid_array(nx, ny, nz)	/* in hips terminology: col,row,frame */
    int       nx, ny, nz;
{
    register int i, j;

    /* allocate 3-d array for grid  */

    Grid_type ***array;

    /* allocate 2 arrays of pointers */
    if ((array = Calloc(nx, Grid_type **)) == NULL)
	perror("calloc error: grid array ");
    if ((array[0] = Calloc(nx * ny, Grid_type *)) == NULL)
	perror("calloc error: grid array ");

    /* allocate array for data */
    if ((array[0][0] = Calloc(nx * ny * nz, Grid_type)) == NULL)
	perror("calloc error: grid array ");

    /* initialize pointer arrays */
    for (i = 1; i < ny; i++)
	array[0][i] = **array + (nz * i);
    for (i = 1; i < nx; i++) {
	array[i] = *array + (ny * i);
	array[i][0] = **array + (nz * ny * i);
	for (j = 1; j < ny; j++)/* initialize pointer array */
	    array[i][j] = array[i][0] + (nz * j);
    }
    return (array);

}

/***********************************************************/
NORMAL_VECT ***
alloc_3d_normal_array(nx, ny, nz)	/* in hips terminology: col,row,frame */
    int       nx, ny, nz;
{
    register int i, j;

    NORMAL_VECT ***array;

    /* allocate 3-d array for array  */

    /* allocate 2 arrays of pointers */
    if ((array = Calloc(nx, NORMAL_VECT **)) == NULL)
	perror("calloc error: norm array ");
    if ((array[0] = Calloc(nx * ny, NORMAL_VECT *)) == NULL)
	perror("calloc error: norm array ");

    /* allocate array for data */
    if ((array[0][0] = Calloc(nx * ny * nz, NORMAL_VECT)) == NULL)
	perror("calloc error: norm array ");

    /* initialize pointer arrays */
    for (i = 1; i < ny; i++)
	array[0][i] = **array + (nz * i);
    for (i = 1; i < nx; i++) {
	array[i] = *array + (ny * i);
	array[i][0] = **array + (nz * ny * i);
	for (j = 1; j < ny; j++)/* initialize pointer array */
	    array[i][j] = array[i][0] + (nz * j);
    }
    return (array);

}
/***********************************************************/
NORMAL_VECT **
alloc_2d_normal_array(nx, ny)	/* in hips terminology: col,row,frame */
    int       nx, ny;
{
    register int i;

    NORMAL_VECT **array;

    /* allocate array of pointers */
    if ((array = Calloc(nx, NORMAL_VECT *)) == NULL)
	perror("calloc error: 2d normal array ");

    /* allocate array for data */
    if ((array[0] = Calloc(nx * ny, NORMAL_VECT)) == NULL)
	perror("calloc error: 2d normal array ");

    /* initialize pointer arrays */
    for (i = 0; i < nx; i++)
	array[i] = *array + (ny * i);

    return (array);
}

/***********************************************************/
FLOAT_VECT **
alloc_2d_vector_array(nx, ny)	/* in hips terminology: col,row,frame */
    int       nx, ny;
{
    register int i;

    FLOAT_VECT **array;

    /* allocate array of pointers */
    if ((array = Calloc(nx, FLOAT_VECT *)) == NULL)
	perror("calloc error: 2d vector array ");

    /* allocate array for data */
    if ((array[0] = Calloc(nx * ny, FLOAT_VECT)) == NULL)
	perror("calloc error: 2d vector array ");

    /* initialize pointer arrays */
    for (i = 0; i < nx; i++)
	array[i] = *array + (ny * i);

    return (array);
}

/**********************************/
Data_type **
alloc_2d_data_array(nx, ny)
    int       nx, ny;
{
    Data_type **array;
    register int i;

    /* allocate 2-d array for input image data */
    /* allocate array of pointers */
    if ((array = Calloc(nx, Data_type *)) == NULL)
	perror("calloc error: 2d data array ");

    /* allocate array for data */
    if ((array[0] = Calloc(nx * ny, Data_type)) == NULL)
	perror("calloc error: 2d data array ");

    /* initialize pointer arrays */
    for (i = 0; i < nx; i++)
	array[i] = *array + (ny * i);

    return (array);
}

/*********************************************************/
CUBE_TRIANGLES **
alloc_2d_cube_array(nx, ny)
    int       nx, ny;
{
    CUBE_TRIANGLES **array;
    register int i;

    /* allocate 2-d array for input image data */
    /* allocate array of pointers */
    if ((array = Calloc(nx, CUBE_TRIANGLES *)) == NULL)
	perror("calloc error: cube array ");

    /* allocate array for data */
    if ((array[0] = Calloc(nx * ny, CUBE_TRIANGLES)) == NULL)
	perror("calloc error: cube array ");

    /* initialize pointer arrays */
    for (i = 1; i < nx; i++)
	array[i] = array[0] + (ny * i);

    return (array);
}

/**********************************/
Grid_type **
alloc_2d_grid_array(nx, ny)
    int       nx, ny;
{
    Grid_type **array;
    register int i;

    /* allocate 2-d array for input image data */
    /* allocate array of pointers */
    if ((array = Calloc(nx, Grid_type *)) == NULL)
	perror("calloc error: array ");

    /* allocate array for data */
    if ((array[0] = Calloc(nx * ny, Grid_type)) == NULL)
	perror("calloc error: array ");

    /* initialize pointer arrays */
    for (i = 0; i < nx; i++)
	array[i] = *array + (ny * i);

    return (array);
}

/**********************************/
BLOCK_INFO **
alloc_block_info_array(nx, ny)
    int       nx, ny;
{
    BLOCK_INFO **array;
    register int i;

    /* allocate 2-d array for input image data */
    /* allocate array of pointers */
    if ((array = Calloc(nx, BLOCK_INFO *)) == NULL)
	perror("calloc error: array ");

    /* allocate array for data */
    if ((array[0] = Calloc(nx * ny, BLOCK_INFO)) == NULL)
	perror("calloc error: array ");

    /* initialize pointer arrays */
    for (i = 0; i < nx; i++)
	array[i] = *array + (ny * i);

    return (array);
}

/********************************/
int
free_block_info_array(array)
    BLOCK_INFO **array;
{
#ifdef CRAY
    cfree((char *) array[0],NULL,NULL);
    cfree((char *) array,NULL,NULL);
#else
    cfree((char *) array[0]);
    cfree((char *) array);
#endif
}

/********************************/
int
free_3d_data_array(array)
    Data_type ***array;
{
#ifdef CRAY
    cfree((char *) array[0][0],NULL,NULL);
    cfree((char *) array[0],NULL,NULL);
    cfree((char *) array,NULL,NULL);
#else
    cfree((char *) array[0][0]);
    cfree((char *) array[0]);
    cfree((char *) array);
#endif
}

/********************************/
int
free_3d_normal_array(array)
    NORMAL_VECT ***array;
{
#ifdef CRAY
    cfree((char *) array[0][0], NULL, NULL);
    cfree((char *) array[0], NULL, NULL);
    cfree((char *) array, NULL, NULL);
#else
    cfree((char *) array[0][0]);
    cfree((char *) array[0]);
    cfree((char *) array);
#endif
}
/********************************/
int
free_2d_normal_array(array)
    NORMAL_VECT **array;
{
#ifdef CRAY
    cfree((char *) array[0], NULL, NULL);
    cfree((char *) array, NULL, NULL);
#else
    cfree((char *) array[0]);
    cfree((char *) array);
#endif
}
/********************************/
int
free_2d_vector_array(array)
    FLOAT_VECT **array;
{
#ifdef CRAY
    cfree((char *) array[0], NULL, NULL);
    cfree((char *) array, NULL, NULL);
#else
    cfree((char *) array[0]);
    cfree((char *) array);
#endif
}

/********************************/
int
free_3d_grid_array(array)
    Grid_type ***array;
{
#ifdef CRAY
    cfree((char *) array[0][0], NULL, NULL);
    cfree((char *) array[0], NULL, NULL);
    cfree((char *) array, NULL, NULL);
#else
    cfree((char *) array[0][0]);
    cfree((char *) array[0]);
    cfree((char *) array);
#endif
}

/*********************************/
int
free_2d_data_array(array)
    Data_type **array;
{
#ifdef CRAY
    cfree((char *) array[0], NULL, NULL);
    cfree((char *) array, NULL, NULL);
#else
    cfree((char *) array[0]);
    cfree((char *) array);
#endif
}

/*********************************/
int
free_2d_cube_array(array)
    CUBE_TRIANGLES **array;
{
#ifdef CRAY
    cfree((char *) array[0], NULL, NULL);
    cfree((char *) array, NULL, NULL);
#else
    cfree((char *) array[0]);
    cfree((char *) array);
#endif
}

/*********************************/
int
free_2d_grid_array(array)
    Grid_type **array;
{
#ifdef CRAY
    cfree((char *) array[0], NULL, NULL);
    cfree((char *) array, NULL, NULL);
#else
    cfree((char *) array[0]);
    cfree((char *) array);
#endif
}
/***************************************/

#ifdef DEBUG_SIZE
get_mem_size()
{
    int       pid;
    char      argstr[40];

    pid = getpid();

    sprintf(argstr, "show_prog_size %d", pid);

    (void) system(argstr);

#ifdef WANT
    /* execlp uses the shell's path for searching for the executable */
    (void) execlp("show_prog_size", "show_prog_size", arg1, NULL);
#endif
}

#endif

/********************************************************/
init_dup_vertex_check_arrays()
{
    register int i,j;

    /* reset the currslice and prevslice info */
    for (i=0; i< xdim; i++)
	for (j=0; j< ydim; j++) {
	    currslice[j][i].edge_index = -1;
	    currslice[j][i].num_edges = 0;
	    prevslice[j][i].edge_index = -1;
	    prevslice[j][i].num_edges = 0;
	}
}

/***************************************************************/

void
Error(mesg)
char *mesg;
{

    fprintf(stderr,"\n%s: Error, %s \n", MY_NAME, mesg);
    exit_handler();
}
     
/***************************************************************/

void
Status(mesg)
char *mesg;
{
    fprintf(stderr,"%s: %s \n", MY_NAME, mesg);
}


