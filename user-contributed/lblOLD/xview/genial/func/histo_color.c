/*
 * histo_color.c -- routines for color image based on histograms
 *
 */

#include "display.h"
#include "common.h"
#include "ui.h"
#include "log.h"
#include "reg.h"
#include "histo_ui.h"
#include "scale.h"
#include <math.h>

extern GC hgc;
struct hcontext *histo_by_lid();

/*********************************************************************/
/*
 * Repaint callback function for `palcanv'.
 */
void
palcanv_repaint_proc(canvas, paint_window, display, w, rects)
    Canvas    canvas;
    Xv_window paint_window;
    Display  *display;
    Window    w;
    Xv_xrectlist *rects;
{
    int       i;

#ifdef DEBUG
    fputs("histo: palcanv_repaint_proc\n", stderr);
#endif

    for (i = RED; i <= WHITE; i++) {
	XSetForeground(display, gc, pallet[i].pixel);
	XFillRectangle(display, w, gc, 5, ((i) * 40 + 5), 30, 30);
	XSetForeground(display, gc, pallet[BLACK].pixel);
	XDrawRectangle(display, w, gc, 5, ((i) * 40 + 5), 30, 30);
    }
}

/*********************************************************************/
draw_histo_color_lines(hist)
    struct hcontext *hist;
{
    int       i, sy, ey;
    XGCValues gcval;
    XPoint    src;
    double    xdelt, xtmp;

    xtmp = hist->xglab[hist->xglen - 1].val - hist->xglab[0].val;
    if (xtmp == 0.)
	return;
    xdelt = (float) GRWIDTH / xtmp;

    /* loop through all subintervals, drawing dividers and coloring ranges */
    XSetLineAttributes(display, hgc, 2, LineSolid, CapButt, JoinBevel);
    sy = SY + 1;		/* dont overwrite axes */
    ey = SY + GRHGT - 1;

    for (i = 0; i < NUM_SUB; i++) {
	/* make the divider and region the color of the associated range */
	if (hist->interval[i].isactive) {
	    gcval.foreground = pallet[i].pixel;
	    XChangeGC(display, hgc, GCForeground, &gcval);
	    src.x = (short) btox(hist->interval[i].lower, hist);
	    /* center line in rectangle */
	    src.x = src.x + (int) ((.5 * xdelt) + .5);
	    if (src.x > SX && src.x < SX + GRWIDTH) {	/* draw lower */
#ifdef DEBUG
		printf("Drawing new lower line: (%d,%d) to (%d,%d)\n",
		       src.x, sy, src.x, ey);
#endif
		XDrawLine(display, hist->hxid, hgc,
			  src.x, sy, src.x, ey);
	    }
	    src.x = (short) btox(hist->interval[i].upper, hist);
	    src.x = src.x + (int) ((.5 * xdelt) + .5);
	    if (src.x > SX && src.x < SX + GRWIDTH) {	/* draw upper */
#ifdef DEBUG
		printf("Drawing new upper line: (%d,%d) to (%d,%d)\n",
		       src.x, sy, src.x, ey);
#endif
		XDrawLine(display, hist->hxid, hgc,
			  src.x, sy, src.x, ey);
	    }
	}
    }
    XSetLineAttributes(display, hgc, 1, LineSolid, CapButt, JoinBevel);
}

/*********************************************************************/
/* routine to light up a subregion */

lite(hist, r)
    struct hcontext *hist;
    int       r;
{
    int       i, j;
    XPoint    pt;

    hist->interval[r].islit = 1;

    if (hist->interval[r].lower == 0 &&
	hist->interval[r].upper == NUM_BUCKETS - 1)
	return;

    XSetForeground(display, hgc, pallet[r].pixel);
    for (i = hist->interval[r].lower;
	 i <= hist->interval[r].upper; i++) {
	for (j = 0; j < hist->lbtab[i].npts; j++) {
	    itod_pt(hist->lbtab[i].parr + j, &pt);
	    XDrawPoint(display, img_win->d_xid, hgc, pt.x, pt.y);
	}
    }
}

/********************************************************/

/* routine to UN-light a histogram */
unlite(hist, r)
    struct hcontext *hist;
    int       r;
{
    int       i, j, x, y, gval;

    hist->interval[r].islit = 0;

    for (i = hist->interval[r].lower;
	 i <= hist->interval[r].upper; i++) {
	if (hist->lbtab[i].npts > 0) {
	    for (j = 0; j < hist->lbtab[i].npts; j++) {
		x = hist->lbtab[i].parr[j].x;
		y = hist->lbtab[i].parr[j].y;
		gval = XGetPixel(orig_ximg, x, y);
		XSetForeground(display, hgc, gval);
		x = itod(x);
		y = itod(y);
		XDrawPoint(display, img_win->d_xid, hgc, x, y);
	    }
	}
    }
}

/************************************************************/

#ifdef NEED
 /*
  * removed this button from the interface: I dont think it is really
  * necessary  -blt
  */
void
histo_color_paintall_proc(item, event)
    Panel_item item;
    Event    *event;
{
    int       i, hno;
    struct hcontext *hist;

#ifdef DEBUG
    fputs("histo: histo_color_paintall_proc\n", stderr);
#endif

    hno = (int) xv_get(histo_ctrl->histno, PANEL_VALUE, NULL);
    if (check_frame(hno) == 0)
	return;
    if ((hist = histo_by_lid(hno)) == NULL)
	return;

    for (i = 0; i < NUM_SUB; i++)
	lite(hist, i);

    /* turn on all paint check boxes */
    xv_set(histo_ctrl->pnt1, PANEL_VALUE, 1, NULL);
    xv_set(histo_ctrl->pnt2, PANEL_VALUE, 1, NULL);
    xv_set(histo_ctrl->pnt3, PANEL_VALUE, 1, NULL);
    xv_set(histo_ctrl->pnt4, PANEL_VALUE, 1, NULL);
    xv_set(histo_ctrl->pnt5, PANEL_VALUE, 1, NULL);
    xv_set(histo_ctrl->pnt6, PANEL_VALUE, 1, NULL);
    xv_set(histo_ctrl->pnt7, PANEL_VALUE, 1, NULL);
    xv_set(histo_ctrl->pnt8, PANEL_VALUE, 1, NULL);
}

#endif

/************************************************************/
void
histo_color_paint_proc(item, value, event)
    Panel_item item;
    int       value;
    Event    *event;
{
    int       box_num, hno;
    struct hcontext *hist;

#ifdef DEBUG
    fprintf(stderr, "histo: histo_color_paint_proc: value: 0x%x\n", value);
#endif

    hno = (int) xv_get(histo_ctrl->histno, PANEL_VALUE, NULL);
    if (check_frame(hno) == 0)
	return;
    if ((hist = histo_by_lid(hno)) == NULL)
	return;

    box_num = get_selected_box_number(hist);
    if (box_num < 0)
	return;

    if (hist->interval[box_num].islit == 0) {
#ifdef DEBUG
	printf("Painting box: %d \n", box_num);
#endif
	lite(hist, box_num);
    } else {
#ifdef DEBUG
	printf("UN-Painting box: %d \n", box_num);
#endif
	unlite(hist, box_num);
    }
    draw_log();
}

/**************************************************************/
int
get_selected_box_number(hist)
    struct hcontext *hist;
{
    if ((int) xv_get(histo_ctrl->pnt1, PANEL_VALUE, NULL) > 0) {
	if ((int) xv_get(histo_ctrl->min1, PANEL_VALUE, NULL) == 0 &&
	    (int) xv_get(histo_ctrl->max1, PANEL_VALUE, NULL) == 0) {
	    xv_set(histo_ctrl->pnt1, PANEL_VALUE, 0, NULL);
	    return -1;
	}
	if (hist->interval[0].islit == 0)
	    return (0);
    } else {
	if (hist->interval[0].islit == 1)
	    return (0);
    }
    if ((int) xv_get(histo_ctrl->pnt2, PANEL_VALUE, NULL) > 0) {
	if ((int) xv_get(histo_ctrl->min2, PANEL_VALUE, NULL) == 0 &&
	    (int) xv_get(histo_ctrl->max2, PANEL_VALUE, NULL) == 0) {
	    xv_set(histo_ctrl->pnt2, PANEL_VALUE, 0, NULL);
	    return -1;
	}
	if (hist->interval[1].islit == 0)
	    return (1);
    } else {
	if (hist->interval[1].islit == 1)
	    return (1);
    }
    if ((int) xv_get(histo_ctrl->pnt3, PANEL_VALUE, NULL) > 0) {
	if ((int) xv_get(histo_ctrl->min3, PANEL_VALUE) == 0 &&
	    (int) xv_get(histo_ctrl->max3, PANEL_VALUE) == 0) {
	    xv_set(histo_ctrl->pnt3, PANEL_VALUE, 0, NULL);
	    return -1;
	}
	if (hist->interval[2].islit == 0)
	    return (2);
    } else {
	if (hist->interval[2].islit == 1)
	    return (2);
    }
    if ((int) xv_get(histo_ctrl->pnt4, PANEL_VALUE, NULL) > 0) {
	if ((int) xv_get(histo_ctrl->min4, PANEL_VALUE) == 0 &&
	    (int) xv_get(histo_ctrl->max4, PANEL_VALUE) == 0) {
	    xv_set(histo_ctrl->pnt4, PANEL_VALUE, 0, NULL);
	    return -1;
	}
	if (hist->interval[3].islit == 0)
	    return (3);
    } else {
	if (hist->interval[3].islit == 1)
	    return (3);
    }
    if ((int) xv_get(histo_ctrl->pnt5, PANEL_VALUE, NULL) > 0) {
	if ((int) xv_get(histo_ctrl->min5, PANEL_VALUE) == 0 &&
	    (int) xv_get(histo_ctrl->max5, PANEL_VALUE) == 0) {
	    xv_set(histo_ctrl->pnt5, PANEL_VALUE, 0, NULL);
	    return -1;
	}
	if (hist->interval[4].islit == 0)
	    return (4);
    } else {
	if (hist->interval[4].islit == 1)
	    return (4);
    }
    if ((int) xv_get(histo_ctrl->pnt6, PANEL_VALUE, NULL) > 0) {
	if ((int) xv_get(histo_ctrl->min6, PANEL_VALUE) == 0 &&
	    (int) xv_get(histo_ctrl->max6, PANEL_VALUE) == 0) {
	    xv_set(histo_ctrl->pnt6, PANEL_VALUE, 0, NULL);
	    return -1;
	}
	if (hist->interval[5].islit == 0)
	    return (5);
    } else {
	if (hist->interval[5].islit == 1)
	    return (5);
    }
    if ((int) xv_get(histo_ctrl->pnt7, PANEL_VALUE, NULL) > 0) {
	if ((int) xv_get(histo_ctrl->min7, PANEL_VALUE) == 0 &&
	    (int) xv_get(histo_ctrl->max7, PANEL_VALUE) == 0) {
	    xv_set(histo_ctrl->pnt7, PANEL_VALUE, 0, NULL);
	    return -1;
	}
	if (hist->interval[6].islit == 0)
	    return (6);
    } else {
	if (hist->interval[6].islit == 1)
	    return (6);
    }
    if ((int) xv_get(histo_ctrl->pnt8, PANEL_VALUE, NULL) > 0) {
	if ((int) xv_get(histo_ctrl->min8, PANEL_VALUE) == 0 &&
	    (int) xv_get(histo_ctrl->max8, PANEL_VALUE) == 0) {
	    xv_set(histo_ctrl->pnt8, PANEL_VALUE, 0, NULL);
	    return -1;
	}
	if (hist->interval[7].islit == 0)
	    return (7);
    } else {
	if (hist->interval[7].islit == 1)
	    return (7);
    }
    return -1;
}

/*************************************************************/

paint_check_boxs_off(n)
    int       n;
{
    /* turn off everything but n */

    if (n != 0)
	xv_set(histo_ctrl->pnt1, PANEL_VALUE, 0, NULL);
    if (n != 1)
	xv_set(histo_ctrl->pnt2, PANEL_VALUE, 0, NULL);
    if (n != 2)
	xv_set(histo_ctrl->pnt3, PANEL_VALUE, 0, NULL);
    if (n != 3)
	xv_set(histo_ctrl->pnt4, PANEL_VALUE, 0, NULL);
    if (n != 4)
	xv_set(histo_ctrl->pnt5, PANEL_VALUE, 0, NULL);
    if (n != 5)
	xv_set(histo_ctrl->pnt6, PANEL_VALUE, 0, NULL);
    if (n != 6)
	xv_set(histo_ctrl->pnt7, PANEL_VALUE, 0, NULL);
    if (n != 7)
	xv_set(histo_ctrl->pnt8, PANEL_VALUE, 0, NULL);
}

/****************************************************/

void
histo_color_select_proc(item, value, event)
    Panel_item item;
    int       value;
    Event    *event;
{
    int       box_num, hno;
    struct hcontext *hist;

#ifdef DEBUG
    fprintf(stderr, "histo: histo_color_select_proc: value: 0x%x\n", value);
#endif

    box_num = turn_off_other_select_boxes();
    if (box_num < 0)
	return;

    hno = (int) xv_get(histo_ctrl->histno, PANEL_VALUE, NULL);
    hist = histo_by_lid(hno);
    if (hist == NULL)
	return;

    hist->subnum = box_num;

#ifdef DEBUG
    printf("selected box: %d \n", hist->subnum);
#endif
}

/*************************************************************/

int
turn_off_other_select_boxes()
{
    static int prev_box = -1;
    int       box, curr_box = -1;

    if ((int) xv_get(histo_ctrl->sel1, PANEL_VALUE, NULL) > 0) {
	box = 0;
	if (box != prev_box)
	    curr_box = box;
    }
    if ((int) xv_get(histo_ctrl->sel2, PANEL_VALUE, NULL) > 0) {
	box = 1;
	if (box != prev_box)
	    curr_box = box;
    }
    if ((int) xv_get(histo_ctrl->sel3, PANEL_VALUE, NULL) > 0) {
	box = 2;
	if (box != prev_box)
	    curr_box = box;
    }
    if ((int) xv_get(histo_ctrl->sel4, PANEL_VALUE, NULL) > 0) {
	box = 3;
	if (box != prev_box)
	    curr_box = box;
    }
    if ((int) xv_get(histo_ctrl->sel5, PANEL_VALUE, NULL) > 0) {
	box = 4;
	if (box != prev_box)
	    curr_box = box;
    }
    if ((int) xv_get(histo_ctrl->sel6, PANEL_VALUE, NULL) > 0) {
	box = 5;
	if (box != prev_box)
	    curr_box = box;
    }
    if ((int) xv_get(histo_ctrl->sel7, PANEL_VALUE, NULL) > 0) {
	box = 6;
	if (box != prev_box)
	    curr_box = box;
    }
    if ((int) xv_get(histo_ctrl->sel8, PANEL_VALUE, NULL) > 0) {
	box = 7;
	if (box != prev_box)
	    curr_box = box;
    }
    if (curr_box >= 0) {
	select_check_boxs_off(curr_box);
	prev_box = curr_box;
    }
    return (prev_box);
}

/****************************************************/
select_check_boxs_off(n)
    int       n;
{
    /* turn off everything but n */

    if (n != 0)
	xv_set(histo_ctrl->sel1, PANEL_VALUE, 0, NULL);
    if (n != 1)
	xv_set(histo_ctrl->sel2, PANEL_VALUE, 0, NULL);
    if (n != 2)
	xv_set(histo_ctrl->sel3, PANEL_VALUE, 0, NULL);
    if (n != 3)
	xv_set(histo_ctrl->sel4, PANEL_VALUE, 0, NULL);
    if (n != 4)
	xv_set(histo_ctrl->sel5, PANEL_VALUE, 0, NULL);
    if (n != 5)
	xv_set(histo_ctrl->sel6, PANEL_VALUE, 0, NULL);
    if (n != 6)
	xv_set(histo_ctrl->sel7, PANEL_VALUE, 0, NULL);
    if (n != 7)
	xv_set(histo_ctrl->sel8, PANEL_VALUE, 0, NULL);
}

/*********************************************************/
Panel_setting
interval1_proc(item, event)
    Panel_item item;
    Event    *event;
{
    int       min_value = (int) xv_get(histo_ctrl->min1, PANEL_VALUE, NULL);
    int       max_value = (int) xv_get(histo_ctrl->max1, PANEL_VALUE, NULL);
    struct hcontext *hist;
    int       hno;

#ifdef DEBUG
    fprintf(stderr, "histo: interval1_proc: min: %d   max: %d\n",
	    min_value, max_value);
#endif

    if (max_value < min_value) {
	xv_set(histo_ctrl->max1, PANEL_VALUE, min_value, NULL);
	return panel_text_notify(item, event);
    }
    if (min_value > max_value) {
	xv_set(histo_ctrl->min1, PANEL_VALUE, max_value, NULL);
	return panel_text_notify(item, event);
    }
    hno = (int) xv_get(histo_ctrl->histno, PANEL_VALUE, NULL);
    if ((hist = histo_by_lid(hno)) == NULL)
	return panel_text_notify(item, event);

    if (min_value < hist->minv) {
	min_value = hist->minv;
	xv_set(histo_ctrl->min1, PANEL_VALUE, min_value, NULL);
    }
    if (max_value > hist->maxv) {
	max_value = hist->maxv;
	xv_set(histo_ctrl->max1, PANEL_VALUE, max_value, NULL);
    }
    move_color_line(hist, 0, min_value, max_value);

    return panel_text_notify(item, event);
}

/*********************************************************/
Panel_setting
interval2_proc(item, event)
    Panel_item item;
    Event    *event;
{
    int       min_value = (int) xv_get(histo_ctrl->min2, PANEL_VALUE);
    int       max_value = (int) xv_get(histo_ctrl->max2, PANEL_VALUE);

    int       hno;
    struct hcontext *hist;

#ifdef DEBUG
    fprintf(stderr, "histo: interval1_proc: min: %d   max: %d\n",
	    min_value, max_value);
#endif

    if (max_value < min_value) {
	xv_set(histo_ctrl->max2, PANEL_VALUE, min_value, NULL);
	return panel_text_notify(item, event);
    }
    if (min_value > max_value) {
	xv_set(histo_ctrl->min2, PANEL_VALUE, max_value, NULL);
	return panel_text_notify(item, event);
    }
    hno = (int) xv_get(histo_ctrl->histno, PANEL_VALUE);
    if ((hist = histo_by_lid(hno)) == NULL)
	return panel_text_notify(item, event);

    if (min_value < hist->minv) {
	min_value = hist->minv;
	xv_set(histo_ctrl->min2, PANEL_VALUE, min_value, NULL);
    }
    if (max_value > hist->maxv) {
	max_value = hist->maxv;
	xv_set(histo_ctrl->max2, PANEL_VALUE, max_value, NULL);
    }
    move_color_line(hist, 1, min_value, max_value);

    return panel_text_notify(item, event);
}

/*********************************************************/
Panel_setting
interval3_proc(item, event)
    Panel_item item;
    Event    *event;
{
    int       min_value = (int) xv_get(histo_ctrl->min3, PANEL_VALUE);
    int       max_value = (int) xv_get(histo_ctrl->max3, PANEL_VALUE);

    int       hno;
    struct hcontext *hist;

#ifdef DEBUG
    fprintf(stderr, "histo: interval1_proc: min: %d   max: %d\n",
	    min_value, max_value);
#endif

    if (max_value < min_value) {
	xv_set(histo_ctrl->max3, PANEL_VALUE, min_value, NULL);
	return panel_text_notify(item, event);
    }
    if (min_value > max_value) {
	xv_set(histo_ctrl->min3, PANEL_VALUE, max_value, NULL);
	return panel_text_notify(item, event);
    }
    hno = (int) xv_get(histo_ctrl->histno, PANEL_VALUE);
    if ((hist = histo_by_lid(hno)) == NULL)
	return panel_text_notify(item, event);

    if (min_value < hist->minv) {
	min_value = hist->minv;
	xv_set(histo_ctrl->min3, PANEL_VALUE, min_value, NULL);
    }
    if (max_value > hist->maxv) {
	max_value = hist->maxv;
	xv_set(histo_ctrl->max3, PANEL_VALUE, max_value, NULL);
    }
    move_color_line(hist, 2, min_value, max_value);

    return panel_text_notify(item, event);
}

/*********************************************************/
Panel_setting
interval4_proc(item, event)
    Panel_item item;
    Event    *event;
{
    int       min_value = (int) xv_get(histo_ctrl->min4, PANEL_VALUE);
    int       max_value = (int) xv_get(histo_ctrl->max4, PANEL_VALUE);

    int       hno;
    struct hcontext *hist;

#ifdef DEBUG
    fprintf(stderr, "histo: interval1_proc: min: %d   max: %d\n",
	    min_value, max_value);
#endif

    if (max_value < min_value) {
	xv_set(histo_ctrl->max4, PANEL_VALUE, min_value, NULL);
	return panel_text_notify(item, event);
    }
    if (min_value > max_value) {
	xv_set(histo_ctrl->min4, PANEL_VALUE, max_value, NULL);
	return panel_text_notify(item, event);
    }
    hno = (int) xv_get(histo_ctrl->histno, PANEL_VALUE);
    if ((hist = histo_by_lid(hno)) == NULL)
	return panel_text_notify(item, event);

    if (min_value < hist->minv) {
	min_value = hist->minv;
	xv_set(histo_ctrl->min4, PANEL_VALUE, min_value, NULL);
    }
    if (max_value > hist->maxv) {
	max_value = hist->maxv;
	xv_set(histo_ctrl->max4, PANEL_VALUE, max_value, NULL);
    }
    move_color_line(hist, 3, min_value, max_value);

    return panel_text_notify(item, event);
}

/*********************************************************/
Panel_setting
interval5_proc(item, event)
    Panel_item item;
    Event    *event;
{
    int       min_value = (int) xv_get(histo_ctrl->min5, PANEL_VALUE);
    int       max_value = (int) xv_get(histo_ctrl->max5, PANEL_VALUE);

    int       hno;
    struct hcontext *hist;

#ifdef DEBUG
    fprintf(stderr, "histo: interval1_proc: min: %d   max: %d\n",
	    min_value, max_value);
#endif

    if (max_value < min_value) {
	xv_set(histo_ctrl->max5, PANEL_VALUE, min_value, NULL);
	return panel_text_notify(item, event);
    }
    if (min_value > max_value) {
	xv_set(histo_ctrl->min5, PANEL_VALUE, max_value, NULL);
	return panel_text_notify(item, event);
    }
    hno = (int) xv_get(histo_ctrl->histno, PANEL_VALUE);
    if ((hist = histo_by_lid(hno)) == NULL)
	return panel_text_notify(item, event);

    if (min_value < hist->minv) {
	min_value = hist->minv;
	xv_set(histo_ctrl->min5, PANEL_VALUE, min_value, NULL);
    }
    if (max_value > hist->maxv) {
	max_value = hist->maxv;
	xv_set(histo_ctrl->max5, PANEL_VALUE, max_value, NULL);
    }
    move_color_line(hist, 4, min_value, max_value);
    return panel_text_notify(item, event);
}

/*********************************************************/
Panel_setting
interval6_proc(item, event)
    Panel_item item;
    Event    *event;
{
    int       min_value = (int) xv_get(histo_ctrl->min6, PANEL_VALUE);
    int       max_value = (int) xv_get(histo_ctrl->max6, PANEL_VALUE);

    int       hno;
    struct hcontext *hist;

#ifdef DEBUG
    fprintf(stderr, "histo: interval1_proc: min: %d   max: %d\n",
	    min_value, max_value);
#endif

    if (max_value < min_value) {
	xv_set(histo_ctrl->max6, PANEL_VALUE, min_value, NULL);
	return panel_text_notify(item, event);
    }
    if (min_value > max_value) {
	xv_set(histo_ctrl->min6, PANEL_VALUE, max_value, NULL);
	return panel_text_notify(item, event);
    }
    hno = (int) xv_get(histo_ctrl->histno, PANEL_VALUE);
    if ((hist = histo_by_lid(hno)) == NULL)
	return panel_text_notify(item, event);

    if (min_value < hist->minv) {
	min_value = hist->minv;
	xv_set(histo_ctrl->min6, PANEL_VALUE, min_value, NULL);
    }
    if (max_value > hist->maxv) {
	max_value = hist->maxv;
	xv_set(histo_ctrl->max6, PANEL_VALUE, max_value, NULL);
    }
    move_color_line(hist, 5, min_value, max_value);
    return panel_text_notify(item, event);
}

/*********************************************************/
Panel_setting
interval7_proc(item, event)
    Panel_item item;
    Event    *event;
{
    int       min_value = (int) xv_get(histo_ctrl->min7, PANEL_VALUE);
    int       max_value = (int) xv_get(histo_ctrl->max7, PANEL_VALUE);

    int       hno;
    struct hcontext *hist;

#ifdef DEBUG
    fprintf(stderr, "histo: interval1_proc: min: %d   max: %d\n",
	    min_value, max_value);
#endif

    if (max_value < min_value) {
	xv_set(histo_ctrl->max7, PANEL_VALUE, min_value, NULL);
	return panel_text_notify(item, event);
    }
    if (min_value > max_value) {
	xv_set(histo_ctrl->min7, PANEL_VALUE, max_value, NULL);
	return panel_text_notify(item, event);
    }
    hno = (int) xv_get(histo_ctrl->histno, PANEL_VALUE);
    if ((hist = histo_by_lid(hno)) == NULL)
	return panel_text_notify(item, event);

    if (min_value < hist->minv) {
	min_value = hist->minv;
	xv_set(histo_ctrl->min7, PANEL_VALUE, min_value, NULL);
    }
    if (max_value > hist->maxv) {
	max_value = hist->maxv;
	xv_set(histo_ctrl->max7, PANEL_VALUE, max_value, NULL);
    }
    move_color_line(hist, 6, min_value, max_value);
    return panel_text_notify(item, event);
}

/*********************************************************/
Panel_setting
interval8_proc(item, event)
    Panel_item item;
    Event    *event;
{
    int       min_value = (int) xv_get(histo_ctrl->min8, PANEL_VALUE);
    int       max_value = (int) xv_get(histo_ctrl->max8, PANEL_VALUE);

    int       hno;
    struct hcontext *hist;

#ifdef DEBUG
    fprintf(stderr, "histo: interval1_proc: min: %d   max: %d\n",
	    min_value, max_value);
#endif


    if (max_value < min_value) {
	xv_set(histo_ctrl->max8, PANEL_VALUE, min_value, NULL);
	return panel_text_notify(item, event);
    }
    if (min_value > max_value) {
	xv_set(histo_ctrl->min8, PANEL_VALUE, max_value, NULL);
	return panel_text_notify(item, event);
    }
    hno = (int) xv_get(histo_ctrl->histno, PANEL_VALUE);
    if ((hist = histo_by_lid(hno)) == NULL)
	return panel_text_notify(item, event);

    if (min_value < hist->minv) {
	min_value = hist->minv;
	xv_set(histo_ctrl->min8, PANEL_VALUE, min_value, NULL);
    }
    if (max_value > hist->maxv) {
	max_value = hist->maxv;
	xv_set(histo_ctrl->max8, PANEL_VALUE, max_value, NULL);
    }
    move_color_line(hist, 7, min_value, max_value);
    return panel_text_notify(item, event);
}

/*******************************************************/
move_color_line(hist, n, min, max)
    struct hcontext *hist;
    int       n, min, max;
{
    int       oldbuck, newbuck;

#ifdef DEBUG
    printf("\n move_color_line: n=%d, \n", n);
#endif

    hist->interval[n].isactive = 1;

    oldbuck = hist->interval[n].lower;
    undraw_histo_line(hist, oldbuck);
    newbuck = gtob(min, hist);
    hist->interval[n].lower = newbuck;

    oldbuck = hist->interval[n].upper;
    undraw_histo_line(hist, oldbuck);
    newbuck = gtob(max, hist);
    hist->interval[n].upper = newbuck;

    /* redraw all color lines */
    draw_histo_color_lines(hist);
}

/*******************************************************/
void
clear_histo_color_proc(item, event)
    Panel_item item;
    Event    *event;
{
    struct hcontext *hist;
    int       i, hno;

#ifdef DEBUG
    fputs("histo: clear_histo_color_proc\n", stderr);
#endif

    hno = (int) xv_get(histo_ctrl->histno, PANEL_VALUE);
    if ((hist = histo_by_lid(hno)) == NULL) {
	return;
    }
    xv_set(histo_ctrl->min1, PANEL_VALUE, 0, NULL);
    xv_set(histo_ctrl->max1, PANEL_VALUE, 0, NULL);
    xv_set(histo_ctrl->min2, PANEL_VALUE, 0, NULL);
    xv_set(histo_ctrl->max2, PANEL_VALUE, 0, NULL);
    xv_set(histo_ctrl->min3, PANEL_VALUE, 0, NULL);
    xv_set(histo_ctrl->max3, PANEL_VALUE, 0, NULL);
    xv_set(histo_ctrl->min4, PANEL_VALUE, 0, NULL);
    xv_set(histo_ctrl->max4, PANEL_VALUE, 0, NULL);
    xv_set(histo_ctrl->min5, PANEL_VALUE, 0, NULL);
    xv_set(histo_ctrl->max5, PANEL_VALUE, 0, NULL);
    xv_set(histo_ctrl->min6, PANEL_VALUE, 0, NULL);
    xv_set(histo_ctrl->max6, PANEL_VALUE, 0, NULL);
    xv_set(histo_ctrl->min7, PANEL_VALUE, 0, NULL);
    xv_set(histo_ctrl->max7, PANEL_VALUE, 0, NULL);
    xv_set(histo_ctrl->min8, PANEL_VALUE, 0, NULL);
    xv_set(histo_ctrl->max8, PANEL_VALUE, 0, NULL);

    /* turn off all select check boxes */
    xv_set(histo_ctrl->sel1, PANEL_VALUE, 1, NULL);	/* keep this one on */
    xv_set(histo_ctrl->sel2, PANEL_VALUE, 0, NULL);
    xv_set(histo_ctrl->sel3, PANEL_VALUE, 0, NULL);
    xv_set(histo_ctrl->sel4, PANEL_VALUE, 0, NULL);
    xv_set(histo_ctrl->sel5, PANEL_VALUE, 0, NULL);
    xv_set(histo_ctrl->sel6, PANEL_VALUE, 0, NULL);
    xv_set(histo_ctrl->sel7, PANEL_VALUE, 0, NULL);
    xv_set(histo_ctrl->sel8, PANEL_VALUE, 0, NULL);

    /* turn off all paint check boxes */
    xv_set(histo_ctrl->pnt1, PANEL_VALUE, 0, NULL);
    xv_set(histo_ctrl->pnt2, PANEL_VALUE, 0, NULL);
    xv_set(histo_ctrl->pnt3, PANEL_VALUE, 0, NULL);
    xv_set(histo_ctrl->pnt4, PANEL_VALUE, 0, NULL);
    xv_set(histo_ctrl->pnt5, PANEL_VALUE, 0, NULL);
    xv_set(histo_ctrl->pnt6, PANEL_VALUE, 0, NULL);
    xv_set(histo_ctrl->pnt7, PANEL_VALUE, 0, NULL);
    xv_set(histo_ctrl->pnt8, PANEL_VALUE, 0, NULL);

    for (i = 0; i < NUM_SUB; i++) {
	hist->interval[i].lower = 0;
	hist->interval[i].upper = NUM_BUCKETS - 1;
	hist->interval[i].islit = hist->interval[i].isactive = 0;
    }

    hist->subnum = 0;

    histo_repaint_proc(hist->histo_display->histocanv,
		       hist->paintwin, display, hist->hxid, NULL);

    imgwin_repaint_proc(img_win->d_canv, img_win->d_paintwin, display,
			img_win->d_xid, NULL);
}
