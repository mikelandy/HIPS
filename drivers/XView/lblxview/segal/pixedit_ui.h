#ifndef	pixedit_HEADER
#define	pixedit_HEADER

/*
 * pixedit_ui.h - User interface object and function declarations.
 * This file was generated by `gxv' from `pixedit.G'.
 * DO NOT EDIT BY HAND.
 */

extern Attr_attribute	INSTANCE;


typedef struct {
	Xv_opaque	win;
	Xv_opaque	control;
	Xv_opaque	brush_mode;
	Xv_opaque	brush_type;
	Xv_opaque	mask_brush_mode;
	Xv_opaque	brush_size;
	Xv_opaque	zoom_mag;
	Xv_opaque	image_brush_mode;
	Xv_opaque	image_brush_delta;
	Xv_opaque	but_original;
	Xv_opaque	but_undo;
	Xv_opaque	but_save;
	Xv_opaque	but_close;
	Xv_opaque	msg_pixel_value;
	Xv_opaque	canvas;
} pixedit_win_objects;

extern pixedit_win_objects	*pixedit_win_objects_initialize();

extern Xv_opaque	pixedit_win_win_create();
extern Xv_opaque	pixedit_win_control_create();
extern Xv_opaque	pixedit_win_brush_mode_create();
extern Xv_opaque	pixedit_win_brush_type_create();
extern Xv_opaque	pixedit_win_mask_brush_mode_create();
extern Xv_opaque	pixedit_win_brush_size_create();
extern Xv_opaque	pixedit_win_zoom_mag_create();
extern Xv_opaque	pixedit_win_image_brush_mode_create();
extern Xv_opaque	pixedit_win_image_brush_delta_create();
extern Xv_opaque	pixedit_win_but_original_create();
extern Xv_opaque	pixedit_win_but_undo_create();
extern Xv_opaque	pixedit_win_but_save_create();
extern Xv_opaque	pixedit_win_but_close_create();
extern Xv_opaque	pixedit_win_msg_pixel_value_create();
extern Xv_opaque	pixedit_win_canvas_create();
#endif
