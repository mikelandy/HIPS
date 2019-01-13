/* zoom.c
 *
 * Max Rible
 * HIPStool zoom procedures.
 */

#include "hipstool.h"

struct zoomwin {
    Windowinfo winfo;
    Panel panel;
    Panel_item slider;
    int x, y;
};

static struct zoomwin zoom;
static int mag = 2;
static void zoom_event_proc();
static Pixrect *tmpr = NULL;

void
do_zoom(x, y)
     int x, y;
{
    int siz = 128 / mag;
    int i, j, delta, xnew, ynew;

    if(x < 0 || y < 0) return;

    if(zoom.winfo.frame == NULL) {
	newindow(&zoom.winfo, 256, 256, -128, -128, "HIPStool Zoom");
	init_cursor(&zoom.winfo);
	zoom.winfo.pr = mem_create(256, 256, 8);
	window_set(zoom.winfo.canvas, 
		   WIN_EVENT_PROC, zoom_event_proc,
		   WIN_CONSUME_PICK_EVENTS, WIN_NO_EVENTS,
		   LOC_MOVE, WIN_MOUSE_BUTTONS, 0,
		   0);
	if(tmpr == NULL) 
	    tmpr = mem_create(256, 256, 8);
    }

    batching(&zoom.winfo, ON);

    xnew = MAX(MIN(x, base.winfo.width - siz), siz);
    ynew = MAX(MIN(y, base.winfo.height - siz), siz);

    zoom.x = xnew; zoom.y = ynew;

    /* Trick used in UCB Imageview, zoom.c.  It gives credit to
     * Marc Weiser.
     */

    pw_read(tmpr, 0, 0, 256, 256, PIX_SRC, base.winfo.pw,
	    xnew - 128, ynew - 128);

    for(i = 0; i < 2*siz; i++)
	for(delta = 0; delta < mag; delta++)
	    pr_rop(zoom.winfo.pr, i*mag + delta, 0, 1, 256, PIX_SRC,
		   tmpr, 128 - siz + i, 128 - siz);
    for(j = 2*siz - 1; j >= 0; j--)
	for(delta = 0; delta < mag; delta++)
	    pr_rop(zoom.winfo.pr, 0, j*mag + delta, 256, 1, PIX_SRC,
		   zoom.winfo.pr, 0, j);

    pw_rop(zoom.winfo.pw, 0, 0, 256, 256, PIX_SRC, zoom.winfo.pr, 0, 0);

    batching(&zoom.winfo, OFF);
}

/* ARGSUSED */
static void
zoom_event_proc(window, event, arg)
     Window window;
     Event *event;
     caddr_t arg;
{
    int x, y;

    x = event_x(event); y = event_y(event);
    x -= 128; y -= 128; x /= mag; y /= mag;
    x += zoom.x; y += zoom.y;

    if(event_meta_is_down(event) && !event_is_up(event) &&
       event_action(event) == MS_MIDDLE) {
	mag *= 2; if(mag > 8) mag = 1;
	do_zoom(zoom.x, zoom.y);
	return;
    } else if(event_action(event) == LOC_MOVE) {
	Update_cursor(x, y, (*readarr[base.datasize])(x, y, 0));
    } else {
	event_set_x(event, x); event_set_y(event, y);
	base_event_proc(window, event, arg);
	do_zoom(zoom.x, zoom.y);
    }
}

void
kill_zoom()
{
    if(zoom.winfo.frame != NULL) {
	killwindow(&zoom.winfo);
	zoom.x = zoom.y = -1;
    }
}
