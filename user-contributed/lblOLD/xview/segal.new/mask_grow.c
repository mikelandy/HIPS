/*
 *	mask_grow.c - For use with SEGAL
 *
 *	Bryan Skene, LBL July 1991
 *	Rewritten for SEGAL 3d: 7/92
 */

#include "common.h"

static int dx[NUM_DIRECTIONS] = {
	0, 0, -1, 1, 0, 0
	};

static int dy[NUM_DIRECTIONS] = {
	-1, 1, 0, 0, 0, 0
	};

static int df[NUM_DIRECTIONS] = {
	0, 0, 0, 0, 1, -1
	};

static int xoff_ns[8] = {
	0, 1, 1, 1, 0, -1, -1, -1
	};

static int xoff_ew[8] = {
	0, 0, 0, 0, 0, 0, 0, 0
	};

static int xoff_ud[8] = {
	0, 1, 1, 1, 0, -1, -1, -1
	};

static int yoff_ns[8] = {
	0, 0, 0, 0, 0, 0, 0, 0
	};

static int yoff_ew[8] = {
	0, -1, -1, -1, 0, 1, 1, 1
	};

static int yoff_ud[8] = {
	-1, -1, 0, 1, 1, 1, 0, -1
	};

static int foff_ns[8] = {
	-1, -1, 0, 1, 1, 1, 0, -1
	};

static int foff_ew[8] = {
	-1, -1, 0, 1, 1, 1, 0, -1
	};

static int foff_ud[8] = {
	0, 0, 0, 0, 0, 0, 0, 0
	};

static STACK_INFO stack[NUM_BRIDGE_DIRS];
static LOGIC connected[NUM_BRIDGE_DIRS];

/**********************************************/
void
grow_mask()
{
	void set_region();
	void map_buffers();
	int max_frames();
	void grow_frame();
	void set_frame_slider();

	int i;

	/* Things to be done at the beginning of any growth */
	For_all_bridge_strengths {
		if(stack[i].pts != NULL) free(stack[i].pts);

		stack[i].max_pts = NUM_PTS_PER_ALLOC; 
		stack[i].num_pts_per_alloc = NUM_PTS_PER_ALLOC;
		stack[i].pts = (POINT_TYPE *) calloc(stack[i].max_pts, sizeof(POINT_TYPE));
		stack[i].num_pts = 0;
	}

	switch(grow.extent) {
	case GROW_FRAME :
		set_region(win[grow.swin].aspect, 0, 0, win[grow.swin].f,
			win[grow.swin].img_c - 1, win[grow.swin].img_r - 1,
			win[grow.swin].f);
		grow_frame();
		break;

	case GROW_BEG_TO_END :
		/* setup interrupt growth parameters */
		win[grow.swin].f = region.beg_frame;
		win[grow.swin].repaint = TRUE;
		map_buffers();

		if(region.beg_frame <= region.end_frame) {
			frame.upper = region.end_frame;
			frame.lower = region.beg_frame;
		}
		else {
			frame.upper = region.beg_frame;
			frame.lower = region.end_frame;
		}
		set_region(win[grow.swin].aspect, 0, 0, frame.lower,
			win[grow.swin].img_c - 1, win[grow.swin].img_r - 1,
			frame.upper);
		grow_frame();
		break;

	case GROW_ALL :
		set_region(win[grow.swin].aspect, 0, 0, 0,
			win[grow.swin].img_c - 1, win[grow.swin].img_r - 1,
			max_frames(win[grow.swin].aspect));
		grow_frame();
		break;

	default :
	break;
	}
}

/**********************************************/
void
done_growing_mask()
{
	void redisplay_all();

	int i, p;

	For_all_bridge_strengths {
		vprint"freeing stack[%d] (size = %u, points = %u)\n", i,
			(p = pointer_buffer_size(stack[i].pts)), p / 6);
		free(stack[i].pts);
		stack[i].pts = NULL;
	}

	xv_set(Mask_grow_pop_mask_grow->but_stop,
		PANEL_INACTIVE, TRUE,
		NULL);
		 
	xv_set(Mask_grow_pop_mask_grow->but_continue,
		PANEL_INACTIVE, TRUE,
		NULL);

	if(grow.extent == GROW_BEG_TO_END) {
		win[grow.swin].f = region.end_frame;
		set_frame_slider(grow.swin);
	}

	redisplay_all();
}

/**********************************************/
void
set_region(asp_id, x1, y1, f1, x2, y2, f2)
int asp_id, x1, y1, f1, x2, y2, f2;
{
	void get_3d_coords();
	void organize_region();

	get_3d_coords(asp_id, ASPECT_Z, x1, y1, f1,
		&region.x1, &region.y1, &region.f1);
	get_3d_coords(asp_id, ASPECT_Z, x2, y2, f2,
		&region.x2, &region.y2, &region.f2);

	organize_region();
}

/**********************************************/
void
organize_region()
{
	void swap();

	if(region.x1 > region.x2) swap(&region.x1, &region.x2);
	if(region.y1 > region.y2) swap(&region.y1, &region.y2);
	if(region.f1 > region.f2) swap(&region.f1, &region.f2);
}

/**********************************************/
void
swap(s1, s2)
int *s1, *s2;
{
	int t;

	t = *s1;
	*s1 = *s2;
	*s2 = t;
}	

/**********************************************/
void
grow_frame()
{
	void save_mask_undo_2d();
	void save_mask_undo_3d();
	void collect_frame();
	void go_grow_edges();


	if(grow.extent == GROW_FRAME) save_mask_undo_2d(grow.swin);
	else save_mask_undo_3d(segal.e_m);

	vprint"Collecting seed points ...\n");
	collect_frame(grow.seed_pt_src);

	go_grow_edges();
}

/**********************************************/
void
go_grow_edges()
{
	Notify_value grow_edges();

	vprint"Growing edges ...\n");
	if(grow.interractive) {
		grow.itimer.it_value.tv_sec = INTERVAL_SEC;
		grow.itimer.it_interval.tv_sec = INTERVAL_SEC;
		grow.itimer.it_value.tv_usec = INTERVAL_uSEC;
		grow.itimer.it_interval.tv_usec = INTERVAL_uSEC;
		notify_set_itimer_func(Mask_grow_pop_mask_grow->pop_mask_grow,
			grow_edges, ITIMER_REAL, &grow.itimer, NULL);
	}
	else while(grow_edges() != NOTIFY_DONE);
}

/**********************************************/
void
collect_frame(pt_src)
int pt_src;
{
/* puts all the points from pt_src (Edit Mask or Pt List) on the stack */
	void get_3d_coords();
	void push();

	int buf_bit, x, y, x1, y1, f1;

	if(pt_src == SEED_EDIT) buf_bit = segal.e_m;
	else if(pt_src == SEED_PTS) buf_bit = BUF_PTS;

	grow.stack_empty = TRUE;

	for(x = 0; x < win[grow.swin].img_c - 1; x++)
	for(y = 0; y < win[grow.swin].img_r - 1; y++)
		if(BIT_IS_ON(win[grow.swin].m_data[y][x], m[buf_bit].bit_key)) {
			get_3d_coords(win[grow.swin].aspect, ASPECT_Z,
				x, y, win[grow.swin].f,
				&x1, &y1, &f1);
			push(NUM_BRIDGE_DIRS - 1, x1, y1, f1);
		}
	vprint"Collection found %d points\n", stack[NUM_BRIDGE_DIRS - 1].num_pts);
}

/**********************************************/
void
auto_set_grow_params(pt_src)
int pt_src;
{
/* find mins and maxes */
	void erase_thresh_bounds();
	void draw_thresh_bounds();
	void set_grow_params();

	int buf_bit, x, y, x1, y1, f1, g, pval, i;
	LOGIC set_orig_vals, b_satisfied;

	set_watch_cursor();

	if(pt_src == SEED_EDIT) buf_bit = segal.e_m;
	else if(pt_src == SEED_PTS) buf_bit = BUF_PTS;

	set_orig_vals = FALSE;

	for(x = 0; x < win[grow.swin].img_c - 1; x++)
	for(y = 0; y < win[grow.swin].img_r - 1; y++)
		if(BIT_IS_ON(win[grow.swin].m_data[y][x], m[buf_bit].bit_key)) {
			/* set initial values */
			get_3d_coords(win[grow.swin].aspect, ASPECT_Z,
				x, y, win[grow.swin].f,
				&x1, &y1, &f1);

			if(grow.apply_thresholds) {
				if(segal.color)
					pval = MONO(cbuf[RP][f1][y1][x1],
						cbuf[GP][f1][y1][x1],
						cbuf[BP][f1][y1][x1]);
				else pval = ibuf[f1][y1][x1];
				grow.threshold_min = grow.threshold_max = pval;
			}

			if(grow.apply_gradient) {
				grow.gradient_min = grow.gradient_max
					= gradient(x1, y1, f1);
			}

			if(grow.apply_bridge) {
				grow.bridge_max = NUM_BRIDGE_DIRS;
				grow.bridge_min = NUM_BRIDGE_DIRS;
			}

			set_orig_vals = TRUE;
			break;
		}

	if(!set_orig_vals) {
		unset_watch_cursor();
		return; /* empty pt_src */
	}

	for(x = 0; x < win[grow.swin].img_c - 1; x++)
	for(y = 0; y < win[grow.swin].img_r - 1; y++)
		if(BIT_IS_ON(win[grow.swin].m_data[y][x], m[buf_bit].bit_key)) {
			get_3d_coords(win[grow.swin].aspect, ASPECT_Z,
				x, y, win[grow.swin].f,
				&x1, &y1, &f1);

			if(grow.apply_thresholds) {
				if(segal.color)
					pval = MONO(cbuf[RP][f1][y1][x1],
						cbuf[GP][f1][y1][x1],
						cbuf[BP][f1][y1][x1]);
				else pval = ibuf[f1][y1][x1];

				if(pval < grow.threshold_min)
					grow.threshold_min = pval;
				if(pval > grow.threshold_max)
					grow.threshold_max = pval;
			}

			if(grow.apply_gradient) {
				g = gradient(x1, y1, f1);
				if(g < grow.gradient_min) grow.gradient_min = g;
				if(g > grow.gradient_max) grow.gradient_max = g;
			}

			if(grow.apply_bridge) {
			  b_satisfied = FALSE;
			  while(!b_satisfied) {
				For_all_directions
					if(bridge_strength(x1, y1, f1, i)
					>= grow.bridge_min) {
						b_satisfied = TRUE;
						break;
					}
				if(!b_satisfied) {
					if(grow.bridge_min > 1) {
						grow.bridge_min--;
					}
					else {
						grow.bridge_min = NUM_BRIDGE_DIRS;
						if(grow.bridge_dist > 1) {
							grow.bridge_dist--;
						}
						else {
							/* get outta here! */
							vprint"***ERROR***\n");
							b_satisfied = TRUE;
						}
					}
				}
			  }
			}
		}

	erase_thresh_bounds();
	threshold.min = grow.threshold_min;
	threshold.max = grow.threshold_max;
	draw_thresh_bounds();
	set_grow_params();

	unset_watch_cursor();
}

/**********************************************/
void
set_grow_params()
{
	xv_set(Mask_grow_pop_mask_grow->set_threshold_min,
		PANEL_VALUE, grow.threshold_min,
		PANEL_INACTIVE, (grow.apply_thresholds == FALSE),
		NULL);
	xv_set(Mask_grow_pop_mask_grow->set_threshold_max,
		PANEL_VALUE, grow.threshold_max,
		PANEL_INACTIVE, (grow.apply_thresholds == FALSE),
		NULL);
	xv_set(Mask_grow_pop_mask_grow->set_gradient_radius,
		PANEL_VALUE, grow.gradient_rad,
		PANEL_INACTIVE, (grow.apply_gradient == FALSE),
		NULL);
	xv_set(Mask_grow_pop_mask_grow->set_gradient_max,
		PANEL_VALUE, grow.gradient_max,
		PANEL_INACTIVE, (grow.apply_gradient == FALSE),
		NULL);
	xv_set(Mask_grow_pop_mask_grow->set_gradient_min,
		PANEL_VALUE, grow.gradient_min,
		PANEL_INACTIVE, (grow.apply_gradient == FALSE),
		NULL);
	xv_set(Mask_grow_pop_mask_grow->set_bridge_distance,
		PANEL_VALUE, grow.bridge_dist,
		PANEL_INACTIVE, (grow.apply_bridge == FALSE),
		NULL);
	xv_set(Mask_grow_pop_mask_grow->set_bridge_max,
		PANEL_VALUE, grow.bridge_max,
		PANEL_INACTIVE, (grow.apply_bridge == FALSE),
		NULL);
	xv_set(Mask_grow_pop_mask_grow->set_bridge_min,
		PANEL_VALUE, grow.bridge_min,
		PANEL_INACTIVE, (grow.apply_bridge == FALSE),
		NULL);
}

/**********************************************/
Notify_value
grow_edges()
{
/* all points are in the Z aspect */
	void done_growing_mask();
	u_long standout();
	int pop();
	void push();
	void get_3d_coords();
	void paint_cell();
	void paint_view_cells();
	LOGIC bit_is_included();
	LOGIC bit_is_excluded();
	LOGIC grow_into();
	LOGIC shrink_into();

	int i, j, s, x, y, f, x1, y1, f1;

	if(grow.stack_empty) {
		if(grow.interractive) notify_set_itimer_func(
			Mask_grow_pop_mask_grow->pop_mask_grow,
			NOTIFY_FUNC_NULL, ITIMER_REAL, NULL, NULL);
		done_growing_mask();
		return NOTIFY_DONE;
	}

	for(j = 0; j < grow.speed; j++) {
		if(grow.stack_empty) {
			if(!grow.interractive) {
				done_growing_mask();
				return NOTIFY_DONE;
			}
			else break;
		}

		/* some points left ... */
		s = pop(&x, &y, &f);
		if(grow.disp_growth >= DISP_GROW) {
			/* translate (x, y, f) from Z to grow.swin aspect */
			get_3d_coords(ASPECT_Z, win[grow.swin].aspect, x, y, f,
				&x1, &y1, &f1);
			if(f1 == win[grow.swin].f)
				paint_cell(grow.swin, x1, y1, standout(s));
			if(grow.disp_growth == DISP_ALL)
				paint_view_cells(ASPECT_Z, x, y, f, standout(s));
		}

		if(grow.direction == DIR_GROW) {
			For_all_directions {
				x1 = x + dx[i];
				y1 = y + dy[i];
				f1 = f + df[i];

				if(x1 <= region.x2
				&& y1 <= region.y2
				&& f1 <= region.f2
				&& x1 >= region.x1
				&& y1 >= region.y1
				&& f1 >= region.f1
				&& !bit_is_excluded(mbuf[f1][y1][x1])
				&& grow_into(i, x1, y1, f1, &s)) {
					TURN_BIT_ON(mbuf[f1][y1][x1], m[segal.e_m].bit_key)
					push(s - 1, x1, y1, f1);
				}
			}
		}
		else {
			For_all_directions {
				x1 = x + dx[i];
				y1 = y + dy[i];
				f1 = f + df[i];

				if(x1 <= region.x2
				&& y1 <= region.y2
				&& f1 <= region.f2
				&& x1 >= region.x1
				&& y1 >= region.y1
				&& f1 >= region.f1
				&& !bit_is_included(mbuf[f1][y1][x1])
				&& shrink_into(i, x1, y1, f1, &s)) {
					TURN_BIT_OFF(mbuf[f1][y1][x1], m[segal.e_m].bit_key)
					push(s - 1, x1, y1, f1);
				}
			}
		}
	}
}

/**********************************************/
int
pop(x, y, f)
int *x, *y, *f;
{
/* pops from the priority stacks ... 7 first, 0 last */
	void decrease_stack_space();

	int i;

	for(i = NUM_BRIDGE_DIRS - 1; i >=0; i--)
		if(stack[i].num_pts > 0) {
			*x = (int) stack[i].pts[stack[i].num_pts - 1].x;
			*y = (int) stack[i].pts[stack[i].num_pts - 1].y;
			*f = (int) stack[i].pts[stack[i].num_pts - 1].f;
			stack[i].num_pts--;

			if(stack[i].num_pts <
			(stack[i].max_pts - stack[i].num_pts_per_alloc))
				decrease_stack_space(i);
			return(i);
		}
	grow.stack_empty = TRUE;
	/* CHANGE - should return UNDEFINED */
	return(0);
}

/**********************************************/
void
push(s, x, y, f)
int s, x, y, f;
{
	LOGIC increased_stack_space();

	if(stack[s].num_pts >= stack[s].max_pts)
		while(!increased_stack_space(s));

	stack[s].pts[stack[s].num_pts].x = (unsigned short) x;
	stack[s].pts[stack[s].num_pts].y = (unsigned short) y;
	stack[s].pts[stack[s].num_pts].f = (unsigned short) f;
	stack[s].num_pts++;
	grow.stack_empty = FALSE;
}

/**********************************************/
LOGIC
increased_stack_space(s)
int s;
{
	POINT_TYPE *ptr;

	if((ptr = (POINT_TYPE *) realloc(stack[s].pts,
		(stack[s].max_pts + stack[s].num_pts_per_alloc)
		* sizeof(POINT_TYPE))) == NULL) {
		if(stack[s].num_pts_per_alloc < NUM_PTS_PER_ALLOC / 32) {
			vprint"Ran out of memory for the stack ... throwing away half the points!!!\n");
			stack[s].num_pts /= 2;
			stack[s].num_pts_per_alloc = NUM_PTS_PER_ALLOC;
			return(TRUE);
		}
		else {
			vprint"... using smaller stack increment\n");
			stack[s].num_pts_per_alloc /= 2;
			return(FALSE);
		}
	}
	else {
		stack[s].pts = ptr;
		stack[s].max_pts += stack[s].num_pts_per_alloc;
		vprint"Reallocating stack[%d] space ... max_pts up %d to %d\n",
			s, stack[s].num_pts_per_alloc, stack[s].max_pts);
		return(TRUE);
	}
}

/**********************************************/
void
decrease_stack_space(s)
int s;
{
	POINT_TYPE *ptr;

	if((ptr = (POINT_TYPE *) realloc(stack[s].pts, (stack[s].max_pts
		- stack[s].num_pts_per_alloc) * sizeof(POINT_TYPE))) != NULL) {
		stack[s].pts = ptr;
		stack[s].max_pts -= stack[s].num_pts_per_alloc;
		stack[s].num_pts_per_alloc = NUM_PTS_PER_ALLOC;
		vprint"Reallocating stack[%d] space ... max_pts down %d to %d\n", s, stack[s].num_pts_per_alloc, stack[s].max_pts);
	}
}

/**********************************************/
LOGIC
flood_criteria_satisfied(x, y, f)
int x, y, f;
{
	int gradient();

	int g, pval;

	g = gradient(x, y, f);
	if(segal.color) pval = MONO(cbuf[RP][f][y][x], cbuf[GP][f][y][x],
		cbuf[BP][f][y][x]);
	else pval = ibuf[f][y][x];

	return(pval <= grow.threshold_max
	&& pval >= grow.threshold_min
	&& g <= grow.gradient_max
	&& g >= grow.gradient_min);
}

/**********************************************/
LOGIC
grow_into(growth_dir, x, y, f, b)
int growth_dir, x, y, f, *b;
{
	LOGIC flood_criteria_satisfied();
	int bridge_strength();

	return(!BIT_IS_ON(mbuf[f][y][x], m[segal.e_m].bit_key)
	&& flood_criteria_satisfied(x, y, f)
	&& (*b = bridge_strength(x, y, f, growth_dir)) <= grow.bridge_max
	&& *b >= grow.bridge_min);
}

/**********************************************/
LOGIC
shrink_into(growth_dir, x, y, f, b)
int growth_dir, x, y, f, *b;
{
	LOGIC flood_criteria_satisfied();
	int bridge_strength();

	return(BIT_IS_ON(mbuf[f][y][x], m[segal.e_m].bit_key)
	&& flood_criteria_satisfied(x, y, f, growth_dir)
	&& (*b = bridge_strength(x, y, f, growth_dir)) <= grow.bridge_max
	&& *b >= grow.bridge_min);
}

/**********************************************/
int
gradient(x, y, f)
int x, y, f;
{
/* The gradient is really the distance between the average color of the
 * neighborhood of x,y,f and the color at x,y,f itself.
 */
	int num_neighbors, ans, ans_r, ans_g, ans_b;
	int x0, y0, f0, x1, y1, f1;
	int i, j, k;

	if(!grow.apply_gradient) return(grow.gradient_min);

	ans = ans_r = ans_g = ans_b = 0;

	x0 = x - grow.gradient_rad;
	y0 = y - grow.gradient_rad;
	f0 = f - grow.gradient_rad;
	x1 = x + grow.gradient_rad;
	y1 = y + grow.gradient_rad;
	f1 = f + grow.gradient_rad;

	RANGE(x0, 0, (segal.c - 1))
	RANGE(y0, 0, (segal.r - 1))
	RANGE(f0, 0, (segal.f - 1))
	RANGE(x1, 0, (segal.c - 1))
	RANGE(y1, 0, (segal.r - 1))
	RANGE(f1, 0, (segal.f - 1))

	num_neighbors = (f1 - f0) * (y1 - y0) * (x1 - x0);

	if(segal.color) {
		for(i = x0; i <= x1; i++)
		for(j = y0; j <= y1; j++)
		for(k = f0; k <= f1; k++)
			if(i != x && j != y && k != f) {
				ans_r += cbuf[RP][k][j][i];
				ans_g += cbuf[GP][k][j][i];
				ans_b += cbuf[BP][k][j][i];
			}
		ans_r = ans_r / num_neighbors;
		ans_g = ans_g / num_neighbors;
		ans_b = ans_b / num_neighbors;
		return(DISTANCE(ans_r, ans_g, ans_b,
			cbuf[RP][f][y][x], cbuf[GP][f][y][x],
			cbuf[BP][f][y][x]));
	}
	else {
		for(i = x0; i <= x1; i++)
		for(j = y0; j <= y1; j++)
		for(k = f0; k <= f1; k++)
			if(i != x && j != y && k != f) {
				ans += ibuf[k][j][i];
			}
		ans = ans / num_neighbors;
		return(abs(ans - ibuf[f][y][x]));
	}

}

/**********************************************/
int
bridge_strength(x, y, f, growth_dir)
int x, y, f, growth_dir;
{
/* Returns true only if there exists a set of adjacent pixels "grow.min_bridge"
 * long for which the flooding criteria is satisfied (threshold, gradient)
 */
	LOGIC flood_criteria_satisfied();

	int *xoff, *yoff, *foff;
	int i, j, x1, y1, f1;
	int last_connected, num_connections;

	if(!grow.apply_bridge) return grow.bridge_min;

	for(i = 0; i < NUM_BRIDGE_DIRS; i++)
		connected[i] = TRUE;

	switch(growth_dir) {
	case DIR_N :
	case DIR_S :
		xoff = xoff_ns;
		yoff = yoff_ns;
		foff = foff_ns;
		break;
	case DIR_E :
	case DIR_W :
		xoff = xoff_ew;
		yoff = yoff_ew;
		foff = foff_ew;
		break;
	case DIR_U :
	case DIR_D :
		xoff = xoff_ud;
		yoff = yoff_ud;
		foff = foff_ud;
		break;
	default :
		break;
	}

	for(i = 1; i <= grow.bridge_dist; i++)
	for(j = 0; j < NUM_BRIDGE_DIRS; j++) { 
		x1 = x + (xoff[j] * i);
		y1 = y + (yoff[j] * i);
		f1 = f + (foff[j] * i);

		/* if we`re not done yet and have gone out of bounds */
		if(x1 > segal.c - 1
		|| y1 > segal.r - 1
		|| f1 > segal.f - 1
		|| x1 < 0
		|| y1 < 0
		|| f1 < 0)
			connected[j] = FALSE;

		/* if we are in bounds but at a non-candidate pixel */
		if(connected[j]
		&& !flood_criteria_satisfied(x1, y1, f1))
			connected[j] = FALSE;
	}

	/* see how many adjacent connections there are */
	last_connected = NUM_BRIDGE_DIRS;
	num_connections = 0;

	for(i = 0; i < NUM_BRIDGE_DIRS; i++)
		if(connected[i]) num_connections++;
		else if(num_connections > 0) {
			last_connected = i - 1;
			break;
		}

	if(connected[0] && connected[NUM_BRIDGE_DIRS - 1])
	/* then check from the other direction ... they're adjacent */
	for(i = NUM_BRIDGE_DIRS - 1; i > last_connected; i--)
		if(connected[i]) num_connections++;
		else break;


	return num_connections;
}
