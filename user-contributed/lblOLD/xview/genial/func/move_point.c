/*
 * move_point.c -- routines for moving individual points in a plist
 *
 * uses an experimental claim_state() interface to temporarily claim control
 * of the UI
 *
 */

#include "common.h"
#include "llist.h"
#include "display.h"
#include "ui.h"
#include "reg.h"
#include "log.h"
#include "sm.h"

struct plist *mpt;		/* point which we are moving */

/* move_point(): given an (x,y) position of the mouse, find nearest point and
 * interactively move it until the button is released.
 *
 */

move_point(x, y)
    int       x, y;
{
    int       move_dispatch();
    XPoint    pt;

#ifdef TRACE_EX
     fprintf(stderr, "in move_point \n");
#endif

    /*
     * claim the state so that we can intercept mouse events to achieve an
     * interactive move
     */
    dp_push(move_dispatch);
    /* fill mpt appropriately */
    pt.x = x;
    pt.y = y;
#ifdef DEBUG
    printf("moving point at (%d,%d)\n", x, y);
#endif
    mpt = pfind(pt);
    if (mpt == NULL) {
	printf("No such point near position (%d,%d)!\n", x,y);
	XBell(display, 0);
	dp_del(move_dispatch);
    }
}

/***************************************************************/
int
move_dispatch(token, arg)
    int       token;		/* token as defined in sm.h */
    caddr_t   arg;		/* pointer to an optional argument */
{
    struct plist *getcpl();

    if (token == IMG_BUT) {
	/* move the points */
	/* replace old cross */
	ref_cb(img_win->d_xid, &mpt->cb);
	/* and draw a new one */
	mpt->pt.x = dtoi(event_x((Event *) arg));
	mpt->pt.y = dtoi(event_y((Event *) arg));
#ifdef DEBUG
	printf("dispatch: moving point at (%d,%d)\n",
	       (int) mpt->pt.x, (int) mpt->pt.y);
#endif
	cbget(orig_ximg, &mpt->cb, mpt->pt);
	XSetForeground(display, gc, standout);
	draw_cb(img_win->d_xid, &mpt->cb);
	if ((event_action((Event *) arg) == LOC_DRAG) &&
	    (event_is_down((Event *) arg))) {
    		return dispatch_next(token,(char **)arg);
	} else {
#ifdef DEBUG
	    printf("clearing move_dispatch() from the dispatch stack \n");
#endif
	    dp_del(move_dispatch);

	    if (curfunc->reg != NULL) {
		refresh_reg(curfunc->reg);	/* refresh points under old line */
		if (curfunc->reg->r_dlist != NULL) {
		    curfunc->reg->r_dlist->len = 0;
		    curfunc->reg->r_dlist->prev =
			curfunc->reg->r_dlist->next = NULL;
		    curfunc->reg->r_dlist->flags = 0;
		}
	    }
	    pvreg_set(getcpl());
	}
    }
    return dispatch_next(token,(char **)arg);
}
