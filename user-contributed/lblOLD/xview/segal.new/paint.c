/*
 *	paint.c - for use with Segal
 *
 *	By Bryan Skene
 *
 */

#include "common.h"

#define MAX_PCANV_WIDTH		512
#define MAX_PCANV_HEIGHT	512

/*********************************************/
void
paint_setup()
{
	void change_cursor_proc();
	int max_frames();

	int canv_w, canv_h, paint_w;

	change_cursor_proc();

	win[WIN_PAINT].canv_x = (int) xv_get(Paint_win_paint->canvas, XV_X, NULL);
	win[WIN_PAINT].canv_y = (int) xv_get(Paint_win_paint->canvas, XV_Y, NULL);
	win[WIN_PAINT].img_r = win[win[WIN_PAINT].aspect].img_r;
	win[WIN_PAINT].img_c = win[win[WIN_PAINT].aspect].img_c;
	win[WIN_PAINT].img_size = win[win[WIN_PAINT].aspect].img_size;
	RANGE(win[WIN_PAINT].f, 0, max_frames(win[WIN_PAINT].aspect))

	win[WIN_PAINT].canv_w = win[WIN_PAINT].img_c * win[WIN_PAINT].zoom_mag;
	win[WIN_PAINT].canv_h = win[WIN_PAINT].img_r * win[WIN_PAINT].zoom_mag;

	canv_w = win[WIN_PAINT].canv_w;
	canv_h = win[WIN_PAINT].canv_h;

	RANGE(win[WIN_PAINT].canv_w, 0, MAX_PCANV_WIDTH)
	RANGE(win[WIN_PAINT].canv_h, 0, MAX_PCANV_HEIGHT)

	/* paint window needs to be showing for these things to occur */
	xv_set(Paint_win_paint->win_paint,
		XV_SHOW, TRUE,
		NULL);

	if(canv_w <= MAX_PCANV_WIDTH) xv_destroy_safe(win[WIN_PAINT].h_sbar);
	if(canv_h <= MAX_PCANV_HEIGHT) xv_destroy_safe(win[WIN_PAINT].v_sbar);

	xv_set(Paint_win_paint->set_aspect,
		XV_SHOW, TRUE,
		NULL);
	
	xv_set(Paint_win_paint->set_zoom_mag,
		XV_SHOW, TRUE,
		NULL);

	if(segal.r3d) {
		xv_set(Paint_win_paint->set_paint_frame,
			XV_SHOW, TRUE,
			XV_X, win[WIN_PAINT].canv_x + win[WIN_PAINT].canv_w - 90,
			PANEL_MAX_VALUE, max_frames(win[WIN_PAINT].aspect),
			PANEL_SLIDER_WIDTH, win[WIN_PAINT].canv_h,
			PANEL_VALUE, win[WIN_PAINT].f,
			NULL);
	}
	else {
		xv_set(Paint_win_paint->set_paint_frame,
			XV_SHOW, FALSE,
			NULL);
	}


	xv_set(Paint_win_paint->canvas,
		XV_SHOW, TRUE,
		XV_WIDTH, win[WIN_PAINT].canv_w,
		XV_HEIGHT, win[WIN_PAINT].canv_h,
		CANVAS_WIDTH, canv_w,
		CANVAS_HEIGHT, canv_h,
		CANVAS_AUTO_SHRINK, FALSE,
		CANVAS_AUTO_EXPAND, FALSE,
		NULL);

	if(canv_w > MAX_PCANV_WIDTH)
	win[WIN_PAINT].h_sbar =
	(Scrollbar) xv_create(Paint_win_paint->canvas, SCROLLBAR,
		SCROLLBAR_DIRECTION, SCROLLBAR_HORIZONTAL,
		SCROLLBAR_PIXELS_PER_UNIT, win[WIN_PAINT].zoom_mag,
		SCROLLBAR_OBJECT_LENGTH, win[WIN_PAINT].img_c,
		SCROLLBAR_PAGE_LENGTH, MAX_PCANV_WIDTH,
		SCROLLBAR_VIEW_LENGTH, MAX_PCANV_WIDTH,
		NULL);

	if(canv_h > MAX_PCANV_HEIGHT)
	win[WIN_PAINT].v_sbar =
	(Scrollbar) xv_create(Paint_win_paint->canvas, SCROLLBAR,
		SCROLLBAR_DIRECTION, SCROLLBAR_VERTICAL,
		SCROLLBAR_PIXELS_PER_UNIT, win[WIN_PAINT].zoom_mag,
		SCROLLBAR_OBJECT_LENGTH, win[WIN_PAINT].img_r,
		SCROLLBAR_PAGE_LENGTH, MAX_PCANV_HEIGHT,
		SCROLLBAR_VIEW_LENGTH, MAX_PCANV_HEIGHT,
		NULL);

	paint_w = MAX((win[WIN_PAINT].canv_x + win[WIN_PAINT].canv_w + 40),
		(xv_get(Paint_win_paint->but_apply_log, XV_X, NULL) +
		xv_get(Paint_win_paint->but_apply_log, XV_WIDTH, NULL) +
		10));

	xv_set(Paint_win_paint->win_paint,
		XV_WIDTH, paint_w,
		XV_HEIGHT, win[WIN_PAINT].canv_y + win[WIN_PAINT].canv_h + 30,
		NULL);
}

/*********************************************/
int
max_frames(aspect)
int aspect;
{
	switch(aspect) {
	case ASPECT_X :
		return(segal.c - 1);
		break;
	case ASPECT_Y :
		return(segal.r - 1);
		break;
	case ASPECT_Z :
		return(segal.f - 1);
		break;
	}
}

/*********************************************/
void
redisplay_paint()
{
	void map_buffers();

	win[WIN_PAINT].repaint = TRUE;
	map_buffers();
}

/*********************************************/
void
paint_event(event)
Event *event;
{
	void get_stats();
	void draw_location();
	int gradient();
	void erase_thresh_bounds();
	void draw_thresh_bounds();
	void free_2d_byte_array();
	byte **alloc_2d_byte_array();
	void save_image_frame();
	void save_mask_frame();
	void get_3d_coords();
	void save_mask_undo_2d();
	void save_image_undo();
	void paint();
	u_long standout();
	void paint_cell();

	int x, y, x1, y1, f1;
 
/*
mouse events:
        any button: paint
        shift, any button  : show gray value
        ctrl, middle button: sets gradient according to highest gradient
		 between any adjacent pixels encountered (from push to release).
        ctrl, left button  : sets the growth lower threshold slider bar.
        ctrl, right button : sets the growth upper threshold slider bar.
	ctrl+shift, left   : begin a new point list.
	ctrl+shift, left   : add to existing point list.
        meta, any button   : show object statistics
*/
 
	/* meta events */
	if(event_meta_is_down(event)) {
		if(event_is_down(event)) switch(event_id(event)) {
		case MS_RIGHT:
		case MS_LEFT:
		case MS_MIDDLE:
			x = event_x(event) / win[WIN_PAINT].zoom_mag;
			y = event_y(event) / win[WIN_PAINT].zoom_mag;
			RANGE(x, 0, win[WIN_PAINT].img_c - 1)
			RANGE(y, 0, win[WIN_PAINT].img_r - 1)
			/*
			get_stats(x, y);
			*/
			break;
		}
	return;
	}

	/* shift events */
	if(event_shift_is_down(event)
	&& !event_ctrl_is_down(event)) {
		switch (event_id(event)) {
		case MS_RIGHT:
		case MS_LEFT:
		case MS_MIDDLE:
		case LOC_DRAG:
			x = event_x(event) / win[WIN_PAINT].zoom_mag;
			y = event_y(event) / win[WIN_PAINT].zoom_mag;
			RANGE(x, 0, win[WIN_PAINT].img_c - 1)
			RANGE(y, 0, win[WIN_PAINT].img_r - 1)
			/*
			if (event_is_down(event)) draw_location(x, y, TRUE);
			else draw_location(x, y, FALSE);
			*/
			break;
		}
	return;
	}
 
 
	/* ctrl events */
	if(!event_shift_is_down(event)
	&& event_ctrl_is_down(event)) {
		int value, g;

		x = event_x(event) / win[WIN_PAINT].zoom_mag;
		y = event_y(event) / win[WIN_PAINT].zoom_mag;
		RANGE(x, 0, win[WIN_PAINT].img_c - 1)
		RANGE(y, 0, win[WIN_PAINT].img_r - 1)
		if(segal.color)
			value = MONO(win[WIN_PAINT].i_data[RP][y][x],
				win[WIN_PAINT].i_data[GP][y][x],
				win[WIN_PAINT].i_data[BP][y][x]);
		else value = win[WIN_PAINT].i_data[GRAY][y][x];

		switch (event_id(event)) {
		static int which_button;

		case MS_LEFT:
			which_button = MS_LEFT;
			if(event_is_down(event)) {
				/* mask grow */
				xv_set(Mask_grow_pop_mask_grow->set_threshold_min,
					PANEL_VALUE, value,
					NULL);
				grow.threshold_min = value;

				/* threshold */
				erase_thresh_bounds();
				threshold.min = value;
				if(value > threshold.max) threshold.max = value;
				draw_thresh_bounds();
			}
			break;

		case MS_MIDDLE:
			which_button = MS_MIDDLE;
			if(event_is_down(event)) {
				g = gradient(x, y, win[WIN_PAINT].f);
				grow.gradient_min = grow.gradient_max = g;
				xv_set(Mask_grow_pop_mask_grow->set_gradient_max,
					PANEL_VALUE, grow.gradient_max,
					NULL);
				xv_set(Mask_grow_pop_mask_grow->set_gradient_min,
					PANEL_VALUE, grow.gradient_min,
					NULL);
			}
			break;

		case MS_RIGHT:
			which_button = MS_RIGHT;
			if(event_is_down(event)) {
				/* mask grow */
				xv_set(Mask_grow_pop_mask_grow->set_threshold_max,
					PANEL_VALUE, value,
					NULL);
				grow.threshold_max = value;

				/* threshold */
				erase_thresh_bounds();
				threshold.max = value;
				if(value < threshold.min) threshold.min = value;
				draw_thresh_bounds();
			}
			break;

		case LOC_DRAG:
			x = event_x(event) / win[WIN_PAINT].zoom_mag;
			y = event_y(event) / win[WIN_PAINT].zoom_mag;
			RANGE(x, 0, win[WIN_PAINT].img_c - 1)
			RANGE(y, 0, win[WIN_PAINT].img_r - 1)
			if (event_is_down(event)) switch(which_button) {
			case MS_LEFT:
				if(value < grow.threshold_min) {
					grow.threshold_min = value;
					xv_set(Mask_grow_pop_mask_grow->set_threshold_min,
						PANEL_VALUE, grow.threshold_min,
						NULL);
				}

				/* threshold */
				erase_thresh_bounds();
				threshold.min = value;
				if(value > threshold.max) threshold.max = value;
				draw_thresh_bounds();

				break;
			case MS_MIDDLE:
				g = gradient(x, y, win[WIN_PAINT].f);
				if(g > grow.gradient_max) {
					grow.gradient_max = g;
					xv_set(Mask_grow_pop_mask_grow->set_gradient_max,
						PANEL_VALUE, g,
						NULL);
				}
				if(g < grow.gradient_min) {
					grow.gradient_min = g;
					xv_set(Mask_grow_pop_mask_grow->set_gradient_min,
						PANEL_VALUE, g,
						NULL);
				}
				break;
			case MS_RIGHT:
				if(value > grow.threshold_max) {
					grow.threshold_max = value;
					xv_set(Mask_grow_pop_mask_grow->set_threshold_max,
						PANEL_VALUE, grow.threshold_max,
						NULL);
				}

				/* threshold */
				erase_thresh_bounds();
				threshold.max = value;
				if(value < threshold.min) threshold.min = value;
				draw_thresh_bounds();

				break;
			}
			break;
		}
	return;
	}

	/* ctrl + shift events */
	if(event_shift_is_down(event)
	&& event_ctrl_is_down(event)) {
		x = event_x(event) / win[WIN_PAINT].zoom_mag;
		y = event_y(event) / win[WIN_PAINT].zoom_mag;
		RANGE(x, 0, win[WIN_PAINT].img_c - 1)
		RANGE(y, 0, win[WIN_PAINT].img_r - 1)

		get_3d_coords(win[WIN_PAINT].aspect, ASPECT_Z,
			x, y, win[WIN_PAINT].f, &x1, &y1, &f1);

		switch (event_id(event)) {
		case MS_LEFT :
		case MS_MIDDLE :
		case MS_RIGHT :
		case LOC_DRAG:
		default :
			break;
		}
		return;
	}

	/* other events */
	switch (event_id(event)) {
	case LOC_WINENTER:
		break;
	case LOC_WINEXIT:
		break;
	case MS_RIGHT:
	case MS_LEFT:
	case MS_MIDDLE:
		if(event_is_down(event)) {
			switch(brush.mode) {
			case BRUSH_IMAGE :
				save_image_undo(WIN_PAINT);
				break;
			case BRUSH_MASK : 
			case BRUSH_PTS : 
				save_mask_undo_2d(WIN_PAINT);
				break;
			default :
				break;
			}

			x = event_x(event) / win[WIN_PAINT].zoom_mag;
			y = event_y(event) / win[WIN_PAINT].zoom_mag;
			paint(x, y);
		}
		if(event_is_up(event)) {
			switch(brush.mode) {
			case BRUSH_IMAGE :
				save_image_frame(WIN_PAINT);
				break;
			case BRUSH_MASK : 
			case BRUSH_PTS : 
				save_mask_frame(WIN_PAINT);
				break;
			default :
				break;
			}
		}
		break;
	case LOC_DRAG:
		if (event_is_down(event)) {
			x = event_x(event) / win[WIN_PAINT].zoom_mag;
			y = event_y(event) / win[WIN_PAINT].zoom_mag;
			paint(x, y);
		}
		break;
	default:
		break;
	}
}
 
/******************************************************/
void
paint(x, y)
int x, y;
{
	void paint_mask();
	void paint_image();
	void paint_pt();
 
	RANGE(x, 0, win[WIN_PAINT].img_c - 1)
	RANGE(y, 0, win[WIN_PAINT].img_r - 1)

	switch(brush.mode) {
	case BRUSH_IMAGE :
		img.changed_frame = TRUE;
		paint_image(x, y);
		break;
	case BRUSH_MASK : 
		m[segal.e_m].changed_frame = TRUE;
		paint_mask(x, y);
		break;
	case BRUSH_PTS :
		m[segal.e_m].changed_frame = TRUE;
		paint_pt(x, y);
		break;
	default :
		break;
	}
}
 
/******************************************************/
void
paint_mask(x, y)
int x, y;
{
	void erase_mask_pixel();
	void paint_mask_pixel();

	int i, j, bsize, max_i, max_j, cx, cy;

	bsize = brush.size + 1;
	if(brush.size == 5) bsize = 10;
	if(brush.size == 6) bsize = 20;

	max_i = MIN(x + bsize, win[WIN_PAINT].img_c - 1);
	max_j = MIN(y + bsize, win[WIN_PAINT].img_r - 1);

	cx = cy = my_cursor[brush.size][0].corner;

	for(i = x; i < max_i; i++)
	for(j = y; j < max_j; j++) {
		if(brush.mask_affect == MASK_ERASE
		&& BIT_IS_ON(win[WIN_PAINT].m_data[j][i], m[segal.e_m].bit_key)) {
			if(brush.shape == BRUSH_ROUND
			&& my_cursor[brush.size][0].paint_mask_matrix[j-y+cy][i-x+cx])
				erase_mask_pixel(i, j);
			else if(brush.shape == BRUSH_SQUARE)
				erase_mask_pixel(i, j);
		}
		else if(brush.mask_affect == MASK_PAINT
		&& !BIT_IS_ON(win[WIN_PAINT].m_data[j][i], m[segal.e_m].bit_key)) {
			if(brush.shape == BRUSH_ROUND
			&& my_cursor[brush.size][0].paint_mask_matrix[j-y+cy][i-x+cx])
				paint_mask_pixel(i, j);
			else if(brush.shape == BRUSH_SQUARE)
				paint_mask_pixel(i, j);
		}
	}
}
 
/******************************************************/
double
dist(x1, y1, x2, y2)
double x1, y1, x2, y2;
{
	return(pow(((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1)), .5));
} 

/******************************************************/
void
erase_mask_pixel(x, y)
int x, y;
{
	unsigned long standout();
	byte map_rgb_to_xcolor();
	LOGIC turn_bit_off_with_log();
	void paint_view_cells();
	void paint_cell();

	unsigned long pval;
	int intensity;

	if(segal.color)
		intensity = MONO(win[WIN_PAINT].i_data[RP][y][x],
			win[WIN_PAINT].i_data[GP][y][x],
			win[WIN_PAINT].i_data[BP][y][x]);
	else intensity = win[WIN_PAINT].i_data[GRAY][y][x];


	if(BIT_IS_ON(win[WIN_PAINT].m_data[y][x], m[BUF_PTS].bit_key)) {
		if(segal.disp_image)
			pval = colors[pt_lut[intensity]];
		else pval = standout(ORANGE);
	}	
	else {
		if(segal.disp_image) {
		if(segal.color)
			pval =  colors[map_rgb_to_xcolor(
				win[WIN_PAINT].i_data[RP][y][x],
				win[WIN_PAINT].i_data[GP][y][x],
				win[WIN_PAINT].i_data[BP][y][x])];
			else pval = colors[gray_lut[intensity]];

		}
		else pval = standout(CBLACK);
	}

	win[WIN_PAINT].z_data[y][x] = (byte) pval;

	/* turn mask bit off */
	if(turn_bit_off_with_log(&win[WIN_PAINT].m_data[y][x])) {
		/* now paint the cell */
		paint_view_cells(win[WIN_PAINT].aspect, x, y,
			win[WIN_PAINT].f, pval);
		paint_cell(WIN_PAINT, x, y, pval);
	}
}
 
/******************************************************/
void
paint_mask_pixel(x, y)
int x, y;
{
	unsigned long standout();
	LOGIC turn_bit_on_with_log();
	void paint_view_cells();
	void paint_cell();

	unsigned long pval;
	int intensity;

	if(segal.color)
		intensity = MONO(win[WIN_PAINT].i_data[RP][y][x],
			win[WIN_PAINT].i_data[GP][y][x],
			win[WIN_PAINT].i_data[BP][y][x]);
	else intensity = win[WIN_PAINT].i_data[GRAY][y][x];

	if(BIT_IS_ON(win[WIN_PAINT].m_data[y][x], m[BUF_PTS].bit_key))
		pval = standout(ORANGE);
	else {
		if(segal.disp_image)
			pval = colors[blend_lut[intensity]];
		else pval = standout(CWHITE);
	}

	win[WIN_PAINT].z_data[y][x] = (byte) pval;

	/* turn mask bit on */
	if(turn_bit_on_with_log(&win[WIN_PAINT].m_data[y][x])) {
		/* now paint the cell */
		paint_view_cells(win[WIN_PAINT].aspect, x, y,
			win[WIN_PAINT].f, pval);
		paint_cell(WIN_PAINT, x, y, pval);
	}
}

/******************************************************/
void
paint_image(x, y)
int x, y;
{
	void paint_image_pixel();

	int i, j, bsize, max_i, max_j, cx, cy;

	bsize = brush.size + 1;
	if(brush.size == 5) bsize = 10;
	if(brush.size == 6) bsize = 20;

	max_i = MIN(x + bsize, win[WIN_PAINT].img_c - 1);
	max_j = MIN(y + bsize, win[WIN_PAINT].img_r - 1);

	cx = cy = my_cursor[brush.size][0].corner;
	for(i = x; i < max_i; i++)
	for(j = y; j < max_j; j++) {
		if(brush.shape == BRUSH_ROUND
		&& my_cursor[brush.size][0].paint_mask_matrix[j-y+cy][i-x+cx])
			paint_image_pixel(i, j);
		else if(brush.shape == BRUSH_SQUARE)
			paint_image_pixel(i, j);
	}
}

/******************************************************/
void
paint_image_pixel(x, y)
int x, y;
{
/* BIND_PVAL sets i_pval to UNDEFINED if nothing changed */
#define BIND_PVAL(p,d) {i_pval=p+d; RANGE(i_pval,0,255) if(p==i_pval) i_pval = UNDEFINED; else p=i_pval;}

	byte map_rgb_to_xcolor();
	void paint_view_cells();
	void paint_cell();

	int i, i_pval, intensity;
	unsigned long pval;
	LOGIC changed;

	/* add delta to image pixel value */
	changed = FALSE;
	if(segal.color) For_rgb {
		BIND_PVAL(win[WIN_PAINT].i_data[i][y][x], brush.image_affect)
		if(i_pval != UNDEFINED) changed = TRUE;
	}
	else {
		BIND_PVAL(win[WIN_PAINT].i_data[GRAY][y][x], brush.image_affect)
		if(i_pval != UNDEFINED) changed = TRUE;
	}

	/* nothing changed or both mask and points are painted here */
	if(!changed
	|| (BIT_IS_ON(win[WIN_PAINT].m_data[y][x], m[segal.e_m].bit_key)
	&& BIT_IS_ON(win[WIN_PAINT].m_data[y][x], m[BUF_PTS].bit_key)))
		return;

	/* get appropriate pixel value for the gc */
	if(BIT_IS_ON(win[WIN_PAINT].m_data[y][x], m[segal.e_m].bit_key)
	&& segal.disp_mask) {
		if(segal.color)
			intensity = MONO(win[WIN_PAINT].i_data[RP][y][x],
				win[WIN_PAINT].i_data[GP][y][x],
				win[WIN_PAINT].i_data[BP][y][x]);
		else intensity = win[WIN_PAINT].i_data[GRAY][y][x];
		pval = colors[blend_lut[intensity]];
	}
	else if(segal.disp_image) {
		if(segal.color)
			pval =  colors[map_rgb_to_xcolor(
				win[WIN_PAINT].i_data[RP][y][x],
				win[WIN_PAINT].i_data[GP][y][x],
				win[WIN_PAINT].i_data[BP][y][x])];
		else pval = colors[gray_lut[win[WIN_PAINT].i_data[GRAY][y][x]]];
	}
	win[WIN_PAINT].z_data[y][x] = (byte) pval;

	/* now paint the cell */
	paint_view_cells(win[WIN_PAINT].aspect, x, y, win[WIN_PAINT].f, pval);
	paint_cell(WIN_PAINT, x, y, pval);
}

/******************************************************/
void
paint_pt(x, y)
int x, y;
{
/* point list paintbrush */
	void erase_pt_pixel();
	void paint_pt_pixel();

	int i, j, bsize, max_i, max_j, cx, cy;

	bsize = brush.size + 1;
	if(brush.size == 5) bsize = 10;
	if(brush.size == 6) bsize = 20;

	max_i = MIN(x + bsize, win[WIN_PAINT].img_c - 1);
	max_j = MIN(y + bsize, win[WIN_PAINT].img_r - 1);

	cx = cy = my_cursor[brush.size][0].corner;

	for(i = x; i < max_i; i++)
	for(j = y; j < max_j; j++) {
		if(brush.mask_affect == MASK_ERASE
		&& BIT_IS_ON(win[WIN_PAINT].m_data[j][i], m[BUF_PTS].bit_key)) {
			if(brush.shape == BRUSH_ROUND
			&& my_cursor[brush.size][0].paint_mask_matrix[j-y+cy][i-x+cx])
				erase_pt_pixel(i, j);
			else if(brush.shape == BRUSH_SQUARE)
				erase_pt_pixel(i, j);
		}
		else if(brush.mask_affect == MASK_PAINT
		&& !BIT_IS_ON(win[WIN_PAINT].m_data[j][i], m[BUF_PTS].bit_key)) {
			if(brush.shape == BRUSH_ROUND
			&& my_cursor[brush.size][0].paint_mask_matrix[j-y+cy][i-x+cx])
				paint_pt_pixel(i, j);
			else if(brush.shape == BRUSH_SQUARE)
				paint_pt_pixel(i, j);
		}
	}
}
 
/******************************************************/
void
erase_pt_pixel(x, y)
int x, y;
{
	byte map_rgb_to_xcolor();
	unsigned long standout();
	void paint_view_cells();
	void paint_cell();

	unsigned long pval;
	int intensity;

	if(segal.color)
		intensity = MONO(win[WIN_PAINT].i_data[RP][y][x],
			win[WIN_PAINT].i_data[GP][y][x],
			win[WIN_PAINT].i_data[BP][y][x]);
	else intensity = win[WIN_PAINT].i_data[GRAY][y][x];

	if(BIT_IS_ON(win[WIN_PAINT].m_data[y][x], m[segal.e_m].bit_key)) {
		if(segal.disp_image)
			pval = colors[blend_lut[intensity]];
		else pval = standout(CWHITE);
	}
	else {
		if(segal.disp_image) {
			if(segal.color)
				pval =  colors[map_rgb_to_xcolor(
					win[WIN_PAINT].i_data[RP][y][x],
					win[WIN_PAINT].i_data[GP][y][x],
					win[WIN_PAINT].i_data[BP][y][x])];
			else pval = colors[gray_lut[intensity]];

		}
		else pval = standout(CBLACK);
	}

	win[WIN_PAINT].z_data[y][x] = (byte) pval;

	/* turn mask bit off */
	TURN_BIT_OFF(win[WIN_PAINT].m_data[y][x], m[BUF_PTS].bit_key)

	/* now paint the cell */
	paint_view_cells(win[WIN_PAINT].aspect, x, y,
		win[WIN_PAINT].f, pval);
	paint_cell(WIN_PAINT, x, y, pval);
}
 
/******************************************************/
void
paint_pt_pixel(x, y)
int x, y;
{
	unsigned long standout();
	void paint_view_cells();
	void paint_cell();

	unsigned long pval;
	int intensity;

	if(segal.color)
		intensity = MONO(win[WIN_PAINT].i_data[RP][y][x],
			win[WIN_PAINT].i_data[GP][y][x],
			win[WIN_PAINT].i_data[BP][y][x]);
	else intensity = win[WIN_PAINT].i_data[GRAY][y][x];

	if(BIT_IS_ON(win[WIN_PAINT].m_data[y][x], m[segal.e_m].bit_key))
		pval = standout(ORANGE);
	else {
		if(segal.disp_image)
			pval = colors[pt_lut[intensity]];
		else pval = standout(RED);
	}

	win[WIN_PAINT].z_data[y][x] = (byte) pval;

	/* turn mask bit on */
	TURN_BIT_ON(win[WIN_PAINT].m_data[y][x], m[BUF_PTS].bit_key)

	/* now paint the cell */
	paint_view_cells(win[WIN_PAINT].aspect, x, y,
		win[WIN_PAINT].f, pval);
	paint_cell(WIN_PAINT, x, y, pval);
}

/******************************************************/
void
paint_view_cells(asp_in, x, y, f, pval)
int asp_in;
int x, y, f;
unsigned long pval;
{
	void get_3d_coords();
	void paint_cell();

	int i, x1, y1, f1;

	For_all_view {
		get_3d_coords(asp_in, i, x, y, f,
			&x1, &y1, &f1);
		if(f1 == win[i].f) paint_cell(i, x1, y1, pval);
	}
}

/******************************************************/
void
paint_cell(win_id, x, y, pval)
int win_id;
int x, y;
unsigned long pval;
{
	register int x0, y0;
	int x1, x2, y1, y2, npoints;
	XPoint points[MAX_BRUSH_SIZE * MAX_BRUSH_SIZE * MAX_ZOOM_MAG];

	npoints = 0;

	x1 = x * win[win_id].zoom_mag;
	x2 = x1 + win[win_id].zoom_mag;
	y1 = y * win[win_id].zoom_mag;
	y2 = y1 + win[win_id].zoom_mag;

	for(x0 = x1; x0 < x2; x0++)
	for(y0 = y1; y0 < y2; y0++) {
		XPutPixel(win[win_id].ximg, x0, y0, pval);
		points[npoints].x = x0;
		points[npoints++].y = y0;
	}
	XSetForeground(display, gc, pval);
	XDrawPoints(display, win[win_id].xid, gc, points, npoints, CoordModeOrigin);
}

/******************************************************/
void
apply_log(roi)
int roi;
{
	LOGIC bit_is_excluded();
	LOGIC bit_is_included();
	void save_mask_frame();
	void redisplay_paint();
	void redisplay_all();

	byte *mdata;
	int i, size;

	switch(roi) {
	case R2d_WHOLE :
		mdata = win[WIN_PAINT].m_data[0];
		size = win[WIN_PAINT].img_size;
		break;
	case R3d_WHOLE :
		mdata = mbuf[0][0];
		size = segal.r * segal.c * segal.f;
		break;
	default :
		break;
	}

	switch(mlog.apply_order) {
	case ORDER_EI :
		for(i = 0; i < size; i++) {
			if(bit_is_excluded(mdata[i]))
				TURN_BIT_OFF(mdata[i], m[segal.e_m].bit_key)
			if(bit_is_included(mdata[i]))
				TURN_BIT_ON(mdata[i], m[segal.e_m].bit_key)
		}
		break;
	case ORDER_IE :
		for(i = 0; i < size; i++) {
			if(bit_is_included(mdata[i]))
				TURN_BIT_ON(mdata[i], m[segal.e_m].bit_key)
			if(bit_is_excluded(mdata[i]))
				TURN_BIT_OFF(mdata[i], m[segal.e_m].bit_key)
		}
		break;
	default :
		break;
	}

	if(roi == R2d_WHOLE) {
		m[segal.e_m].changed_frame = TRUE;
		save_mask_frame(WIN_PAINT);
	}

	redisplay_all();
}
