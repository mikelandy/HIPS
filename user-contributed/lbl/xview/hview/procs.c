#include <stdio.h>
#include <strings.h>
#include <math.h>
#include <sys/param.h>
#include <sys/types.h>
#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/textsw.h>
#include <xview/xv_xrect.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <xview/cms.h>

#include <hipl_format.h>

#include "hview_ui.h"

extern hview_win_objects *Hview_win;
extern byte *image_data, *image_map, *image_map24;
extern int width, height, im_size;
extern XImage *ximage;
extern int numcol;
extern int depth;

extern Display *display;
extern XID xid;
extern Window mainw;
extern Xv_Window paintwin;

extern int noglob, perfect, ncols, rwcolor, mono;

void      share_proc(), change_mode(), mono_proc();

/*************************************************************/
void
quit_proc(item, event)
    Panel_item item;
    Event    *event;
{
    FreeAllColors();

    xv_set(Hview_win->win, FRAME_NO_CONFIRM, TRUE, NULL);
    (void) xv_destroy_safe(Hview_win->win);
}

/*************************************************************/
void
hview_win_slider1_notify_callback(item, value, event)
    Panel_item item;
    int       value;
    Event    *event;
{
    void      gamma_proc();

    gamma_proc(item, value, event);
}

/*************************************************************/
void
make_perfect_colors(item, event)
    Panel_item item;
    Event    *event;
{
    perfect = abs(perfect - 1);	/* toggle color flash OK flag */

    if (perfect)
	fprintf(stderr, "making perfect colormap \n");
    else
	fprintf(stderr, "making close colormap \n");

    make_new_colormap();
}

/*************************************************************/
make_new_colormap()
{
    FreeAllColors();

    if (rwcolor)
	AllocRWColors(noglob, numcol, perfect, mono, ncols);
    else
	AllocColors(noglob, perfect, mono, numcol, ncols);

    /* remap image_map to color indexes */
    map_image_to_colors(ximage->data, image_map, im_size);

    canvas_repaint_proc(Hview_win->can, paintwin, display, xid, NULL);

    fprintf(stderr, "Done. \n");
}

/***********************************************************************/
/*
 * Event callback function for `win'.
  */
Notify_value
hview_win_event_callback(win, event, arg, type)
    Xv_window win;
    Event    *event;
    Notify_arg arg;
    Notify_event_type type;
{

#ifdef DEBUG
    fprintf(stderr, "hview: hview_win_event_callback: event %d\n", event_id(event));
#endif

    /* gxv_start_connections DO NOT EDIT THIS SECTION */

    if (event_action(event) == WIN_RESIZE) {
	imgwin_resize_proc(win, event, arg, type);
    }
    /* gxv_end_connections */

    return notify_next_event_func(win, (Notify_event) event, arg, type);
}
