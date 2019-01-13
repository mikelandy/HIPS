/*
 *	region_control.c
 *
 *	Bryan Skene, LBL July 1991
 *	part of 'segal'
 */

#include "common.h"

#include "region_control_ui.h"

/* Allocation :) */
region_control_win_objects *region_control_win;

/*****************************************************/
void
region_control_win_init(owner)
Xv_opaque owner;
{
	region_control_win = region_control_win_objects_initialize(NULL, owner);

	/* set region_control_win defaults */
	region.image = 1;
	region.mask = 0;
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
}

/*****************************************************/
void
map_region_control(item, event)
Panel_item      item;
Event           *event;
{
	fputs("segal: map_region_control\n", stderr);
	/* Map / Unmap toggle */
	if (xv_get(region_control_win->win, XV_SHOW, NULL) == FALSE)
		xv_set(region_control_win->win,
			XV_SHOW, TRUE,
			NULL);
	else xv_set(region_control_win->win,
		XV_SHOW, FALSE,
		NULL);
}

/*****************************************************/
void
region_set_proc(item, value, event)
Panel_item      item;
int             value;
Event           *event;
{
	region.image = 0;
	region.mask = 0;

	if(value == 0) region.image = 1;
	if(value == 1) region.mask = 1;
	if(value == 2) {
		region.image = 1;
		region.mask = 1;
	}
}


/*****************************************************/
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

/*****************************************************/

/*****************************************************/

/*****************************************************/

/*****************************************************/
