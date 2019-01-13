/*	magnify . c
%
*/
#include "tuner.h"

void
mag_pan(img, action, bx, by, new_mag_fact, stingy)
image_information	*img;
int	action, bx, by, new_mag_fact;
{
x_bool	fast_pan = False, redraw = False, redither = False;
int	omag_x, omag_y, mag_f = abs(img->mag_fact), ix, iy;

#ifndef	SCROLLBAR_on_CANVAS
	bx += img->x;	by += img->y;
#endif
	ix = img->mag_x + bx / mag_f,
	iy = img->mag_y + by / mag_f;


	/* we could re-open the img->name to handle this ... */
	if (img->scan_data == NULL)
		return;

    switch (action) {
	/*
	* Normalize has to switch the current mag factor with 1 if they
	* differ... It also remembers the old mag_x and mag_y and stuff.  It
	* should use the pixmaps to refresh, so that it will be fast when
	* toggeling in this mode...
	*/
    case ACTION_SWITCH_MAG_MODE:
	if (img->mag_fact == img->save_mag_fact)
		return;
	else {
	    if (img->mag_fact == 1) {
		img->mag_mode = True;
		img->mag_fact = img->save_mag_fact;
		img->mag_x = img->save_mag_x;
		img->mag_y = img->save_mag_y;
		img->mag_w = img->save_mag_w;
		img->mag_h = img->save_mag_h;
		img->save_mag_x = img->save_mag_y = 0;
		img->save_mag_w = img->w; img->save_mag_h = img->h;
		img->save_mag_fact = 1;
		img->refresh_pixmap = img->mag_pixmap;
	    } else {
		img->mag_mode = False;
		img->save_mag_x = img->mag_x;
		img->save_mag_y = img->mag_y;
		img->save_mag_w = img->mag_w;
		img->save_mag_h = img->mag_h;
		img->save_mag_fact = img->mag_fact;
		img->mag_x = img->mag_y = 0;
		img->mag_w = img->w; img->mag_h = img->h;
		img->mag_fact = 1;
		img->refresh_pixmap = img->pixmap;
	    }
	    redraw = True;
	}
	break;
    case ACTION_MAGNIFY:
    case ACTION_UNMAGNIFY:
	if (!new_mag_fact)	new_mag_fact = -2;
	if ((img->mag_fact=new_mag_fact) < 2)	{
		img->mag_x = img->mag_y = 0;
		img->mag_w = img->w; img->mag_h = img->h;
	}
	if ((mag_f=abs(new_mag_fact)) < 2)	{
		img->mag_fact = 1;
		img->refresh_pixmap = img->pixmap;
		if (img->mag_mode)
			redraw = True;
		img->mag_mode = False;
		break;
	} else img->mag_mode = True;
	if (new_mag_fact < 0)
		mag_f = 1;

	img->mag_x = ix - (img->resize_w >> 1)/mag_f;
	img->mag_y = iy - (img->resize_h >> 1)/mag_f;

	img->mag_w = img->w/mag_f;
	img->mag_h = img->h/mag_f;

	if (!img->mag_pixmap && !img->pixmap_failed && !stingy) {
		img->mag_pixmap = XCreatePixmap(img->dpy, img->window,
					img->w, img->h, img->dpy_depth);
		check_pixmap_allocation(img);
	}

	img->refresh_pixmap = img->mag_pixmap;

	redither = redraw = True;
	break;

    case ACTION_PAN:
	fast_pan = img->mag_mode;  /* are we REALLY just panning around? */
	omag_x = img->mag_x;
	omag_y = img->mag_y;

	if (!img->mag_mode) {
		img->mag_fact = img->save_mag_fact;
		img->save_mag_fact = 1;
	}
	if (img->mag_fact > 1)
		img->mag_mode = True;
	else	return;

	img->refresh_pixmap = img->mag_pixmap;

	img->mag_x = ix - (img->resize_w >> 1)/mag_f;
	img->mag_y = iy - (img->resize_h >> 1)/mag_f;

	/* Isnt this always like this? */
	img->mag_w = img->w/mag_f;
	img->mag_h = img->h/mag_f;

	redither = redraw = True;
	break;
    }

    /* check bounds */
    if (img->mag_x < 0)
	img->mag_x = 0;
    if (img->mag_y < 0)
	img->mag_y = 0;
    
    if (img->mag_x + img->mag_w > img->w)
	img->mag_x = img->w - img->mag_w - (img->mag_w * mag_f < img->w);
    if (img->mag_y + img->mag_h > img->h)
	img->mag_y = img->h - img->mag_h - (img->mag_h * mag_f < img->h);

    /* check bounds */
    if (img->mag_x < 0)
	img->mag_x = 0;
    if (img->mag_y < 0)
	img->mag_y = 0;

    /* let the suckers know that we are thinking */
    set_watch_cursor(img->window);

    /*
     * Some could argue that this fast_pan shit is a waste of time, but it
     * does speed things up a bunch, and its really hard to understand.
     * Sorry, no fancy pictures in the comments.  Just code.  We figure out
     * which rectangle is blt-able.  We blt it on the server side, and on the
     * client side (my fancy XCopyImage) and then MAG_scanline the exposed
     * area and XPutImage that stuff too...  Dont change it cuz its right.
     */
    if (fast_pan) {
	int	width, hight,
		src_x, src_y, dst_x, dst_y,
		pwidth = omag_x - img->mag_x,
		phight = omag_y - img->mag_y;

	pwidth = img->mag_w - ((pwidth < 0) ? - pwidth : pwidth);
	phight = img->mag_h - ((phight < 0) ? - phight : phight);

	/*
	 * pwidth and phight now contain the size of the non-changing
	 * (BLT-able) portion of the viewport in rle_pixel space.
	 */
	width = pwidth * img->mag_fact + (img->w - img->mag_w * img->mag_fact);
	hight = phight * img->mag_fact + (img->h - img->mag_h * img->mag_fact);

	/* Now we compute the src_xy and dst_xy for the pixel copy */
	if (omag_x < img->mag_x)
		dst_x = 0,	src_x = img->w - width;
	else
		src_x = 0,	dst_x = img->w - width;

	if (omag_y < img->mag_y)
		dst_y = 0,	src_y = img->h - hight;
	else
		src_y = 0,	dst_y = img->h - hight;

	/* subtract partial pixels if we are going right */
	if (omag_x < img->mag_x)
	    width -= img->w - img->mag_w * img->mag_fact;

	/* subtract partial pixels if we are going down */
	if (omag_y < img->mag_y)
	    hight -= img->h - img->mag_h * img->mag_fact;

	if (src_x == dst_x && src_y == dst_y)
		redraw = redither = False;
	else {
	    /* XCopyImage is only implemented for 8 and 32 bit image pixels */
	    if (img->refresh_pixmap && XCopyImage(img->image, src_x, src_y,
						width, hight, dst_x, dst_y))
	    {
		XCopyArea(img->dpy, img->refresh_pixmap, img->refresh_pixmap,
			img->gc, src_x, src_y, width, hight, dst_x, dst_y);

		if (dst_y) {
		   (*img->MAG_scanline)(img, img->mag_x, img->mag_y,
					img->mag_fact, 0, 0,
					img->w, dst_y, img->image);

		    XPutImage(img->dpy, img->refresh_pixmap, img->gc, img->image,
				0, 0, 0, 0, img->w, dst_y);
		}
		else {
		    if (hight < img->h) {
			(*img->MAG_scanline)
				(img, img->mag_x, img->mag_y + phight,
				img->mag_fact, 0, hight,
				img->w, img->h - hight, img->image);
			XPutImage(img->dpy, img->refresh_pixmap, img->gc,
				img->image, 0, hight, 0, hight,
				img->w, img->h - hight);
		    }
		}

		if (dst_x)	{
		    if (hight && width < img->w){
			(*img->MAG_scanline)
			    (img, img->mag_x, img->mag_y +
			    ((dst_y) ? img->mag_h - phight : 0), img->mag_fact,
			    0, dst_y, img->w - width, hight, img->image);

			XPutImage(img->dpy, img->refresh_pixmap, img->gc,img->image,
				0, dst_y, 0, dst_y, img->w - width, hight);
		    }
		}
		else {
		    if (hight && width < img->w) {
			(*img->MAG_scanline)
			    (img, img->mag_x + pwidth, img->mag_y +
			    ((dst_y) ? img->mag_h - phight: 0), img->mag_fact,
			    width, dst_y, img->w - width, hight, img->image);

			XPutImage(img->dpy, img->refresh_pixmap, img->gc,
				  img->image, width, dst_y, width, dst_y,
				  img->w - width, hight);
		    }
		}

		/*
		* We already redithered...  If XCopyImage failed we arent
		* here and we have to redither the whole thing below.
		*/
		redither = False;
	    }
	}
    }

	/* redither the whole thing, but repaint part */
	mag_f = abs(img->mag_fact);
	bx = img->w,	by = img->h;
	if (img->mag_fact < (ix=iy=0))
		bx /= mag_f,	by /= mag_f;
	mag_f = !img->refresh_pixmap;
    if (redither || (redraw && mag_f))	{
	(*img->MAG_scanline)(img, img->mag_x, img->mag_y,
			img->mag_fact, 0, 0, img->w, img->h, img->image);
	if (mag_f)
		ix = img->x,	iy = img->y;
	XPutImage(img->dpy, mag_f ? img->window:img->refresh_pixmap,
		  img->gc, img->image, ix, iy, 0, 0, bx, by);
    }

    if (redraw && !mag_f)	{
	ix = iy = 0;
#ifdef SCROLLBAR_on_CANVAS
	ix = img->x;	iy = img->y;
#endif
	if (bx > img->resize_w)	bx = img->resize_w;
	if (by > img->resize_h)	by = img->resize_h;
	XCopyArea(img->dpy, img->refresh_pixmap, img->window, img->gc,
		img->x, img->y, bx, by, ix, iy);
    }
set_left_ptr_cursor(img->window);
Draw_ImageScrollBars(img);
if (img->sub_img)	DrawCrop(img, 0, redraw | redither);
}
