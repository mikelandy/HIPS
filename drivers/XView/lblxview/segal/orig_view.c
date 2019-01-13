
/*
 *  view.c            -Brian Tierney,  LBL
 *  orig_view.c       -Bryan Skene,  LBL
 *  for use with segal
 *  these routines (ripped off from view.c) are for displaying the original
 *  image.
 */

#include "common.h"

#include "orig_view_ui.h"

/* Allocation :) */
orig_view_win_objects *orig_view_win;

#include "segal.h"

/* #define DEBUG */
/* #define SHOW_COLORS  */

/************************************************************/
void
orig_view_win_init(owner)
    Xv_opaque owner;
{
    void      make_cms_colormap();

    orig_view_win = orig_view_win_objects_initialize(NULL, owner);
}


/**************************************************************/
void
orig_view_setup()
{
    Rect      srect, ovrect;	/* frame rect structure for segal win and
				 * view win */

    /* set size */
/*  don't need to set canvas size
    (void) xv_set(orig_view_win->canvas, XV_WIDTH, segal.cols + 1, NULL);
    (void) xv_set(orig_view_win->canvas, XV_HEIGHT, segal.rows + 1, NULL);
*/
    (void) xv_set(orig_view_win->win, XV_WIDTH, segal.cols + 1, NULL);
    (void) xv_set(orig_view_win->win, XV_HEIGHT, segal.rows + 1, NULL);

    if((int) xv_get(orig_view_win->win, XV_SHOW, NULL) == TRUE)
	return;

    (void) xv_set(orig_view_win->win, XV_SHOW, TRUE, NULL);

    /*
     * set location: NOTE: to set the location of a frame using
     * 'frame_set_rect'  (at least under the twm window manager) XV_SHOW must
     * be TRUE first
     */

    /* set orig_view window location relative to segal window */
    frame_get_rect(segal_win->win, &srect);

    if (srect.r_top > 0 && srect.r_left > 0) { /* only if segal window
						  is on screen */
	frame_get_rect(orig_view_win->win, &ovrect);
	ovrect.r_top = srect.r_top;
	ovrect.r_left = srect.r_left + srect.r_width + 2;
	frame_set_rect(orig_view_win->win, &ovrect);
    }
}


/*************************************************************
 * Repaint callback function for `canvas'.
 */
void
orig_image_repaint_proc()
{
	if (xv_get(orig_view_win->win, XV_SHOW, NULL) == FALSE)
		return;

	XPutImage(display, orig_view_xid, gc, image, 0, 0, 0, 0,
		  image->width, image->height);

}


/*************************************************************
 * Notify Handler for 'Original'
 */
void
orig_map_proc(item, event)
	Panel_item      item;
	Event           *event;
{
	segal_win_objects       *ip = (segal_win_objects *) xv_get(item,
					XV_KEY_DATA, INSTANCE);

	/* Map / Unmap toggle */
	if (xv_get(orig_view_win->win, XV_SHOW, NULL) == FALSE) {
		(void) xv_set(orig_view_win->win, XV_WIDTH, segal.cols + 1, NULL);
		(void) xv_set(orig_view_win->win, XV_HEIGHT, segal.rows + 1, NULL);
		xv_set(orig_view_win->win, XV_SHOW, TRUE,
			NULL);
		XPutImage(display, orig_view_xid, gc, image, 0, 0, 0, 0,
			image->width, image->height);
	}
	else xv_set(orig_view_win->win, XV_SHOW, FALSE,
			NULL);
}
