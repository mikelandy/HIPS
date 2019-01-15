
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
    register int i, j,k;
    int f,r,c;
    struct header hd;
    u_char ***alloc_3d_byte_array();



    u_char  ***pic1, ***pic2;

    Progname = strsave(*argv);

    read_header(&hd);
    if (hd.pixel_format != PFBYTE)
	perr("image pixel format must be byte");

    update_header(&hd, argc, argv);
    write_header(&hd);

    f = hd.num_frame;
    r = hd.rows;
    c = hd.cols;

    fprintf(stderr, " num cols: %d, num rows: %d \n\n", c,r);

    pic1 = alloc_3d_byte_array(f,r,c);
    pic2 = alloc_3d_byte_array(f,r,c); 
    
    read_3d_byte_array(stdin, pic1, f,r,c);
	
    for (i = 0; i < f; i++)
	for (j = 0; j < r; j++)  
	    for(k=0; k< c; k++) {
	    pic2[i][j][k] = pic1[i][j][k]; 
		fprintf(stderr, "at (%d,%d,%d), pixel val: %d \n", 
			i,j,k, pic1[i][j][k]);
	}
    write_3d_byte_array(stdout, pic2, f,r,c); 


    return (0);
}

