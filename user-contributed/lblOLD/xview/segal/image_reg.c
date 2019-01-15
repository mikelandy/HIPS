/*
 *      image_reg.c
 *
 *      Bryan Skene, LBL January 1992
 *      part of 'segal'
 */
 
#include "common.h"
 
#include "frame_control.h"
#include "threshold.h"
#include "view.h"

#include "image_reg_ui.h"
 
/* Allocation :) */
image_reg_win_objects *image_reg_win;
image_reg_pop_ref_frame_objects *image_reg_pop_ref_frame;

 
/*****************************************************/
void
image_reg_win_init(owner)
Xv_opaque owner;
{
        image_reg_win = image_reg_win_objects_initialize(NULL, owner);
	image_reg_pop_ref_frame = image_reg_pop_ref_frame_objects_initialize(NULL, image_reg_win->win);

	/* registry info */
	registry.scale_factor = 1.0;
	registry.trans_pixels = 1;
	registry.rot_degrees = 0;
}
 
/*****************************************************/
void
map_image_reg(item, event)
Panel_item      item;
Event           *event;
{
	void ref_frame_setup();
	void change_colormap();
	void build_ximages();
	void image_repaint_proc();

        fputs("segal: map_image_reg\n", stderr);
        /* Map / Unmap toggle */
        if (xv_get(image_reg_win->win, XV_SHOW, NULL) == FALSE) {
		segal.reg_flag = 1;
		ref_frame_setup();
                xv_set(image_reg_win->win,
                        XV_SHOW, TRUE,
                        NULL);
                xv_set(image_reg_pop_ref_frame->pop_ref_frame,
                        XV_SHOW, TRUE,
                        NULL);
		change_colormap(NULL, RED_HUE, NULL);
        }
        else {
		segal.reg_flag = 0;
		xv_set(image_reg_win->win,
                        XV_SHOW, FALSE,
                        NULL);
                xv_set(image_reg_pop_ref_frame->pop_ref_frame,
                        XV_SHOW, FALSE,
                        NULL);
		change_colormap(NULL, GREEN_HUE, NULL);
	}
	build_ximages();
	image_repaint_proc();
}
 
/*****************************************************/
Panel_setting
set_ref_frame_proc(item, event)
Panel_item      item;
Event           *event;
{
	void ref_frame_image_repaint_proc();
	void build_ximages();
	void image_repaint_proc();

	char *value = (char *) xv_get(item, PANEL_VALUE);

	himage.ref_frame = atoi(value);
	if(himage.ref_frame < 1)
		himage.ref_frame = 0;
	if(himage.ref_frame > segal.frames)
		himage.ref_frame = segal.frames;

	xv_set(frame_control_win->text_ref_frame,
		PANEL_VALUE, value,
		NULL);

	xv_set(image_reg_win->text_ref_frame,
		PANEL_VALUE, value,
		NULL);

	panel_paint(frame_control_win->text_ref_frame, PANEL_CLEAR);
	panel_paint(image_reg_win->text_ref_frame, PANEL_CLEAR);

	get_ref_frame(himage.ref_frame - 1);

	build_ximages();
	ref_frame_image_repaint_proc();
	image_repaint_proc();

	return panel_text_notify(item, event);
}

/*****************************************************/
void
ref_frame_setup()
{
Rect      vrect, rfrect;	/* frame rect structure for segal win and
				 * reference frame pop-up win */

	/* set size */
	(void) xv_set(image_reg_pop_ref_frame->pop_ref_frame,
		XV_WIDTH, segal.width,
		XV_HEIGHT, segal.height,
		NULL);
	(void) xv_set(image_reg_pop_ref_frame->canv_ref_frame,
		XV_WIDTH, segal.width,
		XV_HEIGHT, segal.height,
		CANVAS_WIDTH, segal.cols + 1,
		CANVAS_HEIGHT, segal.rows + 1,
		CANVAS_AUTO_SHRINK, FALSE,
		CANVAS_AUTO_EXPAND, FALSE,
		NULL);
	if((int) xv_get(image_reg_pop_ref_frame->pop_ref_frame,
		XV_SHOW, NULL) == TRUE)
		return;

	(void) xv_set(image_reg_pop_ref_frame->pop_ref_frame,
		XV_SHOW, TRUE,
		NULL);
    /*
     * set location: NOTE: to set the location of a frame using
     * 'frame_set_rect'  (at least under the twm window manager) XV_SHOW must
     * be TRUE first
     */

    /* set image_reg_pop_ref_frame window location relative to view window */
	frame_get_rect(view_win->win, &vrect);

	if (vrect.r_top >= 0 && vrect.r_left >= 0) { /* only if view window
							is on screen */
		frame_get_rect(image_reg_pop_ref_frame->pop_ref_frame, &rfrect);
		rfrect.r_top = vrect.r_top + 25; 
		rfrect.r_left = vrect.r_left + vrect.r_width + 2;
		frame_set_rect(image_reg_pop_ref_frame->pop_ref_frame, &rfrect);
	}
}


/*************************************************************/
void
ref_frame_image_repaint_proc()
{
	if (xv_get(image_reg_pop_ref_frame->pop_ref_frame,
		XV_SHOW, NULL) == FALSE)
		return;

	XPutImage(display, ref_frame_xid, gc, ref_frame_image,
		0, 0, 0, 0, ref_frame_image->width, ref_frame_image->height);

}

/*************************************************************/
void
ref_frame_map_proc(item, event)
Panel_item      item;
Event           *event;
{
/* Map / Unmap toggle */

	if (xv_get(image_reg_pop_ref_frame->pop_ref_frame,
		XV_SHOW, NULL) == FALSE) {
		(void) xv_set(image_reg_pop_ref_frame->pop_ref_frame,
			XV_WIDTH, segal.cols + 1,
			NULL);
		(void) xv_set(image_reg_pop_ref_frame->pop_ref_frame,
			XV_HEIGHT, segal.rows + 1,
			NULL);
		xv_set(image_reg_pop_ref_frame->pop_ref_frame, XV_SHOW, TRUE,
			NULL);

		XPutImage(display, ref_frame_xid, gc,
			ref_frame_image, 0, 0, 0, 0,
			ref_frame_image->width, ref_frame_image->height);
	}
	else xv_set(image_reg_pop_ref_frame->pop_ref_frame, XV_SHOW, FALSE,
			NULL);
}

/*************************************************************/
Notify_value
ref_frame_mouse_proc(win, event, arg, type)
Xv_window       win;
Event           *event;
Notify_arg      arg;
Notify_event_type type;
{
	void begin_or_end_drawing_line();
	void drag_out_line();

	switch (event_id(event)) {
	case MS_RIGHT:
	case MS_LEFT:
	case MS_MIDDLE:
		begin_or_end_drawing_line(win, event, REF_FRAME, &ref_line);
		break;
	 
	case LOC_DRAG:
		drag_out_line(win, event, REF_FRAME, &ref_line);	
		break;

	default:
		break;
	}
	return notify_next_event_func(win, (Notify_event) event, arg, type);
}

/*************************************************************/
void
begin_or_end_drawing_line(win, event, which_win, line)
Xv_window       win;
Event           *event;
int		which_win;
LINE_INFO	*line;
{
	void draw_line();
	void bind_line_coords();

	if(event_is_down(event)) { /* Beginning of drawing a line */
		/* Set gc logical function to invert -- keeps orig data */
		XSetFunction(display, gc, GXinvert);

		/* erase old line (if there is one) */
		if(!line->refresh)
		draw_line(which_win, line->x1, line->y1,
			line->x2, line->y2);
		else line->refresh = 0;

		line->x1 = event_x(event);
		line->y1 = event_y(event);

		/* originally, (x2,y2) is at the origin too */
		line->x2 = line->x1;
		line->y2 = line->y1;

		/*draw_line_size_mesg();
		*/
	}
	else if(event_is_up(event)) { /* Release of the button */
		/* bind register line within the bounds of the image */
		bind_line_coords(line);

		/* Set gc logical function back to 'copy' */
		XSetFunction(display, gc, GXcopy);

		draw_hint("Scale, Translate, or Rotate");
	}
}

/*************************************************************/
void
drag_out_line(win, event, which_win, line)
Xv_window       win;
Event           *event;
int		which_win;
LINE_INFO	*line;
{
	void draw_line();
	void bind_line_coords();

	/* erase old register line */
	draw_line(which_win, line->x1, line->y1,
		line->x2, line->y2);

	line->x2 = event_x(event);
	line->y2 = event_y(event);


	/* draw new register line */
	bind_line_coords(line);
	draw_line(which_win, line->x1, line->y1,
		line->x2, line->y2);
}

/*************************************************************/
void
draw_line(which_win, x1, y1, x2, y2)
int which_win;
int x1, y1, x2, y2;
{
	XID win_xid;

	switch(which_win) {
	case VIEW_WIN :
		win_xid = view_xid;
		break;
	case REF_FRAME :
		win_xid = ref_frame_xid;
		break;
	default :
		break;
	}

/* could add length message function call here */
	XSetForeground(display, gc, yellow_standout);
	XDrawLine(display, win_xid, gc, x1, y1, x2, y2);
}

/*************************************************************/
void
bind_line_coords(line)
LINE_INFO *line;
{
	if(line->x2 > segal.cols)    line->x2 = segal.cols;
	else if(line->x2 < 0)        line->x2 = 0;
	if(line->y2 > segal.rows)    line->y2 = segal.rows;
	else if(line->y2 < 0)        line->y2 = 0;
}

/*************************************************************/
void
set_scale_factor(item, event)
Panel_item      item;
Event           *event;
{
	char *value = (char *) xv_get(item, PANEL_VALUE);

	registry.scale_factor = atof(value);
}

/*************************************************************/
void
set_trans_pixel_amount(item, value, event)
Panel_item      item;
int		value;
Event           *event;
{
	registry.trans_pixels = value;
}

/*************************************************************/
void
set_rotation_degrees(item, event)
Panel_item      item;
Event           *event;
{
	char *value = (char *) xv_get(item, PANEL_VALUE);

	registry.rot_degrees = atof(value);
}
/*************************************************************/
void
but_up_translate_proc(item, event)
Panel_item      item;
Event           *event;
{
	void translate_image();	
	translate_image(registry.trans_pixels, UP);
}

/*************************************************************/
void
but_down_translate_proc(item, event)
Panel_item      item;
Event           *event;
{
	void translate_image();	
	translate_image(registry.trans_pixels, DOWN);
}

/*************************************************************/
void
but_right_translate_proc(item, event)
Panel_item      item;
Event           *event;
{
	void translate_image();	
	translate_image(registry.trans_pixels, RIGHT);
}

/*************************************************************/
void
but_left_translate_proc(item, event)
Panel_item      item;
Event           *event;
{
	void translate_image();	
	translate_image(registry.trans_pixels, LEFT);
}

/*************************************************************/
void
but_point_translate_proc(item, event)
Panel_item      item;
Event           *event;
{
	void translate_image();	

	int delta_x, delta_y, dir_x, dir_y;

	if(image_line.x1 < ref_line.x1) dir_x = RIGHT;
	else dir_x = LEFT;

	if(image_line.y1 < ref_line.y1) dir_y = DOWN;
	else dir_y = UP;

	delta_x = abs(image_line.x1 - ref_line.x1);
	delta_y = abs(image_line.y1 - ref_line.y1);

	if(delta_x > 0) translate_image(delta_x, dir_x);
	if(delta_y > 0) translate_image(delta_y, dir_y);
}

/*************************************************************/
void
translate_image(pixels, direction)
int pixels;
int direction;
{
	void build_ximages();
	void image_repaint_proc();
	void save_undo();

	int i, j;

	switch(direction) {
	case UP :
		for(j = 0; j < segal.rows; j++)
		for(i = 0; i < segal.cols; i++)
			if(j > segal.rows-1 - pixels) image_buf[j][i] = 0;
			else image_buf[j][i] = himage.data[j + pixels][i];
		image_line.y1 -= pixels; 
	break;
	case DOWN :
		for(j = 0; j < segal.rows; j++)
		for(i = 0; i < segal.cols; i++)
			if(j < pixels) image_buf[j][i] = 0;
			else image_buf[j][i] = himage.data[j - pixels][i];
		image_line.y1 += pixels; 
	break;
	case RIGHT :
		for(j = 0; j < segal.rows; j++)
		for(i = 0; i < segal.cols; i++)
			if(i < pixels) image_buf[j][i] = 0;
			else image_buf[j][i] = himage.data[j][i - pixels];
		image_line.x1 += pixels; 
	break;
	case LEFT :
		for(j = 0; j < segal.rows; j++)
		for(i = 0; i < segal.cols; i++)
			if(i > segal.cols-1 - pixels) image_buf[j][i] = 0;
			else image_buf[j][i] = himage.data[j][i + pixels];
		image_line.x1 -= pixels; 
	break;
	default : break;
	}
	save_undo();
	bcopy(image_buf[0], himage.data[0], segal.rows * segal.cols);

	image_line.refresh = 1;
	build_ximages();
	image_repaint_proc();
}

/*************************************************************/
void
but_manual_scale_proc(item, event)
Panel_item      item;
Event           *event;
{
	void scale_image();

	scale_image(registry.scale_factor);
}

/*************************************************************/
void
but_line_scale_proc(item, event)
Panel_item      item;
Event           *event;
{
	void scale_image();

	double distance_image_line, distance_ref_frame_line;

	distance_image_line = (double) sqrt((double) pow(((double) image_line.x2 - (double) image_line.x1), 2.0) + (double) pow(((double) image_line.y2 - (double) image_line.y1), 2.0));

	distance_ref_frame_line = (double) sqrt((double) pow(((double) ref_line.x2 - (double) ref_line.x1), 2.0) + (double) pow(((double) ref_line.y2 - (double) ref_line.y1), 2.0));


	scale_image(distance_ref_frame_line / distance_image_line);
}

/*************************************************************/
void
scale_image(factor)
double factor;
{
/* builds a parameter list and then calls filter_buf() */
	char param_list[80];

	fprintf(stderr, "factor = %lf\n", factor);

	sprintf(param_list, "-f %lf %lf", factor, factor);
	save_undo();
	filter_buf(image_undo_buf, himage.data, himage.filename,
		"stretch", param_list);
	build_ximages();
	image_repaint_proc();
}

/*************************************************************/
void
but_manual_rotate_proc(item, event)
Panel_item      item;
Event           *event;
{
	void rotate_image();
	rotate_image(registry.rot_degrees);
}

/*************************************************************/
void
but_line_rotate_proc(item, event)
Panel_item      item;
Event           *event;
{
}

/*************************************************************/
void
rotate_image(degrees)
double degrees;
{
/* builds a parameter list and then calls filter_buf() */
	char param_list[80];

	fprintf(stderr, "degrees = %lf\n", degrees);

	sprintf(param_list, "-a %lf", degrees);
	save_undo();
	filter_buf(image_undo_buf, himage.data, himage.filename,
		"rotate", param_list);
	build_ximages();
	image_repaint_proc();
}

/*************************************************************/

/*************************************************************/

/*************************************************************/
