#ifndef	display_HEADER
#define	display_HEADER

/*
 * display_ui.h - User interface object and function declarations.
 * This file was generated by `gxv' from `display.G'.
 * DO NOT EDIT BY HAND.
 */

extern Attr_attribute	INSTANCE;


typedef struct {
	Xv_opaque	ctrlwin;
	Xv_opaque	controls1;
	Xv_opaque	message2;
	Xv_opaque	message1;
	Xv_opaque	gamma;
	Xv_opaque	cmap_min;
	Xv_opaque	cmap_max;
	Xv_opaque	shrink_fac;
} display_ctrlwin_objects;

extern display_ctrlwin_objects	*display_ctrlwin_objects_initialize();

extern Xv_opaque	display_ctrlwin_ctrlwin_create();
extern Xv_opaque	display_ctrlwin_controls1_create();
extern Xv_opaque	display_ctrlwin_message2_create();
extern Xv_opaque	display_ctrlwin_message1_create();
extern Xv_opaque	display_ctrlwin_gamma_create();
extern Xv_opaque	display_ctrlwin_cmap_min_create();
extern Xv_opaque	display_ctrlwin_cmap_max_create();
extern Xv_opaque	display_ctrlwin_shrink_fac_create();
#endif
