
/*
 *  view.c            -Brian Tierney,  LBL
 *  for use with segal
 *  these routines are for displaying the image
 */

#include "common.h"

#include "view_ui.h"
view_win_objects *view_win;

#include "display_control.h"
#include "mask_control.h"
#include "pixedit.h"
#include "segal.h"
#include "threshold.h"

#define MAX_VIEW_WIDTH 800
#define MAX_VIEW_HEIGHT 800
/* #define DEBUG */
/* #define SHOW_COLORS  */

typedef struct box {
    int      x, y;   /* location of upper corner of the box */
    int      size;   /* size of box */
}         BOX_INFO;

BOX_INFO box_info;


/************************************************************/
void
view_win_init(owner)
    Xv_opaque owner;
{
    void      make_cms_colormap();

    view_win = view_win_objects_initialize(NULL, owner);

    make_cms_colormap(GREEN_HUE);
    (void) xv_set(view_win->canvas, WIN_CMS_NAME, "segal", NULL);
    if (verbose)
	fprintf(stderr, "setting colors.. \n");
    (void) xv_set(view_win->canvas, WIN_CMS_DATA, &cms_data, NULL);

#ifdef SHOW_COLORS
    fprintf(stderr, "after canvas creation \n");
    for (i = 0; i < NGRAY * 2; i++)
	fprintf(stderr, "color table: red = %d, green = %d, blue = %d \n",
		cms_data.red[i], cms_data.green[i], cms_data.blue[i]);
#endif

    /* set pointers to colormap */
    colors = (u_long *) xv_get(view_win->canvas, WIN_X_COLOR_INDICES);
    red_standout = colors[RED_STANDOUT];
    green_standout = colors[GREEN_STANDOUT];
    blue_standout = colors[BLUE_STANDOUT];
    yellow_standout = colors[YELLOW_STANDOUT];

#ifdef SHOW_COLORS
    for (i = 0; i < NGRAY * 2; i++)
	fprintf(stderr, " color %d = %d \n", i, colors[i]);
#endif
}

/**************************************************************/
void
view_setup()
{
    void setup_view_scrollbars();

    Rect      srect, vrect;	/* frame rect structure for segal win and
				 * view win */

	segal.width = MIN(segal.cols + 1, MAX_VIEW_WIDTH);
	segal.height = MIN(segal.rows + 1, MAX_VIEW_HEIGHT);

    /* set size */
    (void) xv_set(view_win->canvas,
	XV_WIDTH, segal.width,
	XV_HEIGHT, segal.height,
	CANVAS_WIDTH, segal.cols + 1,
	CANVAS_HEIGHT, segal.rows + 1,
	CANVAS_AUTO_SHRINK, FALSE,
	CANVAS_AUTO_EXPAND, FALSE,
	NULL);

    (void) xv_set(view_win->win,
		XV_WIDTH, segal.width,
		XV_HEIGHT, segal.height,
		NULL);

    setup_view_scrollbars();

    if((int) xv_get(view_win->win, XV_SHOW, NULL) == TRUE)
	return;

    (void) xv_set(view_win->win, XV_SHOW, TRUE, NULL);

	XFlush(display); /* a good idea right about here */
    /*
     * set location: NOTE: to set the location of a frame using
     * 'frame_set_rect'  (at least under the twm window manager) XV_SHOW must
     * be TRUE first
     */

    /* set view window location relative to segal window */
    frame_get_rect(segal_win->win, &srect);

    if (srect.r_top > 0 && srect.r_left > 0) { /* only if segal window
						  is on screen */
	frame_get_rect(view_win->win, &vrect);
	vrect.r_top = srect.r_top;
	vrect.r_left = srect.r_left + srect.r_width + 2;
	frame_set_rect(view_win->win, &vrect);
    }
}

/*************************************************************
 * Repaint callback function for `canvas'.
 */
void
image_repaint_proc()
{
	void draw_crop_rectangle();

	if (xv_get(view_win->win, XV_SHOW, NULL) == FALSE)
		return;

	if (segal.display_type == 0 && image != NULL)
		XPutImage(display, view_xid, gc, image, 0, 0, 0, 0,
		  image->width, image->height);
	if (segal.display_type == 1 && mask_image != NULL)
		XPutImage(display, view_xid, gc, mask_image, 0, 0, 0, 0,
		  mask_image->width, mask_image->height);
	if (segal.display_type == 2 && blend_image != NULL)
		XPutImage(display, view_xid, gc, blend_image, 0, 0, 0, 0,
		  blend_image->width, blend_image->height);

	if(!region.refresh) {
		XSetFunction(display, gc, GXinvert);
		draw_crop_rectangle(region.crx1, region.cry1,
		region.crx2, region.cry2);
		XSetFunction(display, gc, GXcopy);
	}

	/* save info on where rectangle was drawn
	box_info.x = zoom_info.x;
	box_info.y = zoom_info.y;
	box_info.size = bsize;
	*/
}

/*****************************************************************/
void
image_subregion_repaint_proc()
{
    int sx, sy, bsize;

    if (xv_get(view_win->win, XV_SHOW, NULL) == FALSE)
	return;

    sx = box_info.x - 3;
    sy = box_info.y - 3;
    if ( sx < 0)
	sx = 0;
    if ( sy < 0)
	sy = 0;
    bsize = box_info.size + 6;

    if (segal.display_type == 0 && image != NULL)
	XPutImage(display, view_xid, gc, image, sx, sy, sx, sy, bsize, bsize);
    if (segal.display_type == 1 && mask_image != NULL)
	XPutImage(display, view_xid, gc, mask_image, sx, sy,
		  sx, sy, bsize, bsize);
    if (segal.display_type == 2 && blend_image != NULL)
	XPutImage(display, view_xid, gc, blend_image, sx, sy,
		  sx, sy, bsize, bsize);
}

/*****************************************************************/
Notify_value
disp_event_proc(win, event, arg, type)
Xv_Window win;
Event    *event;
Notify_arg arg;
Notify_event_type type;
{
	void draw_box();
	void draw_location();
	void draw_hint();
	void get_stats();
	void call_zoom();
	void draw_crop_rectangle();
	void draw_region_size_mesg();
	void refresh_histogram();
	void fix_rect_coords();
	void begin_or_end_drawing_line();
	void drag_out_line();

	static int counter = 0;

#ifdef DEBUG
    fprintf(stderr, "hview: disp_event_proc: event %d\n", event_id(event));
#endif

    /* if polygon mode */
    if (segal.poly_flag == 1) {
	if (event_is_down(event)) {   /* down events only */
	    switch (event_id(event)) {
	    case MS_RIGHT:
	    case MS_LEFT:
	    case MS_MIDDLE:
	    case LOC_DRAG:
		cross_proc(event);
		break;
	    }
	}
	    return notify_next_event_func(win, (Notify_event) event, 
					  arg, type);
    }
/*
  mouse events:
    any button:				draw crop rectangle 
    shift and control, any button:	show gray value 
    meta, any button:			show object statistics
*/

  /* meta events */
    if	(event_meta_is_down(event)) {
	if (event_is_down(event)) {   /* down events only */
	    switch (event_id(event)) {
	    case MS_RIGHT:
	    case MS_LEFT:
	    case MS_MIDDLE:
		get_stats(event_x(event), event_y(event));
		break;
	    }
 	}
	return notify_next_event_func(win, (Notify_event) event, arg, type);
    }

    /* cntrl and shift events :  display gray value at cursor */
    if	(event_ctrl_is_down(event) || event_shift_is_down(event)) {
	switch (event_id(event)) {
	case MS_RIGHT:
	case MS_LEFT:
	case MS_MIDDLE:
	case LOC_DRAG:
	    if (event_is_down(event))
		draw_location(event_x(event), event_y(event), 1);
	    else
		draw_location(event_x(event), event_y(event), 0);
	    break;
	}
	return notify_next_event_func(win, (Notify_event) event, arg, type);
    }

    /* other events */
    switch (event_id(event)) {
    static int dragging = 0;
    case MS_RIGHT:
    case MS_LEFT:
    case MS_MIDDLE:

	if(segal.reg_flag) {
		begin_or_end_drawing_line(win, event, VIEW_WIN, &image_line);
	 }
	else {
	if(event_is_down(event)) { /* Beginning of cropping a region */
		/* Set gc logical function to invert -- keeps orig data */
		XSetFunction(display, gc, GXinvert);

		/* erase old crop rectangle (if there is one) */
		if(!region.refresh)
			draw_crop_rectangle(region.crx1, region.cry1,
				region.crx2, region.cry2);
		else region.refresh = 0;


		region.crx1 = event_x(event);
		region.cry1 = event_y(event);

		/* originally, (crx2,cry2) is at the origin too */
		region.crx2 = region.crx1;
		region.cry2 = region.cry1;

		draw_region_size_mesg();
	}
	else if(event_is_up(event)) { /* Release of the button */

		/* Put crx1,cry1 in the upper left corner and
		 * put crx2,cry2 in the lower right corner */
		fix_rect_coords();

		/* bind crop rectangle within the bounds of the image */
		if(region.crx2 > segal.cols)	region.crx2 = segal.cols;
		else if(region.crx2 < 0)	region.crx2 = 0;
		if(region.cry2 > segal.rows)	region.cry2 = segal.rows;
		else if(region.cry2 < 0)	region.cry2 = 0;

		/* set region.rows & region.cols */
		region.rows = region.cry2 - region.cry1;
		region.cols = region.crx2 - region.crx1;

		/* Set gc logical function back to 'copy' */
		XSetFunction(display, gc, GXcopy);

		if(xv_get(threshold_win->win, XV_SHOW, NULL))
			refresh_histogram();

		draw_hint("Plant seed pts. or paint by hand");

		/* iff a cropped rectangle is defined should we zoom */
		if(region.rows != 0 && region.cols != 0) {
			(void) xv_set(segal_win->but_edit,
				PANEL_INACTIVE, FALSE,
				NULL);
			call_zoom();
		}
	}
	}
	break;

    case LOC_DRAG:
	if(segal.reg_flag)
		drag_out_line(win, event, VIEW_WIN, &image_line);
	else {
		/* erase old crop rectangle */
		draw_crop_rectangle(region.crx1, region.cry1,
			region.crx2, region.cry2);

		region.crx2 = event_x(event);
		region.cry2 = event_y(event);

		/* draw new crop rectangle */
		draw_crop_rectangle(region.crx1, region.cry1,
			region.crx2, region.cry2);
	}
	break;

    default:
	break;
    }
    return notify_next_event_func(win, (Notify_event) event, arg, type);
}

/**************************************************************/
void
fix_rect_coords()
{
/* Called at the end of the dragging out of a crop rectangle */
	int x1, y1, x2, y2;

	if(region.crx1 <= region.crx2) {
		x1 = region.crx1;
		x2 = region.crx2;
	}
	else {
		x1 = region.crx2;
		x2 = region.crx1;
	}

	if(region.cry1 <= region.cry2) {
		y1 = region.cry1;
		y2 = region.cry2;
	}
	else {
		y1 = region.cry2;
		y2 = region.cry1;
	}

	region.crx1 = x1;
	region.crx2 = x2;
	region.cry1 = y1;
	region.cry2 = y2;
}

/**************************************************************/
void
draw_crop_rectangle(x1, y1, x2, y2)
int x1, y1, x2, y2;
{
	int draw_rect();
	void draw_region_size_mesg();

	draw_rect(x1, y1, x2, y2);
	draw_region_size_mesg();
}

/**************************************************************/
void
draw_region_size_mesg()
{
	char segal_mesg[40];

	if(region.crx1 <= region.crx2) region.cols = region.crx2 - region.crx1;
	else region.cols = region.crx1 - region.crx2;
	if(region.cry1 <= region.cry2) region.rows = region.cry2 - region.cry1;
	else region.rows = region.cry1 - region.cry2;

	if(region.rows == 0 || region.cols == 0) {
		sprintf(segal_mesg, "Edit Region Size: Undefined");
		(void) xv_set(segal_win->but_edit,
			PANEL_INACTIVE, TRUE,
			NULL);
		(void) xv_set(mask_control_win->set_portion,
			PANEL_VALUE, region.region,
			PANEL_INACTIVE, TRUE,
			NULL);
	}
	else {
		sprintf(segal_mesg, "Edit Region Size: %dr x %dc",
			region.rows, region.cols);
		(void) xv_set(mask_control_win->set_portion,
			PANEL_VALUE, region.region,
			PANEL_INACTIVE, FALSE,
			NULL);
	}

	(void) xv_set(segal_win->msg_pixedit,
		PANEL_LABEL_STRING, segal_mesg,
		NULL);
}

/***********************************/
int draw_rect(x, y, x1, y1)
int x, y, x1, y1;
{
/* returns 0 if it didn't draw anything (rect is too small), 1 if it did */
	int w, h;

	w = abs(x - x1);  h = abs(y - y1);
	if (x > x1) x = x1;
	if (y > y1) y = y1;

	if (w < 4 || h < 4) return 0;   /* too small */

	/* keep rectangle inside window */
	if (x < 0) { w += x; x = 0; }
	if (y < 0) { h += y; y = 0; }
	if (x + w > segal.cols) w = segal.cols - x;
	if (y + h > segal.rows) h = segal.rows - y;

	XDrawRectangle(display, view_xid, gc, x, y, w-1, h-1);
	XDrawRectangle(display, view_xid, gc, x+1, y+1, w-3, h-3);
	return 1;
}

/**************************************************************/
void
refresh_display()
{
	void build_ximages();
	void image_repaint_proc();

	fprintf(stderr, "Refreshing Display ...\n");
	build_ximages();
	image_repaint_proc();
}

/**************************************************************/
void
call_zoom()   /* set up information for zoom */
{
	void      set_window_size_and_loc();
	void      zoom();
	int       cx, cy, dist_to_center;

	if (xv_get(edit_win->win, XV_SHOW, NULL) == FALSE)
		return;

	set_watch_cursor();

        if(!region.refresh) {
		zoom_info.x = region.crx1;
		zoom_info.y = region.cry1;
	}
	else {
		zoom_info.x = 0;
		zoom_info.y = 0;
	}
	zoom_info.width = region.cols * zoom_info.mag;
	zoom_info.height = region.rows * zoom_info.mag;

        image_repaint_proc(); /* necessary? */
	set_window_size_and_loc();
	create_zoom_ximages();
	zoom();
	edit_repaint_proc();

	unset_watch_cursor();
}

/******************************************************/
void
draw_location(x, y, funct)
    int       x, y, funct;
{
    char      dstr[20], pvalstr[40];
    int       width, height;

    if (x >= segal.cols || y >= segal.rows || x < 0 || y < 0)
	return;

    /* will have problems if default font changes */
    height = 16;

    if (funct) {
	sprintf(dstr, "(%d,%d) = %d ", x, y, himage.data[y][x]);
	sprintf(pvalstr, "Pixel Value: %s", dstr);
	width = 7 * strlen(dstr);

	XSetForeground(display, gc, WhitePixel(display, screen));
	XClearArea(display, view_xid, 0, 0, width, height, False);
	XDrawString(display, view_xid, gc, 2, height - 4, dstr, strlen(dstr));
    } else {
	sprintf(pvalstr, "Pixel Value:");
	if (segal.display_type == 0)
	    XPutImage(display, view_xid, gc, image, 0, 0, 0, 0, 140, height);
	else if (segal.display_type == 1)
	    XPutImage(display, view_xid, gc, mask_image,
		      0, 0, 0, 0, 140, height);
	else
	    XPutImage(display, view_xid, gc, blend_image,
		      0, 0, 0, 0, 140, height);
    }
	(void) xv_set(segal_win->msg_pixel_value,
		PANEL_LABEL_STRING, pvalstr,
		NULL);
	(void) xv_set(edit_win->msg_pixel_value,
		PANEL_LABEL_STRING, pvalstr,
		NULL);
}

/**************************************************************/
void
map_image_to_lut(buf, newbuf, bufsize)  
 /* used to map the image to the color look-up-table based
    on the value of NGRAY */

    u_char   *buf;		/* image */
    u_char   *newbuf;
    int       bufsize;
{
    register int       i;
    u_char    j;
    float     scale = (float) NGRAY/ 257.;

    for (i = 0; i < bufsize; i++) {
	/* scale to 0 to NGRAY */
	j = (u_char) (buf[i] * scale);
	if (j >= NGRAY)
	    j = NGRAY - 1;
	/* map to lookup table */
	newbuf[i] = (u_char) colors[j];
    }
}

/**************************************************************/
void
map_region_to_lut(buf, newbuf, x, y, width, height)  
/* used to map the image to the color look-up-table based
 * on the value of NGRAY */

u_char   **buf;		/* image */
u_char   *newbuf;
int      x, y, width, height;
{
	register int i, j;
	u_char q;
	float scale = (float) NGRAY/ 257.;

	for(j = y; j < y + height; j++)
	for(i = x; i < x + width; i++) {
		/* scale to 0 to NGRAY */
		q = (u_char) (buf[j][i] * scale);
		if (q >= NGRAY)
			q = NGRAY - 1;
		/* map to lookup table */
		newbuf[j*width + i] = (u_char) colors[q];
	}
}

/**************************************************************************/
void
make_cms_colormap(mask_hue)
int mask_hue;
{
    int       i, j;
    int       color_size = CMAP_SIZE;
    u_char    red[CMAP_SIZE], green[CMAP_SIZE], blue[CMAP_SIZE];
    double    gammaval;
    int       r, g, b;
    int       h = 0;		/* HSV values */
    double    s = 1., v = 1.;
    extern int overlay_hue;	/* command line arg */

    gammaval = (float) xv_get(display_control_win->gamma_val, PANEL_VALUE, NULL) / 10.;

    if (verbose)
	fprintf(stderr, "setting colormap: gamma value of %.1f \n", gammaval);

    for (i = 0; i < NGRAY; i++) {
	if (gammaval == 1.0)
	    j = (int) ((i * 257.) / (float) NGRAY);
	else
	    j = (int) (257. * pow((double) i / (float) NGRAY, 1.0 / gammaval));
	if (j > 255)
	    j = 255;
	red[i] = blue[i] = green[i] = (u_char) j;
    }

    for (; i < NGRAY * 2; i++) { /* blend colors */
	switch(mask_hue) {
	case RED_HUE :
		overlay_hue = 0;
		break;
	case GREEN_HUE :
		overlay_hue = 170;
		break;
	case BLUE_HUE :
		overlay_hue = 280;
		break;
	default : break;
	}

	h = overlay_hue;	/* greenish */
	s = .8;
	v = .5 + (.5 * (float) (i - NGRAY) / (float) NGRAY);	/* .5 to 1.0 */
	HSV_to_RGB((short) h, s, v, &r, &g, &b);
	red[i] = (u_char) r;
	blue[i] = (u_char) b;
	green[i] = (u_char) g;
    }

    /* GREEN_STANDOUT = lime green */
    red[GREEN_STANDOUT] = 50;
    green[GREEN_STANDOUT] = 204;
    blue[GREEN_STANDOUT] = 50;

    /* BLUE_STANDOUT = sky blue */
    red[BLUE_STANDOUT] = 50;
    green[BLUE_STANDOUT] = 153;
    blue[BLUE_STANDOUT] = 204;

    /* YELLOW_STANDOUT = yellow */
    red[YELLOW_STANDOUT] = green[YELLOW_STANDOUT] = 255;
    blue[YELLOW_STANDOUT] = 0;

    /* RED_STANDOUT = red */
    red[RED_STANDOUT] = 255;
    blue[RED_STANDOUT] = green[RED_STANDOUT] = 0;

    cms_data.type = XV_DYNAMIC_CMS;
    cms_data.size = color_size;
    cms_data.rgb_count = color_size;
    cms_data.index = 0;
    cms_data.red = red;
    cms_data.green = green;
    cms_data.blue = blue;

#ifdef SHOW_COLORS
    fprintf(stderr, "after setting colors... \n");
    for (i = 0; i < NGRAY * 2; i++)
	fprintf(stderr, "color table: red = %d, green = %d, blue = %d \n",
		cms_data.red[i], cms_data.green[i], cms_data.blue[i]);
#endif
}

/**************************************************/
void
change_colormap(item, value, event)  /* procedure called when adjusting contrast */
Panel_item item;
int value;
Event    *event;
{
	void      make_cms_colormap();

	if ((int) xv_get(view_win->win, XV_SHOW, NULL) == FALSE)
		return;

	make_cms_colormap(value);

	(void) xv_set(view_win->canvas, WIN_CMS_DATA, &cms_data, NULL);
	(void) xv_set(edit_win->canvas, WIN_CMS_DATA, &cms_data, NULL);
}

/*********************************************************/

HSV_to_RGB(hue, sat, val, red, green, blue)
/* written by David Robertson, LBL
 *
 * HSV_to_RGB - converts a color specified by hue, saturation and intensity
 * to r,g,b values.
 * Hue should be in the range 0-360 (0 is red); saturation and intensity
 * should both be in [0,1].
 */

    short     hue;
    double    sat, val;
    int      *red, *green, *blue;

{
    register double r, g, b;
    int       i;
    register double f, nhue;
    double    p, q, t;

    val *= 255.0;
    if (hue == 360)
	nhue = 0.0;
    else
	nhue = (double) hue / 60.0;
    i = (int) nhue;		/* Get integer part of nhue */
    f = nhue - (double) i;	/* And fractional part */
    p = val * (1.0 - sat);
    q = val * (1.0 - sat * f);
    t = val * (1.0 - sat * (1.0 - f));
    switch (i) {
    case 0:
	r = val;
	g = t;
	b = p;
	break;
    case 1:
	r = q;
	g = val;
	b = p;
	break;
    case 2:
	r = p;
	g = val;
	b = t;
	break;
    case 3:
	r = p;
	g = q;
	b = val;
	break;
    case 4:
	r = t;
	g = p;
	b = val;
	break;
    case 5:
	r = val;
	g = p;
	b = q;
    }

    *red = (int) (r + 0.5);
    *green = (int) (g + 0.5);
    *blue = (int) (b + 0.5);

}
