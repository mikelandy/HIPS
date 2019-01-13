#ifndef	surf_fit_HEADER
#define	surf_fit_HEADER

/*
 * surf_fit_ui.h - User interface object and function declarations.
 * This file was generated by `gxv' from `surf_fit.G'.
 * DO NOT EDIT BY HAND.
 */

extern Attr_attribute	INSTANCE;


typedef struct {
	Xv_opaque	win;
	Xv_opaque	controls1;
	Xv_opaque	set_alpha;
	Xv_opaque	set_k;
	Xv_opaque	set_iterations;
	Xv_opaque	but_do_fitting;
	Xv_opaque	but_close;
} surf_fit_win_objects;

extern surf_fit_win_objects	*surf_fit_win_objects_initialize();

extern Xv_opaque	surf_fit_win_win_create();
extern Xv_opaque	surf_fit_win_controls1_create();
extern Xv_opaque	surf_fit_win_set_alpha_create();
extern Xv_opaque	surf_fit_win_set_k_create();
extern Xv_opaque	surf_fit_win_set_iterations_create();
extern Xv_opaque	surf_fit_win_but_do_fitting_create();
extern Xv_opaque	surf_fit_win_but_close_create();
#endif
