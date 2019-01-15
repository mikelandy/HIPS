/*
 * xscale.c -- Xlib scale labeller
 *
 */

#include <stdio.h>
#include <X11/Xlib.h>
#include "scale.h"

/*
 * xlabelgraph() places a graph label structure in an X Window.
 * xid is the window, lvec and len are the graph_lab vector and its length.
 * (x,y) is the starting point for the labels, orientation is either VERTICAL
 * or HORIZONTAL (as defined in scale.h) and direction is either +1 or -1.
 *
 */

xlabelgraph(display, xid, gc, lvec, len, x, y, width, height, orientation, direction)
    Display  *display;
    XID       xid;
    GC        gc;
    struct graph_lab *lvec;
    int       len, x, y, width, height, orientation, direction;
{
    XGCValues gcval;
    int       i;
    XPoint    loc, org;

    loc.x = x;
    loc.y = y;
    org.x = loc.x - width;
    org.y = loc.y - height;
    for (i = 0; i < len; i++) {
	if (orientation == HORIZONTAL) {
	    loc.x = x + lvec[i].p_off * direction;
	    loc.y = y + 15;
	    org.x = loc.x;
	} else {
	    loc.y = y + lvec[i].p_off * direction;
	    org.y = loc.y;
	}
	gcval.line_style = LineOnOffDash;
	XChangeGC(display, gc, GCLineStyle, &gcval);
	XDrawLine(display, xid, gc, org.x, org.y, loc.x, loc.y);
	if (orientation == HORIZONTAL)
	    XDrawImageString(display, xid, gc, loc.x, loc.y,
			     lvec[i].lab, strlen(lvec[i].lab));
	else
	    XDrawImageString(display, xid, gc, loc.x + 2, loc.y,
			     lvec[i].lab, strlen(lvec[i].lab));
    }
    gcval.line_style = 0;
    XChangeGC(display, gc, GCLineStyle, &gcval);
}
