/*
 * file.c
 */

#include <stdio.h>
#include "ui.h"
#include "sm.h"
#include "gfm.h"

int       gfm_load_callback();
int       gfm_save_callback();

/***************************************/
/*
 * Notify callback function for `lmode'.
 */
void
lmode_proc(item, value, event)
    Panel_item item;
    int       value;
    Event    *event;
{
    set_lmode(value);
#ifdef DEBUG
    fprintf(stderr, "file: lmode_proc: value: %u\n", value);
#endif
}

/*
 * Notify callback function for `load'.
 */
void
load_proc(item, event)
    Panel_item item;
    Event    *event;
{
    char     *fname = (char *) xv_get(file_win->l_fname, PANEL_VALUE);

    if (strlen(fname) == 0) {
	gfm_activate(gfm_ctrl, NULL, NULL, NULL, gfm_load_callback,
		     NULL, GFM_LOAD);
	return;
    }
    if (state_dispatch(LOAD, (caddr_t *) fname) == IMG_LOADED) {
	xv_set(file_win->window1, XV_SHOW, FALSE, NULL);
    }
}


/*******************************************************/
gfm_load_callback(ip, dir, file)
    gfm_popup_objects *ip;
    char     *dir;
    char     *file;
{
    char     *fname;

    if (dir == NULL || file == NULL)
	return;

    fname = strcat(dir, "/");
    fname = strcat(fname, file);

    if (state_dispatch(LOAD, (caddr_t *) fname) == IMG_LOADED) {
	xv_set(file_win->window1,
	       XV_SHOW, FALSE, NULL);
	xv_set(ip->popup, XV_SHOW, FALSE, NULL);
    }
}

/*******************************************************/
gfm_save_callback(ip, dir, file)
    gfm_popup_objects *ip;
    char     *dir;
    char     *file;
{
    char     *fname;

    if (dir == NULL || file == NULL)
	return;

    fname = strcat(dir, "/");
    fname = strcat(fname, file);

    if (save_image(fname, ip->load) >= 0) {
	xv_set(file_win->window1, XV_SHOW, FALSE, NULL);
	xv_set(ip->popup, XV_SHOW, FALSE, NULL);
    }
}

/*******************************************************/
/*
 * Notify callback function for `l_fname'.
 */
Panel_setting
lname_proc(item, event)
    Panel_item item;
    Event    *event;
{
    load_proc(item, event);
    return panel_text_notify(item, event);
}

/*
 * Notify callback function for `smode'.
 */
void
smode_proc(item, value, event)
    Panel_item item;
    int       value;
    Event    *event;
{

    char      rastname[80];
    char     *fname;

    set_smode(value);

    fname = (char *) xv_get(base_win->image_fname, PANEL_LABEL_STRING, NULL);
    switch (value) {
    case 0:
	sprintf(rastname, "%s", fname);
	break;
    case 1:
	sprintf(rastname, "%s", fname);
	break;
    case 2:
	sprintf(rastname, "%s.sub", fname);
	break;
    case 3:
	sprintf(rastname, "%s.log", fname);
	break;
    case 4:
	    sprintf(rastname, "%s.ras", fname);
	break;
    case 5:
	sprintf(rastname, "%s.trace", fname);
	break;
    case 6:
	sprintf(rastname, "%s.hist", fname);
	break;
    default:
	sprintf(rastname, "%s", fname);
	break;
    }

    xv_set(file_win->s_fname, PANEL_VALUE, rastname, NULL);

#ifdef DEBUG
    fprintf(stderr, "file: smode_proc: value: %u\n", value);
#endif
}

/*
 * Notify callback function for `save'.
 */
void
save_proc(item, event)
    Panel_item item;
    Event    *event;
{
    char     *fname = (char *) xv_get(file_win->s_fname, PANEL_VALUE);

    if (strlen(fname) == 0) {
	gfm_activate(gfm_ctrl, NULL, NULL, NULL, gfm_save_callback,
		     NULL, GFM_SAVE);
	return;
    }
    if (save_image(fname, item) >= 0) {
	xv_set(file_win->window1,
	       XV_SHOW, FALSE, NULL);
    }
#ifdef DEBUG
    fputs("file: save_proc\n", stderr);
#endif
}

/*
 * Notify callback function for `s_fname'.
 */
Panel_setting
sname_proc(item, event)
    Panel_item item;
    Event    *event;
{
    save_proc(item, event);
    return panel_text_notify(item, event);
}

/*
 * Notify callback function for `chooser_load'.
  */
void
chooser_load_proc(item, event)
    Panel_item item;
    Event    *event;
{
    gfm_activate(gfm_ctrl, NULL, NULL, NULL, gfm_load_callback,
		 NULL, GFM_LOAD);
}

 /*
  * Notify callback function for `chooser_save'.
  */
void
chooser_save_proc(item, event)
    Panel_item item;
    Event    *event;
{
    gfm_activate(gfm_ctrl, NULL, NULL, NULL, gfm_save_callback,
		 NULL, GFM_SAVE);
}
