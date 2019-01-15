
/*
 * display.c -- routines for maintaining the display
 */

#include "ui.h"
#include "display.h"
#include "sm.h"
#include <math.h>

struct img_data *orig_img = NULL;
XImage   *orig_ximg = NULL, *disp_ximg = NULL;
int       shrink_fac = 0;	/* shrink factor.  Shrinks by 2^sfac. */

static int cmin, cmax;		/* colormap min and max */
static float gam = 1.0;

/* routine to initialize the display window */
init_display(ras)
    struct img_data *ras;
{
    int       cshape = XC_crosshair;
    Cursor    cursor;
    XColor    stout, blk;

    /* initialize the colormap */
    cmin = ras->minv;
    cmax = ras->maxv;
    xv_set(disp_ctrl->cmap_min,
	   PANEL_VALUE, cmin,
	   PANEL_MIN_VALUE, cmin,
	   PANEL_MAX_VALUE, cmax,
	   NULL);
    xv_set(disp_ctrl->cmap_max,
	   PANEL_VALUE, cmax,
	   PANEL_MIN_VALUE, cmin,
	   PANEL_MAX_VALUE, cmax,
	   NULL);

    cmap_init();

    /* initialize the cursor color */
    stout.red = stout.blue = 0;
    stout.green = (255 << 8);
    stout.pixel = standout;
    stout.flags = blk.flags = DoRed | DoGreen | DoBlue;
    blk.red = blk.green = blk.blue = 0;
    blk.flags = DoRed | DoGreen | DoBlue;
    blk.pixel = pallet[BLACK].pixel;

    cursor = XCreateFontCursor(display, cshape);
    XDefineCursor(display, img_win->d_xid, cursor);
    XRecolorCursor(display, cursor, &stout, &blk);

}

XImage   *
mk_x_img(image, xim, use_shrink)
    struct img_data *image;
    XImage   *xim;
    int       use_shrink;	/* boolean -- use shrink factor? */
{
    char     *xim_data = NULL, *dptr = NULL, *mapbuf = NULL;
    register int x, y, shrink, i;
    int       width, height;

    if (xim != NULL) {
	free(xim->data);
	XDestroyImage(xim);
    }
    if (use_shrink && shrink_fac > 0) {
	mapbuf = calloc(image->width * image->height, sizeof(char));
	map_image_to_colors((byte *) mapbuf, (byte *) image->lut,
			    image->width * image->height);

	/* shink by powers of two */
	shrink = shrink_fac << 1;
	fprintf(stderr, "Shrinking image by %d ... \n", shrink);

	width = image->width / shrink;
	height = image->height / shrink;

	xim_data = calloc(width * height, sizeof(char));
	dptr = xim_data;

	i = 0;
	for (y = 0; y < image->height; y += shrink) {
	    for (x = 0; x < image->width; x += shrink) {
		/* just sub-sample for now, really should to averaging! */
		dptr[i] = mapbuf[y * image->width + x];
		i++;
	    }
	}
	free(mapbuf);
    } else {
	width = image->width;
	height = image->height;
	xim_data = calloc(width * height, sizeof(char));
	map_image_to_colors((byte *) xim_data, (byte *) image->lut,
			    width * height);
    }

    switch (depth) {
    case 8:
	xim = XCreateImage(display, winv, depth,
			   ZPixmap, 0, xim_data, width, height, 8, 0);
	break;
    case 24:
    case 32:
	xim = XCreateImage(display, winv, depth,
			   ZPixmap, 0, xim_data, width, height, 32, 0);
	break;
    }
    if (!xim) {
	fprintf(stderr, "Error: couldn't create ximage!");
	return (NULL);
    }
    return (xim);
}

/*************************************************************/
void
disp_img()
{

#ifdef DEBUG
    printf("displaying image\n");
#endif

    xv_set(img_win->d_win,
	   XV_SHOW, TRUE, FRAME_CLOSED, FALSE, NULL);

    xv_set(img_win->d_win,
	   XV_WIDTH, MIN((disp_ximg->width + SCROLL_BAR_SIZE), 512),
	   XV_HEIGHT, MIN((disp_ximg->height + SCROLL_BAR_SIZE), 512),
	   NULL);

    xv_set(img_win->d_canv,
	   CANVAS_AUTO_EXPAND, FALSE,
	   CANVAS_AUTO_SHRINK, FALSE,
	   OPENWIN_AUTO_CLEAR, TRUE,
	   CANVAS_FIXED_IMAGE, FALSE,
	   NULL);
    xv_set(img_win->d_canv,
	   CANVAS_WIDTH, disp_ximg->width,
	   CANVAS_HEIGHT, disp_ximg->height,
	   NULL);

    imgwin_repaint_proc(img_win->d_canv, img_win->d_paintwin, display,
			img_win->d_xid, NULL);
}

/*
 * Repaint callback function for `imgcanv'.
 */
imgwin_repaint_proc(canvas, paint_window, display, w, rects)
    Canvas    canvas;
    Xv_window paint_window;
    Display  *display;
    Window    w;
    Xv_xrectlist *rects;
{
#ifdef DEBUG
    printf(" in imgwin_repaint_proc \n");
#endif

    if (disp_ximg == NULL)
	return;

    XPutImage(display, w, gc, disp_ximg, 0, 0, 0, 0,
	      disp_ximg->width, disp_ximg->height);
    draw_log();
}

/***************************************************************/
imgwin_resize_proc(win, event, arg, type)
    Xv_window win;
    Event    *event;
    Notify_arg arg;
    Notify_event_type type;
{
#ifdef DEBUG
    fprintf(stderr, "genial: imgwin_resize_proc: event %d\n", event_id(event));
#endif

    if (event_id(event) == WIN_RESIZE) {
	if ((int) xv_get(img_win->d_win, XV_WIDTH) > disp_ximg->width +
	    SCROLL_BAR_SIZE)
	    xv_set(img_win->d_win,
		   XV_WIDTH, disp_ximg->width + SCROLL_BAR_SIZE, NULL);

	if ((int) xv_get(img_win->d_win, XV_HEIGHT) > disp_ximg->height +
	    SCROLL_BAR_SIZE)
	    xv_set(img_win->d_win,
		   XV_HEIGHT, disp_ximg->height + SCROLL_BAR_SIZE, NULL);
    }
    return;
}

/*****************************************************/

/* the following function sets / changes the gamma value */
set_gam(v)
    float     v;
{
    gam = v;
    if (getstate() == IMG_UNLOADED)
	return;

    apply_gamma(gam, 0, 0, NCOLORS, 0, 0);

    map_image_to_colors((byte *) disp_ximg->data, (byte *) orig_img->lut,
			disp_ximg->width * disp_ximg->height);

    imgwin_repaint_proc(img_win->d_canv, img_win->d_paintwin, display,
			img_win->d_xid, NULL);
}

/*****************************************************/
set_cmin(v)
    int       v;
{
    cmin = v;
    if (getstate() == IMG_UNLOADED)
	return;

    /*
     * this just sets all pixels outside the range to black. Might be better
     * to actually rescale the image using the new values: ie: and min and
     * max arguements to 'make_lut' (cmap.c), then call build_colomap again
     * (SLOW!)
     */
    remap_image(disp_ximg->data, orig_img->data, orig_img->lut,
		orig_img->width * orig_img->height,
		cmin, cmax);

    imgwin_repaint_proc(img_win->d_canv, img_win->d_paintwin, display,
			img_win->d_xid, NULL);

}

/*****************************************************/
set_cmax(v)
    int       v;
{
    cmax = v;
    if (getstate() == IMG_UNLOADED)
	return;

    remap_image(disp_ximg->data, orig_img->data, orig_img->lut,
		orig_img->width * orig_img->height,
		cmin, cmax);

    imgwin_repaint_proc(img_win->d_canv, img_win->d_paintwin, display,
			img_win->d_xid, NULL);
}

/************************************************/
/* map image to color indexes */
remap_image(mapped_image, data, lut, size, min, max)
    byte     *mapped_image, *data, *lut;
    int       size;
{
    register int i;

    for (i = 0; i < size; i++) {
	if (winv->class == TrueColor) {	/* 24-bit systems */
#ifdef LATER
	    /* need to do something here!! */
#endif

	} else {
	    if (data[i] < min || data[i] > max)
		mapped_image[i] = pallet[BLACK].pixel;
	    else
		mapped_image[i] = (byte) colors[lut[i]];
	}
    }
}

/****************************************************************
 * the following routines are for dealing with display images which differ
 *  from the original image
 *
 */

/* set_shrink_fac() sets the shrink factor to n.  This shrinks the image by
 *  2^n.
 *
 */
set_shrink_fac(n)
    int       n;
{
    shrink_fac = n;
    if (getstate() == IMG_UNLOADED)
	return;
    disp_ximg = mk_x_img(orig_img, disp_ximg, 1);
    disp_img();
}

/* display to image coordinates */
/* dtoi_pt() and itod_pt() are routines that convert XPoints from display to
image coordinates and vice versa. dtoi() and itod() do this same conversion
with simple ints, and are useful for converting things like the width of a
cross */

dtoi_pt(dsp, img)
    XPoint   *dsp, *img;
{
    img->x = (dsp->x << shrink_fac);
    img->y = (dsp->y << shrink_fac);
}

/* and the reverse */
itod_pt(img, dsp)
    XPoint   *img, *dsp;
{
    dsp->x = (img->x >> shrink_fac);
    dsp->y = (img->y >> shrink_fac);
}

int
dtoi(v)
    int       v;
{
    return (v << shrink_fac);
}

int
itod(v)
    int       v;
{
    return (v >> shrink_fac);
}
