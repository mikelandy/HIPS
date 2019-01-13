
/* test_lib.c   to test c_array lib calls
 */

#include <hipl_format.h>
#include <stdio.h>
#include <sys/types.h>

char     *Progname;


main(argc, argv)
    int       argc;
    char     *argv[];
{
    register int i, j, k, check;
    struct header hd;
    u_char **alloc_2d_byte_array();
    u_char ***alloc_3d_byte_array(); 

    u_char ***pic_3d;
    u_char  **pic_2d;

    Progname = strsave(*argv);

    read_header(&hd);
    if (hd.pixel_format != PFBYTE)
	perr("image pixel format must be byte");
    update_header(&hd, argc, argv);
    write_header(&hd);

    if (hd.num_frame == 1) {
	pic_2d = alloc_2d_byte_array(hd.cols, hd.rows);
	read_2d_byte_array(stdin, pic_2d, hd.cols, hd.rows);
	
	for (j = 0; j < hd.cols; j++)
	    for (k = 0; k < hd.rows; k++) 
		    ;  	/* do stuff here */

	write_2d_byte_array(stdout, pic_2d, hd.cols, hd.rows);
	free_2d_byte_array(pic_2d);

  /* 3D test */
    } else {
	pic_3d = alloc_3d_byte_array( hd.cols, hd.rows, hd.num_frame);
	read_3d_byte_array(stdin, pic_3d, hd.cols, hd.rows, hd.num_frame);
	for (i = 0; i < hd.cols; i++)
	    for (j = 0; j < hd.rows; j++)
		for (k = 0; k < hd.num_frame; k++) 
		     check = pic_3d[i][j][k];

	write_3d_byte_array(stdout, pic_3d, hd.cols, hd.rows, hd.num_frame);
	free_3d_byte_array(pic_3d);
    }
    return (0);
}

