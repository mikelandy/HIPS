/*
 *	@(#)xv_file.c	2.7	11/29/91
 *
 * XView preview for HIPS images V1.0
 *
 * Author: 	Cathy Waite
 * 		(c) The Turing Institute
 *
 * Fri Aug  9 17:06:10 BST 1991
 *
 *  xv_file.c : Contains defintions for command popups for loading and 
 *              saving files.
 */
#include "xv_frame.h"
#include <xview/panel.h>

#include "appl.h"
#include "xv_file.h"

h_boolean	save_frame = FALSE;
/************************************************************************
 *
 *		Static XView Items .....
 *
 ************************************************************************/
/* Command popup used to save an image */
Frame		save_command_frame;
Panel_item	save_file_item;

/* Command popup used to load an image */
Frame		load_command_frame;
Panel_item	load_file_item;


Panel_item	frames_item, first_frame_item, last_frame_item;
/***********************************************************************
 *
 *		NOTIFICATION PROCEDURES
 *
 ***********************************************************************/

/* Hide the load_command_frame (if push-pin not in), and call routine
 * to load file.
 */
void	do_load_notify()
{
	/* How do I get it to stay up when the file has not been
	   loaded properly ?.  It is alright when the user has entered
	   Return, but when they use the Load button I seem to have no
	   control over the popup disappearing.
	*/
	char	new_filename[TEXT_LENGTH];

	/* Stop animation */
	stop_notify();

	strcpy(new_filename,(char *)xv_get(load_file_item, PANEL_VALUE));
	xv_set(load_command_frame, FRAME_BUSY, TRUE, NULL);
	xv_set(frame, FRAME_BUSY, TRUE, NULL);

	select_frames = ((h_boolean)xv_get(frames_item, PANEL_VALUE) == 1);

	if (select_frames == TRUE)
	{
		first_frame = (int)xv_get(first_frame_item, PANEL_VALUE);
		last_frame = (int)xv_get(last_frame_item, PANEL_VALUE);
	}
	/* BF: 5, added boolean "startup" parameter */
	if (load_file(new_filename, FALSE) == TRUE)
		xv_set(load_command_frame, XV_SHOW, FALSE, NULL);

	xv_set(load_command_frame, FRAME_BUSY, FALSE, NULL);
	xv_set(frame, FRAME_BUSY, FALSE, NULL);
}

void	hide_load_notify()
{
	xv_set(load_command_frame, FRAME_CMD_PUSHPIN_IN, FALSE, NULL);
}

void	do_save_notify()
{
	char	new_filename[TEXT_LENGTH];

	strcpy(new_filename,(char *)xv_get(save_file_item, PANEL_VALUE));
	xv_set(save_command_frame, FRAME_BUSY, TRUE, NULL);
	if (save_file_as(new_filename, save_frame) == TRUE)
		xv_set(save_command_frame, XV_SHOW, FALSE, NULL);
	
	xv_set(save_command_frame, FRAME_BUSY, FALSE, NULL);
}

void	hide_save_notify()
{
	xv_set(save_command_frame, FRAME_CMD_PUSHPIN_IN, FALSE, NULL);
}

void	frames_notify(item, value, event)
Panel_item	item;
int		value;
Event		*event;
{
	if (value == 0)
	{
		/* All .... */
		xv_set(first_frame_item, PANEL_INACTIVE, TRUE, NULL);
		xv_set(last_frame_item, PANEL_INACTIVE, TRUE, NULL);
	}
	else
	{
		xv_set(first_frame_item, PANEL_INACTIVE, FALSE, NULL);
		xv_set(last_frame_item, PANEL_INACTIVE, FALSE, NULL);
	}

}

void	set_frames(first, last)
int	first, last;
{
	xv_set(first_frame_item, PANEL_VALUE, first, NULL);
	xv_set(last_frame_item, PANEL_VALUE, last, NULL);
}
/************************************************************************
 *
 *		Creation of Frame, Control Panel and Panel_items
 *
 ***********************************************************************/

/*
 * Create the command frame popup for loading images 
 */
void	create_load_command_frame()
{
	Panel		panel;
	Panel_item	load_button, hide_button;
	int		load_x;
	int		load_width;

	load_command_frame = xv_create(frame, FRAME_CMD,
		FRAME_LABEL,	"Load Image",
		NULL);

	panel = (Panel)xv_get(load_command_frame, FRAME_CMD_PANEL);

	load_file_item = (Panel_item) xv_create(panel, PANEL_TEXT,
		PANEL_LABEL_STRING, 		"File:",
		PANEL_VALUE,		 	filename,
		PANEL_VALUE_STORED_LENGTH,	TEXT_LENGTH,
		PANEL_NOTIFY_PROC,		do_load_notify,
		NULL);
        
	frames_item = (Panel_item)xv_create(panel, PANEL_CHOICE,
		PANEL_LABEL_STRING,	"Frames: ",
		PANEL_CHOICE_STRINGS,	"All", "Range", NULL,
		PANEL_VALUE,		0,
		XV_Y,			xv_row(panel,1),
		PANEL_NOTIFY_PROC,	frames_notify,
		NULL);

        first_frame_item = (Panel_item)xv_create(panel, PANEL_NUMERIC_TEXT,
                PANEL_LABEL_STRING,     "First Frame:",
                PANEL_VALUE,            0,
                PANEL_MIN_VALUE,        0,
                PANEL_MAX_VALUE,        500,
                PANEL_VALUE_DISPLAY_LENGTH,     5,
                PANEL_VALUE_STORED_LENGTH,      5,
		PANEL_INACTIVE,		TRUE,
                NULL);

        last_frame_item = (Panel_item)xv_create(panel, PANEL_NUMERIC_TEXT,
                PANEL_LABEL_STRING,     "Last Frame:",
                PANEL_VALUE,            0,
                PANEL_MIN_VALUE,        0,
                PANEL_MAX_VALUE,        500,
                PANEL_VALUE_DISPLAY_LENGTH,     5,
                PANEL_VALUE_STORED_LENGTH,      5,
		PANEL_INACTIVE,		TRUE,
                NULL);

	load_button = (Panel_item) xv_create(panel, PANEL_BUTTON,
		PANEL_LABEL_STRING,	"Load Image",
		PANEL_NOTIFY_PROC,	do_load_notify,
		XV_Y,			xv_row(panel,2),
		NULL);
	
	hide_button = (Panel_item) xv_create(panel, PANEL_BUTTON,
		PANEL_LABEL_STRING,	"Hide",
		PANEL_NOTIFY_PROC,	hide_load_notify,
		XV_Y,			xv_row(panel,2),
		NULL);

	window_fit_height(panel);
	window_fit(load_command_frame);

	/* Center the button along the X-axis, but leave default y-pos */
	load_width = (int)xv_get(load_button,XV_WIDTH);

	load_x = (int) xv_get(panel,XV_WIDTH)/2 - 
		(load_width + 4 + (int)xv_get(hide_button,XV_WIDTH))/2;

	xv_set(load_button, XV_X, load_x, NULL);

	xv_set(hide_button, XV_X, load_x + load_width + 4, NULL);

	xv_set(panel, 
		PANEL_DEFAULT_ITEM, 	load_button, 
		NULL);
}

/*
 * Create the command frame popup for saving images 
 */
void	create_save_command_frame()
{
	Panel		panel;
	Panel_item	save_button, hide_button;
	int		save_x;
	int		save_width;

	save_command_frame = xv_create(frame, FRAME_CMD,
		FRAME_LABEL,	"Save Image",
		NULL);

	panel = (Panel)xv_get(save_command_frame, FRAME_CMD_PANEL);

	save_file_item = (Panel_item) xv_create(panel, PANEL_TEXT,
		PANEL_LABEL_STRING, 		"File:",
		PANEL_VALUE,		 	filename,
		PANEL_VALUE_STORED_LENGTH,	TEXT_LENGTH,
		PANEL_NOTIFY_PROC,		do_save_notify,
		NULL);

	save_button = (Panel_item) xv_create(panel, PANEL_BUTTON,
		PANEL_LABEL_STRING,	"Save Image",
		PANEL_NOTIFY_PROC,	do_save_notify,
		XV_Y,			xv_row(panel,1),
		NULL);
	
	hide_button = (Panel_item) xv_create(panel, PANEL_BUTTON,
		PANEL_LABEL_STRING,	"Hide",
		PANEL_NOTIFY_PROC,	hide_save_notify,
		XV_Y,			xv_row(panel,1),
		NULL);

	window_fit_height(panel);
	window_fit(save_command_frame);

	/* Center the button along the X-axis, but leave default y-pos */
	save_width = (int)xv_get(save_button,XV_WIDTH);

	save_x = (int) xv_get(panel,XV_WIDTH)/2 - 
		(save_width + 4 + (int)xv_get(hide_button,XV_WIDTH))/2;

	xv_set(save_button, XV_X, save_x, NULL);

	xv_set(hide_button, XV_X, save_x + save_width + 4, NULL);

	xv_set(panel, 
		PANEL_DEFAULT_ITEM, 	save_button, 
		NULL);
}
