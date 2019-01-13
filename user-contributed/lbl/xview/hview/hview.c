/*
 * hview.c   simple program to display HIPS images   -Brian Tierney
 */

#include <stdio.h>
#include <strings.h>
#include <math.h>
#include <sys/param.h>
#include <sys/types.h>
#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/textsw.h>
#include <xview/xv_xrect.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <xview/cms.h>

#include "hview_ui.h"

#include <hipl_format.h>

#define SCROLL_BAR_SIZE 21	/* width of scrollbar */

/*
 * Global object definitions.
 */
hview_win_objects *Hview_win;
Attr_attribute INSTANCE;

Display  *display;
Visual   *winv;
XID       xid;
Window    mainw;
Xv_Window paintwin;
GC        gc;
int       depth;

XImage   *ximage = NULL;

char     *myname;
byte     *image_data;		/* original data from file (after 24to8 ) */
byte     *image_map;		/* indices into x-colormap (ximage->data) */

int       width, height, im_size;
byte      red[256], green[256], blue[256];
int       numcol;

void      disp_img(), cmap_init(), share_proc(), change_mode(), mono_proc();
void      build_cmap(), CreateXImage(), apply_gamma();
byte     *load_file();
u_long   *build_colormap();

/* global colormap creation modes */
/* see color.c for an explaination of these flags */
int       noglob = 0, perfect = 0, ncols = 245, rwcolor = 0, mono = 0;
int       use_all = 0, verbose = 2;
int       quantize = 1;

/*****************************************************/
main(argc, argv)
    int       argc;
    char    **argv;
{
    XGCValues gcval;
    char     *fname;

    Progname = strsave(*argv);
    hipserrlev = HEL_SEVERE;	/* only exit if severe errors */

    myname = rindex(argv[0], '/');
    if (!myname)
	myname = argv[0];
    else
	myname++;

    if (argc == 2)
	fname = argv[1];
    else
	fname = "<stdin>";

    /*
     * Initialize XView.
     */
    xv_init(XV_INIT_ARGC_PTR_ARGV, &argc, argv, NULL);
    INSTANCE = xv_unique_key();

    /*
     * Initialize user interface components. Do NOT edit the object
     * initializations by hand.
     */
    Hview_win = hview_win_objects_initialize(NULL, NULL);

    /* set defaults */
    (void) xv_set(Hview_win->slider1, PANEL_VALUE, 12, NULL);

    /* set up display, winv, etc for future X calls */
    display = (Display *) xv_get(Hview_win->win, XV_DISPLAY, NULL);
    depth = DisplayPlanes(display, DefaultScreen(display));

    if (depth != 8 && depth != 24) {
	fprintf(stderr, "Error: This program requires an 8 or 24-bit color monitor \n");
	exit(0);
    }
    if (depth == 24)
	perfect = 1;		/* will always want perfect colors if an
				 * 24-bit display */

    winv = DefaultVisual(display, DefaultScreen(display));
    mainw = (Window) xv_get(Hview_win->win, XV_XID, NULL);
    paintwin = (Xv_Window) canvas_paint_window(Hview_win->can);
    xid = (XID) xv_get(paintwin, XV_XID, NULL);

    /* set up a GC with 'reasonable' values */
    gcval.foreground = BlackPixel(display, DefaultScreen(display));
    gcval.background = WhitePixel(display, DefaultScreen(display));
    gcval.clip_mask = None;
    gc = XCreateGC(display, xid, GCForeground | GCBackground |
		   GCClipMask, &gcval);

    /* initialize the colormap */

    image_data = load_file(fname);
    if (image_data == NULL)
	exit(0);


    /* make colormap */
    cmap_init();

    CreateXImage();

    image_map = (byte *) malloc(im_size);
    memcpy((char *) image_map, (char *) image_data, im_size);
    build_cmap();

    disp_img();

    /*
     * Turn control over to XView.
     */
    xv_main_loop(Hview_win->win);
    exit(0);
}

/***********************************************************/
void
CreateXImage()
{
    char     *dbuf;

    switch (depth) {
    case 8:
	dbuf = (char *) malloc(im_size);
	ximage = XCreateImage(display, winv, depth,
			      ZPixmap, 0, dbuf, width, height, 8, 0);
	if (!ximage) {
	    fprintf(stderr, "Error: couldn't create theImage!");
	    exit(0);
	}
	break;

	/*********************************/
#ifdef LATER			/* someday make work with mono displays */
    case 1:
	dbuf = (char *) malloc(im_size / 8);
	ximage = XCreateImage(display, winv, depth,
			      XYPixmap, 0, dbuf, width, height, 8, 0);
	if (!ximage) {
	    fprintf(stderr, "Error: couldn't create theImage!");
	    exit(0);
	}
	break;
#endif

	/*********************************/
    case 24:
    case 32:
	dbuf = (char *) malloc(im_size * 4);
	ximage = XCreateImage(display, winv, depth,
			      ZPixmap, 0, dbuf, width, height, 32, 0);
	if (!ximage) {
	    fprintf(stderr, "Error: couldn't create theImage!");
	    exit(0);
	}
	break;
    }
}

/*************************************************************/
/*
 * Repaint callback function for `canvas1'.
 */
void
canvas_repaint_proc(canvas, paint_window, display, xid, rects)
    Canvas    canvas;
    Xv_window paint_window;
    Display  *display;
    Window    xid;
    Xv_xrectlist *rects;
{
#ifdef DEBUG
    fputs("hview: canvas_repaint_proc\n", stderr);
#endif

    if (ximage != NULL) {
	XPutImage(display, xid, gc, ximage, 0, 0, 0, 0,
		  ximage->width, ximage->height);
    }
}

/***************************************************************/
imgwin_resize_proc(win, event, arg, type)
    Xv_window win;
    Event    *event;
    Notify_arg arg;
    Notify_event_type type;
{
    int       width, height, ctrl_height;

#ifdef DEBUG
    fprintf(stderr, "hview: imgwin_resize_proc: event %d\n", event_id(event));
#endif
    ctrl_height = (int)xv_get(Hview_win->controls1, XV_HEIGHT, NULL);

    width = ximage->width + SCROLL_BAR_SIZE;
    height = ximage->height + SCROLL_BAR_SIZE + ctrl_height;

    if ((int) xv_get(Hview_win->win, XV_WIDTH) > width)
	xv_set(Hview_win->win, XV_WIDTH, width, NULL);

    if ((int) xv_get(Hview_win->win, XV_HEIGHT) > height)
	xv_set(Hview_win->win, XV_HEIGHT, height, NULL);

    return;
}

/***************************************************/
void
disp_img()
{
    int       win_width, win_height, ctrl_width, ctrl_height;

    win_width = MIN((ximage->width + SCROLL_BAR_SIZE), 512);

    ctrl_width = (int)xv_get(Hview_win->controls1, XV_WIDTH, NULL);
    ctrl_height = (int)xv_get(Hview_win->controls1, XV_HEIGHT, NULL);

    if (win_width < ctrl_width)
	win_width = ctrl_width;
    win_height = MIN((ximage->height + SCROLL_BAR_SIZE + ctrl_height), 512);

    xv_set(Hview_win->win,
	   XV_WIDTH, win_width,
	   XV_HEIGHT, win_height,
	   NULL);

    xv_set(Hview_win->can,
	   CANVAS_AUTO_EXPAND, FALSE,
	   CANVAS_AUTO_SHRINK, FALSE,
	   OPENWIN_AUTO_CLEAR, TRUE,
	   CANVAS_FIXED_IMAGE, FALSE,
	   NULL);
    xv_set(Hview_win->can,
	   CANVAS_WIDTH, ximage->width,
	   CANVAS_HEIGHT, ximage->height,
	   NULL);

    canvas_repaint_proc(Hview_win->can, paintwin, display, xid, NULL);
}

/***************************************************/
byte     *
load_file(fname)
    char     *fname;
{
    FILE     *fp;
    struct header hd;
    byte     *image, *outbuf;
    int       i;
    byte     *pr, *pg, *pb;

    if ((int)(fp = hfopenr(fname)) == -1) {
	    fprintf(stderr, "\n error opening file %s \n\n", fname);
	return (NULL);
	}

    /* read in HIPS header */
    if (fread_header(fp, &hd, (Filename) fname) == HIPS_ERROR) {
	XBell(display, 0);
	fclose(fp);
	return (NULL);
    }
    if (hd.pixel_format != PFBYTE && hd.pixel_format != PFRGB) {
	XBell(display, 0);
	fprintf(stderr, "Data type not currently understood.\n");
	return (NULL);
    }
    if (findparam(hd, "cmap") != NULLPAR) {
	getparam(&hd, "cmap", PFBYTE, &numcol, &pr);
	if (numcol % 3)
	    perr(HE_MSG, "colormap length not a multiple of 3");
	numcol /= 3;
	pg = pr + numcol;
	pb = pg + numcol;
	fprintf(stderr, "Reading colormap of size %d \n", numcol);
	for (i = 0; i < numcol; i++) {
	    red[i] = pr[i];
	    green[i] = pg[i];
	    blue[i] = pb[i];
	}
    } else {
	/* build rgb arrays for gray scale images */
	for (i = 0; i < 256; i++)
	    red[i] = green[i] = blue[i] = i;
	numcol = 256;
    }


    fprintf(stderr, "Loading image...... \n");
    if (hd.pixel_format == PFRGB)
	im_size = hd.cols * hd.rows * 3;
    else if (hd.pixel_format == PFRGBZ)
	im_size = hd.cols * hd.rows * 4;
    else
	im_size = hd.cols * hd.rows;

    image = (byte *) malloc(im_size);
    if (fread(image, im_size, 1, fp) != 1) {
	XBell(display, 0);
	perror("\n error reading data\n");
	return (NULL);
    }
    width = hd.cols;
    height = hd.rows;
    im_size = width * height;

    if (hd.pixel_format == PFRGB) {
	outbuf = (byte *) calloc(im_size, sizeof(byte));

	if (quantize) {
	    Init24to8();
	    numcol = Conv24to8(image, outbuf, width, height, ncols, mono,
			       red, green, blue);
	} else {		/* dither */
	    numcol = To_8(image, outbuf, width, height, ncols,
			  red, green, blue);
	    fprintf(stderr, "RGB image dithered to %d colors \n", numcol);
	}
	image = outbuf;
    }
    return (image);
}

/**********************************************************************/
void
cmap_init()
{
    int       i;
    Colormap  cmap;

    cmap = XDefaultColormap(display, DefaultScreen(display));
}

/*************************************************************/
void
build_cmap()
{
    /* sorted map is returned im image_map */
    build_colormap(display, winv, mainw,
		   image_map, im_size,
		   red, green, blue, numcol,
		   noglob, perfect, ncols, mono, use_all, rwcolor,
		   verbose, myname);

    map_image_to_colors(ximage->data, image_map, im_size);

#ifdef DEBUG
    show_colormap(image_data, im_conv);
#endif
    return;
}

/*************************************************************/
void
gamma_proc(item, value, event)
    Panel_item item;
    int       value;
    Event    *event;
{
    double    gammaval;
    static double old_gammaval = -1.0;
    int       get_gamma();

    gammaval = (double) xv_get(Hview_win->slider1, PANEL_VALUE, NULL) / 10.;

    if (gammaval == old_gammaval)
	return;

    fprintf(stderr, "hview: performing gamma with value %.2f \n",
	    (float) gammaval);

    old_gammaval = gammaval;

    apply_gamma(gammaval, noglob, perfect, ncols, mono, rwcolor);

    map_image_to_colors((byte *) ximage->data, image_map, im_size);

    canvas_repaint_proc(Hview_win->can, paintwin, display, xid, NULL);

    fprintf(stderr, "Done. \n");
}

/*************************************************************/
int
get_gamma(val, gam)
    byte      val;
    double    gam;
{
    double    newval;

    newval = 256. * pow((double) val / 256., (double) (1.0 / gam));
    return (MIN((int) (newval + .5), 255));	/* dont allow values > 255 */
}
