/* cursor.c - cursor functions. */


#include <X11/Intrinsic.h>
#include <X11/cursorfont.h>
#include <Xol/OpenLook.h>

#include <xdisp.h>


/*******************************
 * create_cursor()
 *******************************/

/* create a cursor from bitmap information and return it */

Cursor create_cursor(bits,w,h,x_hot,y_hot,mbits,mw,mh)
  char 		*bits;			/* bitmap */
  int		w,h,x_hot,y_hot;	/* bitmap width,height,hotspot */
  char		*mbits;			/* mask bitmap */
  int		mw,mh;			/* mask width,height */
{
    XColor fg_color,bg_color;
    Cursor cursor;

    Pixmap cursor_pixmap = XCreateBitmapFromData(dpy,rws,bits,w,h);
    Pixmap mask_pixmap = XCreateBitmapFromData(dpy,rws,mbits,mw,mh);

    fg_color.pixel = black_pixel;
    fg_color.flags = DoRed | DoGreen | DoBlue;
    bg_color.pixel = white_pixel;
    bg_color.flags = DoRed | DoGreen | DoBlue;
			
    XQueryColor(dpy,app_colormap,&fg_color);
    XQueryColor(dpy,app_colormap,&bg_color);

    cursor = XCreatePixmapCursor(dpy,
				 cursor_pixmap,
				 mask_pixmap,
				 fg_color,
				 bg_color,
				 x_hot,
				 y_hot);

    XFreePixmap(dpy,cursor_pixmap);
    XFreePixmap(dpy,mask_pixmap);

    return(cursor);
 
}


/*****************************************
 * grab_pointer()
 *****************************************/

void grab_pointer(cursor)
  Cursor	cursor;
{
    XGrabPointer(dpy,rws,True,0,GrabModeAsync,GrabModeAsync,
			None,cursor,CurrentTime);
}
		


/*****************************************
 * ungrab_pointer()
 *****************************************/

void ungrab_pointer()
{
    XUngrabPointer(dpy,CurrentTime);
}



/*****************************************
 * set_cursor()
 *****************************************/

void set_cursor(w,curs)
  Widget w;
  Cursor curs;
{
    XDefineCursor(dpy,XtWindow(w),curs);
}
		



