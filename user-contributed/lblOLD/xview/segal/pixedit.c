/*
 *  pixedit.c     edit pixels in a binary image         -Brian Tierney,  LBL
 *    part of 'segal'
 */

#include "common.h"
#include "pixedit_ui.h"

/* Allocation :) */
pixedit_win_objects *edit_win;

#include "display_control.h"
#include "mask_control.h"
#include "segal.h"
#include "view.h"

/************************************************************/
void
edit_win_init(owner)
    Xv_opaque owner;
{
    void      Edit_make_simple_colormap();
    void      create_cursors(), pixedit_resize_proc();

    edit_win = pixedit_win_objects_initialize(NULL, owner);

    /* use same colormap as the view window */
    (void) xv_set(edit_win->canvas, WIN_CMS_NAME, "segal", NULL);

    create_cursors();

    /* zoom defaults */
    zoom_info.mag = 2;
}

/**************************************************************/
void
map_edit_win(item, event)               /* procedure for 'mode' setting */
Panel_item item;
Event    *event;
{
	void draw_hint();
	void edit_setup();
	void call_zoom();

	if(region.rows == 0
	|| region.cols == 0
	|| (!segal.image_loaded && !segal.masks))
	/* crash-proofing: do nothing until an image is loaded and a
	 * region is selected
	 */
		return;

	if(xv_get(edit_win->win, XV_SHOW, NULL) == TRUE) {
		(void) xv_set(edit_win->win,
			XV_SHOW, FALSE,
			NULL);
	}
	else {
		set_watch_cursor();
		edit_setup();
		draw_hint("Crop a region in the image");
		unset_watch_cursor();
	}
}

/**************************************************************/
void
edit_setup()
{
	void      edit_repaint_proc(), zoom();
	void      create_zoom_ximages(), image_repaint_proc();
	void      set_window_size_and_loc();

	if ((int) xv_get(view_win->win, XV_SHOW, NULL) == FALSE)
		return;			/* must load images first */

	if (segal.poly_flag != 0 ) 
		poly_proc(NULL,0);

	(void) xv_set(display_control_win->blend_type, PANEL_VALUE, 0, NULL);
	(void) xv_set(display_control_win->mask_type, PANEL_VALUE, 1, NULL);
	(void) xv_set(display_control_win->display_type, PANEL_VALUE,
		segal.display_type, NULL);

	/* initialize to "Build" mode */
	mask_type_proc(NULL, 1, NULL);

	/* intial zoom */
	zoom_info.x = region.crx1;
	zoom_info.y = region.cry1;

	zoom_info.mag = 2;
	(void) xv_set(edit_win->zoom_mag,
		PANEL_VALUE, 1,
		NULL);

	zoom_info.width = zoom_info.mag * region.cols;
	zoom_info.height = zoom_info.mag * region.rows;

	image_repaint_proc(); /* necessary? */
	set_window_size_and_loc();
	create_zoom_ximages();
	zoom();
	XFlush(display);
	edit_repaint_proc();
}

/*************************************************************/
void
set_brush_mode(item, value, event)
Panel_item      item;
int             value;
Event           *event;
{
	if(value == 0) {
		(void) xv_set(edit_win->mask_brush_mode,
			PANEL_INACTIVE, FALSE,
			NULL);
		(void) xv_set(edit_win->image_brush_mode,
			PANEL_INACTIVE, TRUE,
			NULL);
		(void) xv_set(edit_win->image_brush_delta,
			PANEL_INACTIVE, TRUE,
			NULL);
	}
	else if(value == 1) {
		(void) xv_set(edit_win->mask_brush_mode,
			PANEL_INACTIVE, TRUE,
			NULL);
		(void) xv_set(edit_win->image_brush_mode,
			PANEL_INACTIVE, FALSE,
			NULL);
		(void) xv_set(edit_win->image_brush_delta,
			PANEL_INACTIVE, FALSE,
			NULL);
	}
}

/*************************************************************/
void
set_window_size_and_loc(win_size)
int       win_size;
{
	void setup_edit_scrollbars();

	int min_dim;
	Rect srect, erect;

	min_dim = MIN(segal.cols, segal.rows);

#ifdef DEBUG
	fprintf(stderr, "zoom window size set to %d x %d\n", zoom_info.width, zoom_info.height);
#endif

	edit_info.canvas_width = MIN(zoom_info.width, MAX_ZOOM_WIDTH); 
	edit_info.canvas_height = MIN(zoom_info.height, MAX_ZOOM_HEIGHT);

	/* compute  window size */
	edit_info.win_height = edit_info.canvas_height
		+ EDIT_CONTROL_HEIGHT + 10;
	if (edit_info.canvas_width >= EDIT_CONTROL_WIDTH)
		edit_info.win_width = edit_info.canvas_width + 5;
	else
		edit_info.win_width = EDIT_CONTROL_WIDTH;

	/* set size */
	(void) xv_set(edit_win->canvas,
		XV_WIDTH, edit_info.canvas_width,
		XV_HEIGHT, edit_info.canvas_height,
		CANVAS_WIDTH, zoom_info.width,
		CANVAS_HEIGHT, zoom_info.height,
		CANVAS_AUTO_SHRINK, FALSE,
		CANVAS_AUTO_EXPAND, FALSE,
		NULL);
	(void) xv_set(edit_win->win,
		XV_WIDTH, edit_info.win_width,
		NULL);
	(void) xv_set(edit_win->win,
		XV_HEIGHT, edit_info.win_height,
		NULL);

	(void) xv_set(edit_win->win,
		XV_SHOW, TRUE,
		NULL);

	setup_edit_scrollbars();

	XFlush(display); /* a good idea right about here */
/*
 * set location: NOTE: to set the location of a frame using
 * 'frame_set_rect'  (at least under the twm window manager) XV_SHOW must
 * be TRUE first
 */

	/* set edit window location relative to segal window */
	frame_get_rect(segal_win->win, &srect);

	if (srect.r_top > 0 && srect.r_left > 0) { /* segal window must be
						    * mapped */
		frame_get_rect(edit_win->win, &erect);
		erect.r_top = srect.r_top + srect.r_height;
		erect.r_left = srect.r_left;
		frame_set_rect(edit_win->win, &erect);
	}
#ifdef DEBUG
	fprintf(stderr, " segal frame at: %d,%d \n", srect.r_top, srect.r_left);
	fprintf(stderr, " setting edit frame to: %d,%d : size: %d, %d\n",
	erect.r_top, erect.r_left, erect.r_width, erect.r_height);
#endif
}

/****************************************************************/
void
edit_repaint_proc()
{
    if ((int) xv_get(edit_win->win, XV_SHOW, NULL) == FALSE)
	return;

#ifdef DEBUG
    fputs("pixedit: edit_repaint_proc\n", stderr);
#endif

    if (segal.display_type == 0 && zoom_image != NULL)
	XPutImage(display, edit_xid, gc, zoom_image, 0, 0, 0, 0,
		  zoom_image->width, zoom_image->height);
    if (segal.display_type == 1 && zoom_mask_image != NULL)
	XPutImage(display, edit_xid, gc, zoom_mask_image, 0, 0, 0, 0,
		  zoom_mask_image->width, zoom_mask_image->height);
    if (segal.display_type == 2 && zoom_blend_image != NULL)
	XPutImage(display, edit_xid, gc, zoom_blend_image, 0, 0, 0, 0,
		  zoom_blend_image->width, zoom_blend_image->height);
}

/**************************************************************/
Notify_value
edit_event_proc(win, event, arg, type)
    Xv_Window win;
    Event    *event;
    Notify_arg arg;
    Notify_event_type type;
{
    void save_undo();
    void draw_location(), paint();
    void apply_mask_log();

    int       x, y;

/*
mouse events:
	any button: paint
	shift, any button  : show gray value
	ctrl, middle button: sets gradient according to highest gradient between
			any adjacent pixels encountered (from push to release).
	ctrl, left button  : sets the growth lower threshold slider bar.
	ctrl, right button : sets the growth upper threshold slider bar.
	meta, any button   : show object statistics
*/

#ifdef DEBUG
    fprintf(stderr, "event id is %d \n", event_id(event));
#endif

    /* meta events */
    if (event_meta_is_down(event)) {
	if (event_is_down(event)) {	/* down events only */
	    switch (event_id(event)) {
	    case MS_RIGHT:
	    case MS_LEFT:
	    case MS_MIDDLE:
		x = zoom_info.x + (event_x(event) / zoom_info.mag);
		y = zoom_info.y + (event_y(event) / zoom_info.mag);
		get_stats(x, y);
		break;
	    }
	}
	return notify_next_event_func(win, (Notify_event) event, arg, type);

    }
    /* shift events */
    if (event_shift_is_down(event)) {
	switch (event_id(event)) {
	case MS_RIGHT:
	case MS_LEFT:
	case MS_MIDDLE:
	case LOC_DRAG:
	    x = zoom_info.x + (event_x(event) / zoom_info.mag);
	    y = zoom_info.y + (event_y(event) / zoom_info.mag);
	    if (event_is_down(event))
		draw_location(x, y, 1);	/* routine in view.c */
	    else
		draw_location(x, y, 0);
	    break;
	}
	return notify_next_event_func(win, (Notify_event) event, arg, type);
    }


    /* ctrl events */
    if (event_ctrl_is_down(event)) {
	int value;

	x = zoom_info.x + (event_x(event) / zoom_info.mag);
	y = zoom_info.y + (event_y(event) / zoom_info.mag);
	/* CHANGE
	if (x >= segal.cols || y >= segal.rows || x < 0 || y < 0)
		break;
	*/
	value = himage.data[y][x];

	switch (event_id(event)) {
	static int which_button;

	case MS_LEFT:
		which_button = MS_LEFT;
		xv_set(mask_control_win->grow_threshold_low,
			PANEL_VALUE, value,
			NULL);
		set_grow_threshold_low(NULL, value, NULL);
		break;

	case MS_MIDDLE:
		which_button = MS_MIDDLE;
		grow.gradient = gradient(x, y, grow.matrix_width, 2 * grow.matrix_type);
		xv_set(mask_control_win->grow_gradient,
			PANEL_VALUE, grow.gradient,
			NULL);
		break;

	case MS_RIGHT:
		which_button = MS_RIGHT;
		xv_set(mask_control_win->grow_threshold_high,
			PANEL_VALUE, value,
			NULL);
		set_grow_threshold_high(NULL, value, NULL);
		break;

	case LOC_DRAG:
	    x = zoom_info.x + (event_x(event) / zoom_info.mag);
	    y = zoom_info.y + (event_y(event) / zoom_info.mag);
	    if (event_is_down(event)) switch(which_button) {
	    case MS_LEFT:
		xv_set(mask_control_win->grow_threshold_low,
			PANEL_VALUE, himage.data[y][x],
			NULL);
		set_grow_threshold_low(NULL, value, NULL);
		break;
	    case MS_MIDDLE:
		grow.gradient = gradient(x, y, grow.matrix_width, 2 * grow.matrix_type);
		xv_set(mask_control_win->grow_gradient,
			PANEL_VALUE, grow.gradient,
			NULL);
		break;
	    case MS_RIGHT:
		xv_set(mask_control_win->grow_threshold_high,
			PANEL_VALUE, himage.data[y][x],
			NULL);
		set_grow_threshold_high(NULL, value, NULL);
		break;
	    }
	    break;
	}
	return notify_next_event_func(win, (Notify_event) event, arg, type);
    }

    /* other events */
    switch (event_id(event)) {
    case LOC_WINENTER:
	/* slow down the mouse for painting */
	/* CHANGE
	XChangePointerControl(display, True, True, 2, 3, 2);	/* 2/3 speed */
	break;
    case LOC_WINEXIT:
	/* CHANGE
	XChangePointerControl(display, True, True, 3, 1, 1);
	*/
	break;
    case MS_RIGHT:
    case MS_LEFT:
    case MS_MIDDLE:
	if(event_is_down(event)) {
		save_undo();
		paint(event_x(event), event_y(event));
	}
	else if(event_is_up(event))  /* Release of the button */
		apply_mask_log();
	break;
    case LOC_DRAG:
	if (event_is_down(event))
	    paint(event_x(event), event_y(event));
	else if(event_is_up(event))  /* Release of the button */
		apply_mask_log();
	break;
    default:
	break;
    }

    return notify_next_event_func(win, (Notify_event) event, arg, type);
}

/******************************************************/
void
paint(xpos, ypos)		/* determine if ok to paint and what kind of
				 * painting */
int       xpos, ypos;
{
	void      draw_points();

	if (xpos < 0)
		xpos = 0;
	if (ypos < 0)
		ypos = 0;
	if (xpos >= zoom_info.width)
		xpos = zoom_info.width - 1;
	if (ypos >= zoom_info.height)
		ypos = zoom_info.height - 1;

	if(xv_get(edit_win->brush_mode, PANEL_VALUE, NULL) == 0) {
		/* brush mode = mask */
		segal.changed = 1; /* mask modified flag */
		if (((int) xv_get(edit_win->mask_brush_mode,
			PANEL_VALUE, NULL)) == 0)
			draw_points(xpos, ypos, PVAL);
		else
			draw_points(xpos, ypos, 0);
	}
	else { /* brush mode = image */
		himage.changed = 1; /* image modified flag */
		draw_points(xpos, ypos, 0); /* val doesn't matter */
	}
}

/**************************************************************/
void
draw_points(x, y, val)
    int       x, y, val;
{
    void      paint_zoom_image(), paint_main_image();
    void      draw_filenames();
    int       brush_size, paint_size, cnum, round_curs;

    draw_filenames();

#ifdef DEBUG
    fprintf(stderr, " painting from point: %d,%d \n", x, y);
#endif

    XSetFunction(display, gc, GXcopy);

    brush_size = ((int) xv_get(edit_win->brush_size, PANEL_VALUE, NULL)) + 1;
    round_curs = ((int) xv_get(edit_win->brush_type, PANEL_VALUE, NULL));

    cnum = brush_size * zoom_info.mag;
    if (my_cursor[cnum].size <= 0) {
	fprintf(stderr, " cursor structure error \n");
	return;
    }
    if (brush_size == 7)	/* last brush size setting is 20 */
	brush_size = 20;
    paint_size = brush_size * zoom_info.mag;

    paint_zoom_image(x, y, val, paint_size, cnum, round_curs);

    paint_main_image(x, y, val, brush_size, brush_size, round_curs);
}

/**********************************************************/
void
paint_zoom_image(x, y, val,paint_size, cnum, rcurs)
int       x, y, val;
int       paint_size, cnum, rcurs;
{
/* Paints either the image or the mask in the zoom (pixedit) window */

	void map_region_to_lut();

	register int i, j, i2, j2;
	int       is = 0, js = 0, rx, ry;
	int mask_apply, amount, new_color;
	u_long    paint_pixel, blend_pixel, get_blend_pixel();
	u_char  **pmask, **pimage;

	if (val == 0)
		paint_pixel = colors[0];
	else
		paint_pixel = colors[NGRAY - 1];
	XSetForeground(display, gc, paint_pixel);

	if (rcurs) {		/* round cursor, uses mask to determine paint
				 * locations */
		pmask = my_cursor[cnum].paint_mask_matrix;
		pimage = my_cursor[cnum].paint_image_matrix;
		is = js = my_cursor[cnum].corner;
		paint_size++;
		x -= paint_size / 2. + .5;
		y -= paint_size / 2. + .5;
		if (y < 0)
			y = 0;
		if (x < 0)
			x = 0;
#ifdef DEBUG
fprintf(stderr, " paint size: %d;  cursor size: %d \n",
paint_size, my_cursor[cnum].size);
fprintf(stderr, "  x: %d, y: %d, i2: %d, j2: %d \n", x, y, i2, j2);
for (i = is; i < is + paint_size; i++) {
fprintf(stderr, " \nMask matrix =\n");
for (j = js; j < js + paint_size; j++) {
fprintf(stderr, "%d ", pmask[i][j]);
fprintf(stderr, " \nImage matrix =\n");
for (j = js; j < js + paint_size; j++) {
fprintf(stderr, "%d ", pimage[i][j]);
}
}
fprintf(stderr, "\n");
#endif
	}
	i2 = is;			/* for handling round cursors */
	j2 = js;

	mask_apply = (xv_get(edit_win->brush_mode, PANEL_VALUE, NULL) == 0);
	amount = xv_get(edit_win->image_brush_delta, PANEL_VALUE, NULL) * NGRAY / 256.;
	if(xv_get(edit_win->image_brush_mode, PANEL_VALUE, NULL) == 0)
		amount *= -1; /* subtractive */

	for (i = y; i < y + paint_size && i < zoom_image->height; i++) {
	for (j = x; j < x + paint_size && j < zoom_image->width; j++) {
	if(mask_apply && ((rcurs && pmask[i2][j2]) > 0 || !rcurs)) {
		rx = zoom_info.x + (j / zoom_info.mag);
		ry = zoom_info.y + (i / zoom_info.mag);
		XPutPixel(zoom_mask_image, j, i, paint_pixel);
		if (himage.fp != NULL) {
			blend_pixel = get_blend_pixel(rx, ry, val);
			XPutPixel(zoom_blend_image, j, i, blend_pixel);
		}
		if (segal.display_type == 2)
			XSetForeground(display, gc, blend_pixel);
		else
			XSetForeground(display, gc, paint_pixel);
		XDrawPoint(display, edit_xid, gc, j, i);
	}
	else if(!mask_apply) {
		rx = zoom_info.x + (j / zoom_info.mag);
		ry = zoom_info.y + (i / zoom_info.mag);
		new_color = himage.data[ry][rx] + amount;

		if(new_color < 0) new_color = 0;
		else if(new_color > 255) new_color = 255;

		himage.data[ry][rx] = new_color;

		paint_pixel = colors[gray_lut[new_color]];
		XPutPixel(zoom_image, j, i, paint_pixel);

		blend_pixel = get_blend_pixel(rx, ry, val);
		XPutPixel(zoom_blend_image, j, i, blend_pixel);

		if(segal.display_type == 2) {
			if(work_buf[ry][rx] == PVAL)
				XSetForeground(display, gc, blend_pixel);
			else XSetForeground(display, gc, paint_pixel);
		}
		else if(segal.display_type == 0)
			XSetForeground(display, gc, paint_pixel);
		XDrawPoint(display, edit_xid, gc, j, i);
	} /* if */
	j2++;
	} /* for j */
	j2 = js;
	i2++;
	} /* for i */
	map_region_to_lut(himage.data, image->data, x/zoom_info.mag, y/zoom_info.mag, paint_size, paint_size);
}

/*******************************************************/
void
paint_main_image(x, y, val, paint_size, cnum, rcurs)
    int       x, y, val;
    int       paint_size, cnum, rcurs;

 /*
  * Implementation note: it would be faster to just do the blend once in the
  * view image, then replicate to the zoom image. However, magnify the
  * pixelization effects with the round cursors.  -BT
  */
{
    register int i, j, i2, j2;
    int is = 0, js = 0, rx, ry;
    int mask_apply;
    u_long paint_pixel, blend_pixel, get_blend_pixel();
    u_char **pmask;

    if (val == 0)
	paint_pixel = colors[0];
    else
	paint_pixel = colors[NGRAY - 1];

    if (rcurs) {		/* round cursor, uses mask to determine paint
				 * locations */
	pmask = my_cursor[cnum].paint_mask_matrix;
	is = js = my_cursor[cnum].corner;
	paint_size++;
	rx = zoom_info.x + (((float) x / zoom_info.mag) -
			    ((float) paint_size / 2.) + .5);
	ry = zoom_info.y + (((float) y / zoom_info.mag) -
			    ((float) paint_size / 2.) + .5);
	if (rx < 0)
	    rx = 0;
	if (ry < 0)
	    ry = 0;
#ifdef DEBUG
	fprintf(stderr, " cursor size: %d \n", my_cursor[cnum].size);
	fprintf(stderr, "  rx: %d, ry: %d, i2: %d, j2: %d \n", rx, ry, i2, j2);
	for (i = is; i < is + paint_size; i++) {
	    fprintf(stderr, " \n");
	    for (j = js; j < js + paint_size; j++) {
		fprintf(stderr, "%d ", pmask[i][j]);
	    }
	}
	fprintf(stderr, "\n");
#endif
    } else {
	rx = zoom_info.x + ((float) (x / zoom_info.mag) + .5);
	ry = zoom_info.y + ((float) (y / zoom_info.mag) + .5);
    }
	
	mask_apply = (xv_get(edit_win->brush_mode, PANEL_VALUE, NULL) == 0);

    i2 = is;			/* for handling round cursors */
    j2 = js;
    for (i = ry; i < ry + paint_size && i < segal.rows; i++) {
	for (j = rx; j < rx + paint_size && j < segal.cols; j++) {
	    if(mask_apply && ((rcurs && pmask[i2][j2] > 0) || !rcurs)) {
		work_buf[i][j] = val;
		if (himage.fp != NULL) {
		    blend_pixel = get_blend_pixel(j, i, val);
		    XPutPixel(blend_image, j, i, blend_pixel);
		}
		if (segal.display_type == 2 && himage.fp != NULL) {
		    XSetForeground(display, gc, blend_pixel);
		    XDrawPoint(display, view_xid, gc, j, i);
		} else {
		    XSetForeground(display, gc, paint_pixel);
		    XDrawPoint(display, view_xid, gc, j, i);
		}
		XSetForeground(display, gc, paint_pixel);
		XPutPixel(mask_image, j, i, paint_pixel);
	    }
	    else if(!mask_apply) {
		paint_pixel = colors[gray_lut[himage.data[i][j]]];
		XPutPixel(image, j, i, paint_pixel);

		blend_pixel = get_blend_pixel(j, i, val);
		XPutPixel(blend_image, j, i, blend_pixel);

		if(segal.display_type == 2 && work_buf[i][j] == PVAL)
			    paint_pixel = blend_pixel;

		XSetForeground(display, gc, paint_pixel);
		XDrawPoint(display, view_xid, gc, j, i);
	    }
	    j2++;
	}
	j2 = js;
	i2++;
    }
}

/***************************************************************/
void
original_proc(item, event)
Panel_item item;
Event    *event;
{
	void      edit_repaint_proc(), blend(), image_repaint_proc();
	void      draw_filenames();

	if(!segal.changed && !himage.changed) return;
	/* Not necessary to do any of this */

	set_watch_cursor();

	if (segal.changed) {
		bcopy((char *) m[segal.edit_m].data[0], (char *) work_buf[0],
			segal.rows * segal.cols);
		segal.changed = 0;
	}
	if(himage.changed) {
		bcopy((char *) image_buf[0], (char *) himage.data[0],
			segal.rows * segal.cols);
		himage.changed = 0;
	}
	draw_filenames();

	if (himage.fp != NULL) {
		blend(himage.data[0], work_buf[0], (u_char *) blend_image->data,
			segal.rows * segal.cols);
	}
	map_image_to_lut(work_buf[0], mask_image->data,
		mask_image->width * mask_image->height);

	image_repaint_proc();

	zoom();

	edit_repaint_proc();

	unset_watch_cursor();
}

/***************************************************************/
void
undo_proc(item, event)
Panel_item item;
Event    *event;
{
	void      edit_repaint_proc(), blend(), image_repaint_proc();
	void      draw_filenames();

	set_watch_cursor();
	bcopy((char *) work_undo_buf[0], (char *) work_buf[0],
		segal.rows * segal.cols);

	if(himage.changed) {
		bcopy((char *) image_undo_buf[0], (char *) himage.data[0],
			segal.rows * segal.cols);
	}

	draw_filenames();

	if (himage.fp != NULL) {
		blend(himage.data[0], work_buf[0], (u_char *) blend_image->data,
			segal.rows * segal.cols);
	}
	map_image_to_lut(work_buf[0], mask_image->data,
		mask_image->width * mask_image->height);

	image_repaint_proc();

	zoom();

	edit_repaint_proc();

	unset_watch_cursor();
}

/****************************************************************/
void
mag_proc(item, event)		/* procedure called when change the
				 * magnification */
    Panel_item item;
    Event    *event;
{
    static int last_mag = -1;
    void call_zoom();

    zoom_info.mag = ((int) xv_get(edit_win->zoom_mag, PANEL_VALUE, 0)) + 1;
    if (zoom_info.mag == last_mag)
	return;
    last_mag = zoom_info.mag;

    call_zoom();
}

/*********************************************************/
void
pixedit_resize_proc()
{				/* DOESNT WORK !! */
    /*
     * Note: this will get called many time when we dont want it to be
     * called, so all the 'ifs' at the beginning are to try to screen out
     * unwanted calls
     */

    /* This not yet working correctly: try to fix later */

    static int sw = 0, sh = 0;
    int       win_size;
    Rect      rect;

    if ((int) xv_get(edit_win->win, XV_SHOW, NULL) == FALSE)
	return;

    frame_get_rect(edit_win->win, &rect);

    if (sw == 0 && sh == 0) {
	sw = rect.r_width;
	sh = rect.r_height;
	return;
    }
    /* make sure changes at least 20 pixels */
    if ((sw > rect.r_width - 20 && sw < rect.r_width + 20) ||
	(sh > rect.r_height && sh < rect.r_height + 20))
	return;

    if (edit_info.changing == 1 || rect.r_width == 0 || rect.r_height == 0) {
	edit_info.changing = 0;
	return;			/* ignore resize event */
    }
    if (verbose)
	fprintf(stderr, "window resized \n");

    zoom_info.mag = ((int) xv_get(edit_win->zoom_mag, PANEL_VALUE, 0)) + 1;

    win_size = MIN(MAX(rect.r_width, EDIT_CONTROL_WIDTH),
		   (rect.r_height - EDIT_CONTROL_HEIGHT));
    if (verbose)
	fprintf(stderr, " win_size set to: %d \n", win_size);

    set_watch_cursor();
    set_window_size_and_loc(win_size);
    create_zoom_ximages();

    zoom();

    edit_repaint_proc();

    image_repaint_proc();
    unset_watch_cursor();

    sw = rect.r_width;
    sh = rect.r_height;

}
