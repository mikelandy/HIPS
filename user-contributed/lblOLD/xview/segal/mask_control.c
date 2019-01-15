/*
 *	mask_control.c
 *
 *	Bryan Skene, LBL July 1991
 *	part of 'segal'
 */

#include "common.h"

#include "frame_control.h"
#include "mask_log.h"
#include "pixedit.h"

#include "mask_control_ui.h"

/* Allocation :) */
mask_control_win_objects *mask_control_win;

/* Matrix types - remember each has an x and y component */
#define GRADIENT 0
#define SOBEL 2
#define LAPLACIAN 4
#define KASVAND 6

static int gradient_matrix_x[9][9] = {
	{-1, -2, -3, -4, 0, 4, 3, 2, 1},
	{-2, -3, -4, -5, 0, 5, 4, 3, 2},
	{-3, -4, -5, -6, 0, 6, 5, 4, 3},
	{-4, -5, -6, -7, 0, 7, 6, 5, 4},
	{-5, -6, -7, -8, 0, 8, 7, 6, 5},
	{-4, -5, -6, -7, 0, 7, 6, 5, 4},
	{-3, -4, -5, -6, 0, 6, 5, 4, 3},
	{-2, -3, -4, -5, 0, 5, 4, 3, 2},
	{-1, -2, -3, -4, 0, 4, 3, 2, 1}
};

static int gradient_matrix_y[9][9] = {
	{-1, -2, -3, -4, -5, -4, -3, -2, -1},
	{-2, -3, -4, -5, -6, -5, -4, -3, -2},
	{-3, -4, -5, -6, -7, -6, -5, -4, -3},
	{-4, -5, -6, -7, -8, -7, -6, -5, -4},
	{ 0,  0,  0,  0,  0,  0,  0,  0,  0},
	{ 4,  5,  6,  7,  8,  7,  6,  5,  4},
	{ 3,  4,  5,  6,  7,  6,  5,  4,  3},
	{ 2,  3,  4,  5,  6,  5,  4,  3,  2},
	{ 1,  2,  3,  4,  5,  4,  3,  2,  1}
};

static int sobel_matrix_x[3][3] = {
	{ 1,  0, -1},
	{ 2,  0, -2},
	{ 1,  0, -1}
};

static int sobel_matrix_y[3][3] = {
	{ 1,  2,  1},
	{ 0,  0,  0},
	{-1, -2, -1}
};

static int laplacian_matrix[3][3] = {
	{-1, -1, -1},
	{-1,  8, -1},
	{-1, -1, -1},
};

static int kasvand_matrix[5][5] = {
	{-1, -1, -1, -1, -1},
	{-1, -1, -1, -1, -1},
	{-1, -1, 24, -1, -1},
	{-1, -1, -1, -1, -1},
	{-1, -1, -1, -1, -1},
};

#define MATRIX_SCALE_FACTOR 1./8.

typedef struct {
	int	x;
	int	y;
	}	POINT_TYPE;

static POINT_TYPE *edge_points;
static int num_pts;

/* Good Lord this XView method of interrupt handling is Kludgy and dangerous! */
#define INTERVAL_SEC 0
#define INTERVAL_uSEC 50
struct itimerval itimevalue;

/*****************************************************/
void
mask_control_win_init(owner)
Xv_opaque owner;
{
	void scale_matrices();

	mask_control_win = mask_control_win_objects_initialize(NULL, owner);

	/* defaults */
	grow.direction = 0; /* not implemented */
	grow.matrix_width = 1;
	grow.matrix_type = GRADIENT;
	grow.apply_in = 0;
	grow.extent_3D = 1;
	grow.gradient = 10;
	grow.distance = 0; /* not implemented */
	grow.threshold_high = 255;
	grow.threshold_low = 0;

	region.whole = 1;
	region.region = 0;
	region.beg_frame = 0;
	region.end_frame = 0;
	region.refresh = 1;
	region.rows = 0;
	region.cols = 0;
	region.crx1 = 0;
	region.cry1 = 0;
	region.crx2 = 0;
	region.cry2 = 0;

	(void) xv_set(mask_control_win->grow_threshold_high,
		PANEL_VALUE, grow.threshold_high,
		NULL);

	scale_matrices();
}

/*****************************************************/
void
map_mask_control(item, event)
Panel_item      item;
Event           *event;
{
	void draw_hint();

	fputs("segal: map_mask_control\n", stderr);
        /* Map / Unmap toggle */
	if (xv_get(mask_control_win->win, XV_SHOW, NULL) == FALSE) {
		xv_set(mask_control_win->win,
			XV_SHOW, TRUE,
			NULL);
		draw_hint("Shift+mouse button: get thresholds");
	}
	else xv_set(mask_control_win->win,
			XV_SHOW, FALSE,
			NULL);
}

/**********************************************/
void
mask_clear_proc(item, event)
Panel_item      item;
Event           *event;
{
	mask_op_proc(0);
}

/**********************************************/
void
mask_fill_proc(item, event)
Panel_item      item;
Event           *event;
{
	mask_op_proc(1);
}

/**********************************************/
void
mask_invert_proc(item, event)
Panel_item      item;
Event           *event;
{
	mask_op_proc(2);
}

/**********************************************/
void
poly_mask_proc(item, event)
Panel_item      item;
Event           *event;
{
}

/**********************************************/
/* Not implemented
void
set_grow_direction(item, value, event)
Panel_item item;
int value;
Event *event;
{
	grow.direction = value;
}
*/

/**********************************************/
void
set_grow_matrix_type(item, value, event)
Panel_item item;
int value;
Event *event;
{
/* Each matrix has 2 components... therefore, value * 2.... */
	void bind_matrix_width();

	grow.matrix_type = value * 2;
	bind_matrix_width(&grow.matrix_width);

	(void) xv_set(mask_control_win->set_matrix_size,
		PANEL_VALUE, grow.matrix_width - 1,
		NULL);
	panel_paint(mask_control_win->set_matrix_size, PANEL_CLEAR);
}

/**********************************************/
void
set_grow_matrix_size(item, value, event)
Panel_item item;
int value;
Event *event;
{
	void bind_matrix_width();

	grow.matrix_width = value + 1;
	bind_matrix_width(&grow.matrix_width);

	(void) xv_set(mask_control_win->set_matrix_size,
		PANEL_VALUE, grow.matrix_width - 1,
		NULL);
	panel_paint(mask_control_win->set_matrix_size, PANEL_CLEAR);
}

/**********************************************/
void
set_grow_apply_in(item, value, event)
Panel_item item;
int value;
Event *event;
{
	grow.apply_in = value;	
}

/**********************************************/
void
set_grow_extent_3D(item, value, event)
Panel_item item;
int value;
Event *event;
{
	grow.extent_3D = value;	
}

/**********************************************/
void
bind_matrix_width(width)
int *width;
{
	int max_width;

	switch(grow.matrix_type) {
	case GRADIENT :
		max_width = 4;
	break;
	case SOBEL :
	case LAPLACIAN :
		max_width = 1;
	break;
	case KASVAND :
		max_width = 2;
	break;
	}
	if(*width > max_width) *width = max_width;
}

/**********************************************/
void
set_grow_gradient(item, value, event)
Panel_item item;
int value;
Event *event;
{
	grow.gradient = value;
}

/**********************************************/
void
set_grow_threshold_high(item, value, event)
Panel_item item;
int value;
Event *event;
{
	grow.threshold_high = value;
	if(grow.threshold_low > grow.threshold_high) {
		grow.threshold_low = grow.threshold_high;
		(void) xv_set(mask_control_win->grow_threshold_low,
			PANEL_VALUE, grow.threshold_low,
			NULL);
	}
}

/**********************************************/
void
set_grow_threshold_low(item, value, event)
Panel_item item;
int value;
Event *event;
{
	void draw_hint();

	grow.threshold_low = value;
	if(grow.threshold_high < grow.threshold_low) {
		grow.threshold_high = grow.threshold_low;
		(void) xv_set(mask_control_win->grow_threshold_high,
			PANEL_VALUE, grow.threshold_high,
			NULL);
	}
	draw_hint("Get gradient ... then GROW");
}

/***********************************************/
void
portion_set_proc(item, value, event)
Panel_item      item;
int             value;
Event           *event;
{
	region.whole = 0;
	region.region = 0;

	if(value == 0) region.whole = 1;
	if(value == 1) region.region = 1;
}

/**********************************************/
void
draw_hints_grow_region(item, value, event)
Panel_item item;
int value;
{
	void draw_hint();

	int from;

	from = xv_get(mask_control_win->set_region, PANEL_VALUE, NULL);
	switch(from) {
	case 0 : /* From seeds in this frame */
		draw_hint("Set Gradient, Thresholds.  Grow");
		(void) xv_set(mask_control_win->beg_frame,
			PANEL_INACTIVE, TRUE,
			NULL);
		(void) xv_set(mask_control_win->end_frame,
			PANEL_INACTIVE, TRUE,
			NULL);
	break;
	case 1 : /* Begin to end */
		draw_hint("Region Ctrl: Set Beg. & End Frames");
		(void) xv_set(mask_control_win->beg_frame,
			PANEL_INACTIVE, FALSE,
			NULL);
		(void) xv_set(mask_control_win->end_frame,
			PANEL_INACTIVE, FALSE,
			NULL);
	break;
	case 2 : /* To previous */
	case 3 : /* To next */
		draw_hint("If this frame is good, Grow");
		(void) xv_set(mask_control_win->beg_frame,
			PANEL_INACTIVE, TRUE,
			NULL);
		(void) xv_set(mask_control_win->end_frame,
			PANEL_INACTIVE, TRUE,
			NULL);
	break;
	}
}

/**********************************************/
void
scale_matrices()
{
	int i, j;

	for(i = -4; i <= 4; i++)
	for(j = -4; j <= 4; j++) {
		gradient_matrix_x[4 + i][4 + j] *= MATRIX_SCALE_FACTOR;
		gradient_matrix_y[4 + i][4 + j] *= MATRIX_SCALE_FACTOR;
	}
}

/**********************************************/
void
grow_proc(item, event)
Panel_item      item;
Event           *event;
{
	void save_undo();
	void draw_hint();
	int  get_frame_proc();
	void grow_current();
	int  flood_from_current();
	Notify_value grow_next_frame();

	int i, f, q, from;
	int old_segal_curr_frame;

	XSetForeground(display, gc, red_standout);

	from = xv_get(mask_control_win->set_region, PANEL_VALUE, NULL);
	switch(from) {
	case 0 : /* From seeds in this frame */
		draw_hint("Candidate points in Red");
		save_undo();
		grow_current();
		write_mask_frame(1, segal.edit_m);
		draw_hint("Adjust sliders, [clear/plant], grow again");
	break;

	case 1 : /* Beginning frame to ending frame */
		if(segal.changed) write_mask_frame(1, segal.edit_m);

		region.beg_frame = atoi(xv_get(
			mask_control_win->beg_frame, PANEL_VALUE, NULL));
		region.end_frame = atoi(xv_get(
			mask_control_win->end_frame, PANEL_VALUE, NULL));

		if(region.beg_frame <= region.end_frame) {
			frame.offset = 1;
			frame.current = region.beg_frame;
			frame.upper = region.end_frame;
			frame.lower = region.beg_frame;
		}
		else {
			frame.offset = -1;
			frame.current = region.end_frame;
			frame.upper = region.beg_frame;
			frame.lower = region.end_frame;
		}

		if(segal.changed) write_mask_frame(1, segal.edit_m);
		get_frame_proc(item, frame.current, event);

		draw_hint("Press Stop if auto-fill errors");

		itimevalue.it_value.tv_sec = INTERVAL_SEC;
		itimevalue.it_interval.tv_sec = INTERVAL_SEC;
		itimevalue.it_value.tv_usec = INTERVAL_uSEC;
		itimevalue.it_interval.tv_usec = INTERVAL_uSEC;

		notify_set_itimer_func(mask_control_win->win,
			grow_next_frame, ITIMER_REAL, &itimevalue, NULL);
	break;

	case 2 : /* To previous frame */
	case 3 : /* To next frame */
		old_segal_curr_frame = segal.curr_frame;

		if(segal.changed) write_mask_frame(1, segal.edit_m);

		if(from == 2 && segal.curr_frame > 0)
			--segal.curr_frame;
		else if(from == 3 && segal.curr_frame < segal.frames - 1)
			++segal.curr_frame;

		if(old_segal_curr_frame != segal.curr_frame) {
			/* changed */
			(void) xv_set(frame_control_win->curr_frame,
				PANEL_VALUE, segal.curr_frame + 1,
				NULL);
			if(!flood_from_current(item, segal.curr_frame, event)) {
				grow_current();
				write_mask_frame(1, segal.edit_m);
			}
		}
	break;

	default :
	break;
	}
}

/**********************************************/
Notify_value
grow_next_frame()
{
	void draw_hint();
	int flood_from_current();

	fprintf(stderr, "frame %d being processed\n", frame.current + frame.offset);

	(void) xv_set(frame_control_win->curr_frame,
		PANEL_VALUE, frame.current + frame.offset + 1,
		NULL);

	if(flood_from_current(NULL, frame.current + frame.offset, NULL)) {
		notify_set_itimer_func(mask_control_win->win,
			NOTIFY_FUNC_NULL, ITIMER_REAL, NULL, NULL);
		draw_hint("Grow tried to go out of bounds");
		return NOTIFY_DONE;
	}

	if(grow.extent_3D == 1) grow_current();
	write_mask_frame(1, segal.edit_m);

	frame.current += frame.offset;
	/* kinda messy... but hey, this is 'c' :) */
	if(frame.current > frame.upper - frame.offset
	|| frame.current < frame.lower - frame.offset)
		/* turn it off */
		notify_set_itimer_func(mask_control_win->win,
			NOTIFY_FUNC_NULL, ITIMER_REAL, NULL, NULL);
	else {
		draw_hint("Grew in 3-d: check frames");
		return NOTIFY_DONE;
	}
}

/**********************************************/
void
grow_stop_proc(item, event)
Panel_item item;
Event *event;
{
	notify_set_itimer_func(mask_control_win->win,
		NOTIFY_FUNC_NULL, ITIMER_REAL, NULL, NULL);
}

/**********************************************/
void
grow_current()
{
	void draw_filenames();
	void collect_edges();
	void grow_edges();
	void refresh_after_growing();
	void apply_mask_log();

	int i;

	collect_edges();

	for(i = 0; i < num_pts; i++)
		grow_edges(edge_points[i].x, edge_points[i].y);
	apply_mask_log();
	refresh_after_growing();

	segal.changed = 1;
	draw_filenames();
}

/**********************************************/
void
apply_mask_log()
{
/* Adds the inclusive and subtracts the exclusive masks to the edit mask */

	void exclude_from_edit_m();
	void include_with_edit_m();
	int i;

	if(xv_get(mask_log_win->set_which_apply_first, PANEL_VALUE) == 0) {
	/* Exclude, then Include */
		for(i = 0; i < segal.masks; i++)
			if(m[i].mask_type == MASK_EXCLUSIVE)
				exclude_from_edit_m(i);
		for(i = 0; i < segal.masks; i++)
			if(m[i].mask_type == MASK_INCLUSIVE)
				include_with_edit_m(i);
	}
	else {
	/* Include, then Exclude */
		for(i = 0; i < segal.masks; i++)
			if(m[i].mask_type == MASK_INCLUSIVE)
				include_with_edit_m(i);
		for(i = 0; i < segal.masks; i++)
			if(m[i].mask_type == MASK_EXCLUSIVE)
				exclude_from_edit_m(i);
	}
}

/**********************************************/
void
exclude_from_edit_m(index)
int index;
{
	int i, j;

	for(j = 0; j < segal.rows; j++)
	for(i = 0; i < segal.cols; i++)
	if(m[index].data[j][i] != 0)
		work_buf[j][i] = 0;
}

/**********************************************/
void
include_with_edit_m(index)
int index;
{
	int i, j;

	for(j = 0; j < segal.rows; j++)
	for(i = 0; i < segal.cols; i++)
	if(m[index].data[j][i] != 0)
		work_buf[j][i] = PVAL;
}

/**********************************************/
int
flood_from_current(item, to_frame, event)
Panel_item item;
int to_frame;
Event *event;
{
/* Returns 0 if successful, 1 otherwise */
	int get_frame_proc();
	void save_undo();
	int grow_into();
	void refresh_after_growing();

	int i, j;

	if(to_frame < 0 || to_frame > segal.frames - 1) {
		fprintf(stderr, "Can't flood frame %d - does not exist!\n", to_frame);
		return(1);
	}

	/* copy current mask to mask buffer */
	bcopy((char *) work_buf[0], (char *) mask_buf[0],
		segal.rows * segal.cols);

	/* advance to frame */
	/* remember that get_frame_proc takes as actual param desired_frame+1 */
	get_frame_proc(item, to_frame + 1, event);

	/* save before planting seeds */
	save_undo();

	/* traverse the mask_buf, attempting to fill new frame */
	for(j = 0; j < segal.rows; j++)
	for(i = 0; i < segal.cols; i++)
	{ 
		if(mask_buf[j][i] == 255
		&& grow_into(i, j)) work_buf[j][i] = 255;
		else work_buf[j][i] = 0;
	}

	refresh_after_growing();
	return(0);
}

/**********************************************/
void
refresh_after_growing()
{
	map_image_to_lut(work_buf[0], mask_image->data,
		segal.rows * segal.cols);

	if (himage.fp != NULL) {
		blend(himage.data[0], work_buf[0], (u_char *) blend_image->data,
		segal.rows * segal.cols);
	}

	image_repaint_proc();

	if ((int) xv_get(edit_win->win, XV_SHOW, NULL) == TRUE) {
		zoom();
		edit_repaint_proc();
	}
}

/**********************************************/
void
plant_seeds()
{
/* fills in patches where the gradient < MIN_GRAD and values in the image are
 * within SPREAD of the midpoint between the threshold values.
 * NOTE: NOT CURRENTLY USED - REPLACED IN FAVOR OF FILL_FROM_CURRENT
 * 	 Didn't delete because it may come in handy someday - could be an
 *	 option if someone has multiple but very unrelated multiple frames in
 *	 an image.		- Bryan
 */
	int avg_grad_x();
	int avg_grad_y();

	int i, j;
	int low_limit, high_limit;

#define SPREAD		5  /* low and high limits from the avg of thresh vals */
#define DIST		2  /* "radius" to collect points to avg gradient */
#define MIN_GRAD	3  /* minimum change to allow in gradient over SPREAD */ 
	i = (grow.threshold_low + grow.threshold_high) / 2;
	low_limit = i - SPREAD;
	high_limit = i + SPREAD;

	/* fill in x direction */
	for(j = 0; j < segal.rows; j++)
	for(i = DIST; i < segal.cols - DIST - 1; i++)
		if(avg_grad_x(i, j, DIST) < MIN_GRAD
		&& himage.data[j][i] > low_limit
		&& himage.data[j][i] < high_limit)
			work_buf[j][i] = 255;

	/* fill in y direction */
	for(i = 0; i < segal.cols; i++)
	for(j = DIST; j < segal.rows - DIST - 1; j++)
		if(avg_grad_y(i, j, DIST) < MIN_GRAD
		&& himage.data[j][i] > low_limit
		&& himage.data[j][i] < high_limit)
			work_buf[j][i] = 255;
}

/**********************************************/
int
avg_grad_x(x, y, distance)
int x, y, distance;
{
/* NOTE: NOT CURRENTLY CALLED */
	int gradient();

	int i, answer = 0;

	for(i = -1 * distance; i < distance; i++)
		answer += gradient(x + i, y, grow.matrix_width, 0);

	answer /= (double) (2 * distance + 1);
	
	return answer;
}

/**********************************************/
int
avg_grad_y(x, y, distance)
int x, y, distance;
{
/* NOTE: NOT CURRENTLY CALLED */
	int gradient();

	int i, answer = 0;

	for(i = -1 * distance; i < distance; i++)
		answer += gradient(x, y + i, grow.matrix_width, 0);

	answer /= (double) (2 * distance + 1);
	
	return answer;
}

/**********************************************/
void
grow_edges(x, y)
int x, y;
{
	XDrawPoint(display, view_xid, gc, x, y);

	if(grow_right(x, y)) {
		work_buf[y][x + 1] = 255;
		grow_edges(x + 1, y);
	}

	if(grow_left(x, y)) {
		work_buf[y][x - 1] = 255;
		grow_edges(x - 1, y);
	}

	if(grow_up(x, y)) {
		work_buf[y - 1][x] = 255;
		grow_edges(x, y - 1);
	}

	if(grow_down(x, y)) {
		work_buf[y + 1][x] = 255;
		grow_edges(x, y + 1);
	}
}

/**********************************************/
void
collect_edges()
{
/* Eventually, may want to collect edges within a non-rectangular region.  As
 * of now, there are 2 options ... Either the region or the whole.
 */

	void add_to_edge_points();

	int sx, sy, swidth, sheight;
	int i, j, black, white, curr_color;

	num_pts = 0;
	if(edge_points != NULL) free(edge_points);
	edge_points = (POINT_TYPE *) calloc(segal.rows * segal.cols, sizeof(POINT_TYPE));

	black = 0;
	white = 255;

	if(region.region) {
		sx = region.crx1;
		sy = region.cry1;
		swidth = region.crx2 - 1;
		sheight = region.cry2 - 1;
	}
	else {
		sx = 0;
		sy = 0;
		swidth = segal.cols - 1;
		sheight = segal.rows - 1;
	}
	curr_color = work_buf[sy][sx];

	/* collect edge points in the x direction */
	for(j = sy; j < sheight; j++)
		for(i = sx; i < swidth; i++)
			if(curr_color == black &&
			   work_buf[j][i] == white) {
				add_to_edge_points(i, j);
				curr_color = white;
			}
			else if(curr_color == white &&
				work_buf[j][i + 1] == black) {
				add_to_edge_points(i, j);
				curr_color = black;
				i++;
			}
					
	curr_color = work_buf[sy][sx];

	/* collect edge points in the y direction */
	for(i = sx; i < swidth; i++)
		for(j = sy; j < sheight; j++)
			if(curr_color == black &&
			   work_buf[j][i] == white) {
				add_to_edge_points(i, j);
				curr_color = white;
			}
			else if(curr_color == white &&
				work_buf[j + 1][i] == black) {
				add_to_edge_points(i, j);
				curr_color = black;
				j++;
			}
}					

/**********************************************/
void
add_to_edge_points(x, y)
int x, y;
{
	edge_points[num_pts].x = x;
	edge_points[num_pts].y = y;
	num_pts++;
}

/**********************************************/
int
grow_into(x, y)
int x, y;
{
	int gradient();

	if(region.region) {
		if(x < region.crx1) return(0);
	}

	return(gradient(x, y, grow.matrix_width, 2 * grow.matrix_type) < grow.gradient
		&& gradient(x, y, grow.matrix_width, 2 * grow.matrix_type + 1) < grow.gradient
		&& himage.data[y][x] >= grow.threshold_low
		&& himage.data[y][x] <= grow.threshold_high);
}

/**********************************************/
int
grow_left(x, y)
int x, y;
{
	int gradient();

	if(region.region) {
		if(x - 1 < region.crx1) return(0);
	}
	else if(x - 1 <= 0) return(0);

	return(gradient(x - 1, y, grow.matrix_width, 2 * grow.matrix_type)
		< grow.gradient
		&& himage.data[y][x - 1] >= grow.threshold_low
		&& himage.data[y][x - 1] <= grow.threshold_high
		&& !work_buf[y][x - 1]);
}

/**********************************************/
int
grow_right(x, y)
int x, y;
{
	int gradient();

	if(region.region) {
		if(x + 1 > region.crx2) return(0);
	}
	else if(x + 1 >= segal.cols) return(0);

	return(gradient(x + 1, y, grow.matrix_width, 2 * grow.matrix_type)
		< grow.gradient
		&& himage.data[y][x + 1] >= grow.threshold_low
		&& himage.data[y][x + 1] <= grow.threshold_high
		&& !work_buf[y][x + 1]);
}

/**********************************************/
int
grow_up(x, y)
int x, y;
{
	int gradient();

	if(region.region) {
		if(y - 1 < region.cry1) return(0);
	}
	else if(y - 1 <= 0) return(0);

	return(gradient(x, y - 1, grow.matrix_width, 2 * grow.matrix_type + 1)
		< grow.gradient
		&& himage.data[y - 1][x] >= grow.threshold_low
		&& himage.data[y - 1][x] <= grow.threshold_high
		&& !work_buf[y - 1][x]);
}

/**********************************************/
int
grow_down(x, y)
int x, y;
{
	int gradient();

	if(region.region) {
		if(y + 1 > region.cry2) return(0);
	}
	else if(y + 1 >= segal.rows) return(0);

	return(gradient(x, y + 1, grow.matrix_width, 2 * grow.matrix_type + 1)
		< grow.gradient
		&& himage.data[y + 1][x] >= grow.threshold_low
		&& himage.data[y + 1][x] <= grow.threshold_high
		&& !work_buf[y + 1][x]);
}

/**********************************************/
int
gradient(x, y, width, matrix)
int x, y, width, matrix;
{
/* Computes the gradient using a matrix of width X width (max width = 4) 
 * Remember: 9 x 9 matrix has width 4 ... width + 1 + width 
 * The size of matrix adjusts to keep the gradient mask within the boundaries
 * of the image.
 */

	int answer = 0;
	int i, j, k, i_corr_factor = 0, j_corr_factor = 0, k_corr_factor = 0;

	for(i = -1*width + i_corr_factor; i <= width - i_corr_factor; i++) {
		if(!i_corr_factor) while(x + i < 0) {
			i_corr_factor++;
			i++;
		}
		if (x + i >= segal.cols) break;

		for(j = -1*width + j_corr_factor; j <= width - j_corr_factor; j++) {
			if(!j_corr_factor) while(y + j < 0) {
				j_corr_factor++;
				j++;
			}
			if(y + j >= segal.rows) break;

			answer += himage.data[y + j][x + i];
			if(grow.apply_in && frame.stack_rad > 0) {
				if(matrix/2. > 1.) { /* x dir grad */
					answer += stack_buf[grow.matrix_width + 1 + i][y + j][x];
				}
				else { /* y dir grad */
					answer += stack_buf[grow.matrix_width + 1 + j][y][x + i];
				}
			}

			switch(matrix) {
			case GRADIENT :
				answer *= gradient_matrix_x[4 + j][4 + i];
			break;
			case GRADIENT + 1 :
				answer *= gradient_matrix_y[4 + j][4 + i];
			break;
			case SOBEL :
				answer *= sobel_matrix_x[4 + j][4 + i];
			break;
			case SOBEL + 1 :
				answer *= sobel_matrix_y[4 + j][4 + i];
			break;
			case LAPLACIAN :
			case LAPLACIAN + 1 :
				answer *= laplacian_matrix[4 + j][4 + i];
			break;
			case KASVAND :
			case KASVAND + 1 :
				answer *= kasvand_matrix[4 + j][4 + i];
			break;
			}

		}
	}
	if(answer >= 0) return answer;
	else return -1 * answer;
}

/**********************************************/

/**********************************************/

/**********************************************/

/**********************************************/

