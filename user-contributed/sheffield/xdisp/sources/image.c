/* image Pixmap cache setup routines */

#include <xdisp.h>


/**********************
 * create_image()
 **********************/

void create_image()
{
    int i; 

    sample_pix_data.gc = gc;

    image_pix_data.origin.r = image_pix_data.origin.c = 0;
    image_pix_data.mag = 1;
    image_pix_data.gc = gc;
    image_pix_data.depth = depth;
    image_pix_data.pix = NULL;
    image_pix_data.width = image_pix_data.height = 0;
    image_pix_data.max_width = ncols;
    image_pix_data.max_height = nrows;
    image_pix_data.im = images[0];
    image_pix_data.pix_cache = (Pixmap *)malloc((nframes+1)*sizeof(Pixmap *));

    for (i = 0; i <= nframes; i++)
	image_pix_data.pix_cache[i] = NULL;
}
