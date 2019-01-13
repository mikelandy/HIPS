/*  common.h : for use with the program 'segal'  by Brian Tierney, LBL */

/***********************************************************************/
/*  COPYRIGHT NOTICE         *******************************************/
/***********************************************************************/

/*   This program is copyright (C) 1990, Regents  of  the
University  of  California.   Anyone may reproduce this software,
in whole or in part, provided that:
(1)  Any copy  or  redistribution  must  show  the
     Regents  of  the  University of California, through its
     Lawrence Berkeley Laboratory, as the source,  and  must
     include this notice;
(2)  Any use of this software must reference this  distribu-
     tion,  state that the software copyright is held by the
     Regents of the University of California, and  that  the
     software is used by their permission.

     It is acknowledged that the U.S. Government has  rights
to this software under  Contract DE-AC03-765F00098 between the U.S.
Department of Energy and the University of California.

     This software is provided as a professional  academic
contribution for  joint exchange.  Thus it is experimental, is
provided ``as is'', with no warranties of any kind  whatsoever,
no  support,  promise  of updates, or printed documentation.
Bug reports or fixes may be sent to the author, who may or may
not act on them as he desires.
*/

/*   Authors: Brian L. Tierney &
 *            Bryan Skene
 *            Lawrence Berkeley Laboratory
 *            Imaging and Distributed Computing Group
 *            email: bltierney@lbl.gov
 *		     skene@lbl.gov
*/
/**********************************************************************/
/* NOTES:
 *
 * The program is broken into several modules, usually one per window.  Along
 * with each module and window there is associated a "#include __.h" file that
 * externs the window objects of that module and #includes the "_ui.h" for the
 * module.  In this way, only the ".c" files that specifically require access
 * to a window's objects gets recompiled when that window is altered (in GUIDE,
 * most likely).
 */

#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <sys/param.h>
#include <sys/types.h>

/* xview/x windows include files */
#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/textsw.h>
#include <xview/xv_xrect.h>
#include <xview/cms.h>
#include <xview/cursor.h>
#include <xview/scrollbar.h>
#include <xview/svrimage.h>
#include <xview/notice.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>

/* HIPS include file */
#include "hipl_format.h"

#define REGARDLESS 1		/* useful for a flag */
#define UNDEF -1

#define VIEW_WIN 0		/* for generic window procs */
#define REF_FRAME 1

#define NGRAY 64		/* program scales all data in image to 0 to
				 * NGRAY and then allocates another NGRAY
				 * for blend colors.  (never set above 120)
				 */
#define RED_HUE 0		/* hue of ramp used for mask */
#define GREEN_HUE 1
#define BLUE_HUE 2

#define NUM_STANDOUTS	4
#define RED_STANDOUT	NGRAY * 2	/* Then the next four slots in the */
#define GREEN_STANDOUT	NGRAY * 2 + 1	/* colormap are used for the Standout */
#define BLUE_STANDOUT	NGRAY * 2 + 2	/* Colors */
#define YELLOW_STANDOUT	NGRAY * 2 + 3	

#define CMAP_SIZE NGRAY * 2 + NUM_STANDOUTS 
				/* Size of the required color map */

#define PVAL  255		/* high pixel value  */

#define ZOOM_WIN_SIZE 325       /* size of zoom window (fixed at compile) */
#define MAX_ZOOM_WIDTH 500	/* max size of zoom window (bigger -> gets */
#define MAX_ZOOM_HEIGHT 500	/* scroll bars. */

#define MAX_VIEW_WIDTH 800	/* max size of view window (bigger -> gets */
#define MAX_VIEW_HEIGHT 800	/* scroll bars. */

#define EDIT_CONTROL_HEIGHT 132 /* size of edit control panel */
#define EDIT_CONTROL_WIDTH  480 

#define MAX_MASKS 50		/* should be >>> than enough! */
#define MAX_FRAMES 1024		/* max number of frames in image & masks */
#define MAX_STACK_RADIUS 4	/* largest gradient matrix radius */
				/* gives 8+1 = 9 for 9x9 matrix */
				/* REMEMBER: Keep matrix dims odd */
#define MAX_STACK_HEIGHT 2 * MAX_STACK_RADIUS + 1

#define UP 0			/* directions */
#define DOWN 1
#define RIGHT 2
#define LEFT 3

#define FRAME_BLANK 0		/* The internal frame statuses */
#define FRAME_ALTERED 1
#define FRAME_SAVED 2
#define FRAME_LOCKED 3

#define FRAME_UNREGISTERED 0	/* The internal image frame stuses */
#define FRAME_REGISTERED 1

#define MASK_NO_APPLY 0		/* The internal mask types */
#define MASK_EDIT 1		/* NOTE: exactly 1 mask must be the edit mask */
#define MASK_EXCLUSIVE 2
#define MASK_INCLUSIVE 3

/* Macros ... */
#define Calloc(x,y) (y *)calloc((unsigned)(x), sizeof(y))
#define Fread(a,b,c,d) fread((char *)(a), b, (int)(c), d)
#define Fwrite(a,b,c,d) fwrite((char *)(a), b, (int)(c), d)

/* MONO returns total intensity of r,g,b components */
#define MONO(rd,gn,bl) (((rd)*11 + (gn)*16 + (bl)*5) >> 5)  /*.33R+ .5G+ .17B*/

/* RANGE forces a to be in the range b..c (inclusive) */
#define RANGE(a,b,c) { if (a<b) a=b;  if (a>c) a=c; }

/* useful structures */
typedef struct zi {
	int	x;		/* upper, left corner of zoom image */
	int	y;
	int	width;
	int	height;
	int	mag;		/* zoom magnification setting */
	} ZOOM_INFO;

	int	frame_reg;	/* 0=unregistered, 1=registered */

typedef struct i_frame {		/* IMAGE_FRAME_INFO */
	int	frame_reg;	/* 0=unregistered, 1=registered */
		} IMAGE_FRAME_INFO;

typedef struct img {		/* IMAGE_INFO */
	struct header	hd;	/* HIPS header */
	char	path[MAXPATHLEN];
	char	filename[MAXPATHLEN];
	FILE	*fp;		/* FILE ptr for the image */
	char	frame_filename[MAXPATHLEN];
	FILE	*frame_fp;	/* FILE ptr for image's frame info file */
	IMAGE_FRAME_INFO frames[MAX_FRAMES]; /* see above */
	int	ref_frame;	/* frame used to register the image */
	long	start;		/* position of first byte in file (after
				 * header) */
	int	changed;	/* image got changed in pixedit window? */
	u_char	**data;		/* stored [y][x] */
		} IMAGE_INFO;

typedef struct m_frame {		/* MASK_FRAME_INFO */
	int	frame_status;	/* 0=blank, 1=altered, 2=saved, 3=locked */
	/* Below: for growing from other masks; not implemented as of yet. */
	int	grow_t_low;	/* frame's grow_thresh_low value */
	int	grow_t_high;	/* frame's grow_thresh_high value */
	int	gradient;	/* frame's grow_gradient value */
		} MASK_FRAME_INFO;

typedef struct mask {		/* MASK_INFO */
	struct header	hd;	/* HIPS header */
	char	path[MAXPATHLEN];
	char	filename[MAXPATHLEN];
	FILE	*fp;		/* FILE ptr for the mask */
	char	frame_filename[MAXPATHLEN];
	FILE	*frame_fp;	/* FILE ptr for mask's frame status file */
	MASK_FRAME_INFO frames[MAX_FRAMES]; /* see above */
	int	mask_type;	/* if this is a mask, what type is it?
 				 * (EDIT, EXCLUSIVE, INCLUSIVE, NOT APPLIED) */	
	int	mask_color;	/* Green, Red, Yellow, or Blue */
	long	start;		/* position of first byte in file (after
				 * header) */
	u_char	**data;		/* stored [y][x] */
		} MASK_INFO;

typedef struct info {
	int	rows;		/* rows in image */
	int	cols;		/* columns in image */
	int	bit_map_len;	/* r*c/8 + 1 */
	int	frames;		/* frames in image file (0-9 ==> 10 frames) */
	int	curr_frame;	/* numbered from 0, not 1 */
	int	width;		/* what actual width is being displayed */
	int	height;		/* what actual height is being displayed */
	int	changed;	/* did the edit mask get changed */
	/* SETTINGS: */
	int	image_loaded;	/* is an image loaded? */
	int	masks;		/* number of masks loaded */
	int	edit_m;		/* of the loaded masks, which is EDIT MASK? */
	int	poly_flag;	/* polygon mask mode */
	int	reg_flag;	/* image registry mode */
	int	display_type;	/* image, mask, both */
	int	mask_type;	/* cut, build (defaults to build) */
	int	blend_type;	/* col. blend, overlay (default = col. blend) */
	int	slider1;	/* image opacity slider value */
	int	slider2;	/* mask opacity slider value */
	}         SEGAL_INFO;

typedef struct ed_info {
	int	win_width;
	int	win_height;
	int	canvas_width;
	int	canvas_height;
	int	changing;	/* flag used by resize proc */
	}	EDIT_INFO;

typedef struct thresh_info {
	int	upper;		/* value of upper threshold slider */
	int	lower;		/* value of lower threshold slider */
	}	THRESHOLD_INFO;

typedef struct regis_info {
	double	scale_factor;	/* scaling factor */
	int	trans_pixels;	/* value of translate pixels slider */
	double	rot_degrees;	/* degrees for rotation */
	}	REGIS_INFO;

typedef struct line_reg_info {
	int	refresh;	/* 0=no, 1=yes */
	int	x1, y1;		/* a line is defined by 2 points */
	int	x2, y2;
	}	LINE_INFO;

typedef struct reg_info {
	/* FLAGS */
	int	region;		/* apply to the cropped region of the mask */
	int	whole;		/* apply to whole mask */
	int	beg_frame;	/* for 3-d growing routines */
	int	end_frame;	/* for 3-d growing routines */
	int	refresh;	/* flag set to 1 when image or mask loaded */
	int	rows;		/* of the region */
	int	cols;		/* of the region */
	int	crx1;		/* 'crop' x1,y1 = upper left corner of region */
	int	cry1;
	int	crx2;		/* x2,y2 = lower rt corner of cropped region */
	int	cry2;
	}	REGION_INFO;

typedef struct gr_info {
	int	matrix_width;	/* width of gradient mask */
				/* (2*matrix_width + 1 = W for a WxW matrix) */
	int	matrix_type;	/* simple gradient, sobel, laplacian, etc. */
	int	apply_in;	/* the 2-d frame or the stack of frames (3-d) */
	int	extent_3D;	/* Into frame or Into and then grow_current() */
	int	gradient;	/* slider... don't grow if gradient > . */
	int	threshold_high;	/* slider... don't grow if p(x,y) > . */
	int	threshold_low;	/* slider... don't grow if p(x,y) < . */
	int	direction;	/* not implemented - for growing in or out */
	int	distance;	/* not implemented - further restrict growth */
	}	GROW_INFO;

typedef struct frm_info {	/* keeps track of stuff for the auto growing */
				/* in 3-d routines which use itimers */
	int	current;	/* this frame */
	int	offset;		/* -1 = grow to previous, 1 = grow to next */
	int	upper;		/* where to stop growing */
	int	lower;		/* where to start growing from */
	int	stack_rad;/* set to number of frames below and above loaded into
			   * stack (for gradient matrix application in growing.
			   */
	}	FRAME_INFO; /* for use with timers */

typedef struct s_2d {		/* BP's Segmentation 2D */
	int	alpha;		/* used in surface fitting */
	double	k;		/* used in surface fitting */
	int	iterations;	/* used in surface fitting */
	}	SEGMENTATION_2D_INFO;

/********************************************/

/* all of these are defined in segal.c */
extern IMAGE_INFO himage;
extern MASK_INFO m[MAX_MASKS];
extern ZOOM_INFO zoom_info;
extern SEGAL_INFO segal;
extern EDIT_INFO edit_info;
extern THRESHOLD_INFO threshold;
extern REGIS_INFO registry;
extern LINE_INFO ref_line, image_line;
extern REGION_INFO region;
extern GROW_INFO grow;
extern FRAME_INFO frame;
extern SEGMENTATION_2D_INFO seg_2d;

extern int ac;			/* argc and argv, for use in update header */
extern char **av;

/* window stuff shared by all parts - DEFINED in segal.c */
extern Display *display;
extern u_long *colors;
extern u_long red_standout, green_standout, blue_standout, yellow_standout;
extern GC gc;
extern int screen;
extern Xv_cmsdata cms_data;

/* XImages used everywhere */
extern XImage *image;
extern XImage *orig_image;
extern XImage *mask_image;
extern XImage *blend_image;
extern XImage *ref_frame_image;

extern XImage *zoom_image;
extern XImage *zoom_mask_image;
extern XImage *zoom_blend_image;

/* Buffers */
extern u_char **image_buf;		/* used in editting */
extern u_char **image_undo_buf;		/* used to store for undo*/
extern u_char **work_buf;		/* used in editting */
extern u_char **work_undo_buf;		/* used to store for undo*/
extern u_char **ref_image_frame_buf;	/* used in image registry */
extern u_char **ref_mask_frame_buf;	/* used in image registry */
extern u_char **mask_buf;		/* used in growing */
extern u_char **stack_buf[MAX_STACK_HEIGHT]; /* used in growing */

/* Cursor stuff */
#define MAX_CURSORS 50
typedef struct curs {
    Xv_Cursor paint;     /* round cursors */
    Xv_Cursor erase;
    Xv_Cursor sq_paint;  /* square cursors */
    Xv_Cursor sq_erase;
    u_char **paint_mask_matrix;	/* gives shape to the round cursors */
    u_char **paint_image_matrix;/* not implemented */
    int size;
    int radius;
    int corner;
} SEGAL_CURSOR;

/* from blend.c */
extern u_char gray_lut[256];
extern u_char blend_lut[256];

/* in cursor.c */
extern SEGAL_CURSOR my_cursor[MAX_CURSORS];

/* cursor used everywhere */
extern Cursor watch_cursor;

/* xid's needed everywhere */
extern XID       view_xid, orig_view_xid, edit_xid, edit_control_xid;
extern XID	segal_control_xid, pop_load_xid, pop_save_xid;
extern XID	display_ctrl_xid, image_reg_xid, ref_frame_xid;
extern XID	segmentation_xid, frame_ctrl_xid, mask_ctrl_xid;
extern XID	region_control_xid, threshold_xid, image_frame_status_xid;
extern XID	mask_frame_status_xid;

/* global flag for indicating whether to print status/debug messages */
int verbose;
