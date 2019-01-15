
/*
 *     c_array.c   a collection of routines for handeling
 *                 dynamically allocated arrays in C.  
 *
 *     Brian Tierney  LBL
 *
 *  This routines allocate and access array by creating arrays of pointers.
 *   The reason for these routines are speed and code readability. Allocating
 *   arrays in this manner means that elements can be accessed by pointers
 *   instead of by a multiplication and an addition. This method uses more 
 *   memory, but on a machine with plenty of memory the increase in speed 
 *   is worth it.  
 *
 *    for example, this:
 * 
 *       for (r = 0; r < nrow; r++)
 *           for (c = 0; c < ncol; c++) {
 *		    pixel = image[r][c];
 *
 * is faster and more readable (in my opinion), than this:
 *
 *       for (r = 0; r < nrow; r++)
 *           for (c = 0; c < ncol; c++) {
 *		    pixel = image[r * nrow + c];
 */

/* routines in this file:

      alloc_3d_byte_array(nx,ny,nz)
      alloc_2d_byte_array(nx,ny)
      read_3d_byte_array(fp,array,nx,ny,nz)
      read_2d_byte_array(fp,array,nx ,ny)
      write_3d_byte_array(fp,array,nx,ny,nz)
      write_2d_byte_array(fp,array,nx,ny)
      free_3d_byte_array(array)
      free_2d_byte_array(array)

  ** and same routines for short, int, and float

  *
  *  all read/write routines return a 0 if successful and 1 otherwise 
  *
  *
  *  sample use with hips images:  r=rows, c=cols, f = frames
  *    2D case:
  *    u_char **pic;
  *    pic = alloc_2d_byte_array(r,c);
  *    read_2d_byte_array(stdin, pic1, r,c);
  *
  *    for (i = 0; i < r; i++)
  *	for (j = 0; j < c; j++)  {  * want column to vary fastest *
  *	    val = pic[i][j]; 
  *	}
  *    write_2d_byte_array(stdout, pic2, r,c); 
  *
  ****************************************************
  *   3D case:
  *    u_char **pic;
  *    pic = alloc_3d_byte_array(f,r,c);
  *  
  *  read_3d_byte_array(stdin, pic, f,r,c);
  *	
  *   for (i = 0; i < f; i++)
  *      for (j = 0; j < r; j++)  
  *	    for(k=0; k< c; k++) {  * vary col fastest, frame slowest *
  *            val = pic[i][j][k]; 
  */

/***********************************************************************/
/*  COPYRIGHT NOTICE         *******************************************/
/***********************************************************************/

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

     This software is provided as a professional  academic
contribution for  joint exchange.  Thus it is experimental, is
provided ``as is'', with no warranties of any kind  whatsoever,
no  support,  promise  of updates, or printed documentation.
Bug reports or fixes may be sent to the author, who may or may
not act on them as he desires.
*/

/*   Author:  Brian L. Tierney
 *            Lawrence Berkeley Laboratory
 *            Imaging and Distributed Computing Group
 *            email: bltierney@lbl.gov
*/

#include <stdio.h>
#include <sys/types.h>

#define Calloc(x,y) (y *)calloc((unsigned)(x), sizeof(y))
#define Fread(a,b,c,d) fread((char *)(a), b, (int)(c), d)
#define Fwrite(a,b,c,d) fwrite((char *)(a), b, (int)(c), d)

/*********************************/
u_char ***
alloc_3d_byte_array(nx,ny,nz)  /* in hips terminology: col,row,frame */
int nx,ny,nz;
{
    u_char ***array;
    register int i, j;

    /* allocate 3-d array for input image data */

    /* allocate 2 arrays of pointers */
    if ((array = Calloc(nx, u_char **)) == NULL)
	perror("calloc error: array ");
    if ((array[0] = Calloc(nx * ny, u_char *)) == NULL)
	perror("calloc error: array ");

    /* allocate array for data */
    if ((array[0][0] = Calloc(nx * ny * nz, u_char)) ==	NULL)
	perror("calloc error: array ");

    /* initialize pointer arrays */
    for (i = 1; i < ny; i++)
	array[0][i] = **array + (nz * i);
    for (i = 1; i < nx; i++) {
	array[i] = *array + (ny * i);
	array[i][0] = **array + (nz * ny * i);
	for (j = 1; j < ny; j++)/* initialize pointer array */
	    array[i][j] = array[i][0] + (nz * j);
    }
    return(array);
}

/**********************************/
u_char **
alloc_2d_byte_array(nx,ny)
int nx,ny;
{
    u_char **array;
    register int i;

    /* allocate 2-d array for input image data */
    /* allocate array of pointers */
    if ((array = Calloc(nx, u_char *)) == NULL)
	perror("calloc error: array ");
 
    /* allocate array for data */
    if ((array[0] = Calloc(nx * ny, u_char)) == NULL)
	perror("calloc error: array ");

    /* initialize pointer arrays */
    for (i = 1; i < nx; i++) 
	array[i] = array[0] + (ny * i);

    return(array);
}

/**********************************/
char **
alloc_2d_char_array(nx,ny)
int nx,ny;
{
    char **array;
    register int i;

    /* allocate 2-d array for input image data */
    /* allocate array of pointers */
    if ((array = Calloc(nx, char *)) == NULL)
	perror("calloc error: array ");
 
    /* allocate array for data */
    if ((array[0] = Calloc(nx * ny, char)) == NULL)
	perror("calloc error: array ");

    /* initialize pointer arrays */
    for (i = 1; i < nx; i++) 
	array[i] = array[0] + (ny * i);

    return(array);
}
/********************************/
int
read_3d_byte_array(fp,array,nx,ny,nz)
FILE *fp;
u_char ***array;
int nx,ny,nz;
{
    long      rsize;

    rsize = nx * ny * nz; 
    if (Fread(array[0][0], sizeof(u_char), rsize, fp)  != rsize) {
	perror("\n error reading file\n");
	return(-1);
    }
    return(0);
}

/********************************/
int
read_2d_byte_array(fp,array,nx ,ny)
FILE *fp;
u_char **array;
int nx,ny;
{
    long      rsize;

    rsize = nx * ny; 

    if (Fread(array[0], sizeof(u_char), rsize, fp)  != rsize) {
	    perror("\n error reading file\n"); 
	    return (-1);
	} 

    return(0);
}

/*******************************/
int
write_3d_byte_array(fp,array,nx,ny,nz)
FILE *fp;
u_char ***array;
int nx,ny,nz;
{
    long      size;

    size = nx * ny * nz;  
    if (Fwrite(array[0][0], sizeof(u_char), size, fp)  != size) {
	perror("\n error writing file\n");
	return (-1);
    }

    return(0);
}

/********************************/
int
write_2d_byte_array(fp,array,nx ,ny)
FILE *fp;
u_char **array;
int nx,ny;
{
    long      size;

    size = nx* ny; 

    if (Fwrite(array[0], sizeof(u_char), size, fp)  != size) {
	perror("\n error writing file\n");
	return(-1);
    }
    return(0);
}

/********************************/
int
free_3d_byte_array(array)
u_char ***array;
{
    cfree((char *)array[0][0]);
    cfree((char *)array[0]);
    cfree((char *)array);
}

/*********************************/
int
free_2d_byte_array(array)
u_char **array;
{
    cfree((char *)array[0]);
    cfree((char *)array);
}

/********************************************************/
/* same routines for data type short                    */
/********************************************************/

short ***
alloc_3d_short_array(nx,ny,nz)
int nx,ny,nz;
{
    short ***array;
    register int i, j;

    /* allocate 3-d array for input image data */

    /* allocate 2 arrays of pointers */
    if ((array = Calloc(nx, short **)) == NULL)
	perror("calloc error: array ");
    if ((array[0] = Calloc(nx * ny, short *)) == NULL)
	perror("calloc error: array ");

    /* allocate array for data */
    if ((array[0][0] = Calloc(nx * ny * nz, short)) ==	NULL)
	perror("calloc error: array ");

    /* initialize pointer arrays */
    for (i = 1; i < ny; i++)
	array[0][i] = **array + (nz * i);
    for (i = 1; i < nx; i++) {
	array[i] = *array + (ny * i);
	array[i][0] = **array + (ny * nz * i);
	for (j = 1; j < ny; j++)/* initialize pointer array */
	    array[i][j] = array[i][0] + (nz * j);
    }
    return(array);
}

/**********************************/
short **
alloc_2d_short_array(nx,ny)
int nx,ny;
{
    short **array;
    register int i;

    /* allocate 2-d array for input image data */
    /* allocate array of pointers */
    if ((array = Calloc(nx, short *)) == NULL)
	perror("calloc error: array ");
 
    /* allocate array for data */
    if ((array[0] = Calloc(nx * ny, short)) == NULL)
	perror("calloc error: array ");

    /* initialize pointer arrays */
    for (i = 0; i < nx; i++) 
	array[i] = *array + (ny * i);

    return(array);
}

/********************************/
int
read_3d_short_array(fp,array,nx,ny,nz)
FILE *fp;
short ***array;
int nx,ny,nz;
{
    long      rsize;

    rsize = nx * ny * nz; 
    if (Fread(array[0][0], sizeof(short), rsize, fp)  != rsize) {
	perror("\n error reading file\n");
	return(-1);
    }
    return(0);
}

/********************************/
int
read_2d_short_array(fp,array,nx ,ny)
FILE *fp;
short **array;
int nx,ny;
{
    long      rsize;

    rsize = nx * ny; 
    if (Fread(array[0], sizeof(short), rsize, fp)  != rsize) {
	    perror("\n error reading file\n");
	    return(-1);
	}
    return(0);
}

/*******************************/
int
write_3d_short_array(fp,array,nx,ny,nz)
FILE *fp;
short ***array;
int nx,ny,nz;
{
    long      size;

    size = nx * ny * nz;  
    if (Fwrite(array[0][0], sizeof(short), size, fp)  != size) {
	perror("\n error writing file\n");
	return(-1);
    }
    return(0);
}

/********************************/
int
write_2d_short_array(fp,array,nx ,ny)
FILE *fp;
short **array;
int nx,ny;
{
    long      size;

    size = nx* ny; 

    if (Fwrite(array[0], sizeof(short), size, fp)  != size) {
	perror("\n error writing file\n");
	return(-1);
    }
    return(0);
}

/********************************/
int
free_3d_short_array(array)
short ***array;
{
    cfree((char *)array[0][0]);
    cfree((char *)array[0]);
    cfree((char *)array);
}

/*********************************/
int
free_2d_short_array(array)
short **array;
{
    cfree((char *)array[0]);
    cfree((char *)array);
}

/****************************************************/
/*  int routines */
/**************************************************/

int ***
alloc_3d_int_array(nx,ny,nz)
int nx,ny,nz;
{
    int ***array;
    register int i, j;

    /* allocate 3-d array for input image data */

    /* allocate 2 arrays of pointers */
    if ((array = Calloc(nx, int **)) == NULL)
	perror("calloc error: array ");
    if ((array[0] = Calloc(nx * ny, int *)) == NULL)
	perror("calloc error: array ");

    /* allocate array for data */
    if ((array[0][0] = Calloc(nx * ny * nz, int)) ==	NULL)
	perror("calloc error: array ");

    /* initialize pointer arrays */
    for (i = 1; i < ny; i++)
	array[0][i] = **array + (nz * i);
    for (i = 1; i < nx; i++) {
	array[i] = *array + (ny * i);
	array[i][0] = **array + (ny * nz * i);
	for (j = 1; j < ny; j++)/* initialize pointer array */
	    array[i][j] = array[i][0] + (nz * j);
    }
    return(array);
}

/**********************************/
int **
alloc_2d_int_array(nx,ny)
int nx,ny;
{
    int **array;
    register int i;

    /* allocate 2-d array for input image data */
    /* allocate array of pointers */
    if ((array = Calloc(nx, int *)) == NULL)
	perror("calloc error: array ");
 
    /* allocate array for data */
    if ((array[0] = Calloc(nx * ny, int)) == NULL)
	perror("calloc error: array ");

    /* initialize pointer arrays */
    for (i = 0; i < nx; i++) 
	array[i] = *array + (ny * i);

    return(array);
}

/********************************/
int
read_3d_int_array(fp,array,nx,ny,nz)
FILE *fp;
int ***array;
int nx,ny,nz;
{
    long      rsize;

    rsize = nx * ny * nz; 
    if (Fread(array[0][0], sizeof(int), rsize, fp)  != rsize) {
	perror("\n error reading file\n");
	return(-1);
    }
    return(0);
}

/********************************/
int
read_2d_int_array(fp,array,nx ,ny)
FILE *fp;
int **array;
int nx,ny;
{
    long      rsize;

    rsize = nx * ny; 
    if (Fread(array[0], sizeof(int), rsize, fp)  != rsize) {
	    perror("\n error reading file\n");
	    return(-1);
	}
    return(0);
}

/*******************************/
int
write_3d_int_array(fp,array,nx,ny,nz)
FILE *fp;
int ***array;
int nx,ny,nz;
{
    long      size;

    size = nx * ny * nz;  
    if (Fwrite(array[0][0], sizeof(int), size, fp)  != size) {
	perror("\n error writing file\n");
	return(-1);
    }
    return(0);
}

/********************************/
int
write_2d_int_array(fp,array,nx ,ny)
FILE *fp;
int **array;
int nx,ny;
{
    long      size;

    size = nx* ny; 

    if (Fwrite(array[0], sizeof(int), size, fp)  != size) {
	perror("\n error writing file\n");
	return(-1);
    }
    return(0);
}

/********************************/
int
free_3d_int_array(array)
int ***array;
{
    cfree((char *)array[0][0]);
    cfree((char *)array[0]);
    cfree((char *)array);
}

/*********************************/
int
free_2d_int_array(array)
int **array;
{
    cfree((char *)array[0]);
    cfree((char *)array);
}

/****************************************************/
/*  float routines */
/**************************************************/

float ***
alloc_3d_float_array(nx,ny,nz)
int nx,ny,nz;
{
    float ***array;
    register int i, j;

    /* allocate 3-d array for input image data */

    /* allocate 2 arrays of pointers */
    if ((array = Calloc(nx, float **)) == NULL)
	perror("calloc error: array ");
    if ((array[0] = Calloc(nx * ny, float *)) == NULL)
	perror("calloc error: array ");

    /* allocate array for data */
    if ((array[0][0] = Calloc(nx * ny * nz, float)) ==	NULL)
	perror("calloc error: array ");

    /* initialize pointer arrays */
    for (i = 1; i < ny; i++)
	array[0][i] = **array + (nz * i);
    for (i = 1; i < nx; i++) {
	array[i] = *array + (ny * i);
	array[i][0] = **array + (ny * nz * i);
	for (j = 1; j < ny; j++)/* initialize pointer array */
	    array[i][j] = array[i][0] + (nz * j);
    }
    return(array);
}

/**********************************/
float **
alloc_2d_float_array(nx,ny)
int nx,ny;
{
    float **array;
    register int i;

    /* allocate 2-d array for input image data */
    /* allocate array of pointers */
    if ((array = Calloc(nx, float *)) == NULL)
	perror("calloc error: array ");
 
    /* allocate array for data */
    if ((array[0] = Calloc(nx * ny, float)) == NULL)
	perror("calloc error: array ");

    /* initialize pointer arrays */
    for (i = 0; i < nx; i++) 
	array[i] = *array + (ny * i);

    return(array);
}

/********************************/
int
read_3d_float_array(fp,array,nx,ny,nz)
FILE *fp;
float ***array;
int nx,ny,nz;
{
    long      rsize;

    rsize = nx * ny * nz; 
    if (Fread(array[0][0], sizeof(float), rsize, fp)  != rsize) {
	perror("\n error reading file\n");
	return(-1);
    }
    return(0);
}

/********************************/
int
read_2d_float_array(fp,array,nx ,ny)
FILE *fp;
float **array;
int nx,ny;
{
    long      rsize;

    rsize = nx * ny; 
    if (Fread(array[0], sizeof(float), rsize, fp)  != rsize) {
	    perror("\n error reading file\n");
	    return(-1);
	}
    return(0);
}

/*******************************/
int
write_3d_float_array(fp,array,nx,ny,nz)
FILE *fp;
float ***array;
int nx,ny,nz;
{
    long      size;

    size = nx * ny * nz;  
    if (Fwrite(array[0][0], sizeof(float), size, fp)  != size) {
	perror("\n error writing file\n");
	return(-1);
    }
    return(0);
}

/********************************/
int
write_2d_float_array(fp,array,nx ,ny)
FILE *fp;
float **array;
int nx,ny;
{
    long      size;

    size = nx* ny; 

    if (Fwrite(array[0], sizeof(float), size, fp)  != size) {
	perror("\n error writing file\n");
	return(-1);
    }
    return(0);
}

/********************************/
int
free_3d_float_array(array)
float ***array;
{
    cfree((char *)array[0][0]);
    cfree((char *)array[0]);
    cfree((char *)array);
}

/*********************************/
int
free_2d_float_array(array)
float **array;
{
    cfree((char *)array[0]);
    cfree((char *)array);
}

/**************************************************************/
/*                  long routines                             */
/**************************************************************/

long ***
alloc_3d_long_array(nx,ny,nz)  /* in hips terminology: col,row,frame */
int nx,ny,nz;
{
    long ***array;
    register int i, j;

    /* allocate 3-d array for input image data */

    /* allocate 2 arrays of pointers */
    if ((array = Calloc(nx, long **)) == NULL)
	perror("calloc error: array ");
    if ((array[0] = Calloc(nx * ny, long *)) == NULL)
	perror("calloc error: array ");

    /* allocate array for data */
    if ((array[0][0] = Calloc(nx * ny * nz, long)) ==	NULL)
	perror("calloc error: array ");

    /* initialize pointer arrays */
    for (i = 1; i < ny; i++)
	array[0][i] = **array + (nz * i);
    for (i = 1; i < nx; i++) {
	array[i] = *array + (ny * i);
	array[i][0] = **array + (nz * ny * i);
	for (j = 1; j < ny; j++)/* initialize pointer array */
	    array[i][j] = array[i][0] + (nz * j);
    }
    return(array);
}

/**********************************/
long **
alloc_2d_long_array(nx,ny)
int nx,ny;
{
    long **array;
    register int i;

    /* allocate 2-d array for input image data */
    /* allocate array of pointers */
    if ((array = Calloc(nx, long *)) == NULL)
	perror("calloc error: array ");
 
    /* allocate array for data */
    if ((array[0] = Calloc(nx * ny, long)) == NULL)
	perror("calloc error: array ");

    /* initialize pointer arrays */
    for (i = 0; i < nx; i++) 
	array[i] = *array + (ny * i);

    return(array);
}

/********************************/
int
free_3d_long_array(array)
long ***array;
{
    cfree((char *)array[0][0]);
    cfree((char *)array[0]);
    cfree((char *)array);
}

/*********************************/
int
free_2d_long_array(array)
long **array;
{
    cfree((char *)array[0]);
    cfree((char *)array);
}

/***********************************************************************/
/*  all types: all use these if array is cast when calling the routine */
/***********************************************************************/
int
free_3d_array(array)
char ***array;
{
    cfree(array[0][0]);
    cfree(array[0]);
    cfree(array);
}

/*********************************/
int
free_2d_array(array)
char **array;
{
    cfree(array[0]);
    cfree(array);
}

