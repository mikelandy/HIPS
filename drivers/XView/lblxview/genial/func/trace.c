
/*
 * trace.c -- routines for running traces along lines or splines
 */

#include "display.h"
#include "ui.h"
#include "log.h"
#include "reg.h"
#include "scale.h"
#include "trace_ui.h"

GC        trgc;

struct trcontext *newtrace(), *trace_by_win();
struct trcontext *trace_by_lid(), *trace_by_panelwin();

trace_tcntrl_objects *trace_tcntrl;
trace_trwin_objects *trace_twin;

extern int shrink_fac;

/*************************************************************/
trace_init()
{
    /* do not raise options window; let the options routine handle that. */
    clear_info();
    lab_info("Please select a region for trace", 1);
    lab_info("Hit <eval> when done. ", 2);

    /* initialize trace control window if we havent already */
    if (trace_tcntrl == NULL) {
	trace_tcntrl = trace_tcntrl_objects_initialize(NULL,
						       base_win->ctrlwin);
	xv_set(trace_tcntrl->traceno,
	       PANEL_VALUE, curfunc->id,
	       NULL);
    };

    /* set the region type */
    reg_setdom(TRACE);

    return 0;
}

/***************************************************************/
int
trace_eval()
{
    struct pval *cpbuf;		/* continuous pbuf formed from the log entry */
    int       np;		/* number of points in above */
    struct trcontext *curtrace;

    /*
     * trace windows are interactive, so things will be much more efficient
     * if we have a continous point buffer
     */

    /* type BOX disabled: What is a box trace?? -BLT */
    if (curfunc->reg->r_type == BOX) {
	np = box_vec(curfunc->reg->r_plist, &cpbuf);
    } else {
	np = makepbuf(curfunc->reg->r_dlist, &cpbuf);
    }
    /* make a new trace context */
    curtrace = newtrace(np);
    curtrace->npts = np;
    curtrace->pbuf = cpbuf;
    if (curfunc->reg->r_type == BOX) {
	curtrace->flags |= RECT;
    }
    /*
     * if there is information in the auxiliary data, handle it.  If not, put
     * it there
     */
    tr_auxd(curfunc, curtrace);

    if (curtrace->t_attr.scale_type == 1) {
	scale_trace(curtrace);
    }
    /* set the control window appropriately */
    trace_panel(curtrace);
    xv_set(trace_tcntrl->traceno, PANEL_VALUE, curfunc->id, NULL);

    return 0;
}

/***************************************************************/
int
trace_change(id)
    int       id;
{
    struct pval *cpbuf;		/* continuous pbuf formed from the log entry */
    int       np;		/* number of points in above */
    struct trcontext *curtrace;

#ifdef DEBUG
    printf("in trace change: \n");
#endif

    curtrace = trace_by_lid(id);
    if (curtrace == NULL) {
	fprintf(stderr, "trace_change: Trace not found \n");
	return -1;
    }
    np = makepbuf(curfunc->reg->r_dlist, &cpbuf);

    curtrace->npts = np;
    curtrace->pbuf = cpbuf;

/* ADD: handle shrink mode */
    xv_set(curtrace->win_info->trwin,
	   XV_WIDTH, MAX(np + 85, 265),
	   NULL);
    xv_set(curtrace->win_info->trcanv,
	   XV_WIDTH, np + 85,
	   NULL);

    if (curtrace->t_attr.scale_type == 1) {
	scale_trace(curtrace);
    }
    /* set the control window appropriately */
    trace_panel(curtrace);
    xv_set(trace_tcntrl->traceno, PANEL_VALUE, id, NULL);

    return 0;
}

/*************************************************************/
trace_clear(id)
    int       id;
/* clear the current trace */
{
    struct trcontext *ct;

#ifdef DEBUG
    printf("trace_clear: log id # %d \n", id);
#endif

    if ((ct = trace_by_lid(id)) == NULL) {
	printf("trace_clear: trace #%d not found \n", id);
	return -1;
    }
    integ_clear(ct);
    deltrcontext(ct);
    return 0;
}

/*************************************************************/
trace_reset()
{
    return;
}

/*************************************************************/

struct trcontext *
newtrace(np)
    int       np;		/* number of points: needed to set width. */
{
    struct trcontext *trace;
    char      title[80];
    Notify_value trcanv_event_proc();
    XGCValues gcval;
    XFontStruct *fs;

    if (curfunc->trace != NULL)
	free(curfunc->trace);

    trace = (struct trcontext *) malloc(sizeof(struct trcontext));
    curfunc->trace = trace;

    trace->pbuf = NULL;
    trace->npts = NULL;
    trace->x = -1;
    trace->y = -1;
    trace->cb = (struct cbstore *) malloc(sizeof(struct cbstore));
    trace->cb->cht = 0;
    trace->cb->cwt = 0;
    trace->flags = 0;
    trace->integ.iwin = 0;
    newtrattr(&trace->t_attr);
    integ_init(&trace->integ);
    sprintf(title, "Trace: %d", curfunc->id);

    /* create the actual window and canvas */
    trace->win_info = trace_trwin_objects_initialize(NULL, base_win->ctrlwin);

    trace->trpntw = canvas_paint_window(trace->win_info->trcanv);
    trace->trxid = (XID) xv_get(trace->trpntw, XV_XID, NULL);

    /* ADD: handle shrink mode!! */
    xv_set(trace->win_info->trwin, XV_WIDTH, MAX(np + 85, 265),
	   XV_LABEL, title,
	   XV_SHOW, TRUE,
	   FRAME_CMD_PUSHPIN_IN, TRUE,
	   NULL);
    xv_set(trace->win_info->trcanv,
	   XV_WIDTH, np + 85,
	   NULL);

    /* set up a GC for the window */
    gcval.foreground = BlackPixel(display, DefaultScreen(display));
    gcval.background = WhitePixel(display, DefaultScreen(display));
    gcval.clip_mask = None;
    trgc = XCreateGC(display, trace->trxid,
		     GCForeground | GCBackground | GCClipMask, &gcval);

    if (!(fs = XLoadQueryFont(display, "9x15"))) {
	fprintf(stderr, "Font 9x15 not found, trying \'fixed\' \n");
	if (!(fs = XLoadQueryFont(display, "fixed"))) {
	    fprintf(stderr, "error geting fonts, exitting...\n");
	    exit(-1);
	}
    }
    XSetFont(display, trgc, fs->fid);

    return trace;
}

/*******************************************************/
deltrcontext(trace)
    struct trcontext *trace;
{
    if (trace->cb->cwt != 0 && trace->cb->cht != 0)
	ref_cb(img_win->d_xid, trace->cb);

    free(trace->pbuf);
    free(trace->cb);
    free(trace);

    xv_set(trace->win_info->trwin,
	   FRAME_CMD_PUSHPIN_IN, FALSE, NULL);
    xv_set(trace->win_info->trwin,
	   XV_SHOW, FALSE, NULL);
}

/*******************************************************/
newtrattr(opts)
    struct tropts *opts;
{
    opts->scale_type = 1;
    opts->avgrad = (int) xv_get(trace_tcntrl->radius, PANEL_VALUE, NULL);
    opts->axmin = 0;
    opts->axmax = orig_img->maxv;
}

/*******************************************************/
scale_trace(trace)
    struct trcontext *trace;
{
    int       min, max;
    int       i;

    if (trace->t_attr.scale_type == 1) {
	min = trace->pbuf[0].oval;
	max = trace->pbuf[0].oval;
	for (i = 0; i < trace->npts; i++) {
	    if (trace->pbuf[i].oval < min)
		min = trace->pbuf[i].oval;
	    else if (trace->pbuf[i].oval > max)
		max = trace->pbuf[i].oval;
	}
	trace->t_attr.axmin = min;
	trace->t_attr.axmax = max;
    }
    if (trace->t_attr.scale_type == 0) {
	trace->t_attr.axmin = orig_img->minv;
	trace->t_attr.axmax = orig_img->maxv;
    }
    if (trace->t_attr.scale_type == 2) {
	trace->t_attr.axmin = (int) xv_get(trace_tcntrl->min, PANEL_VALUE, NULL);
	trace->t_attr.axmax = (int) xv_get(trace_tcntrl->max, PANEL_VALUE, NULL);
    }
}

/*****************************************************************/
struct trcontext *
trace_by_win(win)
    Xv_window win;
{
    struct trcontext *trace;
    struct logent *log;

    log = loghead;
    while (log != NULL) {
	trace = log->trace;
	if (trace != NULL) {
	    if ((trace->trpntw == win) || (trace->win_info->trwin == win))
		return trace;
	}
	log = log->next;
    }
    return NULL;
}

/*****************************************************************/
struct trcontext *
trace_by_panelwin(win)
    Xv_Window win;
{
    struct trcontext *trace;
    struct logent *log;

    log = loghead;
    while (log != NULL) {
	trace = log->trace;
	if (trace != NULL) {
	    if (trace->win_info->controls3 == win)
		return trace;
	}
	log = log->next;
    }
    return NULL;
}

/*****************************************************************/
struct trcontext *
trace_by_lid(id)
    int       id;
{
    struct logent *log;

    log = loghead;
    while (log != NULL) {
	if (log->id == id)
	    return (log->trace);
	log = log->next;
    }
    return NULL;
}

/**************************************************************
 * Repaint callback function for `h_trcanv'.
 */
void
trcanv_repaint_proc(canvas, paint_window, display, xid, rects)
    Canvas    canvas;
    Xv_window paint_window;
    Display  *display;
    Window    xid;
    Xv_xrectlist *rects;
{
    struct integ_context *ctx;
    register int i;
    XGCValues gcval;
    unsigned long g;
    struct trcontext *curtrace;
    double    scfac;
    int       trsum = 0, trave = 0;
    char      message[80];
    int       ylen, grheight;
    int       slen, dlen, width, height;
    /*
     * slen and dlen are the lengths of the sample and display sections of
     * the display
     */

    curtrace = trace_by_win(paint_window);
    if (curtrace == NULL) {
	printf("no such trace associated with that window!\n");
	return;
    }
    if (curtrace->pbuf == NULL)
	return;

#ifdef DEBUG
    printf("repainting tracewin\n");
#endif

    XClearWindow(display, curtrace->trxid);
    if ((ylen = drawaxes(curtrace)) <= 0)
	return;
    width = (int) xv_get(curtrace->win_info->trcanv, XV_WIDTH, NULL);
    height = (int) xv_get(curtrace->win_info->trcanv, XV_HEIGHT, NULL);
    slen = irint((double) height * SMPHT);
    dlen = height - slen;
    scfac = (double) (dlen - STARTY) /
	(curtrace->yglab[ylen - 1].val - curtrace->yglab[0].val);

    for (i = 0; i < curtrace->npts; i++) {

    /* ADD: handle shrink mode!!! */
	gcval.foreground = curtrace->pbuf[i].val;
	XChangeGC(display, trgc, GCForeground, &gcval);
	XDrawLine(display, curtrace->trxid, trgc, i, height - slen, i, height);
	gcval.foreground = BlackPixel((Display *) display,
				      DefaultScreen((Display *) display));
	XChangeGC(display, trgc, GCForeground, &gcval);
	g = trace_value(i, curtrace);
	trsum += g;
	if (g > curtrace->yglab[0].val) {
	    grheight = dlen - irint((double)
				    (g - curtrace->yglab[0].val) * scfac);
	    XDrawLine(display, curtrace->trxid, trgc, i, dlen + 1, i,
		      (int) grheight);
	}
    }

  /* draw any overlay lines  */
    XSetLineAttributes(display, trgc, 2, LineSolid, CapButt, JoinBevel);
    ctx = &curtrace->integ;
    XSetForeground(display, trgc, pallet[RED].pixel);
    for (i = 0; i < ctx->cnum; i++) {
	XDrawLine(display, curtrace->trxid, trgc, ctx->ireg[i].min,
		  0, ctx->ireg[i].min, dlen);
	XDrawLine(display, curtrace->trxid, trgc, ctx->ireg[i].max,
		  0, ctx->ireg[i].max, dlen);
    }
    trave = trsum / curtrace->npts;
    XSetLineAttributes(display, trgc, 1, LineSolid, CapButt, JoinBevel);

    /* sum and average computed in trcanv_repaint_proc */
    sprintf(message, "Value:  0");
    xv_set(curtrace->win_info->mes1, PANEL_LABEL_STRING, message, NULL);
    sprintf(message, "Sum: %d", trsum);
    xv_set(curtrace->win_info->mes3, PANEL_LABEL_STRING, message, NULL);
    sprintf(message, "Average: %d", trave);
    xv_set(curtrace->win_info->mes4, PANEL_LABEL_STRING, message, NULL);
    panel_paint(curtrace->win_info->mes1, PANEL_CLEAR);
    panel_paint(curtrace->win_info->mes3, PANEL_CLEAR);
    panel_paint(curtrace->win_info->mes4, PANEL_CLEAR);
}

/*************************************************************/
/* trace_value() -- gets the trace value at point p along trace */
unsigned
trace_value(p, trace)
    int       p;
    struct trcontext *trace;
{
    int       i, start, end;
    unsigned  sum;

    if (trace->t_attr.avgrad == 0) {
	return trace->pbuf[p].oval;
    } else {
	if (p - trace->t_attr.avgrad / 2 < 0) {
	    start = 0;
	} else {
	    start = p - trace->t_attr.avgrad / 2;
	}

	if (p + trace->t_attr.avgrad / 2 >= trace->npts) {
	    end = trace->npts - 1;
	} else {
	    end = p + trace->t_attr.avgrad / 2;
	}

	sum = 0;
	for (i = start; i <= end; i++) {
	    sum += trace->pbuf[i].oval;
	}
	return sum / (end + 1 - start);
    }
}

/*************************************************************/
/* label the axes on a trace window */
drawaxes(curtrace)
    struct trcontext *curtrace;
{
    XGCValues gcval;
    int       ylen;
    int       slen, dlen, width, height;	/* slen and dlen are the
						 * lengths of the sample and
						 * display sections of the
						 * display */


    width = (int) xv_get(curtrace->win_info->trcanv, XV_WIDTH, NULL);
    height = (int) xv_get(curtrace->win_info->trcanv, XV_HEIGHT, NULL);
    slen = irint((double) height * SMPHT);
    dlen = height - slen;
    gcval.foreground = BlackPixel((Display *) display,
				  DefaultScreen((Display *) display));
    XChangeGC(display, trgc, GCForeground, &gcval);
    ylen = build_glab(curtrace->t_attr.axmin, curtrace->t_attr.axmax, 5,
		      dlen - STARTY, &curtrace->yglab);
    if (ylen > 0)
	xlabelgraph(display, curtrace->trxid, trgc, curtrace->yglab, ylen,
		    curtrace->npts + 2, dlen, width, dlen, VERTICAL, -1);
    return ylen;
}

/*************************************************************/
/*
 * Event callback function for `h_trcanv'.
 */
Notify_value
trcanv_event_proc(win, event, arg, type)
    Xv_Window win;
    Event    *event;
    Notify_arg arg;
    Notify_event_type type;
{
    XGCValues gcval;
    XPoint    pt;
    struct trcontext *curtrace;
    char      message[80];
    int       slen, dlen, width, height;
    /*
     * slen and dlen are the lengths of the sample and display sections of
     * the display
     */

    if (event_id(event) != MS_RIGHT && event_id(event) != MS_LEFT &&
	event_id(event) != MS_MIDDLE && event_id(event) != LOC_DRAG)
	return notify_next_event_func(win, event, arg, type);

    curtrace = trace_by_win(win);
    if (curtrace == NULL) {
	printf("no such trace associated with that window!\n");
	return notify_next_event_func(win, event, arg, type);
    }
    width = (int) xv_get(curtrace->win_info->trcanv, XV_WIDTH, NULL);
    height = (int) xv_get(curtrace->win_info->trcanv, XV_HEIGHT, NULL);
    if (event_x(event) <= 0 || event_x(event) >= curtrace->npts ||
	event_y(event) <= 0 || event_y(event) >= height)
	return notify_next_event_func(win, event, arg, type);

    if (curtrace->pbuf == NULL)
	return notify_next_event_func(win, event, arg, type);

    /* do integrate select if checked */
    if ((int) xv_get(curtrace->win_info->integrate, PANEL_VALUE, NULL) > 0) {
	integrate_event_proc(event, curtrace);
	return notify_next_event_func(win, event, arg, type);
    }
    /* un-draw previous lines and crosses */
    draw_log();			/* redraw endpoints and log # */
    slen = irint((double) height * SMPHT);
    dlen = height - slen;

    XSetLineAttributes(display, trgc, 2, LineSolid, CapButt, JoinBevel);

    if (curtrace->cb->cht != 0 || curtrace->cb->cwt != 0)
	/* refresh image under cross bar */
	ref_cb(img_win->d_xid, curtrace->cb);

    draw_cpl();			/* redraw crosses in point list */
    draw_pvec(img_win->d_xid, curtrace->pbuf, curtrace->npts);
    XSetFunction(display, trgc, GXxor);
    XSetForeground(display, trgc,
		   (BlackPixel(display, DefaultScreen(display)) ^ standout));
    XDrawLine(display, curtrace->trxid, trgc, curtrace->x, 0,
	      curtrace->x, dlen);
    XSetFunction(display, trgc, GXcopy);

    curtrace->x = event_x(event);
    curtrace->y = event_y(event);

    /* use xor to restore line to black */
    gcval.foreground = pallet[RED].pixel;
    XChangeGC(display, trgc, GCForeground, &gcval);
    XSetFunction(display, trgc, GXxor);
    XSetForeground(display, trgc,
		   (BlackPixel(display, DefaultScreen(display)) ^ standout));
    XDrawLine(display, curtrace->trxid, trgc, curtrace->x, 0,
	      curtrace->x, dlen);
    XSetFunction(display, trgc, GXcopy);

    /* draw marker on image */
    pt.x = (short) curtrace->pbuf[curtrace->x].pt.x;
    pt.y = (short) curtrace->pbuf[curtrace->x].pt.y;
    cbget(orig_ximg, curtrace->cb, pt);
    gcval.foreground = pallet[RED].pixel;
    XChangeGC(display, gc, GCForeground, &gcval);
    draw_cb(img_win->d_xid, curtrace->cb);


    sprintf(message, "Value:  %d", trace_value(curtrace->x, curtrace));
    xv_set(curtrace->win_info->mes1, PANEL_LABEL_STRING, message, NULL);
    panel_paint(curtrace->win_info->mes1, PANEL_CLEAR);

    XSetLineAttributes(display, trgc, 1, LineSolid, CapButt, JoinBevel);

    return notify_next_event_func(win, event, arg, type);
}

/***********************************************************/
/* handle_auxd will set certain attributes if there is auxiliary data in the
log, and if not, will place the current attributes in the auxdata area
for logging purposes */
tr_auxd(func, ctx)
    struct logent *func;
    struct trcontext *ctx;
{
    u_char   *data;

    if (func->auxdsize != 0) {
	data = func->auxdata;
	ctx->t_attr.avgrad = *((int *) data);
	data += sizeof(int);
	ctx->t_attr.axmin = *((int *) data);
	data += sizeof(int);
	ctx->t_attr.axmax = *((int *) data);
	data += sizeof(int);
    } else {
	func->auxdsize = (unsigned) 3 *sizeof(int);
	data = func->auxdata = (u_char *) malloc(func->auxdsize);
	*((int *) data) = ctx->t_attr.avgrad;
	data += sizeof(int);
	*((int *) data) = ctx->t_attr.axmin;
	data += sizeof(int);
	*((int *) data) = ctx->t_attr.axmax;
	data += sizeof(int);
    }
}

/*****************************************************************/
void
trace_refresh_proc(item, event)
    Panel_item item;
    Event    *event;
{
    struct trcontext *context, *trace_by_win();
    trace_trwin_objects *ip = (trace_trwin_objects *) xv_get(item,
						     XV_KEY_DATA, INSTANCE);

#ifdef DEBUG
    fputs("trace: trace_refresh_proc\n", stderr);
#endif

    context = trace_by_win(ip->trwin);
    if (context == NULL) {
	return;
    }
    /* repaint the window */
    trcanv_repaint_proc(context->win_info->trcanv, context->trpntw, display,
			context->trxid, (Xv_xrectlist *) NULL);
}
