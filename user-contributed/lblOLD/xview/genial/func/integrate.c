
/*
 * integrate.c -- routines for trace integration
 */

#include "display.h"
#include "ui.h"
#include "log.h"
#include "reg.h"
#include "scale.h"
#include "trace_ui.h"

GC        igc;
extern GC trgc;

static trace_intgwin_objects *trace_iwin = NULL;

static struct trcontext *curtrace;
struct trcontext *trace_by_panelwin();

/*********************************************************/

integ_init(ctx)
    struct integ_context *ctx;
{
    int       i;

    for (i = 0; i < NINT; i++) {
	ctx->ireg[i].min = ctx->ireg[i].max = ctx->ireg[i].isize = 0;
	ctx->ireg[i].ival = NULL;
    }
    ctx->ipntw = ctx->iwin = ctx->icanv = NULL;
    ctx->cnum = 0;
    ctx->yglab = NULL;
    ctx->cbl = (struct cbstore *) malloc(sizeof(struct cbstore));
    ctx->cbr = (struct cbstore *) malloc(sizeof(struct cbstore));
    ctx->cbl->cht = ctx->cbl->cwt = 0;
    ctx->cbr->cht = ctx->cbr->cwt = 0;


}

/***********************************************************/
int
integrate_eval_proc(item, event)
    Panel_item item;
    Event    *event;
{
    struct integ_context *ctx;
    int id;

#ifdef DEBUG
    fputs("trace: integrate_eval_proc: \n", stderr);
#endif

    curtrace = trace_by_panelwin(event_window(event));
    if (curtrace == NULL) {
	printf("(integrate_eval_proc)no such trace!\n");
	return -1;
    }
    ctx = &curtrace->integ;
    if (ctx->ireg[ctx->cnum].max <= 0 ||
	ctx->ireg[ctx->cnum].min <= 0)
	return -1;

    comp_integ(curtrace);

    disp_integ(curtrace);

#ifdef MAKE_WORK_LATER		/* use this to do multiple integration
				 * windows */
    if (ctx->cnum < NINT - 2)
	ctx->cnum++;
#endif
    return 0;
}

/***********************************************************/
comp_integ(trace)
    struct trcontext *trace;
{
    int       i, j, sum;
    struct integ_context *ctx;

    ctx = &trace->integ;
    ctx->ireg[ctx->cnum].isize =
	ctx->ireg[ctx->cnum].max - ctx->ireg[ctx->cnum].min;

    ctx->ireg[ctx->cnum].ival =
	(int *) calloc(ctx->ireg[ctx->cnum].isize, sizeof(int));

    j = 0;
    sum = 0;
    for (i = ctx->ireg[ctx->cnum].min; i < ctx->ireg[ctx->cnum].max; i++) {
	sum += trace->pbuf[i].oval;
	ctx->ireg[ctx->cnum].ival[j] = sum;
	j++;
    }
}

/***********************************************************/
/* build a display for the integral stuff */
disp_integ(trace)
    struct trcontext *trace;
{
    char      title[80], message[80];
    struct integ_context *ctx;
    XGCValues gcval;
    void      icanv_repaint_proc();

    ctx = &trace->integ;

    if (ctx->iwin == NULL) {
	trace_iwin = trace_intgwin_objects_initialize(NULL, base_win->ctrlwin);

	ctx->iwin = trace_iwin->intgwin;
	ctx->icanv = trace_iwin->intgcanv;
	ctx->ipntw = canvas_paint_window(ctx->icanv);
	ctx->ixid = (XID) xv_get(ctx->ipntw, XV_XID, NULL);

	/* set up a GC for the window */
	gcval.foreground = BlackPixel(display, DefaultScreen(display));
	gcval.background = WhitePixel(display, DefaultScreen(display));
	gcval.clip_mask = None;
	igc = XCreateGC(display, ctx->ixid,
			GCForeground | GCBackground | GCClipMask, &gcval);
    }
    sprintf(title, "Integrate Trace: %d.%d", curfunc->id-1, ctx->cnum+1);
    xv_set(ctx->iwin,
	   XV_WIDTH, MAX(ctx->ireg[ctx->cnum].isize + 85, 210),
	   XV_LABEL, title,
	   XV_SHOW, TRUE,
	   FRAME_CLOSED, FALSE,
	   NULL);
    xv_set(ctx->icanv,
	   XV_WIDTH, ctx->ireg[ctx->cnum].isize + 85,
	   NULL);

    sprintf(message, "Integral Total: %d",
	    ctx->ireg[ctx->cnum].ival[ctx->ireg[ctx->cnum].isize - 1]);
    xv_set(trace_iwin->message1,
	   PANEL_LABEL_STRING, message, NULL);
    panel_paint(trace_iwin->message1, PANEL_CLEAR);

    icanv_repaint_proc(ctx->icanv, ctx->ipntw, display, ctx->ixid, NULL);
}

/***********************************************************/
/* * Repaint callback function for `icanv'.  */
void
icanv_repaint_proc(canvas, paint_window, display, xid, rects)
    Canvas    canvas;
    Xv_window paint_window;
    Window    xid;
    Xv_xrectlist *rects;
{
    XGCValues gcval;
    struct integ_context *ctx, *find_integ();
    float    range;
    int      i, y, ylen, height;

#ifdef DEBUG
    printf("icanv_repaint_proc \n");
#endif

    if (curtrace == NULL)
	return;

    ctx = &curtrace->integ;
    if (ctx->ireg[ctx->cnum].ival == NULL)
	return;

    XClearWindow(display, ctx->ixid);
    if ((ylen = integ_axes(ctx)) <= 0)
	return;

    height = (int)xv_get(ctx->icanv, XV_HEIGHT, NULL) - STARTY;
    range = (float) (ctx->yglab[ylen - 1].val - ctx->yglab[0].val);

    gcval.foreground = BlackPixel((Display *) display,
				  DefaultScreen((Display *) display));
    XChangeGC(display, igc, GCForeground, &gcval);

    for (i = 0; i < ctx->ireg[ctx->cnum].isize; i++) {
	y = height - (int)((height * ctx->ireg[ctx->cnum].ival[i]) / range);
	XDrawLine(display, ctx->ixid, igc, i, y, i, height);
    }
}

/***********************************************************/
integrate_event_proc(event, curtrace)
    Event    *event;
    struct trcontext *curtrace;
{
    struct integ_context *integ;
    int       height, slen, dlen, x, y;
    static int prev_left = -1, prev_right = -1;

    height = (int)xv_get(curtrace->win_info->trcanv, XV_HEIGHT, NULL);
    slen = irint((double) height * SMPHT);
    dlen = height - slen;

    x = event_x(event);
    y = event_y(event);

    /* if they pressed the first button, draw a line in red */
    integ = &curtrace->integ;
    if (event_id(event) == BUT(1) && event_is_down(event)) {
	if (integ->ireg[integ->cnum].max > 0 &&
	    x >= integ->ireg[integ->cnum].max)
	    return;
	if (prev_left != -1)	/* undraw previous line */
	    undraw_intg_line(prev_left, curtrace, dlen);

	prev_left = x;
	XSetForeground(display, trgc, pallet[RED].pixel);
	XSetLineAttributes(display, trgc, 2, LineSolid, CapButt, JoinBevel);
	XDrawLine(display, curtrace->trxid, trgc, x, 0, x, dlen);
	integ->ireg[integ->cnum].min = x;

	move_cross_bar_left(x, integ, curtrace);
    }
    /* if they pressed the third button, draw the end line in red */
    if (event_id(event) == BUT(3) && event_is_down(event)) {
	if (integ->ireg[integ->cnum].min > 0 &&
	    x <= integ->ireg[integ->cnum].min)
	    return;
	if (prev_right != -1)
	    undraw_intg_line(prev_right, curtrace, dlen);

	prev_right = x;
	XSetForeground(display, trgc, pallet[RED].pixel);
	XSetLineAttributes(display, trgc, 2, LineSolid, CapButt, JoinBevel);
	XDrawLine(display, curtrace->trxid, trgc, x, 0, x, dlen);
	integ->ireg[integ->cnum].max = x;

	move_cross_bar_right(x, integ, curtrace);
    }
    XSetLineAttributes(display, trgc, 1, LineSolid, CapButt, JoinBevel);
}

/***************************************************************/
move_cross_bar_left(x, integ, curtrace)
    int       x;
    struct integ_context *integ;
    struct trcontext *curtrace;
{
    XPoint    pt;
    XGCValues gcval;

    /* undraw previous cross bar */
    if (integ->cbl->cht != 0 || integ->cbl->cwt != 0)
	/* refresh image under cross bar */
	ref_cb(img_win->d_xid, integ->cbl);
    draw_cpl();			/* redraw crosses in point list */
    draw_pvec(img_win->d_xid, curtrace->pbuf, curtrace->npts);

    /* draw marker on image */
    pt.x = (short) curtrace->pbuf[x].pt.x;
    pt.y = (short) curtrace->pbuf[x].pt.y;

    cbget(orig_ximg, integ->cbl, pt);
    gcval.foreground = pallet[RED].pixel;
    XChangeGC(display, gc, GCForeground, &gcval);
    draw_cb(img_win->d_xid, integ->cbl);
}

/***************************************************************/
move_cross_bar_right(x, integ, curtrace)
    int       x;
    struct integ_context *integ;
    struct trcontext *curtrace;
{
    XPoint    pt;
    XGCValues gcval;

    /* undraw previous cross bar */
    if (integ->cbr->cht != 0 || integ->cbr->cwt != 0)
	/* refresh image under cross bar */
	ref_cb(img_win->d_xid, integ->cbr);
    draw_cpl();			/* redraw crosses in point list */
    draw_pvec(img_win->d_xid, curtrace->pbuf, curtrace->npts);

    /* draw marker on image */
    pt.x = (short) curtrace->pbuf[x].pt.x;
    pt.y = (short) curtrace->pbuf[x].pt.y;

    cbget(orig_ximg, integ->cbr, pt);
    gcval.foreground = pallet[RED].pixel;
    XChangeGC(display, gc, GCForeground, &gcval);
    draw_cb(img_win->d_xid, integ->cbr);
}

/***********************************************************/
undraw_intg_line(x, curtrace, dlen)
    int       x;
    struct trcontext *curtrace;
    int       dlen;
{
    XGCValues gcval;
    int       grheight, ylen;
    double    scfac;

#ifdef DEBUG
    printf("undrawing line at: %d \n", x);
#endif

    /* shouldnt need 'drawaxes' every time..... -blt */
    ylen = drawaxes(curtrace);
    scfac = (double) (dlen - STARTY) /
	(curtrace->yglab[ylen - 1].val - curtrace->yglab[0].val);

    gcval.foreground = WhitePixel((Display *) display,
				  DefaultScreen((Display *) display));
    XChangeGC(display, trgc, GCForeground, &gcval);
    XSetLineAttributes(display, trgc, 2, LineSolid, CapButt, JoinBevel);

    XDrawLine(display, curtrace->trxid, trgc, x, 0, x, dlen);

    gcval.foreground = BlackPixel((Display *) display,
				  DefaultScreen((Display *) display));
    XChangeGC(display, trgc, GCForeground, &gcval);
    XSetLineAttributes(display, trgc, 1, LineSolid, CapButt, JoinBevel);

    grheight = dlen - irint((double) (trace_value(x, curtrace)
				      - curtrace->yglab[0].val) * scfac);
    XDrawLine(display, curtrace->trxid, trgc, x, dlen + 1, x,
	      (int) grheight);

    /* one position below */
    grheight = dlen - irint((double) (trace_value(x - 1, curtrace)
				      - curtrace->yglab[0].val) * scfac);
    XDrawLine(display, curtrace->trxid, trgc, x - 1, dlen + 1, x - 1,
	      (int) grheight);

    /* one position below */
    grheight = dlen - irint((double) (trace_value(x + 1, curtrace)
				      - curtrace->yglab[0].val) * scfac);
    XDrawLine(display, curtrace->trxid, trgc, x + 1, dlen + 1, x + 1,
	      (int) grheight);
}

/***********************************************************/
struct integ_context
         *
find_integ(pntw)
    Xv_window pntw;
{
    struct trcontext *trace;
    struct integ_context *intg;
    struct logent *log;

    log = loghead;
    while (log != NULL) {
	trace = log->trace;
	if (trace != NULL) {
	    intg = &trace->integ;
	    if (intg->iwin == pntw) 
		return intg;
	}
	log = log->next;
    }
    return NULL;
}

/***********************************************************/
int
integ_axes(ctx)
    struct integ_context *ctx;
{
    int       maxv;
    int       ylen, width, height;

    if (ctx->ireg[ctx->cnum].ival == NULL)
	return -1;
    maxv = ctx->ireg[ctx->cnum].ival[ctx->ireg[ctx->cnum].isize - 1];

    width = (int)xv_get(ctx->icanv, XV_WIDTH, NULL);
    height = (int)xv_get(ctx->icanv, XV_HEIGHT, NULL);
    XSetForeground(display, igc, BlackPixel(display, DefaultScreen(display)));

    /* build_glab(min, max, max # labels, pixel length, return values) */
    ylen = build_glab(0, maxv, 5, height - (STARTY * 2), &ctx->yglab);

    /*
     * xlabelgraph(window stuff, # of labels, starting loc, width,height of
     * window, orientation, direction
     */
    xlabelgraph(display, ctx->ixid, igc, ctx->yglab, ylen,
	 width - 80, height - STARTY, width, height - STARTY, VERTICAL, -1);
    return ylen;
}

/*********************************/
integ_clear(trace)
    struct trcontext *trace;
{
    struct integ_context *ctx;

    ctx = &trace->integ;
    if (ctx->iwin != NULL)
	xv_set(ctx->iwin, XV_SHOW, FALSE, NULL);

    if (ctx->cbl->cwt != 0 && ctx->cbl->cht != 0)
		    ref_cb(img_win->d_xid, ctx->cbl);

    if (ctx->cbr->cwt != 0 && ctx->cbr->cht != 0)
		    ref_cb(img_win->d_xid, ctx->cbr);

}
