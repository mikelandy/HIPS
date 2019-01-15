/*	GETX . H
#
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

This software is copyright (C) by the Lawrence Berkeley National Laboratory.
Permission is granted to reproduce this software for non-commercial
purposes provided that this notice is left intact.

It is acknowledged that the U.S. Government has rights to this software
under Contract DE-AC03-76SF00098 between the U.S.  Department of Energy
and the University of California.

This software is provided as a professional and academic contribution
for joint exchange. Thus, it is experimental, and is provided ``as is'',
with no warranties of any kind whatsoever, no support, no promise of
updates, or printed documentation. By using this software, you
acknowledge that the Lawrence Berkeley Laboratory and Regents of the
University of California shall have no liability with respect to the
infringement of other copyrights by any part of this software.

For further information about this notice, contact William Johnston,
Bld. 50B, Rm. 2239, Lawrence Berkeley National Laboratory, Berkeley, CA, 94720.
(wejohnston@lbl.gov)

For further information about this software, contact:
	Jin Guojun
	Bld. 50B, Rm. 2275
	Lawrence Berkeley National Laboratory, Berkeley, CA, 94720.
	g_jin@lbl.gov

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% AUTHOR:	Jin Guojun - LBL	4/1/91
*/

#if	!defined PANEL_H & defined X_WINDOW_DEP
#	include <stdio.h>
#	include <math.h>
#	include <ctype.h>
#	include <X11/X.h>
#	include <X11/Xlib.h>
#	include <X11/Xutil.h>
#	include <X11/cursorfont.h>
#endif
#include "rle.h"
#include "imagedef.h"

#define COUNT_OF(_array_)	(sizeof(_array_) / sizeof(_array_[0]))
#define IMAGE_BORDERWIDTH	3

#ifdef USE_STDLIB_H
#include <stdlib.h>
#else

#ifdef USE_STRINGS_H
#include <strings.h>
#endif
extern char	*getenv();
extern void	*malloc(), *realloc(), free();
#endif	USE_STDLIB_H

typedef longword	Pixel;
typedef void	VOID_FUNCTION();
typedef int	array16[16];

#define MALLOC_FAILURE	3
#define FILE_FAILURE	2
#define FATAL_FAILURE	1
#define SUCCESS		0

#define VPRINTF		if (verbose)	fprintf
#define DPRINTF		if (DEBUGANY)	fprintf
#define	DEBUGMESG(s)	if (DEBUGANY)	mesg(s)

#define SHIFT_MASK_PIXEL(r, g, b)	\
	( (((r) << red_shift) & red_mask)	\
	| ( ((g) << green_shift) & green_mask )	\
	| ( ((b) << blue_shift) & blue_mask ) )

#define SHIFT_MASK_PIXEL_32(r, g, b)	\
	( ((r) << red_shift) | ((g) << green_shift) | ((b) << blue_shift) )

typedef	int	x_bool;

#if	defined	HIPS2_HF || defined HIPS_IMAGE
extern	char	*Progname;
#define	progname Progname
#else
extern	char*	progname;
#endif

extern double	display_gamma;
extern bool	jump_flag, load_frame, multi_frame, tuner_flag,
		verbose;	/*	-v	*/
extern	int 	iflag, screen, stingy_flag, specified_levels;
extern	Image	*rw_set_id;

/*	X11/NeWS server bug workaround	*/
extern int	no_color_ref_counts;

/*	Color map, gamma correction map, & lookup tables	*/

extern int	red_shift, green_shift, blue_shift;
extern Pixel	red_mask, green_mask, blue_mask, pixel_base;

/*	pointer arithmetic. Returns Y'th row in our saved data array.	*/

#ifndef	SAVED_RLE_ROW

#ifndef	GETX_OFFSET
#define	GETX_OFFSET	0
#endif
#define SAVED_RLE_ROW(img, y)	\
	(((Image*)img)->scan_data +	\
	(((y) + GETX_OFFSET) * ((Image*)img)->width * (img)->dpy_channels))
#define ORIG_RLE_ROW(img, y)	\
	(((Image*)img)->data +	\
	(((y) + GETX_OFFSET) * ((Image*)img)->width * (img)->dpy_channels))
#endif

#define duff8(counter, block)	unroll8_bwd(, counter, block)

#define	UnmapPixWindow(img)	XUnmapWindow (img->dpy, img->pix_info_window)

#define DESIRED_ICON_WIDTH	128
#define DESIRED_ICON_HEIGHT	96

#ifndef	USE_ROOT_WIN
#define	USE_ROOT_WIN	-2
#endif
