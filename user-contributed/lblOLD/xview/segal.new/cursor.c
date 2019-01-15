/*
 *  cursor.c   routines for handling cursors   -Brian Tierney, LBL
 *   for use with segal
 *
 *   these routines handle the creation of various sized square and
 *       round paint and erase cursors.
 */

#include "common.h"

Xv_singlecolor fg;		/* color for the cursor */

/*************************************************/
void
set_watch_cursor()
{
	int i;

	For_all_windows XDefineCursor(display, win[i].xid, watch_cursor);
	XFlush(display);
}

/***********************************************/
void
unset_watch_cursor()
{
	void change_cursor_proc();

	int i;

	For_all_windows XUndefineCursor(display, win[i].xid);
	change_cursor_proc();
}

/***************************************************************/
void
change_cursor_proc()
{
	if(brush.mask_affect == MASK_PAINT) {
		if(brush.shape == BRUSH_ROUND)
			xv_set(canvas_paint_window(Paint_win_paint->canvas),
				WIN_CURSOR, my_cursor[brush.size][win[WIN_PAINT].zoom_mag-1].paint,
				NULL);
		else
			xv_set(canvas_paint_window(Paint_win_paint->canvas),
				WIN_CURSOR, my_cursor[brush.size][win[WIN_PAINT].zoom_mag-1].sq_paint,
				NULL);
	}
	else {
		if(brush.shape == BRUSH_ROUND)
			xv_set(canvas_paint_window(Paint_win_paint->canvas),
				WIN_CURSOR, my_cursor[brush.size][win[WIN_PAINT].zoom_mag-1].erase,
				NULL);
		else
			xv_set(canvas_paint_window(Paint_win_paint->canvas),
				WIN_CURSOR, my_cursor[brush.size][win[WIN_PAINT].zoom_mag-1].sq_erase,
				NULL);
	}
	XFlush(display);
}

/******************************************************/
void
create_cursors()
{
	void make_box_cursor();
	void make_round_cursor();

	int i, bsize, mag, size;

	vprint_if" creating edit cursors... \n");

#define RED_CURSOR

#ifdef BLUE_CURSOR
    fg.green = 255, fg.blue = 255, fg.red = 0;	/* blue  cursors */
#endif
#ifdef YELLOW_CURSOR
    fg.green = 255, fg.blue = 0, fg.red = 255;	/* yellow  cursors */
#endif
#ifdef RED_CURSOR
    fg.green = 0, fg.blue = 0, fg.red = 255;	/* red  cursors */
#endif

	for (bsize = 0; bsize < NUM_BRUSH_SIZES; bsize++) /* 1..5, 10, 20 */
	for (mag = 0; mag < MAX_ZOOM_MAG; mag++) {
		if (bsize == 5) /* index # 5 is 10x10 cursor */
		size = (mag+1) * 10;
		else if (bsize == 6) /* index # 6 is 20x20 cursor */
		size = (mag+1) * 20;
		else
		size = (mag+1) * (bsize+1);

		make_box_cursor(bsize, mag, size);
		make_round_cursor(bsize, mag, size);
	}
}

/******************************************************/
void
make_box_cursor(bsize, mag, box_size)
int bsize, mag, box_size;
{
	Server_image serv_image;
	Pixmap    pixmap;
	GC        cgc;
	XGCValues gcvalues;
	int       size, hot;

	if (box_size <= 18)		/* min cursor size */
	size = 20;
	else
	size = box_size + 2;

	serv_image = (Server_image) xv_create(XV_NULL, SERVER_IMAGE,
					  XV_WIDTH, size,
					  XV_HEIGHT, size,
					  NULL);

	pixmap = (Pixmap) xv_get(serv_image, XV_XID, NULL);

	/* make paint cursor */
	gcvalues.foreground = 0;
	gcvalues.background = 1;
	cgc = XCreateGC(display, pixmap,
		    GCForeground | GCBackground, &gcvalues);
	XFillRectangle(display, pixmap, cgc, 0, 0, size, size);

	gcvalues.foreground = 1;
	gcvalues.background = 0;
	XChangeGC(display, cgc, GCForeground | GCBackground, &gcvalues);

	hot = (size - box_size) / 2;/* upper, left corner */

	/* draw paint cursor shape into pixmap */
	XDrawLine(display, pixmap, cgc, 1, 1, hot, hot);
	XDrawRectangle(display, pixmap, cgc, hot - 1, hot - 1,
		   box_size, box_size);

	my_cursor[bsize][mag].sq_paint = xv_create(XV_NULL, CURSOR,
					 CURSOR_IMAGE, serv_image,
					 CURSOR_XHOT, hot,
					 CURSOR_YHOT, hot,
					 CURSOR_FOREGROUND_COLOR, &fg,
					 NULL);

	/* make erase cursor */
	gcvalues.foreground = 0;
	gcvalues.background = 1;
	XChangeGC(display, cgc, GCForeground | GCBackground, &gcvalues);
	XFillRectangle(display, pixmap, cgc, 0, 0, size, size);

	gcvalues.foreground = 1;
	gcvalues.background = 0;
	XChangeGC(display, cgc, GCForeground | GCBackground, &gcvalues);

	/* draw cursor shape into pixmap */
	XDrawRectangle(display, pixmap, cgc, hot - 1, hot - 1,
		   box_size, box_size);
	/* draw X thru box (plus handle ) for erase cursor */
	XDrawLine(display, pixmap, cgc, 1, size - 3, hot + box_size - 1, hot - 1);
	XDrawLine(display, pixmap, cgc, hot - 1, hot - 1,
	      hot + box_size - 1, hot + box_size - 1);

	my_cursor[bsize][mag].sq_erase = xv_create(XV_NULL, CURSOR,
					 CURSOR_IMAGE, serv_image,
					 CURSOR_XHOT, hot,
					 CURSOR_YHOT, hot,
					 CURSOR_FOREGROUND_COLOR, &fg,
					 NULL);
	XFreeGC(display, cgc);
}

/******************************************************/
void
make_round_cursor(bsize, mag, cir_size)
int bsize, mag, cir_size;
{
	Server_image serv_image;
	Pixmap    pixmap;
	GC        cgc;
	XGCValues gcvalues;
	int       start, s1, s2, hot;
	float     val;
	u_char  **alloc_2d_byte_array();

/* #define CDEBUG */

	if (cir_size <= 20)		/* minimum size */
	my_cursor[bsize][mag].size = 24;
	else
	my_cursor[bsize][mag].size = cir_size + 4;
	my_cursor[bsize][mag].radius = cir_size / 2;

#ifdef CDEBUG
	fprintf(stderr, "bsize, mag: %d,   size: %d, diam: %d, rad: %d \n",
	    bsize, mag, my_cursor[bsize][mag].size, cir_size,
	    my_cursor[bsize][mag].radius);
#endif

	serv_image = (Server_image) xv_create(XV_NULL, SERVER_IMAGE,
		XV_WIDTH, my_cursor[bsize][mag].size,
		XV_HEIGHT, my_cursor[bsize][mag].size,
		NULL);

	pixmap = (Pixmap) xv_get(serv_image, XV_XID, NULL);

	/* make paint cursor */
	gcvalues.foreground = 0;
	gcvalues.background = 1;
	cgc = XCreateGC(display, pixmap,
		GCForeground | GCBackground, &gcvalues);

	/* clear server image */
	XFillRectangle(display, pixmap, cgc, 0, 0,
		my_cursor[bsize][mag].size, my_cursor[bsize][mag].size);

	gcvalues.foreground = 1;
	gcvalues.background = 0;
	XChangeGC(display, cgc, GCForeground | GCBackground, &gcvalues);

	hot = my_cursor[bsize][mag].size / 2;	/* center of circle */
	start = hot - my_cursor[bsize][mag].radius;

	/* upper left hand corner */
	my_cursor[bsize][mag].corner = start;

	/* draw cursor shape into pixmap */
	XDrawLine(display, pixmap, cgc, 1, 1, hot, hot);
	XDrawPoint(display, pixmap, cgc, start, start);
	XDrawArc(display, pixmap, cgc, start, start,
		cir_size, cir_size,
		0, 360 * 64);

	my_cursor[bsize][mag].paint = xv_create(XV_NULL, CURSOR,
		CURSOR_IMAGE, serv_image,
		CURSOR_XHOT, start,
		CURSOR_YHOT, start,
		CURSOR_FOREGROUND_COLOR, &fg,
		NULL);

	/* make erase cursor */
	gcvalues.foreground = 0;
	gcvalues.background = 1;
	XChangeGC(display, cgc, GCForeground | GCBackground, &gcvalues);
	XFillRectangle(display, pixmap, cgc, 0, 0,
		my_cursor[bsize][mag].size, my_cursor[bsize][mag].size);

	gcvalues.foreground = 1;
	gcvalues.background = 0;
	XChangeGC(display, cgc, GCForeground | GCBackground, &gcvalues);

	/* draw cursor shape into pixmap */
	XDrawLine(display, pixmap, cgc, 1, my_cursor[bsize][mag].size - 1,
		hot, hot);
	XDrawArc(display, pixmap, cgc, start, start,
		cir_size, cir_size, 0, 360 * 64);

	/* draw X thru circle for erase cursor */
	val = (cir_size / 2.) * (cir_size / 2.);
	s1 = hot - (int) sqrt((double) (val / 2.));
	s2 = hot + (int) sqrt((double) (val / 2.));

	XDrawLine(display, pixmap, cgc, s1, s1, s2, s2);
	XDrawLine(display, pixmap, cgc, s1, s2, s2, s1);

	my_cursor[bsize][mag].erase = xv_create(XV_NULL, CURSOR,
		CURSOR_IMAGE, serv_image,
		CURSOR_XHOT, start,
		CURSOR_YHOT, start,
		CURSOR_FOREGROUND_COLOR, &fg,
		NULL);

	/* build paint mask matrix */
	/* in SEGAL 3d, only one of these matrices is needed */
	if(mag == 0) {
		my_cursor[bsize][mag].paint_mask_matrix =
		alloc_2d_byte_array(my_cursor[bsize][mag].size,
			my_cursor[bsize][mag].size);

		build_paint_mask_matrix(my_cursor[bsize][mag].radius,
			(my_cursor[bsize][mag].size / 2),
			my_cursor[bsize][mag].paint_mask_matrix);

#ifdef CDEBUG
		if (my_cursor[bsize][mag].size < 40)
		show_mask(my_cursor[bsize][mag].paint_mask_matrix,
			my_cursor[bsize][mag].size);
#endif
	}

	XFreeGC(display, cgc);
}

/*****************************************/
build_paint_mask_matrix(radius, shift, mask)
    int       radius, shift;
    u_char  **mask;
 /*
  * this routine inspired by routine on page 445 of Foley & Van Dam which is
  * accredited to J. Michner
  */
{
    int       x, y, d;

    x = 0;
    y = radius;
    d = 3 - 2 * radius;

    while (x < y) {
	set_points(x, y, shift, mask);
	if (d < 0)
	    d = d + 4 * x + 6;
	else {
	    d = d + 4 * (x - y) + 10;
	    y--;
	}
	x++;
    }
    if (x == y)
	set_points(x, y, shift, mask);
}

/**************************************/
set_points(x, y, shift, mask)
    int       x, y, shift;
    u_char  **mask;
{
    int       x1, y1, x2, y2;
    register int i;

    /* shift origin */
    x1 = x + shift;
    x2 = -x + shift;
    y1 = y + shift;
    y2 = -y + shift;

    for (i = x2; i <= x1; i++) {
	mask[i][y1] = TRUE;
	mask[i][y2] = TRUE;
    }
    for (i = y2; i <= y1; i++) {
	mask[i][x1] = TRUE;
	mask[i][x2] = TRUE;
    }
}

/*************************************************/
#ifdef CDEBUG
show_mask(mask, size)		/* for debugging */
    u_char  **mask;
    int       size;
{
    int       x, y;

    fprintf(stderr, " \n\n mask size %d is: ", size);
    for (y = 0; y < size; y++) {
	fprintf(stderr, " \n");
	for (x = 0; x < size; x++) {
	    fprintf(stderr, "%d ", mask[y][x]);
	}
    }
    fprintf(stderr, "\n");
}

#endif
