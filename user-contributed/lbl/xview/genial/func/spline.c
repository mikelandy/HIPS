/*
 * spline.c -- draw splines
 *
 */

#include "reg.h"
#include "display.h"
#include "ui.h"
#include <X11/Xlib.h>
#include <stdio.h>
#include <math.h>

#define THRESHOLD 1.0		/* 1.0 is better, but superplot uses 3.0 */
#define round(q) ((int)((q) + .5))

/* Spline routines in here are stolen outright from
 * superplot, which is: */
/*************************************************************/
/*                                                           */
/*  Copyright (c) 1986                                       */
/*  Marc S. Majka - UBC Laboratory for Computational Vision  */
/*                                                           */
/*  Permission is hereby granted to copy all or any part of  */
/*  this program for free distribution.   The author's name  */
/*  and this copyright notice must be included in any copy.  */
/*                                                           */
/*************************************************************/

static int lastx, lasty;
static void spline(), Cspline();
static struct pval pbuf[4000], *pind, *last = NULL;

static int psz = 0;

static struct dlist *vbuf, *vstart;
static int v[100];
static int len;

draw_spline(reg)
    struct region *reg;
{
    sp_interp(&reg->r_dlist, reg->r_plist, 0);
    draw_dlist(img_win->d_xid, reg->r_dlist);
}

draw_clspline(reg)
    struct region *reg;
{
    sp_interp(&reg->r_dlist, reg->r_plist, 1);
    draw_dlist(img_win->d_xid, reg->r_dlist);
}

sp_interp(pvec, ptlist, c)
    struct dlist **pvec;
    struct plist *ptlist;
    int       c;
{
    struct plist *tr;

    vbuf = vstart = NULL;
    len = 0;
    psz = 0;
    for (tr = ptlist; tr != NULL; tr = tr->next) {
	v[(len * 2)] = tr->pt.x;
	v[(len * 2) + 1] = tr->pt.y;
	len++;
    }
    pind = pbuf;
    spline(c, len, v);
    *pvec = vstart;
}

drawto(x1, y1, x2, y2)
    int       x1, y1, x2, y2;
{
    XPoint    start, end;

    if (vbuf == NULL) {
	vbuf = (struct dlist *) malloc(sizeof(struct dlist));
	vbuf->next = vbuf->prev = NULL;
	vstart = vbuf;
    } else {
	vbuf->next = (struct dlist *) malloc(sizeof(struct dlist));
	vbuf->next->prev = vbuf;
	vbuf->next->next = NULL;
	vbuf = vbuf->next;
    }
    start.x = (short) x1;
    start.y = (short) y1;
    end.x = (short) x2;
    end.y = (short) y2;
    ras_line(start, end, vbuf, orig_ximg, orig_img);
    /* copy from the retained point list into the current buffer of points. */
    bcopy((unsigned char *) vbuf->points, (unsigned char *) pind,
	  sizeof(struct pval) * vbuf->len);
    pind += vbuf->len;
    last = pind - 1;
    psz += vbuf->len;
}

/* "move":  put point at x, y
 * "draw":  draw a line to x, y
 * "dblmove":  move, takes doubles as input
 * "dbldraw":  move, takes doubles as input
 */

/* This comment is from Superplot:
 * based on the spline algorithms in:
 * FIG : Facility for Interactive Generation of figures
 * by Supoj Sutanthavibul (supoj@sally.UTEXAS.EDU)
 * used with permission from the author.
 */



/* spline():  draw splines
 * Apparently n is the number of points, and v is a vector of
 * numbers, length 2*n, with the even entries being x and the
 * odd entries being y.  k seems to be a boolean flag:
 * it seems to determine whether the spline is open or
 * closed.
 */
static void
spline(k, n, v)
    int       k;		/* Flags */
    int       n, *v;
{
    double    cx_1, cy_1, cx_2, cy_2, cx_3, cy_3, cx_4, cy_4;
    double    x_1, y_1, x_2, y_2;
    int       i;

    x_1 = v[0];
    y_1 = v[1];
    x_2 = v[2];
    y_2 = v[3];

    cx_1 = (x_1 + x_2) / 2.0;
    cy_1 = (y_1 + y_2) / 2.0;
    cx_2 = (x_1 + 3.0 * x_2) / 4.0;
    cy_2 = (y_1 + 3.0 * y_2) / 4.0;

    drawto(round(x_1), round(y_1),
	   lastx = round(cx_1), lasty = round(cy_1));

    for (i = 2; i < n; i++) {
	x_1 = x_2;
	y_1 = y_2;
	x_2 = v[i * 2];
	y_2 = v[i * 2 + 1];
	cx_3 = (3.0 * x_1 + x_2) / 4.0;
	cy_3 = (3.0 * y_1 + y_2) / 4.0;
	cx_4 = (x_1 + x_2) / 2.0;
	cy_4 = (y_1 + y_2) / 2.0;

	Cspline(cx_1, cy_1, cx_2, cy_2, cx_3, cy_3, cx_4, cy_4);

	cx_1 = cx_4;
	cy_1 = cy_4;
	cx_2 = (x_1 + 3.0 * x_2) / 4.0;
	cy_2 = (y_1 + 3.0 * y_2) / 4.0;
    }
    if (k == 0) {
	drawto(round(cx_1), round(cy_1),
	       lastx = round(x_2), lasty = round(y_2));
    } else {
	x_1 = x_2;
	y_1 = y_2;
	x_2 = v[0];
	y_2 = v[1];
	cx_3 = (3.0 * x_1 + x_2) / 4.0;
	cy_3 = (3.0 * y_1 + y_2) / 4.0;
	cx_4 = (x_1 + x_2) / 2.0;
	cy_4 = (y_1 + y_2) / 2.0;
	Cspline(cx_1, cy_1, cx_2, cy_2, cx_3, cy_3, cx_4, cy_4);
	drawto(lastx, lasty,
	       round(x_2), round(y_2));
    }
}

/****************************************************************************

	The following spline drawing routine is from

	"An Algorithm for High-Speed Curve Generation"
	by George Merrill Chaikin,
	Computer Graphics and Image Processing, 3, Academic Press,
	1974, 346-349.

	and

	"On Chaikin's Algorithm" by R. F. Riesenfeld,
	Computer Graphics and Image Processing, 4, Academic Press,
	1975, 304-310.

*****************************************************************************/

static void
Cspline(x_1, y_1, x_2, y_2, x_3, y_3, x_4, y_4)
    double    x_1, y_1, x_2, y_2, x_3, y_3, x_4, y_4;
{
    double    xmid, ymid, fabs();

    xmid = (x_2 + x_3) / 2.0;
    ymid = (y_2 + y_3) / 2.0;

    if (fabs(x_1 - xmid) < THRESHOLD && fabs(y_1 - ymid) < THRESHOLD) {
	drawto(round(x_1), round(y_1),
	       lastx = round(xmid), lasty = round(ymid));
    } else {
	Cspline(x_1, y_1, ((x_1 + x_2) / 2.0), ((y_1 + y_2) / 2.0),
		((x_2 + xmid) / 2.0), ((y_2 + ymid) / 2.0), xmid, ymid);
    }

    if (fabs(xmid - x_4) < THRESHOLD && fabs(ymid - y_4) < THRESHOLD) {
	drawto(round(xmid), round(ymid),
	       lastx = round(x_4), lasty = round(y_4));
    } else {
	Cspline(xmid, ymid, ((xmid + x_3) / 2.0), ((ymid + y_3) / 2.0),
		((x_3 + x_4) / 2.0), ((y_3 + y_4) / 2.0), x_4, y_4);
    }
}

/*****************************************************************/

char     *
spline_info(reg)		/* create string containing message giving
				 * info on spline */
    struct region *reg;
{
    char      mesg[80];
    int       xmax, xmin, ymax, ymin, width, height;
    struct plist *tr;
    
    xmax = xmin = reg->r_plist->pt.x;
    ymax = ymin = reg->r_plist->pt.y;

    /* find min and max of points defining the spline */

    for (tr = reg->r_plist; tr != NULL; tr = tr->next) {
	if (tr->pt.x > xmax)
	    xmax = tr->pt.x;
	if (tr->pt.x < xmin)
	    xmin = tr->pt.x;
	if (tr->pt.y > ymax)
	    ymax = tr->pt.y;
	if (tr->pt.y < ymin)
	    ymin = tr->pt.y;
    }
    width = xmax - xmin;
    height = ymax - ymin;

    sprintf(mesg, "Spline width: %d pixels; Spline height: %d pixels ",
	    width, height);

    return (mesg);
}

