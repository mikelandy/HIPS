
/* speed test program for 3D arrays routines  */
/* Brian Tierney, LBL,  10/90 */

#include <stdio.h>
#include <sys/types.h>

#define Calloc(x,y) (y *)calloc((unsigned)(x), sizeof(y))

#define SX 90
#define SY 100
#define SZ 110

/*****************************************************/
main()
{
    int       size, val, xlimit, ylimit, zlimit;
    register int i, j, k, idx1, idx2;
    register u_char   *aptr;		/* pointer to array */

    long      tbuf[4], ut1, ut2;/* for recording process times */

    /* array type 1: statically allocated 3D array */
    u_char    array1[SX][SY][SZ];

    /* array type 2: dynamically allocated 1D array */
    u_char   *array2;

    /* array type 3: dynamically allocated 3D array */
    u_char ***array3;

    u_char ***alloc_3d_byte_array();

    size = SX * SY * SZ;

    /* allocate dynamic arrays */
    array3 = alloc_3d_byte_array(SX, SY, SZ);

    array2 = (u_char *) calloc(size, sizeof(u_char));

    /*****************************************************************/
    /* sequential access test                                        */
    /*****************************************************************/

    fprintf(stderr, "\n Sequential access test: \n");

    times(tbuf);
    ut1 = tbuf[0];

    for (i = 0; i < SX; i++)
	for (j = 0; j < SY; j++)
	    for (k = 0; k < SZ; k++)
		array1[i][j][k] = 100;

    times(tbuf);
    ut2 = tbuf[0];
    fprintf(stderr, "result method 1: user time was = %d\n", ut2 - ut1);

    /*************************************/

    times(tbuf);
    ut1 = tbuf[0];

    for (i = 0; i < size; i++) {
	array2[i] = 100;
    }

    times(tbuf);
    ut2 = tbuf[0];
    fprintf(stderr, "result method 2: user time was = %d\n", ut2 - ut1);

    /*************************************/

    times(tbuf);
    ut1 = tbuf[0];

    aptr = array2;

    for (i = 0; i < size; i++) {
	*(aptr++) = 100;
    }

    times(tbuf);
    ut2 = tbuf[0];
    fprintf(stderr, "result method 3: user time was = %d\n", ut2 - ut1);

    /*************************************/

    times(tbuf);
    ut1 = tbuf[0];

    for (i = 0; i < SX; i++)
	for (j = 0; j < SY; j++)
	    for (k = 0; k < SZ; k++)
		array3[i][j][k] = 100;

    times(tbuf);
    ut2 = tbuf[0];
    fprintf(stderr, "result method 4: user time was = %d\n", ut2 - ut1);


    /*****************************************************************/
    /* random     access test                                        */
    /*****************************************************************/

    fprintf(stderr, "\n Random access test: \n");

    xlimit = SX - 1;
    ylimit = SY - 1;
    zlimit = SZ - 1;

    times(tbuf);
    ut1 = tbuf[0];

    for (i = 1; i < xlimit; i++)
	for (j = 1; j < ylimit; j++)
	    for (k = 1; k < zlimit; k++)
		val = array1[i - 1][j - 1][k + 1] + array1[i + 1][j + 1][k - 1];

    times(tbuf);
    ut2 = tbuf[0];
    fprintf(stderr, "result method 1: user time was = %d\n", ut2 - ut1);

    /*************************************/

    times(tbuf);
    ut1 = tbuf[0];

    for (i = 1; i < xlimit; i++)
	for (j = 1; j < ylimit; j++)
	    for (k = 1; k < zlimit; k++) {
		idx1 = ((i - 1) * SX) + ((j - 1) * SY) + (k + 1);
		idx2 = ((i + 1) * SX) + ((j + 1) * SY) + (k - 1);
		val = array2[idx1] + array2[idx2];
	    }

    times(tbuf);
    ut2 = tbuf[0];
    fprintf(stderr, "result method 2: user time was = %d\n", ut2 - ut1);

    /*************************************/

    times(tbuf);
    ut1 = tbuf[0];

    aptr = array2;

    for (i = 1; i < xlimit; i++)
	for (j = 1; j < ylimit; j++)
	    for (k = 1; k < zlimit; k++) {
		idx1 = ((i - 1) * SX) + ((j - 1) * SY) + (k + 1);
		idx2 = ((i + 1) * SX) + ((j + 1) * SY) + (k - 1);
		val = *(aptr + idx1) + *(aptr + idx2);
	    }


    times(tbuf);
    ut2 = tbuf[0];
    fprintf(stderr, "result method 3: user time was = %d\n", ut2 - ut1);

    /*************************************/

    times(tbuf);
    ut1 = tbuf[0];

    for (i = 1; i < xlimit; i++)
	for (j = 1; j < ylimit; j++)
	    for (k = 1; k < zlimit; k++)
		val = array3[i - 1][j - 1][k + 1] +
		    array3[i + 1][j + 1][k - 1];

    times(tbuf);
    ut2 = tbuf[0];
    fprintf(stderr, "result method 4: user time was = %d\n", ut2 - ut1);

    /*************************************/

    fprintf(stderr, "\n\n finished. \n\n");
    return (0);
}

/******************************************************************/
u_char ***
alloc_3d_byte_array(nx, ny, nz)
    int       nx, ny, nz;
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
    if ((array[0][0] = Calloc(nx * ny * nz, u_char)) == NULL)
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
    return (array);
}
