
#include <stdio.h>
#include <sys/types.h>
#include <hipl_format.h>

char     *Progname;

u_char ***image;

/* input image info */
int       rw, cl, nf;		/* number of rows, columns in the image */
int       pix_format;


/*****************************************************/
main(argc, argv)
    int       argc;
    char    **argv;
{
    register int c, r, f, idx;

    struct header hd;		/* hips header */

    u_char ***alloc_3d_byte_array(), *image_ptr;

    Progname = strsave(*argv);

    read_header(&hd);
    if (hd.pixel_format != PFBYTE)
	perr("image must be in byte format");

    rw = hd.rows;		/* y axis */
    cl = hd.cols;		/* x axis */
    nf = hd.num_frame;
    pix_format = hd.pixel_format;

    fprintf(stderr, "\n rows: %d,  cols: %d,  frames: %d \n", rw, cl, nf);

    image = alloc_3d_byte_array(nf, rw, cl);


    fprintf(stderr, "\n loading image... \n ");

    read_3d_byte_array(stdin, image, nf, rw, cl);


#ifdef DEBUG
    show_slice(fr);
#endif

        image_ptr = **image;
	idx = 0;

	for (f = 0; f < nf; f++) {
	    fprintf(stderr, " frame %d ", f);
	    for (r = 0; r < rw; r++)
		for (c = 0; c < cl; c++) {
/* works
		    if (image[f][r][c] != (*((**image)+idx)))
*/
/* works 
		    if (image[f][r][c] != (**image)[idx++])
*/

/* also works
		    if (image[f][r][c] != *((**image)++))
*/
/* fastest method */
		    if (image[f][r][c] != *(image_ptr++))
			fprintf(stderr, " val 1: %d, val 2: %d \n",
				image[f][r][c], (**image)[idx-1]);
		}
	}

    fprintf(stderr, "\n\n finished. \n\n");
    return (0);
}


/***************************************************************/

show_slice(sn)			/* for debugging  */
    int       sn;
{
    register int c, r;

    fprintf(stderr, "\n frame #: %d \n", sn);

    for (r = 0; r < rw; r++) {
	for (c = 0; c < cl; c++)
	    fprintf(stderr, "%3d", image[r][c]);
	fprintf(stderr, "\n");
    }
}
