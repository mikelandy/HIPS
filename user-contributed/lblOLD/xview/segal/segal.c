/*
 *
 *  This in the main routine for segal, a binary image mask building/editing
 *          analysis tool.
 *
 *  This program requires X-windows to run and the xview toolkit to
 *   complile.  Much of the user interface was created using 'guide' from
 *   Sun Microsystems. All files ending with _ui.c and _ui.h were created
 *   with 'gxv', the code generator for 'guide'.
 */

#include "common.h"
#include "segal_ui.h"

#include "hips.h"
#include "image_reg.h"
#include "display_control.h"
#include "frame_control.h"
#include "load_save.h"
#include "mask_control.h"
#include "mask_log.h"
#include "orig_view.h"
#include "pixedit.h"
#include "threshold.h"
#include "view.h"

/* BP's ui stuff */
#include "bp.h"
#include "surf_fit.h"

segal_win_objects *segal_win;
segal_stats_win_objects *segal_stats_win;

Attr_attribute INSTANCE;

int       ac;
char    **av;

/* global stuff used in all routines (see segal.h) */
IMAGE_INFO himage;
MASK_INFO m[MAX_MASKS];
ZOOM_INFO zoom_info;
SEGAL_INFO segal;
EDIT_INFO edit_info;
THRESHOLD_INFO threshold;
REGIS_INFO registry;
LINE_INFO ref_line, image_line;
REGION_INFO region;
GROW_INFO grow;
FRAME_INFO frame;

/* BP's structures */
SEGMENTATION_2D_INFO seg_2d;

Display *display;
u_long *colors;
u_long red_standout, green_standout, blue_standout, yellow_standout;

GC        gc;
int       screen;
Xv_cmsdata cms_data;

/* Image Pointers */
XImage   *image;
XImage   *orig_image;
XImage   *ref_frame_image;
XImage   *mask_image;
XImage   *blend_image;
XImage   *zoom_image;
XImage   *zoom_mask_image;
XImage   *zoom_blend_image;

u_char	**work_buf;	 /* used in editting */
u_char	**image_buf;	 /* used in editting */
u_char	**ref_image_frame_buf; /* used in registering */
u_char	**ref_mask_frame_buf; /* used in registering */

u_char	**work_undo_buf;
u_char	**image_undo_buf;

u_char	**mask_buf;	/* used in growing */
u_char	**stack_buf[MAX_STACK_HEIGHT]; /* used in growing 3-d */

Cursor    watch_cursor;

XID	view_xid, orig_view_xid, edit_xid, edit_control_xid;
XID	segal_control_xid, pop_load_xid, pop_create_xid, display_control_xid;
XID	frame_control_xid, mask_control_xid, mask_log_xid;
XID	threshold_xid, filter_xid; /* necessary here? */
XID	region_xid, image_frame_status_xid, mask_frame_status_xid;
XID	ref_frame_xid, image_reg_xid;

/* command line args */
char     *in_image, *in_mask, *in_list;
int       overlay_hue;

/**********************************************************/
void
main(argc, argv)
    int       argc;
    char    **argv;
{
    void parse_args();
    void init_segal_info();
    void but_load_proc();
    void but_create_proc();
    void load_list();
    void draw_filenames();
    void draw_hint();

    void view_win_init();
    void filter_win_init();
    void bp_win_init();
    void surf_fit_win_init();
    void orig_view_win_init();
    void edit_win_init();
    void frame_control_win_init();
    void mask_control_win_init();
    void mask_log_win_init();
    void load_save_init();
    void display_control_win_init();
    void threshold_win_init();

    xv_init(XV_INIT_ARGC_PTR_ARGV, &argc, argv, 0);
    INSTANCE = xv_unique_key();

    /* save in globals */

    ac = argc;			/* save argc and argv in globals to be used
				 * later */
    av = argv;			/* by the hips header routines */

    parse_args(argc, argv);

    /*
     * Initialize XView.
     */

    image = mask_image = blend_image = NULL;
    zoom_image = zoom_mask_image = zoom_blend_image = NULL;
    work_buf = image_buf = mask_buf = work_undo_buf = image_undo_buf = NULL;
    himage.data = NULL;
    himage.fp = NULL;

	/* Initialize the segal info structure */
	init_segal_info(); 
    /*
     * Initialize user interface components.
     */

	segal_win = segal_win_objects_initialize(NULL, NULL);
	segal_stats_win = segal_stats_win_objects_initialize(NULL,
		segal_win->win);

	display_control_win_init(segal_win->win);
	load_save_init(segal_win->win);
	mask_control_win_init(segal_win->win);

	/* Segal Main Control Defaults */
	(void) xv_set(segal_win->but_display,
		PANEL_INACTIVE, TRUE,
		NULL);

	(void) xv_set(segal_win->but_edit,
		PANEL_INACTIVE, TRUE,
		NULL);

	(void) xv_set(segal_win->but_frame,
		PANEL_INACTIVE, TRUE,
		NULL);

	(void) xv_set(segal_win->but_image_reg,
		PANEL_INACTIVE, TRUE,
		NULL);

	(void) xv_set(segal_win->but_mask,
		PANEL_INACTIVE, TRUE,
		NULL);

	(void) xv_set(segal_win->but_mask_log,
		PANEL_INACTIVE, TRUE,
		NULL);

	(void) xv_set(segal_win->but_undo,
		PANEL_INACTIVE, TRUE,
		NULL);

	(void) xv_set(segal_win->but_original,
		PANEL_INACTIVE, TRUE,
		NULL);

	(void) xv_set(segal_win->but_filter,
		PANEL_INACTIVE, TRUE,
		NULL);

	/* set text input fields to call procs on carraige-return */
	(void) xv_set(load_save_pop_load->load_fname,
		PANEL_VALUE, in_image,
		PANEL_NOTIFY_LEVEL, PANEL_SPECIFIED,
		PANEL_NOTIFY_PROC, but_load_proc,
		NULL);
	(void) xv_set(load_save_pop_create->create_fname,
		PANEL_NOTIFY_LEVEL, PANEL_SPECIFIED,
		PANEL_NOTIFY_PROC, but_create_proc,
		NULL);

	/* The following 2 xv_sets do 4 things: getwd returns and strcopies */
	(void) xv_set(load_save_pop_load->load_dname,
		PANEL_VALUE, getwd(himage.path),
		NULL);
	(void) xv_set(load_save_pop_create->create_dname,
		PANEL_VALUE, getwd(m[1].path),
		NULL);

	if(in_image != NULL)
		sprintf(himage.filename, "%s", in_image);
	else {
		(void) xv_set(segal_win->msg_image_size,
			PANEL_LABEL_STRING, "Image Size: Undefined",
			NULL);
	}
	if(in_mask != NULL)
		sprintf(m[0].filename, "%s", in_mask);

	if(in_list != NULL)
		(void) xv_set(load_save_pop_load->load_which,
			PANEL_VALUE, 2,
			NULL);

	draw_filenames();
	draw_hint("File I/O: Load an image");

	(void) xv_set(segal_win->msg_pixedit,
		PANEL_LABEL_STRING, "Edit Region Size: Undefined",
		NULL);

	/* default gamma set to 1.5 */
	(void) xv_set(display_control_win->gamma_val,
		PANEL_VALUE, 15,
		NULL);

	/* default slider values */
	(void) xv_set(display_control_win->bg_slider,
		PANEL_VALUE, 100,
		NULL);
	(void) xv_set(display_control_win->fg_slider,
		PANEL_VALUE, 0,
		NULL);

	/* until an image is loaded, the Original selector is invalid. */
	(void) xv_set(display_control_win->display_original,
		PANEL_INACTIVE, TRUE,
		NULL);

	/* mask control window defaults */
	(void) xv_set(mask_control_win->grow_gradient,
		PANEL_VALUE, 10,
		NULL);

	(void) xv_set(mask_control_win->grow_threshold_high,
		PANEL_VALUE, 255,
		NULL);

        (void) xv_set(mask_control_win->beg_frame,
		PANEL_VALUE, 0,
		PANEL_INACTIVE, TRUE,
		NULL);

	(void) xv_set(mask_control_win->end_frame,
		PANEL_VALUE, 0,
		PANEL_INACTIVE, TRUE,
		NULL);

	panel_paint(mask_control_win->beg_frame, PANEL_CLEAR);
	panel_paint(mask_control_win->end_frame, PANEL_CLEAR);

	/* get global X window stuff */
	display = (Display *) xv_get(segal_win->win, XV_DISPLAY, NULL);
	screen = DefaultScreen(display);

	if (DisplayPlanes(display, screen) == 1) {
		fprintf(stderr, "This program requires an 8-bit color monitor \n");
		exit(0);
	}
	gc = DefaultGC(display, screen);
	XSetForeground(display, gc, XWhitePixel(display, screen));
	watch_cursor = XCreateFontCursor(display, XC_watch);

	frame_control_win_init(segal_win->win);
	threshold_win_init(segal_win->win);
	filter_win_init(segal_win->win);
	image_reg_win_init(segal_win->win);
	view_win_init(segal_win->win);
	orig_view_win_init(segal_win->win);
	edit_win_init(segal_win->win);
	mask_log_win_init(segal_win->win);
	filter_win_init(segal_win->win);
	bp_win_init(segal_win->win);
	surf_fit_win_init(segal_win->win);

	/* windows start out closed */
	(void) xv_set(edit_win->win,
		XV_SHOW, FALSE,
		NULL);
	(void) xv_set(view_win->win,
		XV_SHOW, FALSE,
		NULL);
	(void) xv_set(orig_view_win->win,
		XV_SHOW, FALSE,
		NULL);
	(void) xv_set(load_save_pop_load->pop_load,
		XV_SHOW, FALSE,
		NULL);
	(void) xv_set(load_save_pop_create->pop_create,
		XV_SHOW, FALSE,
		NULL);
	(void) xv_set(display_control_win->win,
		XV_SHOW, FALSE,
		NULL);
	(void) xv_set(frame_control_win->win,
		XV_SHOW, FALSE,
		NULL);
	(void) xv_set(image_reg_win->win,
		XV_SHOW, FALSE,
		NULL);
	(void) xv_set(image_reg_pop_ref_frame->pop_ref_frame,
		XV_SHOW, FALSE,
		NULL);
	(void) xv_set(mask_control_win->win,
		XV_SHOW, FALSE,
		NULL);
	(void) xv_set(mask_log_win->win,
		XV_SHOW, FALSE,
		NULL);
	(void) xv_set(filter_win->win,
		XV_SHOW, FALSE,
		NULL);

	/* BP's stuff */
	(void) xv_set(bp_win->win,
		XV_SHOW, FALSE,
		NULL);
	(void) xv_set(surf_fit_win->win,
		XV_SHOW, FALSE,
		NULL);

	/* Set mask mode to "Build" */
	(void) xv_set(display_control_win->mask_type,
		PANEL_VALUE, 1,
		NULL);
	segal.mask_type = 1;

	/* set edit defaults */
	(void) xv_set(edit_win->zoom_mag,
		PANEL_VALUE, 1,
		NULL);
	(void) xv_set(edit_win->brush_size, 
		PANEL_VALUE, 1, 
		NULL);
	(void) xv_set(edit_win->mask_brush_mode,
		PANEL_INACTIVE, FALSE,
		NULL);
	(void) xv_set(edit_win->image_brush_mode,
		PANEL_INACTIVE, TRUE,
		NULL);
	(void) xv_set(edit_win->image_brush_delta,
		PANEL_INACTIVE, TRUE,
		NULL);

	/* NOT IMPLEMENTED STUFF (MIGHT BE USEFUL TO HAVE LATER) */

	(void) xv_set(threshold_win->msg_histogram,
		PANEL_INACTIVE, TRUE,
		NULL);

	(void) xv_set(threshold_win->histo_set,
		PANEL_INACTIVE, TRUE,
		NULL);

	/* polygon fill defaults */
	/*
	(void) xv_set(segal_win->poly_mesg, PANEL_LABEL_STRING, " ", NULL);
	(void) xv_set(segal_win->show_poly_item, PANEL_INACTIVE, TRUE, NULL);
	(void) xv_set(segal_win->poly_fill_item, PANEL_INACTIVE, TRUE, NULL);
	(void) xv_set(segal_win->clear_item, PANEL_INACTIVE, TRUE, NULL);
	*/

	/* set XID's */
	view_xid = (XID) xv_get(canvas_paint_window(
		view_win->canvas), XV_XID);
	orig_view_xid = (XID) xv_get(canvas_paint_window(
		orig_view_win->canvas), XV_XID);
	edit_xid = (XID) xv_get(canvas_paint_window(
		edit_win->canvas), XV_XID);
	edit_control_xid = (XID) xv_get(canvas_paint_window(
		edit_win->control), XV_XID);
	segal_control_xid = (XID) xv_get(canvas_paint_window(
		segal_win->controls), XV_XID);
	threshold_xid = (XID) xv_get(canvas_paint_window(
		threshold_win->canv_histo), XV_XID);
	ref_frame_xid = (XID) xv_get(canvas_paint_window(
		image_reg_pop_ref_frame->canv_ref_frame), XV_XID);
	image_reg_xid = (XID) xv_get(canvas_paint_window(
		image_reg_win->controls), XV_XID);
	pop_load_xid = (XID) xv_get(canvas_paint_window(
		load_save_pop_load->controls_load), XV_XID);
	pop_create_xid = (XID) xv_get(canvas_paint_window(
		load_save_pop_create->controls_create), XV_XID);
	display_control_xid = (XID) xv_get(canvas_paint_window(
		display_control_win->controls), XV_XID);
	frame_control_xid = (XID) xv_get(canvas_paint_window(
		frame_control_win->controls), XV_XID);
	image_frame_status_xid = (XID) xv_get(canvas_paint_window(
		frame_control_win->canv_image_frame_status), XV_XID);
	mask_frame_status_xid = (XID) xv_get(canvas_paint_window(
		frame_control_win->canv_mask_frame_status), XV_XID);
	mask_control_xid = (XID) xv_get(canvas_paint_window(
		mask_control_win->controls), XV_XID);
	mask_log_xid = (XID) xv_get(canvas_paint_window(
		mask_log_win->controls), XV_XID);

	/*
	* Turn control over to XView.
	*/

	window_main_loop(segal_win->win);
	exit(0);
}

/**********************************************************/
void
init_segal_info()
{
	segal.rows = 0;
	segal.cols = 0;
	segal.bit_map_len = 0;
	segal.frames = 0;
	segal.curr_frame = 0;
	segal.width = 0;
	segal.height = 0;
	segal.image_loaded = 0;
	segal.masks = 0;
	segal.edit_m = UNDEF;
	segal.poly_flag = 0;
	segal.display_type = 0;
	segal.mask_type = 0;
	segal.blend_type = 0;
	segal.slider1 = 0;
	segal.slider2 = 0;
}

/**********************************************************/
void
build_ximages()
{				/* allocates and maps all ximages */
    u_char   *dbuf;
    XVisualInfo *winv;
    void      map_images(), blend();

    if (verbose)
	fprintf(stderr, "creating X images ... \n");

    winv = (XVisualInfo *) malloc(sizeof(XVisualInfo));
    if ((int) XMatchVisualInfo(display, screen,
			       XDisplayPlanes(display, screen),
			       PseudoColor, winv) == 0) {
	fprintf(stderr, "unable to find correct visual \n");
	exit(0);
    }
    if (image == NULL) {
	dbuf = Calloc(segal.rows * segal.cols, u_char);
	image = XCreateImage(display, winv->visual, 8, ZPixmap, 0,
			     (char *) dbuf, segal.cols, segal.rows, 8, 0);
	if (image == NULL) {
	    fprintf(stderr, " Error creating image! \n");
	    return;
	}
    }
    if (ref_frame_image == NULL) {
	dbuf = Calloc(segal.rows * segal.cols, u_char);
	ref_frame_image = XCreateImage(display, winv->visual, 8, ZPixmap, 0,
			     (char *) dbuf, segal.cols, segal.rows, 8, 0);
	if (ref_frame_image == NULL) {
	    fprintf(stderr, " Error creating ref_frame_image! \n");
	    return;
	}
    }
    if (mask_image == NULL) {
	dbuf = Calloc(segal.rows * segal.cols, u_char);
	mask_image = XCreateImage(display, winv->visual, 8, ZPixmap, 0,
			       (char *) dbuf, segal.cols, segal.rows, 8, 0);
	if (mask_image == NULL) {
	    fprintf(stderr, " Error creating mask_image! \n");
	    return;
	}
    }
    if (blend_image == NULL) {
	dbuf = Calloc(segal.rows * segal.cols, u_char);
	blend_image = XCreateImage(display, winv->visual, 8, ZPixmap, 0,
				   (char *) dbuf,
				   segal.cols, segal.rows, 8, 0);
	if (blend_image == NULL) {
	    fprintf(stderr, " Error creating blend_image! \n");
	    return;
	}
    }
    /* allocate the canvas for the histogram in threshold */

    map_images();
}

/****************************************************/
void
set_reference_mask_apply(item, value, event)
Panel_item item;
int value;
Event *event;
{
}

/****************************************************/
void
draw_hint(message)
char *message;
{
	char foo[80];

	sprintf(foo, "Hint: %s", message);
	(void) xv_set(segal_win->msg_hint,
		PANEL_LABEL_STRING, foo,
		NULL);

	panel_paint(segal_win->msg_hint, PANEL_CLEAR);
}

/****************************************************/
void
draw_filenames()
{
	void draw_frame_statuses();
	char foo[MAXPATHLEN];

	strcpy(foo, "Image: ");
	if(!segal.image_loaded) strcpy(himage.filename, "Not Loaded");
	strcat(foo, himage.filename);
	if(himage.changed)
		strcat(foo, " (Altered)");
	(void) xv_set(segal_win->msg_image_fname,
		PANEL_LABEL_STRING, foo,
		NULL);

	strcpy(foo, "Mask: ");

	fprintf(stderr, "*** segal.edit_m = %d\n", segal.edit_m);

	if(segal.edit_m == UNDEF) {
		strcat(foo, "Not Loaded/Created");
	}
	else {
		strcat(foo, m[segal.edit_m].filename);
		if(m[segal.edit_m].frames[segal.curr_frame].frame_status
		== FRAME_ALTERED)
			strcat(foo, " (Altered)");
		(void) xv_set(frame_control_win->set_mask_frame_status,
			PANEL_VALUE, m[segal.edit_m].frames[segal.curr_frame].frame_status,
			NULL);
	}

	(void) xv_set(segal_win->msg_mask_fname,
		PANEL_LABEL_STRING, foo,
		NULL);

	draw_frame_statuses();
}

/****************************************************/

/****************************************************/
void
test_proc(item, event)
Panel_item      item;
Event           *event;
{
	fputs("segal: test_proc\n", stderr);
}

/****************************************************/
void
map_images()
{				/* called by build_ximages, step_up, and
				 * step_down routines */
	if(segal.image_loaded) {
		map_image_to_lut(himage.data[0], (u_char *) image->data,
			segal.rows * segal.cols);
		map_image_to_lut(ref_image_frame_buf[0], (u_char *)
			ref_frame_image->data, segal.rows * segal.cols);
	}

	if (segal.masks)
		map_image_to_lut(m[segal.edit_m].data[0], 
			(u_char *) mask_image->data, segal.rows * segal.cols);

	if (segal.image_loaded && segal.masks) {
		if(segal.reg_flag)
			blend(himage.data[0], ref_mask_frame_buf[0],
			(u_char *) blend_image->data, segal.rows * segal.cols);
		else
			blend(himage.data[0], m[segal.edit_m].data[0],
			(u_char *) blend_image->data, segal.rows * segal.cols);
	}
}

/***********************************************************/
void
parse_args(argc, argv)
    int       argc;
    char     *argv[];
{
    void      usageterm();

    in_image = in_mask = NULL;
    verbose = 0;
    overlay_hue = 170;		/* default  (dark greenish) */

    /* Interpret options  */
    while (--argc > 0 && (*++argv)[0] == '-') {
	char     *s;
	for (s = argv[0] + 1; *s; s++)
	    switch (*s) {
	    case 'i':
		if (argc < 2)
		    usageterm();
		in_image = *++argv;
		fprintf(stderr, " using image file: %s\n", in_image);
		argc--;
		break;
	    case 'l': /* File containing image + list of masks */
		if (argc < 2)
		    usageterm();
		in_list = *++argv;
		fprintf(stderr, " using list file: %s\n", in_list);
		argc--;
		break;
	    case 'm':
		if (argc < 2)
		    usageterm();
		in_mask = *++argv;
		fprintf(stderr, " using mask file: %s\n", in_mask);
		argc--;
		break;
	    case 'c':
		if (argc < 2)
		    usageterm();
		sscanf(*++argv, "%d", &overlay_hue);
		fprintf(stderr, " overlay hue set to %d \n", overlay_hue);
		argc--;
		break;
	    case 'v':
		verbose++;
		break;
	    case 'h':
		usageterm();
		break;
	    default:
		usageterm();
		break;
	    }
    }				/* while */
    if (overlay_hue < 0 || overlay_hue > 360) {
	fprintf(stderr, " Error: color value (hue) must be between 0 and 360. \n");
	exit(0);
    }
}

/******************************************************/
void
usageterm()
{
    fprintf(stderr, "Usage: segal [-i image][-m mask] [-v] [-c NN][-h][-help] \n");
    fprintf(stderr, "        [-i HIPS file] load specified image file \n");
    fprintf(stderr, "        [-m HIPS file] load specified mask file \n");
    fprintf(stderr, "        [-l list_file] load image and mask(s) specified in list_file \n");
    fprintf(stderr, "        [-v] verbose mode \n");
    fprintf(stderr, "        [-c NN] hue (0 to 360) for overlay blend color (default = 170) \n");
    fprintf(stderr, "        [-h] this list \n");
    fprintf(stderr, "        [-help] list of window attribute help \n\n");
    exit(0);
}
