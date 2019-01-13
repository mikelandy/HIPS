/*
 * pixedit_ui.c - User interface object initialization functions.
 * This file was generated by `gxv' from `pixedit.G'.
 * DO NOT EDIT BY HAND.
 */

#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <xview/xview.h>
#include <xview/canvas.h>
#include <xview/icon_load.h>
#include <xview/panel.h>
#include <xview/scrollbar.h>
#include <xview/svrimage.h>
#include <xview/termsw.h>
#include <xview/text.h>
#include <xview/tty.h>
#include <xview/xv_xrect.h>
#include "pixedit_ui.h"

/*
 * Initialize an instance of object `win'.
 */
pixedit_win_objects *
pixedit_win_objects_initialize(ip, owner)
	pixedit_win_objects	*ip;
	Xv_opaque	owner;
{
	if (!ip && !(ip = (pixedit_win_objects *) calloc(1, sizeof (pixedit_win_objects))))
		return (pixedit_win_objects *) NULL;
	if (!ip->win)
		ip->win = pixedit_win_win_create(ip, owner);
	if (!ip->control)
		ip->control = pixedit_win_control_create(ip, ip->win);
	if (!ip->brush_mode)
		ip->brush_mode = pixedit_win_brush_mode_create(ip, ip->control);
	if (!ip->brush_type)
		ip->brush_type = pixedit_win_brush_type_create(ip, ip->control);
	if (!ip->mask_brush_mode)
		ip->mask_brush_mode = pixedit_win_mask_brush_mode_create(ip, ip->control);
	if (!ip->brush_size)
		ip->brush_size = pixedit_win_brush_size_create(ip, ip->control);
	if (!ip->zoom_mag)
		ip->zoom_mag = pixedit_win_zoom_mag_create(ip, ip->control);
	if (!ip->image_brush_mode)
		ip->image_brush_mode = pixedit_win_image_brush_mode_create(ip, ip->control);
	if (!ip->image_brush_delta)
		ip->image_brush_delta = pixedit_win_image_brush_delta_create(ip, ip->control);
	if (!ip->but_original)
		ip->but_original = pixedit_win_but_original_create(ip, ip->control);
	if (!ip->but_undo)
		ip->but_undo = pixedit_win_but_undo_create(ip, ip->control);
	if (!ip->but_save)
		ip->but_save = pixedit_win_but_save_create(ip, ip->control);
	if (!ip->but_close)
		ip->but_close = pixedit_win_but_close_create(ip, ip->control);
	if (!ip->msg_pixel_value)
		ip->msg_pixel_value = pixedit_win_msg_pixel_value_create(ip, ip->control);
	if (!ip->canvas)
		ip->canvas = pixedit_win_canvas_create(ip, ip->win);
	return ip;
}

/*
 * Create object `win' in the specified instance.

 */
Xv_opaque
pixedit_win_win_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	Xv_opaque	obj;
	
	obj = xv_create(owner, FRAME,
		XV_KEY_DATA, INSTANCE, ip,
		XV_WIDTH, 479,
		XV_HEIGHT, 201,
		XV_LABEL, "Edit Window",
		FRAME_CLOSED, FALSE,
		FRAME_SHOW_FOOTER, FALSE,
		FRAME_SHOW_RESIZE_CORNER, TRUE,
		NULL);
	return obj;
}

/*
 * Create object `control' in the specified instance.

 */
Xv_opaque
pixedit_win_control_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 0,
		XV_Y, 0,
		XV_WIDTH, WIN_EXTEND_TO_EDGE,
		XV_HEIGHT, 132,
		WIN_BORDER, TRUE,
		NULL);
	return obj;
}

/*
 * Create object `brush_mode' in the specified instance.

 */
Xv_opaque
pixedit_win_brush_mode_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	extern int		set_brush_mode();
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_CHOICE,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 12,
		XV_Y, 4,
		XV_WIDTH, 192,
		XV_HEIGHT, 23,
		PANEL_VALUE_X, 104,
		PANEL_VALUE_Y, 4,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_CHOICE_NROWS, 1,
		PANEL_LABEL_STRING, "Brush Mode:",
		PANEL_NOTIFY_PROC, set_brush_mode,
		PANEL_CHOICE_STRINGS,
			"Mask",
			"Image",
			0,
		NULL);
	return obj;
}

/*
 * Create object `brush_type' in the specified instance.

 */
Xv_opaque
pixedit_win_brush_type_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	extern int		change_cursor_proc();
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_CHOICE,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 263,
		XV_Y, 4,
		XV_WIDTH, 210,
		XV_HEIGHT, 23,
		PANEL_VALUE_X, 362,
		PANEL_VALUE_Y, 4,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_CHOICE_NROWS, 1,
		PANEL_LABEL_STRING, "Brush Shape:",
		PANEL_NOTIFY_PROC, change_cursor_proc,
		PANEL_CHOICE_STRINGS,
			"Square",
			"Round",
			0,
		NULL);
	return obj;
}

/*
 * Create object `mask_brush_mode' in the specified instance.

 */
Xv_opaque
pixedit_win_mask_brush_mode_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	extern int		change_cursor_proc();
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_CHOICE,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 12,
		XV_Y, 32,
		XV_WIDTH, 192,
		XV_HEIGHT, 23,
		PANEL_VALUE_X, 105,
		PANEL_VALUE_Y, 32,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_CHOICE_NROWS, 1,
		PANEL_LABEL_STRING, "Mask Brush:",
		PANEL_NOTIFY_PROC, change_cursor_proc,
		PANEL_CHOICE_STRINGS,
			"Paint ",
			"Erase",
			0,
		NULL);
	return obj;
}

/*
 * Create object `brush_size' in the specified instance.

 */
Xv_opaque
pixedit_win_brush_size_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	extern int		change_cursor_proc();
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_CHOICE, PANEL_DISPLAY_LEVEL, PANEL_CURRENT,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 276,
		XV_Y, 32,
		XV_WIDTH, 159,
		XV_HEIGHT, 23,
		PANEL_VALUE_X, 361,
		PANEL_VALUE_Y, 32,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_CHOICE_NROWS, 1,
		PANEL_LABEL_STRING, "Brush Size:",
		PANEL_NOTIFY_PROC, change_cursor_proc,
		PANEL_CHOICE_STRINGS,
			"1x1",
			"2x2",
			"3x3",
			"4x4",
			"5x5",
			"6x6",
			"20x20",
			0,
		NULL);
	return obj;
}

/*
 * Create object `zoom_mag' in the specified instance.

 */
Xv_opaque
pixedit_win_zoom_mag_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	extern int		mag_proc();
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_CHOICE, PANEL_DISPLAY_LEVEL, PANEL_CURRENT,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 280,
		XV_Y, 56,
		XV_WIDTH, 135,
		XV_HEIGHT, 23,
		PANEL_VALUE_X, 361,
		PANEL_VALUE_Y, 56,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_CHOICE_NROWS, 1,
		PANEL_LABEL_STRING, "Zoom Mag:",
		PANEL_NOTIFY_PROC, mag_proc,
		PANEL_CHOICE_STRINGS,
			"x 1",
			"x 2",
			"x 3",
			"x 4",
			"x 5",
			"x 6",
			0,
		NULL);
	return obj;
}

/*
 * Create object `image_brush_mode' in the specified instance.

 */
Xv_opaque
pixedit_win_image_brush_mode_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_CHOICE,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 5,
		XV_Y, 60,
		XV_WIDTH, 212,
		XV_HEIGHT, 23,
		PANEL_VALUE_X, 103,
		PANEL_VALUE_Y, 60,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_CHOICE_NROWS, 1,
		PANEL_LABEL_STRING, "Image Brush:",
		PANEL_CHOICE_STRINGS,
			"Darker",
			"Lighter",
			0,
		NULL);
	return obj;
}

/*
 * Create object `image_brush_delta' in the specified instance.

 */
Xv_opaque
pixedit_win_image_brush_delta_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_SLIDER,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 38,
		XV_Y, 88,
		XV_WIDTH, 222,
		XV_HEIGHT, 20,
		PANEL_VALUE_X, 102,
		PANEL_VALUE_Y, 88,
		PANEL_SLIDER_WIDTH, 100,
		PANEL_LABEL_STRING, "Amount:",
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_DIRECTION, PANEL_HORIZONTAL,
		PANEL_SLIDER_END_BOXES, FALSE,
		PANEL_SHOW_RANGE, FALSE,
		PANEL_SHOW_VALUE, TRUE,
		PANEL_MIN_VALUE, 0,
		PANEL_MAX_VALUE, 100,
		PANEL_TICKS, 0,
		NULL);
	return obj;
}

/*
 * Create object `but_original' in the specified instance.

 */
Xv_opaque
pixedit_win_but_original_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	extern void		original_proc();
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_BUTTON,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 220,
		XV_Y, 104,
		XV_WIDTH, 67,
		XV_HEIGHT, 19,
		PANEL_LABEL_STRING, "Original",
		PANEL_NOTIFY_PROC, original_proc,
		NULL);
	return obj;
}

/*
 * Create object `but_undo' in the specified instance.

 */
Xv_opaque
pixedit_win_but_undo_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	extern void		undo_proc();
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_BUTTON,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 300,
		XV_Y, 104,
		XV_WIDTH, 50,
		XV_HEIGHT, 19,
		PANEL_LABEL_STRING, "Undo",
		PANEL_NOTIFY_PROC, undo_proc,
		NULL);
	return obj;
}

/*
 * Create object `but_save' in the specified instance.

 */
Xv_opaque
pixedit_win_but_save_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	extern void		menu_save_frame_proc();
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_BUTTON,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 364,
		XV_Y, 104,
		XV_WIDTH, 46,
		XV_HEIGHT, 19,
		PANEL_LABEL_STRING, "Save",
		PANEL_NOTIFY_PROC, menu_save_frame_proc,
		NULL);
	return obj;
}

/*
 * Create object `but_close' in the specified instance.

 */
Xv_opaque
pixedit_win_but_close_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	extern void		map_edit_win();
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_BUTTON,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 424,
		XV_Y, 104,
		XV_WIDTH, 51,
		XV_HEIGHT, 19,
		PANEL_LABEL_STRING, "Close",
		PANEL_NOTIFY_PROC, map_edit_win,
		NULL);
	return obj;
}

/*
 * Create object `msg_pixel_value' in the specified instance.

 */
Xv_opaque
pixedit_win_msg_pixel_value_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_MESSAGE,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 14,
		XV_Y, 110,
		XV_WIDTH, 79,
		XV_HEIGHT, 13,
		PANEL_LABEL_STRING, "Pixel Value:",
		PANEL_LABEL_BOLD, TRUE,
		NULL);
	return obj;
}

/*
 * Create object `canvas' in the specified instance.

 */
Xv_opaque
pixedit_win_canvas_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	extern Notify_value	edit_event_proc();
	extern void	edit_repaint_proc();
	Xv_opaque	obj;
	
	obj = xv_create(owner, CANVAS,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 0,
		XV_Y, 132,
		XV_WIDTH, 72,
		XV_HEIGHT, WIN_EXTEND_TO_EDGE,
		CANVAS_REPAINT_PROC, edit_repaint_proc,
		CANVAS_X_PAINT_WINDOW, TRUE,
		NULL);
	xv_set(canvas_paint_window(obj), WIN_CONSUME_EVENTS,
		WIN_MOUSE_BUTTONS,
		LOC_WINENTER,
		LOC_WINEXIT,
		LOC_DRAG,
		LOC_MOVE,
		NULL, NULL);
	notify_interpose_event_func(canvas_paint_window(obj),
		(Notify_func) edit_event_proc, NOTIFY_SAFE);
	xv_set(canvas_paint_window(obj), XV_KEY_DATA, INSTANCE, ip, NULL);
	return obj;
}

