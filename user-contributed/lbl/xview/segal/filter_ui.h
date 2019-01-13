#ifndef	filter_HEADER
#define	filter_HEADER

/*
 * filter_ui.h - User interface object and function declarations.
 * This file was generated by `gxv' from `filter.G'.
 * DO NOT EDIT BY HAND.
 */

extern Attr_attribute	INSTANCE;


typedef struct {
	Xv_opaque	win;
	Xv_opaque	controls;
	Xv_opaque	filter_name;
	Xv_opaque	filter_parameters;
	Xv_opaque	set_source_buf;
	Xv_opaque	set_dest_buf;
	Xv_opaque	but_apply;
	Xv_opaque	but_close;
} filter_win_objects;

extern filter_win_objects	*filter_win_objects_initialize();

extern Xv_opaque	filter_win_win_create();
extern Xv_opaque	filter_win_controls_create();
extern Xv_opaque	filter_win_filter_name_create();
extern Xv_opaque	filter_win_filter_parameters_create();
extern Xv_opaque	filter_win_set_source_buf_create();
extern Xv_opaque	filter_win_set_dest_buf_create();
extern Xv_opaque	filter_win_but_apply_create();
extern Xv_opaque	filter_win_but_close_create();
#endif
