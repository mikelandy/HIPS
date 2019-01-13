
/*
 * @(#)xv_frame.h	2.5	11/29/91
 *
 * XView preview for HIPS images V1.0
 * xv_frame.h : This file contains the declarations of UI routines used
 *		extnerally.
 *
 *
 * Author: 	Cathy Waite
 * 		(c) The Turing Institute
 *
 * Fri Aug  9 17:06:10 BST 1991
 *
 */
#include <hipl_format.h>

#include <xview/xview.h>
#include <xview/openmenu.h>
#include <xview/panel.h>

/* Used as "stored_length" for text items, and the length of all
 * character arrays
 */
						/* Used in 		*/
extern  Display		*display;		/* appl.c, 
						   colors.c,
						   xv_canvas.c
									*/
extern  int		screen;			/* colors.c
						   adjust.c
									*/
extern	Window		frame_xwin,		/* colors.c		*/
			panel_xwin;		/* xv_canvas.c		*/
extern	Visual		*visual;		/* appl.c
						   colors.c		*/
extern  h_boolean		valid_image;		/* appl.c		*/
/* RF: 9 */
extern  h_boolean		with_control_panel;	/* xv_canvas.c		*/
extern	Frame		frame;			/* app.c
						   xv_adjust.c
						   xv_animation.c
						   xv_canvas.c
						   xv_file.c
						   xv_header.c		*/
extern	Menu_item	animation_menu_item;	/* xv_animation.c	*/
extern	GC		image_gc;		/* appl.c
						   xv_canvas.c		*/
extern	Panel		control_panel;		/* appl.c
						   colors.c
						   xv_canvas.c		*/
#define TEXT_LENGTH     256

extern	char		filename[TEXT_LENGTH];	/* appl.c
						   xv_adjust.c
						   xv_animation.c
						   xv_file.c
						   xv_header.c		*/
extern  char		info[TEXT_LENGTH];	/* appl.c
						   xv_canvas.c		*/

extern 	void		show_control_panel();	/* xv_canvas.c		*/
extern	void		enable_revert();	/* appl.c
						   colors.c		*/
extern  void		disable_revert();	/* appl.c		*/
extern  void		report_error();		/* appl.c		*/
extern  void		print_message();	/* appl.c		
						   canvas.c		*/
extern  void		update_frame();		/* appl.c
						   xv_adjust.c		*/
extern	void		enable_options();	/* appl.c		*/
extern  void		disable_options();	/* appl.c		*/
