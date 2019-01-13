
/*
 * @(#)xv_file.h	2.2	11/29/91
 *
 * XView preview for HIPS images V1.0
 * xv_file.h : This file contains the declarations of UI routines used
 *		extnerally.
 *
 *
 * Author: 	Cathy Waite
 * 		(c) The Turing Institute
 *
 * Fri Aug  9 17:06:10 BST 1991
 *
 */

							/* used in:	*/
extern	Frame		load_command_frame;		/* xv_frame.c	*/
extern	Frame		save_command_frame;		/* xv_frame.c	*/
extern	Panel_item	load_file_item;			/* xv_frame.c	*/
extern	Panel_item	save_file_item;			/* xv_frame.c	*/
extern	h_boolean		save_frame;			/* xv_frame.c	*/

extern void		create_load_commad_frame();	/* xv_frame.c	*/
extern void		create_save_commad_frame();	/* xv_frame.c	*/
