
/*
 * @(#)xv_adjust.h	2.2	11/29/91
 *
 * XView preview for HIPS images V1.0
 * xv_adjust.h : This file contains the declarations of UI routines used
 *		extnerally.
 *
 *
 * Author: 	Cathy Waite
 * 		(c) The Turing Institute
 *
 * Fri Aug  9 17:06:10 BST 1991
 *
 */

							/* Used in:	*/
extern	Window		adjust_xwin;			/* colors.c	*/
extern	Frame		adjust_command_frame;		/* xv_frame.c   */


extern void		set_bright_contr();		/* colors.c 	*/
extern void		set_width_height();		/* xv_frame.c	*/
extern void		create_adjust_command_frame();  /* xv_frame.c	*/
extern void		update_adjust();		/* appl.c	*/
