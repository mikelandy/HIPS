
/* ltest3.c   to test c_array lib calls
 */

#include <hipl_format.h>
#include <stdio.h>
#include <sys/types.h>

char     *Progname;


main(argc, argv)
    int       argc;
    char     *argv[];
{
    register int i, j;
    int r,c;
    struct header hd;
    u_char **alloc_2d_byte_array();



    u_char  **pic1, **pic2;

    Progname = strsave(*argv);

    read_header(&hd);
    if (hd.pixel_format != PFBYTE)
	perr("image pixel format must be byte");

    update_header(&hd, argc, argv);
    write_header(&hd);


    r = hd.rows;
    c = hd.cols;

    fprintf(stderr, " num cols: %d, num rows: %d \n\n", c,r);

    pic1 = alloc_2d_byte_array(r,c);
    pic2 = alloc_2d_byte_array(r,c); 
    
    read_2d_byte_array(stdin, pic1, r,c);
	
    for (i = 0; i < r; i++)
	for (j = 0; j < c; j++)  {
	    pic2[i][j] = pic1[i][j]; 
		fprintf(stderr, "at (%d,%d), pixel val: %d \n", 
			i,j,pic1[i][j]);
	}
    write_2d_byte_array(stdout, pic2, r,c); 


    return (0);
}

