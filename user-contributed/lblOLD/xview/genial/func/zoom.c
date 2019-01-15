/*
 * zoom.c -- function to zoom in on a rectangular region
 */

#include <stdio.h>
#include "display.h"
#include "ui.h"
#include "common.h"
#include "log.h"
#include "reg.h"
#include "zoom_ui.h"

/* #define ZOOM_DEBUG */

GC        zgc;
struct zcontext *zoom_by_lid(), *zoom_by_win(), *zoom_by_pwin();

/***********************************************************/
zoom_init()
{
    setrtype(BOX);
    reg_setdom(NONE);

    lab_info("Please select a rectangular region to zoom", 1);
    lab_info("Hit <eval> when finished", 2);

    return 0;
}

/***********************************************************/
zoom_eval()
{
    struct zcontext *curzoom;

#ifdef ZOOM_DEBUG
    printf("zoom eval \n");
#endif

    curzoom = newzoom();
    comp_zoom(curzoom);
    draw_zoom(curzoom);
    return 0;
}

/***********************************************************/
zoom_clear(id)
    int       id;
{
    struct zcontext *zoom;

    zoom = zoom_by_lid(id);
    if (zoom == NULL) {
	fprintf(stderr, "zoom # %d not found \n", id);
	return 0;
    }
    delzcontext(zoom);
    return 0;
}

/***************************************************************/
struct zcontext *
newzoom()
{
    struct zcontext *zoom;

#ifdef ZOOM_DEBUG
    printf("newzoom\n");
#endif

    if (curfunc->zoom != NULL)
	free(curfunc->zoom);

    zoom_mk_cursors();
    zoom = (struct zcontext *) malloc(sizeof(struct zcontext));
    curfunc->zoom = zoom;
    makezoom(zoom);
    return zoom;
}

/*******************************************************************/
makezoom(zoom)
    struct zcontext *zoom;
{
    XGCValues gcval;
    char      title[80];
    u_char   *zbuf;

#ifdef ZOOM_DEBUG
    printf("makezoom \n");
#endif

    zoom->display = zoom_zmwin_objects_initialize(NULL, base_win->ctrlwin);
    zoom->paintwin = canvas_paint_window(zoom->display->zmcanv);
    zoom->zxid = (XID) xv_get(zoom->paintwin, XV_XID, NULL);

    /* set up a GC for the window */
    gcval.foreground = BlackPixel(display, DefaultScreen(display));
    gcval.background = WhitePixel(display, DefaultScreen(display));
    gcval.clip_mask = None;
    zgc = XCreateGC(display, zoom->zxid,
		    GCForeground | GCBackground | GCClipMask, &gcval);

    zoom->zmfac = 2;

    if (curfunc->reg->r_plist->pt.x < curfunc->reg->r_plist->next->pt.x) {
	zoom->p1.x = curfunc->reg->r_plist->pt.x;
	zoom->p2.x = curfunc->reg->r_plist->next->pt.x;
    } else {
	zoom->p2.x = curfunc->reg->r_plist->pt.x;
	zoom->p1.x = curfunc->reg->r_plist->next->pt.x;
    }
    if (curfunc->reg->r_plist->pt.y < curfunc->reg->r_plist->next->pt.y) {
	zoom->p1.y = curfunc->reg->r_plist->pt.y;
	zoom->p2.y = curfunc->reg->r_plist->next->pt.y;
    } else {
	zoom->p2.y = curfunc->reg->r_plist->pt.y;
	zoom->p1.y = curfunc->reg->r_plist->next->pt.y;
    }

    zoom->can_width = abs(zoom->p2.x - zoom->p1.x) * zoom->zmfac;
    zoom->can_height = abs(zoom->p2.y - zoom->p1.y) * zoom->zmfac;

#ifdef ZOOM_DEBUG
    printf("zooming window of size %d x %d \n",
	   abs(zoom->p2.x - zoom->p1.x), abs(zoom->p2.y - zoom->p1.y));
    printf("zoom factor: %d \n", zoom->zmfac);
    printf("creating zoom ximage: size %d x %d \n",
	   zoom->can_width, zoom->can_height);
#endif

    switch (depth) {
    case 8:
	zbuf = (u_char *) malloc((unsigned) zoom->can_width * zoom->can_height);
	zoom->zim = XCreateImage(display, winv, depth,
				 ZPixmap, 0, zbuf, zoom->can_width,
				 zoom->can_height, 8, 0);
	break;
    case 24:
    case 32:
	zbuf = (u_char *) malloc(zoom->can_width * zoom->can_height * 4);
	zoom->zim = XCreateImage(display, winv, depth,
				 ZPixmap, 0, zbuf, zoom->can_width,
				 zoom->can_height, 32, 0);
	break;
    }
    if (!zoom->zim) {
	fprintf(stderr, "Error: couldn't create ximage!");
	return (-1);
    }
    sprintf(title, "Zoom: %d", curfunc->id);
    xv_set(zoom->display->zmwin, XV_LABEL, title, NULL);

    return (1);
}

/********************************************************************/
/* comp_zoom: actually do the zooming into the XImage structure */
/* no, this isn't the most efficient implementation in the world, but we can
   change that if we HAVE to (a faster method is to use bcopy to
   replicate entire rows at a time)
   */
int
comp_zoom(zoom)
    struct zcontext *zoom;
{
    u_char   *zbuf;
    u_long    v;
    XPoint    p1, p2;
    int       zmfac;
    int       sx, ex, sy, ey;
    register int i, j, x, y;
    void      pixel_replicate();

#ifdef ZOOM_DEBUG
    printf("comp_zoom \n");
#endif

    set_watch_cursor();
    p1.x = zoom->p1.x;
    p1.y = zoom->p1.y;
    p2.x = zoom->p2.x;
    p2.y = zoom->p2.y;
    zmfac = zoom->zmfac;

    if ((zoom->zim->width != zoom->can_width) ||
	(zoom->zim->height != zoom->can_height)) {
	free(zoom->zim->data);
	XDestroyImage(zoom->zim);
	zbuf = (u_char *) malloc((unsigned) zoom->can_width * zoom->can_height);
	zoom->zim = XCreateImage(display, winv, depth,
				 ZPixmap, 0, zbuf,
				 zoom->can_width, zoom->can_height, 8, 0);
#ifdef ZOOM_DEBUG
	printf("re-creating zoom ximage: size %d x %d \n",
	       zoom->can_width, zoom->can_height);
#endif
    }
    for (y = p1.y; y < p2.y; y++) {
	for (x = p1.x; x < p2.x; x++) {
	    v = XGetPixel(orig_ximg, x, y);

	    sy = (y - p1.y) * zmfac;
	    ey = sy + zmfac;
	    for (j = sy; j < ey; j++) {
		sx = (x - p1.x) * zmfac;
		ex = sx + zmfac;
		for (i = sx; i < ex; i++) {
		    if (i >= zoom->can_width || j >= zoom->can_height) {
			fprintf(stderr, "Error: invalid zoom window location \n");
			return -1;
		    }
		    XPutPixel(zoom->zim, i, j, v);
		}
	    }
	}
    }

    unset_watch_cursor();

    return 1;
}

/*******************************************************************/

delzcontext(zoom)
    struct zcontext *zoom;
{
    xv_set(zoom->display->zmwin,
	   XV_SHOW, FALSE,
	   NULL);
    free(zoom->zim->data);
    XDestroyImage(zoom->zim);
}

/***********************************************************/
struct zcontext *
zoom_by_pwin(win)
    Xv_Window win;
{
    struct zcontext *zoom;
    struct logent *log;

    log = loghead;
    while (log != NULL) {
	zoom = log->zoom;
	if (zoom != NULL) {
	    if (zoom->paintwin == win)
		return zoom;
	}
	log = log->next;
    }
    return (NULL);
}

/***********************************************************/
struct zcontext *
zoom_by_win(win)
    Xv_Window win;
{
    struct zcontext *zoom;
    struct logent *log;

    log = loghead;
    while (log != NULL) {
	zoom = log->zoom;
	if (zoom != NULL) {
	    if (zoom->display->zmwin == win)
		return zoom;
	}
	log = log->next;
    }
    return (NULL);
}

/*****************************************************************/
struct zcontext *
zoom_by_lid(id)
    int       id;
{
    struct logent *log;

    log = loghead;
    while (log != NULL) {
	if (log->id == id)
	    return (log->zoom);
	log = log->next;
    }
    fprintf(stderr, "zoom_by_lid: not found \n");
    return NULL;
}

/***********************************************************/
draw_zoom(zoom)
    struct zcontext *zoom;
{
    void      zmcanv_repaint_proc();

#ifdef ZOOM_DEBUG
    printf("draw_zoom \n");
#endif

    set_watch_cursor();
    xv_set(zoom->display->zmwin,
	   XV_SHOW, TRUE, FRAME_CLOSED, FALSE, NULL);

    xv_set(zoom->display->zmwin,
	   XV_WIDTH, MIN((zoom->can_width + SCROLL_BAR_SIZE), 512),
	   XV_HEIGHT, MIN((zoom->can_height + SCROLL_BAR_SIZE), 512),
	   NULL);

    xv_set(zoom->display->zmcanv,
	   CANVAS_AUTO_SHRINK, FALSE,
	   CANVAS_AUTO_EXPAND, FALSE,
	   OPENWIN_AUTO_CLEAR, TRUE,
	   CANVAS_FIXED_IMAGE, FALSE,
	   NULL);

    xv_set(zoom->display->zmcanv,
	   CANVAS_WIDTH, zoom->can_width,
	   CANVAS_HEIGHT, zoom->can_height,
	   NULL);

#ifdef ZOOM_DEBUG
    printf("set window size: %dx%d \n",
	   MIN((zoom->can_width + SCROLL_BAR_SIZE), 512),
	   MIN((zoom->can_height + SCROLL_BAR_SIZE), 512));
    printf("set canvas size: %dx%d \n", zoom->can_width, zoom->can_height);
#endif

    zmcanv_repaint_proc(zoom->display->zmcanv, zoom->paintwin,
			display, zoom->zxid, NULL);
    unset_watch_cursor();
}

/***************************************************************/
/*
 * Repaint callback function for `zmcanv'.
 */
void
zmcanv_repaint_proc(canvas, paint_window, display, xid, rects)
    Canvas    canvas;
    Xv_window paint_window;
    Display  *display;
    Window    xid;
    Xv_xrectlist *rects;
{
    struct zcontext *zoom;

    zoom = zoom_by_pwin(paint_window);
    if (zoom == NULL || zoom->zim == NULL) {
	return;
    }
#ifdef ZOOM_DEBUG
    printf("zoom_repaint_proc: putimage size; %d x %d \n", zoom->can_width,
	   zoom->can_height);
#endif

    XPutImage(display, xid, zgc, zoom->zim, 0, 0, 0, 0,
	      zoom->can_width, zoom->can_height);
}

/**************************************************************/
void
zoom_resize_proc(win, event, arg, type)
    Xv_window win;
    Event    *event;
    Notify_arg arg;
    Notify_event_type type;
{
    struct zcontext *zoom;
    int       width, height;

    if (logtail == NULL)
	return;

    if ((zoom = zoom_by_win(win)) == NULL) {
	return;
    }
#ifdef ZOOM_DEBUG
    fprintf(stderr, "genial: got zoom resize event \n");
#endif

    width = (int) xv_get(zoom->display->zmwin, XV_WIDTH, NULL);
    height = (int) xv_get(zoom->display->zmwin, XV_HEIGHT, NULL);

    if (width > zoom->can_width + SCROLL_BAR_SIZE) {
	xv_set(zoom->display->zmwin,
	       XV_WIDTH, zoom->can_width + SCROLL_BAR_SIZE, NULL);
    }
    if (height > zoom->can_height + SCROLL_BAR_SIZE) {
	xv_set(zoom->display->zmwin,
	       XV_HEIGHT, zoom->can_height + SCROLL_BAR_SIZE, NULL);
    }
    return;
}

/***********************************************************************/
Notify_value
zmcanv_event_proc(win, event, arg, type)
    Xv_window win;
    Event    *event;
    Notify_arg arg;
    Notify_event_type type;
{
    struct zcontext *zoom;
    int       x, y, dir;

    if ((zoom = zoom_by_pwin(win)) == NULL) {
	fprintf(stderr, "zoom_event_prov: win not found \n");
	return notify_next_event_func(win, (Notify_event) event, arg, type);
    }

    x = event_x(event);
    y = event_y(event);
    /* which direction is the user heading */
    dir = direction(x, y, zoom);
    /* make the cursor reflect that */
    setcursor(win, dir);

   /* middle button = pan */
    if (event_is_down(event) && event_id(event) == BUT(2)) {
	/*
	 * move in that direction if possible
	 */
	zoom_xlate(zoom, x, y, dir);
	draw_zoom(zoom);
    }

    /* left button = zoom in */
    if (event_is_down(event) && event_id(event) == BUT(1)) {
	/*
	 * double the scale factor, keep region size same
	 */
	if (zoom->zmfac >= 8)
	    return notify_next_event_func(win, (Notify_event) event, arg, type);

	zoom->zmfac++;
	zoom->can_width = abs(zoom->p2.x - zoom->p1.x) * zoom->zmfac;
	zoom->can_height = abs(zoom->p2.y - zoom->p1.y) * zoom->zmfac;

	comp_zoom(zoom);
	draw_zoom(zoom);
    }

    /* right button = zoom out */
    if (event_is_down(event) && event_id(event) == BUT(3)) {
	/*
	 * halve the scale factor, keep region size same
	 */
	if (zoom->zmfac <= 1)
	    return notify_next_event_func(win, (Notify_event) event, arg, type);

	zoom->zmfac--;
	zoom->can_width = abs(zoom->p2.x - zoom->p1.x) * zoom->zmfac;
	zoom->can_height = abs(zoom->p2.y - zoom->p1.y) * zoom->zmfac;

	comp_zoom(zoom);
	draw_zoom(zoom);
    }
    return notify_next_event_func(win, (Notify_event) event, arg, type);
}

/*****************************************************************/
/* routine to translate zoom to (x,y) in the direction specified */
zoom_xlate(zoom, x, y, dir)
    struct zcontext *zoom;
    int       x, y, dir;
{
    int       cx, cy, width, height;
    int       box_width, box_height;
    struct region *treg;	/* temporary region */

    width = (int) xv_get(zoom->display->zmwin, XV_WIDTH, NULL);
    height = (int) xv_get(zoom->display->zmwin, XV_HEIGHT, NULL);

    box_width = abs(zoom->p2.x - zoom->p1.x);
    box_height = abs(zoom->p2.y - zoom->p1.y);

    cx = x - width / 2;
    cy = y - height / 2;
    treg = newreg(BOX);
    if ((dir == LEFT) || (dir == RIGHT)) {
	zoom->p1.x += cx;
	zoom->p2.x = zoom->p1.x + box_width;
    } else {
	zoom->p1.y += cy;
	zoom->p2.y = zoom->p1.y + box_height;
    }
    /* make sure the numbers here are within reasonable bounds */
    zoom_bound(zoom, box_width, box_height);

    reg_addpt(treg, zoom->p1.x, zoom->p1.y);
    reg_addpt(treg, zoom->p2.x, zoom->p2.y);
    draw_box(treg);
    log_chreg(curfunc->prev->id, treg);
    comp_zoom(zoom);
    draw_zoom(zoom);
}

/*****************************************************************/
/* direction returns the direction the user is heading in the most */
int
direction(x, y, zoom)
    int       x, y;
    struct zcontext *zoom;
{
    int       cx, cy;		/* x and y in coordinate form */
    int       width, height;

    width = (int) xv_get(zoom->display->zmwin, XV_WIDTH, NULL);
    height = (int) xv_get(zoom->display->zmwin, XV_HEIGHT, NULL);

    cx = x - width / 2;
    cy = y - height / 2;
    if (abs(cx) > abs(cy)) {
	return ((cx > 0) ? RIGHT : LEFT);
    } else
	return ((cy > 0) ? DOWN : UP);
}

/*****************************************************************/
/* zoom_bound() makes sure that p1 and p2 members of the zoom structure are
within reasonable bounds
*/

zoom_bound(zoom, box_width, box_height)
    struct zcontext *zoom;
    int       box_width, box_height;
{

    box_width = abs(zoom->p2.x - zoom->p1.x);
    box_height = abs(zoom->p2.y - zoom->p1.y);

    /* if p1 is < 0, shift the whole region forward */
    if (zoom->p1.x < 0) {
	zoom->p1.x = 0;
	zoom->p2.x = box_width;
    }
    if (zoom->p1.y < 0) {
	zoom->p1.y = 0;
	zoom->p2.y = box_height;
    }
    /* if p2 is past the edge of the image, shift the whole region back */
    if (zoom->p2.x > orig_ximg->width) {
	zoom->p1.x = orig_ximg->width - box_width;
	zoom->p2.x = orig_ximg->width;
    }
    if (zoom->p2.y > orig_ximg->height) {
	zoom->p1.y = orig_ximg->height - box_height;
	zoom->p2.y = orig_ximg->height;
    }
}
