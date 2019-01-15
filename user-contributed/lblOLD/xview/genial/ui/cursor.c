/*
 * cursor.c -- routines to set / reset the cursors of the UI
 *
 */

/* NOTE: need to fix this to find ALL open windows and change cursor
   ( or maybe there is a way just do some parent window ??)  */

#include <stdio.h>
#include "display.h"
#include "ui.h"

/* set the cursor to busy */
set_watch_cursor()
{
  int cshape=XC_watch;
  Cursor cursor;

  cursor=XCreateFontCursor(display, cshape);
  XDefineCursor(display, xv_get(canvas_paint_window(file_win->controls1),
				XV_XID), cursor);  
  XDefineCursor(display, xv_get(canvas_paint_window(base_win->controls1),
				XV_XID), cursor);  
  XFlush(display);
}

/*set cursor back to normal */
unset_watch_cursor()
{
  int cshape=XC_top_left_arrow;
  Cursor cursor;

  cursor=XCreateFontCursor(display, cshape);
  XDefineCursor(display, xv_get(canvas_paint_window(file_win->controls1),
				XV_XID), cursor);  
  XDefineCursor(display, xv_get(canvas_paint_window(base_win->controls1),
				XV_XID), cursor);  
  XFlush(display);
}

