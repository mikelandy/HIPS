#ifndef	orig_view_HEADER
#define	orig_view_HEADER

/*
 * orig_view_ui.h - User interface object and function declarations.
 * This file was generated by `gxv' from `orig_view.G'.
 * DO NOT EDIT BY HAND.
 */

extern Attr_attribute	INSTANCE;


typedef struct {
	Xv_opaque	win;
	Xv_opaque	canvas;
} orig_view_win_objects;

extern orig_view_win_objects	*orig_view_win_objects_initialize();

extern Xv_opaque	orig_view_win_win_create();
extern Xv_opaque	orig_view_win_canvas_create();
#endif