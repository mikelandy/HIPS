/* sun_primits.c
 * Max Rible
 *
 * Graphics primitives that work under Sunview.
 */

#include "hipstool.h"

#ifdef SUNTOOLS
#ifndef MACROIZED
/* Feed it 3 arrays of 256 unsigned chars */
void
colormap256(winfo, arr, gee, bee)
     Windowinfo *winfo;
     unsigned char *arr, *gee, *bee;
{
    pw_setcmsname(winfo->pw, Progname);
    pw_putcolormap(winfo->pw, 0, 256, arr, gee, bee);
}

void
put_pix(winfo, x, y, color)
     Windowinfo *winfo;
     int x, y, color;
{
    pw_put(winfo->pw, x, y, color);
}

int
get_pix(winfo, x, y)
     Windowinfo *winfo;
     int x, y;
{
    return(pw_get(winfo->pw, x, y));
}

void
line(winfo, x_1, y_1, x_2, y_2, color)
    Windowinfo *winfo;
    int x_1, y_1, x_2, y_2, color;
{
    if(color == XOR_COLOR)
	pw_vector(winfo->pw, x_1, y_1, x_2, y_2, PIX_SRC ^ PIX_DST, 0xFF);
    else
	pw_vector(winfo->pw, x_1, y_1, x_2, y_2, PIX_SRC, color);
}

void
text(winfo, x, y, string, color)
     Windowinfo *winfo;
     int x, y, color;
     char *string;
{
    pw_ttext(winfo->pw, x, y, PIX_SRC | PIX_COLOR(color), (Pixfont *) NULL,
	     string);
}

void
wipe(winfo, color)
     Windowinfo *winfo;
     int color;
{
    pw_writebackground(winfo->pw, 0, 0, winfo->width, winfo->height,
		       PIX_SRC | PIX_COLOR(color));
}

char *
getstring(item)
     Panel_item item;
{
    return((char *) panel_get_value(item));
}

void
putstring(item, value)
     Panel_item item;
     char *value;
{
    panel_set_value(item, value);
}     
#endif

void
refresh(winfo, command)
     Windowinfo *winfo;
     Command *command;
{
    int i;

    pw_rop(winfo->pw, 0, 0, winfo->width, winfo->height, PIX_SRC, 
	   winfo->pr, 0, 0);
    if(command != NULL) {
	for(i = 0; i < command->func.len; i++)
	    (*priminfo[command->func.primit[i].breed].display)
		(command->func.primit + i, DRAW_IN_PIXWIN);
    }
}

void
batching(winfo, status)
     Windowinfo *winfo;
     int status;
{
    if(status == ON) {
	pw_batch_on(winfo->pw);
    } else {
	pw_batch_off(winfo->pw);
    }
}

void
newindow(winfo, width, height, posx, posy, label)
     Windowinfo *winfo;
     int width, height, posx, posy;
     char *label;
{
    winfo->width = width;
    winfo->height = height;

    winfo->frame = 
	window_create(io.control, FRAME,
		      FRAME_LABEL, label,
		      FRAME_SHOW_LABEL, TRUE,
		      FRAME_EMBOLDEN_LABEL, TRUE,
#ifdef SUNVIEW_4_0
		      FRAME_SHOW_SHADOW, FALSE,
#endif
		      WIN_X, posx,
		      WIN_Y, posy,
		      WIN_SHOW, TRUE,
		      WIN_HEIGHT, Win_height(winfo) + MARG_Y,
		      WIN_WIDTH, Win_width(winfo) + MARG_X,
		      0);

    winfo->canvas = 
	window_create(winfo->frame, CANVAS,
		      CANVAS_WIDTH, width,
		      CANVAS_HEIGHT, height,
		      CANVAS_AUTO_SHRINK, FALSE,
		      CANVAS_AUTO_EXPAND, FALSE,
		      CANVAS_FIXED_IMAGE, FALSE,
		      CANVAS_RETAINED, TRUE,
		      WIN_X, 0,
		      WIN_Y, 0,
		      0);

    winfo->pw = canvas_pixwin(winfo->canvas);

    colormap256(winfo, red, green, blue);
}

void
changewindow(winfo, dimx, dimy, label)
     Windowinfo *winfo;
     int dimx, dimy;
     char *label;
{
    if(label != NULL)
	window_set(winfo->frame,
		   FRAME_LABEL, label,
		   0);
    if(dimx >= 0) {
	window_set(winfo->frame,
		   WIN_WIDTH, dimx + MARG_X,
		   0);
    }
    if(dimy >= 0) {
	window_set(winfo->frame,
		   WIN_HEIGHT, dimy + MARG_Y,
		   0);
    }

    winfo->canvas = 
	window_create(winfo->frame, CANVAS,
		      CANVAS_WIDTH, winfo->width,
		      CANVAS_HEIGHT, winfo->height,
		      CANVAS_AUTO_SHRINK, FALSE,
		      CANVAS_AUTO_EXPAND, FALSE,
		      CANVAS_FIXED_IMAGE, FALSE,
		      CANVAS_RETAINED, TRUE,
		      WIN_X, 0,
		      WIN_Y, 0,
		      0);

    winfo->pw = canvas_pixwin(winfo->canvas);

    colormap256(winfo, red, green, blue);
}

void
killwindow(winfo)
     Windowinfo *winfo;
{
    if(winfo->vbar != NULL)
	scrollbar_destroy(winfo->vbar);
    if(winfo->hbar != NULL)
	scrollbar_destroy(winfo->hbar);
    if(winfo->pr)
	pr_destroy(winfo->pr);
    window_set(winfo->frame, FRAME_NO_CONFIRM, TRUE, 0);
    window_destroy(winfo->frame);

    winfo->frame = (Frame) NULL; 
    winfo->canvas = (Canvas) NULL;
    winfo->pw = (Pixwin *) NULL;
    winfo->pr = (Pixrect *) NULL;
    winfo->vbar = winfo->hbar = NULL;
    winfo->width = winfo->height = -1;
}

void
init_cursor(winfo)
     Windowinfo *winfo;
{
    Cursor cursor;

    cursor = window_get(winfo->canvas, WIN_CURSOR);
    cursor_set(cursor, CURSOR_OP, PIX_SRC ^ PIX_DST, 0);
    window_set(winfo->canvas, WIN_CURSOR, cursor, 0);
}

void
pixrefresh(which)
     FileInfo *which;
{
    pr_destroy(which->winfo.pr);
    switch(which->datasize) {
    case 1:
	which->winfo.pr = 
	    chars_to_pixrect(which->buf.chars, which->image,
			     which->winfo.width, which->winfo.height,
			     which->user_lut);
	break;
    case 2:
	which->winfo.pr = 
	    shorts_to_pixrect(which->buf.shorts, which->image,
			      which->winfo.width, which->winfo.height,
			      which->user_lut);
	break;
    case 4:
	which->winfo.pr = 
	    longs_to_pixrect(which->buf.longs, which->image,
			     which->winfo.width, which->winfo.height,
			     which->user_lut);
	break;
    }
    refresh(&which->winfo, cur_func);
}
#endif				/* SUNTOOLS */
