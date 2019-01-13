
/*
 * @(#)xv_adjust.c	2.4	11/29/91
 *
 * XView preview for HIPS images V1.0
 *
 * Author: 	Cathy Waite
 * 		(c) The Turing Institute
 *
 * Fri Aug  9 17:06:10 BST 1991
 *
 *  xv_adjust.c : Contains defintions for command popup for adjusting
 *  		  images.
 */

#include "xv_frame.h"
#include <xview/panel.h>
#include <xview/notice.h>

#include "xv_adjust.h"
#include "colors.h"
#include "appl.h"

/************************************************************************
 *
 *		Static XView Items .....
 *
 ************************************************************************/
Window		adjust_xwin;
Frame		adjust_command_frame;
Panel_item	adjust_name;
Panel_item	brightness_slider, contrast_slider;
Panel_item	resize_al, resize_width, resize_height, resize_button;

/***********************************************************************
 *
 *		NOTIFICATION PROCEDURES
 *
 ***********************************************************************/

void	hide_adjust_notify()
{
	xv_set(adjust_command_frame, FRAME_CMD_PUSHPIN_IN, FALSE, NULL);
}

void	brightness_notify(item, value, event)
Panel_item	item;
int		value;
Event		*event;
{
	/* Convert brightness to a offest in range 
 	 *	-no_colors -- no_colors 
	 * If this algorithim is changed ... be sure to change the
	 * method that converts back from brightness_range to the
	 * value for the slider,  set_bright_contr.
	 */
		
	brightness = (value - 50)*no_colors/50;
	update_color_map();
}

void	contrast_notify(item, value, event)
Panel_item	item;
int		value;
Event		*event;
{
	/* Range:   0 -> 0.02
	 *	   50 -> 1
	 *	  100 -> 2
	 * If this algorithim is changed ... be sure to change the
	 * method that converts back from brightness_range to the
	 * value for the slider,  set_bright_contr.
	 */
	contrast = value/50.0;
	update_color_map();
}

/* Given a brightness and contrast value, map it onto the slider.  This
 * uses the inverse of the equations in the previous methods.
 */
void	set_bright_contr(bright, contr)
int	bright;
float	contr;
{
	int bright_val, contr_val;

	bright_val = brightness*50/no_colors+50;
	contr_val = contr*50;
	xv_set(brightness_slider, PANEL_VALUE, bright_val, NULL);
	xv_set(contrast_slider, PANEL_VALUE, contr_val, NULL);
}


/* Reset the brightnesss to the standard.  This assumes the standard is
 * halfway up the slider ... i.e. 50.  The color_map is update also.
 */
void 	reset_brightness_notify()
{
	xv_set(brightness_slider, PANEL_VALUE, 50, NULL);
	brightness = std_brightness;
	update_color_map();
}

/* Reset the brightnesss to the standard.  This assumes the standard is
 * halfway up the slider ... i.e. 50. The color map is updated also,
 */
void 	reset_contrast_notify()
{
	xv_set(contrast_slider, PANEL_VALUE, 50, NULL);
	contrast = std_contrast;
	update_color_map();
}

/* Update the resize algorithm depending on choice */
void	resize_al_notify(item, value, event)
Panel_item	item;
int		value;
Event		*event;
{
	slow_resize = value;
}

/* Actually resize the window depending on the values set in the 
 *  textfield.
 */
void	resize_notify()
{
	int new_width, new_height;
	new_width = (int)xv_get(resize_width, PANEL_VALUE);
	new_height = (int)xv_get(resize_height, PANEL_VALUE);
	update_frame(new_width, new_height,TRUE);
}

/* Given a width and height, update the textfields appropriately */
void	set_width_height(new_width, new_height)
{
	xv_set(resize_width, PANEL_VALUE, new_width, NULL);
	xv_set(resize_height, PANEL_VALUE, new_height, NULL);
}

/************************************************************************
 *
 *		Creation of Frame, Control Panel and Panel_items
 *
 ***********************************************************************/

/*
 * Create command frame popup containg the adjust controls. 
 *
 */
void	create_adjust_command_frame()
{
	Panel		panel;
	Panel_item	hide_button;

	adjust_command_frame = xv_create(frame, FRAME_CMD,
		FRAME_LABEL,	"Adjustment",
		NULL);

	panel = (Panel)xv_get(adjust_command_frame,
		FRAME_CMD_PANEL);

	adjust_name = (Panel_item) xv_create(panel, PANEL_MESSAGE,
		PANEL_LABEL_STRING,	filename,
		PANEL_LABEL_BOLD,	TRUE,
		NULL);

	(void)xv_create(panel, PANEL_MESSAGE,
		PANEL_LABEL_STRING,	"Brightness:  Low",
		PANEL_LABEL_BOLD,	TRUE,
		XV_Y,			xv_row(panel,1),
		XV_X,			42,
		NULL);

	brightness_slider = (Panel_item) xv_create(panel, PANEL_SLIDER,
		PANEL_LABEL_STRING,	"",
		PANEL_VALUE,		50,
		PANEL_MIN_VALUE,	0,
		PANEL_MAX_VALUE,	100,
		PANEL_SLIDER_WIDTH,	300,
		PANEL_NOTIFY_LEVEL,	PANEL_ALL,
		PANEL_SHOW_VALUE,	FALSE,
		PANEL_SHOW_RANGE,	FALSE,
		PANEL_NOTIFY_PROC,	brightness_notify,
		XV_Y,			xv_row(panel,1),
		NULL);

	(void)xv_create(panel, PANEL_MESSAGE,
		PANEL_LABEL_STRING,	"High",
		PANEL_LABEL_BOLD,	TRUE,
		XV_Y,			xv_row(panel,1),
		NULL);

	(void)xv_create(panel, PANEL_BUTTON,
		PANEL_LABEL_STRING,	"Reset",
		PANEL_NOTIFY_PROC,	reset_brightness_notify,
		NULL);


	(void)xv_create(panel, PANEL_MESSAGE,
		PANEL_LABEL_STRING,	"Contrast:  Low",
		PANEL_LABEL_BOLD,	TRUE,
		XV_Y,			xv_row(panel,2),
		XV_X,			58,
		NULL);

	contrast_slider = (Panel_item) xv_create(panel, PANEL_SLIDER,
		PANEL_LABEL_STRING,	"",
		PANEL_VALUE,		50,
		PANEL_MIN_VALUE,	1,
		PANEL_MAX_VALUE,	100,
		PANEL_SLIDER_WIDTH,	300,
		PANEL_NOTIFY_PROC,	contrast_notify,
		PANEL_NOTIFY_LEVEL,	PANEL_ALL,
		PANEL_SHOW_VALUE,	FALSE,
		PANEL_SHOW_RANGE,	FALSE,
		XV_Y,			xv_row(panel,2),
		NULL);

	(void)xv_create(panel, PANEL_MESSAGE,
		PANEL_LABEL_STRING,	"High",
		PANEL_LABEL_BOLD,	TRUE,
		XV_Y,			xv_row(panel,2),
		NULL);

	(void)xv_create(panel, PANEL_BUTTON,
		PANEL_LABEL_STRING,	"Reset",
		PANEL_NOTIFY_PROC,	reset_contrast_notify,
		NULL);

	resize_al = (Panel_item)xv_create(panel, PANEL_CHOICE,
		PANEL_LABEL_STRING,	"Resize Algorithm: ",
		PANEL_CHOICE_STRINGS,	"Replication","Interpolation",NULL,
		PANEL_VALUE,		(slow_resize == TRUE)?1:0 ,
		XV_X,			0,
		XV_Y,			xv_row(panel, 3),
		PANEL_NOTIFY_PROC,	resize_al_notify,
		NULL);

	resize_width = (Panel_item)xv_create(panel, PANEL_NUMERIC_TEXT,
		PANEL_LABEL_STRING,	"Width:",
		PANEL_VALUE,		0,
		PANEL_MIN_VALUE,	1,
		PANEL_MAX_VALUE,	DisplayWidth(display,screen),
		PANEL_VALUE_DISPLAY_LENGTH,	5,
		PANEL_VALUE_STORED_LENGTH,	5,
		XV_X,			76,
		XV_Y,			xv_row(panel,4),
		NULL);

	resize_height = (Panel_item)xv_create(panel, PANEL_NUMERIC_TEXT,
		PANEL_LABEL_STRING,	"Height:",
		PANEL_VALUE,		0,
		PANEL_MIN_VALUE,	1,
		PANEL_MAX_VALUE,	DisplayHeight(display,screen),
		PANEL_VALUE_DISPLAY_LENGTH,	5,
		PANEL_VALUE_STORED_LENGTH,	5,
		NULL);

	resize_button = (Panel_item) xv_create(panel, PANEL_BUTTON,
		PANEL_LABEL_STRING,	"Resize",
		PANEL_NOTIFY_PROC,	resize_notify,
		NULL);

	hide_button = (Panel_item) xv_create(panel, PANEL_BUTTON,
		PANEL_LABEL_STRING,	"Hide",
		PANEL_NOTIFY_PROC,	hide_adjust_notify,
		XV_Y,			xv_row(panel, 5),
		NULL);

	window_fit(panel);
	window_fit(adjust_command_frame);
	xv_set(hide_button,XV_X,
		(int)xv_get(panel,XV_WIDTH) -10
		-(int)xv_get(hide_button,XV_WIDTH),NULL);

	adjust_xwin = (XID)xv_get(adjust_command_frame, XV_XID);
}

void	update_adjust(filename)
char	*filename;
{
	xv_set(adjust_name, PANEL_LABEL_STRING, filename, NULL);
}

