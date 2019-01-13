
/*
 * @(#)xv_animation.h	2.2	11/29/91
 *
 * XView preview for HIPS images V1.0
 * xv_animation.h : This file contains the declarations of UI routines used
 *		extnerally.
 *
 *
 * Author: 	Cathy Waite
 * 		(c) The Turing Institute
 *
 * Fri Aug  9 17:06:10 BST 1991
 *
 */
							/* Used in 	*/
extern	Frame	animation_command_frame;		/* xv_frame.c	*/
extern	Window	animation_xwin;				/* colors.c	*/
extern  h_boolean	running;				/* xv_frame.c	*/


extern	void	stop_notify();				/* xv_file.c	*/
extern 	void	interrupt_timer();			/* xv_frame.c	*/
							/* xv_canvas.c	*/
extern 	void	restart_timer();			/* xv_frame.c	*/
							/* xv_canvas.c  */
extern 	void	create_animation_command_frame();	/* xv_frame.c	*/
extern 	void	update_animation();			/* appl.c	*/
