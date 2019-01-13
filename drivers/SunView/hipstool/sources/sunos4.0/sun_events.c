/* sun_events.c
 * Max Rible
 * 
 * Sunview event handling for hipstool.
 */

#include "hipstool.h"

#ifdef SUNTOOLS
/* ARGSUSED */
void
base_event_proc(window, event, arg)
     Window window;
     Event *event;
     caddr_t arg;
{
    int x, y, dx = 0, dy = 0, up;
    Bool hbar, vbar;

    x = event_x(event); y = event_y(event);
    up = event_is_up(event);

    switch(event_action(event)) {
    case WIN_RESIZE:		/* It gets called for these anyway */
	x = (int) window_get(base.winfo.frame, WIN_WIDTH) - MARG_X;
	y = (int) window_get(base.winfo.frame, WIN_HEIGHT) - MARG_Y;
	if(base.winfo.vbar != NULL) 
	    x -= (dx = (int) scrollbar_get(base.winfo.vbar, SCROLL_THICKNESS));
	if(base.winfo.hbar != NULL) 
	    y -= (dy = (int) scrollbar_get(base.winfo.hbar, SCROLL_THICKNESS));
	if(base.winfo.height > y) vbar = 1; else vbar = 0;
	if(base.winfo.width > x) hbar = 1; else hbar = 0;
	scrollset(&(base.winfo), vbar, hbar);
	if(x > base.winfo.width)
	    window_set(base.winfo.frame, WIN_WIDTH, 
		       base.winfo.width + MARG_X + (vbar ? dx : 0), 0);
	if(y > base.winfo.height)
	    window_set(base.winfo.frame, WIN_HEIGHT, 
		       base.winfo.height + MARG_Y + (hbar ? dy : 0), 0);
    case SCROLL_REQUEST:	/* Fall through */
	update_frame_label(&base);
	return;
    case WIN_REPAINT:		/* Why?  I don't know. */
    case KBD_USE:
    case KBD_DONE:
    case SCROLL_ENTER:
    case SCROLL_EXIT:
    case LOC_RGNENTER:
	return;
    }

    if((x < 0) || (y < 0)) return;

    if((dx = (x % resolution)) || (dy = (y % resolution))) {
	if(dx < resolution/2) x -= dx; else x += resolution - dx;
	if(dy < resolution/2) y -= dy; else y += resolution - dy;
	event_set_x(event, x); event_set_y(event, y);
	window_set(base.winfo.frame, WIN_MOUSE_XY, 
		   event_x(canvas_window_event(base.winfo.canvas, event)) + 
		   BORDER, event_y(event) + TOP_BORDER, 0);
    }

    Update_cursor(x, y, (*readarr[base.datasize])(x, y, 0));

    if(event_meta_is_down(event)) {
	if(up) return;
	switch(event_action(event)) { 
	case MS_LEFT:	do_zoom(x, y); break;
	case MS_RIGHT:	kill_zoom(); break;
	}
	return;
    }

    if(cur_func == NULL) return;

    switch(event_action(event)) {
    case LOC_MOVE:
	break;
    case MS_LEFT:		/* Undo rectangle and set upper left */
	select_search(x, y, up, cur_func);
	break;
    case LOC_DRAG:
	if(up) fprintf(stderr, "LOC_DRAG up events exist.  ");
	if(window_get(base.winfo.canvas, WIN_EVENT_STATE, MS_RIGHT))
	    break;
    case MS_MIDDLE:		/* Set lower right of rectangle */
	if(complete || 
	   ( (priminfo[cur_func->func.primit->breed].modify == list_modify) &&
	    (window_get(base.winfo.canvas, WIN_EVENT_STATE, MS_MIDDLE) || up)))
	    modify_search(x, y, up, cur_func);
	else
	    select_search(x, y, up, cur_func);
	break;
    case MS_RIGHT:		/* Restore original image */
	if(!up && functions[cur_func->breed].right_eval) {
	    (*functions[cur_func->breed].eval)(cur_func);
	    if(auto_log)
		log_current_action();
	}
	break;
    default:
	fprintf(stderr, "Base received %d.  ", event_action(event)); 
	fflush(stderr);
	break;
    }
    return;
}

/* ARGSUSED */
void
child_event_proc(window, event, arg)
     Window window;
     Event *event;
     caddr_t arg;
{
    int x, y;

    x = event_x(event);
    y = event_y(event);

    /* Change frame parameters if it receives a RESIZE event! */
    switch(event_action(event)) {
    case SCROLL_REQUEST:
    case WIN_RESIZE:
	update_frame_label(&proj);
	break;
    default:
	break;
    }

    if(event_is_up(event) || (x < 0) || (y < 0)) return;

    Update_cursor(x, y, 0);

    return;
}
#endif /* SUNTOOLS */
