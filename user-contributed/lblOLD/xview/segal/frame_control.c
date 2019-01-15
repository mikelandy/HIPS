/*
 *	frame_control.c
 *
 *	Bryan Skene, LBL July 1991
 *	part of 'segal'
 */

#include "common.h"

#include "frame_control_ui.h"

/* Allocation :) */
frame_control_win_objects *frame_control_win;

/*****************************************************/
void
frame_control_win_init(owner)
Xv_opaque owner;
{

	frame_control_win = frame_control_win_objects_initialize(NULL, owner);

}

/*****************************************************/
void
map_frame_control(item, event)
Panel_item      item;
Event           *event;
{
	void draw_frame_statuses();

	fputs("segal: map_frame_control\n", stderr);
        /* Map / Unmap toggle */
	if (xv_get(frame_control_win->win, XV_SHOW, NULL) == FALSE) {
		xv_set(frame_control_win->win,
			XV_SHOW, TRUE,
			NULL);
		draw_frame_statuses();
	}
	else xv_set(frame_control_win->win,
			XV_SHOW, FALSE,
			NULL);
}

/*************************************************/
void
draw_frame_statuses()
{
	void draw_image_frame_statuses();	
	void draw_mask_frame_statuses();	

	draw_image_frame_statuses();	
	draw_mask_frame_statuses();	
}

/*************************************************/
void
draw_mask_frame_statuses()
{
	int x;
	u_long color;

	for(x = 0; x < segal.frames; x++) {
		switch(m[segal.edit_m].frames[x].frame_status) {
		case FRAME_BLANK : color = blue_standout;
			break;
		case FRAME_ALTERED : color = yellow_standout;
			break;
		case FRAME_SAVED : color = red_standout;
			break;
		case FRAME_LOCKED : color = green_standout;
			break;
		default : break;
		}

		XSetForeground(display, gc, color);
		XDrawLine(display, mask_frame_status_xid, gc,
			x, 0, x, 25); 
	}
}

/*************************************************/
void
draw_image_frame_statuses()
{
	int x;
	u_long color;

	for(x = 0; x < segal.frames; x++) {
		switch(himage.frames[x].frame_reg) {
		case FRAME_UNREGISTERED : color = blue_standout;
			break;
		case FRAME_REGISTERED : color = red_standout;
			break;
		default : break;
		}

		XSetForeground(display, gc, color);
		XDrawLine(display, image_frame_status_xid, gc,
			x, 0, x, 25); 
	}
}

/*************************************************/
void
movie_go_proc(item, event)
Panel_item      item;
Event           *event;
{
	void	get_frame_proc();
	int	i, beg_frame, end_frame;

	beg_frame = atoi(xv_get(frame_control_win->movie_beg_frame,
		PANEL_VALUE, NULL));
	end_frame = atoi(xv_get(frame_control_win->movie_end_frame,
		PANEL_VALUE, NULL));

	/* make sure within data set */
	/* HERE
	if(beg_frame < 0)
		beg_frame = 0;
	else if(beg_frame > segal.frames)
		beg_frame = segal.frames;
	if(end_frame < 0)
		end_frame = 0;
	else if(end_frame > segal.frames)
		end_frame = segal.frames;
	(void) xv_set(frame_control_win->movie_beg_frame,
		PANEL_VALUE, beg_frame,
		NULL);
	(void) xv_set(frame_control_win->movie_end_frame,
		PANEL_VALUE, end_frame,
		NULL);
	*/

	if(beg_frame <= end_frame) 
		for(i = beg_frame; i <= end_frame; i++) {
			(void) xv_set(frame_control_win->curr_frame,
				PANEL_VALUE, i,
				NULL);
			get_frame_proc(item, i, event);
		}
	else
		for(i = beg_frame; i >= end_frame; i--) {
			(void) xv_set(frame_control_win->curr_frame,
				PANEL_VALUE, i,
				NULL);
			get_frame_proc(item, i, event);
		}
}

/*************************************************/
void
set_mask_frame_status_proc(item, value, event)
Panel_item item;
int value;
Event *event;
{
	void	draw_mask_frame_statuses();

	m[segal.edit_m].frames[segal.curr_frame].frame_status = value;
	draw_mask_frame_statuses();
}

/*************************************************/
void
set_image_frame_status_proc(item, value, event)
Panel_item item;
int value;
Event *event;
{
	void	draw_image_frame_statuses();

	himage.frames[segal.curr_frame].frame_reg = value;
	draw_image_frame_statuses();
}
