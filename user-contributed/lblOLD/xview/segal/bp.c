/*
 *	bp.c
 *
 *      Bryan Skene, LBL January 1992
 *      part of 'segal'
 */
 
#include "common.h"
 
#include "bp_ui.h"
 
/* Allocation :) */
bp_win_objects *bp_win;
 
/*****************************************************/
void
bp_win_init(owner)
Xv_opaque owner;
{
        bp_win = bp_win_objects_initialize(NULL, owner);
 
        /* bp_tools info initialization */
/*
        bp_tools.whatever = 0;
*/
}
 
/*****************************************************/
void
map_bp(item, event)
Panel_item      item;
Event           *event;
{
        fputs("segal: map_bp\n", stderr);
        /* Map / Unmap toggle */
        if (xv_get(bp_win->win, XV_SHOW, NULL) == FALSE) {
                xv_set(bp_win->win,
                        XV_SHOW, TRUE,
                        NULL);
        }
        else {
                xv_set(bp_win->win,
                        XV_SHOW, FALSE,
                        NULL);
        }
}
 
/*****************************************************/
Menu_item
test_proc_2d(item, op)
        Menu_item       item;
        Menu_generate   op;
{
        Xv_opaque ip = (Xv_opaque) xv_get(item, XV_KEY_DATA, INSTANCE);
 
        switch (op) {
        case MENU_DISPLAY:
        case MENU_DISPLAY_DONE:
        case MENU_NOTIFY_DONE:
                break;
 
        case MENU_NOTIFY:
                fputs("bp: test_proc_2d: MENU_NOTIFY\n", stderr);
                break;
        }
        return item;
}
 
/*****************************************************/
Menu_item
menu_map_surf_fit(item, op)
        Menu_item       item;
        Menu_generate   op;
{
        Xv_opaque ip = (Xv_opaque) xv_get(item, XV_KEY_DATA, INSTANCE);
 
        switch (op) {
        case MENU_DISPLAY:
        case MENU_DISPLAY_DONE:
        case MENU_NOTIFY_DONE:
                break;
 
        case MENU_NOTIFY:
                fputs("bp: map_surf_fit: MENU_NOTIFY\n", stderr);
		map_surf_fit();
                break;
        }
        return item;
}

 
/*****************************************************/
 
/*****************************************************/
 
/*****************************************************/
void
do_nothing()
{
	void refresh_display();

	/* do some image processing on himage.data, m[i].data,
	 * or whatever.  Store results in himage.data, etc.
	 * Then, call refresh_display(); to see the results
	 */

	refresh_display();
}
