/*
 *	common.h - to be included with all SEGAL source files.
 *
 *	By Bryan Skene
 *
 */

/***** Include Files *****/
#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <sys/param.h>
#include <sys/types.h>
#include <varargs.h>

/* XView includes */
#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/textsw.h>
#include <xview/xv_xrect.h>
#include <xview/cms.h>
#include <xview/cursor.h>
#include <xview/scrollbar.h>
#include <xview/svrimage.h>
#include <xview/notice.h>
#include <xview/notify.h>
 
/* X Windows includes */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
  
/* HIPS include file */
#include "hipl_format.h"

#include "segal.h"
#define X_WINDOW_DEP
#include "function.h"

/***** Macros *****/
#define Calloc(x,y) (y *)calloc((unsigned)(x), sizeof(y))
#define Fread(a,b,c,d) fread((char *)(a), b, (int)(c), d)
#define Fwrite(a,b,c,d) fwrite((char *)(a), b, (int)(c), d)

/* MONO returns total intensity of r,g,b components */
#define MONO(rd,gn,bl) (((rd)*11 + (gn)*16 + (bl)*5) >> 5)  /*.33R+ .5G+ .17B*/

/* RANGE forces a to be in the range b..c (inclusive) */
#define RANGE(a,b,c) { if (a<b) a=b;  if (a>c) a=c; }

/* BIT_IS_ON determines whether a bit is set in a byte */
#define BIT_IS_ON(p,key) ((p) & (key))

/* TURN_BIT_ON sets a bit in a byte.  Remember not to use a ";" */
#define TURN_BIT_ON(p,key) {if(!BIT_IS_ON((p),(key))) p += (key);}

/* TURN_BIT_OFF zeros a bit in a byte.  Remember not to use a ";" */
#define TURN_BIT_OFF(p,key) {if(BIT_IS_ON((p),(key))) p -= (key);}

/* DISTANCE returns the distance between two 3-d points */
#define DISTANCE(x1,y1,z1,x2,y2,z2) sqrt((double)(((x2)-(x1))*((x2)-(x1))+((y2)-(y1))*((y2)-(y1))+((z2)-(z1))*((z2)-(z1))))

#define For_all_bits for(i = 0; i < NUM_OF_BITS; i++)
#define For_rgb for(i = 0; i < NUM_CPLANES; i++)
#define For_all_windows for(i = 0; i < NUM_WINS; i++)
#define For_all_aspects for(i = 0; i < NUM_ASPECTS; i++)
#define For_all_view for(i = 0; i <= WIN_VZ; i++)
#define For_all_view_handles for(i = 0; i <= NUM_VIEW_HANDLES; i++)
#define For_all_masks for(i = 0; i < MAX_MASKS; i++)
#define For_all_loaded_masks for(i = 0; i < segal.num_m; i++)
#define For_all_directions for(i = 0; i < NUM_DIRECTIONS; i++)
#define For_all_bridge_strengths for(i = 0; i < NUM_BRIDGE_DIRS; i++)

#define vprint fprintf(stderr,
#define vprint_if if(verbose)fprintf(stderr,

/***** Constants *****/
#define UNDEFINED	-1
#define UNDEFINED_BYTE	255
#define LOGIC unsigned char
#define FALSE	0
#define TRUE	1

/* Running things in the background */
#define INTERVAL_SEC    0
#define INTERVAL_uSEC   30000   /* 30 microseconds */

/* job types */
#define MAX_JOBS	20	/* Max # bg jobs allowed in queue at once */
#define JOB_QUIT	-1
#define JOB_LOAD_IMAGE	0
#define JOB_LOAD_MASK	1
#define JOB_SAVE_IMAGE	2
#define JOB_SAVE_MASK	3

/* semaphore flags */
#define UNLOCKED	0
#define LOCKED		1

/* mask colors */
#define PALSIZE		10
#define RED		0	/* stack[0] will be red */
#define BROWN		1
#define ORANGE		2
#define YELLOW		3
#define GREEN		4
#define TURQUOISE	5
#define BLUE		6
#define PURPLE		7	/* stack[7] will be purple */
#define CBLACK		8	/* actual color black */
#define CWHITE		9	/* actual color white */

/* segal internal mask colors/operations */
#define BLACK	0
#define WHITE	1
#define INVERT	2

/* crop stuff */
#define CROP_RESTORE	0
#define CROP_FILL	1

/* maximums */
#define NUM_OF_BITS		8
#define MAX_MASKS		7	/* 7 masks + 1 region of interest */
#define MAX_ZOOM_MAG		6
#define NUM_BRUSH_SIZES		7
#define MAX_BRUSH_SIZE		20
#define NUM_GRAY		64
#define NUM_COLORS		100	/* 2*NUM_GRAY + 100 = 228 ... */
#define DELTA			(float) (NUM_GRAY - 1) / (256.)
#define NUM_PTS_PER_ALLOC	(segal.r * segal.c / 4)
#define NUM_BRIDGE_DIRS		8
#define NUM_VIEW_HANDLES	4

/* quantizing */
#define Q_BEST		0
#define Q_MEDIUM	1
#define Q_WORST		2

/* color image buffers */
#define NUM_CPLANES	3	/* RGB */
#define GRAY		0	/* grayscale image */
#define RP		0
#define GP		1
#define BP		2
#define VAL_RGB		3	/* MONO(r,g,b) */

/* windows */
#define NUM_WINS	5
#define WIN_VX		0
#define WIN_VY		1
#define WIN_VZ		2
#define WIN_PAINT	3
#define WIN_REF		4

/* aspects */
#define NUM_ASPECTS	3
#define ASPECT_X	0
#define ASPECT_Y	1
#define ASPECT_Z	2

/* misc. buffer types */
#define BUF_PTS		5
#define BUF_UNDO	6

/* bit mask key indices */
#define MASK_UNDO_X	0
#define MASK_UNDO_Y	1
#define MASK_UNDO_Z	2
#define MASK_CLIP_X	3
#define MASK_CLIP_Y	4
#define MASK_CLIP_Z	5
#define MASK_UNDO_3d	6
#define MASK_ROI_3d	7

/* mask types */
#define MASK_EDIT	0
#define MASK_NO_APPLY	1
#define MASK_INCLUSIVE	2
#define MASK_EXCLUSIVE	3

/* mask colors */
#define MASK_RED	0
#define MASK_GREEN	1
#define MASK_BLUE	2
#define MASK_YELLOW	3

/* 3d directions */
#define NUM_DIRECTIONS	6
#define DIR_N		0
#define DIR_S		1
#define DIR_E		2
#define DIR_W		3
#define DIR_U		4
#define DIR_D		5

/* growth directions */
#define DIR_GROW	0
#define DIR_SHRINK	1

/* growth extents */
#define GROW_FRAME	0
#define GROW_BEG_TO_END	1
#define GROW_ALL	2

/* seed points source */
#define SEED_EDIT	0
#define SEED_PTS	1
#define SEED_ROI	2

/* roi's */
#define R2d		0
#define R3d		1
#define R2d_WHOLE	2
#define R2d_CROP	3
#define R2d_PT_LIST	4
#define R3d_WHOLE	5
#define R3d_CUBE	6
#define R3d_PT_LIST	7

/* order of apply */
#define ORDER_EI	0
#define ORDER_IE	1

/* display options */
#define DISP_IMAGE	0
#define DISP_MASK	1
#define DISP_ORIG	2
#define DISP_XY		3

/* growth display options */
#define DISP_WHEN_DONE	0
#define DISP_GROW	1
#define DISP_ALL	2

/* modes */
#define MODE_SEGMENT	0
#define MODE_REGISTER	1

/* brush modes */
#define BRUSH_IMAGE	0	/* Painting the Image with an airbrush */
#define BRUSH_MASK	1	/* Painting the mask */
#define BRUSH_PTS	2	/* Painting the point list */

/* brush shapes */
#define BRUSH_SQUARE	0
#define BRUSH_ROUND	1

/* mask brush affects */
#define MASK_ERASE	0
#define MASK_PAINT	1

/* threshold effects */
#define THRESH_OVERWRITE	0
#define THRESH_ADD_TO		1
#define THRESH_REMOVE_FROM	2

/***** Structures *****/
typedef struct {
	int	x, y, z;	/* current x, y, z */
	int	r1, c1, f1, r2, c2, f2; /* used to determine r, c, f */
	int	r, c, f;	/* rows, columns, frames of what is loaded */
	LOGIC	color;		/* load image as a color or gray image? */
	LOGIC	r3d;		/* 3d = TRUE, 2d = FALSE */
	int	roi;		/* region of interest */
	LOGIC	disp_image;	/* display image? */
	LOGIC	disp_mask;	/* display mask? */
	LOGIC	disp_pts;	/* display points? */
	LOGIC	disp_xy;	/* display X & Y views? */
	int	mode;		/* segmentation or registration */
	int	bg_i;		/* background job index */
	int	num_m;		/* number of masks loaded (7 max) */
	int	e_m;		/* current edit mask */
	int	new_m;		/* new mask */
	}	SEGAL_INFO;

typedef struct {
	int	sel_m;		/* selected mask in the log */
	LOGIC	apply_log;	/* Auto apply log when painting? */
	int	apply_order;	/* exclusive-inclusive, or vice-versa? */
	}	MLOG_INFO;

typedef struct {
	int	job;		/* load, save, quit, etc. */
	int	arg;		/* argument for this job */
	}	QUEUE_EL;

typedef struct {
	int	semaphore;	/* locked/unlocked: 1 bg job at a time */
	int	width, height;	/* timer window */
	XID	xid;		/* parent window of bg job */
	char	message[MAXPATHLEN];
				/* job description */
	int	qfront, qrear;	/* for the scheduling queue */
	QUEUE_EL queue[MAX_JOBS];
				/* scheduling queue */
	}	TIMER_INFO;

typedef struct {
	LOGIC	loaded;		/* an image was successfuly loaded */
	Image	hd;		/* structure for using Jin's ccl */
	int	r, c, f;	/* of stored image file */
	int	frame_size;	/* of stored image file */
	char	dname[MAXPATHLEN];
	char	fname[MAXPATHLEN];
	FILE	*fp;		/* fp for the image */
	LOGIC	color;		/* color image? */
	LOGIC	changed_image;
	LOGIC	changed_frame;
	byte	*undo[NUM_ASPECTS][NUM_CPLANES];
	byte	*orig[NUM_ASPECTS][NUM_CPLANES];
	}	IMAGE_INFO;

typedef struct {
	LOGIC	loaded;		/* mask was successfuly loaded */
	Image	hd;
	int	f;
	char    dname[MAXPATHLEN];
	char    fname[MAXPATHLEN];
	LOGIC   changed_mask;
	LOGIC   changed_frame;
	int	mask_type;	/* edit, inclusive, exclusize, etc. */
	int	mask_hue;	/* green, blue, red, etc. */
	int	bit_key;	/* which mask bit */
	}	MASK_INFO;

typedef struct {
	int	zoom_mag;	/* magnification to zoom the window */
	int	aspect;		/* variable for paint and reference windows */
	int	f;		/* current frame */
	int	img_r, img_c, img_size;
	int	canv_x, canv_y, canv_w, canv_h;
	LOGIC	repaint;
	XID	xid;
	Scrollbar h_sbar, v_sbar; /* horizontal & vertical scrollbars */
	byte	***i_data;	/* color planes X rows X cols of one slice */
	byte	**m_data;	/* one slice of the bit masks */
	byte	**z_data;	/* zoom area containing replicated xcolors */
	XImage	*ximg;
	}	WIN_INFO;

typedef struct {
	int	x, y;
	}	HANDLE_INFO;

typedef struct {
	int	win_id;
	int	x1, y1, f1, x2, y2, f2;
	HANDLE_INFO handle[NUM_ASPECTS][NUM_VIEW_HANDLES];
	}	CROP_INFO;

typedef struct {
	unsigned short  x, y, f;
	}       POINT_TYPE;

typedef struct {
	byte	r, g, b;	/* color components */
	}	COLOR_TYPE;

typedef struct {
	byte	r, g, b;	/* color components */
	u_int	count;		/* count for histogram */
	byte	col_i;		/* index into the allocated color[] */
	}	RGB_HISTO;

typedef struct {
	int	quality;
	int	win;
	int	num_from_histo;
	int	auto_r, auto_g, auto_b;
	int	max_samples;	/* the more the better quality but slower */
	int	min_rgb_dist;	/* keeps alloc'd colors apart in rgb-space */
	}	QUANT_INFO;

typedef struct {
	QUANT_INFO quant;
	int	num_histo;
	RGB_HISTO *histo;
	int	map_r, map_g, map_b;
	byte	***map;
	}	RGB_INFO;

typedef struct {
	/* x, y, f, etc. are in the Z aspect */
	int	x1, y1, f1;	/* left upper below corner */
	int	x2, y2, f2;	/* right lower above corner */
	int	beg_frame;
	int	end_frame;
	}	REGION_INFO;

typedef struct {
	int	min, max;
	int	roi;
	int	plane;
	int	mask_effect;
	int	image_effect;
	int	degree;
	XID	xid;
	}	THRESH_INFO;

typedef struct {
	int	offset;
	int	current;
	int	upper;
	int	lower;
	}	FRAME_INFO;

typedef struct {
	int	mode;
	int	image_affect;
	int	degree;
	int	mask_affect;
	int	shape;
	int	size;
	}	BRUSH_INFO;

typedef struct {
	int	swin;
	int	direction;
	LOGIC	apply_thresholds;
	int	threshold_min;
	int	threshold_max;
	LOGIC	apply_gradient;
	int	gradient_rad;
	int	gradient_min;
	int	gradient_max;
	LOGIC	apply_bridge;
	int	bridge_dist;
	int	bridge_min;
	int	bridge_max;
	int	extent;
	int	seed_pt_src;
	LOGIC	overwrite;
	int	disp_growth;
	LOGIC	interractive;
	struct itimerval itimer;
	int	speed;		/* number of points per interval */
	LOGIC	stack_empty;
	}	GROW_INFO;

typedef struct {
	int num_pts;
	int num_pts_per_alloc;
	int max_pts;
	POINT_TYPE *pts;
	}	STACK_INFO;

typedef struct curs {
	Xv_Cursor paint;	/* round cursors */
	Xv_Cursor erase;
	Xv_Cursor sq_paint;	/* square cursors */
	Xv_Cursor sq_erase;
	u_char **paint_mask_matrix; /* gives shape to the round cursors */
	int size;
	int radius;
	int corner;
	}	SEGAL_CURSOR;


/***** User Interface Objects *****/
extern file_pop_load_image_objects     *File_pop_load_image;
extern file_pop_load_mask_objects      *File_pop_load_mask;
extern file_pop_new_mask_objects       *File_pop_new_mask;
extern file_pop_save_as_objects        *File_pop_save_as;
extern file_pop_save_image_objects     *File_pop_save_image;
extern filter_pop_filter_objects       *Filter_pop_filter;
extern image_reg_pop_image_reg_objects *Image_reg_pop_image_reg;
extern image_reg_pop_ref_frame_objects *Image_reg_pop_ref_frame;
extern list_pop_list_objects           *List_pop_list;
extern mask_grow_pop_mask_grow_objects *Mask_grow_pop_mask_grow;
extern mask_grow_pop_options_objects   *Mask_grow_pop_options;
extern mask_log_pop_mask_log_objects   *Mask_log_pop_mask_log;
extern mask_log_pop_options_objects    *Mask_log_pop_options;
extern paint_win_paint_objects         *Paint_win_paint;
extern preferences_pop_preferences_display_objects
					*Preferences_pop_preferences_display;
extern threshold_pop_threshold_objects *Threshold_pop_threshold;
extern view_pop_timer_objects	       *View_pop_timer;
extern view_win_objects                *View_win;


/***** Global Variables *****/
extern BRUSH_INFO	brush;
extern CROP_INFO	crop;
extern FRAME_INFO	frame;
extern GROW_INFO	grow;
extern IMAGE_INFO	img;
extern MASK_INFO	m[MAX_MASKS + 1];
extern MLOG_INFO	mlog;
extern REGION_INFO	region;
extern RGB_INFO		rgb;
extern SEGAL_CURSOR	my_cursor[NUM_BRUSH_SIZES][MAX_ZOOM_MAG];
extern SEGAL_INFO	segal;
extern THRESH_INFO	threshold;
extern TIMER_INFO	timer;
extern WIN_INFO		win[NUM_WINS];

/* Running things in the background */
extern struct itimerval itimer;

/* HIPS header stuff */
extern int ac;
extern char **av;

/* command line args */
extern char	*in_image, *in_mask, *in_list;
extern int	overlay_hue;

/* colors */
extern int num_rgb;		/* number of colors in rgb_histo */

/* 3d buffers */
extern u_char	***cbuf[NUM_CPLANES]; /* space for a 3d color image */
extern u_char	***ibuf;	/* space for a 3d gray image */
extern u_char	***mbuf;	/* space for 7 bit-masks and a 3d region */

/* 1d buffers */
extern u_char	*bm;		/* bit masks ... */
extern byte	bm_key[NUM_OF_BITS];

/***** X Windows *****/
/* window stuff shared by all parts - DEFINED in segal.c */
extern Display *display;
extern u_long *colors;
extern GC gc;
extern int screen;
extern Xv_cmsdata cms_data;
extern Visual *winv;
extern XVisualInfo *winv_info;
extern Window mainw;

extern u_char *image_map;
extern int im_size;

/* cmap.c */
extern u_char gray_lut[256];	/* just image */
extern u_char blend_lut[256];	/* mask-image blend */
extern u_char pt_lut[256];	/* point list */
 
/* cursor used everywhere */
extern Cursor watch_cursor;

/* preferences */
extern int	verbose;

/***** Global Routines *****/
void set_watch_cursor();
void unset_watch_cursor();
