
/*
 *	@(#)xv_animation.c	2.6 11/29/91
 *
 * XView preview for HIPS images V1.0
 *
 * Author: 	Cathy Waite
 * 		(c) The Turing Institute
 *
 * Fri Aug  9 17:06:10 BST 1991
 *
 *  xv_animation.c : Contains defintions for command popups for animating
 *  		     sequences of imnages.
 */
#include "xv_frame.h"
#include <xview/panel.h>

#include "appl.h"
#include "xv_animation.h"
#include <xview/notify.h>
#include <xview/svrimage.h>


/************************************************************************
 *
 *		Static XView Items .....
 *
 ************************************************************************/
Window			animation_xwin;
Frame			animation_command_frame;
Panel_item		animation_name;
Panel_item		frame_slider;
Panel_item		speed_slider;
Panel_item		step_size;
Panel_item		direction_item;
Panel_item		range_toggle;
struct	itimerval	timer;
h_boolean			run_once = FALSE;
h_boolean			there_and_back = FALSE;
h_boolean			running = FALSE;
int			step = 1;

#define FORWARDS	0
#define BACKWARDS	1	
int			prev_direction = FORWARDS;

/* Read in inages used for the direction chice buttons */
short palindromic_bits[] = {
#include "palindromic.ptr"
};

short backwards_bits[] = {
#include "backwards.ptr"
};

short forwards_bits[] = {
#include "forwards.ptr"
};

/***********************************************************************
 *
 *		NOTIFICATION PROCEDURES
 *
 ***********************************************************************/

/* Routine used to advance (or retreat) current frame by the step size.
 * Used both by Step buttion and timer.
 */

void	do_step()
{
	int	current,direction,new_val,max_val;

	direction = (int)xv_get(direction_item, PANEL_VALUE);
	current = (int)xv_get(frame_slider,PANEL_VALUE);
	max_val = (int)xv_get(step_size,PANEL_MAX_VALUE);
	switch (direction) {
		case 0 :	/* Forwards */
			/* CAW: BF 1.	was >= */
			if ((current+step) > max_val)
			{
				new_val = 0;
				if (run_once == TRUE)
					stop_notify();
			}
			else
				new_val = current+step;
			break;
		case 1 :	/* Backwards */
			/* CAW: BF 1. 	was <= */
			if ((current-step) < 0) 
			{
				new_val =  max_val;
				if (run_once == TRUE)
					stop_notify();
			}
			else
				new_val = current-step;
			break;
		case 2 : 	/* Palindormic */
			if (prev_direction == FORWARDS)
			{
				new_val = current+step;
				if (new_val > max_val)
				{
					new_val = max_val-1;
					prev_direction = BACKWARDS;
					if (run_once == TRUE)
					{
						if (there_and_back)
							stop_notify();
						else
							there_and_back = TRUE;
					}
				}
			}
			else
			{
				new_val = current - step;
				if (new_val < 0)
				{
					new_val = 1;
					prev_direction = FORWARDS;
					if (run_once == TRUE)
					{
						if (there_and_back)
							stop_notify();
						else
							there_and_back = TRUE;
					}
				}
			}
			break;
	}
	xv_set(frame_slider,PANEL_VALUE,new_val,NULL);
	current_frame = new_val;
	render_image();
}

/* Notify for framenumber slider ... */
void	frame_no_notify(item, value, event)
Panel_item	item;
int		value;
Event		*event;
{
	current_frame = value;
	render_image();
}

void	hide_animation_notify()
{
	xv_set(animation_command_frame, FRAME_CMD_PUSHPIN_IN, FALSE, NULL);
}

/* Timer noitification routines .... */
void	stop_notify()
{
	notify_set_itimer_func(frame, NOTIFY_FUNC_NULL, ITIMER_REAL, NULL,
	NULL);
	running = FALSE;
	run_once = FALSE;
}

void	step_notify()
{
	stop_notify();
	do_step();
}

void	set_timer()
{
	int	value = (int)xv_get(speed_slider, PANEL_VALUE);
	long	timer_val;

	if (value == 0)
	{
		timer.it_interval.tv_usec = 0;
		timer.it_interval.tv_sec = 0;
		timer.it_value.tv_usec = 0;
		timer.it_value.tv_sec = 0;
	}
	else
	{
		if (((int)xv_get(range_toggle ,PANEL_VALUE) == 0)
			|| (value == 1))
		{
			/* Slow range */
			timer.it_interval.tv_usec = 0;
			timer.it_interval.tv_sec = value;
			timer.it_value.tv_usec = 0;
			timer.it_value.tv_sec = value;
		}
		else
		{
			/* Fast Range */
			timer_val = 1000000/value;;
			timer.it_interval.tv_usec = timer_val;
			timer.it_interval.tv_sec = 0;
			timer.it_value.tv_usec = timer_val;
			timer.it_value.tv_sec = 0;
		}
	}
	notify_set_itimer_func(frame, do_step, ITIMER_REAL, &timer,
		NULL);
}

void	run_notify()
{
	run_once = FALSE;
	running = TRUE;
	set_timer();
}

void	run_once_notify()
{
	run_once = TRUE;
	there_and_back = FALSE;
	running = TRUE;
	set_timer();
}

/* Temporariily stop the timer */
void	interrupt_timer()
{
	if (running)
		notify_set_itimer_func(frame, NOTIFY_FUNC_NULL, ITIMER_REAL, 
			NULL, NULL);
}

/* ... and restat it again */
void	restart_timer()
{
	if (running)
		set_timer();
}

/* Change the speed of the timer notification events */
void	update_speed()
{
	if (running == TRUE)
		set_timer();
}

/* change the step ... */
void	update_step()
{
	step = (int)xv_get(step_size,PANEL_VALUE);
}

/* Change the speed mode */
void	range_notify(item, value, event)
Panel_item	item;
int		value;
Event		*event;
{
	if (value)
	{
		/* Fast */
		xv_set(speed_slider, PANEL_LABEL_STRING, "Frames/Sec:",NULL);
		update_speed();
	}
	else
	{
		/* Slow */
		xv_set(speed_slider, PANEL_LABEL_STRING, "Secs/Frame:",NULL);
		update_speed();
	}
}

/************************************************************************
 *
 *		Creation of Frame, Control Panel and Panel_items
 *
 ***********************************************************************/
void	create_animation_command_frame()
{
	Panel		panel;
	Panel_item	hide_button;
	Server_image	palindromic_image, backwards_image, forwards_image;

	animation_command_frame = xv_create(frame, FRAME_CMD,
		FRAME_LABEL,	"Animation",
		NULL);

	panel = (Panel)xv_get(animation_command_frame,
		FRAME_CMD_PANEL);

	animation_name = (Panel_item) xv_create(panel, PANEL_MESSAGE,
		PANEL_LABEL_STRING,	filename,
		PANEL_LABEL_BOLD,	TRUE,
		NULL);

	(void)	xv_create(panel, PANEL_BUTTON,
		PANEL_LABEL_STRING,	"Run",
		PANEL_NOTIFY_PROC,	run_notify,
		XV_Y,			xv_row(panel,1),
		XV_X,			120,
		NULL);

	(void)	xv_create(panel, PANEL_BUTTON,
		PANEL_LABEL_STRING,	"Run Once",
		PANEL_NOTIFY_PROC,	run_once_notify,
		XV_Y,			xv_row(panel,1),
		NULL);

	(void)	xv_create(panel, PANEL_BUTTON,
		PANEL_LABEL_STRING,	"Stop",
		PANEL_NOTIFY_PROC,	stop_notify,
		XV_Y,			xv_row(panel,1),
		NULL);

	(void)	xv_create(panel, PANEL_BUTTON,
		PANEL_LABEL_STRING,	"Step",
		PANEL_NOTIFY_PROC,	step_notify,
		XV_Y,			xv_row(panel,1),
		NULL);

	step_size = (Panel_item)xv_create(panel, PANEL_NUMERIC_TEXT,
		PANEL_LABEL_STRING, 		"Step Size:",
		PANEL_VALUE,		 	1,
		PANEL_MAX_VALUE,		100,
		PANEL_MIN_VALUE,		1,
		PANEL_VALUE_DISPLAY_LENGTH,	5,
		PANEL_VALUE_STORED_LENGTH,	5,
		PANEL_NOTIFY_PROC,		update_step,
		XV_Y,				xv_row(panel,1),
		NULL);

	palindromic_image = (Server_image)xv_create(NULL, SERVER_IMAGE,
		XV_WIDTH,		32,
		XV_HEIGHT,		16,
		SERVER_IMAGE_BITS,	palindromic_bits,
		NULL);

	backwards_image = (Server_image)xv_create(NULL, SERVER_IMAGE,
		XV_WIDTH,		32,
		XV_HEIGHT,		16,
		SERVER_IMAGE_BITS,	backwards_bits,
		NULL);

	forwards_image = (Server_image)xv_create(NULL, SERVER_IMAGE,
		XV_WIDTH,		32,
		XV_HEIGHT,		16,
		SERVER_IMAGE_BITS,	forwards_bits,
		NULL);

	direction_item = (Panel_item)xv_create(panel, PANEL_CHOICE,
		PANEL_LABEL_STRING,	"Direction: ",
		PANEL_CHOICE_IMAGES,	forwards_image, backwards_image,
		palindromic_image, NULL,
		PANEL_VALUE,		0,
		XV_X,			42,
		XV_Y,			xv_row(panel,3),
		NULL);

	frame_slider = (Panel_item)xv_create(panel, PANEL_SLIDER,
		PANEL_LABEL_STRING,	"Frame Number:",
		PANEL_VALUE,		0,
		PANEL_MIN_VALUE,	0,
		PANEL_MAX_VALUE,	10,
		PANEL_NOTIFY_PROC,	frame_no_notify,
		PANEL_SLIDER_WIDTH,	300,
		PANEL_NOTIFY_LEVEL,	PANEL_ALL,
		PANEL_SLIDER_END_BOXES, TRUE,
		XV_X,			xv_col(panel,0),
		XV_Y,			xv_row(panel,2),
		NULL);

	speed_slider = (Panel_item)xv_create(panel, PANEL_SLIDER,
		PANEL_LABEL_STRING,	"Frames/Sec:",
		PANEL_VALUE,		2,
		PANEL_MIN_VALUE,	1,
		PANEL_MAX_VALUE,	60,
		PANEL_NOTIFY_PROC,	update_speed,
		PANEL_NOTIFY_LEVEL,	PANEL_ALL,
		PANEL_SLIDER_END_BOXES, TRUE,
		PANEL_SLIDER_WIDTH,	300,
		XV_X,			26,
		XV_Y,			xv_row(panel,4),
		NULL);

	range_toggle = (Panel_item)xv_create(panel, PANEL_CHOICE,
		PANEL_LABEL_STRING,	"Speed Range:",
		PANEL_CHOICE_STRINGS,	"Slow","Fast",NULL,
		PANEL_VALUE,		1,
		PANEL_NOTIFY_PROC,	range_notify,
		XV_X,			20,
		XV_Y,			xv_row(panel,5),
		NULL);

	hide_button = (Panel_item) xv_create(panel, PANEL_BUTTON,
		PANEL_LABEL_STRING,	"Hide",
		PANEL_NOTIFY_PROC,	hide_animation_notify,
		XV_X,			xv_col(panel, 3),
		XV_Y,			xv_row(panel, 5),
		NULL);

	window_fit(panel);
	window_fit(animation_command_frame);
	animation_xwin = (XID)xv_get(animation_command_frame, XV_XID);
	xv_set(hide_button,XV_X,
		(int)xv_get(panel,XV_WIDTH) -10
		-(int)xv_get(hide_button,XV_WIDTH),NULL);
}

void	update_animation(image_header, filename)
struct header	image_header;
char	*filename;
{
	if (image_header.num_frame > 1)
	{
		xv_set(animation_name, PANEL_LABEL_STRING,filename,NULL);
		xv_set(animation_menu_item, MENU_INACTIVE, FALSE, NULL);
		xv_set(frame_slider, PANEL_MAX_VALUE,
			image_header.num_frame-1, NULL);
		xv_set(step_size, PANEL_MAX_VALUE,
			image_header.num_frame-1, NULL);
	}
	else
	{
		xv_set(animation_command_frame, 
			FRAME_CMD_PUSHPIN_IN, FALSE, 
			XV_SHOW, FALSE,
			NULL);
		xv_set(animation_menu_item, MENU_INACTIVE, TRUE, NULL);
	}
}

