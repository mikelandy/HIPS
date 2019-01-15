/*
 * histo.c
 */

#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/textsw.h>
#include <xview/xv_xrect.h>
#include "histo_ui.h"
#include "trace_ui.h"
#include "histo.h"
#include "ui.h"

histo_histo_ctrl_objects *histo_ctrl;

struct hcontext *histo_by_lid(), *histo_by_panelwin();

/*
 * Notify callback function for `histno'.
 */
Panel_setting
histno_proc(item, event)
    Panel_item item;
    Event    *event;
{
    int       value = (int) xv_get(item, PANEL_VALUE);
    int       bucket;
    struct hcontext *hist;

#ifdef DEBUG
    fprintf(stderr, "histo: histno_proc: value: %d\n", value);
#endif

    if (value < 1) {
	xv_set(item, PANEL_VALUE, 1, NULL);
	return;
    }
    if ((hist = histo_by_lid(value)) == NULL) {
	xv_set(item, PANEL_VALUE, value - 1, NULL);
	return;
    }
    hist->subnum = turn_off_other_select_boxes();

    bucket = hist->interval[0].lower;
    if (bucket != 0)
	xv_set(histo_ctrl->min1, PANEL_VALUE, btog(bucket, hist), NULL);
    else
	xv_set(histo_ctrl->min1, PANEL_VALUE, 0, NULL);
    bucket = hist->interval[0].upper;
    if (bucket != NUM_BUCKETS - 1)
	xv_set(histo_ctrl->max1, PANEL_VALUE, btog(bucket, hist), NULL);
    else
	xv_set(histo_ctrl->max1, PANEL_VALUE, 0, NULL);

    bucket = hist->interval[1].lower;
    if (bucket != 0)
	xv_set(histo_ctrl->min2, PANEL_VALUE, btog(bucket, hist), NULL);
    else
	xv_set(histo_ctrl->min2, PANEL_VALUE, 0, NULL);
    bucket = hist->interval[1].upper;
    if (bucket != NUM_BUCKETS - 1)
	xv_set(histo_ctrl->max2, PANEL_VALUE, btog(bucket, hist), NULL);
    else
	xv_set(histo_ctrl->max2, PANEL_VALUE, 0, NULL);

    bucket = hist->interval[2].lower;
    if (bucket != 0)
	xv_set(histo_ctrl->min3, PANEL_VALUE, btog(bucket, hist), NULL);
    else
	xv_set(histo_ctrl->min3, PANEL_VALUE, 0, NULL);
    bucket = hist->interval[2].upper;
    if (bucket != NUM_BUCKETS - 1)
	xv_set(histo_ctrl->max3, PANEL_VALUE, btog(bucket, hist), NULL);
    else
	xv_set(histo_ctrl->max3, PANEL_VALUE, 0, NULL);

    bucket = hist->interval[3].lower;
    if (bucket != 0)
	xv_set(histo_ctrl->min4, PANEL_VALUE, btog(bucket, hist), NULL);
    else
	xv_set(histo_ctrl->min4, PANEL_VALUE, 0, NULL);
    bucket = hist->interval[3].upper;
    if (bucket != NUM_BUCKETS - 1)
	xv_set(histo_ctrl->max4, PANEL_VALUE, btog(bucket, hist), NULL);
    else
	xv_set(histo_ctrl->max4, PANEL_VALUE, 0, NULL);

    bucket = hist->interval[4].lower;
    if (bucket != 0)
	xv_set(histo_ctrl->min5, PANEL_VALUE, btog(bucket, hist), NULL);
    else
	xv_set(histo_ctrl->min5, PANEL_VALUE, 0, NULL);
    bucket = hist->interval[4].upper;
    if (bucket != NUM_BUCKETS - 1)
	xv_set(histo_ctrl->max5, PANEL_VALUE, btog(bucket, hist), NULL);
    else
	xv_set(histo_ctrl->max5, PANEL_VALUE, 0, NULL);

    bucket = hist->interval[5].lower;
    if (bucket != 0)
	xv_set(histo_ctrl->min6, PANEL_VALUE, btog(bucket, hist), NULL);
    else
	xv_set(histo_ctrl->min6, PANEL_VALUE, 0, NULL);
    bucket = hist->interval[5].upper;
    if (bucket != NUM_BUCKETS - 1)
	xv_set(histo_ctrl->max6, PANEL_VALUE, btog(bucket, hist), NULL);
    else
	xv_set(histo_ctrl->max6, PANEL_VALUE, 0, NULL);

    bucket = hist->interval[6].lower;
    if (bucket != 0)
	xv_set(histo_ctrl->min7, PANEL_VALUE, btog(bucket, hist), NULL);
    else
	xv_set(histo_ctrl->min7, PANEL_VALUE, 0, NULL);
    bucket = hist->interval[6].upper;
    if (bucket != NUM_BUCKETS - 1)
	xv_set(histo_ctrl->max7, PANEL_VALUE, btog(bucket, hist), NULL);
    else
	xv_set(histo_ctrl->max7, PANEL_VALUE, 0, NULL);

    bucket = hist->interval[7].lower;
    if (bucket != 0)
	xv_set(histo_ctrl->min8, PANEL_VALUE, btog(bucket, hist), NULL);
    else
	xv_set(histo_ctrl->min8, PANEL_VALUE, 0, NULL);
    bucket = hist->interval[7].upper;
    if (bucket != NUM_BUCKETS - 1)
	xv_set(histo_ctrl->max8, PANEL_VALUE, btog(bucket, hist), NULL);
    else
	xv_set(histo_ctrl->max8, PANEL_VALUE, 0, NULL);

    return panel_text_notify(item, event);
}

/*******************************************/
histo_control_init()
{

    xv_set(histo_ctrl->sel1, PANEL_VALUE, 0, NULL);
    xv_set(histo_ctrl->sel2, PANEL_VALUE, 0, NULL);
    xv_set(histo_ctrl->sel3, PANEL_VALUE, 0, NULL);
    xv_set(histo_ctrl->sel4, PANEL_VALUE, 0, NULL);
    xv_set(histo_ctrl->sel5, PANEL_VALUE, 0, NULL);
    xv_set(histo_ctrl->sel6, PANEL_VALUE, 0, NULL);
    xv_set(histo_ctrl->sel7, PANEL_VALUE, 0, NULL);
    xv_set(histo_ctrl->sel8, PANEL_VALUE, 0, NULL);

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

}

/*******************************************/

void
histo_refresh_proc(item, event)
    Panel_item item;
    Event    *event;
{
    struct hcontext *hist, *histo_by_panelwin();

#ifdef DEBUG
    fputs("trace: trace_refresh_proc\n", stderr);
#endif

    hist = histo_by_panelwin(event_window(event));
    if (hist == NULL)
	return;

    /* repaint the window */
    histo_repaint_proc(hist->histo_display->histocanv,
		       hist->paintwin, display,
		       hist->hxid, (Xv_xrectlist *) NULL);

}
