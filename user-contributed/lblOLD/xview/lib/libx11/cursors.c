/*	CURSORS . C
#
% AUTHOR and Copyright:	see included header files
*/

#include "function.h"

#include "circle.bitmap"
#include "circle_mask.bitmap"

Cursor	circle_cursor, left_ptr_cursor, watch_cursor, cursor;

void
set_window_cursor(Window	window, Cursor	cursor_type)
{
	XDefineCursor(Dpy, window, cursor_type);
	XFlush(Dpy);
}

void
set_watch_cursor(Window	window)
{
	set_window_cursor(window, watch_cursor);
}

void
set_circle_cursor(Window	window)
{
	set_window_cursor(window, circle_cursor);
}

void
set_left_ptr_cursor(Window	window)
{
	set_window_cursor(window, left_ptr_cursor);
}

void
get_cursors(Window window)
{
	if (!circle_cursor)
		circle_cursor = XCreateFontCursor(Dpy, XC_circle);
	if (!watch_cursor)
		watch_cursor = XCreateFontCursor(Dpy, XC_watch);
	if (!left_ptr_cursor)
		left_ptr_cursor = XCreateFontCursor(Dpy, XC_left_ptr);

	if (circle_cursor == NULL) {
	XColor	color_1, color_2;
	Pixmap	source = XCreateBitmapFromData(Dpy, window, circle_bits,
				circle_width, circle_height),
		mask = XCreateBitmapFromData(Dpy, window, circle_mask_bits,
				circle_width, circle_height);

		color_1.pixel = WhitePixel(Dpy, Screen);
		color_1.red   = color_1.green = color_1.blue  = 0xffff;
		color_1.pixel = BlackPixel(Dpy, Screen);
		color_2.red   = color_2.green = color_2.blue  = 0;
		color_1.flags = color_2.flags = DoAll;

		circle_cursor = XCreatePixmapCursor(Dpy, source, mask,
					&color_1, &color_2,
					circle_x_hot, circle_y_hot);
		XFreePixmap(Dpy, source);
		XFreePixmap(Dpy, mask);
	}
	if (!watch_cursor)
		watch_cursor = circle_cursor;
	if (!left_ptr_cursor)
		left_ptr_cursor = circle_cursor;
}

/*=======================================
%	Remove those non useful events.	%
%	If no exposure_handle passed,	%
%	exposure events will be saved.	%
=======================================*/
void
Delay_Clear(AnyWindow*	aw, int	(*exp_hd)(), Image **imgp,
		int imgs, int fdelay)
{
int	dl;
    while (--fdelay)	{
	while (ImageEvent(aw, ButtonPressMask))
		XBell(aw->dpy, 50);
	RemoveImageEvent(aw, PointerMotionMask);
	if (exp_hd && ImageEvent(aw, ExposureMask)) {
	register Image	*img=NULL;
	register int	i;
	    if (imgp && (i =
		WhichImage(aw->event->xany.window, imgp, imgs)) >= 0)
		img = imgp[i];
	    exp_hd(aw->event, img, NULL);
	} else if (!(fdelay&7))	usleep(1000);
    }
}

void
FlushingCursor(aw, exp_hd, imgp, imgs, x0, y0, cw, ch, ftimes, rev)
AnyWindow*	aw;
int	(*exp_hd)();
Image	**imgp;
{
int	clear=0, show=0;
#define	fdelay	ftimes
fdelay = 32 - (ftimes & 0x1F);
	do {
	/* light up writing position cursor & flushing delay */
		Delay_Clear(aw, exp_hd, imgp, imgs, fdelay);
		if (!clear && !(show>>=1))	{
		    if (rev)
			XSetFunction(aw->dpy, aw->gc, GXinvert);
		    XFillRectangle(aw->dpy, aw->win, aw->gc, x0, y0, cw, ch);
		    clear = fdelay;
		}
		XSetFunction(aw->dpy, aw->gc, GXcopy);
		/*	erase writing position cursor	*/
		Delay_Clear(aw, exp_hd, imgp, imgs, fdelay);
		if (!show && !(clear>>=1))	{
		    if (rev)
			XSetFunction(aw->dpy, aw->gc, GXinvert),
			XFillRectangle(aw->dpy, aw->win, aw->gc, x0, y0, cw, ch);
		    else	XClearArea(aw->dpy, aw->win, x0, y0, cw, ch, 0);
		    show = fdelay;
		}
		XSetFunction(aw->dpy, aw->gc, GXcopy);
	} while (!ImageEvent(aw, KeyPressMask));

	if (clear)	/* clear cursor */
	    if (rev)
		XSetFunction(aw->dpy, aw->gc, GXinvert),
		XFillRectangle(aw->dpy, aw->win, aw->gc, x0, y0, cw, ch);
	    else	XClearArea(aw->dpy, aw->win, x0, y0, cw, ch, 0);

	XSetFunction(aw->dpy, aw->gc, GXcopy);
}

void
TopWindow(AnyWindow*	awin, int (*exp_hd)(), int use_unmap)
{
if (use_unmap) {
	XUnmapWindow(awin->dpy, awin->frame);
	XMapWindow(awin->dpy, awin->frame);
} else	{
	XClearArea(awin->dpy, awin->win, 0, 0, 0, 0, True);
	XRaiseWindow(awin->dpy, awin->frame);
}

XSync(awin->dpy, No);
if (exp_hd)	while (ImageEvent(awin, ExposureMask))
	exp_hd(awin->event, awin, No);
else	XWindowEvent(awin->dpy, awin->win, ExposureMask, awin->event);
}
