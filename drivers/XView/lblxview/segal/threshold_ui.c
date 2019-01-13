/*
 * threshold_ui.c - User interface object initialization functions.
 * This file was generated by `gxv' from `threshold.G'.
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
#include "threshold_ui.h"

/*
 * Initialize an instance of object `win'.
 */
threshold_win_objects *
threshold_win_objects_initialize(ip, owner)
	threshold_win_objects	*ip;
	Xv_opaque	owner;
{
	if (!ip && !(ip = (threshold_win_objects *) calloc(1, sizeof (threshold_win_objects))))
		return (threshold_win_objects *) NULL;
	if (!ip->win)
		ip->win = threshold_win_win_create(ip, owner);
	if (!ip->controls2)
		ip->controls2 = threshold_win_controls2_create(ip, ip->win);
	if (!ip->canv_histo)
		ip->canv_histo = threshold_win_canv_histo_create(ip, ip->win);
	if (!ip->controls1)
		ip->controls1 = threshold_win_controls1_create(ip, ip->win);
	if (!ip->controls)
		ip->controls = threshold_win_controls_create(ip, ip->win);
	if (!ip->msg_histogram)
		ip->msg_histogram = threshold_win_msg_histogram_create(ip, ip->controls);
	if (!ip->histo_set)
		ip->histo_set = threshold_win_histo_set_create(ip, ip->controls);
	if (!ip->msg_upper)
		ip->msg_upper = threshold_win_msg_upper_create(ip, ip->controls);
	if (!ip->msg_lower)
		ip->msg_lower = threshold_win_msg_lower_create(ip, ip->controls);
	if (!ip->but_threshold)
		ip->but_threshold = threshold_win_but_threshold_create(ip, ip->controls);
	if (!ip->but_close)
		ip->but_close = threshold_win_but_close_create(ip, ip->controls);
	if (!ip->controls_stats)
		ip->controls_stats = threshold_win_controls_stats_create(ip, ip->win);
	if (!ip->msg_0)
		ip->msg_0 = threshold_win_msg_0_create(ip, ip->controls_stats);
	if (!ip->msg_256)
		ip->msg_256 = threshold_win_msg_256_create(ip, ip->controls_stats);
	if (!ip->threshold_value_upper)
		ip->threshold_value_upper = threshold_win_threshold_value_upper_create(ip, ip->controls_stats);
	if (!ip->threshold_value_lower)
		ip->threshold_value_lower = threshold_win_threshold_value_lower_create(ip, ip->controls_stats);
	return ip;
}

/*
 * Create object `win' in the specified instance.

 */
Xv_opaque
threshold_win_win_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	Xv_opaque	obj;
	
	obj = xv_create(owner, FRAME,
		XV_KEY_DATA, INSTANCE, ip,
		XV_WIDTH, 545,
		XV_HEIGHT, 311,
		XV_LABEL, "Segal : Thresholding",
		FRAME_CLOSED, FALSE,
		FRAME_SHOW_FOOTER, TRUE,
		FRAME_SHOW_RESIZE_CORNER, TRUE,
		NULL);
	return obj;
}

/*
 * Create object `controls2' in the specified instance.

 */
Xv_opaque
threshold_win_controls2_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 0,
		XV_Y, 0,
		XV_WIDTH, 64,
		XV_HEIGHT, 256,
		WIN_BORDER, FALSE,
		NULL);
	return obj;
}

/*
 * Create object `canv_histo' in the specified instance.

 */
Xv_opaque
threshold_win_canv_histo_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	extern void	generate_histogram();
	Xv_opaque	obj;
	
	obj = xv_create(owner, CANVAS,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 61,
		XV_Y, 0,
		XV_WIDTH, 256,
		XV_HEIGHT, 256,
		CANVAS_REPAINT_PROC, generate_histogram,
		NULL);
	xv_set(canvas_paint_window(obj), XV_KEY_DATA, INSTANCE, ip, NULL);
	return obj;
}

/*
 * Create object `controls1' in the specified instance.

 */
Xv_opaque
threshold_win_controls1_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 317,
		XV_Y, 0,
		XV_WIDTH, 15,
		XV_HEIGHT, 256,
		WIN_BORDER, FALSE,
		NULL);
	return obj;
}

/*
 * Create object `controls' in the specified instance.

 */
Xv_opaque
threshold_win_controls_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 332,
		XV_Y, 0,
		XV_WIDTH, WIN_EXTEND_TO_EDGE,
		XV_HEIGHT, WIN_EXTEND_TO_EDGE,
		WIN_BORDER, FALSE,
		NULL);
	return obj;
}

/*
 * Create object `msg_histogram' in the specified instance.

 */
Xv_opaque
threshold_win_msg_histogram_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_MESSAGE,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 8,
		XV_Y, 16,
		XV_WIDTH, 129,
		XV_HEIGHT, 13,
		PANEL_LABEL_STRING, "Histogram Method:",
		PANEL_LABEL_BOLD, TRUE,
		NULL);
	return obj;
}

/*
 * Create object `histo_set' in the specified instance.

 */
Xv_opaque
threshold_win_histo_set_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_CHOICE, PANEL_DISPLAY_LEVEL, PANEL_CURRENT,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 0,
		XV_Y, 32,
		XV_WIDTH, 167,
		XV_HEIGHT, 23,
		PANEL_VALUE_X, 9,
		PANEL_VALUE_Y, 32,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_CHOICE_NROWS, 1,
		PANEL_CHOICE_STRINGS,
			"Normal",
			"Exclude Edge Points",
			"Smooth",
			0,
		NULL);
	return obj;
}

/*
 * Create object `msg_upper' in the specified instance.

 */
Xv_opaque
threshold_win_msg_upper_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_MESSAGE,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 0,
		XV_Y, 268,
		XV_WIDTH, 43,
		XV_HEIGHT, 13,
		PANEL_LABEL_STRING, "Upper",
		PANEL_LABEL_BOLD, TRUE,
		NULL);
	return obj;
}

/*
 * Create object `msg_lower' in the specified instance.

 */
Xv_opaque
threshold_win_msg_lower_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_MESSAGE,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 0,
		XV_Y, 288,
		XV_WIDTH, 42,
		XV_HEIGHT, 13,
		PANEL_LABEL_STRING, "Lower",
		PANEL_LABEL_BOLD, TRUE,
		NULL);
	return obj;
}

/*
 * Create object `but_threshold' in the specified instance.

 */
Xv_opaque
threshold_win_but_threshold_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	extern void		threshold_proc();
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_BUTTON,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 68,
		XV_Y, 288,
		XV_WIDTH, 79,
		XV_HEIGHT, 19,
		PANEL_LABEL_STRING, "Threshold",
		PANEL_NOTIFY_PROC, threshold_proc,
		NULL);
	return obj;
}

/*
 * Create object `but_close' in the specified instance.

 */
Xv_opaque
threshold_win_but_close_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	extern void		map_threshold();
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_BUTTON,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 156,
		XV_Y, 288,
		XV_WIDTH, 51,
		XV_HEIGHT, 19,
		PANEL_LABEL_STRING, "Close",
		PANEL_NOTIFY_PROC, map_threshold,
		NULL);
	return obj;
}

/*
 * Create object `controls_stats' in the specified instance.

 */
Xv_opaque
threshold_win_controls_stats_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 0,
		XV_Y, 256,
		XV_WIDTH, 332,
		XV_HEIGHT, WIN_EXTEND_TO_EDGE,
		WIN_BORDER, FALSE,
		NULL);
	return obj;
}

/*
 * Create object `msg_0' in the specified instance.

 */
Xv_opaque
threshold_win_msg_0_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_MESSAGE,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 60,
		XV_Y, 0,
		XV_WIDTH, 8,
		XV_HEIGHT, 13,
		PANEL_LABEL_STRING, "0",
		PANEL_LABEL_BOLD, FALSE,
		NULL);
	return obj;
}

/*
 * Create object `msg_256' in the specified instance.

 */
Xv_opaque
threshold_win_msg_256_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_MESSAGE,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 304,
		XV_Y, 0,
		XV_WIDTH, 24,
		XV_HEIGHT, 13,
		PANEL_LABEL_STRING, "256",
		PANEL_LABEL_BOLD, FALSE,
		NULL);
	return obj;
}

/*
 * Create object `threshold_value_upper' in the specified instance.

 */
Xv_opaque
threshold_win_threshold_value_upper_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	extern int		set_thresh_upper();
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_SLIDER,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 4,
		XV_Y, 12,
		XV_WIDTH, 321,
		XV_HEIGHT, 15,
		PANEL_VALUE_X, 10,
		PANEL_VALUE_Y, 12,
		PANEL_SLIDER_WIDTH, 255,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_DIRECTION, PANEL_HORIZONTAL,
		PANEL_SLIDER_END_BOXES, FALSE,
		PANEL_SHOW_RANGE, FALSE,
		PANEL_SHOW_VALUE, TRUE,
		PANEL_MIN_VALUE, 0,
		PANEL_MAX_VALUE, 255,
		PANEL_TICKS, 0,
		PANEL_NOTIFY_PROC, set_thresh_upper,
		NULL);
	return obj;
}

/*
 * Create object `threshold_value_lower' in the specified instance.

 */
Xv_opaque
threshold_win_threshold_value_lower_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	extern int		set_thresh_lower();
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_SLIDER,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 4,
		XV_Y, 32,
		XV_WIDTH, 321,
		XV_HEIGHT, 15,
		PANEL_VALUE_X, 13,
		PANEL_VALUE_Y, 32,
		PANEL_SLIDER_WIDTH, 255,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_DIRECTION, PANEL_HORIZONTAL,
		PANEL_SLIDER_END_BOXES, FALSE,
		PANEL_SHOW_RANGE, FALSE,
		PANEL_SHOW_VALUE, TRUE,
		PANEL_MIN_VALUE, 0,
		PANEL_MAX_VALUE, 255,
		PANEL_TICKS, 0,
		PANEL_NOTIFY_PROC, set_thresh_lower,
		NULL);
	return obj;
}

