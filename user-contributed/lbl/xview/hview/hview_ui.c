/*
 * hview_ui.c - User interface object initialization functions.
 * This file was generated by `gxv' from `hview.G'.
 * DO NOT EDIT BY HAND.
 */

#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <xview/xview.h>
#include <xview/canvas.h>
#include <xview/panel.h>
#include <xview/scrollbar.h>
#include <xview/svrimage.h>
#include <xview/termsw.h>
#include <xview/text.h>
#include <xview/tty.h>
#include <xview/xv_xrect.h>
#include "hview_ui.h"

/*
 * Initialize an instance of object `win'.
 */
hview_win_objects *
hview_win_objects_initialize(ip, owner)
	hview_win_objects	*ip;
	Xv_opaque	owner;
{
	if (!ip && !(ip = (hview_win_objects *) calloc(1, sizeof (hview_win_objects))))
		return (hview_win_objects *) NULL;
	if (!ip->win)
		ip->win = hview_win_win_create(ip, owner);
	if (!ip->controls1)
		ip->controls1 = hview_win_controls1_create(ip, ip->win);
	if (!ip->button2)
		ip->button2 = hview_win_button2_create(ip, ip->controls1);
	if (!ip->slider1)
		ip->slider1 = hview_win_slider1_create(ip, ip->controls1);
	if (!ip->button1)
		ip->button1 = hview_win_button1_create(ip, ip->controls1);
	if (!ip->can)
		ip->can = hview_win_can_create(ip, ip->win);
	return ip;
}

/*
 * Create object `win' in the specified instance.
 */
Xv_opaque
hview_win_win_create(ip, owner)
	hview_win_objects	*ip;
	Xv_opaque	owner;
{
	extern Notify_value	hview_win_event_callback();
	Xv_opaque	obj;
	
	obj = xv_create(owner, FRAME,
		XV_KEY_DATA, INSTANCE, ip,
		XV_WIDTH, 356,
		XV_HEIGHT, 327,
		XV_LABEL, "HIPS Image Display",
		FRAME_SHOW_FOOTER, FALSE,
		FRAME_SHOW_RESIZE_CORNER, TRUE,
		NULL);
	xv_set(obj, WIN_CONSUME_EVENTS,
		NULL, NULL);
	notify_interpose_event_func(obj,
		(Notify_func) hview_win_event_callback, NOTIFY_SAFE);
	return obj;
}

/*
 * Create object `controls1' in the specified instance.
 */
Xv_opaque
hview_win_controls1_create(ip, owner)
	hview_win_objects	*ip;
	Xv_opaque	owner;
{
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 0,
		XV_Y, 0,
		XV_WIDTH, WIN_EXTEND_TO_EDGE,
		XV_HEIGHT, 45,
		WIN_BORDER, FALSE,
		NULL);
	return obj;
}

/*
 * Create object `button2' in the specified instance.
 */
Xv_opaque
hview_win_button2_create(ip, owner)
	hview_win_objects	*ip;
	Xv_opaque	owner;
{
	extern void		make_perfect_colors();
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_BUTTON,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 8,
		XV_Y, 8,
		PANEL_LABEL_STRING, "Perfect",
		PANEL_NOTIFY_PROC, make_perfect_colors,
		NULL);
	return obj;
}

/*
 * Create object `slider1' in the specified instance.
 */
Xv_opaque
hview_win_slider1_create(ip, owner)
	hview_win_objects	*ip;
	Xv_opaque	owner;
{
	extern void		hview_win_slider1_notify_callback();
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_SLIDER,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 88,
		XV_Y, 8,
		PANEL_SLIDER_WIDTH, 100,
		PANEL_TICKS, 0,
		PANEL_LABEL_STRING, "Contrast:",
		PANEL_DIRECTION, PANEL_HORIZONTAL,
		PANEL_SLIDER_END_BOXES, FALSE,
		PANEL_SHOW_RANGE, FALSE,
		PANEL_SHOW_VALUE, FALSE,
		PANEL_MIN_VALUE, 2,
		PANEL_MAX_VALUE, 52,
		PANEL_VALUE, 0,
		PANEL_NOTIFY_PROC, hview_win_slider1_notify_callback,
		NULL);
	return obj;
}

/*
 * Create object `button1' in the specified instance.
 */
Xv_opaque
hview_win_button1_create(ip, owner)
	hview_win_objects	*ip;
	Xv_opaque	owner;
{
	extern void		quit_proc();
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_BUTTON,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 280,
		XV_Y, 8,
		PANEL_LABEL_STRING, "Quit",
		PANEL_NOTIFY_PROC, quit_proc,
		NULL);
	return obj;
}

/*
 * Create object `can' in the specified instance.
 */
Xv_opaque
hview_win_can_create(ip, owner)
	hview_win_objects	*ip;
	Xv_opaque	owner;
{
	extern void	canvas_repaint_proc();
	Xv_opaque	obj;
	
	obj = xv_create(owner, CANVAS,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 0,
		XV_Y, 42,
		XV_WIDTH, WIN_EXTEND_TO_EDGE,
		XV_HEIGHT, WIN_EXTEND_TO_EDGE,
		CANVAS_REPAINT_PROC, canvas_repaint_proc,
		CANVAS_X_PAINT_WINDOW, TRUE,
		NULL);
	xv_create(obj, SCROLLBAR, SCROLLBAR_DIRECTION, SCROLLBAR_HORIZONTAL, NULL);
	xv_create(obj, SCROLLBAR, SCROLLBAR_DIRECTION, SCROLLBAR_VERTICAL, NULL);
	/*
	 * This line is here for backwards compatibility. It will be
	 * removed for the next release.
	 */
	xv_set(canvas_paint_window(obj), XV_KEY_DATA, INSTANCE, ip, NULL);
	return obj;
}

