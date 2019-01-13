/*
 *	mask_log.c	For mask type selection
 *
 */

#include "common.h"

#include "frame_control.h"

#include "mask_log_ui.h"

/* Allocation :) */
mask_log_win_objects *mask_log_win;

static int selected_m;

/*****************************************************/
void
mask_log_win_init(owner)
Xv_opaque owner;
{
	mask_log_win = mask_log_win_objects_initialize(NULL, owner);
}

/*****************************************************/
void
map_mask_log(item, event)
Panel_item      item;
Event           *event;
{
        /* Map / Unmap toggle */
	if (xv_get(mask_log_win->win, XV_SHOW, NULL) == FALSE) {
		xv_set(mask_log_win->win,
		XV_SHOW, TRUE,
		NULL);
	}
	else xv_set(mask_log_win->win,
		XV_SHOW, FALSE,
		NULL);
}

/*****************************************************/
int
update_mask_log_proc(item, string, client_data, op, event)
Panel_item      item;
char            *string;
Xv_opaque       client_data;
Panel_list_op   op;
Event           *event;
{
	int	find_mask_in_list();

        switch(op) {
        case PANEL_LIST_OP_SELECT:
                fprintf(stderr, "mask_log: update_mask_log_proc: PANEL_LIST_OP_SELECT: %s\n",string);

		selected_m = find_mask_in_list(string);
		xv_set(mask_log_win->set_mask_type,
			PANEL_VALUE, m[selected_m].mask_type,
			NULL);
		xv_set(mask_log_win->set_mask_color,
			PANEL_VALUE, m[selected_m].mask_color,
			NULL);
                break;
        }
        return XV_OK;
}

/*****************************************************/
void
add_to_mask_log(index)
int index;
{
	char foo[80], tag[10];
	/* add a mask's filename to the mask log */
	if(index == 0) {
		m[index].mask_type = MASK_EDIT;
		m[index].mask_color = GREEN_STANDOUT;
		strcpy(tag, "E");
	}
	else {
		m[index].mask_type = MASK_NO_APPLY;
		m[index].mask_color = BLUE_STANDOUT;
		strcpy (tag, "0");
	}
	sprintf(foo, "<%s> %s", tag, m[index].filename);
	(void) xv_set(mask_log_win->ls_mask_filenames,
		PANEL_LIST_INSERT, index,
		PANEL_LIST_STRING, index,
			foo,
		NULL);
	sprintf(foo, "Masks Loaded: %3d", index + 1);
	(void) xv_set(mask_log_win->msg_masks_loaded,
		PANEL_LABEL_STRING, foo,
		NULL);
}

/*****************************************************/
int
find_mask_in_list(string)
char *string;
{
	int i;

	for(i = 0; i < segal.masks; i++) {
		if(strcmp(m[i].filename, &string[4]) == 0)
			return(i);
	}

	fprintf(stderr, "Did Not Find In List!\n");
	return(-1); /* Did not find ... shouldn't happen, though */
}

/*****************************************************/
void
set_mask_type(item, value, event)
Panel_item      item;
int		value;
Event           *event;
{
	void	update_states_after_changing_edit_m();

	char	foo[80], tag[20];

	/* Mask will now be of type ... */
	switch(value) {
	case MASK_NO_APPLY :
		strcpy(tag, "0");
		break;

	case MASK_EDIT :
		strcpy(tag, "E");

		/* The old Edit Mask now is Not Applied ... */
		/* (Can have at most one Edit Mask at a time) */
		m[segal.edit_m].mask_type = MASK_NO_APPLY;

		sprintf(foo, "<0> %s", m[segal.edit_m].filename);
		(void) xv_set(mask_log_win->ls_mask_filenames,
			PANEL_LIST_STRING, segal.edit_m, foo,
			NULL);

		segal.edit_m = selected_m;
		update_states_after_changing_edit_m();
		break;

	case MASK_EXCLUSIVE :
		strcpy(tag, "-");
		break;

	case MASK_INCLUSIVE :
		strcpy(tag, "+");
		break;

	default :
		break;
	}

	sprintf(foo, "<%s> %s", tag, m[selected_m].filename);
	(void) xv_set(mask_log_win->ls_mask_filenames,
		PANEL_LIST_STRING, selected_m, foo,
		NULL);

	m[selected_m].mask_type = value;
}

/*****************************************************/
void
set_mask_color(item, value, event)
Panel_item      item;
int		value;
Event           *event;
{
	m[selected_m].mask_color = value;
}

/*****************************************************/
Notify_value
edit_mask_defined(win, event, arg, type)
Xv_window       win;
Event           *event;
Notify_arg      arg;
Notify_event_type type;
{
	void set_mask_type();

	int i, found;

	switch(event_id(event)) {
	case LOC_WINEXIT :
		/* Make sure one of the masks is of mask_type MASK_EDIT ... */
		found = FALSE;
		for(i = 0; i < segal.masks; i++)
			if(m[i].mask_type == MASK_EDIT)
				found = TRUE;
		if(!found) {
			/* Pop up a warning box, stay in, make 'em pick one */
			/* For Now, Default is m[0] = the edit mask iff there
			 * is no other edit mask defined
			 */
			selected_m = 0;
			set_mask_type(NULL, MASK_EDIT, NULL);
			xv_set(mask_log_win->set_mask_type,
				PANEL_VALUE, MASK_EDIT,
				NULL);
			xv_set(mask_log_win->set_mask_color,
				PANEL_VALUE, m[0].mask_color,
				NULL);
			xv_set(mask_log_win->ls_mask_filenames,
				PANEL_LIST_SELECT, 0, TRUE,
				NULL);
		}
		break;
	default :
		break;
	}

	return notify_next_event_func(win, (Notify_event) event, arg, type);
}

/*****************************************************/
void
update_states_after_changing_edit_m()
{
	void    draw_frame_statuses();

	/* frame_control */
	draw_frame_statuses();
	xv_set(frame_control_win->set_mask_frame_status,
		PANEL_VALUE, m[segal.edit_m].frames[segal.curr_frame].frame_status,
		NULL);

	/* pixedit, zoom, blend, view, etc. */
}

/*****************************************************/

/*****************************************************/

/*****************************************************/
