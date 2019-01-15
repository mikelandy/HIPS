/*	X_WINDOW . C
%
%	Copyright (c)	Jin, Guojun
%
%	void	DumpScan_to_dpy(img)
%	handle_exposure(img, sub_win, x, y, width, height, img_h)
%
%		# the connection with map_scan.c #
%	Map_Scanline(img, data_buf, save_scan, x0, y, w, icon_factor)
%	Maintain_Flush(img, previous, imgp, image_y)
%	MapRGB(img, prev_img, imgp, data_buf, save_scan, max_h-y, w, y, icn_fact)
%
% AUTHOR:	Jin Guojun - LBL	8/1/91
*/

#include "panel.h"
#include "imagedef.h"


void
DumpScan_to_dpy(img)	/* dump entire image to the screen */
image_information*	img;
{
byte	*read_scan[3], *save_scan[3];
register int	i, y=img->h, ifactor = get_iconsize(img, 0);
	while (y--)	{
		read_scan[0] = ORIG_RLE_ROW(img, y);
		save_scan[0] = SAVED_RLE_ROW(img, y);
		for (i=1; i < img->img_channels; i++)
		    read_scan[i] = read_scan[i-1] + img->w,
		    save_scan[i] = save_scan[i-1] + img->w;
		Map_Scanline(img, read_scan, save_scan, 0, y, img->w, ifactor);
	}
	i = strlen(img->name) + sizeof(HELP_INFO) + 8;
	if (pointer_buffer_size(img->title) < i)
		img->title = (char *)realloc(img->title, i);
	sprintf(img->title, "%s(%d) %s", img->name, img->RGB, HELP_INFO);
	XStoreName(img->dpy, img->window, img->title);
	XPutImage(img->dpy, img->window, img->gc, img->image,
		0, 0, 0, 0, img->w, img->h);
	if (img->refresh_pixmap)
		XCopyArea(img->dpy, img->window, img->refresh_pixmap, img->gc,
		0, 0, img->w, img->h, 0, 0);
}

void
handle_exposure(img, sub_win, x, y, width, height, img_h, tuner_flag)
register image_information	*img;
int	(*sub_win)();
register int	x, y;
{
	/*
	* If window has been resized (bigger), dont bother redrawing
	* the area outside the image.
	*/
	if (x < 0)	width -= x,	x = 0;

	if (y < img->h - img_h && img->in_type==RLE)	{
/*
%	if the image has not yet read itself in, dont blit any of it
%	instead clear out that top portion of the window (not needed oh well)
*/		XClearArea(img->dpy, img->window, x, y,
			width, img->h - img_h - y, False);

		height -= img->h - img_h - y;	/* to RLE convention	*/
		y = img->h - img_h;
	}

	if (height < 1)	return;	/* hardly happen */

	if (y + height >= img->h)
		height = img->h - y;

	/*	if bitmap, round beginning pixel to beginning of word	*/

	if (img->binary_img) {
		int offset = x % BitmapPad(img->dpy);
		x -= offset;
		width += offset;
	}
	if (x + width >= img->w)
		width = img->w - x;

	if (width > 0 && height > 0)
		exposure_r(img, sub_win, x, y, width, height, True);
}


Map_Scanline(img, data_buf, save_scan, x0, y, w, icon_factor)
image_information*	img;
byte	*data_buf[], *save_scan[];
{
    if (img->mono_img)	/*	map  data_buf to save_scan	*/
	map_rgb_to_bw(img, data_buf, save_scan[0], w);	/* 0 = img->w */
    else
	map_rgb_to_rgb(img, data_buf, save_scan, w);

	/*	map save_scan (1 line, width `w') to img->image->data	*/
    (*img->map_scanline)(img, save_scan, img->dpy_channels, w, 1, y, x0,
			img->image);

	/* Subsample image to create icon; icon_factor > 16 means False	*/
    if (img->icn_image && (y%icon_factor == 0))
	(*img->map_scanline)(img, save_scan, img->dpy_channels,
		 img->icn_w, icon_factor, y / icon_factor, 0, img->icn_image);
    if (img->in_type==RLE)	y--;	else	y++;
return	y;
}

Maintain_Flush(cur_img, previous, imgp, image_y)
image_information	*previous, **imgp;
{
XEvent event;
image_information	*img = imgp[cur_img];

    while (XPending(img->dpy)) {
	XNextEvent(img->dpy, &event);
	if (event.type == Expose) {
		image_information *eimg;
		register int	 i;
		/* get the right window bro....  */
		i = WhichImage(event.xany.window, imgp, cur_img);
		if (previous || i<0)
			eimg = imgp[cur_img];	/*flip_book override */
		else	eimg = imgp[i];
		handle_exposure(eimg, Draws, event.xexpose.x, event.xexpose.y,
			event.xexpose.width, event.xexpose.height,
			(i==cur_img) ? (img->in_type==RLE) ? image_y :
			img->h-image_y : eimg->h);
		XFlush(img->dpy);
	}
    }
}

MapRGB(cur_img, previous, imgp, data_buf, save_scan, y, w, image_y, icon_factor)
image_information	*previous, **imgp;
byte	*data_buf[], *save_scan[];
{
y = Map_Scanline(imgp[cur_img], data_buf, save_scan, 0, y, w, icon_factor);
Maintain_Flush(cur_img, previous, imgp, image_y);
return	y;
}
