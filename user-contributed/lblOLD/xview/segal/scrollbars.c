/*
 * scrollbars.c		creates scrollbars for view and edit windows
 *			Bryan Skene, LBL
 *	part of 'segal'
 */

#include "common.h"

#include "pixedit.h"
#include "view.h"

Scrollbar horiz_edit_scrollbar;
Scrollbar vert_edit_scrollbar;

Scrollbar horiz_view_scrollbar;
Scrollbar vert_view_scrollbar;

/**********************************************/
void setup_edit_scrollbars()
{
/* if zoom_image width or height are too big, then put scrollbars on the zoom
 * image
 */
	if(MAX_ZOOM_WIDTH < zoom_info.width)
	horiz_edit_scrollbar = (Scrollbar) xv_create(edit_win->canvas, SCROLLBAR,
		SCROLLBAR_DIRECTION, SCROLLBAR_HORIZONTAL,
		SCROLLBAR_PIXELS_PER_UNIT, zoom_info.mag,
		SCROLLBAR_OBJECT_LENGTH, segal.cols + 1,
		SCROLLBAR_PAGE_LENGTH, region.cols,
		SCROLLBAR_VIEW_LENGTH, region.cols,
		NULL);
	else xv_destroy_safe(horiz_edit_scrollbar);

	if(MAX_ZOOM_HEIGHT < zoom_info.height)
	vert_edit_scrollbar = (Scrollbar) xv_create(edit_win->canvas, SCROLLBAR,
		SCROLLBAR_DIRECTION, SCROLLBAR_VERTICAL,
		SCROLLBAR_PIXELS_PER_UNIT, zoom_info.mag,
		SCROLLBAR_OBJECT_LENGTH, segal.rows + 1,
		SCROLLBAR_PAGE_LENGTH, region.rows,
		SCROLLBAR_VIEW_LENGTH, region.rows,
		NULL);
	else xv_destroy_safe(vert_edit_scrollbar);
}

/**********************************************/
void setup_view_scrollbars()
{
/* if image width or height are too big, then put scrollbars on the
 * image
 */
	if(MAX_VIEW_WIDTH < segal.cols + 1)
	horiz_view_scrollbar = (Scrollbar) xv_create(view_win->canvas, SCROLLBAR,
		SCROLLBAR_DIRECTION, SCROLLBAR_HORIZONTAL,
		SCROLLBAR_PIXELS_PER_UNIT, 1,
		SCROLLBAR_OBJECT_LENGTH, segal.cols + 1,
		SCROLLBAR_PAGE_LENGTH, segal.width,
		SCROLLBAR_VIEW_LENGTH, segal.width,
		NULL);
	else xv_destroy_safe(horiz_view_scrollbar);

	if(MAX_VIEW_HEIGHT < segal.rows + 1)
	vert_view_scrollbar = (Scrollbar) xv_create(view_win->canvas, SCROLLBAR,
		SCROLLBAR_DIRECTION, SCROLLBAR_VERTICAL,
		SCROLLBAR_PIXELS_PER_UNIT, 1,
		SCROLLBAR_OBJECT_LENGTH, segal.rows + 1,
		SCROLLBAR_PAGE_LENGTH, segal.height,
		SCROLLBAR_VIEW_LENGTH, segal.height,
		NULL);
	else xv_destroy_safe(vert_view_scrollbar);
}
