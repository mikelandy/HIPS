/*
 * box.c -- routines for selecting box regions
 *
 */

#include <stdio.h>
#include <X11/Xlib.h>

#include "ui.h"
#include "reg.h"
#include "common.h"
#include "llist.h"
#include "display.h"
#include "scale.h"

draw_box(reg)
    struct region *reg;
{
    box_interp(reg->r_dlist, reg->r_plist);
    draw_dlist(img_win->d_xid, reg->r_dlist);
}

box_interp(dlist, ptlist)
    struct dlist *dlist;
    struct plist *ptlist;
{
    struct plist *org, *dst;
    XPoint    p1, p2;
    struct dlist *seg;

    seg = dlist;
    org = ptlist;
    dst = org->next;
    p1 = org->pt;
    p2.y = p1.y;
    p2.x = dst->pt.x;
    ras_line(p1, p2, seg, orig_ximg, orig_img);

    seg = (struct dlist *) malloc(sizeof(struct dlist));
    seg->next = seg->prev = NULL;
    seg->len = seg->flags = 0;
    llist_add((llist *) seg, (llist **) & dlist, (llist **) NULL);


    p1 = p2;
    p2 = dst->pt;
    ras_line(p1, p2, seg, orig_ximg, orig_img);

    seg = (struct dlist *) malloc(sizeof(struct dlist));
    seg->next = seg->prev = NULL;
    seg->len = seg->flags = 0;
    llist_add((llist *) seg, (llist **) & dlist, (llist **) NULL);

    p1 = p2;
    p2.x = org->pt.x;
    ras_line(p1, p2, seg, orig_ximg, orig_img);

    seg = (struct dlist *) malloc(sizeof(struct dlist));
    seg->next = seg->prev = NULL;
    seg->len = seg->flags = 0;
    llist_add((llist *) seg, (llist **) & dlist, (llist **) NULL);

    p1 = p2;
    p2 = org->pt;
    ras_line(p1, p2, seg, orig_ximg, orig_img);
}

box_direction(pl)
    struct plist *pl;
{
    int       h, v;

    v = pl->next->pt.y - pl->pt.y;
    h = pl->next->pt.x - pl->pt.x;
    if (h > v)
	return HORIZONTAL;
    else
	return VERTICAL;
}


/*****************************************************************/

char     *
box_info(reg)			/* create string containing message giving
				 * info on spline */
    struct region *reg;
{
    char      mesg[80];
    int       x1, x2, y1, y2, width, height;

    x1 = reg->r_plist->pt.x;
    y1 = reg->r_plist->pt.y;
    x2 = reg->r_plist->next->pt.x;
    y2 = reg->r_plist->next->pt.y;
    width = abs(x2 - x1);
    height = abs(y2 - y1);

    sprintf(mesg, "Box cols: %d pxls;  rows: %d pxls;  area: %d ",
	    width, height, width * height);

    return (mesg);
}
