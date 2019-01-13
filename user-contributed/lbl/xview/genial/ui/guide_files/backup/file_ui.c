/*
 * file_ui.c - User interface object initialization functions.
 * This file was generated by `gxv' from `file.G'.
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
#include "file_ui.h"

/*
 * Initialize an instance of object `window1'.
 */
file_window1_objects *
file_window1_objects_initialize(ip, owner)
	file_window1_objects	*ip;
	Xv_opaque	owner;
{
	if (!ip && !(ip = (file_window1_objects *) calloc(1, sizeof (file_window1_objects))))
		return (file_window1_objects *) NULL;
	if (!ip->window1)
		ip->window1 = file_window1_window1_create(ip, owner);
	if (!ip->controls1)
		ip->controls1 = file_window1_controls1_create(ip, ip->window1);
	if (!ip->message1)
		ip->message1 = file_window1_message1_create(ip, ip->controls1);
	if (!ip->lmode)
		ip->lmode = file_window1_lmode_create(ip, ip->controls1);
	if (!ip->load)
		ip->load = file_window1_load_create(ip, ip->controls1);
	if (!ip->chooser_load)
		ip->chooser_load = file_window1_chooser_load_create(ip, ip->controls1);
	if (!ip->l_fname)
		ip->l_fname = file_window1_l_fname_create(ip, ip->controls1);
	if (!ip->message2)
		ip->message2 = file_window1_message2_create(ip, ip->controls1);
	if (!ip->smode)
		ip->smode = file_window1_smode_create(ip, ip->controls1);
	if (!ip->save)
		ip->save = file_window1_save_create(ip, ip->controls1);
	if (!ip->chooser_save)
		ip->chooser_save = file_window1_chooser_save_create(ip, ip->controls1);
	if (!ip->s_fname)
		ip->s_fname = file_window1_s_fname_create(ip, ip->controls1);
	return ip;
}

/*
 * Create object `window1' in the specified instance.
 */
Xv_opaque
file_window1_window1_create(ip, owner)
	file_window1_objects	*ip;
	Xv_opaque	owner;
{
	Xv_opaque	obj;
	
	obj = xv_create(owner, FRAME,
		XV_KEY_DATA, INSTANCE, ip,
		XV_WIDTH, 385,
		XV_HEIGHT, 168,
		XV_LABEL, "File I/O",
		XV_SHOW, FALSE,
		FRAME_SHOW_FOOTER, FALSE,
		FRAME_SHOW_RESIZE_CORNER, FALSE,
		NULL);
	return obj;
}

/*
 * Create object `controls1' in the specified instance.
 */
Xv_opaque
file_window1_controls1_create(ip, owner)
	file_window1_objects	*ip;
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
 * Create object `message1' in the specified instance.
 */
Xv_opaque
file_window1_message1_create(ip, owner)
	file_window1_objects	*ip;
	Xv_opaque	owner;
{
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_MESSAGE,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 8,
		XV_Y, 16,
		PANEL_LABEL_STRING, "Load:",
		PANEL_LABEL_BOLD, TRUE,
		NULL);
	return obj;
}

/*
 * Create object `lmode' in the specified instance.
 */
Xv_opaque
file_window1_lmode_create(ip, owner)
	file_window1_objects	*ip;
	Xv_opaque	owner;
{
	extern void		lmode_proc();
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_CHOICE, PANEL_DISPLAY_LEVEL, PANEL_CURRENT,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 28,
		XV_Y, 31,
		PANEL_CHOICE_NROWS, 1,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_LABEL_STRING, "mode:",
		PANEL_NOTIFY_PROC, lmode_proc,
		PANEL_CHOICE_STRINGS,
			"GENIAL Image",
			"GENIAL log file",
			NULL,
		NULL);
	return obj;
}

/*
 * Create object `load' in the specified instance.
 */
Xv_opaque
file_window1_load_create(ip, owner)
	file_window1_objects	*ip;
	Xv_opaque	owner;
{
	extern void		load_proc();
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_BUTTON,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 329,
		XV_Y, 33,
		PANEL_LABEL_STRING, "load",
		PANEL_NOTIFY_PROC, load_proc,
		NULL);
	return obj;
}

/*
 * Create object `chooser_load' in the specified instance.
 */
Xv_opaque
file_window1_chooser_load_create(ip, owner)
	file_window1_objects	*ip;
	Xv_opaque	owner;
{
	extern void		chooser_load_proc();
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_BUTTON,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 280,
		XV_Y, 57,
		PANEL_LABEL_STRING, "File Chooser...",
		PANEL_NOTIFY_PROC, chooser_load_proc,
		NULL);
	return obj;
}

/*
 * Create object `l_fname' in the specified instance.
 */
Xv_opaque
file_window1_l_fname_create(ip, owner)
	file_window1_objects	*ip;
	Xv_opaque	owner;
{
	extern Panel_setting	lname_proc();
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_TEXT,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 28,
		XV_Y, 59,
		PANEL_VALUE_DISPLAY_LENGTH, 20,
		PANEL_VALUE_STORED_LENGTH, 255,
		PANEL_LABEL_STRING, "Filename:",
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_READ_ONLY, FALSE,
		PANEL_NOTIFY_PROC, lname_proc,
		NULL);
	return obj;
}

/*
 * Create object `message2' in the specified instance.
 */
Xv_opaque
file_window1_message2_create(ip, owner)
	file_window1_objects	*ip;
	Xv_opaque	owner;
{
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_MESSAGE,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 8,
		XV_Y, 84,
		PANEL_LABEL_STRING, "Save:",
		PANEL_LABEL_BOLD, TRUE,
		NULL);
	return obj;
}

/*
 * Create object `smode' in the specified instance.
 */
Xv_opaque
file_window1_smode_create(ip, owner)
	file_window1_objects	*ip;
	Xv_opaque	owner;
{
	extern void		smode_proc();
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_CHOICE, PANEL_DISPLAY_LEVEL, PANEL_CURRENT,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 32,
		XV_Y, 97,
		PANEL_CHOICE_NROWS, 1,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_LABEL_STRING, "mode:",
		PANEL_NOTIFY_PROC, smode_proc,
		PANEL_CHOICE_STRINGS,
			"GENIAL Image w/ log",
			"GENIAL Image w/o log",
			"Rectangular GENIAL subimage",
			"GENIAL log file",
			"Sun Rasterfile Image Dump",
			"ASCII trace vector",
			"ASCII histogram vector",
			NULL,
		NULL);
	return obj;
}

/*
 * Create object `save' in the specified instance.
 */
Xv_opaque
file_window1_save_create(ip, owner)
	file_window1_objects	*ip;
	Xv_opaque	owner;
{
	extern void		save_proc();
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_BUTTON,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 328,
		XV_Y, 99,
		PANEL_LABEL_STRING, "save",
		PANEL_NOTIFY_PROC, save_proc,
		NULL);
	return obj;
}

/*
 * Create object `chooser_save' in the specified instance.
 */
Xv_opaque
file_window1_chooser_save_create(ip, owner)
	file_window1_objects	*ip;
	Xv_opaque	owner;
{
	extern void		chooser_save_proc();
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_BUTTON,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 280,
		XV_Y, 125,
		PANEL_LABEL_STRING, "File Chooser...",
		PANEL_NOTIFY_PROC, chooser_save_proc,
		NULL);
	return obj;
}

/*
 * Create object `s_fname' in the specified instance.
 */
Xv_opaque
file_window1_s_fname_create(ip, owner)
	file_window1_objects	*ip;
	Xv_opaque	owner;
{
	extern Panel_setting	sname_proc();
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_TEXT,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 32,
		XV_Y, 127,
		PANEL_VALUE_DISPLAY_LENGTH, 20,
		PANEL_VALUE_STORED_LENGTH, 255,
		PANEL_LABEL_STRING, "Filename:",
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_READ_ONLY, FALSE,
		PANEL_NOTIFY_PROC, sname_proc,
		NULL);
	return obj;
}

/*
 * Initialize an instance of object `rect_choice_win'.
 */
file_rect_choice_win_objects *
file_rect_choice_win_objects_initialize(ip, owner)
	file_rect_choice_win_objects	*ip;
	Xv_opaque	owner;
{
	if (!ip && !(ip = (file_rect_choice_win_objects *) calloc(1, sizeof (file_rect_choice_win_objects))))
		return (file_rect_choice_win_objects *) NULL;
	if (!ip->rect_choice_win)
		ip->rect_choice_win = file_rect_choice_win_rect_choice_win_create(ip, owner);
	if (!ip->controls2)
		ip->controls2 = file_rect_choice_win_controls2_create(ip, ip->rect_choice_win);
	if (!ip->message3)
		ip->message3 = file_rect_choice_win_message3_create(ip, ip->controls2);
	if (!ip->box_lid)
		ip->box_lid = file_rect_choice_win_box_lid_create(ip, ip->controls2);
	if (!ip->box_save)
		ip->box_save = file_rect_choice_win_box_save_create(ip, ip->controls2);
	if (!ip->box_cancel)
		ip->box_cancel = file_rect_choice_win_box_cancel_create(ip, ip->controls2);
	return ip;
}

/*
 * Create object `rect_choice_win' in the specified instance.
 */
Xv_opaque
file_rect_choice_win_rect_choice_win_create(ip, owner)
	file_rect_choice_win_objects	*ip;
	Xv_opaque	owner;
{
	Xv_opaque	obj;
	
	obj = xv_create(owner, FRAME_CMD,
		XV_KEY_DATA, INSTANCE, ip,
		XV_WIDTH, 456,
		XV_HEIGHT, 133,
		XV_LABEL, "Rectangular Subimage Save",
		XV_SHOW, FALSE,
		FRAME_SHOW_FOOTER, FALSE,
		FRAME_SHOW_RESIZE_CORNER, FALSE,
		FRAME_CMD_PUSHPIN_IN, FALSE,
		NULL);
	xv_set(xv_get(obj, FRAME_CMD_PANEL), WIN_SHOW, FALSE, NULL);
	return obj;
}

/*
 * Create object `controls2' in the specified instance.
 */
Xv_opaque
file_rect_choice_win_controls2_create(ip, owner)
	file_rect_choice_win_objects	*ip;
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
 * Create object `message3' in the specified instance.
 */
Xv_opaque
file_rect_choice_win_message3_create(ip, owner)
	file_rect_choice_win_objects	*ip;
	Xv_opaque	owner;
{
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_MESSAGE,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 20,
		XV_Y, 28,
		PANEL_LABEL_STRING, "Please enter the log ID # of a rectangle:",
		PANEL_LABEL_BOLD, TRUE,
		NULL);
	return obj;
}

/*
 * Create object `box_lid' in the specified instance.
 */
Xv_opaque
file_rect_choice_win_box_lid_create(ip, owner)
	file_rect_choice_win_objects	*ip;
	Xv_opaque	owner;
{
	extern Panel_setting	boxid_proc();
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_NUMERIC_TEXT,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 44,
		XV_Y, 52,
		PANEL_VALUE_DISPLAY_LENGTH, 2,
		PANEL_VALUE_STORED_LENGTH, 8,
		PANEL_LABEL_STRING, "Log ID:",
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_MAX_VALUE, 100,
		PANEL_MIN_VALUE, 0,
		PANEL_VALUE, 0,
		PANEL_READ_ONLY, FALSE,
		PANEL_NOTIFY_PROC, boxid_proc,
		NULL);
	return obj;
}

/*
 * Create object `box_save' in the specified instance.
 */
Xv_opaque
file_rect_choice_win_box_save_create(ip, owner)
	file_rect_choice_win_objects	*ip;
	Xv_opaque	owner;
{
	extern void		box_save_proc();
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_BUTTON,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 120,
		XV_Y, 88,
		PANEL_LABEL_STRING, "save",
		PANEL_NOTIFY_PROC, box_save_proc,
		NULL);
	return obj;
}

/*
 * Create object `box_cancel' in the specified instance.
 */
Xv_opaque
file_rect_choice_win_box_cancel_create(ip, owner)
	file_rect_choice_win_objects	*ip;
	Xv_opaque	owner;
{
	extern void		box_cancel_proc();
	Xv_opaque	obj;
	
	obj = xv_create(owner, PANEL_BUTTON,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 216,
		XV_Y, 88,
		PANEL_LABEL_STRING, "cancel",
		PANEL_NOTIFY_PROC, box_cancel_proc,
		NULL);
	return obj;
}
