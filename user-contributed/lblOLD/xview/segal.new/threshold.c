/*
 *	threshold.c
 *
 *	Bryan Skene, LBL July 1991
 *	part of 'segal'
 *
 *	Thresholding a grayscale image is pretty easy.  Thresholding a color
 *	image involves a lot more user-input and processing.  To threshold a
 *	color image, we define a closed surface in RGB space, call it S,
 *	which contains the colors represented in some region of interest.
 *	Any voxel with a color in S satisfies the threshold; others don't.  One
 *	alternative approach is to build just some list L of colors in some
 *	representative region of interest (cropped box perhaps) and then test
 *	each voxel's color for membership in L.  I will implement both methods
 *	and see which seems to be most useful.
 *
 *	BUT!  Before I do any of this, I will see what results I get by
 *	simply thresholding on the MONO(r,g,b) value (= intensity).
 */

#include "common.h"

#define PANELS 32

int hsum, hmu;

int histo[256];
int num_panels;
byte panel[PANELS];
byte map[256];

/***********************************************/
void
refresh_histogram()
{
	void generate_histogram();
	void draw_histogram();
	void draw_thresh_bounds();
	void generate_histo_stats();

	generate_histogram();
	draw_histogram();
	draw_thresh_bounds();
	generate_histo_stats();
}

/***********************************************/
void
generate_histogram()
{
	void traverse_region();
	void add_to_histogram();
	void scale_histogram();

	int i;

	for(i = 0; i < 256; i++) {
		histo[i] = 0;
	}

	traverse_region(threshold.roi, 0, 0, win[WIN_PAINT].f,
		add_to_histogram);

	scale_histogram();
}

/***********************************************/
void
add_to_histogram(x, y, f)
int x, y, f;
{
	if(segal.color) {
		if(threshold.plane == VAL_RGB)
			histo[MONO(cbuf[RP][f][y][x], cbuf[GP][f][y][x],
				cbuf[BP][f][y][x])]++;
		else histo[cbuf[threshold.plane][f][y][x]]++;
	}
	else histo[ibuf[f][y][x]]++;
}

/***********************************************/
void
scale_histogram()
{
	char foo[14];
	int i, min, max, max_i, old_max, delta;
#define LEAVE_OUT_ZERO

#ifdef LEAVE_OUT_ZERO
#define BEG_VALUE 1
#else
#define BEG_VALUE 0
#endif

	/* sum pre-scaled histogram */
	hsum = 0;
	for(i = BEG_VALUE; i < 256; i++) hsum += histo[i];

	hmu = hsum / PANELS;

	/* everything greater than 4*hmu becomes 4*hmu */
	for(i = BEG_VALUE; i < 256; i++)
		if(histo[i] > 4*hmu) histo[i] = 4 * hmu;

	/* scale histogram */
	max_i = 0;
	min = max = old_max = histo[0];
	for(i = BEG_VALUE; i < 256; i++) {
		if(histo[i] < min) min = histo[i];
		if(histo[i] > max) {
			old_max = max;
			max_i = i;
			max = histo[i];
		}
	}

	if(max - old_max > old_max) {
		histo[max_i] = old_max;
		max = old_max;
	}

	/* label the histogram with the "scaled" max value */
	sprintf(foo, "%d", max);
	xv_set(Threshold_pop_threshold->msg_max_histo,
		PANEL_LABEL_STRING, foo,
		NULL);

	delta = max - min;

	for(i = 0; i < 256; i++)
		histo[i] = (int) ((float) 256 * (float) histo[i] /
			(float) delta);

	/* sum post-scaled histogram */
	hsum = 0;
	for(i = BEG_VALUE; i < 256; i++) hsum += histo[i];

	hmu = hsum / PANELS;
}

/***********************************************/
void
draw_histogram()
{
	unsigned long standout();

	int i;

	XClearArea(display, threshold.xid, 0, 0, 0, 0, FALSE);
	XSetForeground(display, gc, standout(TURQUOISE));
	for(i = 0; i < 256; i++) {
		XDrawLine(display, threshold.xid, gc,
			i, 255, i, 255 - histo[i]);
	}

	xv_set(Threshold_pop_threshold->msg_mu,
		XV_SHOW, FALSE,
		NULL);
}

/***********************************************/
void
erase_thresh_bounds()
{
	XSetForeground(display, gc, standout(CWHITE));
	XDrawLine(display, threshold.xid, gc,
		threshold.min, 0, threshold.min, 255 - histo[threshold.min]);
	XDrawLine(display, threshold.xid, gc,
		threshold.max, 0, threshold.max, 255 - histo[threshold.max]);

	XSetForeground(display, gc, standout(TURQUOISE));
	XDrawLine(display, threshold.xid, gc,
		threshold.min, 255 - histo[threshold.min], threshold.min, 255);
	XDrawLine(display, threshold.xid, gc,
		threshold.max, 255 - histo[threshold.max], threshold.max, 255);
}

/***********************************************/
void
draw_thresh_bounds()
{
	char foo[20];
	int min_color, max_color;

	sprintf(foo, "Max: %d", threshold.max);
	xv_set(Threshold_pop_threshold->msg_max,
		PANEL_LABEL_STRING, foo,
		NULL);
	sprintf(foo, "Min: %d", threshold.min);
	xv_set(Threshold_pop_threshold->msg_min,
		PANEL_LABEL_STRING, foo,
		NULL);

	if(threshold.min == threshold.max) {
		min_color = max_color = CBLACK;
	}
	else {
		min_color = RED;
		max_color = GREEN;
	}

	XSetForeground(display, gc, standout(min_color));
	XDrawLine(display, threshold.xid, gc,
		threshold.min, 0, threshold.min, 255 - histo[threshold.min]);
	XSetForeground(display, gc, standout(max_color));
	XDrawLine(display, threshold.xid, gc,
		threshold.max, 0, threshold.max, 255 - histo[threshold.max]);

	XSetForeground(display, gc, standout(PURPLE));
	XDrawLine(display, threshold.xid, gc,
		threshold.min, 255 - histo[threshold.min], threshold.min, 255);
	XDrawLine(display, threshold.xid, gc,
		threshold.max, 255 - histo[threshold.max], threshold.max, 255);
}

/***********************************************/
void
generate_histo_stats()
{
/* it may be useful to replace BEG_VALUE w/ lower thresh and 256 w/ upper */
#define LEAVE_OUT_ZERO

#ifdef LEAVE_OUT_ZERO
#define BEG_VALUE 1
#else
#define BEG_VALUE 0
#endif

	int i, j, sum, pan_w, sx0, sx1, dx0;

	pan_w = 256 / PANELS;

	/* build panel[] = array of panel's rightmost slice x-coords */
	num_panels = 0;
	sum = 0;
	i = BEG_VALUE;
	while(i < 256 && num_panels < PANELS) {
		sum += histo[i];
		if(sum > hmu) {
			if(sum-hmu >= histo[i]/2
			&& i > 1
			&& ((num_panels > 0 && panel[num_panels - 1] != i - 1)
			  || num_panels == 0)) {
				/* give this slice to the next panel */
				panel[num_panels] = i - 1;
			}
			else {
				/* give this slice to this panel */
				panel[num_panels] = i;
				i++;
			}
			vprint"panel[%2d] = %d\n",
				num_panels, panel[num_panels]);

			num_panels++;
			sum = 0;
		}
		else i++;
	}

	/* make mappings for panels */
	for(i = 0; i < num_panels; i++) {

		/* source */
		if(i > 0) sx0 = panel[i - 1];
		else sx0 = 0;

		sx1 = panel[i];

		/* destination */
		dx0 = i * pan_w;

		for(j = sx0; j < sx1; j++)
			map[j] = dx0 + (int)(((float)(j - sx0) / (float)(sx1 - sx0)) * (float)pan_w);
	}
}

/***********************************************/
void
draw_histo_stats()
{
	int i, y0;
	char foo[20];

	/* draw mu */
	XSetForeground(display, gc, standout(CBLACK));
	XDrawLine(display, threshold.xid, gc,
		0, 255 - hmu, 255, 255 - hmu);

	y0 = xv_get(Threshold_pop_threshold->canvas, XV_Y, NULL);
	sprintf(foo, "Mu = %d\n", hmu);
	xv_set(Threshold_pop_threshold->msg_mu,
		XV_SHOW, TRUE,
		XV_Y, y0 + 255 - hmu,
		PANEL_LABEL_STRING, foo,
		NULL);

	/* draw panels */
	XSetForeground(display, gc, standout(ORANGE));
	for(i = 0; i < num_panels; i++)
		XDrawLine(display, threshold.xid, gc,
			panel[i], 255, panel[i], 255 - histo[panel[i]]);
}

/***********************************************/
void
histoeq()
{
	void save_image_undo();
	void eq_i_pixel();
	void refresh_histogram();
	void redisplay_all();

	if(threshold.roi == R2d_WHOLE
	|| threshold.roi == R2d_CROP
	|| threshold.roi == R2d_PT_LIST)
		save_image_undo(WIN_PAINT);

	/* map the image to it's new values */
	traverse_region(threshold.roi, 0, 0, win[WIN_PAINT].f, eq_i_pixel);
	img.changed_frame = TRUE;
	refresh_histogram();
	redisplay_all();
}

/***********************************************/
void
eq_i_pixel(x, y, f)
int x, y, f;
{
	if(segal.color) {
		if(threshold.plane == VAL_RGB) {
			cbuf[RP][f][y][x] += (11./32.)
				* (map[cbuf[RP][f][y][x]] - cbuf[RP][f][y][x]);
			cbuf[GP][f][y][x] += (16./32.)
				* (map[cbuf[GP][f][y][x]] - cbuf[RP][f][y][x]);
			cbuf[BP][f][y][x] += (5./32.)
				* (map[cbuf[BP][f][y][x]] - cbuf[RP][f][y][x]);
		}
	else cbuf[threshold.plane][f][y][x]
		= map[cbuf[threshold.plane][f][y][x]];
	}
	else ibuf[f][y][x] = map[ibuf[f][y][x]];
}

/***********************************************/
void
threshold_mask()
{
	void traverse_region();
	void thresh_m_pixel();
	void redisplay_all();

	traverse_region(threshold.roi, 0, 0, win[WIN_PAINT].f, thresh_m_pixel);
	m[segal.e_m].changed_frame = TRUE;
	redisplay_all();
}

/***********************************************/
void
thresh_m_pixel(x, y, f)
int x, y, f;
{
	int pval;

	if(segal.color) {
		if(threshold.plane == VAL_RGB)
			pval = MONO(cbuf[RP][f][y][x], cbuf[GP][f][y][x],
				cbuf[BP][f][y][x]);
		else pval = cbuf[threshold.plane][f][y][x];
	}
	else pval = ibuf[f][y][x];

	if(pval >= threshold.min
	&& pval <= threshold.max)
		switch(threshold.mask_effect) {
		case THRESH_OVERWRITE :
		case THRESH_ADD_TO :
			TURN_BIT_ON(mbuf[f][y][x], m[segal.e_m].bit_key)
			break;
		case THRESH_REMOVE_FROM :
			TURN_BIT_OFF(mbuf[f][y][x], m[segal.e_m].bit_key)
			break;
		default :
			break;
		}
	else switch(threshold.mask_effect) {
		case THRESH_OVERWRITE :
			TURN_BIT_OFF(mbuf[f][y][x], m[segal.e_m].bit_key)
			break;
		case THRESH_REMOVE_FROM :
		case THRESH_ADD_TO :
		default :
			break;
	}
}

/***********************************************/
void
threshold_image()
{
	void save_image_undo();
	void traverse_region();
	void thresh_i_pixel();
	void refresh_histogram();
	void redisplay_all();

	if(threshold.roi == R2d_WHOLE
	|| threshold.roi == R2d_CROP
	|| threshold.roi == R2d_PT_LIST)
		save_image_undo(WIN_PAINT);

	traverse_region(threshold.roi, 0, 0, win[WIN_PAINT].f, thresh_i_pixel);
	img.changed_frame = TRUE;
	refresh_histogram();
	redisplay_all();
}

/***********************************************/
void
thresh_i_pixel(x, y, f)
int x, y, f;
{
#define BIND_PVAL(p,d) {pval=p+d; RANGE(pval,0,255) p=pval;}
	int pval;

	if(segal.color) {
		if(threshold.plane == VAL_RGB)
			pval = MONO(cbuf[RP][f][y][x], cbuf[GP][f][y][x],
				cbuf[BP][f][y][x]);
		else pval = cbuf[threshold.plane][f][y][x];
	}
	else pval = ibuf[f][y][x];

	if(pval >= threshold.min
	&& pval <= threshold.max) {
		if(segal.color) {
			if(threshold.plane == VAL_RGB) {
				BIND_PVAL(cbuf[RP][f][y][x],
					threshold.image_effect)
				BIND_PVAL(cbuf[GP][f][y][x],
					threshold.image_effect)
				BIND_PVAL(cbuf[BP][f][y][x],
					threshold.image_effect)
			}
			else BIND_PVAL(cbuf[threshold.plane][f][y][x],
				threshold.image_effect)
		}
		else BIND_PVAL(ibuf[f][y][x], threshold.image_effect)
	}
#undef BIND_PVAL
}

/***********************************************/
void
thresh_event(event)
Event *event;
{
/* mouse events:
 *	left button  : change min
 *	right button : change max
 */
	void erase_thresh_bounds();
	void draw_thresh_bounds();

	int x;
	static int which_button;

	switch(event_id(event)) {
	case MS_LEFT :
		if(event_is_down(event)) {
			which_button = MS_LEFT;
			x = event_x(event);
			if(x <= threshold.max
			&& x >= 0) {
				erase_thresh_bounds();
				threshold.min = x;
				draw_thresh_bounds();
			}
		}
		break;
	case MS_RIGHT :
		if(event_is_down(event)) {
			which_button = MS_RIGHT;
			x = event_x(event);
			if(x <= 255
			&& x >= threshold.min) {
				erase_thresh_bounds();
				threshold.max = x;
				draw_thresh_bounds();
			}
		}
		break;
	case MS_MIDDLE :
		break;
	case LOC_DRAG :
		if(event_is_down(event)) switch(which_button) {
		case MS_LEFT :
			x = event_x(event);
			if(x <= threshold.max
			&& x >= 0) {
				erase_thresh_bounds();
				threshold.min = x;
				draw_thresh_bounds();
			}
			break;
		case MS_RIGHT :
			x = event_x(event);
			if(x <= 255
			&& x >= threshold.min) {
				erase_thresh_bounds();
				threshold.max = x;
				draw_thresh_bounds();
			}
			break;
		default :
			break;
		}
		break;
	default :
		break;
	}
}
