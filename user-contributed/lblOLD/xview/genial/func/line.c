/*
 * line.c -- routines for drawing line regions
 */

#include "common.h"
#include "llist.h"
#include "reg.h"
#include "display.h"
#include "ui.h"
#include <X11/Xlib.h>
#include <stdio.h>

/* Keep a huge buffer that way we only have to malloc and bcopy() at the
   end of the sleection process */

#define PVEC_SIZE 1450

static struct pval pvec[PVEC_SIZE];	/* temporary buffer for storing
					 * points */

/* draw_line() actually draws a line from the region */
draw_line(reg)
    struct region *reg;
{
    struct plist *trav;
    float    *xvec, *yvec;
    int       len, i;
    XPoint    p1, p2;

    if (reg->r_type == DUBLIN) {
	draw_double_line(reg->r_dlist, reg->r_plist);
	draw_dlist(img_win->d_xid, reg->r_dlist);
	return;
    }
    if (reg->r_flags == LSQ) {
	/* call the measley fit() algorithm */
	len = llist_depth((llist *) reg->r_plist);
	if ((xvec = (float *) malloc(len * sizeof(float))) == NULL) {
	    perror("malloc");
	    exit(0);
	}
	if ((yvec = (float *) malloc(len * sizeof(float))) == NULL) {
	    perror("malloc");
	    exit(0);
	}
	for (i = 0, trav = reg->r_plist; trav != NULL; trav = trav->next, i++) {
	    xvec[i] = (float) trav->pt.x;
	    yvec[i] = (float) trav->pt.y;
	}
	fit(xvec, yvec, len, &p1, &p2);
	free(xvec);
	free(yvec);
    } else {
	if (reg->r_plist == NULL || reg->r_plist->next == NULL)
	    return;
	p1.x = reg->r_plist->pt.x;
	p1.y = reg->r_plist->pt.y;
	p2.x = reg->r_plist->next->pt.x;
	p2.y = reg->r_plist->next->pt.y;
    }
    ras_line(p1, p2, (struct dlist *) llist_tail((llist *) reg->r_dlist),
	     orig_ximg, orig_img);
    draw_dlist(img_win->d_xid, reg->r_dlist);
}

/*****************************************************************/
draw_double_line(dlist, ptlist)
    struct dlist *dlist;
    struct plist *ptlist;
{
    XPoint    p1, p2;
    struct plist *pl;
    struct dlist *seg;

    seg = dlist;
    if (seg == NULL)
	return -1;
    pl = ptlist;
    if (pl == NULL)
	return -1;

    p1.x = pl->pt.x;
    p1.y = pl->pt.y;
    pl = pl->next;
    if (pl == NULL)
	return -1;
    p2.x = pl->pt.x;
    p2.y = pl->pt.y;

    ras_line(p1, p2, seg, orig_ximg, orig_img);
    draw_dlist(img_win->d_xid, dlist);

    seg = (struct dlist *) malloc(sizeof(struct dlist));
    seg->next = seg->prev = NULL;
    seg->len = seg->flags = 0;

    llist_add((llist *) seg, (llist **) & dlist, (llist **) NULL);

    pl = pl->next;
    if (pl == NULL)
	return -1;

    p1.x = pl->pt.x;
    p1.y = pl->pt.y;
    pl = pl->next;
    if (pl == NULL)
	return -1;
    p2.x = pl->pt.x;
    p2.y = pl->pt.y;

    ras_line(p1, p2, seg, orig_ximg, orig_img);

    return(1);
}

/*******************************************************************/

/* actual line rasterization algorithm (Bresenham) */
ras_line(spt, ept, dlist, bgimg, scimg)
    XPoint    spt, ept;
    struct dlist *dlist;
    XImage   *bgimg;
    struct img_data *scimg;
{
    register int npts, dx, dy;
    register int x1, x2, y1, y2;
    register int yinc, xinc;
    register int err;
    register int x, y;

    npts = 0;
    x1 = (int) spt.x;
    y1 = (int) spt.y;
    x2 = (int) ept.x;
    y2 = (int) ept.y;
    dx = x2 - x1;
    dy = y2 - y1;
    if (abs(dx) > abs(dy)) {
	if (dx < 0) {
	    x1 = x2;
	    x2 = spt.x;
	    y1 = y2;
	    y2 = spt.y;
	}
	if (y2 > y1)
	    yinc = 1;
	else
	    yinc = -1;
	/* first octant algorithm */
	err = -(dx / 2);
	x = x1;
	y = y1;
	pvec[npts].pt.x = (short) x;
	pvec[npts].pt.y = (short) y;
	pvec[npts].val = (unsigned char) XGetPixel(bgimg, x, y);
	pvec[npts].oval = dval(x, y, scimg, 0);
	npts++;
	if (npts >= PVEC_SIZE - 1)
	    return;
	while (x < x2) {
	    err += abs(dy);
	    if (err > 0) {
		y += yinc;
		err -= abs(dx);
	    }
	    x++;
	    pvec[npts].pt.x = (short) x;
	    pvec[npts].pt.y = (short) y;
	    pvec[npts].val = (unsigned char) XGetPixel(bgimg, x, y);
	    pvec[npts].oval = dval(x, y, scimg, 0);
	    npts++;
	    if (npts >= PVEC_SIZE - 1)
		return;
	}
    } else {
	if (dy < 0) {
	    x1 = x2;
	    x2 = spt.x;
	    y1 = y2;
	    y2 = spt.y;
	}
	if (x2 > x1)
	    xinc = 1;
	else
	    xinc = -1;
	/* second octant algorithm. */
	err = -(dy / 2);
	x = x1;
	y = y1;
	pvec[npts].pt.x = (short) x;
	pvec[npts].pt.y = (short) y;
	pvec[npts].val = (unsigned char) XGetPixel(bgimg, x, y);
	pvec[npts].oval = dval(x, y, scimg, 0);
	npts++;
	if (npts >= PVEC_SIZE - 1)
	    return;
	while (y < y2) {
	    err += abs(dx);
	    if (err > 0) {
		x += xinc;
		err -= abs(dy);
	    }
	    y++;
	    pvec[npts].pt.x = (short) x;
	    pvec[npts].pt.y = (short) y;
	    pvec[npts].val = (unsigned char) XGetPixel(bgimg, x, y);
	    pvec[npts].oval = dval(x, y, scimg, 0);
	    npts++;
	    if (npts >= PVEC_SIZE - 1)
		return;
	}
    }

    /* put them in the dlist structure */
    dlist->points = (struct pval *) malloc(sizeof(struct pval) * (npts));
    if (dlist->points == NULL) {
	perror("malloc");
	exit(1);
    }
    dlist->len = npts;
    bcopy((char *) pvec, (char *) dlist->points, sizeof(struct pval) * npts);
}

/* This routine was taken from the Numerical Recipes file
 * "fit.c".  I stripped out such things as significance and
 * so on.
 */
fit(x, y, ndata, p1, p2)	/* least squares fit algorithm */
    float     x[], y[];
    int       ndata;
    XPoint   *p1, *p2;
{
    int       i;
    float     t, sxoss, sx = 0.0, sy = 0.0, st2 = 0.0, ss, a, b;
    float     minx = 65535.0, maxx = 0.0;

    b = 0.0;
    for (i = 0; i < ndata; i++) {
	sx += x[i];
	sy += y[i];
	if (x[i] < minx)
	    minx = x[i];
	if (x[i] > maxx)
	    maxx = x[i];
    }
    ss = ndata;
    sxoss = sx / ss;
    for (i = 0; i < ndata; i++) {
	t = x[i] - sxoss;
	st2 += t * t;
	b += t * y[i];
    }
    b = b / st2;
    a = (sy - sx * (b)) / ss;

    p1->x = (short) minx;
    p1->y = (short) (a + b * minx);
    p2->x = (short) maxx;
    p2->y = (short) (a + b * maxx);
}

/*****************************************************************/

char     *
line_info(reg)			/* create string containing message giving
				 * length of a line */
    struct region *reg;
{
    char      mesg[80];
    int       x1, x2, y1, y2;
    double    dist;

    if (reg == NULL)
	return (NULL);
    if (reg->r_plist == NULL)
	return (NULL);
    if (reg->r_plist->next == NULL)
	return (NULL);

    /* this does not handle LS mode properly yet ! */

    x1 = reg->r_plist->pt.x;
    y1 = reg->r_plist->pt.y;
    x2 = reg->r_plist->next->pt.x;
    y2 = reg->r_plist->next->pt.y;

    dist = sqrt(pow((double) (x2 - x1), (double) 2) +
		pow((double) (y2 - y1), (double) 2));

    sprintf(mesg, "Length of current line is %d pixels ", (int) (dist + .5));
    return (mesg);
}
