/*
 *  cursor.c   routines for handling cursors   -Brian Tierney, LBL
 *   for use with segal
 *
 *   these routines handle the creation of various sized square and
 *       round paint and erase cursors.
 */

#include "common.h"

#include "pixedit.h"
#include "view.h"

SEGAL_CURSOR my_cursor[MAX_CURSORS];
Xv_singlecolor fg;		/* color for the cursor */

/*************************************************/
set_watch_cursor()
{
    XDefineCursor(display, view_xid, watch_cursor);
    XDefineCursor(display, edit_xid, watch_cursor);
    XDefineCursor(display, edit_control_xid, watch_cursor);
    XDefineCursor(display, segal_control_xid, watch_cursor);
    XFlush(display);
}

/***********************************************/
unset_watch_cursor()
{
    void      change_cursor_proc();

    XUndefineCursor(display, view_xid);
    XUndefineCursor(display, edit_xid);
    XUndefineCursor(display, edit_control_xid);
    XUndefineCursor(display, segal_control_xid);

    change_cursor_proc();
}

/***************************************************************/
void
change_cursor_proc()
{
    int       bs, mag, rcurs;

    bs = ((int) xv_get(edit_win->brush_size, PANEL_VALUE, NULL)) + 1;
    mag = ((int) xv_get(edit_win->zoom_mag, PANEL_VALUE, NULL)) + 1;
    rcurs = ((int) xv_get(edit_win->brush_type, PANEL_VALUE, NULL));

    if ((int) xv_get(edit_win->mask_brush_mode, PANEL_VALUE, NULL) == 0) {
	if (rcurs)
	    xv_set(canvas_paint_window(edit_win->canvas),
		   WIN_CURSOR, my_cursor[bs * mag].paint, NULL);
	else
	    xv_set(canvas_paint_window(edit_win->canvas),
		   WIN_CURSOR, my_cursor[bs * mag].sq_paint, NULL);
    } else {
	if (rcurs)
	    xv_set(canvas_paint_window(edit_win->canvas),
		   WIN_CURSOR, my_cursor[bs * mag].erase, NULL);
	else
	    xv_set(canvas_paint_window(edit_win->canvas),
		   WIN_CURSOR, my_cursor[bs * mag].sq_erase, NULL);
    }
    XFlush(display);
}

/******************************************************/
void
create_cursors()
{
    int       i, cursor_num, cur_size, size, mag;
    void      make_box_cursor(), make_round_cursor();
    void      make_paint_image_matrix();

    if (verbose)
	fprintf(stderr, " creating edit cursors... \n");

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

    for (i = 0; i < MAX_CURSORS; i++)	/* initialize */
	my_cursor[i].size = 0;

    for (cur_size = 1; cur_size <= 7; cur_size++) {
	for (mag = 1; mag <= 6; mag++) {

	    cursor_num = cur_size * mag;
	    if (cursor_num > MAX_CURSORS) {
		fprintf(stderr, " error creating cursors \n");
		return;
	    }
	    if (cur_size == 7)	/* index # 7 is 20x20 cursor */
		size = mag * 20;
	    else
		size = mag * cur_size;

	    if (my_cursor[cursor_num].size == 0) {	/* else already done */
		make_box_cursor(cursor_num, size);
		make_round_cursor(cursor_num, size);
		make_paint_image_matrix(cursor_num, size / mag);
	    }
	}
    }
}

/******************************************************/
void
make_box_cursor(cnum, box_size)
    int       cnum, box_size;
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

    my_cursor[cnum].sq_paint = xv_create(XV_NULL, CURSOR,
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

    my_cursor[cnum].sq_erase = xv_create(XV_NULL, CURSOR,
					 CURSOR_IMAGE, serv_image,
					 CURSOR_XHOT, hot,
					 CURSOR_YHOT, hot,
					 CURSOR_FOREGROUND_COLOR, &fg,
					 NULL);
    XFreeGC(display, cgc);
}

/******************************************************/
void
make_round_cursor(cnum, cir_size)
    int       cnum, cir_size;
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
	my_cursor[cnum].size = 24;
    else
	my_cursor[cnum].size = cir_size + 4;
    my_cursor[cnum].radius = cir_size / 2;

#ifdef CDEBUG
    fprintf(stderr, "cnum: %d,   size: %d, diam: %d, rad: %d \n",
	    cnum, my_cursor[cnum].size, cir_size,
	    my_cursor[cnum].radius);
#endif

    serv_image = (Server_image) xv_create(XV_NULL, SERVER_IMAGE,
					  XV_WIDTH, my_cursor[cnum].size,
					  XV_HEIGHT, my_cursor[cnum].size,
					  NULL);

    pixmap = (Pixmap) xv_get(serv_image, XV_XID, NULL);

    /* make paint cursor */
    gcvalues.foreground = 0;
    gcvalues.background = 1;
    cgc = XCreateGC(display, pixmap,
		    GCForeground | GCBackground, &gcvalues);
    XFillRectangle(display, pixmap, cgc, 0, 0,
		   my_cursor[cnum].size, my_cursor[cnum].size);

    gcvalues.foreground = 1;
    gcvalues.background = 0;
    XChangeGC(display, cgc, GCForeground | GCBackground, &gcvalues);

    hot = my_cursor[cnum].size / 2;	/* center of circle */
    start = hot - my_cursor[cnum].radius;
    my_cursor[cnum].corner = start;

    /* draw cursor shape into pixmap */
    XDrawLine(display, pixmap, cgc, 1, 1, hot, hot);
    XDrawPoint(display, pixmap, cgc, start, start);
    XDrawArc(display, pixmap, cgc, start, start,
	     cir_size, cir_size,
	     0, 360 * 64);

    my_cursor[cnum].paint = xv_create(XV_NULL, CURSOR,
				      CURSOR_IMAGE, serv_image,
				      CURSOR_XHOT, hot,
				      CURSOR_YHOT, hot,
				      CURSOR_FOREGROUND_COLOR, &fg,
				      NULL);

    /* make erase cursor */
    gcvalues.foreground = 0;
    gcvalues.background = 1;
    XChangeGC(display, cgc, GCForeground | GCBackground, &gcvalues);
    XFillRectangle(display, pixmap, cgc, 0, 0,
		   my_cursor[cnum].size, my_cursor[cnum].size);

    gcvalues.foreground = 1;
    gcvalues.background = 0;
    XChangeGC(display, cgc, GCForeground | GCBackground, &gcvalues);

    /* draw cursor shape into pixmap */
    XDrawLine(display, pixmap, cgc, 1, my_cursor[cnum].size - 1,
	      hot, hot);
    XDrawArc(display, pixmap, cgc, start, start,
	     cir_size, cir_size, 0, 360 * 64);

    /* draw X thru circle for erase cursor */
    val = (cir_size / 2.) * (cir_size / 2.);
    s1 = hot - (int) sqrt((double) (val / 2.));
    s2 = hot + (int) sqrt((double) (val / 2.));

    XDrawLine(display, pixmap, cgc, s1, s1, s2, s2);
    XDrawLine(display, pixmap, cgc, s1, s2, s2, s1);

    my_cursor[cnum].erase = xv_create(XV_NULL, CURSOR,
				      CURSOR_IMAGE, serv_image,
				      CURSOR_XHOT, hot,
				      CURSOR_YHOT, hot,
				      CURSOR_FOREGROUND_COLOR, &fg,
				      NULL);

    /* build paint mask matrix */
    my_cursor[cnum].paint_mask_matrix =
	alloc_2d_byte_array(my_cursor[cnum].size, my_cursor[cnum].size);

    build_paint_mask_matrix(my_cursor[cnum].radius, (my_cursor[cnum].size / 2),
		     my_cursor[cnum].paint_mask_matrix);

#ifdef CDEBUG
    if (my_cursor[cnum].size < 40)
	show_mask(my_cursor[cnum].paint_mask_matrix, my_cursor[cnum].size);
#endif

    XFreeGC(display, cgc);
}

/**********************************************************/
old_create_cursor()
{				/* not used */
    Cursor    p_cursor, e_cursor;	/* paint and erase cursor */

    p_cursor = XCreateFontCursor(display, XC_pencil);
    e_cursor = XCreateFontCursor(display, XC_draft_small);

    if ((int) xv_get(edit_win->mask_brush_mode, PANEL_VALUE, NULL) == 0)
	XDefineCursor(display, edit_xid, p_cursor);
    else
	XDefineCursor(display, edit_xid, e_cursor);
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
	mask[i][y1] = 1;
	mask[i][y2] = 1;
    }
    for (i = y2; i <= y1; i++) {
	mask[i][x1] = 1;
	mask[i][x2] = 1;
    }
}

/*****************************************/
void
make_paint_image_matrix(num, radius)
int num, radius;
{
	int i, j;
	u_char **alloc_2d_byte_array();

	my_cursor[num].paint_image_matrix = alloc_2d_byte_array(2 * radius + 1, 2 * radius + 1);

	for(j = -1*radius; j < radius; j++)
	for(i = -1*radius; i < radius; i++)
		my_cursor[num].paint_image_matrix[radius + j][radius + i] = radius - sqrt((i - radius)*(i - radius) + (j - radius)*(j - radius));
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
