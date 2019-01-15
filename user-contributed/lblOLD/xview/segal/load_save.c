/*
 *  load_save.c   routines for loading and saving of images   -Brian Tierney
 *
 *	NOTE: Also includes a semi-"state dispatch" mechanism (see load_proc)
 */

#include "common.h"

#include "load_save_ui.h"

/* Allocation :) */
load_save_pop_load_objects *load_save_pop_load;
load_save_pop_create_objects *load_save_pop_create;

#include "display_control.h"
#include "frame_control.h"
#include "hips.h"
#include "image_reg.h"
#include "mask_control.h"
#include "mask_log.h"
#include "orig_view.h"
#include "pixedit.h"
#include "segal.h"
#include "threshold.h"
#include "view.h"

#define IMAGE 0
#define MASK 1
/* Note: may want to add memory mapping to this some day
     for greater speed on multi-framed images */

void write_mask_frame();

extern char *in_list;


/*****************************************************/
void
load_save_init(owner)
Xv_opaque owner;
{

	load_save_pop_load = load_save_pop_load_objects_initialize(NULL, owner);
	load_save_pop_create = load_save_pop_create_objects_initialize(NULL, owner);
}

/**********************************************************/
Menu_item
menu_load_proc(item, op)
	Menu_item	item;
	Menu_generate	op;
{
	switch (op) {
	case MENU_DISPLAY:
	case MENU_DISPLAY_DONE:
	case MENU_NOTIFY_DONE:
		break;

	case MENU_NOTIFY:
		(void) xv_set(load_save_pop_load->pop_load,
			XV_SHOW, TRUE,
			NULL);
		break;
	}
	return item;
}

/**********************************************************/
Menu_item
menu_save_frame_proc(item, op)
	Menu_item	item;
	Menu_generate	op;
{
	void write_image_frame();
	void write_mask_frame();

	segal_win_objects * ip = (segal_win_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	Panel panel = (Panel) xv_get(item, PANEL_PARENT_PANEL, NULL);
	int result;
	
	switch (op) {
	case MENU_DISPLAY:
	case MENU_DISPLAY_DONE:
	case MENU_NOTIFY_DONE:
		break;

	case MENU_NOTIFY:
		fputs("segal: menu_save_frame_proc: MENU_NOTIFY\n", stderr);
		if(himage.changed) {
			result = notice_prompt(panel, NULL,
				NOTICE_MESSAGE_STRINGS,
				"Save IMAGE frame: Are you sure?", NULL,
				NOTICE_BUTTON_YES, "Save IMAGE Frame",
				NOTICE_BUTTON_NO, "Do Not Save IMAGE Frame",
				NULL);

			if (result == NOTICE_YES) write_image_frame(); 

			/* until the null parent window bug is found ... */
			write_image_frame();
		}

		if(segal.changed) {
			result = notice_prompt(panel, NULL,
				NOTICE_MESSAGE_STRINGS,
				"Save MASK frame: Are you sure?", NULL,
				NOTICE_BUTTON_YES, "Save MASK Frame",
				NOTICE_BUTTON_NO, "Do Not Save MASK Frame",
				NULL);

			if (result == NOTICE_YES) write_mask_frame(1, segal.edit_m); 

			/* until the null parent window bug is found ... */
			write_mask_frame(1, segal.edit_m);
		}

		break;
	}
	return item;
}

/**********************************************************/
Menu_item
menu_create_proc(item, op)
	Menu_item	item;
	Menu_generate	op;
{
	segal_win_objects * ip = (segal_win_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	
	switch (op) {
	case MENU_DISPLAY:
	case MENU_DISPLAY_DONE:
	case MENU_NOTIFY_DONE:
		break;

	case MENU_NOTIFY:
		fputs("segal: menu_save_as_proc: MENU_NOTIFY\n", stderr);
		(void) xv_set(load_save_pop_create->pop_create,
			XV_SHOW, TRUE,
			NULL);
		sprintf(m[segal.masks].filename, "%s.mask", himage.filename);
		(void) xv_set(load_save_pop_create->create_dname,
			PANEL_VALUE, himage.path,
			NULL);
		(void) xv_set(load_save_pop_create->create_fname,
			PANEL_VALUE, m[segal.masks].filename,
			NULL);
		panel_paint(load_save_pop_create->create_dname, PANEL_CLEAR);
		panel_paint(load_save_pop_create->create_fname, PANEL_CLEAR);
		break;
	}
	return item;
}

/**********************************************************/
Menu_item
menu_quit_proc(item, op)
	Menu_item	item;
	Menu_generate	op;
{
	segal_win_objects * ip = (segal_win_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	Panel     panel = (Panel) xv_get(item, PANEL_PARENT_PANEL);
	int       result;
	char	fname[MAXPATHLEN];
	FILE	*fp;
	int	i, j, fd;
	
	switch (op) {
	case MENU_DISPLAY:
	case MENU_DISPLAY_DONE:
	case MENU_NOTIFY_DONE:
		break;

	case MENU_NOTIFY:
		fputs("segal: menu_quit_proc: MENU_NOTIFY\n", stderr);

		if (segal.changed == 1) {
			result = notice_prompt(panel, NULL,
				NOTICE_MESSAGE_STRINGS,
				"Mask image have not been saved!", NULL,
				NOTICE_BUTTON_YES, "Save mask image",
				NOTICE_BUTTON_NO, "Exit without saving",
				NULL);

			if (result == NOTICE_YES) {
				write_mask_frame(0, segal.edit_m);
			}
		}
		/* reset mouse speed */
		/*
		XChangePointerControl(display, True, True, 1, 1, 1);
		*/

		fclose(himage.fp);

		for(i = 0; i < segal.masks; i++) {
			fclose(m[i].fp);
			/* save all the frame info arrays */
			if((fp = fopen(m[i].frame_filename, "w")) == NULL)
				fprintf(stderr, "Can't open frame  file for %s ...\n", m[i].frame_filename);
			else {
				fprintf(stderr, "Saving %s\n", m[i].frame_filename);
				for(j = 0; j < segal.frames; j++)
					fprintf(fp, "%d\n", m[i].frames[j].frame_status);
				fclose(fp);
			}
		}
		xv_set(segal_win->win, FRAME_NO_CONFIRM, TRUE, NULL);
		(void) xv_destroy_safe(segal_win->win);
		/* this will also kill all child windows */
		break;
	}
	return item;
}

/**********************************************************/
void
set_load_which(item, value, event)
Panel_item item;
int value;
Event *event;
{
	switch(value) {
	case 0:
		(void) xv_set(load_save_pop_load->load_fname,
			PANEL_VALUE, himage.filename,
			NULL);
		break;
		
	case 1:
		sprintf(m[segal.masks].filename, "%s.mask", himage.filename);
		(void) xv_set(load_save_pop_load->load_fname,
			PANEL_VALUE, m[segal.masks].filename,
			NULL);
		break;
	}
	panel_paint(load_save_pop_load->load_fname, PANEL_CLEAR);
}

/**********************************************************/
void
but_load_proc(item, event)
Panel_item      item;
Event           *event;
{
	int load_proc();
	char *get_non_comment();

	FILE *list_fp;
	char	path[MAXPATHLEN], fname[MAXPATHLEN];

	switch(xv_get(load_save_pop_load->load_which, PANEL_VALUE, NULL)) {
	case 0 : /* load an image */
		strcpy(himage.path, (char *) xv_get(load_save_pop_load->load_dname, PANEL_VALUE, NULL));
		strcpy(himage.filename, (char *) xv_get(load_save_pop_load->load_fname, PANEL_VALUE, NULL));

		if(!load_proc(IMAGE)) {
			(void) xv_set(load_save_pop_load->pop_load,
				XV_SHOW, FALSE,
				NULL);
			(void) xv_set(load_save_pop_create->create_fname,
				PANEL_VALUE, himage.filename,
				NULL);
		}
		break;
	case 1 : /* load a mask */
		strcpy(m[segal.masks].path, (char *) xv_get(load_save_pop_load->load_dname, PANEL_VALUE, NULL));
		strcpy(m[segal.masks].filename, (char *) xv_get(load_save_pop_load->load_fname, PANEL_VALUE, NULL));

		if(!load_proc(MASK)) {
			(void) xv_set(load_save_pop_load->pop_load,
				XV_SHOW, FALSE,
				NULL);
			(void) xv_set(display_control_win->display_type,
				PANEL_VALUE, 2,
				NULL);
			(void) xv_set(mask_control_win->display_type,
				PANEL_VALUE, 2,
				NULL);
			change_image_proc(event, 2, item);
		}
		break;
	case 2 : /* load a list stored in file in_list */
		/* loads in:
		 * 	1. A path,
		 *	2. An image,
		 *	3. Followed by 0 or more masks,
		 * listed in list_fname.
		 */

		if((list_fp = fopen(in_list, "r")) == NULL) {
			fprintf(stderr, "Couldn't open list file %s\n", in_list);
			return;
		}

		/* First, the path */
		strcpy(path, get_non_comment(list_fp));
		xv_set(load_save_pop_load->load_dname,
			PANEL_VALUE, path,
			NULL);

		/* Second, the image */
		xv_set(load_save_pop_load->load_which,
			PANEL_VALUE, 0,
			NULL);
		strcpy(fname, get_non_comment(list_fp));
		xv_set(load_save_pop_load->load_fname,
			PANEL_VALUE, fname,
			NULL);
		but_load_proc(NULL, NULL); /* NOTICE: Recursion */

		/* Third, the masks */
		xv_set(load_save_pop_load->load_which,
			PANEL_VALUE, 1,
			NULL);
		while(strcmp(strcpy(fname, get_non_comment(list_fp)), "**EOF") != 0) {
			xv_set(load_save_pop_load->load_fname,
				PANEL_VALUE, fname,
				NULL);
			but_load_proc(NULL, NULL); /* NOTICE: Recursion */
		}
		fclose(list_fp);
		break;
	default :
		break;
	}
}

/**********************************************************/
char
*get_non_comment(fp)
FILE *fp;
{
/* Really useful function for reading in configuration data ... */
/* If the line starts with 2 '**' then it is skipped */
	int comment;
	char foo[MAXPATHLEN];

	while(fscanf(fp, "%s", foo) != EOF)
		if(strncmp(foo, "**", 2) != 0) {
		/* i.e. Doesn't start w/ '**' */
			if(!comment) return(foo);
		}
		else {
			if(comment == 0) comment = 1;
			else comment = 0;
		}
	return("**EOF");
}

/**********************************************************/
void
but_create_proc(item, event)
Panel_item      item;
Event           *event;
{
	void	update_state_after_loading();
	void	add_to_mask_log();
	int	create_mask_file();

	fprintf(stderr, "creating mask ...\n");
	strcpy(m[segal.masks].path, (char *) xv_get(load_save_pop_create->create_dname,
		PANEL_VALUE, NULL));
	strcpy(m[segal.masks].filename, (char *) xv_get(load_save_pop_create->create_fname,
		PANEL_VALUE, NULL));

	if(create_mask_file(item, event) == -1) {
		fprintf(stderr, "Error creating mask file.\n");
		/* exit(-1); */
	}
	
	(void) xv_set(load_save_pop_create->pop_create,
		XV_SHOW, FALSE,
		NULL);
	(void) xv_set(display_control_win->display_type,
		PANEL_VALUE, 2,
		NULL);
	add_to_mask_log(segal.masks);
	segal.masks++;

	update_state_after_loading(1, 1, 1);
}

/**************************************************************/
int
load_proc(which)	/* procedure for the 'load' button */
int which;
{
/* Takes care of a lot of house cleaning ... 
 * Returns 0 if successful, 1 otherwise
 */

	void refresh_histogram();
	void draw_filenames();
	void update_state_after_loading();
	void build_ximages();
	void get_ref_frame();
	void orig_view_setup();
	void ref_frame_setup();
	void view_setup();
	void edit_setup();

	int  rval1 = 0, rval2 = 0, rval3 = 0;
	int  mask_loaded, both_loaded;

	set_watch_cursor();

	mask_loaded = 0;
	both_loaded = 0;
	switch(which) {
	case IMAGE :
		if ((rval1 = load_image(!REGARDLESS)) < 0)
			himage.fp = NULL;
		else segal.image_loaded = 1;
		break;
	case MASK :
		if ((rval2 = load_mask(!REGARDLESS)) < 0)
			m[segal.masks].fp = NULL;
		else mask_loaded = 1;
		break;
	default :
		break;
	}

	if(segal.image_loaded && mask_loaded) both_loaded = 1;

	if (!segal.image_loaded && !mask_loaded) {
		unset_watch_cursor();
		return;
	}

	/* Something got loaded! :) */

	draw_filenames();

	update_state_after_loading(segal.image_loaded, mask_loaded, both_loaded);

	if (rval1 > 0 || rval2 > 0) { /* one of the files not loaded already */
		build_ximages();
		if (!segal.image_loaded && mask_loaded)
			segal.display_type = 1;
		else if (segal.display_type != 0)
			segal.display_type = 0;
		xv_set(display_control_win->display_type,
			PANEL_VALUE, segal.display_type,
			NULL);
		xv_set(mask_control_win->display_type,
			PANEL_VALUE, segal.display_type,
			NULL);
	}
	if ((int) xv_get(edit_win->win, XV_SHOW) == TRUE) {
		edit_setup();
		edit_repaint_proc();
	}
	if ((int) xv_get(orig_view_win->win, XV_SHOW) == TRUE) {
		orig_view_setup();
		orig_image_repaint_proc();
	}
	if ((int) xv_get(image_reg_pop_ref_frame->pop_ref_frame,
		XV_SHOW) == TRUE) {
		ref_frame_setup();
		ref_frame_image_repaint_proc();
	}
	if (himage.data != NULL || m[segal.masks].data != NULL) {
		view_setup();
		image_repaint_proc();
		(void) xv_set(display_control_win->display_original,
			PANEL_INACTIVE, FALSE,
			NULL);
	}
	himage.ref_frame = 1;
        get_ref_frame(himage.ref_frame);
	build_ximages();

	unset_watch_cursor();
	return(0);
}

/*************************************************/
void
update_state_after_loading(image_loaded, mask_loaded, both_loaded)
int image_loaded, mask_loaded, both_loaded;
{
	char foo[80];

	/*** display_control ***/
	(void) xv_set(segal_win->but_display,
		PANEL_INACTIVE, FALSE,
		NULL);

	/*** edit_window ***/

	/*** frame_control ***/
	if(segal.frames > 1) {
		(void) xv_set(segal_win->but_frame,
			PANEL_INACTIVE, FALSE,
			NULL);

		frame.stack_rad = 0; /* Nothing below this frame .... */

		(void) xv_set(frame_control_win->curr_frame,
			PANEL_VALUE, segal.curr_frame,
			PANEL_MIN_VALUE, 1,
			PANEL_MAX_VALUE, segal.frames,
			PANEL_SLIDER_WIDTH, segal.frames,
			NULL);
		if(mask_loaded)
			(void) xv_set(frame_control_win->set_mask_frame_status,
				PANEL_VALUE, m[segal.masks].frames[segal.curr_frame].frame_status,
				NULL);
		if(image_loaded)
			(void) xv_set(frame_control_win->set_image_frame_status,
				PANEL_VALUE, himage.frames[segal.curr_frame].frame_reg,
				NULL);
		if(both_loaded) {
			sprintf(foo, "%d", himage.ref_frame);
			(void) xv_set(frame_control_win->text_ref_frame,
				PANEL_VALUE, foo,
				NULL);
		}
	}
	else {
		(void) xv_set(frame_control_win->win,
			XV_SHOW, FALSE,
			NULL);

		(void) xv_set(segal_win->but_frame,
			PANEL_INACTIVE, TRUE,
			NULL);

	}

	/*** image_reg ***/
	if(both_loaded && segal.frames > 1) {
		(void) xv_set(segal_win->but_image_reg,
			PANEL_INACTIVE, FALSE,
			NULL);
	}

	/*** mask_control ***/
	if(mask_loaded) {
		(void) xv_set(segal_win->but_mask,
			PANEL_INACTIVE, FALSE,
			NULL);
		(void) xv_set(mask_control_win->display_type,
			PANEL_VALUE, 2,
			NULL);
		if(segal.frames > 1) {
			(void) xv_set(mask_control_win->set_region,
				PANEL_INACTIVE, FALSE,
				NULL);
			(void) xv_set(mask_control_win->beg_frame,
				PANEL_INACTIVE, FALSE,
				NULL);
			(void) xv_set(mask_control_win->end_frame,
				PANEL_INACTIVE, FALSE,
				NULL);
		}
		else {
			(void) xv_set(mask_control_win->set_region,
				PANEL_INACTIVE, TRUE,
				NULL);
			(void) xv_set(mask_control_win->beg_frame,
				PANEL_INACTIVE, TRUE,
				NULL);
			(void) xv_set(mask_control_win->end_frame,
				PANEL_INACTIVE, TRUE,
				NULL);
		}
	}
	else {
		(void) xv_set(mask_control_win->win,
			XV_SHOW, FALSE,
			NULL);
		(void) xv_set(segal_win->but_mask,
			PANEL_INACTIVE, TRUE,
			NULL);
		(void) xv_set(mask_control_win->set_region,
			PANEL_INACTIVE, TRUE,
			NULL);
		(void) xv_set(mask_control_win->beg_frame,
			PANEL_INACTIVE, TRUE,
			NULL);
		(void) xv_set(mask_control_win->end_frame,
			PANEL_INACTIVE, TRUE,
			NULL);
	}
	panel_paint(mask_control_win->set_region, PANEL_CLEAR);
	panel_paint(mask_control_win->beg_frame, PANEL_CLEAR);
	panel_paint(mask_control_win->end_frame, PANEL_CLEAR);

	/*** mask_log ***/
	if(mask_loaded) {
		(void) xv_set(segal_win->but_mask_log,
			PANEL_INACTIVE, FALSE,
			NULL);
	}

	/*** undo/original ***/
	(void) xv_set(segal_win->but_undo,
		PANEL_INACTIVE, FALSE,
		NULL);

	(void) xv_set(segal_win->but_original,
		PANEL_INACTIVE, FALSE,
		NULL);

	/*** filter ***/
	if(image_loaded || mask_loaded)
		(void) xv_set(segal_win->but_filter,
			PANEL_INACTIVE, FALSE,
			NULL);
	else
		(void) xv_set(segal_win->but_filter,
			PANEL_INACTIVE, TRUE,
			NULL);

	/*** mask_control ***/

	/*** threshold ***/
	if(image_loaded) refresh_histogram();
}

/**********************************************************/
int
load_image(regardless)
int regardless;
{
/* returns: -1 if error,
            0 if file already loaded, and
            1 if successful load

	if 'regardless' == 1 then loads the image regardless of what the last
	image loaded is named.
*/
	void	segal_info_init(), allocate_image_data();
	void draw_hint();

	char	fname[MAXPATHLEN];
	static char prev_fname[MAXPATHLEN];
	int	i, fd, stat_fd;

	strcpy(fname, himage.path);
	strcat(fname, "/");
	strcat(fname, himage.filename);

	if (fname == NULL || strlen(fname) == 0) /* check for blank name */
		return (-1);
	if (!regardless && strcmp(fname, prev_fname) == 0)
	/* only do if new file name and not regardless*/
		return (0);

	fprintf(stderr, " Loading image --> %s ... \n", fname);

	himage.fp = fopen(fname, "r+");
	/* himage.fp = hfopenr(fname); */
	if (himage.fp == NULL) {
		fprintf(stderr, "\n*** error opening file %s\n\n", fname);
		return (-1);
	}
	fread_hdr_a(himage.fp, &himage.hd, fname);
	himage.start = ftell(himage.fp);
	if (himage.hd.pixel_format != PFBYTE) {
		fprintf(stderr, "input sequence must be byte \n");
		return (-1);
	}

	fprintf(stderr, "image size: %d x %d, %d frame(s) \n",
		himage.hd.orows, himage.hd.ocols, himage.hd.num_frame);

	/* free and reallocate if size changes */
	if (segal.rows != himage.hd.orows || segal.cols != himage.hd.ocols) {
		allocate_image_data(himage.hd.orows, himage.hd.ocols);
	}
	if (read_2d_byte_array(himage.fp, himage.data, himage.hd.orows, himage.hd.ocols) == -1)
		return (-1);

	/* intialize segal_info structure */
	segal_info_init(himage.hd);
	segal.display_type = (int) xv_get(display_control_win->display_type,
		PANEL_VALUE, NULL);

	segal.image_loaded = 1;

	strcpy(prev_fname, fname);

	bcopy((char *) himage.data[0],
		(char *) image_buf[0], segal.rows * segal.cols);
	bcopy((char *) himage.data[0], (char *) image_undo_buf[0],
		segal.rows * segal.cols);

	/* get all the frame info for the image file */
	/***CHANGE***/

	draw_hint("File I/O: Load or create a mask");

	return (1);
}

/**************************************************************/
int
load_mask(regardless)
int regardless;
{
/* returns: -1 if error,
            0 if file already loaded, and
            1 if successful load

        if 'regardless' == 1 then loads the image regardless of what the last
	image loaded is named.

*/

/* DON'T FREAK OUT WHEN YOU READ THIS */

	u_char  **alloc_2d_byte_array();
	void	draw_hint();
	int	determine_frame_();
	void	add_to_mask_log();

	char	fname[MAXPATHLEN];
	static char prev_fname[MAXPATHLEN];
	int	i, fd, stat_fd, offset;
	void	segal_info_init();

	strcpy(fname, m[segal.masks].path);
	strcat(fname, "/");
	strcat(fname, m[segal.masks].filename);


	if (fname == NULL || strlen(fname) == 0)
		/* check for blank name */
		return (-1);

	if (!regardless && strcmp(fname, prev_fname) == 0)
		/* only do if new file name */
		return (0);

	fprintf(stderr, " Loading mask %s ... \n", fname);

	m[segal.masks].fp = fopen(fname, "r+");	/* read/write */
	if (m[segal.masks].fp == NULL)
		return (-1);

	fread_hdr_a(m[segal.masks].fp, &m[segal.masks].hd, fname);
	m[segal.masks].start = ftell(m[segal.masks].fp);

	if (m[segal.masks].hd.pixel_format != PFBYTE) {
		fprintf(stderr, "input sequence must be byte \n");
		return (-1);
	}
	if (verbose)
		fprintf(stderr, "mask size: %d x %d, %d frame(s) \n",
			m[segal.masks].hd.orows, m[segal.masks].hd.ocols, m[segal.masks].hd.num_frame);

	if (himage.fp == NULL) {	/* if no image file */
		/* free and reallocate if size changes */
		if (segal.rows != m[segal.masks].hd.orows || segal.cols != m[segal.masks].hd.ocols)
			allocate_image_data(m[segal.masks].hd.orows, m[segal.masks].hd.ocols);
	}
	offset = m[segal.masks].start + segal.curr_frame * segal.rows * segal.cols;
	if (fseek(m[segal.masks].fp, offset, 0) == -1)
		perror("fseek");

	if (himage.fp != NULL) {
		if (segal.rows != m[segal.masks].hd.orows || segal.cols != m[segal.masks].hd.ocols ||
		    segal.frames != m[segal.masks].hd.num_frame) {
			fprintf(stderr, "Error: image and mask must be the same size!! \n");
			segal.display_type = 0;
			(void) xv_set(display_control_win->display_type, 
				PANEL_VALUE, 0, 
				NULL);
			(void) xv_set(mask_control_win->display_type, 
				PANEL_VALUE, 0, 
				NULL);
			return (-1);
		}
		else {
		/* the mask file is found and the mask is the right size */
		m[segal.masks].data = NULL;
		m[segal.masks].data = alloc_2d_byte_array(m[segal.masks].hd.orows,
			m[segal.masks].hd.ocols);
			if (read_2d_byte_array(m[segal.masks].fp,
				m[segal.masks].data, segal.rows, segal.cols) == -1)
				return (-1);
			/* The mask data for frame 0 is now in
			 * m[segal.masks].data[][].
			 * For now, load it into work_buf.
			 */
			bcopy((char *) m[segal.masks].data[0], (char *) work_buf[0],
				segal.rows * segal.cols);
			bcopy((char *) m[segal.masks].data[0], (char *) work_undo_buf[0],
				segal.rows * segal.cols);

			strcpy(prev_fname, fname);

			/* load the frame es from the file fname.stat */
			strcat(fname, ".stat");
			strcpy(m[segal.masks].frame_filename, fname);
			m[segal.masks].frame_fp = fopen(m[segal.masks].frame_filename, "r");
			if(m[segal.masks].frame_fp == NULL) {
			/* can't find  file - create it */
				fprintf(stderr, "Creating %s\n", m[segal.masks].frame_filename);
				m[segal.masks].frame_fp = fopen(m[segal.masks].frame_filename, "w");
				for(i = 0; i < segal.frames; i++) {
					fprintf(m[segal.masks].frame_fp, "0\n");
					m[segal.masks].frames[i].frame_status = determine_frame_status(segal.masks, i);
				}
			}
			else {
			/* read in the mask frame  array */
				fprintf(stderr, "Loading %s ...\n", m[segal.masks].frame_filename);
				for(i = 0; i < segal.frames; i++)
					fscanf(m[segal.masks].frame_fp, "%d", &m[segal.masks].frames[i].frame_status);
			}
			fclose(m[segal.masks].frame_fp);

			add_to_mask_log(segal.masks);

			draw_hint("Crop rectangular region in image");
		}
	}

	/* intialize segal_info structure if not done in load_image() */
	if (himage.fp == NULL) {
		segal_info_init(m[segal.masks].hd);
		segal.display_type = 1;	/* must be set to mask */
		(void) xv_set(display_control_win->display_type, 
			PANEL_VALUE, 0, 
			NULL);
		(void) xv_set(mask_control_win->display_type, 
			PANEL_VALUE, 0, 
			NULL);
	}
	if(segal.edit_m == UNDEF)
		segal.edit_m = segal.masks;

	segal.masks++;

	return (1);
}

/**************************************************************/
int
determine_frame_status(index_mask, index_frame)
int index_mask;
int index_frame;
{
	void	draw_hint();

	int i, j;
	long offset;
	char foo[80];

	sprintf(foo, "Determining  of frame %d\n", index_frame);
	draw_hint(foo);

	/* load mask frame */
	offset = m[index_mask].start + (index_frame * segal.rows * segal.cols);
	if (fseek(m[index_mask].fp, offset, 0) == -1)
		perror("fseek");
	if (read_2d_byte_array(m[index_mask].fp, m[index_mask].data,
		segal.rows, segal.cols) == -1) {
		m[index_mask].fp = NULL;
		return(0);
	}

	/* traverse mask frame */
	for(j = 0; j < segal.rows; j++)
	for(i = 0; i < segal.cols; i++)
		if(m[index_mask].data[j][i])
		  return(FRAME_SAVED);

	return(FRAME_BLANK);
		
}

/**************************************************************/
void
save_undo()
{
	fprintf(stderr, "saving mask and/or image into undo buffers\n");

	if(segal.edit_m != UNDEF)
		bcopy((char *) work_buf[0], (char *) work_undo_buf[0],
			segal.rows * segal.cols);
	bcopy((char *) himage.data[0], (char *) image_undo_buf[0],
		segal.rows * segal.cols);
}

/**************************************************************/
void
allocate_image_data(rows, cols)
    int       rows, cols;
{
    u_char  **alloc_2d_byte_array();
    int i, masks;

    if (segal.rows > 0) {	/* so don't do the 1st time through */
	free_2d_byte_array(himage.data);
	for(masks = 0; masks < segal.masks; masks++) {
		free_2d_byte_array(m[masks].data);
		m[masks].data = NULL;
	}
	free_2d_byte_array(work_buf);
	free_2d_byte_array(mask_buf);
	free_2d_byte_array(image_buf);
	free_2d_byte_array(ref_image_frame_buf);
	free_2d_byte_array(ref_mask_frame_buf);
	free_2d_byte_array(work_undo_buf);
	free_2d_byte_array(image_undo_buf);
	himage.data = work_buf = mask_buf = ref_image_frame_buf = NULL;
	ref_mask_frame_buf = image_buf = work_undo_buf = image_undo_buf = NULL;

	/* Should stack_buf be free'd prior to this? */
	for(i = 0; i < MAX_STACK_RADIUS; i++) {
		stack_buf[MAX_STACK_RADIUS - 1 - i] = NULL;
		stack_buf[MAX_STACK_RADIUS + 1 + i] = NULL;
	}
    }
    if (image != NULL) {
	XDestroyImage(image);
	image = NULL;
    }
    if (orig_image != NULL) {
	XDestroyImage(orig_image);
	orig_image = NULL;
    }
    if (ref_frame_image != NULL) {
	XDestroyImage(ref_frame_image);
	ref_frame_image = NULL;
    }
    if (mask_image != NULL) {
	XDestroyImage(mask_image);
	mask_image = NULL;
    }
    if (blend_image != NULL) {
	XDestroyImage(blend_image);
	blend_image = NULL;
    }
    himage.data = alloc_2d_byte_array(rows, cols);

    /* mask data is allocated during load_mask */

    work_buf = alloc_2d_byte_array(rows, cols);

    mask_buf = alloc_2d_byte_array(rows, cols);

    image_buf = alloc_2d_byte_array(rows, cols);

    ref_image_frame_buf = alloc_2d_byte_array(rows, cols);

    ref_mask_frame_buf = alloc_2d_byte_array(rows, cols);

    work_undo_buf = alloc_2d_byte_array(rows, cols);

    image_undo_buf = alloc_2d_byte_array(rows, cols);

    for(i = 0; i < 2 * MAX_STACK_RADIUS + 1; i++)
	    stack_buf[i] = alloc_2d_byte_array(rows, cols);
}

/*************************************************************/
void
segal_info_init(hd)
struct header hd;
{				/* initializes global structure of info used
				 * everywhere */
    char      segal_mesg[30];

    segal.rows = hd.rows;
    segal.cols = hd.cols;
    segal.frames = hd.num_frame;
    segal.curr_frame = 0;
    poly_proc(NULL,0);
    segal.changed = 0;
    segal.slider1 = (int) xv_get(display_control_win->bg_slider, PANEL_VALUE, 0);
    segal.slider2 = 100 - (int) xv_get(display_control_win->fg_slider, PANEL_VALUE, 0);
    make_blend_lut();

	sprintf(segal_mesg, "Image Size: %dr x %dc x %df",
		segal.rows, segal.cols, segal.frames);
	(void) xv_set(segal_win->msg_image_size, PANEL_LABEL_STRING,
		segal_mesg, NULL);

	sprintf(segal_mesg, "Edit Region Size: Undefined");
	(void) xv_set(segal_win->msg_pixedit, PANEL_LABEL_STRING,
		segal_mesg, NULL);

	(void) xv_set(mask_control_win->set_portion,
		PANEL_INACTIVE, TRUE,
		NULL);
	region.refresh = 1;

	(void) xv_set(frame_control_win->curr_frame, PANEL_VALUE,
		segal.curr_frame + 1, NULL);
}

/****************************************************/
void
get_ref_frame(frame)
int frame;
{
	long offset;

	if (himage.fp != NULL) {
		offset = himage.start + (frame * segal.rows * segal.cols);
		if (fseek(himage.fp, offset, 0) == -1)
			perror("fseek");
		if (read_2d_byte_array(himage.fp, ref_image_frame_buf,
			segal.rows, segal.cols) == -1) {
			fprintf(stderr, "Error reading in reference frame\n");
			return;
		}
	}
	if (segal.masks != NULL) {
		offset = m[segal.edit_m].start + (frame * segal.rows * segal.cols);
		if (fseek(m[segal.edit_m].fp, offset, 0) == -1)
			perror("fseek");
		if (read_2d_byte_array(m[segal.edit_m].fp, ref_mask_frame_buf,
			segal.rows, segal.cols) == -1) {
			fprintf(stderr, "Error reading in reference frame\n");
			return;
		}
	}
}

/****************************************************/
void
get_frame_proc(item, value, event)
Panel_item item;
int             value;
Event    *event;
{
	void refresh_histogram();
	void build_ximages();

	int f, masks;
	long      offset;

	/* write mask frame */
	if (segal.masks && segal.changed != 0) {
		write_mask_frame(1, segal.edit_m);
	}

	segal.curr_frame = value - 1;

	if(segal.curr_frame < grow.matrix_width) {
		frame.stack_rad = segal.curr_frame;
	}
	else if(segal.curr_frame + grow.matrix_width > segal.frames - 1) {
		frame.stack_rad = segal.frames - 1 - segal.curr_frame;
	}
	else frame.stack_rad = grow.matrix_width;

	set_watch_cursor();

	if (himage.fp != NULL) {
		xv_set(frame_control_win->set_image_frame_status,
			PANEL_VALUE, himage.frames[segal.curr_frame].frame_reg,
			NULL);
		/* fill stack */
		if(xv_get(frame_control_win->set_stack_load,
			PANEL_VALUE, NULL) == 0) {
			for(f = 0; f < 2*frame.stack_rad + 1; f++) {
				offset = himage.start + ((segal.curr_frame - frame.stack_rad + f) * segal.rows * segal.cols);
				if (fseek(himage.fp, offset, 0) == -1)
					perror("fseek");
				if (read_2d_byte_array(himage.fp, stack_buf[f],
					segal.rows, segal.cols) == -1)
					return;
			}
		}
		else {

			if (verbose)
				fprintf(stderr, " Loading frame %d ... \n",
					segal.curr_frame + 1);
			frame.stack_rad = 0;
			offset = himage.start + segal.curr_frame
				* segal.rows * segal.cols;
			if(fseek(himage.fp, offset, 0) == -1)
				perror("fseek");
			if(read_2d_byte_array(himage.fp, stack_buf[frame.stack_rad],
				segal.rows, segal.cols) == -1)
				return;
		}

		bcopy((char *) stack_buf[frame.stack_rad][0],
			(char *) himage.data[0], segal.rows * segal.cols);
		bcopy((char *) himage.data[0],
			(char *) image_buf[0], segal.rows * segal.cols);
		bcopy((char *) himage.data[0],
			(char *) image_undo_buf[0], segal.rows * segal.cols);
	}
	if (segal.masks > 0) {
	  xv_set(frame_control_win->set_mask_frame_status,
	    PANEL_VALUE, m[segal.edit_m].frames[segal.curr_frame].frame_status,
	    NULL);
	  for(masks = 0; masks < segal.masks; masks++) {
		offset = m[masks].start + (segal.curr_frame *
			(segal.rows * segal.cols));
		if (fseek(m[masks].fp, offset, 0) == -1)
			perror("fseek");
		if (read_2d_byte_array(m[masks].fp, m[masks].data,
			segal.rows, segal.cols) == -1) {
			m[masks].fp = NULL;
			return;
		}
	  } /* for masks */
	bcopy((char *) m[segal.edit_m].data[0], (char *) work_buf[0],
		segal.rows * segal.cols);
	bcopy((char *) m[segal.edit_m].data[0], (char *) work_undo_buf[0],
		segal.rows * segal.cols);
	} /* if segal.masks > 0 */

	build_ximages();

	if (himage.data != NULL || segal.masks) {
		fprintf(stderr, "*** Repainting the images ***\n");
		image_repaint_proc();
		orig_image_repaint_proc();
	}

	if ((int) xv_get(view_win->win, XV_SHOW, NULL) == TRUE) {
		zoom();
		edit_repaint_proc();
	}

	if ((int) xv_get(threshold_win->win, XV_SHOW, NULL) == TRUE) {
		refresh_histogram();
	}
	unset_watch_cursor();
}

/*****************************************************************/
void
write_image_frame()	/* save a one frame image (SAVE button) */
{
	void draw_filenames();
	long      offset;

	if(himage.fp == NULL) {
		fprintf(stderr, "No image to write to!\n");
		return;
	}

	set_watch_cursor();

	offset = himage.start + (segal.curr_frame *
		(segal.rows * segal.cols));

	if (offset <= 0) {
		fprintf(stderr, "Error: invalid offset for write \n");
		return;
	}
	if (fseek(himage.fp, offset, 0) == -1)
		perror("fseek");
	if (verbose)
		fprintf(stderr, " start: %d, frame size: %d \n", himage.start,
			segal.rows * segal.cols);
	fprintf(stderr, " writing image frame %d, offset: %d, to file %s... \n",
		segal.curr_frame + 1, offset, himage.filename);
	write_2d_byte_array(himage.fp, himage.data, 
		segal.rows, segal.cols);

	himage.changed = 0;
	himage.frames[segal.curr_frame].frame_reg = FRAME_REGISTERED;

	draw_filenames();
	unset_watch_cursor();
}

/**********************************************************/
int
create_mask_file(item, event)
Panel_item item;
Event    *event;
{
/* Returns -1 if unsuccessful, otherwise returns 0 */
	void      write_blank_frames(), write_mask_header();

	char fname[MAXPATHLEN];

	strcpy(fname, m[segal.masks].path);
	strcat(fname, "/");
	strcat(fname, m[segal.masks].filename);
	if (check_valid_mask_name(item, fname, segal.masks) < 0)
		return (-1);

	fprintf(stderr, "creating mask file: %s \n", fname);

	/* create new file */
	if ((m[segal.masks].fp = fopen(fname, "w+")) == NULL) {
		fprintf(stderr, "\n error opening file %s \n\n", fname);
		return (-1);
	}
	write_mask_header(segal.masks);

	if (segal.frames - 1 > 0)
		write_blank_frames(segal.masks);

	write_mask_frame(1, segal.edit_m);
	fclose(m[segal.masks].fp);

	(void) xv_set(segal_win->but_mask,
		PANEL_INACTIVE, FALSE,
		NULL);

	(void) xv_set(segal_win->but_mask_log,
		PANEL_INACTIVE, FALSE,
		NULL);

	return (0);
}

/**********************************************************/
void
write_mask_frame(step_flag, index)
int step_flag;
int index;
{
/* called from step_up and step_down */
	void draw_filenames();

	long      offset;

	if(m[index].fp == NULL) {
		fprintf(stderr, "No mask to write to!\n");
		xv_set(load_save_pop_create->pop_create,
			XV_SHOW, TRUE,
			NULL);
		return;
	}

	set_watch_cursor();

	offset = m[index].start + (segal.curr_frame *
		(segal.rows * segal.cols));

	if (offset <= 0) {
		fprintf(stderr, "Error: invalid offset for write \n");
		return;
	}
	if (fseek(m[index].fp, offset, 0) == -1)
		perror("fseek");
	if (verbose)
		fprintf(stderr, " start: %d, frame size: %d \n", m[index].start,
			segal.rows * segal.cols);
	fprintf(stderr, " writing mask frame %d, offset: %d, to file %s... \n",
		segal.curr_frame + 1, offset, m[index].filename);
	write_2d_byte_array(m[index].fp, work_buf, segal.rows, segal.cols);

	if(m[index].frames[segal.curr_frame].frame_status != FRAME_LOCKED)
		m[index].frames[segal.curr_frame].frame_status = FRAME_SAVED;

	segal.changed = 0;

	draw_filenames();

	if (!step_flag) { /* indicates came from step_up or step_down
			   * proc */
		m[index].data = NULL;
		m[index].data = alloc_2d_byte_array(m[index].hd.orows,
			m[index].hd.ocols);
		bcopy((char *) work_buf[0], (char *) m[index].data[0],
			segal.rows * segal.cols);
	}
	unset_watch_cursor();
}

/*******************************************************/
void
write_blank_frames(index)
int index;
{
/* creates mask image file that is all zeros */
    int       i;
    u_char    *buf;

    buf = Calloc(segal.rows * segal.cols, u_char);

    fprintf(stderr, " Creating blank mask frames... \n");

    for (i = 0; i < segal.frames; i++)
	Fwrite(buf, sizeof(char), segal.rows * segal.cols, m[index].fp);

    cfree((char *) buf);

}

/*********************************************************/
int
check_valid_mask_name(item, fname, index)
Panel_item	item;
char		*fname;
int		index;
{
    int       result;
    Panel     panel = (Panel) xv_get(item, PANEL_PARENT_PANEL);

    if (fname == NULL || (strlen(fname) == 0)) {
	result = notice_prompt(panel, NULL,
			       NOTICE_MESSAGE_STRINGS,
			       "Error: must enter mask file name!", NULL,
			       NOTICE_BUTTON_YES, "OK",
			       NULL);
	return (-1);
    }
    if (strcmp(fname, himage.filename) == 0) {
	result = notice_prompt(panel, NULL,
			       NOTICE_MESSAGE_STRINGS,
			       "Error: Mask file name must be different",
			       "than the image file name!", NULL,
			       NOTICE_BUTTON_YES, "OK",
			       NULL);
	return (-1);
    }

    if ((m[index].fp = fopen(fname, "r")) != NULL) {	/* check if exists */
	result = notice_prompt(panel, NULL,
			       NOTICE_MESSAGE_STRINGS,
			       "Overwrite existing file?", NULL,
			       NOTICE_BUTTON_YES, "Yes",
			       NOTICE_BUTTON_NO, "No",
			       NULL);

	fclose(m[index].fp);
	if (result == NOTICE_NO)
	    return (-1);
    }
    return (0);
}

/*********************************************************/
void
write_mask_header(index)
int index;
{
    int       fd;

    if (m[index].fp == NULL) {
	fprintf(stderr, "ERROR: attempting to write header to un-opened file \n");
	return;
    }
    if (himage.fp != NULL)
	m[index].hd = himage.hd;     /* add to image header */
    else
	init_header(&m[index].hd, "segal", "", segal.frames, "", segal.rows, segal.cols,
		    PFBYTE, 1, "");

    update_header(&m[index].hd, ac, av);
    fwrite_header(m[index].fp, &m[index].hd, m[index].filename);
    m[index].start = ftell(m[index].fp);
}
