/*
 *	@(#)xv_canvas.c	2.6	11/29/91
 *
 * XView preview for HIPS-2 images V1.0
 *
 * Author: 	Cathy Waite
 * 		(c) The Turing Institute
 *
 * Fri Aug  9 17:06:10 BST 1991
 *
 * xv_canvas.h : 
 */
#include <X11/cursorfont.h>
#include "xv_frame.h"
#include <xview/canvas.h>

#include "xv_animation.h"
#include "appl.h"
#include "colors.h"


/************************************************************************
 *
 *              Static XView Items .....
 *
 ************************************************************************/
Canvas		canvas;
Window		canvas_xwin;
Xv_window	canvas_paint_win;

Cursor		top_left, bottom_right, default_cursor;
int		top_left_shape = XC_top_left_corner;
int		bottom_right_shape = XC_bottom_right_corner;
int		default_shape = XC_top_left_arrow;

#define		NONE 	 0
#define		WAITING	 1
#define		DRAGGING 2
int 		anchor_x, anchor_y, x2, y2;
int		extract_mode = NONE;
int		print_mode = FALSE;
int		last_x, last_y;
/***********************************************************************
 *
 *		NOTIFICATION PROCEDURES
 *
 ***********************************************************************/
void	canvas_paint_proc()
{
	render_image();
}	

void	canvas_resize_proc(canvas, width, height)
Canvas	canvas;
int	width, height;
{
	resize_image(width, height);	
}

/*
 *	Routine to define interactively an area to extract from the image
 */
void	interactive_extract()
{
	interrupt_timer();
	print_message("Middle mouse click cancels extract");
	XDefineCursor (display, canvas_xwin, top_left);
	XDefineCursor (display, panel_xwin, top_left);
	extract_mode = WAITING;
}

void	draw_rectangle(x,y,x2,y2)
int	x,y,x2,y2;
{
	XSetFunction(display, image_gc, GXinvert);
	XDrawRectangle(display,canvas_xwin, image_gc, x, y, x2-x, y2-y);
	sprintf(info,"(width, height): %d, %d",abs(x2 -x), abs(y2-y));
	print_message(info);
	XSetFunction(display, image_gc, GXcopy);
}

void	cancel_extract()
{
	if (extract_mode == NONE)
		return;

	if (extract_mode == DRAGGING)
		draw_rectangle(anchor_x, anchor_y, x2, y2);
	/* Cancel the extracting ... */
	XDefineCursor (display, canvas_xwin, default_cursor);
	XDefineCursor (display, panel_xwin, default_cursor);
	print_message("");
	extract_mode = NONE;
	restart_timer();
}

void	print_pixel_value(event)
Event	*event;
{
	int x = event_x(event);
	int y = event_y(event);
	if ( (x >= 0) && (y>=0) && (x<= image_width) && (y<=image_height))
	{
		sprintf(info,"(x,y) = (%d,%d); pixel = %d\n",event_x(event),
		event_y(event), get_pixel_value(event_x(event),
		event_y(event)));
		print_message(info);
		last_x = x;
		last_y = y;
	}
}

void	print_new_pixel_value()
{
	if (print_mode == FALSE)
		return;
	sprintf(info,"(x,y) = (%d,%d); pixel = %d\n",last_x, last_y,
	get_pixel_value(last_x, last_y));
	print_message(info);
}

void	canvas_event_proc(window, event)
Xv_window	window;
Event		*event;
{

	switch(event_action(event))
	{
		case LOC_MOVE:
			if (extract_mode == WAITING)
			{
				sprintf(info,"(x,y): %d, %d\n", event_x(event),
					event_y(event));
				print_message(info);
			}
			break;
		case LOC_DRAG:
			if (extract_mode == DRAGGING)
			{
				draw_rectangle(anchor_x, anchor_y, x2, y2);

				x2 = event_x(event);
				x2 = MAX(0, MIN(image_width-1,x2));
				y2 = event_y(event);
				y2 = MAX(0, MIN(image_height-1,y2));

				draw_rectangle(anchor_x, anchor_y, x2, y2);

			}
			else if (event_right_is_down(event))
				print_pixel_value(event);

			break; 
		case ACTION_SELECT:
			if ((event_is_down(event)) && (extract_mode ==
				WAITING))
			{
				anchor_x = x2 = event_x(event);
				anchor_y = y2 = event_y(event);
				XDefineCursor (display, panel_xwin, bottom_right);
				XDefineCursor (display, canvas_xwin, bottom_right);
				extract_mode = DRAGGING;
				draw_rectangle(anchor_x, anchor_y, x2, y2);
			}
			else if ((event_is_up(event)) && (extract_mode ==
			DRAGGING))
			{
				int x,y,w,h;

				draw_rectangle(anchor_x, anchor_y, x2, y2);
				XDefineCursor (display, canvas_xwin, default_cursor);
				XDefineCursor (display, panel_xwin, default_cursor);
				x2 = event_x(event);
				y2 = event_y(event);
				extract_mode = NONE;
				print_message("");
				x = MIN(anchor_x,x2);
				y = MIN(anchor_y,y2);
				w = MAX(1, MAX(anchor_x, x2) - x );
				h = MAX(1, MAX(anchor_y, y2) - y);
				extract(x, y, w, h); 
				restart_timer();
			}
			else if (with_control_panel == FALSE)
				if (event_is_down(event))
					show_control_panel();
			break;
		case ACTION_ADJUST: 
			if (extract_mode != NONE)
				cancel_extract();
			
			else if (with_control_panel == FALSE)
				if (event_is_down(event))
					show_control_panel();
			break;
		case ACTION_MENU:
			if (with_control_panel == TRUE)
			{
				if (event_is_down(event))
				{
					print_mode = TRUE;
					print_pixel_value(event);
				}	
				else if (event_is_up(event))
					print_mode = FALSE;
			}
			else
				if (event_is_down(event))
					show_control_panel();
			break;
		default: return;
	}
}

/************************************************************************
 *
 *              	Create the Canvas ...
 *
 ***********************************************************************/

void	create_canvas()
{

	canvas = (Canvas)xv_create(frame, CANVAS, 
		XV_X,			0,
		CANVAS_X_PAINT_WINDOW,	TRUE,
		CANVAS_REPAINT_PROC,	canvas_paint_proc,
		CANVAS_RESIZE_PROC,	canvas_resize_proc,
		CANVAS_AUTO_SHRINK,	FALSE,
		CANVAS_AUTO_EXPAND,	FALSE,

		/* RF: 1 ... using XView Color Segments */
		/*
		WIN_CMS,		cms,
		*/
		CANVAS_CMS_REPAINT,	FALSE,

		NULL);
	
	/* RF: 9 */
	if (with_control_panel == TRUE)
		xv_set(canvas,
			WIN_BELOW,	control_panel,
		NULL);
	else
		xv_set(canvas,
		XV_X,		0,
		XV_Y,		0,
		NULL);

	top_left = XCreateFontCursor(display, top_left_shape);
	bottom_right = XCreateFontCursor(display, bottom_right_shape);
	default_cursor = XCreateFontCursor(display, default_shape);

	canvas_paint_win = canvas_paint_window(canvas);
	canvas_xwin = (XID)xv_get(canvas_paint_win, XV_XID);
	xv_set(canvas_paint_win,
		WIN_EVENT_PROC,		canvas_event_proc,
		WIN_CONSUME_EVENTS,
			LOC_MOVE, LOC_DRAG, WIN_MOUSE_BUTTONS,NULL,
		NULL);

}

