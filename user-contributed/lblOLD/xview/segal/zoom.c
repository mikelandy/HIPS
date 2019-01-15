
/*  zoom.c                   -Brian Tierney ,  LBL
 *   for use with segal
 *
 *    zooms an area of an image
 */

#include "common.h"

#include "pixedit.h"

/*************************************************/
void
zoom()
{
    int       check;
    void      pixel_replicate();

    if (zoom_info.mag <= 0)
	return;

    /* check/reset zoom window size */

/* REMOVE?
    check = zoom_info.x + (zoom_info.width / zoom_info.mag);
    if (check >= segal.cols)
	zoom_info.x = segal.cols - (zoom_info.width / zoom_info.mag) - 5;
    check = zoom_info.y + (zoom_info.height / zoom_info.mag);
    if (check >= segal.rows)
	zoom_info.y = segal.rows - (zoom_info.height / zoom_info.mag) - 5;
    if (zoom_info.x < 0)
	zoom_info.x = 0;
    if (zoom_info.y < 0)
	zoom_info.y = 0;
*/

    pixel_replicate(zoom_info.x, zoom_info.y, zoom_info.mag);
}

/************************************************************/

static void
pixel_replicate(xoff, yoff, mag)
    int       xoff, yoff, mag;
{

/* the basic ideas in this method are from the program 'ximageview' by
        UC Berkeley Information Systems and Technology
        Advanced Technology Planning,        Copyright (c) 1989
*/

    register int x1, y1, delta;
    int       x2, y2;
    void      mag_x(), mag_y();

    /* magnify x dimension */
    x2 = 0;
    for (x1 = 0; x1 < region.cols; x1++) {
	for (delta = 0; delta < mag; delta++) {
	    mag_x(x1 + xoff, yoff, x2, 0, 1, region.rows * mag); 
		    /* sx,sy,dx,dy,w,h */
	    x2++;
	}
    }

    /* magnify y dimension */
    y2 = region.rows * mag - 1;
    for (y1 = region.rows - 1; y1 >= 0; y1--) {
	for (delta = 0; delta < mag; delta++) {
	    mag_y(0, y1, 0, y2, region.cols * mag, 1);
	    y2--;
	}
    }

}

/***********************************************************/
void
mag_x(sx, sy, dx, dy, width, height)	/* magnify in the x direction */
    int       sx, sy, dx, dy, width, height;
{
    register char *sp, *dp;
    char     *s1, *d1, *s2, *d2, *s3, *d3;
    int       s0, d0, of1, of2;
    register int i;

    /* this version assumes width = 1 */
    if (width != 1)
	return;

    if (dx + width > zoom_image->width)
	return;
    if (dy + height > zoom_image->height)
	height = zoom_image->height - (dy + 1);

#ifdef ZOOM_DEBUG
    fprintf(stderr, " copying %d x %d pixels from %d,%d to %d,%d \n",
	    width, height, sx, sy, dx, dy);
#endif

    s0 = (sy * image->bytes_per_line) + sx;
    d0 = (dy * zoom_image->bytes_per_line) + dx;

    if (himage.fp != NULL) {
	s1 = image->data + s0;
	d1 = zoom_image->data + d0;
    }

    s2 = mask_image->data + s0;
    d2 = zoom_mask_image->data + d0;

    s3 = blend_image->data + s0;
    d3 = zoom_blend_image->data + d0;

    for (i = 0; i < height; i++) {
	of1 = i * image->bytes_per_line;
	of2 = i * zoom_image->bytes_per_line;

	if (himage.fp != NULL) {
	    sp = s1 + of1;
	    dp = d1 + of2;
	    *dp = *sp;
	}
	sp = s2 + of1;
	dp = d2 + of2;
	*dp = *sp;

	sp = s3 + of1;
	dp = d3 + of2;
	*dp = *sp;
    }
}

/***********************************************************/
void
mag_y(sx, sy, dx, dy, width, height)	/* magnify in the y direction */
    int       sx, sy, dx, dy, width, height;
{
    char     *s1, *d1, *s2, *d2, *s3, *d3;
    int       s0, d0;

    /* this version assumes height = 1 */
    if (height != 1)
	return;

    if (dx + width > zoom_image->width)
	width = zoom_image->width - (dx + 1);
    if (dy + height > zoom_image->height)
	return;

#ifdef ZOOM_DEBUG
    fprintf(stderr, " copying %d x %d pixels from %d,%d to %d,%d \n",
	    width, height, sx, sy, dx, dy);
#endif

    s0 = (sy * zoom_image->bytes_per_line) + sx;
    d0 = (dy * zoom_image->bytes_per_line) + dx;

    if (himage.fp != NULL) {
	s1 = zoom_image->data + s0;
	d1 = zoom_image->data + d0;
    }
    s2 = zoom_mask_image->data + s0;
    d2 = zoom_mask_image->data + d0;

    s3 = zoom_blend_image->data + s0;
    d3 = zoom_blend_image->data + d0;

    if (himage.fp != NULL)
	bcopy(s1, d1, width);
    bcopy(s2, d2, width);
    bcopy(s3, d3, width);
}

/***********************************************/
void
create_zoom_ximages()
{				/* create ximage structure for zoom */
    XVisualInfo *winv;
    char     *dbuf;

    zoom_info.width = region.cols * zoom_info.mag;
    zoom_info.height = region.rows * zoom_info.mag;

    if (zoom_image != NULL) {
	if (zoom_image->width != zoom_info.width) {
	    XDestroyImage(zoom_image);
	    XDestroyImage(zoom_mask_image);
	    XDestroyImage(zoom_blend_image);
	} else {
	    return;
	}
    }
    winv = (XVisualInfo *) malloc(sizeof(XVisualInfo));
    if ((int) XMatchVisualInfo(display, screen,
			       XDisplayPlanes(display, screen),
			       PseudoColor, winv) == 0) {
	fprintf(stderr, "unable to find correct visual \n");
	exit(0);
    }
    if (verbose)
	fprintf(stderr, " creating zoom image.. \n");

    dbuf = Calloc(zoom_info.width * zoom_info.height, char);
    zoom_image = XCreateImage(display, winv->visual, 8, ZPixmap, 0,
			      dbuf, zoom_info.width, zoom_info.height, 8, 0);
    if (zoom_image == NULL) {
	fprintf(stderr, " Error creating zoom image! \n");
	return;
    }
    dbuf = Calloc(zoom_info.width * zoom_info.height, char);
    /* intialize to black */
    memset(dbuf, (int) colors[0], zoom_info.width * zoom_info.height);

    zoom_mask_image = XCreateImage(display, winv->visual, 8, ZPixmap, 0,
				dbuf, zoom_info.width, zoom_info.height, 8, 0);
    if (zoom_mask_image == NULL) {
	fprintf(stderr, " Error creating zoom image! \n");
	return;
    }
    dbuf = Calloc(zoom_info.width * zoom_info.height, char);
    zoom_blend_image = XCreateImage(display, winv->visual, 8, ZPixmap, 0,
				dbuf, zoom_info.width, zoom_info.height, 8, 0);
    if (zoom_blend_image == NULL) {
	fprintf(stderr, " Error creating zoom image! \n");
	return;
    }
}
