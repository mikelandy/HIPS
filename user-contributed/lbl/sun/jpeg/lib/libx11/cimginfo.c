/*	ColorImageINFO . C
#
%	Copyright (c)	Jin Guojun - All Rights Reserved
%
%	Display Image Pixel Info.
%
% Author:	Jin Guojun - LBL	Mon. Apr 1, 1991
*/

#include "panel.h"

extern	PColor	MGray;	/* in function.c	*/
extern	void	DrawPixWindow();

ColorImageInfo(img, top, fforward, regular_rle)
image_information*	img;
{
x_bool first = 1;
int	mg=GetGray(img->dpy, img->colormap, img->entries, 96),
bg=GetCloseColor(img->dpy, img->colormap, img->entries, NULL, 192, 248, 160);
/* kludge handle multi-frame scan_data, also last line in this routine	*/
int	multi_f_offset = fforward ? img->dpy_channels * img->w * img->h : 0;
	img->scan_data += multi_f_offset;

	set_left_ptr_cursor(img->window);

	MapPixWindow(img, top, mg < 0 ? MGray : mg);
	XSetForeground(img->dpy, img->gc, bg < 0 ? White : bg);
	TraceMouse(img, DrawPixWindow, (caddr_t)regular_rle, 0);
	set_circle_cursor(img->window);
	UnmapPixWindow(img);
	img->scan_data -= multi_f_offset;
}

TraceMouse(img, func, client_data, param, flags)
Image	*img;
void	(*func)();
caddr_t	client_data;	/* any single data or structure pointer	*/
{
Window	parent,child;
int	rx, ry, x, y, lx, ly;
unsigned	mask;
	func(img, client_data, lx=0, ly=0, param, flags);
    Loop	{	/* loop until button released */

	if (XQueryPointer(img->dpy, img->win, &parent, &child, &rx, &ry,
		&x, &y, &mask))	{
		if (!(mask&(Button1Mask | Button3Mask)))
			break; /* released */
		mask = abs(img->mag_fact);
		toREALxy(img, x, y);

		if (img->mag_fact > 0)
			x /= mask,	y /= mask;
		else	x *= mask,	y *= mask;
		x += img->mag_x;	y += img->mag_y;
			/* wait for new pixel */
		if ((x != lx || y != ly) &&
		    (x >= 0 && x < img->width &&
		     y >= 0 && y < img->height)) {
			func(img, client_data, x, y, param, flags);
			lx = x;  ly = y;
		}
	} else	break;
    }
}
