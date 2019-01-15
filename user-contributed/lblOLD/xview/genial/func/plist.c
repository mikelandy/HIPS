/*
 * plist.c -- routines for manipulating lists of points
 *
 */

#include "common.h"
#include "llist.h"
#include "display.h"
#include "ui.h"
#include "reg.h"
#include "log.h"

/* the current point list manipulated during region selection */
static struct plist *cplhead = NULL, *cpltail = NULL;

/* the number of current points, and the point limit */
static int ncp = 0, plim = 0;

/***********************************************************/
void
set_plist_globals(reg)
    struct region *reg;
{
    if (reg != NULL) {
	cplhead = reg->r_plist;
	ncp = reg->r_plen+1;
	cpltail = reg->r_plist;
	set_region_plim(reg->r_type);	/* this sets plim */
	if (reg->r_type == AN_TEXT && reg->r_sbs != NULL)
	    set_string(reg->r_sbs->string);
    } else {
	cplhead = cpltail = NULL;
	ncp = 0;
	set_region_plim(getrtype());
    }
}

/*********************************************/
/* setplim() -- set point limit */
int
setplim(p)
    int       p;
{
    plim = p;
}
/*********************************************/
int
getplim()
{
    return(plim);
}

/******************************************************/

/* flush_cpl() -- flush the current point list (set reg->r_plist) */
flush_cpl(reg)
    struct region *reg;
{
    if (cplhead == NULL)
	return;			/* necessary for use with logging */
    reg->r_plist = cplhead;
    reg->r_plen = ncp;
    cplhead = cpltail = NULL;
    ncp = 0;
}

/* add_point adds a point to the current point list of the region */
add_point(x, y)
    int       x, y;
{
    struct plist *t;

    if (ncp >= plim) {
	return;
    }
    t = (struct plist *) malloc(sizeof(struct plist));
    t->pt.x = x;
    t->pt.y = y;
    cbget(orig_ximg, &t->cb, t->pt);
    llist_add((llist *) t, (llist **) & cplhead, (llist **) & cpltail);
    ncp++;
    draw_cpl();
    if (load_log == 0)
	pvreg_set(cplhead);
    curfunc->pvreg->r_plen++;
}

/* add_log_point: same as add_point, but doesnt call draw_cpl() */
add_log_point(x, y)
    int       x, y;
{
    struct plist *t;

    if (ncp >= plim) {
	return;
    }
    t = (struct plist *) malloc(sizeof(struct plist));
    t->pt.x = x;
    t->pt.y = y;
    cbget(orig_ximg, &t->cb, t->pt);
    llist_add((llist *) t, (llist **) & cplhead, (llist **) & cpltail);
    ncp++;
    if (load_log == 0)
	pvreg_set(cplhead);
}

/* delete a point from the current point list entry of the region */
del_point(x, y)
    int       x, y;
{
    XPoint    pt;
    struct plist *t;

    pt.x = x;
    pt.y = y;
    if ((t = pfind(pt)) == NULL) {
	XBell(display, 0);
	return;
    }
    ref_cb(img_win->d_xid, &t->cb);
    llist_del((llist *) t, (llist **) & cplhead, (llist **) & cpltail);
    ncp--;
    if (load_log == 0)
	pvreg_set(cplhead);
}

clear_plist()
{
    struct plist *t;

    for (t = cplhead; t != NULL; t = t->next) {
	ref_cb(img_win->d_xid, &t->cb);
	llist_del((llist *) t, (llist **) & cplhead, (llist **) & cpltail);
    }
    ncp = 0;
    if (load_log == 0)
	pvreg_set(cplhead);
}

/* get the number of points in the currently selected region */
int
getnpoints()
{
    return ncp;
}

/* set the number of points in the currently selected region */
setnpoints(n)
{
    ncp = n;
}

/* for use only by move_point.c: */
struct plist
         *
getcpl()
{
    return cplhead;
}

/* pl_find() -- find an entry in a point list with the exact coordinates of
   the point passed to it. */
struct plist
         *
pl_find(pnt, list)
    XPoint    pnt;
    struct plist *list;
{
    struct plist *tr;

    tr = list;
    while (tr != NULL) {
	if (tr->pt.x == pnt.x && tr->pt.y == pnt.y)
	    return tr;		/* it's gonna be deleted anyway, so we'll
				 * just return the actual indice. */
	tr = tr->next;
    }
    return NULL;
}

/* cbget() -- get the backing store for a cross from an ximage */
cbget(xi, bst, pt)
    XImage   *xi;
    struct cbstore *bst;
    XPoint    pt;
{
    register int x, y;
    /* check top of cross and bottom of cross for border hit... */

    bst->top.y = pt.y - CRSIZE;
    bst->left.x = pt.x - CRSIZE;
    bst->top.x = pt.x;
    bst->left.y = pt.y;
    bst->cht = CRSIZE * 2 + 1;
    bst->cwt = CRSIZE * 2 + 1;
    if (((int) pt.y - CRSIZE) < 0) {
	bst->top.y = 0;
	bst->cht = bst->cht - pt.y;
    } else if (((int) pt.y + CRSIZE) > xi->height) {
	bst->top.y = ((short) xi->height - (CRSIZE * 2));
	bst->cht = bst->cht - (xi->height - pt.y);
    }
    if (((int) pt.x - CRSIZE) < 0) {
	bst->left.x = 0;
	bst->cwt = bst->cwt - pt.x;
    }
    if (((int) pt.x + CRSIZE) > xi->width) {
	bst->left.x = ((short) xi->width - (CRSIZE * 2));
	bst->cwt = bst->cwt - (xi->width - pt.x);
    }
    x = (int) bst->left.x;
    y = (int) bst->left.y;
    while (x <= (bst->left.x + bst->cwt)) {
	bst->hvals[(x - bst->left.x)] = (unsigned char) XGetPixel(xi, x, y);
	x++;
    }
    x = (int) bst->top.x;
    y = (int) bst->top.y;
    while (y <= (bst->top.y + bst->cht)) {
	bst->vvals[(y - bst->top.y)] = (unsigned char) XGetPixel(xi, x, y);
	y++;
    }
}

/***************************************************************/
/* pfind() -- find an entry in a plist within 20 pixels' distance from the
   given point */
struct plist
         *
pfind(dpt)
    XPoint    dpt;
{
    int       sm = 20;
    int       dist;
    struct plist *trp;

    /*
     * We could do all sorts of neat things to optimize this.  but since
     * there aren't  ever going to be more than a handful of points in the
     * buffer, why bother?
     */

    for (trp = cplhead; trp != NULL; trp = trp->next) {
	dist = (int) irint(distance(trp->pt, dpt));
#ifdef DEBUG
	printf("distance to point:%d\n", dist);
#endif
	if (dist <= sm) {
	    return trp;
	}
    }
    return NULL;
}

/***************************************************************/
/* draw_pl() -- refresh the crosses in a given plist */
/* once called cr_refresh().... */
draw_pl(listhead, dwin)
    struct plist *listhead;
    struct disp_win *dwin;
{
    register XGCValues gcval;
    register struct plist *tr;

    gcval.foreground = standout;
    XChangeGC(display, gc, GCForeground, &gcval);
    for (tr = listhead; tr != NULL; tr = tr->next) {
	draw_cb(dwin->d_xid, &(tr->cb));
    }
}

/* routine to draw a cross on the display */
draw_cb(xid, cb)
    XID       xid;
    struct cbstore *cb;
{
    XPoint    pt;
    int       len;

    itod_pt(&cb->top, &pt);
    len = itod(cb->cht);
    XDrawLine(display, xid, gc, pt.x, pt.y, pt.x, len + pt.y);
    itod_pt(&cb->left, &pt);
    len = itod(cb->cwt);
    XDrawLine(display, xid, gc, pt.x, pt.y, len + pt.x, pt.y);
}

/* refresh a cross from its backing store */
ref_cb(xid, cb)
    XID       xid;
    struct cbstore *cb;
{
    /*
     * should eventually add routines to checkand make sure no collisions
     * with other crosses occur, but WE'LL WAIT....
     */
    register XGCValues gcval;
    register int x, y, dx, dy;

    x = cb->top.x;
    y = cb->top.y;

#ifdef DEBUG
    printf("ref_cb: %d,%d \n", x, y);
#endif

    dx = itod(x);
    while (y <= (cb->top.y + cb->cht)) {
	gcval.foreground = cb->vvals[(y - cb->top.y)];
	XChangeGC(display, gc, GCForeground, &gcval);
	dy = itod(y);
	XDrawPoint(display, xid, gc, dx, dy);
	y++;
    }
    x = cb->left.x;
    y = cb->left.y;
    dy = itod(y);
    while (x <= (cb->left.x + cb->cwt)) {
	gcval.foreground = cb->hvals[(x - cb->left.x)];
	XChangeGC(display, gc, GCForeground, &gcval);
	dx = itod(x);
	XDrawPoint(display, xid, gc, dx, dy);
	x++;
    }
}


/* cr_refresh() -- refresh the backing store in a given plist */
ref_pl(listhead, dwin)
    struct plist *listhead;
    struct disp_win *dwin;
{
    struct plist *tr;

    for (tr = listhead; tr != NULL; tr = tr->next) {
	ref_cb(dwin->d_xid, &tr->cb);
    }
}

/* refresh the current point list */
draw_cpl()
{
#ifdef TRACE_EX
    fprintf(stderr, " in draw_cpl \n");
#endif

    if (!clean_mode)
       draw_pl(cplhead, img_win);
}
