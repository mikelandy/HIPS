/*	PARA_WIN . C
#
%	for c_tuner.c
*/

#include "tuner.h"

void
MapPixWindow(img, use_top, win_bg)
image_information *img;
{
int	x = 0, font_height = img->font_h + 4,
	y = use_top ? x : img->resize_h - font_height;
#ifdef	SCROLLBAR_on_CANVAS
	x += img->x;	y += img->y;
#endif
    if (img->pix_info_window == NULL) {
	img->pix_info_window = XCreateSimpleWindow(img->dpy, img->window,
		x, y, img->resize_w, font_height, 0, None, win_bg);
	} else	XMoveResizeWindow(img->dpy, img->pix_info_window,
				x, y, img->resize_w, font_height);

	XMapWindow(img->dpy, img->pix_info_window);
}

void
DrawPixWindow(img, rle_form, x, y)
image_information *img;
caddr_t	rle_form;
{
char	str[256];
byte	*r=SAVED_RLE_ROW(img, y) + x, *g=r + img->w, *b;
int	*lktg=lkt+MaxColors, *lktb=lkt+(MaxColors<<1);

if (rle_form)	y = img->h - y - 1;
	switch (img->dpy_channels) {
	case 1:
	sprintf(str, "(%3d, %3d): (%3d)", x, y, *r);
	if (tuner_flag && lkt)
		x = strlen(str),	sprintf(str+x, " [%d]", lkt[*r]);
	break;
	case 2:
		sprintf(str, "(%3d, %3d): (%d, %d)", x, y, *r, *g);
	break;
	case 3:
	b = g + img->w;
	sprintf(str, "(%3d, %3d): 0x%02X%02X%02X (%3d, %d, %d)",
		x, y, *r, *g, *b, *r, *g, *b);
	if (tuner_flag && lkt)
		x = strlen(str),
		sprintf(str+x, " [%d, %d, %d]", lkt[*r], lktg[*g], lktb[*b]);
	}

if (abs(img->mag_fact) > 1)
	x = strlen(str),	sprintf(str+x, " (%d MAG)", img->mag_fact);
if (img->sub_img)
	x = strlen(str),
	sprintf(str+x, " {%d x %d sub_image}", img->sub_img_w, img->sub_img_h);
XClearWindow (img->dpy, img->pix_info_window);
XDrawString(img->dpy, img->pix_info_window, img->gc,
	4, 2 + img->ascent, str, strlen(str));
}

void
DrawSpeedWindow(img, s)
image_information *img;
int	s;
{
char str[256];

	if (s)	sprintf(str, "%s%d Frames/Second", (s>0) ? "" : "1/", abs(s));
	else	sprintf(str, "As fast as possible");

	XClearWindow(img->dpy, img->pix_info_window);
	XDrawString(img->dpy, img->pix_info_window, img->gc,
		4, 2 + img->ascent, str, strlen(str));
}
