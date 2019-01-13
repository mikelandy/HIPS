
/*
 * @(#)appl.h	2.4	11/29/91
 *
 * XView preview for HIPS images V1.0
 * appl.h : 	This file contains the declarations of those application
 *		routines used externally.
 *
 *
 * Author: 	Cathy Waite
 * 		(c) The Turing Institute
 *
 * Fri Aug  9 17:06:10 BST 1991
 *
 */

#include <X11/Xlib.h>
#include <X11/Xutil.h>

typedef	byte *frame_ptr;

					/* Used in ... 			*/
extern 	int		no_frames, 	/* colors.c			*/
			image_size, 	/* colors.c			*/
			image_width, 	/* xv_canvas.c, xv_frame.c	*/
			image_height;	/* xv_canvas.c, xv_frame.c  	*/
extern  frame_ptr 	*orig_frames, 	/* colors.c			*/
			*resized_frames;/* colors.c			*/
extern  int		current_frame;	/* xv_animation.c		*/
extern	h_boolean		slow_resize;	/* xv_adjust.c			*/
extern	int		global_argc;	/* xv_frame.c			*/
extern	char		**global_argv;	/* xv_frame.c			*/
/* BF 2. */
extern  h_boolean		saved_stdout;	/* xv_frame.c			*/

extern	h_boolean		select_frames;	/* xv_frame.c			*/
extern  int		first_frame;	/* xv_frame.c			*/
extern  int		last_frame;	/* xv_frame.c			*/
/* Routines defined in appl.c and used externally */
extern	h_boolean		load_file();	/* xv_frame.c, xv_file.c	*/
extern  h_boolean		save_file_as(); /* xv_file.c			*/
extern  h_boolean		save_file();	/* xv_frame.c, xv_file.c	*/
extern  void		resize_image(); /* xv_canvas.c			*/
extern  void		revert_image(); /* xv_frame.c			*/
extern  void		extract();	/* xv_canvas.c			*/
extern  void		render_image(); /* colors.c, xv_animation.c
					  xv_canvas.c			*/

