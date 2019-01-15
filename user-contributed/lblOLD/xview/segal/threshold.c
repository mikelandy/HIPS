/*
 *	threshold.c
 *
 *	Bryan Skene, LBL July 1991
 *	part of 'segal'
 */

#include "common.h"

#include "display_control.h"
#include "pixedit.h"
#include "threshold_ui.h"

/* Allocation :) */
threshold_win_objects *threshold_win;

int histo[256];

/*****************************************************/
void
threshold_win_init(owner)
Xv_opaque owner;
{

	threshold_win = threshold_win_objects_initialize(NULL, owner);

	/* window defaults */
	threshold.upper = 255;
	threshold.lower = 0;

	(void) xv_set(threshold_win->threshold_value_upper,
		PANEL_VALUE, threshold.upper,
		NULL);

	(void) xv_set(threshold_win->threshold_value_lower,
		PANEL_VALUE, threshold.lower,
		NULL);
}

/*****************************************************/
void
map_threshold(item, event)
Panel_item      item;
Event           *event;
{
	void refresh_histogram();

	fputs("segal: map_threshold\n", stderr);
        /* Map / Unmap toggle */
	if (xv_get(threshold_win->win, XV_SHOW, NULL) == FALSE) {
		xv_set(threshold_win->win,
			XV_SHOW, TRUE,
			NULL);
		refresh_histogram();
	}
	else xv_set(threshold_win->win,
			XV_SHOW, FALSE,
			NULL);
}

/***********************************************/
void
refresh_histogram()
{
	void generate_histogram();
	void draw_histogram();

	XClearArea(display, threshold_xid, 0, 0, 0, 0, FALSE);
	generate_histogram();
	draw_histogram();
}

/***********************************************/
void
generate_histogram()
{
	int i, min, max, max_i, old_max, delta;
	int x, y;

	for(i = 0; i < 256; i++) {
		histo[i] = 0;
	}

	if(region.whole) {
		for(y = 0; y < segal.rows; y++)
			for(x = 0; x < segal.cols; x++)
				histo[himage.data[y][x]]++;
	}
	else {
		for(y = region.cry1; y < region.cry2; y++)
			for(x = region.crx1; x < region.crx2; x++)
				histo[himage.data[y][x]]++;
	}

#define LEAVE_OUT_ZERO

#ifdef LEAVE_OUT_ZERO
#define BEG_VALUE 1
#else
#define BEG_VALUE 0
#endif

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

	delta = max - min;

	for(i = 0; i < 256; i++)
		histo[i] = (int) ((float) 256 * (float) histo[i] /
			(float) delta);
}

/***********************************************/
void
draw_histogram()
{
	int i;

	XSetForeground(display, gc, blue_standout);
	for(i=0; i < 256; i++) {
		XDrawLine(display, threshold_xid, gc, i, 255, i, 255 - histo[i]);
	}
}

/***********************************************/
void
set_thresh_lower(item, value, event)
Panel_item item;
int value;
Event *event;
{
	threshold.lower = value;

	if(threshold.upper <= threshold.lower) {
		threshold.upper = value + 1;
		(void) xv_set(threshold_win->threshold_value_upper,
			PANEL_VALUE, threshold.upper,
			NULL);
	}
}

/***********************************************/
void
set_thresh_upper(item, value, event)
Panel_item item;
int value;
Event *event;
{
	threshold.upper = value;

	if(threshold.lower >= threshold.upper) {
		threshold.lower = value - 1;
		(void) xv_set(threshold_win->threshold_value_lower,
			PANEL_VALUE, threshold.lower,
			NULL);
	}
}

/***********************************************/
void
threshold_proc(item, event)
Panel_item item;
Event *event;
{
	void draw_filenames();

	register int i, j, size;
	register u_char *iptr, *mptr;

	if (himage.fp == NULL)
		return;

	set_watch_cursor();

	if (segal.display_type == 0) {
		segal.display_type = 1;
		xv_set(display_control_win->display_type, PANEL_VALUE, 1, NULL);
	}

	size = segal.rows * segal.cols;

	if(region.whole) {
		iptr = himage.data[0];
		mptr = work_buf[0];

		for (i = 0; i < size; i++) {
			if (iptr[i] >= threshold.lower
			&&  iptr[i] <= threshold.upper)
				mptr[i] = PVAL;	/* white */
			else
				mptr[i] = 0;	/* black */
		}
	}
	else {
		for(j = region.cry1; j <= region.cry2; j++)
		for(i = region.crx1; i <= region.crx2; i++) {
			if (himage.data[j][i] >= threshold.lower
			&&  himage.data[j][i] <= threshold.upper)
				work_buf[j][i] = PVAL;	/* white */
			else
				work_buf[j][i] = 0;	/* black */
		}
	}

	map_image_to_lut(work_buf[0], mask_image->data, size);

	blend(himage.data[0], work_buf[0], (u_char *) blend_image->data, size);

	image_repaint_proc();
	if ((int) xv_get(edit_win->win, XV_SHOW, NULL) == TRUE) {
		zoom();
		edit_repaint_proc();
	}
	segal.changed = 1;		/* mask modified flag */
	draw_filenames();

	unset_watch_cursor();
}
