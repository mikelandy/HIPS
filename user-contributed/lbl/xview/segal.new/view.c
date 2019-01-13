/*
 *	view.c - for use with SEGAL
 *
 *	By Bryan Skene
 */

#include "common.h"

#define MAX_VCANV_WIDTH		256 
#define MAX_VCANV_HEIGHT	256

/************************************************************/
void
view_setup()
{
	int view_w, view_h, canv_w, canv_h, slid_x, slid_y,
		zoom_x, zoom_y, pref_x, pref_w;
	int max_zcanv_width, max_zcanv_height;

	/* set size and location */

	/***** Z Aspect *****/
	win[WIN_VZ].canv_x = xv_get(View_win->msg_image, XV_X, NULL) + 20;
	win[WIN_VZ].canv_y = xv_get(View_win->but_image, XV_Y, NULL) + 100;
	win[WIN_VZ].canv_w = win[WIN_VZ].img_c * win[WIN_VZ].zoom_mag;
	win[WIN_VZ].canv_h = win[WIN_VZ].img_r * win[WIN_VZ].zoom_mag;
	
	canv_w = win[WIN_VZ].canv_w;
	canv_h = win[WIN_VZ].canv_h;

	if(segal.disp_xy) {
		max_zcanv_width = MAX_VCANV_WIDTH;
		max_zcanv_height = MAX_VCANV_HEIGHT;
	}
	else {
		max_zcanv_width = 2 * MAX_VCANV_WIDTH;
		max_zcanv_height = 2 * MAX_VCANV_HEIGHT;
	}

	RANGE(win[WIN_VZ].canv_w, 0, max_zcanv_width)
	RANGE(win[WIN_VZ].canv_h, 0, max_zcanv_height)

	if(canv_w <= max_zcanv_width) xv_destroy_safe(win[WIN_VZ].h_sbar);
	if(canv_h <= max_zcanv_height) xv_destroy_safe(win[WIN_VZ].v_sbar);
	xv_set(View_win->canv_z,
		XV_SHOW, TRUE,
		XV_X, win[WIN_VZ].canv_x,
		XV_Y, win[WIN_VZ].canv_y,
		XV_WIDTH, win[WIN_VZ].canv_w,
		XV_HEIGHT, win[WIN_VZ].canv_h,
		CANVAS_WIDTH, canv_w,
		CANVAS_HEIGHT, canv_h,
		CANVAS_AUTO_SHRINK, FALSE,
		CANVAS_AUTO_EXPAND, FALSE,
		NULL);

	if(canv_w > max_zcanv_width)
	win[WIN_VZ].h_sbar = (Scrollbar) xv_create(View_win->canv_z, SCROLLBAR,
		SCROLLBAR_DIRECTION, SCROLLBAR_HORIZONTAL,
		SCROLLBAR_PIXELS_PER_UNIT, win[WIN_VZ].zoom_mag,
		SCROLLBAR_OBJECT_LENGTH, win[WIN_VZ].img_c,
		SCROLLBAR_PAGE_LENGTH, max_zcanv_width,
		SCROLLBAR_VIEW_LENGTH, max_zcanv_width,
		NULL);

	if(canv_h > max_zcanv_height)
	win[WIN_VZ].v_sbar = (Scrollbar) xv_create(View_win->canv_z, SCROLLBAR,
		SCROLLBAR_DIRECTION, SCROLLBAR_VERTICAL,
		SCROLLBAR_PIXELS_PER_UNIT, win[WIN_VZ].zoom_mag,
		SCROLLBAR_OBJECT_LENGTH, win[WIN_VZ].img_r,
		SCROLLBAR_PAGE_LENGTH, max_zcanv_height,
		SCROLLBAR_VIEW_LENGTH, max_zcanv_height,
		NULL);

	slid_x = win[WIN_VZ].canv_x + win[WIN_VZ].canv_w - 90;
	slid_y = win[WIN_VZ].canv_y - 30 + 4;
	zoom_x = win[WIN_VZ].canv_x;
	zoom_y = slid_y - 28;

	if(segal.r3d) {
		xv_set(View_win->set_frame_z,
			XV_SHOW, TRUE,
			XV_X, slid_x,
			XV_Y, slid_y,
			PANEL_VALUE, win[WIN_VZ].f,
			PANEL_MIN_VALUE, 0,
			PANEL_MAX_VALUE, segal.f - 1,
			PANEL_SLIDER_WIDTH, win[WIN_VZ].canv_h,
			NULL);
	}
	else {
		xv_set(View_win->set_frame_z,
			XV_SHOW, FALSE,
			NULL);
	}

	xv_set(View_win->set_zoom_z,
		XV_SHOW, TRUE,
		XV_X, zoom_x,
		XV_Y, zoom_y,
		NULL);

	/***** X Aspect *****/
	if(segal.disp_xy && segal.r3d) {
	win[WIN_VX].canv_w = win[WIN_VX].img_c * win[WIN_VX].zoom_mag;
	win[WIN_VX].canv_h = win[WIN_VX].img_r * win[WIN_VX].zoom_mag;
	
	canv_w = win[WIN_VX].canv_w;
	canv_h = win[WIN_VX].canv_h;

	RANGE(win[WIN_VX].canv_w, 0, MAX_VCANV_WIDTH)
	RANGE(win[WIN_VX].canv_h, 0, MAX_VCANV_HEIGHT)

	if(canv_w <= MAX_VCANV_WIDTH) xv_destroy_safe(win[WIN_VX].h_sbar);
	if(canv_h <= MAX_VCANV_HEIGHT) xv_destroy_safe(win[WIN_VX].v_sbar);

	zoom_x = xv_get(View_win->but_preferences, XV_X, NULL)
		+ xv_get(View_win->but_preferences, XV_WIDTH, NULL) + 20;
	win[WIN_VX].canv_x = zoom_x;
	slid_x = zoom_x + win[WIN_VX].canv_w - 90;

	zoom_y = xv_get(View_win->msg_image, XV_Y, NULL) - 4;
	slid_y = zoom_y + 25;
	win[WIN_VX].canv_y = slid_y + 25;

	xv_set(View_win->canv_x,
		XV_SHOW, TRUE,
		XV_X, win[WIN_VX].canv_x,
		XV_Y, win[WIN_VX].canv_y,
		XV_WIDTH, win[WIN_VX].canv_w,
		XV_HEIGHT, win[WIN_VX].canv_h,
		CANVAS_WIDTH, canv_w,
		CANVAS_HEIGHT, canv_h,
		CANVAS_AUTO_SHRINK, FALSE,
		CANVAS_AUTO_EXPAND, FALSE,
		NULL);

	if(canv_w > MAX_VCANV_WIDTH)
	win[WIN_VX].h_sbar = (Scrollbar) xv_create(View_win->canv_x, SCROLLBAR,
		SCROLLBAR_DIRECTION, SCROLLBAR_HORIZONTAL,
		SCROLLBAR_PIXELS_PER_UNIT, win[WIN_VX].zoom_mag,
		SCROLLBAR_OBJECT_LENGTH, win[WIN_VX].img_c,
		SCROLLBAR_PAGE_LENGTH, MAX_VCANV_WIDTH,
		SCROLLBAR_VIEW_LENGTH, MAX_VCANV_WIDTH,
		NULL);

	if(canv_h > MAX_VCANV_HEIGHT)
	win[WIN_VX].v_sbar = (Scrollbar) xv_create(View_win->canv_x, SCROLLBAR,
		SCROLLBAR_DIRECTION, SCROLLBAR_VERTICAL,
		SCROLLBAR_PIXELS_PER_UNIT, win[WIN_VX].zoom_mag,
		SCROLLBAR_OBJECT_LENGTH, win[WIN_VX].img_r,
		SCROLLBAR_PAGE_LENGTH, MAX_VCANV_HEIGHT,
		SCROLLBAR_VIEW_LENGTH, MAX_VCANV_HEIGHT,
		NULL);

	xv_set(View_win->set_frame_x,
		XV_SHOW, TRUE,
		XV_X, slid_x,
		XV_Y, slid_y,
		PANEL_VALUE, win[WIN_VX].f,
		PANEL_MIN_VALUE, 0,
		PANEL_MAX_VALUE, segal.c - 1,
		PANEL_SLIDER_WIDTH, win[WIN_VX].canv_h,
		NULL);

	xv_set(View_win->set_zoom_x,
		XV_SHOW, TRUE,
		XV_X, zoom_x,
		XV_Y, zoom_y,
		NULL);

	/***** Y Aspect *****/
	win[WIN_VY].canv_w = win[WIN_VY].img_c * win[WIN_VY].zoom_mag;
	win[WIN_VY].canv_h = win[WIN_VY].img_r * win[WIN_VY].zoom_mag;
	
	canv_w = win[WIN_VY].canv_w;
	canv_h = win[WIN_VY].canv_h;

	RANGE(win[WIN_VY].canv_w, 0, MAX_VCANV_WIDTH)
	RANGE(win[WIN_VY].canv_h, 0, MAX_VCANV_HEIGHT)

	if(canv_w <= MAX_VCANV_WIDTH) xv_destroy_safe(win[WIN_VY].h_sbar);
	if(canv_h <= MAX_VCANV_HEIGHT) xv_destroy_safe(win[WIN_VY].v_sbar);

	/* zoom_x does not change here */
	win[WIN_VY].canv_x = zoom_x;
	slid_x = zoom_x + win[WIN_VY].canv_w - 90;

	zoom_y = win[WIN_VX].canv_y + win[WIN_VX].canv_h + 30;
	slid_y = zoom_y + 25;
	win[WIN_VY].canv_y = slid_y + 25;

	xv_set(View_win->canv_y,
		XV_SHOW, TRUE,
		XV_X, win[WIN_VY].canv_x,
		XV_Y, win[WIN_VY].canv_y,
		XV_WIDTH, win[WIN_VY].canv_w,
		XV_HEIGHT, win[WIN_VY].canv_h,
		CANVAS_WIDTH, canv_w,
		CANVAS_HEIGHT, canv_h,
		CANVAS_AUTO_SHRINK, FALSE,
		CANVAS_AUTO_EXPAND, FALSE,
		NULL);

	if(canv_w > MAX_VCANV_WIDTH)
	win[WIN_VY].h_sbar = (Scrollbar) xv_create(View_win->canv_y, SCROLLBAR,
		SCROLLBAR_DIRECTION, SCROLLBAR_HORIZONTAL,
		SCROLLBAR_PIXELS_PER_UNIT, win[WIN_VY].zoom_mag,
		SCROLLBAR_OBJECT_LENGTH, win[WIN_VY].img_c,
		SCROLLBAR_PAGE_LENGTH, MAX_VCANV_WIDTH,
		SCROLLBAR_VIEW_LENGTH, MAX_VCANV_WIDTH,
		NULL);

	if(canv_h > MAX_VCANV_HEIGHT)
	win[WIN_VY].v_sbar = (Scrollbar) xv_create(View_win->canv_y, SCROLLBAR,
		SCROLLBAR_DIRECTION, SCROLLBAR_VERTICAL,
		SCROLLBAR_PIXELS_PER_UNIT, win[WIN_VY].zoom_mag,
		SCROLLBAR_OBJECT_LENGTH, win[WIN_VY].img_r,
		SCROLLBAR_PAGE_LENGTH, MAX_VCANV_HEIGHT,
		SCROLLBAR_VIEW_LENGTH, MAX_VCANV_HEIGHT,
		NULL);

	xv_set(View_win->set_frame_y,
		XV_SHOW, TRUE,
		XV_X, slid_x,
		XV_Y, slid_y,
		PANEL_VALUE, win[WIN_VY].f,
		PANEL_MIN_VALUE, 0,
		PANEL_MAX_VALUE, segal.r - 1,
		PANEL_SLIDER_WIDTH, win[WIN_VY].canv_h,
		NULL);

	xv_set(View_win->set_zoom_y,
		XV_SHOW, TRUE,
		XV_X, zoom_x,
		XV_Y, zoom_y,
		NULL);
	} /* display X & Y && this is a 3d image */
	else {
	/* make X & Y canvases, messages, etc. invisible */
		xv_set(View_win->canv_x,
			XV_SHOW, FALSE,
			NULL);
		xv_set(View_win->set_frame_x,
			XV_SHOW, FALSE,
			NULL);
		xv_set(View_win->set_zoom_x,
			XV_SHOW, FALSE,
			NULL);
		xv_set(View_win->canv_y,
			XV_SHOW, FALSE,
			NULL);
		xv_set(View_win->set_frame_y,
			XV_SHOW, FALSE,
			NULL);
		xv_set(View_win->set_zoom_y,
			XV_SHOW, FALSE,
			NULL);
	}

	/***** View Window *****/
	if(segal.disp_xy) {
		view_w = MAX(win[WIN_VX].canv_x + win[WIN_VX].canv_w, win[WIN_VY].canv_x + win[WIN_VY].canv_w);
		view_h = MAX(win[WIN_VY].canv_y + win[WIN_VY].canv_h, win[WIN_VZ].canv_y + win[WIN_VZ].canv_h);
	}
	else {
		pref_x = xv_get(View_win->but_preferences, XV_X, NULL);
		pref_w = xv_get(View_win->but_preferences, XV_WIDTH, NULL);

		view_w = MAX(win[WIN_VZ].canv_x + win[WIN_VZ].canv_w,
			pref_x + pref_w);
		view_h = win[WIN_VZ].canv_y + win[WIN_VZ].canv_h;
	}

	(void) xv_set(View_win->win,
		XV_WIDTH, view_w + 40,
		XV_HEIGHT, view_h + 30,
		NULL);

	if((int) xv_get(View_win->win, XV_SHOW, NULL) == TRUE)
		return;

	xv_set(View_win->win, XV_SHOW, TRUE, NULL);

	XFlush(display); /* a good idea right about here */
}

/************************************************************/
void
redisplay_all()
{
	void map_buffers();

	int i;

	vprint"redisplaying all buffers ... \n");
	For_all_windows win[i].repaint = TRUE;
	map_buffers();
}

/************************************************************/
void
redisplay_view()
{
	void map_buffers();

	int i;

	vprint"redisplaying view buffers ... \n");
	For_all_view win[i].repaint = TRUE;
	map_buffers();
}

/************************************************************/
void
view_event(win_id, event)
int win_id;
Event *event;
{
/* Events:
 *	shift+click	: display pixel value.
 *	click left	: coordinate aspects.
 *	click middle	: draw crop rectangle.
 *	click right	: draw polygon points.
 */
	void display_pixel_value();
	void coordinate_aspects();
	void fix_2d_coords();
	void draw_crop_rectangle();
	void add_to_polygon();

	int x, y, f, which_button;

	if(!img.loaded) return;

	x = event_x(event) / win[win_id].zoom_mag;
	y = event_y(event) / win[win_id].zoom_mag;
	f = win[win_id].f;
	RANGE(x, 0, win[win_id].img_c - 1)
	RANGE(y, 0, win[win_id].img_r - 1)

	if(event_meta_is_down(event)) {
	}

	else if(event_ctrl_is_down(event)) {
	}

	else if(event_shift_is_down(event)) {
		switch(event_id(event)) {
		case MS_RIGHT :
		case MS_LEFT :
		case MS_MIDDLE :
			if(event_is_down(event)) {
				xv_set(View_win->msg_pixel_value,
					XV_SHOW, TRUE,
					NULL);
				display_pixel_value(win[win_id].aspect, x, y, f);
			}

			else if(event_is_up(event))
				xv_set(View_win->msg_pixel_value,
					XV_SHOW, FALSE,
					NULL);
			break;
		case LOC_DRAG :
			display_pixel_value(win[win_id].aspect, x, y, f);
			break;
		}
	}

	/* other events */
	else if(event_is_down(event)) switch(event_id(event)) {
		case MS_LEFT :
		/* coordinate aspects */
			coordinate_aspects(win_id, x, y);
			break;

		case MS_MIDDLE :
		/* begin to crop a rectangle */
			which_button = MS_MIDDLE;
			if(event_is_down(event)) {
				/* erase old crop rectangle */
				draw_crop_rectangle();

				crop.win_id = win_id;
				crop.x1 = crop.x2 = x;
				crop.y1 = crop.y2 = y;
			}
			else if(event_is_up(event)) {
				fix_2d_coords(&crop.x1, &crop.y1, &crop.x2, &crop.y2);
			}

			break;

		case MS_RIGHT :
		/* draw a polygon point */
			add_to_polygon(win_id, x, y);
			break;

		case LOC_DRAG :
			if(which_button == MS_MIDDLE) {
			/* continue to crop a rectangle */
				/* erase the old crop rectangle */
				draw_crop_rectangle();

				/* get new crop values */
				crop.x2 = x;
				crop.y2 = y;

				/* draw new one */
				draw_crop_rectangle();
			}
			break;
		default :
			break;
	}
}

/************************************************************/
void
draw_crop_rectangle()
{
	void draw_rectangle();

	if(crop.x1 != crop.x2 && crop.y1 != crop.y2)
		draw_rectangle(crop.win_id, crop.x1, crop.y1, crop.x2, crop.y2);
}

/************************************************************/
void
draw_rectangle(win_id, x1, y1, x2, y2)
int win_id, x1, y1, x2, y2;
{
/* expects x1 < x2 and y1 < y2 */
	void fix_2d_coords();

	int zx1, zy1, zx2, zy2;

	XSetFunction(display, gc, GXinvert);

	fix_2d_coords(&x1, &y1, &x2, &y2);

	zx1 = x1 * win[win_id].zoom_mag;
	zy1 = y1 * win[win_id].zoom_mag;
	zx2 = x2 * win[win_id].zoom_mag;
	zy2 = y2 * win[win_id].zoom_mag;
	XDrawRectangle(display, win[win_id].xid, gc,
		zx1, zy1, zx2 - zx1, zy2 - zy1);

	XSetFunction(display, gc, GXcopy);
}

/************************************************************/
void
fix_2d_coords(x1, y1, x2, y2)
int *x1, *y1, *x2, *y2;
{
	if(*x1 > *x2) swap(x1, x2);
	if(*y1 > *y2) swap(y1, y2);
}

/************************************************************/
void
display_pixel_value(aspect_id, x, y, f)
int aspect_id, x, y, f;
{
	void get_3d_coords();

	int x1, y1, f1;
	char foo[80];

	get_3d_coords(aspect_id, ASPECT_Z, x, y, f, &x1, &y1, &f1);

	/* coordinates displayed in the z aspect .... */
	sprintf(foo, "Pixel value: (%3d,%3d,%3d) = %3d",
		x1, y1, f1, ibuf[f1][y1][x1]);

	xv_set(View_win->msg_pixel_value,
		PANEL_LABEL_STRING, foo,
		NULL);
}

/************************************************************/
void
coordinate_aspects(win_id, mx, my)
int win_id, mx, my;
{
	void redisplay_view();
	void get_3d_coords();
	u_long standout();

	int i, x, y, f;
	XRectangle recs[3]; /* 3 aspects */

	For_all_view {
		get_3d_coords(win_id, i, mx, my, win[win_id].f, &x, &y, &f);
		if(i != win_id) {
			win[i].f = f;
			recs[i].x = (short) ((x-1) * win[i].zoom_mag);
			recs[i].y = (short) ((y-1) * win[i].zoom_mag);
		}
		else {
			recs[i].x = (short) ((mx-1) * win[i].zoom_mag);
			recs[i].y = (short) ((my-1) * win[i].zoom_mag);
		}
		recs[i].width = (unsigned short) win[i].zoom_mag * 5;
		recs[i].height = (unsigned short) win[i].zoom_mag * 5;
	}

	xv_set(View_win->set_frame_x,
		PANEL_VALUE, win[WIN_VX].f,
		NULL);

	xv_set(View_win->set_frame_y,
		PANEL_VALUE, win[WIN_VY].f,
		NULL);

	xv_set(View_win->set_frame_z,
		PANEL_VALUE, win[WIN_VZ].f,
		NULL);

	redisplay_view();

	XSetForeground(display, gc, standout(RED));
	For_all_view XDrawRectangle(display, win[i].xid, gc, recs[i].x,
			recs[i].y, recs[i].width, recs[i].height);
}

/************************************************************/
void
add_to_polygon(win_id, x, y)
int win_id, x, y;
{
}

/************************************************************/
void
get_3d_coords(asp_in, asp_out, x0, y0, f0, x1, y1, f1)
int asp_in, asp_out, x0, y0, f0;
int *x1, *y1, *f1;
{
	if(asp_in == asp_out) {
		*x1 = x0;
		*y1 = y0;
		*f1 = f0;
	}
	else switch(asp_in) {
	case ASPECT_X :
		switch(asp_out) {
		case ASPECT_Y :
			*x1 = f0;
			*y1 = y0;
			*f1 = segal.r - 1 - x0;
			break;
		case ASPECT_Z :
			*x1 = f0;
			*y1 = x0;
			*f1 = segal.f - 1 - y0;
			break;
		default :
			break;
		}
		break;
	case ASPECT_Y :
		switch(asp_out) {
		case ASPECT_X :
			*x1 = segal.r - 1 - f0;
			*y1 = y0;
			*f1 = x0;
			break;
		case ASPECT_Z :
			*x1 = x0;
			*y1 = segal.r - 1 - f0;
			*f1 = segal.f - 1 - y0;
			break;
		default :
			break;
		}
		break;
	case ASPECT_Z :
		switch(asp_out) {
		case ASPECT_X :
			*x1 = y0;
			*y1 = segal.f - 1 - f0;
			*f1 = x0;
			break;
		case ASPECT_Y :
			*x1 = x0;
			*y1 = segal.f - 1 - f0;
			*f1 = segal.r - 1 - y0;
			break;
		default :
			break;
		}
		break;
	default:
		break;
	}
}
