/*
 *	storage.c - for use with SEGAL.
 *
 *	By Bryan Skene
 *
 */

#include "common.h"

/***** Global Variables *****/
BRUSH_INFO	brush;
CROP_INFO	crop;
FRAME_INFO	frame;
GROW_INFO	grow;
IMAGE_INFO	img;
MASK_INFO	m[MAX_MASKS + 1];
MLOG_INFO	mlog;
REGION_INFO	region;
RGB_INFO	rgb;
SEGAL_CURSOR	my_cursor[NUM_BRUSH_SIZES][MAX_ZOOM_MAG];
SEGAL_INFO	segal;
THRESH_INFO	threshold;
TIMER_INFO	timer;
WIN_INFO	win[NUM_WINS];

/* Running things in the background */
struct itimerval itimer;

/* HIPS header stuff */
int ac;
char **av;
 
/* command line args */
char     *in_image, *in_mask, *in_list;
int      overlay_hue;

/* colors */
int	num_rgb;	/* number of colors in rgb_histo */

/* 3d buffers */
u_char   ***cbuf[NUM_CPLANES]; /* space for a 3d color image */
u_char   ***ibuf;        /* space for a 3d image */
u_char   ***mbuf;        /* space for 7 bit-masks and a 3d region */
 
/* 1d buffers */
u_char   *bm;           /* bit masks ... */
byte	 bm_key[NUM_OF_BITS];	 /* bit mask keys */

/***** X Windows *****/
/* window stuff shared by all parts - DEFINED in segal.c */
Display *display;
u_long *colors;
GC gc;
int screen;
Xv_cmsdata cms_data;
Visual   *winv;
XVisualInfo *winv_info;
Window    mainw;

u_char *image_map;
int im_size;

/* from cmap.c */
u_char gray_lut[256];	/* just image */
u_char blend_lut[256];	/* mask-image blend */
u_char pt_lut[256];	/* point list */

/* cursor used everywhere */
Cursor watch_cursor;

/* preferences */
int	verbose;

/***********************************************************/
void
info_init()
{
        int i, j;
 
        segal.x = UNDEFINED;
        segal.y = UNDEFINED;
        segal.z = UNDEFINED;
	segal.r1 = UNDEFINED;
	segal.c1 = UNDEFINED;
	segal.f1 = UNDEFINED;
	segal.r2 = UNDEFINED;
	segal.c2 = UNDEFINED;
	segal.f2 = UNDEFINED;
        segal.r = UNDEFINED;
        segal.c = UNDEFINED;
        segal.f = UNDEFINED;
	segal.color = TRUE;
        segal.r3d = FALSE;
        segal.roi = R3d_WHOLE;
        segal.disp_image = TRUE;
        segal.disp_mask = TRUE;
        segal.disp_pts = TRUE;
        segal.disp_xy = TRUE;
        segal.mode = MODE_SEGMENT;
	segal.bg_i = UNDEFINED;
        segal.num_m = 0;
        segal.e_m = UNDEFINED;
	segal.new_m = UNDEFINED;

        mlog.sel_m = UNDEFINED;
        mlog.apply_log = TRUE;
        mlog.apply_order = ORDER_EI;
 
	timer.semaphore = UNLOCKED;
	timer.width = UNDEFINED;
	timer.height = UNDEFINED;
	strcpy(timer.message, "No message");
	timer.qfront = 0;
	timer.qrear = 0;

        img.loaded = FALSE;
	img.r = UNDEFINED;
	img.c = UNDEFINED;
	img.f = UNDEFINED;
	img.frame_size = UNDEFINED;
        img.dname[0] = NULL;
        img.fname[0] = NULL;
        img.fp = NULL;
        img.color = FALSE;
        img.changed_image = FALSE;
        img.changed_frame = FALSE;
	For_all_aspects for(j = 0; j < NUM_CPLANES; j++) {
		img.undo[i][j] = NULL;
		img.orig[i][j] = NULL;
	}
 
        For_all_masks {
                m[i].loaded = FALSE;
		m[i].f = UNDEFINED;
                m[i].dname[0] = NULL;
                m[i].fname[0] = NULL;
                m[i].changed_mask = FALSE;
                m[i].changed_frame = FALSE;
                m[i].mask_type = MASK_NO_APPLY;
                m[i].mask_hue = MASK_GREEN;
                m[i].bit_key = (int) pow(2.0, (double) i);
        }
 
        For_all_windows {
                win[i].zoom_mag = UNDEFINED;
 
                if(i <= ASPECT_Z) win[i].aspect = i;
                else win[i].aspect = ASPECT_Z;
 
                win[i].f = UNDEFINED;
                win[i].img_r = UNDEFINED;
                win[i].img_c = UNDEFINED;
                win[i].img_size = UNDEFINED;
                win[i].canv_x = UNDEFINED;
                win[i].canv_y = UNDEFINED;
                win[i].canv_w = UNDEFINED;
                win[i].canv_h = UNDEFINED;
                win[i].repaint = TRUE;
                win[i].xid = UNDEFINED;
                win[i].ximg = NULL;
        }
 
	crop.win_id = UNDEFINED;
	crop.x1 = UNDEFINED;
	crop.y1 = UNDEFINED;
	crop.f1 = UNDEFINED;
	crop.x2 = UNDEFINED;
	crop.y2 = UNDEFINED;
	crop.f2 = UNDEFINED;
	For_all_aspects for(j = 0; j < NUM_VIEW_HANDLES; j++) {
		crop.handle[i][j].x = UNDEFINED;
		crop.handle[i][j].y = UNDEFINED;
	}
 
	rgb.quant.quality = Q_BEST;
	rgb.quant.win = WIN_VZ;
	rgb.quant.num_from_histo = 100; /* determined by quality */
	rgb.quant.auto_r = UNDEFINED;
	rgb.quant.auto_g = UNDEFINED;
	rgb.quant.auto_b = UNDEFINED;
	rgb.quant.max_samples = UNDEFINED;
	rgb.quant.min_rgb_dist = UNDEFINED;
	rgb.num_histo = 0;
	rgb.histo = NULL;
	rgb.map_r = 32;			/* determined by quality */
	rgb.map_g = 24;
	rgb.map_b = 16;
	rgb.map = NULL;

        region.x1 = UNDEFINED;
        region.y1 = UNDEFINED;
        region.f1 = UNDEFINED;
        region.x2 = UNDEFINED;
        region.y2 = UNDEFINED;
        region.f2 = UNDEFINED;
        region.beg_frame = UNDEFINED;
        region.end_frame = UNDEFINED;

	threshold.min = 0;
	threshold.max = 255;
	threshold.roi = R2d_WHOLE;
	threshold.plane = VAL_RGB;
	threshold.mask_effect = THRESH_OVERWRITE;
	threshold.image_effect = -20;
	threshold.degree = 20;
	threshold.xid = UNDEFINED;
 
        frame.offset = 0;
        frame.current = UNDEFINED;
        frame.upper = UNDEFINED;
        frame.lower = UNDEFINED;
 
        brush.mode = BRUSH_MASK;
        brush.image_affect = -20;
	brush.degree = 20;
        brush.mask_affect = MASK_PAINT;
        brush.shape = BRUSH_SQUARE;
        brush.size = 1;
 
        grow.swin = WIN_PAINT;
        grow.direction = DIR_GROW;
        grow.apply_thresholds = TRUE;
        grow.threshold_min = 0;
        grow.threshold_max = 255;
        grow.apply_gradient = TRUE;
        grow.gradient_rad = 1;
        grow.gradient_min = 0;
        grow.gradient_max = 100;
        grow.apply_bridge = TRUE;
	grow.bridge_dist = 5;
        grow.bridge_min = 1;
        grow.bridge_max = 8;
        grow.extent = GROW_FRAME;
        grow.seed_pt_src = SEED_EDIT;
        grow.overwrite = TRUE;
        grow.disp_growth = DISP_ALL;
	grow.interractive = TRUE;
        grow.speed = 2000;
        grow.stack_empty = TRUE;

        For_all_bits bm_key[i] = (int) pow(2.0, (double) i);
}
