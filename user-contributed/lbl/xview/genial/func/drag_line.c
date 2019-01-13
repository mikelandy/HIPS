/*
 * drag_line.c : click and drag line drawing mode
 *
 */

#include "common.h"
#include "ui.h"
#include "llist.h"
#include "display.h"
#include "reg.h"
#include "sm.h"

struct plist *mpt;		/* point which we are moving */

/*
 * start_line() -- takes an (x,y) starting point for a line, and pushes a
 * routine to drag the endpoint of the line on to the dispatch function
 * stack.
 */

start_line(x, y)
    int       x, y;
{
    int       dragline_dispatch();

    if ((mpt = (struct plist *) malloc(sizeof(struct plist))) == NULL) {
	perror("malloc");
	exit(1);
    }
    /*
     * claim the state so that we can intercept mouse events to achieve an
     * interactive move
     */
    dp_push(dragline_dispatch);
    /* fill mpt appropriately */
    mpt->pt.x = x;
    mpt->pt.y = y;
    printf("moving point at (%d,%d)\n", x, y);
    /* add this (x,y) as the position of the first point */
    add_point(x, y);
    add_point(x, y);
    mpt = pfind(mpt->pt);
}

int
dragline_dispatch(token, arg)
    int       token;		/* token as defined in sm.h */
    caddr_t   arg;		/* pointer to an optional argument */
{
    struct plist *getcpl();

    if (token == IMG_BUT) {
	if ((event_action((Event *) arg) == LOC_DRAG) &&
	    (event_is_down((Event *) arg))) {
	    /* move the points */
	    /* replace old cross */
	    ref_cb(img_win->d_xid, &mpt->cb);
	    /* and draw a new one */
	    mpt->pt.x = dtoi(event_x((Event *) arg));
	    mpt->pt.y = dtoi(event_y((Event *) arg));
#ifdef DEBUG
	    printf("moving point at (%d,%d)\n", (int) mpt->pt.x, (int) mpt->pt.y);
#endif
	    cbget(orig_ximg, &mpt->cb, mpt->pt);
	    XSetForeground(display, gc, standout);
	    draw_cb(img_win->d_xid, &mpt->cb);
	    pvreg_set(getcpl());
	    return 1;
	} else {
	    if (event_is_up((Event *) arg)) {
#ifdef DEBUG
		printf("clearing move_dispatch() from the dispatch stack \n");
#endif
		dp_del(dragline_dispatch);
	    }
	}
    }
    return dispatch_next(token, (char **)arg);
}
