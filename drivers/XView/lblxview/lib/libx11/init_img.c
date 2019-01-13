/*	INIT_IMG . C
#
%	initialize and build image
%
% AUTHOR:	Jin, Guojun - LBL
*/

#include "panel.h"
#include "imagedef.h"

#ifndef	VERS_TUNER
#define	VERS_TUNER	"A8-2"
#endif

int	stingy_flag;
char	*F_I_I =	"fancy icon image";


void
init_img_info(i, dpy, mid_ver, color_dpy) /*	extracted from x_window.c */
image_information *i;
Display	*dpy;
{
	if (dpy)	i->dpy = dpy;
	i->lvls = specified_levels;
	i->window = i->icn_window = NULL;
	i->gc = i->icn_gc = NULL;
	i->image = i->icn_image = NULL;
	i->colormap = NULL;
	if (mid_ver > 0)
	    format_init(i, IMAGE_INIT_TYPE, mid_ver, -1, Progname, VERS_TUNER);
	i->marray = ZALLOC(sizeof(*(i->marray)), 3, "m_3channel");
	i->pixmap_failed = False;

	i->name = i->title = NULL;
	i->img_num = i->sub_img = i->img_channels = 0;
	i->scan_data = NULL;
	i->map_scanline = NULL;
	i->MAG_scanline = NULL;
	i->x = i->y =
	i->w = i->h =
	i->icn_w = i->icn_h = 0;

	i->mag_x = i->mag_x =
	i->mag_w = i->mag_h = 0;
	i->mag_fact = i->save_mag_fact = 1;
	i->mag_mode = False;

	i->binary_img = i->dither_img = i->mono_img = False;
	i->rw_cmap = i->sep_colors = i->mono_color = False;
	i->color_dpy = color_dpy < 0 ? True : color_dpy;

	i->in_cmap = NULL;
	if (mid_ver > HIPS) {
		i->gamma = 0.0;
		i->ncmap = 0;
		i->pix_info_window = NULL;
		i->pixmap = i->icn_pixmap = i->mag_pixmap = NULL;
		i->modN = i->divN = i->pixel_table = NULL;
		i->dm16 = NULL;
		if (!i->visual_class)
			i->visual_class = -1;
	}
}

BuildColorImage(img, prev_img, window_geometry, icon_factor)
image_information	*img, *prev_img;
char	*window_geometry;
int	*icon_factor;
{
extern XImage	*get_XImage();

	if (!img->title && (img->title =
		nzalloc(strlen(img->name) + 16 + sizeof(HELP_INFO), 1, No)))
	    sprintf(img->title, "%s (%d) %s", img->name,
		(img->in_type !=RLE ? img->RGB : img->img_num) + 1, HELP_INFO);

	get_dither_colors(img);
	if (!prev_img)	/* Get X color map */
		init_color(img);
	/*
	* If running movie, we need to be nasty here. img->w, img->h and
	* img->img_channels must match the previous one - prev_img. Otherwise,
	* it can be screwed up.
	*/
	if (prev_img && prev_img != img) {
	    if (img->w != prev_img->w || img->h != prev_img->h ||
		img->img_channels != prev_img->img_channels)	{
		return	prgmerr(1-FATAL_FAILURE,
			"Images %s & %s dont match in size or channels",
			prev_img->title, img->title);
	    }
	    if ((img->mono_color &&
		!eq_cmap(prev_img->in_cmap, prev_img->cmaplen,
			img->in_cmap, img->cmaplen)))
		return	prgmerr(1-MALLOC_FAILURE,
				"Images %s and %s have different colormaps",
				prev_img->title, img->title);
	}
	else if (icon_factor && !stingy_flag) {	/* icon is not for a moive! */
		*icon_factor = get_iconsize(img, 0);
		if (!img->icn_image && !(img->icn_image =
			get_XImage(img, img->icn_w, img->icn_h, 1, F_I_I)))
			return prgmerr(1-MALLOC_FAILURE, F_I_I);
	}
	/* If we have to conserve memory on the client side and not allocate a
	* new Ximage structure with associated memory for image data.  Also, if
	* the server was unable to supply us with a pixmap in prev_img,
	* we need to save a XImage there. If the slow mode is used, then we
	* will save images in server side for movies.
	*/
	if (!prev_img || prev_img==img || !prev_img->pixmap) {
	    if (img->image &&
		(img->w+1 != img->image->width || img->h != img->image->height))
		CFREE(img->image->data),	XDestroyImage(img->image),
		img->image = NULL;
	    if (!img->image && !(img->image =
			get_XImage(img, img->w+1, img->h, Yes, img->name)))
		return	prgmerr(1-MALLOC_FAILURE, "get XImage");
	}
	else	img->image = prev_img->image;

	create_windows(img, window_geometry, stingy_flag | !icon_factor);
	set_watch_cursor(img->window);
	if (!img->map_scanline)
		choose_scanline_converter(img);	/* map_scan L102 */
	if (icon_factor)	/* not root a window	*/
		XMapWindow(img->dpy, img->frame),
		XMapWindow(img->dpy, img->window);
return	SUCCESS;
}
