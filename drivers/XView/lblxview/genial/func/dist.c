/*
 * dist.c -- routines for measuring distance
 *
 */

#include "ui.h"
#include "display.h"
#include "log.h"
#include "reg.h"
#include "common.h"

dist_init()
{
    clear_info();
    lab_info("Please select a line for distance measure", 1);
    lab_info("hit <eval> when finished", 2);

    setrtype(LINE);
    reg_setdom(NONE);

    return 0;
}

dist_eval()
{
    struct dlist *dl;
    double    d;
    static char msg[80];
    XPoint    p1, p2;

    dl = curfunc->reg->r_dlist;
    p1.x = dl->points[0].pt.x;
    p1.y = dl->points[0].pt.y;
    p2.x = dl->points[dl->len - 1].pt.x;
    p2.y = dl->points[dl->len - 1].pt.y;
    d = distance(p1, p2);
    sprintf(msg, "Distance from (%d,%d) to (%d,%d): %.2f", 
	    p1.x, p1.y, p2.x, p2.y, (float) d);

    xv_set(base_win->infomesg, PANEL_LABEL_STRING, msg, NULL);
    panel_paint(base_win->infomesg, PANEL_CLEAR);

    return 0;
}

dist_clear()
{
    clear_info();
    return 0;
}
