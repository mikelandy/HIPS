
/*
 * xhist.c                              Brian Tierney,  12/89
 *					Felix Huang,  6/27/91
 *
 * This is an X window's based program to display a histogram
 *   of a buffer of values.  The Buffer can be byte, short, or int
 *   and of any arbitrary size.
 */

#include "xhist.h"

 /* global window stuff */
static xhist_win_objects *hist_win;
Attr_attribute INSTANCE;

static Display *dpy;
static Display *display;
static XID xid;
static GC gc;
u_long    color_Red;
u_long    color_vRed;
u_long    color_Blue;
u_long    color_Green;
u_long    color_Orange;

Xv_window cpw;			/* global canvas paint window id */

/******************************************************/
void
main(argc, argv)
    int       argc;
    char    **argv;
{
    cur_fname = NULL;

    Progname = strsave(*argv);
    hipserrlev = HEL_SEVERE;  /* only exit if severe errors */
    hipserrprt = HEL_ERROR;  /* print messages for hips errors */

    if (argc > 1) {
	cur_fname = (char *) malloc(100);
	strcpy(cur_fname, argv[1]);
	load_image();
    } else if (isatty(0) == 0) {/* input redirect,  <  or ... |   */
	cur_fname = "<stdin>";
	load_image();
    }
    /*
     * Initialize XView.
     */
    xv_init(XV_INIT_ARGS, argc, argv, 0);
    INSTANCE = xv_unique_key();

    dpy = XOpenDisplay(NULL);

    /*
     * Initialize user interface components.
     */
    hist_win = xhist_win_objects_initialize(NULL, NULL);

    /* printf("in main, after set: hist_win\n"); */

    cpw = (Xv_window) xv_get(hist_win->canvas, CANVAS_NTH_PAINT_WINDOW, NULL);
    make_simple_colormap(cpw);

    /* printf("in main, after  make_simple_colormap(cpw)\n"); */

    if (argc > 1)
	(void) xv_set(hist_win->file_item, PANEL_VALUE, argv[1], NULL);

    if (size != 0)
	eval_proc();		/* image exists.	 */

    /*
     * Turn control over to XView.
     */
    window_main_loop(hist_win->win);
    exit(0);
}

 /**************************************************************************/

Xv_Font   font_small;
Xv_Font   font_bold;

make_simple_colormap(xvwin)
    Xv_window xvwin;
{
    Colormap  colmap;
    XColor    mycolor;
    XGCValues gc_val;
    int       screen;

    display = (Display *) xv_get(xvwin, XV_DISPLAY, NULL);
/*    XSetWindowBorderWidth(display, hist_win->canvas, 0); canvas */

    screen = XDefaultScreen(display);
    colmap = XDefaultColormap(display, screen);

    font_bold = (Xv_Font) xv_find(xvwin, FONT,
				  FONT_FAMILY, FONT_FAMILY_COUR,
				  FONT_STYLE, FONT_STYLE_BOLD,
				  NULL);

    font_small = (Xv_Font) xv_find(xvwin, FONT,
				   FONT_FAMILY, FONT_FAMILY_ROMAN,
				   NULL);

    if (XParseColor(display, colmap, "red", &mycolor, &mycolor) < 1) {
	fprintf(stderr, "Error parsing color %s:", "red");
	perror((char *) NULL);
	exit(0);
    }
    mycolor.flags = DoRed | DoGreen | DoBlue;
    if (XAllocColor(display, colmap, &mycolor) < 1) {
	fprintf(stderr, "Error allocating color ");
	perror((char *) NULL);
    }
    XInstallColormap(display, colmap);
    color_Red = mycolor.pixel;

    if (XParseColor(display, colmap, "blue", &mycolor, &mycolor) < 1) {
	fprintf(stderr, "Error parsing color %s:", "red");
	perror((char *) NULL);
	exit(0);
    }
    mycolor.flags = DoRed | DoGreen | DoBlue;
    if (XAllocColor(display, colmap, &mycolor) < 1) {
	fprintf(stderr, "Error allocating color ");
	perror((char *) NULL);
    }
    XInstallColormap(display, colmap);
    color_Blue = mycolor.pixel;

    if (XParseColor(display, colmap, "orange", &mycolor, &mycolor) < 1) {
	fprintf(stderr, "Error parsing color %s:", "red");
	perror((char *) NULL);
	exit(0);
    }
    mycolor.flags = DoRed | DoGreen | DoBlue;
    if (XAllocColor(display, colmap, &mycolor) < 1) {
	fprintf(stderr, "Error allocating color ");
	perror((char *) NULL);
    }
    XInstallColormap(display, colmap);
    color_Orange = mycolor.pixel;

    if (XParseColor(display, colmap, "violet red", &mycolor, &mycolor) < 1) {
	fprintf(stderr, "Error parsing color %s:", "red");
	perror((char *) NULL);
	exit(0);
    }
    mycolor.flags = DoRed | DoGreen | DoBlue;
    if (XAllocColor(display, colmap, &mycolor) < 1) {
	fprintf(stderr, "Error allocating color ");
	perror((char *) NULL);
    }
    XInstallColormap(display, colmap);
    color_vRed = mycolor.pixel;

    if (XParseColor(display, colmap, "medium aquamarine", &mycolor, &mycolor) < 1) {
	fprintf(stderr, "Error parsing color %s:", "red");
	perror((char *) NULL);
	exit(0);
    }
    mycolor.flags = DoRed | DoGreen | DoBlue;
    if (XAllocColor(display, colmap, &mycolor) < 1) {
	fprintf(stderr, "Error allocating color ");
	perror((char *) NULL);
    }
    XInstallColormap(display, colmap);
    color_Green = mycolor.pixel;

    /* stuff needed for changing colors and other X-windows stuff */
    xid = (XID) xv_get(xvwin, XV_XID);
    gc_val.foreground = BlackPixel(display, DefaultScreen(display));
    gc_val.background = WhitePixel(display, DefaultScreen(display));
    gc_val.font = (Font) xv_get(font_small, XV_XID);
    gc = XCreateGC(display, xid, GCForeground | GCBackground | GCFont, &gc_val);
}


 /************************************************************/
void
load_proc()
{
    char     *fname;

    fname = (char *) xv_get(hist_win->file_item, PANEL_VALUE);

    if (fname == NULL || strlen(fname) == 0)
	return;

/*  printf("in load_proc, fname = %s\n", fname);  */

    if (cur_fname == NULL || strcmp(fname, cur_fname) != 0) {	/* only if new file name */

	/* free any previously allocated stuff */
	if (size != 0) {
	    if (pix_format == PFINT)
		cfree(i_hist);
	    else if (pix_format == PFSHORT)
		cfree(s_hist);
	    else if (pix_format == PFFLOAT)
		cfree(f_hist);

	    size = 0;
	}
	if (cur_fname != NULL && strcmp(cur_fname, "<stdin>") != 0)
	    free(cur_fname);
	cur_fname = (char *) malloc(100);
	strcpy(cur_fname, fname);
	if (load_image() < 0)
		return;

	restore_proc();		/* draw the picture	 */
    }				/* end of if new file	 */
}				/* end of load_proc		 */

set_count_proc()
{
    char     *s;
    int       i;
    int       n;

    s = (char *) xv_get(hist_win->count, PANEL_VALUE, NULL);

    i = 0;			/* index of s		 */
    while (s[i] == ' ' || s[i] == '\t')
	i++;			/* skip beginning blanks  */

    if (s[i] < '0' || '9' < s[i])
	return;

    n = 0;			/* sum	 */
    while ('0' <= s[i] && s[i] <= '9') {
	n = n * 10 + (s[i] - '0');
	i++;
    }

    if (Bottom <= n && n <= Top)
	set_NewBottom(n);
}

check_proc()
{				/* check specified pixel value	 */
    char     *s;
    int       i, j;
    int       pv_sign, n;
    float     f, g;

    s = (char *) xv_get(hist_win->pixel_value, PANEL_VALUE, NULL);

    i = 0;			/* index of s		 */
    while (s[i] == ' ' || s[i] == '\t')
	i++;			/* skip beginning blanks  */

    if (s[i] == '-') {
	pv_sign = -1;
	i++;
    } else
	pv_sign = 1;

    if (s[i] < '0' || '9' < s[i])
	return;

    n = 0;			/* sum	 */
    while ('0' <= s[i] && s[i] <= '9') {
	n = n * 10 + (s[i] - '0');
	i++;
    }

    n = n * pv_sign;

    if (pix_format != PFFLOAT) {
#ifdef aax
	printf("in check_proc, pixel value=%d\n", n);
#endif

	for (j = 0; j <= range; j++)
	    if (histo_va[j] <= n && n < histo_va[j + 1]) {
		mouse_move(j);
		break;
	    }
	return;
    }
    /* float	 */
    f = 0.;
    if (s[i] == '.') {
	i++;
	if ('0' <= s[i] && s[i] <= '9') {
	    f = .1 * (s[i] - '0');
	    i++;
	    if ('0' <= s[i] && s[i] <= '9')
		f = f + .01 * (s[i] - '0');
	}
    }
    g = f * pv_sign + n;

#ifdef aax
    printf("in check_proc, pixel value=%.2f\n", g);
#endif

    for (j = 0; j <= range; j++)
	if (fhisto_va[j] <= g && g < fhisto_va[j + 1]) {
	    mouse_move(j);
	    break;
	}
}				/* end of check_proc ()	 */

restore_proc()
{
    paint = 0;			/* fisrt picture of this image	 */

    eval_proc();
}

eval_proc()
{
    CV_HT = (int) xv_get(hist_win->canvas, XV_HEIGHT);	/* canvas height */
    HGT = CV_HT - 9 * TXTHT;
    /* printf("in eval_proc,  CV_HT=%d\n", CV_HT);  */

    comp_hg();			/* compute histo_graph         */
    histogram_repaint_proc();
}

/*************************************************************/
void
quit_proc()
{
    xv_set(hist_win->win, FRAME_NO_CONFIRM, TRUE, NULL);
    (void) xv_destroy_safe(hist_win->win);
    /* this will also kill all child windows */
}

/*************************************************************/
void
move_left_proc()
{
    if (move_j == -1)
	mouse_move(range / 3);
    else if (move_j > 0)
	mouse_move(move_j - 1);
}

void
move_right_proc()
{
    if (move_j == -1)
	mouse_move(range * 2 / 3);
    else if (move_j < range)
	mouse_move(move_j + 1);
}

void
select_proc()
{
    if (NewLeft != -1 && NewRight != -1)
	return;

    if (move_j == -1)
	move_j = range / 3;

    if (NewLeft == -1)
	NewLeft = move_j;	/* relative	 */
    else if (NewLeft < move_j)
	NewRight = move_j;
    else if (move_j < NewLeft) {
	NewRight = NewLeft;
	NewLeft = move_j;
    } else
	return;			/* NewLeft != -1 && NewLeft == move_j	 */

    draw_vertibar();		/* draw vertical bars	 */
}

set_NewBottom(occ)		/* set NewBottom or NewTop	 */
    int       occ;		/* occurrence, Bottom <= occ <= Top		 */
{
    if (NewBottom == -1)
	NewBottom = occ;
    else if (NewBottom < occ)
	NewTop = occ;
    else if (occ < NewBottom) {
	NewTop = NewBottom;
	NewBottom = occ;
    } else
	return;

#ifdef aax
    printf("in set_NewBottom, NewBottom=%d  NewTop=%d\n",
	   NewBottom, NewTop);
#endif

    draw_horizbar();		/* draw horizontal bar	 */
}

draw_horizbar()
{				/* draw horizontal bar     */
    char      label[30];
    int       ypos;
    int       occ;		/* occurrence, Bottom <= occ <= Top		 */

    if (NewBottom != -1) {
	occ = NewBottom;
	ypos = 3 * TXTHT + HGT - (occ - Bottom) * y_unit;
	XSetForeground(display, gc, color_Green);
	HorDashLine(2 * TXTWD, 2 * TXTWD + range + 3, ypos);
	sprintf(label, "%d", occ);
	XDrawString(display, xid, gc, (int) (2 * TXTWD + range + 5), ypos + 4,
		    label, strlen(label));
    }
    if (NewTop != -1) {
	occ = NewTop;
	ypos = 3 * TXTHT + HGT - (occ - Bottom) * y_unit;
	XSetForeground(display, gc, color_Green);
	HorDashLine(2 * TXTWD, 2 * TXTWD + range + 3, ypos);
	sprintf(label, "%d", occ);
	XDrawString(display, xid, gc, (int) (2 * TXTWD + range + 5), ypos + 4,
		    label, strlen(label));
    }
}

draw_vertibar()
{
    int       x, y1, y2;
    char      label[30];
    Font_string_dims dims;
    int       w0;

    if (NewLeft != -1) {
	x = 2 * TXTWD + NewLeft;
	XSetForeground(display, gc, color_Blue);
	sprintf(label, "%d", histo_graph[NewLeft]);
	xv_get(font_small, FONT_STRING_DIMS, label, &dims);
	w0 = dims.width / 2;
	XDrawString(display, xid, gc, x - w0, 2 * TXTHT - 6,
		    label, strlen(label));

	XSetForeground(display, gc, color_Orange);
	y1 = 2 * TXTHT;
	y2 = 6 * TXTHT + HGT;
	VerDashLine(x, y1, y2);
	draw_pv(NewLeft, x, (int) (7 * TXTHT + HGT - 6));
    }
    if (NewRight != -1) {
	x = 2 * TXTWD + NewRight;
	XSetForeground(display, gc, color_Blue);
	sprintf(label, "%d", histo_graph[NewRight]);
	xv_get(font_small, FONT_STRING_DIMS, label, &dims);
	w0 = dims.width / 2;
	XDrawString(display, xid, gc, x - w0, 2 * TXTHT - 6,
		    label, strlen(label));

	XSetForeground(display, gc, color_Orange);
	y1 = 2 * TXTHT;
	y2 = 6 * TXTHT + HGT;
	VerDashLine(x, y1, y2);
	draw_pv(NewRight, x, (int) (7 * TXTHT + HGT - 6));
    }
}				/* end of  draw_vertibar ()  	 */

draw_pv(j, x, y)
    int       j, x, y;
{
    char      label[30];
    Font_string_dims dims;
    int       w0;

    if (pix_format != PFFLOAT) {
	if (histo_va[j + 1] - histo_va[j] <= 1) {
	    sprintf(label, "%d", histo_va[j]);
	    xv_get(font_small, FONT_STRING_DIMS, label, &dims);
	    w0 = dims.width / 2;
	    XDrawString(display, xid, gc, x - w0, y, label, strlen(label));
	} else {
	    sprintf(label, "%d-", histo_va[j]);
	    xv_get(font_small, FONT_STRING_DIMS, label, &dims);
	    w0 = dims.width / 2;
	    XDrawString(display, xid, gc, x - w0, y, label, strlen(label));
	    sprintf(label, "%d", histo_va[j + 1] - 1);
	    xv_get(font_small, FONT_STRING_DIMS, label, &dims);
	    w0 = dims.width / 2;
	    XDrawString(display, xid, gc, x - w0, y + TXTHT, label, strlen(label));
	}
    } else {			/* float	 */
	sprintf(label, "%.2f-", fhisto_va[j]);
	xv_get(font_small, FONT_STRING_DIMS, label, &dims);
	w0 = dims.width / 2;
	XDrawString(display, xid, gc, x - w0, y, label, strlen(label));
	sprintf(label, "%.2f", fhisto_va[j + 1] - .01);
	xv_get(font_small, FONT_STRING_DIMS, label, &dims);
	w0 = dims.width / 2;
	XDrawString(display, xid, gc, x - w0, y + TXTHT, label, strlen(label));
    }

}				/* end of draw_pv	 */

/******************************************************************/

/*
 * Event callback function for `canvas'.
 */
Notify_value
canvas_event_proc(win, event, arg, type)
    Xv_Window win;
    Event    *event;
    Notify_arg arg;
    Notify_event_type type;
{
    int       xpos, ypos;

    int       occ;		/* occurrence, Bottom <= occ <= Top		 */


    if (event_is_up(event))	/* ingore all up events */
	return notify_next_event_func(win, event, arg, type);

    if (event_meta_is_down(event))	/* ingore all meta events */
	return notify_next_event_func(win, event, arg, type);

    xpos = event_x(event);
    ypos = event_y(event);

    if (size == 0 || xpos < 2 * TXTWD || xpos > 2 * TXTWD + range
	|| ypos < 3 * TXTHT || ypos > 3 * TXTHT + HGT)
	return;

    XSetFunction(display, gc, GXcopy);
    XSetForeground(display, gc, color_Red);

    switch (event_id(event)) {
    case MS_LEFT:
    case LOC_DRAG:
/*	printf("in canvas_proc, LOC_MOVE,  xpos=%d  ypos=%d  size=%d\n",
		xpos, ypos, size);  */
	if (xpos - 2 * TXTWD != move_j)
	    mouse_move(xpos - 2 * TXTWD);
	break;
    case MS_MIDDLE:
	if (xpos - 2 * TXTWD != move_j)
	    mouse_move(xpos - 2 * TXTWD);
	select_proc();		/* select vertical bar   */
	break;
    case MS_RIGHT:
	if (NewTop == -1 || NewBottom == -1) {
	    occ = Bottom + (3 * TXTHT + HGT - ypos) / y_unit;
	    set_NewBottom(occ);
	}
	break;
    }

    XSetForeground(display, gc, BlackPixel(display, DefaultScreen(display)));
    return notify_next_event_func(win, event, arg, type);
}

/************************************************************************/
/*
 * Repaint callback function for `canvas'.
 */

histogram_repaint_proc()
{
    /*
     * this routine uses a combination of standard X and Sun pixwin calls.
     * Probably should change everything to standard X, but I like of pw text
     * routines better.
     */
    int       i;

    int       wd, ht;		/* frame width, height	 */

    int       ypos;

    int       x, y1, y2;

    wd = (int) xv_get(hist_win->win, XV_WIDTH);
    FRM_WD = range + 17 * TXTWD;
    if (FRM_WD < 500)
	FRM_WD = 500;
/*    printf("in repaint, wd=%d   FRM_WD=%d\n", wd, FRM_WD); */
    xv_set(hist_win->win, XV_WIDTH, FRM_WD, NULL);

    ht = (int) xv_get(hist_win->win, XV_HEIGHT);
    CV_HT = (int) xv_get(hist_win->canvas, XV_HEIGHT);	/* canvas height */
    HGT = CV_HT - 9 * TXTHT;
    /* printf("in repaint, ht=%d   CV_HT=%d\n", ht, CV_HT); */
    XMapWindow(dpy, hist_win->win);

    XSetFunction(display, gc, GXclear);
    XSetForeground(display, gc, BlackPixel(display, DefaultScreen(display)));
    XSetBackground(display, gc, WhitePixel(display, DefaultScreen(display)));
    /* XClearWindow(display, cpw); */

    xv_rop(cpw, 0, 0, (int) xv_get(cpw, XV_WIDTH, NULL),
	   (int) xv_get(cpw, XV_HEIGHT, 0),
	   PIX_CLR, NULL, 0, 0);

    XSetFunction(display, gc, GXcopy);

    if (size == 0)
	return;			/* no image		 */

    pri_hint();			/* print hint		 */
/*    printf("in repaint, after pri_hint(), cv_wd=%d  cv_ht=%d\n",
		xv_get(cpw, XV_WIDTH), xv_get(cpw, XV_HEIGHT) );  */

    XSetForeground(display, gc, color_Black);	/* black	 */
    y_unit = HGT / (Top - Bottom);
    for (i = 0; i <= range; i++) {
	if (histo_graph[i] > 0) {
	    x = 2 * TXTWD + i;
	    y1 = 3 * TXTHT + HGT;
	    y2 = 3 * TXTHT + HGT - (histo_graph[i] - Bottom) * y_unit;

	    XDrawLine(display, xid, gc, x, y1, x, y2);
	}
    }

    draw_coor();		/* draw coordinates	 */

    XMapWindow(dpy, hist_win->win);

    move_j = -1;
    NewLeft = -1;
    NewRight = -1;
    NewTop = -1;
    NewBottom = -1;
}

draw_coor()
{				/* draw coordinates  */
    int       i;
    int       xpos;
    int       ypos;

    /* Horizontal labels, vertical lines */
    XSetForeground(display, gc, color_Blue);	/* blue	 */

    for (i = 0; i < end_x; i++) {
	xpos = x_pos[i];
	VerDashLine(xpos, 3 * TXTHT, (int) (3 * TXTHT + HGT + 5));
    }

    ypos = 3 * TXTHT;
    XDrawLine(display, xid, gc, 2 * TXTWD, ypos, 2 * TXTWD + range + 3, ypos);
    ypos = 3 * TXTHT + HGT;
    XDrawLine(display, xid, gc, 2 * TXTWD, ypos, 2 * TXTWD + range + 3, ypos);
    ypos = 3 * TXTHT + HGT + 1;
    XDrawLine(display, xid, gc, 2 * TXTWD, ypos, 2 * TXTWD + range, ypos);

    for (i = 0; i < end_y; i++) {
	ypos = y_pos[i];
	HorDashLine(2 * TXTWD, (int) (2 * TXTWD + range + 5), ypos);
    }
    xy_coor();			/* 0  50  100  150  200	 */
}

xy_coor()
{

    int       i, j;
    int       xpos, x0, x1;

    XSetForeground(display, gc, color_vRed);

    end_xc = 0;			/* end of x_cover_l[] and x_cover_r[]     */

    one_x_lab(0);
    one_x_lab(end_x - 1);
    for (i = 1; i < end_x - 1; i++)
	one_x_lab(i);

    XSetForeground(display, gc, color_Blue);

    end_yc = 0;			/* end of y_cover_s[] and y_cover_g[]     */

    one_y_lab(0);
    one_y_lab(end_y - 1);
    for (i = 1; i < end_y - 1; i++)
	one_y_lab(i);

}				/* end of xy_coor		 */

one_x_lab(i)			/* print one x label	 */
    int       i;
{
    char      label[30];
    Font_string_dims dims;
    int       w0;

    int       j;
    int       xpos, x0, x1;

    xpos = x_pos[i];
    if (pix_format != PFFLOAT)
	sprintf(label, "%d", x_label[i]);
    else
	sprintf(label, "%.2f", fx_label[i]);

    xv_get(font_small, FONT_STRING_DIMS, label, &dims);
    /*
     * printf("in repaint.., string-width=%d  string-height=%d", dims.width,
     * dims.height );
     */
    w0 = dims.width / 2;
    x0 = xpos - w0;
    x1 = xpos + w0;
    for (j = 0; j < end_xc; j++)
	if ((x_cover_l[j] <= x0 && x0 <= x_cover_r[j]) ||
	    (x_cover_l[j] <= x1 && x1 <= x_cover_r[j]))
	    break;

    if (j < end_xc)
	return;

    x_cover_l[end_xc] = x0;
    x_cover_r[end_xc] = x1;
    end_xc++;
    XDrawString(display, xid, gc, x0, (int) (4 * TXTHT + HGT - 1),
		label, strlen(label));
}

one_y_lab(i)			/* print one y label	 */
    int       i;
{
    char      label[30];
    Font_string_dims dims;
    int       h0;

    int       j;
    int       ypos, y0, y1;

    ypos = y_pos[i];

    sprintf(label, "%d", y_label[i]);
    xv_get(font_small, FONT_STRING_DIMS, label, &dims);

    /*
     * printf("in repaint.., string-width=%d  string-height=%d", dims.width,
     * dims.height );
     */

    h0 = dims.height / 2;
    y0 = ypos - h0;
    y1 = ypos + h0;
    for (j = 0; j < end_yc; j++)
	if ((y_cover_s[j] <= y0 && y0 <= y_cover_g[j]) ||
	    (y_cover_s[j] <= y1 && y1 <= y_cover_g[j]))
	    break;

    if (j < end_yc)
	return;

    y_cover_s[end_yc] = y0;
    y_cover_g[end_yc] = y1;
    end_yc++;
    XDrawString(display, xid, gc, 2 * TXTWD + range + 5, ypos + 4,
		label, strlen(label));
}

VerDashLine(x, y1, y2)
    int       x, y1, y2;
{
    int       m;
    int       i;

    XPoint   *q;

    m = (y2 - y1) / 2;

    q = Calloc(m, XPoint);

    q[0].x = x;
    q[0].y = y1;
    for (i = 1; i < m; i++) {
	q[i].x = 0;
	q[i].y = 2;
    }

    XDrawPoints(display, xid, gc, q, m, CoordModePrevious);
}

HorDashLine(x1, x2, y)
    int       x1, x2, y;
{
    int       m;
    int       i;

    XPoint   *q;

    m = (x2 - x1) / 2;

    q = Calloc(m, XPoint);

    q[0].x = x1;
    q[0].y = y;
    for (i = 1; i < m; i++) {
	q[i].x = 2;
	q[i].y = 0;
    }

    XDrawPoints(display, xid, gc, q, m, CoordModePrevious);
}

mouse_move(j)
    int       j;
{
    char      label[30];
    Font_string_dims dims;
    int       w0;
    int       x;

    if (move_j >= 0)
	move_rcv();
    move_j = j;

    x = 2 * TXTWD + move_j;
    XSetForeground(display, gc, color_Blue);	/* blue	 */
    sprintf(label, "%d", histo_graph[j]);
    xv_get(font_small, FONT_STRING_DIMS, label, &dims);
    w0 = dims.width / 2;
    XDrawString(display, xid, gc, x - w0, 3 * TXTHT - 6, label, strlen(label));

    XSetForeground(display, gc, color_Red);	/* red	 */
    VerDashLine(x, 3 * TXTHT, (int) (4 * TXTHT + HGT));

    draw_pv(j, x, (int) (5 * TXTHT + HGT - 6));

}				/* end of mouse_move		 */

move_rcv()
{
    int       x, y1, y2;
    int       y;

    x = 2 * TXTWD + move_j;
    XSetForeground(display, gc, color_White);
    XDrawLine(display, xid, gc, x, 3 * TXTHT, x, (int) (4 * TXTHT + HGT));
    for (y = 4 * TXTHT + HGT; y <= 6 * TXTHT + HGT; y++)
	XDrawLine(display, xid, gc, 0, y, (int) (5 * TXTWD + range - 1), y);
    for (y = 2 * TXTHT; y < 3 * TXTHT; y++)
	XDrawLine(display, xid, gc, 0, y, (int) (5 * TXTWD + range), y);

    if (histo_graph[move_j] > 0) {
	XSetForeground(display, gc, color_Black);
	y1 = 3 * TXTHT + HGT;
	y2 = 3 * TXTHT + HGT - (histo_graph[move_j] - Bottom) * y_unit;
	XDrawLine(display, xid, gc, x, y1, x, y2);
    }
    draw_coor();		/* draw coordinates	 */
    draw_vertibar();		/* draw vertical bars	 */
    draw_horizbar();		/* draw horizontal bar     */
}

pri_hint()
{				/* print hint	 */
    int       i;
    int       x, y;
    XGCValues gc_val;
    char      label[40];
    int       b;		/* base index of hint_s[]	 */

/*
 for (i=0; i<7; i++)
   printf("%s  str_len=%d\n", hint_s[i], strlen(hint_s[i]) );
*/

    gc_val.font = (Font) xv_get(font_bold, XV_XID);
    gc = XCreateGC(display, xid, GCForeground | GCFont, &gc_val);

    XSetForeground(display, gc, color_Blue);	/* blue	 */
    XDrawString(display, xid, gc, TXTWD, TXTHT - 6, "Count", 5);

    XSetForeground(display, gc, color_Red);
    XDrawString(display, xid, gc, TXTWD, CV_HT - 7, "pixel value", 11);

    XSetForeground(display, gc, color_Black);	/* black	 */

    b = 13;
    sprintf(hint_s[b], "On the current graph -------");

    if (pix_format == PFFLOAT)
	sprintf(hint_s[b + 1], " Left pixel value = %.2f", fhisto_va[0]);
    else
	sprintf(hint_s[b + 1], " Left pixel value = %d", histo_va[0]);

    if (pix_format == PFFLOAT)
	sprintf(hint_s[b + 2], " Right pixel value = %.2f",
		fhisto_va[range + 1] - .01);
    else
	sprintf(hint_s[b + 2], " Right pixel value = %d",
		histo_va[range + 1] - 1);

    sprintf(hint_s[b + 3], " min count = %d", Bottom);
    sprintf(hint_s[b + 4], " Max count = %d", Top);

    sprintf(hint_s[b + 5], "");

    sprintf(hint_s[b + 6], " # of different pixel values :");
    sprintf(hint_s[b + 7], "    %d", cur_nofdv);

    sprintf(hint_s[b + 8], "");

    sprintf(hint_s[b + 9], " # of pixels : %d", cur_size);
    /* # of pixels in current graph */
    sprintf(hint_s[b + 10], " distribution : %.2f %%",
	    (float) cur_size * 100. / (float) size);

    x = 5 * TXTWD + range;
    for (i = 0; i <= b + 10; i++) {
	y = i * TXTHT + 12;
	XDrawString(display, xid, gc, x, y, hint_s[i], strlen(hint_s[i]));
    }

    gc_val.font = (Font) xv_get(font_small, XV_XID);
    gc = XCreateGC(display, xid, GCForeground | GCBackground, &gc_val);

    XDrawLine(display, xid, gc, x, b * TXTHT + 16, x, CV_HT - 8);

}				/* end of pri_hint	 */
