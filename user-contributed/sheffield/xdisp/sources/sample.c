/* image subsample handling routines */

#include <xdisp.h>


/*****************************
 * sample_image()
 *****************************/

void sample_image()
{
    int i,j;
    int fr;

    sample_rate = (ncols > nrows) ? (float)ncols/SAMPLE_SIZE :
					(float)nrows/SAMPLE_SIZE;

    srows = nrows/sample_rate;
    scols = ncols/sample_rate;

    for (fr = 0; fr <= nframes; fr++)
	samples[fr] = (unsigned char *)malloc(srows*scols);

    for (fr = 0; fr < nframes; fr++) {
    	for (i = 0; i < srows; i++)
	    for (j = 0; j < scols; j++)
	    	samples[fr][i*scols+j] = 
		    images[fr][(int)(i*sample_rate)*ncols+
					(int)(j*sample_rate)];
	}

    sample_pix_data.origin.r = sample_pix_data.origin.c = 0;
    sample_pix_data.mag = 1;
    sample_pix_data.width = 0;
    sample_pix_data.height = 0;
    sample_pix_data.max_width = scols;
    sample_pix_data.max_height = srows;
    sample_pix_data.depth = depth;
    sample_pix_data.pix = NULL;
    sample_pix_data.im = samples[0];
    sample_pix_data.pix_cache = (Pixmap *)malloc((nframes+1)*sizeof(Pixmap *));

    for (i = 0; i <= nframes; i++)
	sample_pix_data.pix_cache[i] = NULL;
}
