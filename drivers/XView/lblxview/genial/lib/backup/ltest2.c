
/* test_lib.c   to test c_array lib calls
 */

#include <stdio.h>
#include <sys/types.h>

#define X 4
#define Y 3
#define Z 2

main(argc, argv)
    int       argc;
    char     *argv[];
{
    register int i, j, k, check;
    u_char ***alloc_3d_byte_array(); 

    u_char ***pic_3d;


    pic_3d = alloc_3d_byte_array( X, Y, Z) ;

    
    for (i = 0; i < X; i++)
	for (j = 0; j < Y; j++)
	    for (k = 0; k < Z; k++) 
		pic_3d[i][j][k] = i;

    for (i = 0; i < X; i++)
	for (j = 0; j < Y; j++)
	    for (k = 0; k < Z; k++) 
		fprintf(stderr, " [%d][%d][%d] = %d \n", i,j,k,pic_3d[i][j][k]);


    return (0);
}

