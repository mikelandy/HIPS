/*
 *	display_control.c
 *
 *	Bryan Skene, LBL July 1991
 *	part of 'segal'
 */

#include "common.h"

#include "mask_control.h"
#include "pixedit.h"

#include "display_control_ui.h"

/* Allocation :) */
display_control_win_objects *display_control_win;

/*****************************************************/
void
display_control_win_init(owner)
Xv_opaque owner;
{

	display_control_win = display_control_win_objects_initialize(NULL, owner);

}

/*****************************************************/
void
map_display_control(item, event)
Panel_item      item;
Event           *event;
{
	fputs("segal: map_display_control\n", stderr);
        /* Map / Unmap toggle */
	if (xv_get(display_control_win->win, XV_SHOW, NULL) == FALSE) {
		xv_set(display_control_win->win,
			XV_SHOW, TRUE,
			NULL);
	}
	else xv_set(display_control_win->win,
			XV_SHOW, FALSE,
			NULL);
}

/**************************************************************/
void
change_image_proc(item, value, event)
Panel_item item;
int value;
Event *event;
{
/* selects which image to display */
	segal.display_type = value;
	(void) xv_set(display_control_win->display_type,
		PANEL_VALUE, value,
		NULL);
	(void) xv_set(mask_control_win->display_type,
		PANEL_VALUE, value,
		NULL);

	image_repaint_proc();
	orig_image_repaint_proc();
	edit_repaint_proc();
}

/***********************************************************/
void
blend_type_proc(item, value, event)	/* procedure for 'blend type' setting */
    Panel_item item;
    int       value;
    Event    *event;
{
    static int old_val = -1;
    if (verbose)
	fprintf(stderr, "segal: blend_type_proc: value: %u\n", value);

    if (value == old_val)
	return;

    set_watch_cursor();
    segal.blend_type = value;

    if (himage.fp != NULL) {
	blend(himage.data[0], work_buf[0], (u_char *) blend_image->data,
	      segal.rows * segal.cols);
    }
    image_repaint_proc();
    orig_image_repaint_proc();

    if ((int) xv_get(edit_win->win, XV_SHOW, NULL) == TRUE) {
	zoom();
	edit_repaint_proc();
    }
    old_val = value;
    unset_watch_cursor();
}

/***********************************************************/
void
mask_type_proc(item, value, event)	/* procedure for 'mask type' setting */
    Panel_item item;
    int       value;
    Event    *event;
{
    static int old_val = -1;
    if (verbose)
	fprintf(stderr, "segal: mask_type_proc: value: %u\n", value);

    if (value == old_val)
	return;

    set_watch_cursor();
    segal.mask_type = value;

    if (himage.fp != NULL) {
	blend(himage.data[0], work_buf[0], (u_char *) blend_image->data,
	      segal.rows * segal.cols);
    }
    image_repaint_proc();
    orig_image_repaint_proc();

    if ((int) xv_get(edit_win->win, XV_SHOW, NULL) == TRUE) {
	zoom();
	edit_repaint_proc();
    }
    old_val = value;

    if (value == 1)
	xv_set(edit_win->mask_brush_mode, PANEL_CHOICE_STRINGS,
	       "paint", "erase", 0, NULL);
    else
	xv_set(edit_win->mask_brush_mode, PANEL_CHOICE_STRINGS,
	       " cut ", "fill ", 0, NULL);

    panel_paint(edit_win->mask_brush_mode, PANEL_CLEAR);
    unset_watch_cursor();
}

/**************************************************************/
void
mask_control_pop_polygon(item, event)
Panel_item      item;
Event           *event;
{
	fputs("segal: Mask_control_pop_polygon\n", stderr);
}
