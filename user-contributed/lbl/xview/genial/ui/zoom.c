/*
 * zoom.c
 */

#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/textsw.h>
#include <xview/xv_xrect.h>
#include "display.h"
#include "ui.h"
#include "reg.h"
#include "zoom_ui.h"
#include "zoom.h"

extern void       zoom_resize_proc();
 
static Cursor lcurs = NULL, rcurs, ucurs, dcurs;	/* cursors in left,
							 * right, up and down
							 * directions */
setcursor(win, dir)
    Xv_Window win;
    int       dir;
{
    XID       xid = (XID) xv_get(win, XV_XID);

    switch (dir) {
    case LEFT:
	XDefineCursor(display, xid, lcurs);
	break;
    case RIGHT:
	XDefineCursor(display, xid, rcurs);
	break;
    case UP:
	XDefineCursor(display, xid, ucurs);
	break;
    case DOWN:
	XDefineCursor(display, xid, dcurs);
	break;
    }
}

/* routine to make our cursors */
zoom_mk_cursors()
{
    lcurs = XCreateFontCursor(display, XC_left_side);
    rcurs = XCreateFontCursor(display, XC_right_side);
    ucurs = XCreateFontCursor(display, XC_top_side);
    dcurs = XCreateFontCursor(display, XC_bottom_side);
}

/*
 * Event callback function for `zmwin'.
 */
Notify_value
zoom_zmwin_event_callback(win, event, arg, type)
    Xv_window win;
    Event    *event;
    Notify_arg arg;
    Notify_event_type type;
{
#ifdef DEBUG
    fprintf(stderr, "zoom: zoom_zmwin_event_callback: event %d\n",
	    event_id(event));
#endif

    if (event_action(event) == WIN_RESIZE) {
	zoom_resize_proc(win, event, arg, type);
    }

    return notify_next_event_func(win, (Notify_event) event, arg, type);
}
