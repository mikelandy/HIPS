
/*
 * @(#)colors.h	2.4	11/29/91
 *
 * XView preview for HIPS images V1.0
 * colors.h:: 	This file contains the declarations of those color
 *		structures and routines used externally.
 *
 *
 * Author: 	Cathy Waite
 * 		(c) The Turing Institute
 *
 * 		Tue Aug 27 14:40:25 BST 1991
 *
 */
#include <xview/cms.h>

extern	int		no_colors;			/* xv_adjust.c	*/
extern	int		std_brightness,			/* xv_adjust.c	*/
			brightness; 			/* xv_adjust.c	*/
extern  float		std_contrast,  			/* xv_adjust.c	*/
			contrast; 			/* xv_adjust.c	*/
/* RF: 1 ... changed over to XView color segments */
extern	Cms		cms;				/* xv_canvas.c	*/

extern  h_boolean		do_substitute;			/* xv_frame.c
							   appl.c	*/
/* RF: 4 */
extern 	h_boolean		retain_panel;			/* xv_frame.c	*/
extern	h_boolean		ignore_panel;			/* xv_frame.c	*/
extern  Colormap	color_map;			/* xv_frame.c	*/
extern  void		create_default_color_map(); 	/* xv_frame.c	*/
extern  void		update_color_map();		/* xv_adjust.c	*/
extern  void		revert_colors();		/* appl.c	*/
extern  void		set_contrast_brightness();	/* appl.c	*/
extern  void		set_cb_single_frame();		/* appl.c	*/
extern  int		get_pixel_value();		/* xv_canvas.c	*/
