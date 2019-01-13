/*
 * filter_ui.c - User interface object initialization functions.
 * This file was generated by `gxv' from `filter.G'.
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
#include "filter_ui.h"

/*
 * Initialize an instance of object `win'.
 */
filter_win_objects *
filter_win_objects_initialize(ip, owner)
	filter_win_objects	*ip;
	Xv_opaque	owner;
{
	if (!ip && !(ip = (filter_win_objects *) calloc(1, sizeof (filter_win_objects))))
		return (filter_win_objects *) NULL;
	if (!ip->win)
		ip->win = filter_win_win_create(ip, owner);
	if (!ip->controls)
		ip->controls = filter_win_controls_create(ip, ip->win);
	if (!ip->filter_name)
		ip->filter_name = filter_win_filter_name_create(ip, ip->controls);
	if (!ip->filter_parameters)
		ip->filter_parameters = filter_win_filter_parameters_create(ip, ip->controls);
	if (!ip->set_source_buf)
		ip->set_source_buf = filter_win_set_source_buf_create(ip, ip->controls);
	if (!ip->set_dest_buf)
		ip->set_dest_buf = filter_win_set_dest_buf_create(ip, ip->controls);
	if (!ip->but_apply)
		ip->but_apply = filter_win_but_apply_create(ip, ip->controls);
	if (!ip->but_close)
		ip->but_close = filter_win_but_close_create(ip, ip->controls);
	return ip;
}

/*
 * Create object `win' in the specified instance.

 */
Xv_opaque
filter_win_win_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	Xv_opaque	obj;
	
	obj = xv_create(owner, FRAME,
		XV_KEY_DATA, INSTANCE, ip,
		XV_WIDTH, 349,
		XV_HEIGHT, 174,
		XV_LABEL, "Segal: Filter Apply",
		FRAME_CLOSED, FALSE,
		FRAME_SHOW_FOOTER, FALSE,
		FRAME_SHOW_RESIZE_CORNER, TRUE,
		NULL);
	return obj;
}

/*
 * Create object `controls' in the specified instance.

 */
Xv_opaque
filter_win_controls_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 0,
		XV_Y, 0,
		XV_WIDTH, WIN_EXTEND_TO_EDGE,
		XV_HEIGHT, WIN_EXTEND_TO_EDGE,
		WIN_BORDER, FALSE,
		NULL);
	return obj;
}

/*
 * Create object `filter_name' in the specified instance.

 */
Xv_opaque
filter_win_filter_name_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_TEXT,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 12,
		XV_Y, 24,
		XV_WIDTH, 330,
		XV_HEIGHT, 15,
		PANEL_LABEL_STRING, "Filter Name:",
		PANEL_VALUE_X, 102,
		PANEL_VALUE_Y, 24,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_VALUE_DISPLAY_LENGTH, 30,
		PANEL_VALUE_STORED_LENGTH, 100,
		PANEL_READ_ONLY, FALSE,
		NULL);
	return obj;
}

/*
 * Create object `filter_parameters' in the specified instance.

 */
Xv_opaque
filter_win_filter_parameters_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_TEXT,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 12,
		XV_Y, 48,
		XV_WIDTH, 153,
		XV_HEIGHT, 15,
		PANEL_LABEL_STRING, "Parameters:",
		PANEL_VALUE_X, 101,
		PANEL_VALUE_Y, 48,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_VALUE_DISPLAY_LENGTH, 30,
		PANEL_VALUE_STORED_LENGTH, 100,
		PANEL_READ_ONLY, FALSE,
		NULL);
	return obj;
}

/*
 * Create object `set_source_buf' in the specified instance.

 */
Xv_opaque
filter_win_set_source_buf_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_CHOICE,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 52,
		XV_Y, 68,
		XV_WIDTH, 200,
		XV_HEIGHT, 23,
		PANEL_VALUE_X, 124,
		PANEL_VALUE_Y, 68,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_CHOICE_NROWS, 1,
		PANEL_LABEL_STRING, "Apply To:",
		PANEL_CHOICE_STRINGS,
			"Image",
			"Edit Mask",
			0,
		NULL);
	return obj;
}

/*
 * Create object `set_dest_buf' in the specified instance.

 */
Xv_opaque
filter_win_set_dest_buf_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_CHOICE,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 12,
		XV_Y, 100,
		XV_WIDTH, 240,
		XV_HEIGHT, 23,
		PANEL_VALUE_X, 124,
		PANEL_VALUE_Y, 100,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_CHOICE_NROWS, 1,
		PANEL_LABEL_STRING, "Store Result In:",
		PANEL_CHOICE_STRINGS,
			"Image",
			"Edit Mask",
			0,
		NULL);
	return obj;
}

/*
 * Create object `but_apply' in the specified instance.

 */
Xv_opaque
filter_win_but_apply_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	extern void		but_apply_filter_proc();
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_BUTTON,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 16,
		XV_Y, 132,
		XV_WIDTH, 53,
		XV_HEIGHT, 19,
		PANEL_LABEL_STRING, "Apply",
		PANEL_NOTIFY_PROC, but_apply_filter_proc,
		NULL);
	return obj;
}

/*
 * Create object `but_close' in the specified instance.

 */
Xv_opaque
filter_win_but_close_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	extern void		map_filter();
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_BUTTON,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 152,
		XV_Y, 148,
		XV_WIDTH, 51,
		XV_HEIGHT, 19,
		PANEL_LABEL_STRING, "Close",
		PANEL_NOTIFY_PROC, map_filter,
		NULL);
	return obj;
}
