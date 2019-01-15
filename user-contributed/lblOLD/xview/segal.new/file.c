/*
 *	file.c - for use with SEGAL
 *
 *	By Bryan Skene
 *
 *	Uses the complex conversion library by Jin Goujin
 */

#include "common.h"
#include "file_ui.h"

/*****************************************/
void
load_image_header()
{
	void unload_all_masks();
	void init_info_from_header();
	void update_bytes_required();

	char filename[MAXPATHLEN], foo[80];

	strcpy(img.dname, (char *) xv_get(File_pop_load_image->text_image_dname, PANEL_VALUE, NULL));
	strcpy(img.fname, (char *) xv_get(File_pop_load_image->text_image_fname, PANEL_VALUE, NULL));
	sprintf(filename, "%s/%s", img.dname, img.fname);

	fprintf(stderr, "Loading image: %s\n", filename);

	if((img.hd.IN_FP = fopen(filename, "rb")) == NULL) {
		prgmerr(0, "trying to open for input %s", filename);
		return;
	}

	format_init(&img.hd, IMAGE_INIT_TYPE, HIPS, HIPS, *av, "Segal v. 3d");

	if((*img.hd.header_handle)(HEADER_READ, &img.hd, 0, 0, 0) < 0) {
		prgmerr(0, "Unkown image type");
		return;
	}

	/* if color image, we want the data in separate planes */
	img.hd.color_form = CFM_SEPLANE;

	(*img.hd.header_handle)(HEADER_TRANSF, &img.hd, 0);

	unload_all_masks(FALSE);
	init_info_from_header();

	/* set up the ui stuff */
	sprintf(foo, "Image format: %s (%3dR x %3dC x %3dF)",
		ITypeName[img.hd.in_type], img.r, img.c, img.f);
	xv_set(File_pop_load_image->msg_format,
		PANEL_LABEL_STRING, foo,
		PANEL_INACTIVE, FALSE,
		NULL);

	xv_set(File_pop_load_image->msg_bytes_required,
		PANEL_INACTIVE, FALSE,
		NULL);

	xv_set(File_pop_load_image->row_from,
		PANEL_MIN_VALUE, 0,
		PANEL_MAX_VALUE, img.r - 1,
		PANEL_VALUE, segal.r1,
		PANEL_INACTIVE, FALSE,
		NULL);

	xv_set(File_pop_load_image->row_to,
		PANEL_MIN_VALUE, 0,
		PANEL_MAX_VALUE, img.r - 1,
		PANEL_VALUE, segal.r2,
		PANEL_INACTIVE, FALSE,
		NULL);

	xv_set(File_pop_load_image->col_from,
		PANEL_MIN_VALUE, 0,
		PANEL_MAX_VALUE, img.c - 1,
		PANEL_VALUE, segal.c1,
		PANEL_INACTIVE, FALSE,
		NULL);

	xv_set(File_pop_load_image->col_to,
		PANEL_MIN_VALUE, 0,
		PANEL_MAX_VALUE, img.c - 1,
		PANEL_VALUE, segal.c2,
		PANEL_INACTIVE, FALSE,
		NULL);


	xv_set(File_pop_load_image->frm_from,
		PANEL_MIN_VALUE, 0,
		PANEL_MAX_VALUE, img.f - 1,
		PANEL_VALUE, segal.f1,
		PANEL_INACTIVE, !segal.r3d,
		NULL);

	xv_set(File_pop_load_image->frm_to,
		PANEL_MIN_VALUE, 0,
		PANEL_MAX_VALUE, img.f - 1,
		PANEL_VALUE, segal.f2,
		PANEL_INACTIVE, !segal.r3d,
		NULL);

	xv_set(File_pop_load_image->set_color_format,
		PANEL_VALUE, (img.color ? 0 : 1),
		PANEL_INACTIVE, !img.color,
		NULL);
	
	/* (de)activate other color stuff */
	xv_set(Preferences_pop_preferences_display->msg_quantizing,
		PANEL_INACTIVE, !img.color,
		NULL);

	xv_set(Preferences_pop_preferences_display->set_requant_win,
		PANEL_INACTIVE, !img.color,
		NULL);

	xv_set(Preferences_pop_preferences_display->set_requant_quality,
		PANEL_INACTIVE, !img.color,
		NULL);

	xv_set(Preferences_pop_preferences_display->but_requant,
		PANEL_INACTIVE, !img.color,
		NULL);

	xv_set(Threshold_pop_threshold->set_plane,
		PANEL_INACTIVE, !img.color,
		NULL);

	update_bytes_required();

	/* now that we have a valid image, allow loading */
	xv_set(File_pop_load_image->but_load_image,
		PANEL_INACTIVE, FALSE,
		NULL);
}

/*****************************************/
void
load_image()
{
	void init_info_from_loading();
	void view_setup();
	void paint_setup();
	void allocate_buffers();
	LOGIC begin_timer();
	void enq_bg_job();
	void begin_itimer();
	Notify_value bg_load_image_frame();

	init_info_from_loading();

	view_setup();
	paint_setup();

	allocate_buffers();

	/* prepare to load frames beginning with segal.f1 ... */
	img.hd.load_all = 1;
	vprint"img.frame_size = %d, img.hd.channels = %d\n",
		img.frame_size, img.hd.channels);
	img.hd.data = Calloc(img.frame_size * img.hd.channels, byte);

	/*** Load all the frames ***/

	/* gonna do something time intensive ... background it */
	sprintf(timer.message, "Loading frames: %s ...", img.fname);
	if(!begin_timer()) enq_bg_job(JOB_LOAD_IMAGE, 0);
	else {
		segal.bg_i = segal.f1;
		begin_itimer(INTERVAL_SEC, INTERVAL_uSEC);
		notify_set_itimer_func(File_pop_load_image->pop_load_image,
			bg_load_image_frame, ITIMER_REAL, &itimer, NULL);
	}
}

/*****************************************/
Notify_value
bg_load_image_frame()
{
	void set_timer();
	void deq_bg_job();
	void end_timer();
	void get_2d_slice();
	void get_quant_colors();
	void cmap_init();
	void FreeAllColors();
	void build_cmap();
	void redisplay_all();
	void save_image_undo();
	void save_image_orig();

	int x, y, i, size;
	long offset;
	char filename[MAXPATHLEN];

	if(segal.bg_i <= segal.f2) {
		img.hd.fn = segal.bg_i;
		(*img.hd.std_swif)(FI_ACCESS_ABS_FRAME, &img.hd, img.hd.data, img.hd.fn);

		if(segal.color) { /* extract the RGB planes from hd.data */
			size = segal.r * segal.c;

			if(segal.c == img.c) {
			/* unless the #columns change, can extract fast */
				For_rgb bcopy(
				&img.hd.data[i * size + segal.r1 * img.c],
					cbuf[i][segal.bg_i - segal.f1][0],
					segal.r * segal.c);
			}
			else { /* gotta do it the hard way */
				For_rgb
				for(y = 0; y < segal.r; y++)
				for(x = 0; x < segal.c; x++)
					cbuf[i][segal.bg_i - segal.f1][y][x]
					= img.hd.data[(int) (i*size + x+segal.c1 + ((y+segal.r1) * img.c))];
			}
		}
		else {
			if(segal.c == img.c) {
			/* unless the #columns change, can extract fast */
				bcopy(&img.hd.data[segal.r1 * img.c],
					ibuf[segal.bg_i - segal.f1][0],
					segal.r * segal.c);
			}
			else { /* gotta do it the hard way */
				for(y = 0; y < segal.r; y++)
				for(x = 0; x < segal.c; x++)
					ibuf[segal.bg_i - segal.f1][y][x]
						= img.hd.data[(int) (x+segal.c1
						+ ((y+segal.r1) * img.c))];
			}
		}

		segal.bg_i++;

		set_timer((float) (segal.bg_i - segal.f1) / (float) segal.f);
	}
	else {
	/* Done loading image */
	/*	fclose(img.hd.IN_FP);	*/
		img.loaded = TRUE;
		For_all_windows win[i].f = 0;

		/* allocate colors to fit existing colormap */
		image_map = (byte *) malloc(win[WIN_VZ].img_size);
		if(segal.color) {
			For_rgb get_2d_slice(ASPECT_Z, 0, cbuf[i],
				win[WIN_VZ].i_data[i]);

			vprint"*** Getting Quant Colors\n");
			get_quant_colors(win[WIN_VZ].i_data, win[WIN_VZ].img_r,
				win[WIN_VZ].img_c);
		}
		else get_2d_slice(ASPECT_Z, 0, ibuf, win[WIN_VZ].i_data[GRAY]);
		cmap_init();
		FreeAllColors();
		build_cmap();

		redisplay_all();

		save_image_undo(WIN_PAINT);
		save_image_orig(WIN_PAINT);

		free(img.hd.data);
		img.hd.data = NULL;

		sprintf(filename, "Image: %s (%dr x %dc x %df)",
			img.fname, segal.r, segal.c, segal.f);
		xv_set(View_win->msg_image,
			PANEL_LABEL_STRING, filename,
			NULL);

		xv_set(File_pop_load_image->pop_load_image,
			XV_SHOW, FALSE,
			NULL);

		xv_set(View_win->set_display,
			PANEL_INACTIVE, FALSE,
			NULL);

		xv_set(View_win->but_masks,
			PANEL_INACTIVE, FALSE,
			NULL);

		xv_set(View_win->but_preferences,
			PANEL_INACTIVE, FALSE,
			NULL);

		end_timer();

		notify_set_itimer_func(File_pop_load_image->pop_load_image,
			NOTIFY_FUNC_NULL, ITIMER_REAL, NULL, NULL);

		deq_bg_job();

		return NOTIFY_DONE;
	}
}

/*****************************************/
void
save_image_as()
{
	void save_image();

	char filename[MAXPATHLEN];

	strcpy(img.dname, (char *) xv_get(File_pop_save_image->text_i_save_dname,
		PANEL_VALUE, NULL));
	strcpy(img.fname, (char *) xv_get(File_pop_save_image->text_i_save_fname,
		PANEL_VALUE, NULL));
	sprintf(filename, "%s/%s", img.dname, img.fname);
	img.hd.name = str_save(filename);

	/* TRUE -> save image even if not saved */
	save_image(TRUE);

	xv_set(File_pop_save_image->pop_save_image,
		XV_SHOW, FALSE,
		NULL);
}

/*****************************************/
LOGIC
overwrite_image(notice_msg)
char *notice_msg;
{
	int but_x, but_y, result;

	but_x = (unsigned short) xv_get(View_win->but_image, XV_X, NULL)
		+ (unsigned short) xv_get(View_win->win, XV_X, NULL);
	but_y = (unsigned short) xv_get(View_win->but_image, XV_Y, NULL)
		+ (unsigned short) xv_get(View_win->win, XV_Y, NULL);
	result = notice_prompt(View_win->win, NULL,
		NOTICE_FOCUS_XY, but_x, but_y,
		NOTICE_MESSAGE_STRINGS, notice_msg,
			"Saving will overwrite the image.",
			NULL,
		NOTICE_BUTTON_YES, "Save image",
		NOTICE_BUTTON_NO, "Cancel",
		NULL);
	if(result == NOTICE_NO) return FALSE;
	else return TRUE;
}

/*****************************************/
void
save_image(save_explicit)
LOGIC save_explicit;
{
	LOGIC overwrite_image();
	Notify_value bg_save_image_frame();

	char foo[80];

	if(img.changed_image) {
		sprintf(foo, "The image %s has been altered but not saved.",
			img.fname);
		overwrite_image(foo);
	}
	else if(!save_explicit) return;

	vprint"Save image %s\n", img.fname);
	if((img.hd.OUT_FP = fopen(img.hd.name, "w")) == NULL) {
		prgmerr(0, "Can't open %s for saving!!", img.fname);
		return;
	}
	if((*img.hd.header_handle)(HEADER_WRITE, &img.hd,
		ac, av, img.changed_image) < 0) {
		prgmerr(0, "Unkown image type");
		return;
	}

	/*** Save all the frames ***/

	/* gonna do something time intensive ... background it */
	sprintf(timer.message, "Saving frames: %s ...", img.fname);
	if(!begin_timer()) enq_bg_job(JOB_SAVE_IMAGE, 0);
	else {
		segal.bg_i = 0;
		begin_itimer(INTERVAL_SEC, INTERVAL_uSEC);
		notify_set_itimer_func(File_pop_load_image->pop_load_image,
			bg_save_image_frame, ITIMER_REAL, &itimer, NULL);
	}
}

/*****************************************/
Notify_value
bg_save_image_frame()
{
	void set_timer();
	void deq_bg_job();
	void end_timer();

	if(segal.bg_i < segal.f) {
		(*img.hd.std_swif)(FI_SAVE_FILE, &img.hd, ibuf[segal.bg_i][0], 0);
		segal.bg_i++;

		set_timer((float) segal.bg_i / (float) segal.f);
	}
	else {
		img.changed_image = FALSE;

		end_timer();

		notify_set_itimer_func(File_pop_load_image->pop_load_image,
			NOTIFY_FUNC_NULL, ITIMER_REAL, NULL, NULL);

		deq_bg_job();

		return NOTIFY_DONE;
	}
}

/*****************************************/
void
load_list()
{
	void load_image_header();
	void load_image();
	void load_mask_proc();
	char *get_non_comment();

	char dname[MAXPATHLEN], fname[MAXPATHLEN], filename[MAXPATHLEN];
 
	strcpy(filename, (char *) xv_get(List_pop_list->text_l_fname,
		PANEL_VALUE, NULL));
	vprint"Loading from list %s\n", filename);

	/* First, the image path */
	strcpy(dname, (char *) xv_get(List_pop_list->text_i_dname,
		PANEL_VALUE, NULL));
	xv_set(File_pop_load_image->text_image_dname,
		PANEL_VALUE, dname,
		NULL);

	/* Second, the image */
	strcpy(fname, (char *) xv_get(List_pop_list->text_i_fname,
		PANEL_VALUE, NULL));
	xv_set(File_pop_load_image->text_image_fname,
		PANEL_VALUE, fname,
		NULL);
	load_image_header();
	load_image();

	/* Third, the masks path */
	if(strcmp(strcpy(dname, (char *) xv_get(List_pop_list->text_m_dname,
		PANEL_VALUE,NULL)), "<None>") != 0) {
		xv_set(File_pop_load_mask->text_mask_dname,
			PANEL_VALUE, dname,
			NULL);
	}

	/* Fourth, the masks */
	if(strcmp(strcpy(fname, (char *) xv_get(List_pop_list->text_m1_fname,
		PANEL_VALUE,NULL)), "<None>") != 0) {
		xv_set(File_pop_load_mask->text_mask_fname,
			PANEL_VALUE, fname,
			NULL);
		load_mask_proc();
	}
	if(strcmp(strcpy(fname, (char *) xv_get(List_pop_list->text_m2_fname,
		PANEL_VALUE,NULL)), "<None>") != 0) {
		xv_set(File_pop_load_mask->text_mask_fname,
			PANEL_VALUE, fname,
			NULL);
		load_mask_proc();
	}
	if(strcmp(strcpy(fname, (char *) xv_get(List_pop_list->text_m3_fname,
		PANEL_VALUE,NULL)), "<None>") != 0) {
		xv_set(File_pop_load_mask->text_mask_fname,
			PANEL_VALUE, fname,
			NULL);
		load_mask_proc();
	}
	if(strcmp(strcpy(fname, (char *) xv_get(List_pop_list->text_m4_fname,
		PANEL_VALUE,NULL)), "<None>") != 0) {
		xv_set(File_pop_load_mask->text_mask_fname,
			PANEL_VALUE, fname,
			NULL);
		load_mask_proc();
	}
	if(strcmp(strcpy(fname, (char *) xv_get(List_pop_list->text_m5_fname,
		PANEL_VALUE,NULL)), "<None>") != 0) {
		xv_set(File_pop_load_mask->text_mask_fname,
			PANEL_VALUE, fname,
			NULL);
		load_mask_proc();
	}

	/* close the load list window */
	xv_set(List_pop_list->pop_list,
		XV_SHOW, FALSE,
		NULL);
}
 
/*****************************************/
LOGIC
create_new_mask(notice_msg)
char *notice_msg;
{
	int but_x, but_y, result;

	but_x = (unsigned short) xv_get(View_win->but_image, XV_X, NULL)
		+ (unsigned short) xv_get(View_win->win, XV_X, NULL);
	but_y = (unsigned short) xv_get(View_win->but_image, XV_Y, NULL)
		+ (unsigned short) xv_get(View_win->win, XV_Y, NULL);
	result = notice_prompt(View_win->win, NULL,
		NOTICE_FOCUS_XY, but_x, but_y,
		NOTICE_MESSAGE_STRINGS, notice_msg,
			"Go ahead and create it?",
			NULL,
		NOTICE_BUTTON_YES, "Yes",
		NOTICE_BUTTON_NO, "No",
		NULL);
	if(result == NOTICE_NO) return FALSE;
	else return TRUE;
}

/*****************************************/
void
load_mask_proc()
{
	LOGIC create_new_mask();
	void new_mask_proc();
	LOGIC begin_timer();
	void enq_bg_job();
	void begin_itimer();
	Notify_value bg_load_mask_frame();

	char filename[MAXPATHLEN], foo[120];
	int new, i;

	/* check for max limit of masks */
	if(segal.num_m == MAX_MASKS) {
		vprint"Mask limit (%d) has been reached.  Please remove one first.\n", MAX_MASKS);
		return;
	}

	/* setup space for a new mask */
	segal.new_m  = new = segal.num_m;
	strcpy(m[new].dname, (char *) xv_get(File_pop_load_mask->text_mask_dname, PANEL_VALUE, NULL));
	strcpy(m[new].fname, (char *) xv_get(File_pop_load_mask->text_mask_fname, PANEL_VALUE, NULL));
	sprintf(filename, "%s/%s", m[new].dname, m[new].fname);
	m[new].hd.name = str_save(filename);

	/* no duplicate mask names allowed in log */
	for(i = 0; i < segal.num_m; i++)
		if(strcmp(m[i].fname, m[new].fname) == 0) {
			vprint"The mask %s is already in the Mask Log.\n", m[new].fname);
			return;
		}

	fprintf(stderr, "Loading mask: %s\n", filename);

	if((m[new].hd.IN_FP = fopen(filename, "rw")) == NULL) {
		/* No mask was found - create a new one? */
		sprintf(foo, "The mask %s was not found or couldn't be loaded.",
			m[new].fname);
		if(create_new_mask(foo)) {
			xv_set(File_pop_new_mask->text_new_dname,
				PANEL_VALUE, m[new].dname,
				NULL);
			xv_set(File_pop_new_mask->text_new_fname,
				PANEL_VALUE, m[new].fname,
				NULL);
			new_mask_proc();
		}
		return;
	}

	format_init(&m[new].hd, IMAGE_INIT_TYPE, HIPS, HIPS, *av, "Segal v. 3d");

	if((*m[new].hd.header_handle)(HEADER_READ, &m[new].hd, 0, 0, 0) < 0) {
		prgmerr(0, "Unkown image type");
		return;
	}

	(*m[new].hd.header_handle)(HEADER_TRANSF, &m[new].hd, 0);

	xv_set(File_pop_load_mask->pop_load_mask,
		XV_SHOW, FALSE,
		NULL);

	/*** Load all the frames ***/

	/* gonna do something time intensive ... background it */
	sprintf(timer.message, "Loading frames: %s ...", m[new].fname);
	if(!begin_timer()) enq_bg_job(JOB_LOAD_MASK, new);
	else {
		m[new].f = 0;
		begin_itimer(INTERVAL_SEC, INTERVAL_uSEC);
		notify_set_itimer_func(File_pop_load_mask->pop_load_mask,
			bg_load_mask_frame, ITIMER_REAL, &itimer, NULL);
	}

	segal.num_m++;
}

/*****************************************/
Notify_value
bg_load_mask_frame()
{
	void store_byte_to_bit();
	void set_timer();
	void deq_bg_job();
	void redisplay_all();
	void add_to_mask_log();

	/* recall ... new = segal.new_m at this point */

	if(m[segal.new_m].f < segal.f) {
		(*m[segal.new_m].hd.std_swif)(FI_LOAD_FILE, &m[segal.new_m].hd, 0, 0);

		/* pack the bytes to the appropriate bit in mbuf */
		store_byte_to_bit(m[segal.new_m].hd.data, mbuf[m[segal.new_m].f][0], segal.r * segal.c, segal.new_m);

		m[segal.new_m].f++;
		set_timer((float) m[segal.new_m].f / (float) segal.f);
	}
	else {
		free(m[segal.new_m].hd.data);
		m[segal.new_m].hd.data = NULL;
		fclose(m[segal.new_m].hd.IN_FP);

		m[segal.new_m].loaded = TRUE;
		if(segal.new_m == 0 && segal.disp_mask) {
			segal.e_m = 0;
			redisplay_all(); 
		}

		/* add mask filename to mask log */
		add_to_mask_log(segal.new_m);

		xv_set(File_pop_load_mask->pop_load_mask,
			XV_SHOW, FALSE,
			NULL);

		end_timer();

		notify_set_itimer_func(File_pop_load_mask->pop_load_mask,
			NOTIFY_FUNC_NULL, ITIMER_REAL, NULL, NULL);

		deq_bg_job();

		return NOTIFY_DONE;
	}
}

/*****************************************/
void
unload_mask(redisplay)
LOGIC redisplay;
{
	void save_mask();
	void copy_mask();
	void copy_mask_info();
	char *mask_list_string();
	void redisplay_all();

	LOGIC edit_mask_changed;
	int but_x, but_y, result;
	char foo[80];

	if(mlog.sel_m == UNDEFINED)
		prgmerr(0, "please select a mask to unload");
	vprint"Unload mask %s\n", m[mlog.sel_m].fname);

	/* if mask has not been saved after editing, pop up a warning */
	if(m[mlog.sel_m].changed_mask) {
		but_x = (unsigned short) xv_get(
			View_win->win, XV_X, NULL);
		but_y = (unsigned short) xv_get(
			View_win->win, XV_Y, NULL);
		sprintf(foo, "The mask %s has not been saved.",
			m[mlog.sel_m].fname);
		result = notice_prompt(View_win->win, NULL,
			NOTICE_FOCUS_XY, but_x, but_y,
			NOTICE_MESSAGE_STRINGS, foo, NULL,
			NOTICE_BUTTON_YES, "Save mask",
			NOTICE_BUTTON_NO, "Continue unloading",
			NULL);
		if(result == NOTICE_YES) save_mask();
	}

	strcpy(m[mlog.sel_m].fname, "<None>");

	if(mlog.sel_m == segal.e_m) edit_mask_changed = TRUE;
	else edit_mask_changed = FALSE;

	if(mlog.sel_m == segal.num_m - 1) {
		xv_set(Mask_log_pop_mask_log->ls_mask_filenames,
			PANEL_LIST_DELETE, segal.num_m - 1,
			NULL);
	}
	else {
		copy_mask(mbuf[0][0], mbuf[0][0], m[segal.num_m - 1].bit_key, m[mlog.sel_m].bit_key, segal.r * segal.c * segal.f);
		copy_mask_info(segal.num_m - 1, mlog.sel_m);
		xv_set(Mask_log_pop_mask_log->ls_mask_filenames,
			PANEL_LIST_STRING, mlog.sel_m,
				mask_list_string(mlog.sel_m),
			PANEL_LIST_DELETE, segal.num_m - 1,
			NULL);
	}
	segal.num_m--;	
	sprintf(foo, "Masks Loaded: %3d", segal.num_m);
	xv_set(Mask_log_pop_mask_log->msg_masks_loaded,
		PANEL_LABEL_STRING, foo,
		NULL);
	if(segal.num_m == 0
	|| edit_mask_changed) {
		segal.e_m = UNDEFINED;
		if(redisplay) redisplay_all();
	}
}

/*****************************************/
void
unload_all_masks(redisplay)
LOGIC redisplay;
{
	void unload_mask();

	while(segal.num_m != 0) {
		mlog.sel_m = segal.num_m - 1;
		unload_mask(redisplay);
	}
}

/*****************************************/
void
save_mask_as()
{
	char *mask_list_string();
	void save_mask();

	strcpy(m[mlog.sel_m].dname, (char *) xv_get(File_pop_save_as->text_save_dname, PANEL_VALUE, NULL));
	strcpy(m[mlog.sel_m].fname, (char *) xv_get(File_pop_save_as->text_save_fname, PANEL_VALUE, NULL));
	sprintf(m[mlog.sel_m].hd.name, "%s/%s", m[mlog.sel_m].dname, m[mlog.sel_m].fname);

	/* change the name in the list */
	xv_set(Mask_log_pop_mask_log->ls_mask_filenames,
		PANEL_LIST_STRING, mlog.sel_m, mask_list_string(mlog.sel_m),
		NULL);

	save_mask();

	xv_set(File_pop_save_as->pop_save_as,
		XV_SHOW, FALSE,
		NULL);
}

/*****************************************/
void
save_mask()
{
	Notify_value bg_save_mask_frame();

	extern	debug;

	vprint"Saving mask: %s\n", m[mlog.sel_m].hd.name);

	if((m[mlog.sel_m].hd.OUT_FP = fopen(m[mlog.sel_m].hd.name, "w"))
		== NULL) {
		prgmerr(0, "Can't open %s for saving!!", m[mlog.sel_m].fname);
		return;
	}

	if((*m[mlog.sel_m].hd.header_handle)(HEADER_WRITE, &m[mlog.sel_m].hd,
		ac, av, TRUE) < 0) {
		prgmerr(0, "Unkown image type");
		return;
	}

	/*** Save all the frames ***/

	/* gonna do something time intensive ... background it */
	sprintf(timer.message, "Saving frames: %s ...", m[mlog.sel_m].fname);
	if(!begin_timer()) enq_bg_job(JOB_SAVE_MASK, mlog.sel_m);
	else {
		segal.new_m = mlog.sel_m;
		m[mlog.sel_m].f = 0;
		begin_itimer(INTERVAL_SEC, INTERVAL_uSEC);
		notify_set_itimer_func(File_pop_load_mask->pop_load_mask,
			bg_save_mask_frame, ITIMER_REAL, &itimer, NULL);
	}
}

/*****************************************/
Notify_value
bg_save_mask_frame()
{
	void get_2d_slice();
	void set_timer();
	void end_timer();
	void deq_bg_job();

	int i;

	/* we use th view z mask buffer ... */

	if(m[segal.new_m].f < segal.f) {
		get_2d_slice(ASPECT_Z, m[segal.new_m].f, mbuf, &win[WIN_VZ].m_data[0]);
		for(i = 0; i < segal.r * segal.c; i++)
			if(BIT_IS_ON(win[WIN_VZ].m_data[0][i], m[segal.new_m].bit_key))
				win[WIN_VZ].m_data[0][i] = 255;
			else win[WIN_VZ].m_data[0][i] = FALSE;

		(*m[segal.new_m].hd.std_swif)(FI_SAVE_FILE, &m[segal.new_m].hd,
			win[WIN_VZ].m_data[0], 0);

		m[segal.new_m].f++;
		set_timer((float) m[segal.new_m].f / (float) segal.f);
	}
	else {
		fclose(m[segal.new_m].hd.OUT_FP);
		m[segal.new_m].changed_mask = FALSE;

		end_timer();

		notify_set_itimer_func(File_pop_load_mask->pop_load_mask,
			NOTIFY_FUNC_NULL, ITIMER_REAL, NULL, NULL);

		deq_bg_job();

		return NOTIFY_DONE;
	}
}

/**********************************************************/
void
new_mask_proc()
{
	void add_to_mask_log();

	int new, i;
	char filename[MAXPATHLEN];

	/* check for max limit of masks */
	if(segal.num_m == MAX_MASKS) {
		vprint"Mask limit (%d) has been reached.  Please remove one first.\n", MAX_MASKS);
		return;
	}

	/* setup space for a new mask */
	new = segal.num_m;
	strcpy(m[new].dname, (char *) xv_get(File_pop_new_mask->text_new_dname, PANEL_VALUE, NULL));
	strcpy(m[new].fname, (char *) xv_get(File_pop_new_mask->text_new_fname, PANEL_VALUE, NULL));
	sprintf(filename, "%s/%s", m[new].dname, m[new].fname);

	/* no duplicate mask names allowed in log */
	for(i = 0; i < segal.num_m; i++)
		if(strcmp(m[i].fname, m[new].fname) == 0) {
			vprint"The mask %s is already in the Mask Log.\n", m[new].fname);
			return;
		}

	/* setup hd info */
	m[new].hd.width = segal.c;
	m[new].hd.height = segal.r;
	m[new].hd.frames = segal.f;
	m[new].hd.o_form = IFMT_BYTE;
	format_init(&m[new].hd, IMAGE_INIT_TYPE, HIPS, -1, *av, "Segal v. 3d");
	(*m[new].hd.std_swif)(FI_INIT_NAME, &m[new].hd, filename, 0);

	m[new].hd.data = NULL;
	m[new].loaded = TRUE;

	if(new == 0) segal.e_m = 0; 

	segal.num_m++;

	/* add mask filename to mask log */
	add_to_mask_log(new);

	xv_set(File_pop_new_mask->pop_new_mask,
		XV_SHOW, FALSE,
		NULL);
}
 
/**********************************************************/
char
*get_non_comment(fp)
FILE *fp;
{
/* Really useful function for reading in configuration data ... */
/* If the line starts with 2 '**' then it is skipped */
	int comment = FALSE;
	char foo[MAXPATHLEN];
 
	while(fscanf(fp, "%s", foo) != EOF)
		if(strncmp(foo, "**", 2) != 0) {
		/* i.e. Doesn't start w/ '**' */
			if(!comment) return(foo);
		}
		else {
			if(comment == FALSE) comment = TRUE;
			else comment = FALSE;
		}
	return("**EOF");
}

/**********************************************/
LOGIC
turn_bit_on_with_log(p)
byte *p;
{
	LOGIC bit_is_excluded();

	if(mlog.apply_log
	&& bit_is_excluded(*p)) return(FALSE);

	TURN_BIT_ON(*p, m[segal.e_m].bit_key)
	return(TRUE);
}
 
/**********************************************/
LOGIC
turn_bit_off_with_log(p)
byte *p;
{
	LOGIC bit_is_included();
 
	if(mlog.apply_log
	&& bit_is_included(*p)) return(FALSE);

	TURN_BIT_OFF(*p, m[segal.e_m].bit_key)
	return(TRUE);
}

/*****************************************/
LOGIC
bit_is_included(p)
byte p;
{
	int i;

	For_all_loaded_masks
		if(m[i].mask_type == MASK_INCLUSIVE
		&& BIT_IS_ON(p, m[i].bit_key))
			return(TRUE);
	return(FALSE);
}

/*****************************************/
LOGIC
bit_is_excluded(p)
byte p;
{
	int i;

	For_all_loaded_masks
		if(m[i].mask_type == MASK_EXCLUSIVE
		&& BIT_IS_ON(p, m[i].bit_key))
			return(TRUE);
	return(FALSE);
}

/*****************************************/
void
store_byte_to_bit(byte_buf, bit_buf, size, mask_num)
byte *byte_buf, *bit_buf;
int size;
int mask_num;
{
/* Packs non-zero bytes to be TRUE bits from the byte buffer to the bit buffer.
 * Size is the length of the byte buffer.
 */
	int i;

	for(i = 0; i < size; i++)
		if(byte_buf[i]) {
			/* Turn on the appropriate bit */
			bit_buf[i] = bit_buf[i] | m[mask_num].bit_key;
		}
}

/**********************************************/
void
set_frame_slider(win_id)
int win_id;
{
	switch(win_id) {
	case WIN_PAINT :
		xv_set(Paint_win_paint->set_paint_frame,
			PANEL_VALUE, win[grow.swin].f,
			NULL);
		break;
	case WIN_VX :
		xv_set(View_win->set_frame_x,
			PANEL_VALUE, win[grow.swin].f,
			NULL);
		break;
	case WIN_VY :
		xv_set(View_win->set_frame_y,
			PANEL_VALUE, win[grow.swin].f,
			NULL);
		break;
	case WIN_VZ :
		xv_set(View_win->set_frame_z,
			PANEL_VALUE, win[grow.swin].f,
			NULL);
		break;
	}
}
 
/*****************************************/
void
allocate_buffers()
{
	u_char ***alloc_3d_byte_array();
	u_char **alloc_2d_byte_array();
	void free_3d_byte_array();
	void free_2d_byte_array();
	void build_ximages();

	int i, cp, largest;

	if(img.loaded) { /* don't do the first time through */
		fprintf(stderr, "Freeing buffers for reallocation ...\n");

		For_rgb if(cbuf[i] != NULL) free_3d_byte_array(cbuf[i]);

		if(ibuf != NULL) free_3d_byte_array(ibuf);
		free_3d_byte_array(mbuf);

		For_all_windows {
			free_3d_byte_array(win[i].i_data);
			free_2d_byte_array(win[i].m_data);
			free_2d_byte_array(win[i].z_data);
		}

		For_all_aspects for(cp = 0; cp < NUM_CPLANES; cp++) {
			free(img.undo[i][cp]);
			free(img.orig[i][cp]);
		}

		free(bm);

		For_rgb cbuf[i] = NULL;

		ibuf	= NULL;
		mbuf	= NULL;

		For_all_windows {
			win[i].i_data = NULL;
			win[i].m_data = NULL;
			win[i].z_data = NULL;
		}

		For_all_aspects for(cp = 0; cp < NUM_CPLANES; cp++) {
			img.undo[i][cp] = NULL;
			img.orig[i][cp] = NULL;
		}

		bm	= NULL;
	}

	if(segal.color) For_rgb
		cbuf[i] = alloc_3d_byte_array(segal.f, segal.r, segal.c);
	else ibuf	= alloc_3d_byte_array(segal.f, segal.r, segal.c);
	mbuf		= alloc_3d_byte_array(segal.f, segal.r, segal.c);

	For_all_windows {
		win[i].i_data = alloc_3d_byte_array(NUM_CPLANES,
			win[i].img_r, win[i].img_c);
		win[i].m_data = alloc_2d_byte_array(win[i].img_r, win[i].img_c);
		win[i].z_data = alloc_2d_byte_array(win[i].img_r, win[i].img_c);
	}

	For_all_aspects for(cp = 0; cp < NUM_CPLANES; cp++) {
		img.undo[i][cp] = Calloc(win[i].img_size, byte);
		img.orig[i][cp] = Calloc(win[i].img_size, byte);
		if(largest < win[i].img_size) largest = win[i].img_size;
	}

	bm = Calloc(largest, byte);

	/* get rid of now invalid XImages */
	For_all_windows if(win[i].ximg != NULL) {
		XDestroyImage(win[i].ximg);
		win[i].ximg = NULL;
	}
			
	/* create new XImages */
	build_ximages();
}

/***********************************************************/
void
realloc_window_buffers(win_id)
int win_id;
{
	u_char **alloc_2d_byte_array();
	u_char ***alloc_3d_byte_array();
	void free_2d_byte_array();
	void free_3d_byte_array();

	/* realloc buffers */
	if(win[win_id].i_data != NULL) {
		free_3d_byte_array(win[win_id].i_data);
		free_2d_byte_array(win[win_id].m_data);
		free_2d_byte_array(win[win_id].z_data);

		win[win_id].i_data = NULL;
		win[win_id].m_data = NULL;
		win[win_id].z_data = NULL;
	}

	win[win_id].i_data = alloc_3d_byte_array(NUM_CPLANES, win[win_id].img_r, win[win_id].img_c);
	win[win_id].m_data = alloc_2d_byte_array(win[win_id].img_r, win[win_id].img_c);
	win[win_id].z_data = alloc_2d_byte_array(win[win_id].img_r, win[win_id].img_c);
}

/***********************************************************/
void
realloc_window_ximage(win_id)
int win_id;
{
	byte *dbuf;

	/* realloc ximage */
	XDestroyImage(win[win_id].ximg);
	win[win_id].ximg = NULL;

	dbuf = Calloc(win[win_id].img_size * win[win_id].zoom_mag *
		win[win_id].zoom_mag, byte);
	win[win_id].ximg = XCreateImage(display, winv_info->visual, 8,
		ZPixmap, 0, (char *) dbuf, win[win_id].img_c * 
		win[win_id].zoom_mag, win[win_id].img_r * win[win_id].zoom_mag,
		8, 0);

	if(win[win_id].ximg == NULL) {
		fprintf(stderr, " Error creating image #%d!\n", win_id);
		return;
	}
	win[win_id].ximg->width = win[win_id].img_c * win[win_id].zoom_mag;	
	win[win_id].ximg->height = win[win_id].img_r * win[win_id].zoom_mag;	
}

/***********************************************************/
void
build_ximages()
{
/* Instantiates XImages ... call upon loading and upon changing the size of
 * the 3d data sets.
 */
	int i;

	byte *dbuf;

	vprint"creating X images ...\n");
	 
	For_all_windows if (win[i].ximg == NULL) {
		dbuf = Calloc(win[i].img_size, byte);
		win[i].ximg = XCreateImage(display, winv_info->visual, 8,
			ZPixmap, 0, (char *) dbuf, win[i].img_c, win[i].img_r, 
			8, 0);
		if (win[i].ximg == NULL) {
			fprintf(stderr, " Error creating image #%d!\n", i);
			return;
		}
		win[i].ximg->width = win[i].img_c;	
		win[i].ximg->height = win[i].img_r;	
	}
}

/***********************************************************/
void
map_buffers()
{
/* Maps the appropriate data from the buffers to be displayed in each of the
 * windows.  Factors in what is appropriate include whether or not data is
 * loaded and what options for display have been selected.  Also zooms the
 * data if the zoom_mag is > 1; otherwise zoom_to_ximage just copies data.
 */
	LOGIC okay_to_map();
	void get_2d_slice();
	void draw_crop_rectangle();
	void map_image_to_buf();
	void map_mask_to_buf();
	void blend_images_to_buf();
	void map_nothing_to_buf();
	void zoom_to_ximage();
 
	int i, j, paint_up;

	vprint"mapping buffers\n");

	set_watch_cursor();

	paint_up = xv_get(Paint_win_paint->win_paint, XV_SHOW, NULL);

	if(img.loaded && segal.disp_image) {
	if(segal.e_m != UNDEFINED && segal.disp_mask) {
	/*** both ***/
	For_all_windows if(win[i].repaint && okay_to_map(win[i].aspect)) {
		if(segal.color) for(j = 0; j < NUM_CPLANES; j++)
			get_2d_slice(win[i].aspect, win[i].f, cbuf[j], win[i].i_data[j]);
		else get_2d_slice(win[i].aspect, win[i].f, ibuf, win[i].i_data[GRAY]);
		get_2d_slice(win[i].aspect, win[i].f, mbuf, win[i].m_data);
		blend_images_to_buf(win[i].i_data,
			win[i].m_data,
			win[i].z_data,
			m[segal.e_m].bit_key,
			win[i].img_r,
			win[i].img_c,
			segal.disp_pts);
		zoom_to_ximage(i);
		XPutImage(display, win[i].xid, gc, win[i].ximg, 0, 0, 0, 0,
			win[i].ximg->width, win[i].ximg->height);
		if(crop.win_id == i) draw_crop_rectangle();
		win[i].repaint = FALSE;
	} /* For_all_windows */
	}
	else {
	/*** just image ***/
	For_all_windows if(win[i].repaint && okay_to_map(win[i].aspect)) {
		if(segal.color) for(j = 0; j < NUM_CPLANES; j++)
			get_2d_slice(win[i].aspect, win[i].f, cbuf[j], win[i].i_data[j]);
		else get_2d_slice(win[i].aspect, win[i].f, ibuf, win[i].i_data[GRAY]);
		map_image_to_buf(win[i].i_data,
			win[i].z_data,
			win[i].img_r,
			win[i].img_c);
		zoom_to_ximage(i);
		XPutImage(display, win[i].xid, gc, win[i].ximg, 0, 0, 0, 0,
			win[i].ximg->width, win[i].ximg->height);
		if(crop.win_id == i) draw_crop_rectangle();
		win[i].repaint = FALSE;
	} /* For_all_windows */
	}
	}
	else if(segal.e_m != UNDEFINED && segal.disp_mask) {
	/*** just mask ***/
	For_all_windows if(win[i].repaint && okay_to_map(win[i].aspect)) {
		get_2d_slice(win[i].aspect, win[i].f, mbuf, win[i].m_data);
		map_mask_to_buf(win[i].m_data[0],
			win[i].z_data[0],
			m[segal.e_m].bit_key,
			win[i].img_size,
			segal.disp_pts);
		zoom_to_ximage(i);
		XPutImage(display, win[i].xid, gc, win[i].ximg, 0, 0, 0, 0,
			win[i].ximg->width, win[i].ximg->height);
		if(crop.win_id == i) draw_crop_rectangle();
		win[i].repaint = FALSE;
	} /* For_all_windows */
	}
	else if(!segal.disp_image && !segal.disp_mask && img.loaded) {
	/*** display nothing ***/
	For_all_view {
		map_nothing_to_buf((u_char *) win[i].ximg->data, win[i].img_size);
		XPutImage(display, win[i].xid, gc, win[i].ximg, 0, 0, 0, 0, win[i].ximg->width, win[i].ximg->height);
		if(crop.win_id == i) draw_crop_rectangle();
	} /* i = 0 .. 2 */
	}

	if(img.loaded) {
		if(segal.mode == MODE_REGISTER) {
		}
	}

	unset_watch_cursor();
}

/*****************************************/
LOGIC
okay_to_map(aspect_id)
int aspect_id;
{
	if(!segal.r3d && (aspect_id == ASPECT_X || aspect_id == ASPECT_Y))
		return(FALSE);
	else return(TRUE);
}

/*****************************************/
void
get_2d_slice(aspect_id, frame, buf_3d, buf_2d)
int aspect_id, frame;
u_char ***buf_3d;
u_char **buf_2d;
{
	int z, y;

	vprint_if"getting 2d slice in:");
	switch(aspect_id) {
	case ASPECT_X :
		if(segal.r3d) {
			vprint_if" x-aspect\n");
			for(z = 0; z < segal.f; z++)
			for(y = 0; y < segal.r; y++)
				buf_2d[win[WIN_VX].img_r - 1 - z][y] = buf_3d[z][y][frame];
		}
		break;
	case ASPECT_Y :
		if(segal.r3d) {
			vprint_if" y-aspect\n");
			for(z = 0; z < segal.f; z++)
				bcopy(buf_3d[z][segal.r - 1 - frame], buf_2d[win[WIN_VY].img_r- 1 - z], win[WIN_VY].img_c);
		}
		break;
	case ASPECT_Z :
		vprint_if" z-aspect\n");
		bcopy(buf_3d[frame][0], buf_2d[0], win[WIN_VZ].img_size);
		break;
	default : break;
	}
}

/*****************************************/
void
put_2d_slice(aspect_id, frame, buf_3d, buf_2d)
int aspect_id, frame;
u_char ***buf_3d;
u_char **buf_2d;
{
	int z, y;

	if(segal.e_m == UNDEFINED) {
		vprint"No edit mask to write to!!\n");
		return;
	}

	vprint_if"putting 2d slice in:");
	switch(aspect_id) {
	case ASPECT_X :
		if(segal.r3d) {
			vprint_if" x-aspect\n");
			for(z = 0; z < segal.f; z++)
			for(y = 0; y < segal.r; y++)
				buf_3d[z][y][frame] = buf_2d[win[WIN_VX].img_r - 1 - z][y];
		}
		break;
	case ASPECT_Y :
		if(segal.r3d) {
			vprint_if" y-aspect\n");
			for(z = 0; z < segal.f; z++)
				bcopy(buf_2d[win[WIN_VY].img_r - 1 - z], buf_3d[z][segal.r - 1 - frame], win[WIN_VY].img_c);
		}
		break;
	case ASPECT_Z :
		vprint_if" z-aspect\n");
		bcopy(buf_2d[0], buf_3d[frame][0], (win[WIN_VZ].img_r - 1) * (win[WIN_VZ].img_c - 1));
		break;
	default : break;
	}
}

/*****************************************/
void
save_image_frame(win_id)
int win_id;
{
/* writes the paint image buffer if necessary to the 3d ibuf */
	void put_2d_slice();

	int i;

	if(img.changed_frame) {
		set_watch_cursor();

		if(segal.color) For_rgb
			put_2d_slice(win[win_id].aspect, win[win_id].f, cbuf[i],
				win[win_id].i_data[i]);
		else put_2d_slice(win[win_id].aspect, win[win_id].f, ibuf,
			win[win_id].i_data[GRAY]);

		img.changed_frame = FALSE;
		img.changed_image = TRUE;

		unset_watch_cursor();
	}
}

/*****************************************/
void
save_image_undo(win_id)
{
	int i;

	set_watch_cursor();

	if(segal.color) For_rgb bcopy(win[win_id].i_data[i][0],
		img.undo[win[win_id].aspect][i], win[win_id].img_size);
	else bcopy(win[win_id].i_data[GRAY][0],
		img.undo[win[win_id].aspect][GRAY], win[win_id].img_size);

	unset_watch_cursor();
}

/*****************************************/
void
save_image_orig(win_id)
{
	int i;

	set_watch_cursor();

	if(segal.color) For_rgb bcopy(win[win_id].i_data[i][0],
		img.orig[win[win_id].aspect][i], win[win_id].img_size);
	else bcopy(win[win_id].i_data[GRAY][0],
		img.orig[win[win_id].aspect][GRAY], win[win_id].img_size);

	unset_watch_cursor();
}

/*****************************************/
void
load_image_undo(win_id)
{
	void save_image_frame();
	void refresh_histogram();

	int i;

	set_watch_cursor();
	
	if(segal.color) For_rgb bcopy(img.undo[win[win_id].aspect][i],
		win[win_id].i_data[i][0], win[win_id].img_size);
	else bcopy(img.undo[win[win_id].aspect][GRAY],
		win[win_id].i_data[GRAY][0], win[win_id].img_size);

	img.changed_frame = TRUE;
	save_image_frame(win_id);

	if(xv_get(Threshold_pop_threshold->pop_threshold,
		XV_SHOW, NULL)
	&& (threshold.roi == R2d_WHOLE
	|| threshold.roi == R2d_CROP
	|| threshold.roi == R2d_PT_LIST)) refresh_histogram();

	unset_watch_cursor();
}

/*****************************************/
void
load_image_orig(win_id)
{
	void save_image_frame();
	void refresh_histogram();

	int i;

	set_watch_cursor();
	
	if(segal.color) For_rgb bcopy(img.orig[win[win_id].aspect][i],
		win[win_id].i_data[i][0], win[win_id].img_size);
	else bcopy(img.orig[win[win_id].aspect][GRAY],
		win[win_id].i_data[GRAY][0], win[win_id].img_size);

	img.changed_frame = TRUE;
	save_image_frame(win_id);

	if(xv_get(Threshold_pop_threshold->pop_threshold,
		XV_SHOW, NULL)
	&& (threshold.roi == R2d_WHOLE
	|| threshold.roi == R2d_CROP
	|| threshold.roi == R2d_PT_LIST)) refresh_histogram();

	unset_watch_cursor();
}

/*****************************************/
void
save_mask_frame(win_id)
int win_id;
{
/* writes the paint mask buffer if necessary to the 3d mbuf */
	void put_2d_slice();

	if(m[segal.e_m].changed_frame) {
		put_2d_slice(win[win_id].aspect, win[win_id].f, mbuf,
			win[win_id].m_data);
		m[segal.e_m].changed_frame = FALSE;
		m[segal.e_m].changed_mask = TRUE;
	}
}

/*****************************************/
void
save_mask_undo_2d(win_id)
int win_id;
{
	void copy_mask();
	 
	copy_mask(win[win_id].m_data[0], bm,
		m[segal.e_m].bit_key, bm_key[win[win_id].aspect],
		win[win_id].img_size);
}

/*****************************************/
void
load_mask_undo_2d(win_id)
int win_id;
{
	void copy_mask();
	void save_mask_frame();
	void redisplay_all();
	 
	copy_mask(bm, win[win_id].m_data[0],
		bm_key[win[win_id].aspect], m[segal.e_m].bit_key,
		win[win_id].img_size);
	m[segal.e_m].changed_frame = TRUE;
	save_mask_frame(win_id);
	redisplay_all();
}

/*****************************************/
void
save_mask_undo_3d(mnum)
int mnum;
{
/* Time intensive on large data sets ... use the timer */
	void copy_mask();
	LOGIC begin_timer();
	void set_timer();
	void end_timer();

	int f;

	sprintf(timer.message, "Saving 3d mask to undo buffer");
	if(!begin_timer())
		copy_mask(mbuf[0][0], mbuf[0][0], m[mnum].bit_key, m[BUF_UNDO].bit_key, segal.r * segal.c * segal.f);	
	else {
		for(f = 0; f < segal.f; f++) {
			copy_mask(mbuf[f][0], mbuf[f][0], m[mnum].bit_key, m[BUF_UNDO].bit_key, segal.r * segal.c);	
			set_timer((float) f / (float) segal.f);
		}
		end_timer();
	}
}

/*****************************************/
void
load_mask_undo_3d(mnum)
int mnum;
{
/* Time intensive on large data sets ... use the timer */
	void copy_mask();
	void redisplay_all();
	LOGIC begin_timer();
	void set_timer();
	void end_timer();

	int f;

	sprintf(timer.message, "Loading 3d mask from undo buffer");
	if(!begin_timer())
		copy_mask(mbuf[0][0], mbuf[0][0], m[BUF_UNDO].bit_key, m[mnum].bit_key, segal.r * segal.c * segal.f);	
	else {
		for(f = 0; f < segal.f; f++) {
			copy_mask(mbuf[f][0], mbuf[f][0], m[BUF_UNDO].bit_key, m[mnum].bit_key, segal.r * segal.c);	
			set_timer((float) f / (float) segal.f);
		}
		end_timer();
	}

	redisplay_all();
}

/*****************************************/
void
copy_mask_info(from_mnum, to_mnum)
int from_mnum, to_mnum;
{
	m[to_mnum].loaded = m[from_mnum].loaded;
	m[to_mnum].changed_mask = m[from_mnum].changed_mask;
	m[to_mnum].changed_frame = m[from_mnum].changed_frame;
	m[to_mnum].mask_type = m[from_mnum].mask_type;
	m[to_mnum].mask_hue = m[from_mnum].mask_hue;

	m[to_mnum].hd.name = str_save(m[from_mnum].hd.name);

	strcpy(m[to_mnum].dname, m[from_mnum].dname);
	strcpy(m[to_mnum].fname, m[from_mnum].fname);
}

/*****************************************/
void
copy_mask(from_buf, to_buf, from_key, to_key, size)
byte *from_buf, *to_buf, from_key, to_key;
int size;
{
	int i;

	set_watch_cursor();

	for(i = 0; i < size; i++)
		if(BIT_IS_ON(from_buf[i], from_key))
			TURN_BIT_ON(to_buf[i], to_key)
		else TURN_BIT_OFF(to_buf[i], to_key)

	unset_watch_cursor();
}

/*****************************************/
void
fill_mask(roi, win_id, mcolor)
int roi, win_id, mcolor;	
{
	void save_mask_frame();
	void redisplay_all();

	int i, size;
	byte *mdata;

	if(roi == R2d_WHOLE) {
		size = win[win_id].img_size;
		mdata = win[win_id].m_data[0];
	}
	else {
		size = segal.r * segal.c * segal.f;
		mdata = mbuf[0][0];
	}

	if(mcolor == BLACK)
		for(i = 0; i < size; i++)
			TURN_BIT_OFF(mdata[i], m[segal.e_m].bit_key)
	else if(mcolor == WHITE)
		for(i = 0; i < size; i++)
			TURN_BIT_ON(mdata[i], m[segal.e_m].bit_key)
	else if(mcolor == INVERT)
		for(i = 0; i < size; i++) {
			if(BIT_IS_ON(mdata[i], m[segal.e_m].bit_key))
				TURN_BIT_OFF(mdata[i], m[segal.e_m].bit_key)
			else TURN_BIT_ON(mdata[i], m[segal.e_m].bit_key)
		}

	if(roi == R2d_WHOLE) {
		m[segal.e_m].changed_frame = TRUE;
		save_mask_frame(win_id);
	}
	redisplay_all();
}

/*****************************************/
void
update_bytes_required()
{
	char foo[80];
	int col_multiplier;
	float megs;

	segal.r = segal.r2 - segal.r1 + 1;
	segal.c = segal.c2 - segal.c1 + 1;
	segal.f = segal.f2 - segal.f1 + 1;

/* size of the binary for "segal" */
#define seg_size 2.6

	if(segal.color) col_multiplier = 3 + 1; /* 3 planes + 1 mask */
	else col_multiplier = 1 + 1;

	megs = seg_size
		+ (float) ((segal.r * segal.c * segal.f) * col_multiplier)
		/ 1000000.;

#undef seg_size

	sprintf(foo, "Bytes required: %3.1fM", megs);
	xv_set(File_pop_load_image->msg_bytes_required,
		PANEL_LABEL_STRING, foo,
		NULL);

	if(segal.f > 1) {
	/* setup for 3d */
		segal.r3d = TRUE;
		segal.disp_xy = TRUE;
		xv_set(Paint_win_paint->set_aspect,
			PANEL_INACTIVE, FALSE,
			NULL);
	}
	else {
	/* setup for 2d */
		segal.r3d = FALSE;
		segal.disp_xy = FALSE;
		xv_set(Paint_win_paint->set_aspect,
			PANEL_INACTIVE, TRUE,
			NULL);
		win[WIN_PAINT].aspect = ASPECT_Z;
	}
}

/*****************************************/
void
init_info_from_header()
{
	int i;

	img.r = segal.r = img.hd.height;
	img.c = segal.c = img.hd.width;
	img.f = segal.f = img.hd.frames;
	img.frame_size = img.r * img.c;

	segal.r1 = 0;
	segal.r2 = img.r - 1;
	segal.c1 = 0;
	segal.c2 = img.c - 1;
	segal.f1 = 0;
	segal.f2 = img.f - 1;

	if(segal.f > 1) {
	/* setup for 3d */
		segal.r3d = TRUE;
		segal.disp_xy = TRUE;
		xv_set(Paint_win_paint->set_aspect,
			PANEL_INACTIVE, FALSE,
			NULL);
	}
	else {
	/* setup for 2d */
		segal.r3d = FALSE;
		segal.disp_xy = FALSE;
		xv_set(Paint_win_paint->set_aspect,
			PANEL_INACTIVE, TRUE,
			NULL);
		win[WIN_PAINT].aspect = ASPECT_Z;
	}

	/* should always be either 0 (gray) or 3 (sep color planes) */
	img.color = segal.color = isColorImage(img.hd.in_color);

	img.changed_image = FALSE;
 
	For_all_windows {
		win[i].zoom_mag = 1;
		win[i].f = 0;
	}
}

void
init_info_from_loading()
{
	int i;

	win[WIN_VX].img_r = segal.f;
	win[WIN_VX].img_c = segal.r;
 
	win[WIN_VY].img_r = segal.f;
	win[WIN_VY].img_c = segal.c;

	win[WIN_VZ].img_r = segal.r;
	win[WIN_VZ].img_c = segal.c;

	/* paint and ref_frame windows start out in the z-aspect */
	win[WIN_PAINT].aspect = ASPECT_Z;
	win[WIN_PAINT].img_r = segal.r;
	win[WIN_PAINT].img_c = segal.c;

	win[WIN_REF].aspect = ASPECT_Z;
	win[WIN_REF].img_r = segal.r;
	win[WIN_REF].img_c = segal.c;

	For_all_windows win[i].img_size = win[i].img_r * win[i].img_c;
}

