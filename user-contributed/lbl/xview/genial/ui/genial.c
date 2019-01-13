
/*
 * genial.c
 */

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <xview/cms.h>
#include "display.h"
#include "ui.h"
#include "sm.h"
#include "log.h"
#include "histo_ui.h"
#include "trace_ui.h"
#include "comment_ui.h"
#include "gfm_ui.h"

Xv_Window paintwin;
Display  *display;
Visual   *winv;
XID       xid;
GC        gc;
int       depth;

int       line_mode = CLICK;
int       clean_mode = 0;

Attr_attribute INSTANCE;

/*
 * Global object definitions.
  */

main_control_ctrlwin_objects *base_win;
main_control_imgwin_objects *genial_imgwin;
file_window1_objects *file_win;
display_ctrlwin_objects *disp_ctrl;
gfm_popup_objects *gfm_ctrl;
comment_comm_win_objects *comment_win;

struct disp_win *img_win;

char     *func_names[] =
{"Trace", "Histogram", "Zoom", "Distance",
 "Angle Measure", "Annotate", "Comment"};

extern void imgwin_resize_proc();

/* function to initialize the components of the User Interface */
void
init_ui()
{
    Cms cms;
    static Xv_singlecolor colors[] = {
	{ 255, 0, 0 }, /* red */
        { 0, 0, 255 }, /* blue */
    };

    XGCValues gcval;
    gfm_popup_objects *gfm_initialize();

    /*
     * Initialize user interface components.
     */
    base_win = main_control_ctrlwin_objects_initialize(NULL, NULL);
    genial_imgwin = main_control_imgwin_objects_initialize(NULL, base_win->ctrlwin);
    file_win = file_window1_objects_initialize(NULL, base_win->ctrlwin);
    disp_ctrl = display_ctrlwin_objects_initialize(NULL, base_win->ctrlwin);
    comment_win = comment_comm_win_objects_initialize(NULL, base_win->ctrlwin);
    gfm_ctrl = gfm_initialize(NULL, file_win->window1, "Genial File Control");

    /* set color for help text */
    cms = (Cms) xv_create(NULL, CMS,
			  CMS_CONTROL_CMS, TRUE,
			  CMS_SIZE, CMS_CONTROL_COLORS + 2,
			  CMS_COLORS, colors,
			  NULL);
    xv_set(base_win->controls2, WIN_CMS, cms, NULL);
    /* set all messages to red */
    xv_set(base_win->statmsg1, PANEL_ITEM_COLOR, CMS_CONTROL_COLORS, NULL);
    xv_set(base_win->statmsg2, PANEL_ITEM_COLOR, CMS_CONTROL_COLORS, NULL);
    xv_set(base_win->statmsg3, PANEL_ITEM_COLOR, CMS_CONTROL_COLORS, NULL);
    xv_set(base_win->statmsg4, PANEL_ITEM_COLOR, CMS_CONTROL_COLORS, NULL);

    /* allocate and initialize img_win */

    img_win = (struct disp_win *) malloc(sizeof(struct disp_win));
    if (img_win == NULL) {
	perror("malloc");
	exit(0);
    }
    img_win->d_win = (Xv_Window) genial_imgwin->imgwin;
    img_win->d_canv = (Canvas) genial_imgwin->imgcanv;
    img_win->d_paintwin = (Xv_Window) canvas_paint_window(img_win->d_canv);
    img_win->d_xid = (XID) xv_get(img_win->d_paintwin, XV_XID);

    /* set up display, winv and GC for future X calls */
    display = (Display *) xv_get(img_win->d_win, XV_DISPLAY);
    depth = DisplayPlanes(display, DefaultScreen(display));

    if (depth != 8 && depth != 24) {
	fprintf(stderr, "Error: This program requires an 8 or 24-bit color monitor \n");
	exit(0);
    }
    winv = DefaultVisual(display, DefaultScreen(display));

    /* set up a GC with 'reasonable' values */
    gcval.foreground = BlackPixel(display, DefaultScreen(display));
    gcval.background = WhitePixel(display, DefaultScreen(display));
    gcval.clip_mask = None;
    gc = XCreateGC(display, img_win->d_xid, GCForeground | GCBackground |
		   GCClipMask, &gcval);

    xv_set(file_win->window1, XV_SHOW, FALSE, NULL);

    xv_set(base_win->infomesg, PANEL_LABEL_STRING, "  ", NULL);
    xv_set(base_win->image_fname, PANEL_LABEL_STRING, "image not loaded", NULL);
    panel_paint(base_win->infomesg, PANEL_CLEAR);
    panel_paint(base_win->image_fname, PANEL_CLEAR);

    xv_set(base_win->log_num, PANEL_LABEL_STRING, "1", NULL);
    xv_set(base_win->message9, PANEL_LABEL_STRING, "  ", NULL);
}

lab_info(val, m)
    char     *val;
    int       m;
{
    switch (m) {
    case 1:
	xv_set(base_win->statmsg1,
	       PANEL_LABEL_STRING, val,
	       NULL);
	break;
    case 2:
	xv_set(base_win->statmsg2,
	       PANEL_LABEL_STRING, val,
	       NULL);
	break;
    case 3:
	xv_set(base_win->statmsg3,
	       PANEL_LABEL_STRING, val,
	       NULL);
	break;
    case 4:
	xv_set(base_win->statmsg4,
	       PANEL_LABEL_STRING, val,
	       NULL);
	break;
    }

}

clear_info()
{
    char     *b;
    b = (char *) xv_get(base_win->statmsg1, PANEL_LABEL_STRING);
    xv_set(base_win->statmsg1,
	   PANEL_LABEL_STRING, NULL,
	   NULL);
    free(b);
    b = (char *) xv_get(base_win->statmsg2, PANEL_LABEL_STRING);
    xv_set(base_win->statmsg2,
	   PANEL_LABEL_STRING, NULL,
	   NULL);
    free(b);
    b = (char *) xv_get(base_win->statmsg3, PANEL_LABEL_STRING);
    xv_set(base_win->statmsg3,
	   PANEL_LABEL_STRING, NULL,
	   NULL);
    free(b);
    b = (char *) xv_get(base_win->statmsg4, PANEL_LABEL_STRING);
    xv_set(base_win->statmsg4,
	   PANEL_LABEL_STRING, NULL,
	   NULL);
    free(b);

    panel_paint(base_win->statmsg1, PANEL_CLEAR);
    panel_paint(base_win->statmsg2, PANEL_CLEAR);
    panel_paint(base_win->statmsg3, PANEL_CLEAR);
    panel_paint(base_win->statmsg4, PANEL_CLEAR);
}

/*
 * label_event() -- takes an event and placs the x,y and gray values under
 * the point into the ctrlwin
 *
 */

label_event(event)
    Event    *event;
{
    char      tstr[8];
    int       x = dtoi(event_x(event)), y = dtoi(event_y(event));
    unsigned  gray;

    if (pt_in_xim(x, y, orig_ximg) == 0)
	return;
    gray = dval(x, y, orig_img, 0);
    sprintf(tstr, "%d", x);
    xv_set(base_win->xlabel,
	   PANEL_LABEL_STRING, tstr,
	   NULL);
    sprintf(tstr, "%d", y);
    xv_set(base_win->ylabel,
	   PANEL_LABEL_STRING, tstr,
	   NULL);
    sprintf(tstr, "%d", gray);
    xv_set(base_win->glabel,
	   PANEL_LABEL_STRING, tstr,
	   NULL);
}

/*
 * label_img() -- fill the imglabel member of the base window with the
 * geometry of the image, extrema, and width
 *
 */
label_img(img)
    struct img_data *img;
{
    char      tstr[80];

    sprintf(tstr, "%dx%d,[%d:%d],", img->width, img->height,
	    img->minv, img->maxv);
    switch (img->dsize) {
    case 1:
	strcat(tstr, "byte");
	break;
    case 2:
	strcat(tstr, "short");
	break;
    case 4:
	strcat(tstr, "integer");
	break;
    default:
	XBell(display, 0);
	fprintf(stderr, "No such format!\n");
	break;
    }

    xv_set(base_win->imglabel,
	   PANEL_LABEL_STRING, tstr,
	   NULL);
    /* set the frame numbers and turnon the buttons if appropriate */
    sprintf(tstr, "%d/%d", img->cframe, img->nframes);
    xv_set(base_win->frame_stat,
	   PANEL_LABEL_STRING, tstr,
	   NULL);
    set_frame_buttons(img);
}

/*
 * Notify callback function for `quit'.
 */
void
quit_proc(item, event)
    Panel_item item;
    Event    *event;
{
    int       result;
    Panel     panel = (Panel) xv_get(item, PANEL_PARENT_PANEL, NULL);

    if (orig_img != NULL) {
	if (logtail != NULL && !orig_img->file_saved) {
	    result = notice_prompt(panel, NULL,
				   NOTICE_MESSAGE_STRINGS,
				   "Exit without saving log ?", NULL,
				   NOTICE_BUTTON_YES, "Yes",
				   NOTICE_BUTTON_NO, "No",
				   NULL);

	    if (result == NOTICE_NO)
		return;
	}
    }
    state_dispatch(QUIT, NULL);
}

/*
 * Notify callback function for `file'.
 */
void
file_proc(item, event)
    Panel_item item;
    Event    *event;
{
    if (xv_get(file_win->window1, XV_SHOW, NULL) == FALSE)
	xv_set(file_win->window1,
	       XV_SHOW, TRUE, FRAME_CLOSED, FALSE, NULL);
    else {
	xv_set(file_win->window1,
	       XV_SHOW, FALSE, NULL);
    }
}

/*
 * Notify callback function for `display'.
 */
void
dpyopt_proc(item, event)
    Panel_item item;
    Event    *event;
{
    if (xv_get(disp_ctrl->ctrlwin, XV_SHOW, NULL) == FALSE)
	xv_set(disp_ctrl->ctrlwin,
	       XV_SHOW, TRUE, FRAME_CLOSED, FALSE, NULL);
    else
	xv_set(disp_ctrl->ctrlwin,
	       XV_SHOW, FALSE, NULL);

#ifdef DEBUG
    fputs("genial: dpyopt_proc\n", stderr);
#endif
}


void
refresh_proc(item, event)
    Panel_item item;
    Event    *event;
{
    imgwin_repaint_proc(img_win->d_canv, img_win->d_paintwin, display,
			img_win->d_xid, NULL);

}

/*
 * Notify callback function for `prev'.
 */
void
prev_proc(item, event)
    Panel_item item;
    Event    *event;
{
    state_dispatch(BACK, NULL);
}

/*
 * Notify callback function for `next'.
 */
void
next_proc(item, event)
    Panel_item item;
    Event    *event;
{
    state_dispatch(FORW, NULL);
}

/*
 * Notify callback function for `func_sel'.
 */
void
func_sel_proc(item, value, event)
    Panel_item item;
    int       value;
    Event    *event;
{
#ifdef DEBUG
    fprintf(stderr, "genial: func_sel_proc: value: %u\n", value);
#endif
    xv_set(base_win->reg_type, PANEL_VALUE, 0, NULL);
    state_dispatch(FMENU, (caddr_t *) & value);
}

/*
 * Notify callback function for `eval'.
 */
void
eval_proc(item, event)
    Panel_item item;
    Event    *event;
{
    state_dispatch(EVAL, NULL);
}

/*
 * Notify callback function for `clear'.
 */
void
clear_proc(item, event)
    Panel_item item;
    Event    *event;
{
    state_dispatch(CLEAR, NULL);
    xv_set(base_win->infomesg, PANEL_LABEL_STRING, "  ", NULL);
    panel_paint(base_win->infomesg, PANEL_CLEAR);

}

/*
 * Notify callback function for `lmode'.
 */
void
linemode_proc(item, value, event)
    Panel_item item;
    int       value;
    Event    *event;
{
    void      reg_type_proc();

#ifdef DEBUG
    fprintf(stderr, "genial: linemode_proc: value: %u\n", value);
#endif

    if (getnpoints() == 0) {	/* can't change modes in the middle of
				 * defining a region */
	line_mode = value;
	reg_type_proc((Panel_item) NULL, (int) xv_get(base_win->reg_type,
					      PANEL_VALUE), (Event *) NULL);
    } else {
	xv_set(item, PANEL_VALUE, line_mode, NULL);
	XBell(display, 0);
	fprintf(stderr, "Error: Can't change mode while defining a region. \n");
    }
}

/*
 * Event callback function for `imgcanv'.
 */
Notify_value
imgwin_event_proc(win, event, arg, type)
    Xv_window win;
    Event    *event;
    Notify_arg arg;
    Notify_event_type type;
{
#ifdef STRONGDEBUG
    fprintf(stderr, "genial: imgwin_event_proc: event %d\n", event_id(event));
#endif

    if (orig_img == NULL)
	return notify_next_event_func(win, (Notify_event) event, arg, type);

    if (event_x(event) >= orig_img->width)
	return notify_next_event_func(win, (Notify_event) event, arg, type);
    if (event_y(event) >= orig_img->height)
	return notify_next_event_func(win, (Notify_event) event, arg, type);

    if (event_is_ascii(event) && event_is_down(event)) {
	add_char((char) event_id(event));
	return notify_next_event_func(win, (Notify_event) event, arg, type);
    }
    if (getstate() != IMG_UNLOADED)
	label_event(event);

    /* only continue is mouse button event */
    if (event_id(event) != BUT(1) && event_id(event) != BUT(2) &&
	event_id(event) != BUT(3) && event_id(event) != LOC_DRAG)
	return notify_next_event_func(win, (Notify_event) event, arg, type);

    if (event_is_down(event)) {
#ifdef DEBUG
	printf("button down.\n");
#endif
	state_dispatch(IMG_BUT, (caddr_t) event);
    }
    if (event_is_up(event)) {
#ifdef DEBUG
	printf("button released.\n");
#endif
	state_dispatch(IMG_BUT, (caddr_t) event);
    }
    return notify_next_event_func(win, (Notify_event) event, arg, type);
}

/*
 * Event callback function for `imgwin'.
 */
Notify_value
main_control_imgwin_event_callback(win, event, arg, type)
    Xv_window win;
    Event    *event;
    Notify_arg arg;
    Notify_event_type type;
{

#ifdef DEBUG
    fprintf(stderr, "genial: genial_imgwin_event_callback: event %d\n",
	    event_id(event));
#endif
    /* gxv_start_connections DO NOT EDIT THIS SECTION */

    if (event_action(event) == WIN_RESIZE) {
	imgwin_resize_proc(win, event, arg, type);
    }
    /* gxv_end_connections */

    return notify_next_event_func(win, (Notify_event) event, arg, type);
}

/*
 * Notify callback function for `clear_all'.
 */
void
clear_all_proc(item, event)
    Panel_item item;
    Event    *event;
{
    struct logent *log;

#ifdef DEBUG
    fputs("genial: clear_all_proc\n", stderr);
#endif

    state_dispatch(CLEAR, NULL);/* clear current state first */

    clear_log();
    fxn_reset();
    fxn_init();
    new_state(IMG_LOADED);

    xv_set(base_win->infomesg, PANEL_LABEL_STRING, "  ", NULL);
    panel_paint(base_win->infomesg, PANEL_CLEAR);

    imgwin_repaint_proc(img_win->d_canv, img_win->d_paintwin, display,
			img_win->d_xid, NULL);
}

/*************************************************************/

void
set_clean_mode(item, value, event)
    Panel_item item;
    int       value;
    Event    *event;
{
#ifdef DEBUG
    printf(" clean mode set to: %d \n", clean_mode);
#endif
    clean_mode = value;
}
