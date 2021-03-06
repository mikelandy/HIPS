/*
 * segal.c - Contains main() for project segal
 * This file was generated by `gxv'.
 *
 * Various other initialization code has been added.
 *
 *	-Bryan Skene
 */

#include "common.h"

#include "file_ui.h"
#include "filter_ui.h"
#include "image_reg_ui.h"
#include "mask_grow_ui.h"
#include "mask_log_ui.h"
#include "paint_ui.h"
#include "preferences_ui.h"
#include "threshold_ui.h"
#include "view_ui.h"

#ifdef MAIN
#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/textsw.h>
#include <xview/xv_xrect.h>
#include "list_ui.h"

Attr_attribute	INSTANCE;

/*
 * External variable declarations.
 */
view_win_objects	*View_win;
view_pop_timer_objects	*View_pop_timer;
file_pop_load_image_objects	*File_pop_load_image;
file_pop_load_mask_objects	*File_pop_load_mask;
file_pop_new_mask_objects	*File_pop_new_mask;
file_pop_save_as_objects	*File_pop_save_as;
file_pop_save_image_objects	*File_pop_save_image;
filter_pop_filter_objects	*Filter_pop_filter;
image_reg_pop_image_reg_objects	*Image_reg_pop_image_reg;
image_reg_pop_ref_frame_objects	*Image_reg_pop_ref_frame;
list_pop_list_objects	*List_pop_list;
mask_grow_pop_mask_grow_objects	*Mask_grow_pop_mask_grow;
mask_grow_pop_options_objects	*Mask_grow_pop_options;
mask_log_pop_mask_log_objects	*Mask_log_pop_mask_log;
mask_log_pop_options_objects	*Mask_log_pop_options;
paint_win_paint_objects	*Paint_win_paint;
preferences_pop_preferences_display_objects	*Preferences_pop_preferences_display;
threshold_pop_threshold_objects	*Threshold_pop_threshold;
paint_pop_brush_objects	*Paint_pop_brush;

void
main(argc, argv)
	int		argc;
	char		**argv;
{
	void create_cursors();
	void grow_setup();
	void info_init();
	void parse_args();

	void load_image_header();
	void load_mask();
	void recall_list();
	void new_mask_proc();
	void save_mask_as();
	void save_image_as();

	/* save in globals for HIPS header */
	ac = argc;
	av = argv;

	parse_args(argc, argv);

	/* Initialize XView */
	xv_init(XV_INIT_ARGC_PTR_ARGV, &argc, argv, NULL);
	INSTANCE = xv_unique_key();

	/* initialize global variables */
	info_init(); /* ***IMPORTANT*** init routine in storage.c */

	/*
	 * Initialize user interface components.
	 * Do NOT edit the object initializations by hand.
	 */
	View_win = view_win_objects_initialize(NULL, NULL);
	File_pop_load_image = file_pop_load_image_objects_initialize(NULL, View_win->win);
	File_pop_load_mask = file_pop_load_mask_objects_initialize(NULL, View_win->win);
	File_pop_new_mask = file_pop_new_mask_objects_initialize(NULL, View_win->win);
	File_pop_save_as = file_pop_save_as_objects_initialize(NULL, View_win->win);
	File_pop_save_image = file_pop_save_image_objects_initialize(NULL, View_win->win);
	Filter_pop_filter = filter_pop_filter_objects_initialize(NULL, View_win->win);
	Image_reg_pop_image_reg = image_reg_pop_image_reg_objects_initialize(NULL, View_win->win);
	Image_reg_pop_ref_frame = image_reg_pop_ref_frame_objects_initialize(NULL, View_win->win);
	List_pop_list = list_pop_list_objects_initialize(NULL, View_win->win);
	Mask_grow_pop_mask_grow = mask_grow_pop_mask_grow_objects_initialize(NULL, View_win->win);
	Mask_grow_pop_options = mask_grow_pop_options_objects_initialize(NULL, View_win->win);
	Mask_log_pop_mask_log = mask_log_pop_mask_log_objects_initialize(NULL, View_win->win);
	Mask_log_pop_options = mask_log_pop_options_objects_initialize(NULL, View_win->win);
	Paint_win_paint = paint_win_paint_objects_initialize(NULL, View_win->win);
	Paint_win_paint = paint_win_paint_objects_initialize(NULL, View_win->win);
	Paint_pop_brush = paint_pop_brush_objects_initialize(NULL, Paint_win_paint->win_paint);
	Preferences_pop_preferences_display = preferences_pop_preferences_display_objects_initialize(NULL, View_win->win);
	View_pop_timer = view_pop_timer_objects_initialize(NULL, View_win->win);
	Threshold_pop_threshold = threshold_pop_threshold_objects_initialize(NULL, NULL);
	
        /* get global X window stuff */
        display = (Display *) xv_get(View_win->win, XV_DISPLAY, NULL);
        screen = DefaultScreen(display);
	winv = DefaultVisual(display, screen);
	winv_info = (XVisualInfo *) malloc(sizeof(XVisualInfo));
	if((int) XMatchVisualInfo(display, screen,
			XDisplayPlanes(display, screen),
			PseudoColor, winv_info) == 0) {
		fprintf(stderr, "unable to find correct visual \n");
		exit(0);
	}
 
        if (DisplayPlanes(display, screen) == 1) {
                fprintf(stderr, "This program requires an 8-bit color monitor \n");
                exit(0);
        }
        gc = DefaultGC(display, screen);
        XSetForeground(display, gc, XWhitePixel(display, screen));
        watch_cursor = XCreateFontCursor(display, XC_watch);

	create_cursors();

	/* Allocate our standard colors to start with */
	cmap_init();

	/*** initialize user interface components ***/

	/* set text input fields to call procs on carraige-return */
	(void) xv_set(File_pop_load_image->text_image_fname,
		PANEL_VALUE, in_image,
		PANEL_NOTIFY_LEVEL, PANEL_SPECIFIED,
		PANEL_NOTIFY_PROC, load_image_header,
		NULL);
	(void) xv_set(File_pop_load_mask->text_mask_fname,
		PANEL_VALUE, in_mask,
		PANEL_NOTIFY_LEVEL, PANEL_SPECIFIED,
		PANEL_NOTIFY_PROC, load_mask,
		NULL);
	(void) xv_set(List_pop_list->text_l_fname,
		PANEL_VALUE, in_list,
		PANEL_NOTIFY_LEVEL, PANEL_SPECIFIED,
		PANEL_NOTIFY_PROC, recall_list,
		NULL);
	(void) xv_set(File_pop_new_mask->text_new_fname,
		PANEL_NOTIFY_LEVEL, PANEL_SPECIFIED,
		PANEL_NOTIFY_PROC, new_mask_proc,
		NULL);
	(void) xv_set(File_pop_save_image->text_i_save_fname,
		PANEL_NOTIFY_LEVEL, PANEL_SPECIFIED,
		PANEL_NOTIFY_PROC, save_image_as,
		NULL);
	(void) xv_set(File_pop_save_as->text_save_fname,
		PANEL_NOTIFY_LEVEL, PANEL_SPECIFIED,
		PANEL_NOTIFY_PROC, save_mask_as,
		NULL);

	/* The following xv_set does 2 things: getwd returns and strcopies */
	(void) xv_set(File_pop_load_image->text_image_dname,
		PANEL_VALUE, getwd(img.dname),
		NULL);
	(void) xv_set(File_pop_save_image->text_i_save_dname,
		PANEL_VALUE, img.dname,
		NULL);
	(void) xv_set(File_pop_load_mask->text_mask_dname,
		PANEL_VALUE, img.dname,
		NULL);
	(void) xv_set(File_pop_new_mask->text_new_dname,
		PANEL_VALUE, img.dname,
		NULL);

	/* set XID's */
        win[WIN_VX].xid = (XID) xv_get(canvas_paint_window(
                View_win->canv_x), XV_XID);

        win[WIN_VY].xid = (XID) xv_get(canvas_paint_window(
                View_win->canv_y), XV_XID);

        win[WIN_VZ].xid = (XID) xv_get(canvas_paint_window(
                View_win->canv_z), XV_XID);

        win[WIN_PAINT].xid = (XID) xv_get(canvas_paint_window(
                Paint_win_paint->canvas), XV_XID);

        win[WIN_REF].xid = (XID) xv_get(canvas_paint_window(
                Image_reg_pop_ref_frame->canvas), XV_XID);

	threshold.xid = (XID) xv_get(canvas_paint_window(
		Threshold_pop_threshold->canvas), XV_XID);

	timer.xid = (XID) xv_get(canvas_paint_window(
		View_pop_timer->canv_timer), XV_XID);

	/*
	 * Turn control over to XView.
	 */
	xv_main_loop(View_win->win);
	exit(0);
}

#endif

/***********************************************************/
void
parse_args(argc, argv)
int argc;
char *argv[];
{
    void      usageterm();
 
    in_image = in_mask = NULL;
    verbose = 0;
    overlay_hue = 170;          /* default  (dark greenish) */
 
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
    }                           /* while */
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
    fprintf(stderr, "        [-i HIPS file] load specified image file\n");
    fprintf(stderr, "        [-m HIPS file] load specified mask file\n");
    fprintf(stderr, "        [-l list_file] load list_file information\n");
    fprintf(stderr, "        [-v] verbose mode \n");
    fprintf(stderr, "        [-c NN] hue (0 to 360) for overlay blend color (default = 170)\n");
    fprintf(stderr, "        [-h] this list \n");
    fprintf(stderr, "        [-help] list of window attribute help \n\n");
    exit(0);
}
