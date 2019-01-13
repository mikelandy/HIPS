
/*
 * @(#)xv_header.h	2.2	11/29/91
 *
 * XView preview for HIPS images V1.0
 * xv_header.h : This file contains the declarations of UI routines used
 *		extnerally.
 *
 *
 * Author: 	Cathy Waite
 * 		(c) The Turing Institute
 *
 * Fri Aug  9 17:06:10 BST 1991
 *
 */

#include <xview/textsw.h>
extern	Frame		header_command_frame;		/* xv_frame.c	*/
extern	Window		header_xwin;			/* colors.c	*/
extern  Textsw		header_text;
extern  void		create_header_command_frame();	/* xv_frame.c	*/
extern	void		update_hdr();			/* appl.c	*/	
