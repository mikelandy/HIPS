/*
 *	surf_fit.c
 *
 *      Braham Parvin, LBL January 1992
 *      part of 'segal'
 */
 
#include "common.h"
 
#include "surf_fit_ui.h"
 
/* Allocation :) */
surf_fit_win_objects *surf_fit_win;
 
/*****************************************************/
void
surf_fit_win_init(owner)
Xv_opaque owner;
{
        surf_fit_win = surf_fit_win_objects_initialize(NULL, owner);
 
        /* surf_fit_tools info initialization */
/*
        surf_fit_tools.whatever = 0;
*/
}
 
/*****************************************************/
void
map_surf_fit(item, event)
Panel_item      item;
Event           *event;
{
        fputs("segal: map_surf_fit\n", stderr);
        /* Map / Unmap toggle */
        if (xv_get(surf_fit_win->win, XV_SHOW, NULL) == FALSE) {
                xv_set(surf_fit_win->win,
                        XV_SHOW, TRUE,
                        NULL);
        }
        else {
                xv_set(surf_fit_win->win,
                        XV_SHOW, FALSE,
                        NULL);
        }
}
 
/*****************************************************/
Panel_setting
set_surf_fit_k_proc(item, event)
Panel_item      item;
Event           *event;
{
	char *value = (char *) xv_get(item, PANEL_VALUE);
	
	seg_2d.k = (double) atof(value);
}

/*****************************************************/
void
set_surf_fit_iterations_proc(item, value, event)
Panel_item      item;
int		value;
Event           *event;
{
	seg_2d.iterations = value;
}

/*****************************************************/
void
set_surf_fit_alpha_proc(item, value, event)
Panel_item      item;
int		value;
Event           *event;
{
	seg_2d.alpha = value;
}

/*****************************************************/
void
surf_fit_proc()
{
	void refresh_display();

	fprintf(stderr, "alpha = %d, k = %lf, iterations = %d\n",
		seg_2d.alpha, seg_2d.k, seg_2d.iterations);

	/* do some image processing on himage.data, m[i].data,
	 * or whatever.  Store results in himage.data, etc.
	 * Then, call refresh_display(); to see the results
	 */

	refresh_display();
}
