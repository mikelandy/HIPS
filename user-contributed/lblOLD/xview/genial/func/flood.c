/*
 * regions/flood.c -- routines for doing flood fills on a region
 * this is some of the guliest code in all of GENIAL.
 */

#include "reg.h"
#include "common.h"
#include "display.h"
#include "ui.h"
#include <X11/Xlib.h>
#include <stdio.h>

static XImage *pfrm = NULL;	/* psuedo frame buffer.  perverse, but
				 * expedient */

/* this could be done a LOT more efficiently */
#define RLEV 150000		/* should be plenty */

int
flood_fill(reg, pbuf, size, len)
    struct region *reg;
    XPoint  **pbuf;
    int      *size, *len;
{
    u_char   *tmp;
    static XPoint *stack = NULL;
    static int hiwat = 0;
    int       dpt = 0;		/* depth into the stack */
    register unsigned long v;
    int       x, y;
    int       pt_cnt = 0;	/* count of # of points found */
    int       sx, sy, n;
    struct plist *trav;

    if (pfrm != NULL) {
	XDestroyImage(pfrm);
	pfrm = NULL;
    }
    /*
     * create a zero'ed XImage which will just store points where the polygon
     * is.
     */
    tmp = (unsigned char *) calloc(1, (orig_ximg->width * orig_ximg->height));
    pfrm = XCreateImage(display, winv, depth, ZPixmap,
			0, tmp, orig_ximg->width, orig_ximg->height, 8, 0);

    /*
     * place all the points associated with the polygon or spline into the
     * psuedo frame buffer
     */
    drawpts(reg);

    /* initialize sx and sy */
    sx = sy = n = 0;
    for (trav = reg->r_plist; trav != NULL; trav = trav->next) {
	sx += trav->pt.x;
	sy += trav->pt.y;
	n++;
    }

    sx = sx / n;
    sy = sy / n;
    if (hiwat == 0) {
	hiwat += RLEV;
	stack = (XPoint *) malloc(hiwat * sizeof(XPoint));
    }
    stack[0].x = sx;
    stack[0].y = sy;
    while (dpt >= 0) {
	x = stack[dpt].x;
	y = stack[dpt].y;
	v = (unsigned long) XGetPixel(pfrm, x, y);
	if (v == (unsigned long) standout) {
	    dpt--;
	} else {
#ifdef SHOWFF
	    XDrawPoint(display, img_win->d_xid, gc, x, y);
#endif
	    XPutPixel(pfrm, x, y, (unsigned long) standout);
	    pt_cnt++;
	    /* add the point to the point buffer */
	    if (*len >= *size) {
		*size += BSIZE;
		*pbuf = (XPoint *) realloc(*pbuf, (*size) * sizeof(XPoint));
	    }
	    (*pbuf)[*len].x = x;
	    (*pbuf)[*len].y = y;
	    (*len)++;
	    if (dpt + 4 >= hiwat) {
		printf("reallocing the stack \n");
		hiwat += RLEV;
		stack = (XPoint *) realloc(stack, hiwat * sizeof(XPoint));
	    }
	    ++dpt;
	    stack[dpt].x = x - 1;
	    stack[dpt].y = y;
	    ++dpt;
	    stack[dpt].x = x + 1;
	    stack[dpt].y = y;
	    ++dpt;
	    stack[dpt].x = x;
	    stack[dpt].y = y - 1;
	    ++dpt;
	    stack[dpt].x = x;
	    stack[dpt].y = y + 1;
	}
    }
    return pt_cnt;
}


drawpts(reg)
    struct region *reg;
{
    struct dlist *pstore;
    XGCValues gcval;
    int       i;

    gcval.foreground = standout;
    XChangeGC(display, gc, GCForeground, &gcval);
    for (pstore = reg->r_dlist; pstore != NULL; pstore = pstore->next) {
	for (i = 0; i < pstore->len; i++) {
	    XPutPixel(pfrm, (int) pstore->points[i].pt.x,
		(int) pstore->points[i].pt.y, (unsigned long) standout);
	}
    }
}
