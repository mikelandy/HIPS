/*
 *	mask_log.c	For mask type selection
 *
 */

#include "common.h"

/*****************************************************/
void
add_to_mask_log(index)
int index;
{
	char *mask_list_string();
	void select_mask_in_list();

	char foo[80];

	/* add a mask's filename to the mask log */
	if(index == 0) {
		segal.e_m = 0;
		sprintf(foo, "Mask: %s", m[0].fname);
		xv_set(View_win->msg_mask,
			PANEL_LABEL_STRING, foo,
			NULL);
		xv_set(Paint_win_paint->msg_mask,
			PANEL_LABEL_STRING, foo,
			NULL);
		m[index].mask_type = MASK_EDIT;
		m[index].mask_hue = MASK_GREEN;

		/* activate mask ui stuff */

		xv_set(View_win->but_segment,
			PANEL_INACTIVE, FALSE,
			NULL);

	}
	else {
		m[index].mask_type = MASK_NO_APPLY;
		m[index].mask_hue = MASK_BLUE;
	}
	(void) xv_set(Mask_log_pop_mask_log->ls_mask_filenames,
		PANEL_LIST_INSERT, index,
		PANEL_LIST_STRING, index,
			mask_list_string(index),
		PANEL_LIST_SELECT, index,
			TRUE,
		NULL);

	if(index == 0) {
		xv_set(Mask_log_pop_mask_log->ls_mask_filenames,
			PANEL_LIST_SELECT, 0,
				TRUE,
			NULL);
		select_mask_in_list(mask_list_string(0));
	}

	sprintf(foo, "Masks Loaded: %3d", index + 1);
	(void) xv_set(Mask_log_pop_mask_log->msg_masks_loaded,
		PANEL_LABEL_STRING, foo,
		NULL);
}

/*****************************************************/
char
*mask_list_string(mnum)
int mnum;
{
	char foo[80];

	switch(m[mnum].mask_type) {
	case MASK_NO_APPLY :
		strcpy(foo, "<0> ");
		break;
	case MASK_EDIT :
		strcpy(foo, "<E> ");
		break;
	case MASK_EXCLUSIVE :
		strcpy(foo, "<-> ");
		break;
	case MASK_INCLUSIVE :
		strcpy(foo, "<+> ");
		break;
	default:
		break;
	}

	strcat(foo, m[mnum].fname);
	return(foo);
}

/*****************************************************/
int
find_mask_in_list(string)
char *string;
{
	int i;

	for(i = 0; i < segal.num_m; i++) {
		if(strcmp(m[i].fname, &string[4]) == 0)
			return(i);
	}

	fprintf(stderr, "Did Not Find In List!\n");
	return(-1); /* Did not find ... shouldn't happen, though */
}

/*****************************************************/
void
select_mask_in_list(string)
char *string;
{
	mlog.sel_m = find_mask_in_list(string);
	xv_set(Mask_log_pop_mask_log->set_mask_type,
		PANEL_VALUE, m[mlog.sel_m].mask_type,
		NULL);
	xv_set(File_pop_save_as->text_save_dname,
		PANEL_VALUE, m[mlog.sel_m].dname,
		NULL);
	xv_set(File_pop_save_as->text_save_fname,
		PANEL_VALUE, m[mlog.sel_m].fname,
		NULL);
}

/*****************************************************/
void
set_mask_type_proc(value)
int		value;
{
	void	update_states_after_changing_edit_mask();

	if(value == MASK_EDIT) {
		/* (Can have at most one Edit Mask at a time) */
		if(m[segal.e_m].mask_type == MASK_EDIT) {
			m[segal.e_m].mask_type = MASK_NO_APPLY;
			xv_set(Mask_log_pop_mask_log->ls_mask_filenames,
				PANEL_LIST_STRING, segal.e_m,
					mask_list_string(segal.e_m),
				NULL);
		}

		segal.e_m = mlog.sel_m;
		update_states_after_changing_edit_mask();
	}

	m[mlog.sel_m].mask_type = value;
	xv_set(Mask_log_pop_mask_log->ls_mask_filenames,
		PANEL_LIST_STRING, mlog.sel_m,
			mask_list_string(mlog.sel_m),
		NULL);
}

/*****************************************************/
void
set_mask_hue(item, value, event)
Panel_item      item;
int		value;
Event           *event;
{
	m[mlog.sel_m].mask_hue = value;
}

/*****************************************************/
void
make_edit_mask_defined(event)
Event           *event;
{
	void set_mask_type_proc();

	int i, found;

	if(segal.num_m == 0) return;

	if(event_id(event) == LOC_WINEXIT) {
		/* Make sure one of the masks is of mask_type MASK_EDIT ... */
		found = FALSE;
		for(i = 0; i < segal.num_m; i++)
			if(m[i].mask_type == MASK_EDIT)
				found = TRUE;
		if(!found) {
			/* Pop up a warning box, stay in, make 'em pick one */
			/* For Now, Default is m[0] = the edit mask iff there
			 * is no other edit mask defined
			 */
			mlog.sel_m = 0;
			set_mask_type_proc(NULL, MASK_EDIT, NULL);
			xv_set(Mask_log_pop_mask_log->set_mask_type,
				PANEL_VALUE, MASK_EDIT,
				NULL);
			xv_set(Mask_log_pop_mask_log->ls_mask_filenames,
				PANEL_LIST_SELECT, 0, TRUE,
				NULL);
			xv_set(File_pop_save_as->text_save_dname,
				PANEL_VALUE, m[0].dname,
				NULL);
			xv_set(File_pop_save_as->text_save_fname,
				PANEL_VALUE, m[0].fname,
				NULL);
		}
	}
}

/*****************************************************/
void
update_states_after_changing_edit_mask()
{
	void redisplay_all();

	char foo[80];

	if(img.loaded) redisplay_all();

	sprintf(foo, "Mask: %s", m[segal.e_m].fname);
	xv_set(View_win->msg_mask,
		PANEL_LABEL_STRING, foo,
		NULL);
	xv_set(Paint_win_paint->msg_mask,
		PANEL_LABEL_STRING, foo,
		NULL);
}
