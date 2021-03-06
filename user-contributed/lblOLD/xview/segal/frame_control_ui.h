#ifndef	frame_control_HEADER
#define	frame_control_HEADER

/*
 * frame_control_ui.h - User interface object and function declarations.
 * This file was generated by `gxv' from `frame_control.G'.
 * DO NOT EDIT BY HAND.
 */

extern Attr_attribute	INSTANCE;


typedef struct {
	Xv_opaque	win;
	Xv_opaque	controls_fill1;
	Xv_opaque	msg_image;
	Xv_opaque	canv_image_frame_status;
	Xv_opaque	controls_fill2;
	Xv_opaque	msg_mask;
	Xv_opaque	canv_mask_frame_status;
	Xv_opaque	controls;
	Xv_opaque	curr_frame;
	Xv_opaque	msg_frame_status;
	Xv_opaque	set_image_frame_status;
	Xv_opaque	text_ref_frame;
	Xv_opaque	set_mask_frame_status;
	Xv_opaque	movie_go_button;
	Xv_opaque	but_movie_stop;
	Xv_opaque	movie_beg_frame;
	Xv_opaque	movie_end_frame;
	Xv_opaque	set_stack_load;
	Xv_opaque	but_close;
} frame_control_win_objects;

extern frame_control_win_objects	*frame_control_win_objects_initialize();

extern Xv_opaque	frame_control_win_win_create();
extern Xv_opaque	frame_control_win_controls_fill1_create();
extern Xv_opaque	frame_control_win_msg_image_create();
extern Xv_opaque	frame_control_win_canv_image_frame_status_create();
extern Xv_opaque	frame_control_win_controls_fill2_create();
extern Xv_opaque	frame_control_win_msg_mask_create();
extern Xv_opaque	frame_control_win_canv_mask_frame_status_create();
extern Xv_opaque	frame_control_win_controls_create();
extern Xv_opaque	frame_control_win_curr_frame_create();
extern Xv_opaque	frame_control_win_msg_frame_status_create();
extern Xv_opaque	frame_control_win_set_image_frame_status_create();
extern Xv_opaque	frame_control_win_text_ref_frame_create();
extern Xv_opaque	frame_control_win_set_mask_frame_status_create();
extern Xv_opaque	frame_control_win_movie_go_button_create();
extern Xv_opaque	frame_control_win_but_movie_stop_create();
extern Xv_opaque	frame_control_win_movie_beg_frame_create();
extern Xv_opaque	frame_control_win_movie_end_frame_create();
extern Xv_opaque	frame_control_win_set_stack_load_create();
extern Xv_opaque	frame_control_win_but_close_create();
#endif
