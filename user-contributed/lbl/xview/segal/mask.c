
/*  mask.c       Brian Tierney, LBL       8/90
 *    for use with segal
 *
 *  These routines are used for automatic mask computation
 *     (threshold and polygon mask)
 */

#include "common.h"

#include "display_control.h"
#include "mask_control.h"
#include "pixedit.h"

extern mask_control_pop_polygon_objects *mask_control_pop_polygon;

#define Realloc(x,y,z) (z *)realloc((char *)x,(unsigned)(y * sizeof(z)))

#define MAX_POINTS 100
#define FVAL  10		/* value in buffer which indicates reached by
				 * fill */
#define PLYVAL 20		/* value in buffer which indicates edge of
				 * polygon */
#define CROSS_SIZE 10

typedef struct cross {
    int       x, y;		/* center of cross */
    int       l1x1, l1y1, l1x2, l1y2;	/* endpoints of line 1 */
    int       l2x1, l2y1, l2x2, l2y2;	/* endpoints of line 2 */
}         CROSS;

CROSS     cross_list[MAX_POINTS];
static int num_cross = 0;

u_char  **poly_fill_buf;

typedef struct s_item {
    short     i, j;
}         STACK_ITEM;

STACK_ITEM *stack;

int       sp;			/* global stack pointer */
int       stack_size;
#define STACK_CHUNK 10000

void      mask_op_proc();

/**************************************************************/
void
cross_proc(event)
    Event    *event;
{
    void      move_cross(), add_cross();
    static int cnum;
    int       x, y;

    x = event_x(event);
    y = event_y(event);

    if (event_id(event) == LOC_DRAG) {
	if (x - CROSS_SIZE < cross_list[cnum].x &&
	    x + CROSS_SIZE > cross_list[cnum].x &&
	    y - CROSS_SIZE < cross_list[cnum].y &&
	    y + CROSS_SIZE > cross_list[cnum].y)
	    move_cross(cnum, x, y);
	return;
    }
    if (cnum = cross_nearby(x, y))
	move_cross(cnum, x, y);
    else {
	if (num_cross >= MAX_POINTS - 1) {
	    fprintf(stderr, "Error: Maximum number of polygon vertices exceeded. \n");
	    return;
	}
	add_cross(num_cross, x, y);
	num_cross++;
    }
}

/**************************************************************/
void
add_cross(cnum, x, y)
    int       cnum, x, y;
{
    int       nx1, nx2, ny1, ny2;
    void      draw_cross();

    cross_list[cnum].x = x;
    cross_list[cnum].y = y;

    nx1 = x - CROSS_SIZE;
    nx2 = x + CROSS_SIZE;
    ny1 = y - CROSS_SIZE;
    ny2 = y + CROSS_SIZE;
    if (nx1 < 0)
	nx1 = 0;
    if (ny1 < 0)
	ny1 = 0;
    if (nx2 > segal.cols - 1)
	nx2 = segal.cols - 1;
    if (ny2 > segal.rows - 1)
	ny2 = segal.rows - 1;

    cross_list[cnum].l1x1 = cross_list[cnum].l2x1 = nx1;
    cross_list[cnum].l1x2 = cross_list[cnum].l2x2 = nx2;
    cross_list[cnum].l1y1 = cross_list[cnum].l2y2 = ny1;
    cross_list[cnum].l1y2 = cross_list[cnum].l2y1 = ny2;

    draw_cross(cross_list[cnum]);
}

/**************************************************************/
void
draw_cross(cross)
    CROSS     cross;
{
    XSetForeground(display, gc, red_standout);

    XDrawLine(display, view_xid, gc, cross.l1x1, cross.l1y1,
	      cross.l1x2, cross.l1y2);

    XDrawLine(display, view_xid, gc, cross.l2x1, cross.l2y1,
	      cross.l2x2, cross.l2y2);
}

/**************************************************************/
int
cross_nearby(x, y)
    int       x, y;
{
    int       i;
    static int near = CROSS_SIZE + 5;

    for (i = 0; i < num_cross; i++) {
	if (x - near < cross_list[i].x &&
	    x + near > cross_list[i].x &&
	    y - near < cross_list[i].y &&
	    y + near > cross_list[i].y)
	    return (i);
    }
    return (0);
}

/**************************************************************/
void
move_cross(cnum, x, y)
    int       cnum, x, y;
{
    void      erase_cross();

    erase_cross(cross_list[cnum]);
    add_cross(cnum, x, y);
}

/**************************************************************/
void
erase_cross(cross)
    CROSS     cross;
{
    int       sx, sy, size;

    sx = cross.l1x1 - 3;
    sy = cross.l1y1 - 3;
    if (sx < 0)
	sx = 0;
    if (sy < 0)
	sy = 0;
    size = CROSS_SIZE * 2 + 5;

    /* redraw original image */
    if (segal.display_type == 0 && image != NULL)
	XPutImage(display, view_xid, gc, image, sx, sy,
		  sx, sy, size, size);
    if (segal.display_type == 1 && mask_image != NULL)
	XPutImage(display, view_xid, gc, mask_image, sx, sy,
		  sx, sy, size, size);
    if (segal.display_type == 2 && blend_image != NULL)
	XPutImage(display, view_xid, gc, blend_image, sx, sy,
		  sx, sy, size, size);
}

/****************************************************************/
void
mask_op_proc(value)
    int       value;
{
    void draw_filenames();
    register int i, j, size;
    register u_char *mptr;

    if (verbose)
	fprintf(stderr, "in mask_op_proc, value = %d \n", value);

    set_watch_cursor();
    size = segal.rows * segal.cols;
    mptr = work_buf[0];

	if(region.whole) switch(value) {
		case 0 : /* fill */
			memset((char *) mptr, 0, size);
		break;

		case 1 :
			memset((char *) mptr, PVAL, size);
		break;

		case 2 :
			for (i = 0; i < size; i++)
				if (mptr[i] == PVAL) mptr[i] = 0;
				else mptr[i] = PVAL;
		break;
	}

	else switch(value) {
		case 0 : /* fill */
			for(j = region.cry1; j <= region.cry2; j++)
			for(i = region.crx1; i <= region.crx2; i++)
				work_buf[j][i] = 0;
		break;
		case 1 : /* clear */ 
			for(j = region.cry1; j <= region.cry2; j++)
			for(i = region.crx1; i <= region.crx2; i++)
				work_buf[j][i] = PVAL;
		break;
		case 2 : /* invert */
			for(j = region.cry1; j <= region.cry2; j++)
			for(i = region.crx1; i <= region.crx2; i++)
				if(work_buf[j][i] == 0) work_buf[j][i] = PVAL;
				else work_buf[j][i] = 0;
		break;
	}

    map_image_to_lut(mptr, mask_image->data, size);

    blend(himage.data[0], work_buf[0], (u_char *) blend_image->data, size);

    image_repaint_proc();
    if ((int) xv_get(edit_win->win, XV_SHOW, NULL) == TRUE) {
	zoom();
	edit_repaint_proc();
    }
    segal.changed = 1;
    draw_filenames();
    unset_watch_cursor();
}

/****************************************************************/
void
poly_proc(item, value)
    Panel_item item;
    int       value;
{
    register int i, j;
    u_char  **alloc_2d_byte_array();

/* CHANGE - this needs to be changed, I think */
/*
    (void) xv_set(mask_control_pop_polygon->set_polyfunct, PANEL_VALUE, value, NULL);

    if (value == 1) {
	segal.poly_flag = 1;
	poly_fill_buf = alloc_2d_byte_array(segal.rows, segal.cols);
    } else {
	segal.poly_flag = 0;
	free((char *) poly_fill_buf);
    }
    image_repaint_proc();
*/
}

/****************************************************************/
void
show_poly_proc()
{
    void      draw_poly();
    register int i;

    image_repaint_proc();

    bcopy(work_buf[0], poly_fill_buf[0], segal.rows * segal.cols);

    /* redraw all crosses */
    for (i = 0; i < num_cross; i++)
	draw_cross(cross_list[i]);

    draw_poly();
}

/****************************************************************/
void
fill_proc()
{
    void      draw_filenames();
    void      flood_poly();
    int       size, obj_cnt = 0;
    register int i, j;

    set_watch_cursor();

    flood_poly();		/* do flood fill of polygon */

    size = segal.rows * segal.cols;

    for (i = 0; i < segal.rows; i++)
	for (j = 0; j < segal.cols; j++) {
	    if (poly_fill_buf[i][j] == FVAL || poly_fill_buf[i][j] == PLYVAL)
		work_buf[i][j] = 0;
	    else if (poly_fill_buf[i][j] == PVAL)
		obj_cnt++;
	}

    if (obj_cnt == 0) {  /* this needed if want to use polygon option on
				original image without a mask */
	fprintf(stderr, "No pixels found, setting mask to %d and trying again... \n", PVAL);
	memset((char *) work_buf[0], PVAL, size);
	show_poly_proc();
	fill_proc();
    }

    map_image_to_lut(work_buf[0], mask_image->data, size);

    blend(himage.data[0], work_buf[0], (u_char *) blend_image->data, size);

    segal.display_type = 2;
    (void) xv_set(display_control_win->display_type, PANEL_VALUE, 2, NULL);

    image_repaint_proc();

    if ((int) xv_get(edit_win->win, XV_SHOW, NULL) == TRUE) {
	zoom();
	edit_repaint_proc();
    }
    segal.changed = 1;		/* mask modified flag */
    draw_filenames();
    num_cross = 0;		/* reset cross list */

    unset_watch_cursor();
}

/****************************************************************/
void
clear_proc()
{
    register int i, j;

    image_repaint_proc();
    num_cross = 0;

    bzero((char *) poly_fill_buf[0], segal.rows * segal.cols);
}

/****************************************************************/
void
draw_poly()
{
    int       i;
    void      line_fill();

    for (i = 0; i < num_cross; i++) {
	if (i == num_cross - 1)	/* last point: close the polygon */
	    line_fill(poly_fill_buf, cross_list[i].x, cross_list[i].y,
		      cross_list[0].x, cross_list[0].y);
	else
	    line_fill(poly_fill_buf, cross_list[i].x, cross_list[i].y,
		      cross_list[i + 1].x, cross_list[i + 1].y);
    }
}

/****************************************************************/
void
flood_poly()
{
    stack_size = STACK_CHUNK;
    alloc_stack(stack_size);

    fill(0, 0);

    free((char *) stack);
}

/****************************************************************/

fill(x, y)
    int       x, y;
{
    register int xx, yy;

    sp = 0;			/* initialize stack pointer */
    push(-1, -1);		/* null stack */

    do {
start:
	poly_fill_buf[x][y] = FVAL;	/* mark poly_fill_buf */

	xx = x + 1;
	if (xx < segal.rows && poly_fill_buf[xx][y] != PLYVAL &&
	    poly_fill_buf[xx][y] != FVAL) {
	    push(x, y);
	    x++;
	    goto start;
	}
	xx = x - 1;
	if (xx > 0 && poly_fill_buf[xx][y] != PLYVAL && poly_fill_buf[xx][y] != FVAL) {
	    push(x, y);
	    x--;
	    goto start;
	}
	yy = y - 1;
	if (yy > 0 && poly_fill_buf[x][yy] != PLYVAL && poly_fill_buf[x][yy] != FVAL) {
	    push(x, y);
	    y--;
	    goto start;
	}
	yy = y + 1;
	if (yy < segal.cols && poly_fill_buf[x][yy] != PLYVAL &&
	    poly_fill_buf[x][yy] != FVAL) {
	    push(x, y);
	    y++;
	    goto start;
	}
	pop(&x, &y);

    } while (x >= 0);		/* neg x indicates empty stack */
    if (sp != 0)
	fprintf(stderr, "Error: stack not empty \n");

    return;
}

/***************************************************************/
push(i, j)
    int       i, j;
{
    sp++;

    if (sp >= stack_size) {
	stack_size += STACK_CHUNK;
#ifdef DEBUG
	fprintf(stderr, " increasing stack size to %d.. \n", stack_size);
#endif
	if ((stack = Realloc(stack, stack_size, STACK_ITEM)) == NULL)
	    perror("realloc");
    }
    stack[sp].i = i;
    stack[sp].j = j;
}

/***************************************************************/
pop(i, j)
    int      *i, *j;
{
    *i = stack[sp].i;
    *j = stack[sp].j;
    sp--;
}

/***************************************************************/
alloc_stack(st_size)		/* allocation stack for non-recursive
				 * flood-fill alg */
    int       st_size;
{
    if ((stack = Calloc(st_size, STACK_ITEM)) == NULL)
	perror("calloc: stack");
}

/********************************************************************/
void
line_fill(buf, x1, y1, x2, y2)	/* Bresenhams's scan conversion algorithm */
    u_char  **buf;
    int       x1, y1, x2, y2;

/* this code adapted from:   Digital Line Drawing
 *                           by Paul Heckbert
 * from "Graphics Gems", Academic Press, 1990
 */
{
    int       d, x, y, ax, ay, sx, sy, dx, dy;

    /* absolute value of a */
#ifndef ABS
#define ABS(a)          (((a)<0) ? -(a) : (a))
#endif

    /* take binary sign of a, either -1, or 1 if >= 0 */
#define SGN(a)          (((a)<0) ? -1 : 1)

    if (x1 == x2 && y1 == y2) {
	return;
    }
    dx = x2 - x1;
    ax = ABS(dx) << 1;
    sx = SGN(dx);

    dy = y2 - y1;
    ay = ABS(dy) << 1;
    sy = SGN(dy);

    x = x1;
    y = y1;
    if (ax > ay) {		/* x dominant */
	d = ay - (ax >> 1);
	for (;;) {
	    buf[y][x] = PLYVAL;	/* label buf with PLYVAL */
	    XDrawPoint(display, view_xid, gc, x, y);
	    if (x == x2)
		return;
	    if (d >= 0) {
		y += sy;
		d -= ax;
	    }
	    x += sx;
	    d += ay;
	}
    } else {			/* y dominant */
	d = ax - (ay >> 1);
	for (;;) {
	    buf[y][x] = PLYVAL;	/* label buf with PLYVAL */
	    XDrawPoint(display, view_xid, gc, x, y);
	    if (y == y2)
		return;
	    if (d >= 0) {
		x += sx;
		d -= ay;
	    }
	    y += sy;
	    d += ax;
	}
    }
}
