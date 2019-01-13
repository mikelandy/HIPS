/* Image drawing functions. The values in the image range from 0 to 255. The
   number of hardware colors available is usually less than this, so ymap
   controls the mapping from the 256 levels to the number of colors available.
   Also, gamma correction, via slider controls, is handled by the xmap mapping.
 */

#include <X11/Intrinsic.h>
#include <xdisp.h>

void render_image();

/*********************************
 * draw_image()
 *********************************/

void draw_image(pix_data)
  pix_data_t	*pix_data;
{
  pix_data_t pd;

    if (pix_cache_on && pix_data->pix_cache && pix_data->mag == 1) {
	if (!pix_data->pix_cache[frame]) {
	    pd.im = pix_data->im;
	    pd.width = pd.max_width = pix_data->max_width;
	    pd.height = pd.max_height = pix_data->max_height;
	    pd.depth = pix_data->depth;
	    pd.gc = pix_data->gc;
	    pd.pix = XCreatePixmap(dpy,rws,pd.width,pd.height,pd.depth);
	    pd.origin.r = pd.origin.c = 0;
	    pd.mag = pix_data->mag;
	    render_image(&pd);
	    pix_data->pix_cache[frame] = pd.pix;
	    }
	XCopyArea(dpy,pix_data->pix_cache[frame],pix_data->pix,pix_data->gc,
		pix_data->origin.c,pix_data->origin.r,
 		pix_data->width,pix_data->height,0,0);
	}
    else 
	render_image(pix_data);
}



/*********************************
 * render_image()
 *********************************/

/* Render an image into a supplied pixmap. The (nr)x(nc) section of image
   starting at coordinates (or,oc) are drawn in pix at the resolution given
   by mag. The image is the one given by fr.
*/

void render_image(pix_data)
  pix_data_t	*pix_data;
{
  int i,j,k,l;
  int mag = pix_data->mag;
  Pixmap pix = pix_data->pix;
  GC gc = pix_data->gc;
  int ncols = pix_data->max_width;
  unsigned char	*im = pix_data->im;
  int nr = pix_data->height/mag;
  int nc = pix_data->width/mag;

    init_buffer();

    im = im+(pix_data->origin.r*ncols+pix_data->origin.c);

    for (i = 0; i < nr; i++) {
	for (j = 0; j < nc; j++) {
	    for (k = 0; k < mag; k++)
		for (l = 0; l < mag; l++)
	    	    buffer_point(pix,gc,ymap[*im],j*mag+l,i*mag+k);
	    im++;
	    }
	im+= ncols-nc;
	}

    flush_buffer(pix,gc);
}


/*********************************
 * clear_pix_cache()
 *********************************/

void clear_pix_cache()
{
  int i;

    if (image_pix_data.pix_cache != NULL)
    	for (i = 0; i < nframes; i++)
	    if (image_pix_data.pix_cache[i] != NULL) {
	        XFreePixmap(dpy,image_pix_data.pix_cache[i]);
		image_pix_data.pix_cache[i] = NULL;
		}

    if (sample_pix_data.pix_cache != NULL)
        for (i = 0; i < nframes; i++)
	    if (sample_pix_data.pix_cache[i] != NULL) {
	        XFreePixmap(dpy,sample_pix_data.pix_cache[i]);
		sample_pix_data.pix_cache[i] = NULL;
		}
}
