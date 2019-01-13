/* overlay frame creation stuff */

#include <xdisp.h>

void ov_create_frame()
{
  int i,j;
  int p = 0;
  unsigned char *im;

    free(images[frame]);
    images[frame] = (unsigned char *)malloc(nrows*ncols*sizeof(unsigned char));

    im = images[frame];

    /* create the overlay frame */

    for (i = 0; i < nrows; i++)
	for (j = 0; j < ncols; j++,p++)
	    *im++ = xmap[1][images[red_frame][p]]*NREDS/256*NBLUES*NGREENS+
			xmap[2][images[green_frame][p]]*NGREENS/256*NBLUES+
			xmap[3][images[blue_frame][p]]*NBLUES/256;

    /* and create a subsample overlay frame as well */

    for (i = 0; i < srows; i++)
	for (j = 0; j < scols; j++)
	    samples[frame][i*scols+j] = 
		images[frame][(int)(i*sample_rate)*ncols+
					(int)(j*sample_rate)];

    /* invalidate pix cache */

    if (image_pix_data.pix_cache)
	image_pix_data.pix_cache[nframes] = NULL;
    if (sample_pix_data.pix_cache)
	sample_pix_data.pix_cache[nframes] = NULL;

    /* set the histograms */

    hist_pix_data[1].pix = (cdf_mode) ? cdf_pixmaps[red_frame] :		
		hist_pixmaps[red_frame];
    hist_pix_data[2].pix = (cdf_mode) ? cdf_pixmaps[green_frame] :		
		hist_pixmaps[green_frame];
    hist_pix_data[3].pix = (cdf_mode) ? cdf_pixmaps[blue_frame] :		
		hist_pixmaps[blue_frame];

    /* ...and the colorbars */

    draw_colorcube_bars();

    /* reset sliders */

    for (i = 1; i < 4; i++) {
	update_limit_lines(i);
	gamma_limits[i].llx = gamma_limits[i].ll;
	gamma_limits[i].ulx = gamma_limits[i].ul;
	update_limit_lines(i);
	}

    j = 0;
    for (i = 1; i < 4; i++) slider_low_callback(slider_low[i],i,&j);
    j = 255;
    for (i = 1; i < 4; i++) slider_high_callback(slider_high[i],i,&j);
}
