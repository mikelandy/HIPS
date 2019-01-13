/*
 * histo.c -- routines for finding histograms in subregions of an image
 *
 */

#include <math.h>
#include "display.h"
#include "common.h"
#include "ui.h"
#include "log.h"
#include "reg.h"
#include "histo_ui.h"
#include "scale.h"

GC        hgc = NULL;

struct hcontext *newhist(), *histo_by_win(), *histo_by_panelwin(), *histo_by_lid();

int       color_mode;		/* color mode / magnify mode flag */

/*********************************************************/
histo_init()
{
    if (histo_ctrl == NULL) {
	histo_ctrl = histo_histo_ctrl_objects_initialize(NULL,
							 base_win->ctrlwin);
        histo_control_init();
    }
    reg_setdom(AREA);

    clear_info();
    lab_info("Please select a region for histogram", 1);
    lab_info("Hit <eval> to create histogram.", 2);

    return 0;
}

/*********************************************************/
histo_eval()
{
    char      title[80];
    struct hcontext *curhist;
    XGCValues gcval;
    XFontStruct *fs;

    curhist = newhist();
    xv_set(histo_ctrl->histno,
	   PANEL_VALUE, curfunc->id - 1,
	   NULL);
    if (hgc == NULL) {
	/* set up a GC for the window */
	gcval.foreground = BlackPixel(display, DefaultScreen(display));
	gcval.background = WhitePixel(display, DefaultScreen(display));
	gcval.clip_mask = None;
	hgc = XCreateGC(display, curhist->hxid,
			GCForeground | GCBackground | GCClipMask, &gcval);

	if (!(fs = XLoadQueryFont(display, "9x15"))) {
	    fprintf(stderr, "Font 9x15 not found, trying \'fixed\' \n");
	    if (!(fs = XLoadQueryFont(display, "fixed"))) {
		fprintf(stderr, "error geting fonts, exitting...\n");
		exit(-1);
	    }
	}
	XSetFont(display, hgc, fs->fid);

    }
    if (comp_hist(curhist) > 0) {
	sprintf(title, "Histogram: %d", curfunc->id);
	xv_set(curhist->histo_display->display,
	       XV_LABEL, title,
	       XV_SHOW, TRUE, FRAME_CLOSED, FALSE,
	       NULL);
    }
    return 0;
}

/*************************************************************/
int
histo_change(id)
    int       id;
{
    struct hcontext *curhist;

    printf(" in histo change: .. \n");

    curhist = histo_by_lid(id);
    if (curhist == NULL) {
	XBell(display, 0);
	printf("no such histogram!\n");
	return -1;
    }
    if (comp_hist(curhist) > 0) {
	xv_set(curhist->histo_display->display,
	       XV_SHOW, TRUE, FRAME_CLOSED, FALSE,
	       NULL);
    }
    return 1;
}


/*************************************************************/
/* routine to find an unused histo context or to allocate a new one if none
   are available */
struct hcontext *
newhist()
{
    struct hcontext *hist;

    if (curfunc->hist != NULL)
	free(curfunc->hist);

    hist = (struct hcontext *) malloc(sizeof(struct hcontext));
    makehist(hist);
    curfunc->hist = hist;
    return hist;

}

/**************************************************************/
makehist(ctx)
    struct hcontext *ctx;
{
    int       i;

    ctx->histo_display = histo_display_objects_initialize(NULL,
							  base_win->ctrlwin);
    ctx->paintwin = canvas_paint_window(ctx->histo_display->histocanv);
    ctx->hxid = (XID) xv_get(ctx->paintwin, XV_XID, NULL);
    ctx->rtype = getrtype();

    for (i = 0; i < NUM_BUCKETS; i++) {
	ctx->countvec[i] = 0;
	ctx->lbtab[i].npts = 0;
	ctx->lbtab[i].asize = 0;
    }
    for (i = 0; i < NUM_SUB; i++) {
	ctx->interval[i].lower = 0;
	ctx->interval[i].upper = NUM_BUCKETS - 1;
	ctx->interval[i].islit = ctx->interval[i].isactive = 0;
    }
    ctx->subnum = 0;
    ctx->minb = 0;
    ctx->maxb = NUM_BUCKETS - 1;
    ctx->pbuf = (XPoint *) malloc(sizeof(XPoint) * BSIZE);
    ctx->pbsize = BSIZE;
    ctx->pblen = 0;
    ctx->xglab = ctx->yglab = NULL;
}

/*****************************************/
show_histogram(ctx)		/* for debugging */
    struct hcontext *ctx;
{
    int       i;

    for (i = 0; i < NUM_BUCKETS; i++)
	if (ctx->countvec[i] != 0) {
	    printf("bucket: %d, count %d, gray value: %d \n",
		   i, ctx->countvec[i], btog(i, ctx));
	}
}

/*************************************/
histo_clear(id)
    int       id;
{
    int       i;
    struct hcontext *hist;

#ifdef DEBUG
    printf("clearing histogram %d \n", id);
#endif

    hist = histo_by_lid(id);
    if (hist == NULL) {
	printf("histo_clear: no such histogram! (id=%d)\n", id);
	return -1;
    }
    for (i = 0; i < NUM_SUB; i++) {
	if (hist->interval[i].islit)
	    unlite(hist, i);
    }

    delhcontext(hist);

    if (id == 1) {
	xv_set(histo_ctrl->histo_ctrl,
	       FRAME_CMD_PUSHPIN_IN, FALSE, NULL);
	xv_set(histo_ctrl->histo_ctrl,
	       XV_SHOW, FALSE,
	       NULL);
    }
    return 0;
}

/*************************************************************/
histo_reset()
{
    return 0;
}

/*************************************************************/
histo_color_opts(item, event)
    Panel_item item;
    Event    *event;
{
    void      restore_proc();
    struct hcontext *hist;

    hist = histo_by_panelwin(event_window(event));
    if (hist == NULL)
	return;

    if (((int) xv_get(histo_ctrl->histo_ctrl, XV_SHOW, NULL)) == FALSE) {
	xv_set(histo_ctrl->histo_ctrl,
	       XV_SHOW, TRUE, FRAME_CLOSED, FALSE, NULL);
	xv_set(histo_ctrl->histo_ctrl,
	       FRAME_CMD_PUSHPIN_IN, TRUE, NULL);
	xv_set(hist->histo_display->magnify,
	       PANEL_INACTIVE, TRUE,
	       NULL);
	xv_set(hist->histo_display->unmagnify,
	       PANEL_INACTIVE, TRUE,
	       NULL);
        histo_control_init();
	xv_set(histo_ctrl->sel1, PANEL_VALUE, 1, NULL);
	hist->subnum = 0;
	xv_set(histo_ctrl->histno, PANEL_VALUE, (curfunc->id - 1), NULL);
	restore_proc(item, event);
	color_mode = 1;

    } else {
	xv_set(histo_ctrl->histo_ctrl,
	       FRAME_CMD_PUSHPIN_IN, FALSE, NULL);
	xv_set(histo_ctrl->histo_ctrl, XV_SHOW, FALSE, NULL);
	xv_set(hist->histo_display->magnify,
	       PANEL_INACTIVE, FALSE,
	       NULL);
	xv_set(hist->histo_display->unmagnify,
	       PANEL_INACTIVE, FALSE,
	       NULL);
	color_mode = 0;
	hist->subnum = 0;
    }

    return;
}

/*************************************************************/
/* routine to do the actual computation involved in figuring out what a
   histogram should look like */
/* histogram computation can be a very unpleasant thing.  Since have to do a
   flood_fill, there is no nice way to avoid copying points.  It is, in fact,
   more efficient to copy points to a temporary buffer than it is to recompute
   the flood_fill. And since we have to find the extrema of the gray values
   within the histogram region, we have to make at least two passes over the
   data.  */
int
comp_hist(hist)
    struct hcontext *hist;
{
    /* fill the point buffer based on region type */
    if (fill_pbuf(hist) <= 0) {
	XBell(display, 0);
	fprintf(stderr, "Invalid region! \n");
	return -1;
    }
    /* find extrema of points */
    h_extrema(hist);
    /* fill the actual buckets and lookback table */
    fill_cvec(hist);
    /* and draw it */
    draw_hist(hist);

    return 1;
}

/*************************************************************/
/*
 * fill_pbuf() -- fills the pbuf structure with points corresponding to the
 * selected region
 */
int
fill_pbuf(hist)
    struct hcontext *hist;
{
    int       rval;
    switch (hist->rtype) {
    case BOX:
    case POLYGON:
    case CLSPLINE:
	rval = flood_fill(curfunc->reg, &hist->pbuf, &hist->pbsize, &hist->pblen);
	break;
    }
    return (rval);
}



/*************************************************************/
draw_hist(hist)
    struct hcontext *hist;
{
    histo_repaint_proc(hist->histo_display->histocanv,
		       hist->paintwin, display, hist->hxid, NULL);
}

/*************************************************************/
/*
 * h_extrema() -- extrema of the points in a histogram
 *
 */
h_extrema(hist)
    struct hcontext *hist;
{
    int       i;
    u_long    v, max, min;

    max = min = getpix(orig_img, hist->pbuf->x, hist->pbuf->y);
    for (i = 0; i < hist->pblen; i++) {
	v = getpix(orig_img, hist->pbuf[i].x, hist->pbuf[i].y);
	if (v < min)
	    min = v;
	else if (v > max)
	    max = v;
    }
    hist->minv = (int) min;
    hist->maxv = (int) max;

#ifdef DEBUG
    printf("gray values: min: %d, max: %d \n", min, max);
#endif
}

/*************************************************************/
/* fill_cvec() -- fill the countvec filed in the histogram context structure */
fill_cvec(hist)
    struct hcontext *hist;
{
    int       i, index;
    float     delta;
    unsigned long g;
    char      message[80];
    int       total = 0;

    delta = (float) (NUM_BUCKETS - 1) / (float) (hist->maxv - hist->minv);
    for (i = 0; i < hist->pblen; i++) {
	g = getpix(orig_img, hist->pbuf[i].x, hist->pbuf[i].y);
	/*
	 * not using gtob() here because faster to put delta computation out
	 * of the loop
	 */
	index = (int) (((g - hist->minv) * delta) + .5);
	if (index >= NUM_BUCKETS)
	    index = NUM_BUCKETS - 1;
	lbtab_add(hist, index, hist->pbuf + i);
	hist->countvec[index]++;
	if (hist->countvec[index] > hist->maxc)
	    hist->maxc = hist->countvec[index];
    }
    if (delta == 1) {
	hist->minb = 0;
	hist->maxb = hist->maxv - hist->minv;
    }
    for (i = 0; i < NUM_BUCKETS; i++)
	total += (btog(i, hist) * hist->countvec[i]);

    sprintf(message, "Total Intensity: %d", total);
    xv_set(hist->histo_display->message5,
	   PANEL_LABEL_STRING, message, NULL);
    panel_paint(hist->histo_display->message5, PANEL_CLEAR);
}

/*************************************************************/
lbtab_add(hist, i, pt)
    struct hcontext *hist;
    int       i;
    XPoint   *pt;
{
    struct lbent *lb;

    lb = hist->lbtab + i;
    if (lb->npts >= lb->asize) {
	if (lb->asize == 0) {
	    lb->asize = BSIZE;
	    lb->parr = (XPoint *) malloc(lb->asize * sizeof(XPoint));
	} else {
	    lb->asize += BSIZE;	/* increase size in increments of BSIZE */
	    lb->parr = (XPoint *) realloc(lb->parr, lb->asize * sizeof(XPoint));
	}
    }
    lb->parr[lb->npts].x = pt->x;
    lb->parr[lb->npts].y = pt->y;
    (lb->npts)++;
}

/***************************************************************/
/*
 * Repaint callback function for `histocanv'.
 */
histo_repaint_proc(canvas, paint_window, display, xid, rects)
    Canvas    canvas;
    Xv_window paint_window;
    Display  *display;
    Window    xid;
    Xv_xrectlist *rects;
{
    struct hcontext *hist;
    int       shmin, shmax;	/* horizontal and vertical scaling, max and
				 * min */
    XGCValues gcval;
    XPoint    src;
    double    xdelt, ydelt, xtmp;
    struct graph_lab *xglab, *yglab;
    char      message[80];
    int       xlen, ylen, i, wid, hgt;

    fputs("histo: histo_repaint_proc\n", stderr);
#ifdef DEBUG
#endif

    hist = histo_by_win(paint_window);
    if (hist == NULL) {
	fprintf(stderr, "histogram window not found \n");
	return;
    }
    if (hist->pblen == 0)
	return;

    XClearWindow(display, xid);
    gcval.foreground = BlackPixel((Display *) display,
				  DefaultScreen((Display *) display));
    XChangeGC(display, hgc, GCForeground, &gcval);
    XDrawRectangle(display, xid, hgc, SX, SY, GRWIDTH, GRHGT);

    shmin = btog(hist->minb, hist);
    shmax = btog(hist->maxb, hist);
    /* label the graph */
    if (hist->xglab != NULL) {
	free(hist->xglab);
	free(hist->yglab);
    }
    /* find the min and max gray value counts for this histogram */
    hist->maxc = c_max(hist->minb, hist->maxb, hist);
    hist->minc = c_min(hist->minb, hist->maxb, hist);
    xlen = build_glab(shmin, shmax, 10, GRWIDTH, &xglab);
    xlabelgraph(display, xid, hgc, xglab, xlen, SX, SY + GRHGT,
		GRWIDTH, GRHGT, HORIZONTAL, 1);

#ifdef STRONGDEBUG
    printf("scaled min:%d max: %d\n", shmin, shmax);
    glab_db(xglab, xlen);
#endif

    ylen = build_glab(hist->minc, hist->maxc, 10, GRHGT, &yglab);
    xlabelgraph(display, xid, hgc, yglab, ylen, SX + GRWIDTH, GRHGT + SY,
		GRWIDTH, GRHGT, VERTICAL, -1);

#ifdef STRONGDEBUG
    printf("hist->minc=%d hist->maxc==%d\n", hist->minc, hist->maxc);
    glab_db(yglab, ylen);
#endif

    hist->xglab = xglab;
    hist->yglab = yglab;
    hist->xglen = xlen;
    hist->yglen = ylen;

    xtmp = hist->xglab[hist->xglen - 1].val - hist->xglab[0].val;
    if (xtmp == 0.)
	return;
    xdelt = (float) GRWIDTH / xtmp;
    xtmp = yglab[ylen - 1].val - yglab[0].val;
    if (xtmp == 0.)
	return;
    ydelt = (float) GRHGT / xtmp;

    wid = irint(xdelt);
    if (wid < 1)
	wid = 1;
    for (i = hist->minb; i < hist->maxb; i++) {
	if (i >= NUM_BUCKETS) {
	    fprintf(stderr, "Histogram error!! \n");
	    exit(0);
	}
	hgt = irint((double) hist->countvec[i] * ydelt);
	src.x = (short) btox(i, hist);
	src.y = (short) GRHGT - hgt + SY;
#ifdef STRONGDEBUG
	printf("Rectangle: gray value %d, count: %d at: (%d,%d), width:%d, height: %d\n", btog(i, hist), hist->countvec[i], src.x, src.y, wid, hgt);
#endif
	XFillRectangle(display, xid, hgc, src.x, src.y, wid, hgt);
    }

    draw_histo_color_lines(hist);

    sprintf(message, "Pixel Value:   ");
    xv_set(hist->histo_display->message1,
	   PANEL_LABEL_STRING, message, NULL);
    sprintf(message, "Pixel Count:   ");
    xv_set(hist->histo_display->message2,
	   PANEL_LABEL_STRING, message, NULL);
    panel_paint(hist->histo_display->message1, PANEL_CLEAR);
    panel_paint(hist->histo_display->message2, PANEL_CLEAR);

}

/*******************************************************************/

struct hcontext
         *
histo_by_win(win)
    Xv_Window win;
{
    struct hcontext *hist;
    struct logent *log;

    log = loghead;
    while (log != NULL) {
	hist = log->hist;
	if (hist != NULL) {
	    if (hist->paintwin == win)
		return hist;
	}
	log = log->next;
    }
    printf("histo_by_win: No such histogram window!\n");
    return NULL;
}

/*****************************************************************/
struct hcontext *
histo_by_panelwin(win)
    Xv_Window win;
{
    struct hcontext *hist;
    struct logent *log;

    log = loghead;
    while (log != NULL) {
	hist = log->hist;
	if (hist != NULL) {
	    if (hist->histo_display->controls2 == win)
		return hist;
	}
	log = log->next;
    }
    printf("histo_by_panelwin: No such histogram window!\n");
    return NULL;
}


/*****************************************************************/
struct hcontext *
histo_by_lid(id)
    int       id;
{
    struct logent *log;

    log = loghead;
    while (log != NULL) {
	if (log->id == id)
	    return (log->hist);
	log = log->next;
    }
    return NULL;
}

/*******************************************************************/
/*
 * Event callback function for `histocanv'.
 */
Notify_value
histo_event_proc(win, event, arg, type)
    Xv_window win;
    Event    *event;
    Notify_arg arg;
    Notify_event_type type;
{
    struct hcontext *hist;
    int       buckval, lastbuck, x;
    float     xdelt;
    char      message[80];

    if (!(event_is_down(event)))
	return notify_next_event_func(win, (Notify_event) event, arg, type);
    if (event_id(event) != BUT(1) && event_id(event) != BUT(3))
	return notify_next_event_func(win, (Notify_event) event, arg, type);

#ifdef DEBUG
    fprintf(stderr, "histo: histo_event_proc: event %d\n", event_id(event));
#endif

    hist = histo_by_win(win);
    if (hist == NULL)
	return notify_next_event_func(win, (Notify_event) event, arg, type);

    x = event_x(event);
    if ((x <= SX) || (x >= GRWIDTH + SX)) {
	return notify_next_event_func(win, (Notify_event) event, arg, type);
    }
    /* find the bucket corresponding with the event */
    xdelt = (float) GRWIDTH / (hist->xglab[hist->xglen - 1].val -
			       hist->xglab[0].val);
    /* assume line is centered in rectangle */
    x = x - (int) ((.5 * xdelt) + .5);
    buckval = xtob(x, hist);

    /* set the panel labels appropriately */
    sprintf(message, "Pixel Value: %d", btog(buckval, hist));
    xv_set(hist->histo_display->message1,
	   PANEL_LABEL_STRING, message, NULL);
    sprintf(message, "Pixel Count: %d", hist->countvec[buckval]);
    xv_set(hist->histo_display->message2,
	   PANEL_LABEL_STRING, message, NULL);
    panel_paint(hist->histo_display->message1, PANEL_CLEAR);
    panel_paint(hist->histo_display->message2, PANEL_CLEAR);


#ifdef STRONGDEBUG
    show_histogram(hist);
#endif

    if (event_id(event) == BUT(1)) {
	if (buckval < hist->interval[hist->subnum].upper) {
	    hist->interval[hist->subnum].isactive = 1;
	    lastbuck = hist->interval[hist->subnum].lower;
	    hist->interval[hist->subnum].lower = buckval;	/* set value */
	    histo_line_draw(hist, buckval, lastbuck, TRUE);
	}
    }
    if (event_id(event) == BUT(3)) {
	if (buckval > hist->interval[hist->subnum].lower) {
	    hist->interval[hist->subnum].isactive = 1;
	    lastbuck = hist->interval[hist->subnum].upper;
	    hist->interval[hist->subnum].upper = buckval;	/* set value */
	    histo_line_draw(hist, buckval, lastbuck, FALSE);
	}
    }
    return notify_next_event_func(win, (Notify_event) event, arg, type);
}

/*************************************************/
histo_line_draw(hist, buckval, lastbuck, lower)
    struct hcontext *hist;
    int       buckval, lastbuck, lower;
{

    /* first un-draw red line with a background-color line */
#ifdef DEBUG
    printf("histo_line_draw: repainting bucket %d\n", lastbuck);
#endif

    undraw_histo_line(hist, lastbuck);

    /* redraw all color lines */
    draw_histo_color_lines(hist);

    if (color_mode) {
	if (hist->subnum == 0) {
	    if (lower)
		xv_set(histo_ctrl->min1, PANEL_VALUE, btog(buckval, hist), NULL);
	    else
		xv_set(histo_ctrl->max1, PANEL_VALUE, btog(buckval, hist), NULL);
	}
	if (hist->subnum == 1) {
	    if (lower)
		xv_set(histo_ctrl->min2, PANEL_VALUE, btog(buckval, hist), NULL);
	    else
		xv_set(histo_ctrl->max2, PANEL_VALUE, btog(buckval, hist), NULL);
	}
	if (hist->subnum == 2) {
	    if (lower)
		xv_set(histo_ctrl->min3, PANEL_VALUE, btog(buckval, hist), NULL);
	    else
		xv_set(histo_ctrl->max3, PANEL_VALUE, btog(buckval, hist), NULL);
	}
	if (hist->subnum == 3) {
	    if (lower)
		xv_set(histo_ctrl->min4, PANEL_VALUE, btog(buckval, hist), NULL);
	    else
		xv_set(histo_ctrl->max4, PANEL_VALUE, btog(buckval, hist), NULL);
	}
	if (hist->subnum == 4) {
	    if (lower)
		xv_set(histo_ctrl->min5, PANEL_VALUE, btog(buckval, hist), NULL);
	    else
		xv_set(histo_ctrl->max5, PANEL_VALUE, btog(buckval, hist), NULL);
	}
	if (hist->subnum == 5) {
	    if (lower)
		xv_set(histo_ctrl->min6, PANEL_VALUE, btog(buckval, hist), NULL);
	    else
		xv_set(histo_ctrl->max6, PANEL_VALUE, btog(buckval, hist), NULL);
	}
	if (hist->subnum == 6) {
	    if (lower)
		xv_set(histo_ctrl->min7, PANEL_VALUE, btog(buckval, hist), NULL);
	    else
		xv_set(histo_ctrl->max7, PANEL_VALUE, btog(buckval, hist), NULL);
	}
	if (hist->subnum == 7) {
	    if (lower)
		xv_set(histo_ctrl->min8, PANEL_VALUE, btog(buckval, hist), NULL);
	    else
		xv_set(histo_ctrl->max8, PANEL_VALUE, btog(buckval, hist), NULL);
	}
    }
}

/***************************************************************/
undraw_histo_line(hist, lastbuck)
    struct hcontext *hist;
    int       lastbuck;
{
    int       oldx, hgt, sy, ey;
    float     xdelt, ydelt;
    XGCValues gcval;

    oldx = btox(lastbuck, hist);
    xdelt = (float) GRWIDTH / (hist->xglab[hist->xglen - 1].val -
			       hist->xglab[0].val);
    oldx = oldx + (int) ((.5 * xdelt) + .5);	/* center line in rectangle */


    gcval.foreground = WhitePixel((Display *) display,
				  DefaultScreen((Display *) display));
    XChangeGC(display, hgc, GCForeground, &gcval);

    XSetLineAttributes(display, hgc, 2, LineSolid, CapButt, JoinBevel);

    sy = SY + 1;		/* dont overwrite axes */
    ey = SY + GRHGT - 1;
#ifdef DEBUG
    printf("\nRedrawing white line from: (%d,%d) to (%d,%d)\n", oldx, sy,
	   oldx, ey);
#endif

    if (oldx > SX && oldx < SX + GRWIDTH)	/* dont overwrite axis */
	XDrawLine(display, hist->hxid, hgc, oldx, sy,
		  oldx, ey);

    /*
     * second, draw black line at same location the same height as the graph
     * line
     */
    gcval.foreground = BlackPixel((Display *) display,
				  DefaultScreen((Display *) display));
    XChangeGC(display, hgc, GCForeground, &gcval);

    ydelt = (float) GRHGT / (hist->yglab[hist->yglen - 1].val -
			     hist->yglab[0].val);
    hgt = irint((double) hist->countvec[lastbuck] * ydelt);
    ey = GRHGT + SY;
    sy = ey - hgt;

    XDrawLine(display, hist->hxid, hgc, oldx, sy, oldx, ey);

#ifdef DEBUG
    printf("Redrawing black line from: (%d,%d) to (%d,%d)\n",
	   oldx, sy, oldx, ey);
#endif
}

/*************************************************/
/*
 * Notify callback function for `magnify'.
 */
void
mag_proc(item, event)
    Panel_item item;
    Event    *event;
{
    struct hcontext *hist;

    hist = histo_by_panelwin(event_window(event));
    if (hist == NULL) {
	printf("No histogram to magnify!\n");
	return;
    }
    if (hist->interval[hist->subnum].lower == 0 &&
	hist->interval[hist->subnum].upper == NUM_BUCKETS - 1) {
	/* dont magnify! */
	return;
    }
    if (hist->subnum < 5) {
	/* set the current display to be within the subrange */
	hist->minb = hist->interval[hist->subnum].lower;
	hist->maxb = hist->interval[hist->subnum].upper;
	hist->subnum++;
    }
    draw_hist(hist);
#ifdef DEBUG
    fputs("histo: mag_proc\n", stderr);
#endif
}

/*
 * Notify callback function for `unmagnify'.
 */
void
unmag_proc(item, event)
    Panel_item item;
    Event    *event;
{
    struct hcontext *hist;

    hist = histo_by_panelwin(event_window(event));
    if (hist == NULL) {
	printf("No histogram to unmagnify!\n");
	return;
    }
    /* set the current display to be within the subrange */
    if (hist->subnum > 0)
	hist->subnum--;
    if (hist->subnum > 0) {
	hist->minb = hist->interval[hist->subnum - 1].lower;
	hist->maxb = hist->interval[hist->subnum - 1].upper;
    } else {
	hist->minb = 0;
	hist->maxb = NUM_BUCKETS - 1;
    }


    draw_hist(hist);
#ifdef DEBUG
    fputs("histo: unmag_proc\n", stderr);
#endif
}

/*
 * Notify callback function for `restore'.
 */
void
restore_proc(item, event)
    Panel_item item;
    Event    *event;
{
    struct hcontext *hist;
    int       i;

    hist = histo_by_panelwin(event_window(event));
    if (hist == NULL) {
	printf("No histogram to restore!\n");
	return;
    }
    /* loop through all intervals and unlite them if lit */
    for (i = 0; i < 5; i++) {
	if (hist->interval[i].islit == 1)
	    unlite(hist, i);
	hist->interval[i].isactive = 0;
	hist->interval[i].lower = 0;
	hist->interval[i].upper = NUM_BUCKETS - 1;
    }
    hist->minb = 0;
    hist->maxb = NUM_BUCKETS - 1;
    hist->subnum = 0;
    draw_hist(hist);
    draw_log();
#ifdef DEBUG
    fputs("histo: restore_proc\n", stderr);
#endif
}


/*******************************************************************/
/* btog() -- bucket # to gray value */
int
btog(b, hist)
    int       b;
    struct hcontext *hist;
{
    float     g;

    g = (((float) (b * (hist->maxv - hist->minv))) / NUM_BUCKETS)
	+ hist->minv;

    return ((int) (g + .5));
}

/*************************************************************/
/* btox() -- bucket # to X-coordinate */
int
btox(b, hist)
    int       b;
    struct hcontext *hist;
{
    float     x;
    float     xdelt;		/* width per gray level */

    xdelt = (float) GRWIDTH / (hist->xglab[hist->xglen - 1].val - hist->xglab[0].val);
    x = ((float) ((btog(b, hist) - hist->xglab[0].val)) * xdelt) + SX;

    return ((int) (x + .5));
}

/*************************************************************/
/* gtob() -- gray value to bukcet # */
int
gtob(g, hist)
    int       g;
    struct hcontext *hist;
{
    int       b;
    float     delta;

    delta = (float) (NUM_BUCKETS - 1) / (float) (hist->maxv - hist->minv);
    b = (int) (((g - hist->minv) * delta) + .5);
    if (b < 0)
	b = 0;
    if (b >= NUM_BUCKETS)
	b = NUM_BUCKETS - 1;

#ifdef DEBUG
    printf("gtob: gray value: %d bucket: %d\n", g, b);
#endif
    return (b);
}

/*************************************************************/
/* xtob() -- X-coordinate to bucket # */
int
xtob(x, hist)
    int       x;
    struct hcontext *hist;
{
    int       b, g;		/* bucket, gray value */

    g = xtog(x, hist);
    b = gtob(g, hist);

    if (b >= NUM_BUCKETS)
	b = NUM_BUCKETS - 1;
    else if (b < 0)
	b = 0;

#ifdef DEBUG
    printf("xtob: x: %d, gray value: %d bucket: %d\n", x, g, b);
#endif
    return b;
}

/*************************************************************/
int
xtog(x, hist)
    int       x;
    struct hcontext *hist;
{
    float     g;
    int       range;

    range = hist->xglab[hist->xglen - 1].val - hist->xglab[0].val;

    g = (((float) (x - SX) * range) / (float) GRWIDTH) +
	hist->xglab[0].val;

#ifdef DEBUG
    printf("xtog: x: %d, gray value: %d \n", x, (int) (g + .5));
#endif

    return ((int) (g + .5));

}


/*************************************************************/
/* routine to find the max count in a histogram */
int
c_max(minb, maxb, hist)
    int       minb, maxb;
    struct hcontext *hist;
{
    int       i, max = 0;

    for (i = minb; i <= maxb; i++) {
	if (hist->countvec[i] > max)
	    max = hist->countvec[i];
    }
    return max;
}

/*************************************************************/
/* routine to find the min count in a histogram */
/* note that this functions depends on hist->maxc holding the correct value */
int
c_min(minb, maxb, hist)
    int       minb, maxb;
    struct hcontext *hist;
{
    int       i, min = hist->maxc;

    for (i = minb; i <= maxb; i++) {
	if (hist->countvec[i] < min)
	    min = hist->countvec[i];
    }
    return min;
}

/*************************************************************/
delhcontext(h)
    struct hcontext *h;
{
    xv_set(h->histo_display->display,
	   XV_SHOW, FALSE,
	   NULL);

    free(h->pbuf);
}
