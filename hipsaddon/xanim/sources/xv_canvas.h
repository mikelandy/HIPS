
/*
 * @(#)xv_canvas.h	2.2	11/29/91
 *
 * XView preview for HIPS images V1.0
 * xv_canvas.h : This file contains the declarations of UI routines used
 *		 extnerally.
 *
 *
 * Author: 	Cathy Waite
 * 		(c) The Turing Institute
 *
 * Fri Aug  9 17:06:10 BST 1991
 *
 */

					/* Used in ....		*/
extern	Canvas	canvas;			/* xv_frame.c		*/
extern	Window	canvas_xwin;		/* appl.c		*/
extern	int	print_mode;		/* appl.c		*/

extern	void	interactive_extract();	/* xv_frame.c		*/
extern	void	cancel_extract();	/* xv_frame.c		*/
extern	void	create_canvas();	/* xv_frame.c		*/
extern  void	print_new_pixel_value();/* xv_frame.c		*/

