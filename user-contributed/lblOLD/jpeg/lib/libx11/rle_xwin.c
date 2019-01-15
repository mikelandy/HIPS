/*	RLE_X_WINDOW . C
%
%	Copyright (c)	Jin Guojun	1991
%
%	get_pic(img, window_geometry, previous_img, img_info)
%	handle_exposure(img, x, y, width, height, img_h, sub_win)
%	void	DumpScan_to_dpy(img)
%	void	init_img_info(img, dpy)
%	Maintain_Flush(img, previous, imgp, image_y)
%	MapRGB(img, prev_img, imgp, data_buf, save_scan, max_h-y, w, y, icn_fact)
%
% AUTHOR:	Jin Guojun - LBL	8/1/91
*/

#include "panel.h"
#include "imagedef.h"

#ifndef	VERS_COLOR_TUNER
#define	VERS_COLOR_TUNER	"N18-1"
#endif
#ifndef	HELP_INFO
#define	HELP_INFO	"CTRL+button => panel"
#endif
#ifdef	HIPS2_HF
extern	char*	Progname;
#define	progname Progname
#else
extern	char*	progname;
#endif

bool	tuner_flag, OsameI,
	multi_hd;	/* for multi_frame RLE */

void
DumpScan_to_dpy(img)	/* dump entire image to the screen */
image_information*	img;
{
byte	*read_scan[3], *save_scan[3];
register int	i, y=img->h;
	while (y--)	{
		read_scan[0] = ORIG_RLE_ROW(img, y-1);
		save_scan[0] = SAVED_RLE_ROW(img, y-1);
		for (i=1; i < img->img_channels; i++)
		    read_scan[i] = read_scan[i-1] + img->w,
		    save_scan[i] = save_scan[i-1] + img->w;
		Map_Scanline(img, read_scan, save_scan, y, img->w,
			determine_icon_size(img, &img->icn_w, &img->icn_h));
	}
	i = strlen(img->filename) + sizeof(HELP_INFO) + 8;
	if (pointer_buffer_size(img->title) < i)
		img->title = (char *)realloc(img->title, i);
	sprintf(img->title, "%s(%d) %s", img->filename, img->RGB, HELP_INFO);
	XStoreName(img->dpy, img->window, img->title);
	XPutImage(img->dpy, img->window, img->gc, img->image,
		0, 0, 0, 0, img->w, img->h);
}

/* returns how many lines it blitted */
void
handle_exposure(img, x, y, width, height, img_h, sub_win)
register image_information	*img;
int	(*sub_win)();
{
	/*
	* If window has been resized (bigger), dont bother redrawing
	* the area outside the image.
	*/
	if (x < 0)
		width -= x,	x = 0;

/*	if the image has not yet read itself in, dont blit any of it
	instead clear out that top portion of the window (not needed oh well)
*/

	if (y < img->h - img_h && img->in_type == RLE)	{
		XClearArea(img->dpy, img->window, x, y,
			width, img->h - img_h - y, False);
		height -= img->h - img_h - y;
		y = img->h - img_h;
	}

	if (height <= 0)	return;	/* hardly happen */

	if (y + height >= img->h)
		height = img->h - y;

	/*	if bitmap, round beginning pixel to beginning of word	*/

	if (img->binary_img) {
		int offset = x % BitmapPad (img->dpy);
		x -= offset;
		width += offset;
	}
	if (x + width >= img->w)
		width = img->w - x;

	if (width <= 0 || height <= 0)
		return;

	if (!tuner_flag && img->refresh_pixmap)
		XCopyArea(img->dpy, img->refresh_pixmap, img->window, img->gc,
			x, y, width, height, x, y);
	else
		XPutImage(img->dpy, img->window, img->gc, img->image,
			x, y, x, y, width, height);
	if (img->sub_img) {
	XRectangle	r;
		r.x = x;	r.y = y;
		r.width = width;
		r.height = height;
		XSetClipRectangles(img->dpy, img->gc, 0, 0, &r, 1, Unsorted);
		sub_win(img, 0, 0);
		XSetClipMask(img->dpy, img->gc, None);
	}
	else	img->tmp_offset = 0;
}

Map_Scanline(img, data_buf, save_scan, y, w, icon_factor)
image_information*	img;
byte	*data_buf[], *save_scan[];
{
if (img->mono_img)	/*	map  data_buf to save_scan	*/
	map_rgb_to_bw(img, data_buf, save_scan[0], w);	/* 0 = img->w */
else
	map_rgb_to_rgb(img, data_buf, save_scan, w);

	/*	map save_scan (1 line, width `w') to img->image->data	*/
(*img->map_scanline)(img, save_scan, img->dpy_channels, w, 1, y, img->image);

	/* Subsample image to create icon */
if (img->icn_image && (y%icon_factor == 0))
    (*img->map_scanline)(img, save_scan, img->dpy_channels,
		 img->icn_w, icon_factor, y / icon_factor, img->icn_image);
if (img->in_type==RLE)	y--;
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
		handle_exposure(eimg, event.xexpose.x, event.xexpose.y,
			event.xexpose.width, event.xexpose.height, (i==cur_img) ?
			(img->in_type==RLE) ? image_y : img->h-image_y : eimg->h,
			DrawCrop);
		XFlush(img->dpy);
	}
    }
}

MapRGB(cur_img, previous, imgp, data_buf, save_scan, y, w, image_y, icon_factor)
image_information	*previous, **imgp;
byte	*data_buf[], *save_scan[];
{
y = Map_Scanline(imgp[cur_img], data_buf, save_scan, y, w, icon_factor);
Maintain_Flush(cur_img, previous, imgp, image_y);
return	y;
}

void
init_img_info(i, dpy)
image_information *i;
Display	*dpy;
{
	if (dpy)	i->dpy = dpy;
	i->lvls = specified_levels;
	i->window = i->icn_window = NULL;
	i->pixmap = i->icn_pixmap = NULL;
	i->pix_info_window = NULL;
	i->gc = i->icn_gc = NULL;
	i->image = i->icn_image = NULL;
	i->colormap = NULL;
	i->marray = zalloc(sizeof(*(i->marray)), 3, "m_3channel");
	i->pixmap_failed = False;

	i->img_num = i->sub_img = multi_hd = 0;
	i->filename = i->title = NULL;
	i->fd = stdin;
	i->OUT_FP = stdout;
	i->img_channels = i->dpy_channels = 0;
	i->scan_data = NULL;
	i->map_scanline = NULL;
	i->MAG_scanline = NULL;
	i->gamma = 0.0;
	format_init(i, IMAGE_INIT_TYPE, RLE, RLE, progname, VERS_COLOR_TUNER);
	i->x = i->y = 0;
	i->w = i->h = 0;
	i->icn_w = i->icn_h = 0;

	i->mag_x = i->mag_x = 0;
	i->mag_w = i->mag_h = 0;
	i->mag_fact = 1;
	i->save_mag_fact = 2;
	i->mag_mode = False;
	i->mag_pixmap = NULL;

	i->binary_img = False;
	i->dither_img = False;
	i->mono_img = False;
	i->rw_cmap = False;
	i->sep_colors = False;
	i->mono_color = False;
	i->color_dpy = True;

	i->in_cmap = NULL;
	i->ncmap = i->cmlen = 0;
	i->modN = NULL;
	i->divN = NULL;
	i->dm16 = NULL;
	i->pixel_table = NULL;
	if (!i->visual_class)
		i->visual_class = -1;
}

BuildColorImage(img, previous_img, window_geometry, icon_factor)
image_information	*img, *previous_img;
char	*window_geometry;
int	*icon_factor;
{
extern void	choose_scanline_converter(), get_dither_colors();
extern XImage	*get_X_image();
extern int	eq_cmap();

	if (!img->title)	{
		img->title = (char *) malloc(strlen(img->filename) +
			sizeof(HELP_INFO) + 8);
		sprintf(img->title, "%s(%d) %s", img->filename,
		(img->in_type<RLE ? img->RGB : img->img_num) + 1, HELP_INFO);
	}
	if (!previous_img)	/* better after img->????_color set up. */
		find_appropriate_visual(img);

	get_dither_colors(img);

	if (!previous_img)	/* Get X color map */
		init_color(img);

    /*
     * Here if we are flip_booking we gotta get nasty here... img->w, img->h
     * and img->img_channels must match for well surely be screwed if they
     * dont match up  and we flip_book
     */
	if (previous_img) {
	    if (img->w != previous_img->w || img->h != previous_img->h ||
		img->img_channels != previous_img->img_channels){
		prgmerr(0, "%s: Images %s & %s dont match in size or channels",
			progname, previous_img->title, img->title);
		return	(FATAL_FAILURE);
	    }
	    if ((img->mono_color &&
		!eq_cmap(previous_img->in_cmap, previous_img->cmlen,
			img->in_cmap, img->cmlen)))	{
		prgmerr(0, "%s: Images %s and %s have different colormaps",
			progname, previous_img->title, img->title);
		return	(FATAL_FAILURE);
	    }
	}

    /*
     * Here we have to conserve memory on the client side and not allocate a
     * new Ximage structure with associated memory for image data.  Also, if
     * the server was unable to supply us with a pixmap in previous_img, then
     * we need to save the XImage there...
     */
	if (!previous_img || previous_img->pixmap == NULL)	{
	    if (!img->image)
		img->image = get_X_image(img, img->w+1, img->h, True);

	    if (img->image == NULL) {
		prgmerr(0, "problem getting XImage");
		return	(MALLOC_FAILURE);
	    }
	}
	else	img->image = previous_img->image;

	/* only dick with the icon if we are not flip_booking */
	if (!previous_img) {
		*icon_factor = determine_icon_size(img,
			&img->icn_w, &img->icn_h);

		if (!img->icn_image)
		    img->icn_image = get_X_image(img, img->icn_w, img->icn_h, 1);

		if (img->icn_image == NULL) {
			prgmerr(0, "malloc for fancy icon");
			return	(MALLOC_FAILURE);
		}
	}
	/*
	* Get image and icon windows of the right size
	*/
	create_windows(img, window_geometry);
	set_watch_cursor(img->window);
	if (!img->map_scanline)
		choose_scanline_converter(img);	/* map_scan L77 */
	return	SUCCESS;
}


/*
%	Read an image from the input file and display it.
%	number of scan lines to be drawn in incremental mode
*/

#define LINES_DRAWN	10

get_pic(cur_img, window_geometry, previous_img, img_info)
image_information	*previous_img, **img_info;
char *window_geometry;
{
int	image_xmax, image_ymax, icon_factor;
byte	*save_scan[3], *read_scan[3];
register int	i, image_y, next_y, y_base, lines_buffered;
register image_information	*img = img_info[cur_img];

	/*
	* Read setup info from file.
	*/

    if ((i=(*img->header_handle)(HEADER_READ, img, 0, multi_hd, OsameI)) < 0)
	return	i;	/* rle_err	*/

    if (img->in_type == RLE)
	image_xmax = rle_dflt_hdr.xmax,
	image_ymax = rle_dflt_hdr.ymax;
    else
	image_xmax = img->w-1,
	image_ymax = img->h-1;

    rle_dflt_hdr.xmin = 0;
    rle_dflt_hdr.xmax = img->w - 1;

    if (img->img_channels < img->dpy_channels)
	message("%s (get_pic): dpy_channels %d > img_channels %d\n", progname,
 		img->dpy_channels, img->img_channels);

    if (!img->title &&
	!(img->title = rle_getcom("image_title", &rle_dflt_hdr )) &&
	!(img->title = rle_getcom("IMAGE_TITLE", &rle_dflt_hdr )) &&
	!(img->title = rle_getcom("title", &rle_dflt_hdr)) &&
	!(img->title = rle_getcom("TITLE", &rle_dflt_hdr)) )	{}

    i = BuildColorImage(img, previous_img, window_geometry, &icon_factor);
    if (i != SUCCESS)
	return	i;

	/*
	* If we are going to display a 2 or 3 channel image in one channel (-w)
	* we have to copy the data anyway, so we will always read it into the
	* same spot, and map it into a saved_data array with only one color per
	* scanline this saves memory.
	*
	* read_scan[] is the scan data that we read into with rle_getrow.
	* If the number of image channels is equal to that which is displayed,
	* we will not mallocate new memory for it, but we will move it
	* throughout the saved_data along with save_scan.
	*
	* save_scan[] is the pointer to the current line in img->saved_data
	* that we are saving rle_getrows into.
	*
	* If we cant malloc the memory for the save_scan[] or we are doing
	* flip_book, then we dont want to malloc the memory for this thing.
	*
	* Set up for rle_getrow.  Pretend image x origin is 0.
	*/

/* get img->h rows mem. SAVED_RLE_ROW uses scan_data to calc address! */
    read_scan[0] = img->scan_data = NULL;

	/* if we are flip_booking don't mess with this one huh ? */
    if (!previous_img || tuner_flag) {	/* BIG bug in getx11 */
	img->scan_data = (byte *) malloc(SAVED_RLE_ROW(img, img->h));

	if (! img->scan_data)
		prgmerr(tuner_flag, "malloc for %s scanline buffer failed",
			img->filename);

	/* use the macro to point us to the last line... start saving here */
	save_scan[0] = SAVED_RLE_ROW(img, img->h - 1);
	for (i=1; i < img->img_channels; i++)
	    save_scan[i] = save_scan[i-1] + img->w;
    }

    /* get one line of scan data for reading if we are doing monochrome */

    if (img->mono_img && img->in_type==RLE ||
		img->scan_data==NULL || tuner_flag) {
	if (tuner_flag)
		img->data = nzalloc(img->w * (img->h+1), img->img_channels);
	else	if ((read_scan[0]=(byte*)malloc(img->w*img->img_channels))==NULL){
		prgmerr(0, "malloc for read_scan buffer failed");
		return	(MALLOC_FAILURE);
	}

	if (read_scan[0]==0)
		read_scan[0] = ORIG_RLE_ROW(img, img->h - 1);
	for (i=1; i < img->img_channels; i++)
	    read_scan[i] = read_scan[i-1] + img->w;
	if (img->scan_data == NULL)
		save_scan[0] = read_scan[0],
		save_scan[1] = read_scan[1],
		save_scan[2] = read_scan[2];
    }
    else	read_scan[0] = save_scan[0],
		read_scan[1] = save_scan[1],
		read_scan[2] = save_scan[2];

    if (!img->data)
	img->data = img->scan_data;

    if (img->in_type != RLE){ /* multiple-frame is applied to HIPS and FITS */
	(*img->friend)(FI_LOAD_FILE, img, 0,
		img->in_type==HIPS || img->in_type==FITS ? img->RGB : OsameI);
	if (img->mono_img && !tuner_flag && img->in_type != RLE &&
		img->scan_data==0)
		img->scan_data = read_scan[0] = save_scan[0] = img->data;
    }
/*	For each scan line, read it, save it, dither it and display it	*/
    next_y = img->h - 1;

    for (lines_buffered=image_y=0; image_y<img->h; image_y++) {
	int	y;
	if (img->in_type == RLE)
		y = rle_getrow(&rle_dflt_hdr, read_scan);
	else	y = image_ymax - image_y;

	next_y = MapRGB(cur_img, previous_img, img_info, read_scan, save_scan,
		image_ymax - y, img->w, y, icon_factor);

	if (++lines_buffered >= LINES_DRAWN || (img->fd==stdin && !jump_flag)){
		y_base = next_y + 1 - (img->in_type==RLE ? 0 : lines_buffered);
		XPutImage(img->dpy, img->pixmap?img->pixmap:img->window, img->gc,
			img->image, 0, y_base, 0, y_base, img->w,lines_buffered);
		if (img->pixmap)
		    XCopyArea(img->dpy, img->pixmap, img->window, img->gc, 0, y_base,
			  img->w, lines_buffered, 0, y_base);
		lines_buffered = 0;
	}

	if (img->scan_data) {
		/* move scan up one line */
		save_scan[0] = SAVED_RLE_ROW(img, next_y);
		for (i=1; i < img->img_channels; i++)
			save_scan[i] = save_scan[i-1] + img->w;
	/*
	* remember? if were saving more than one channel then we dont need
	* to move the data after we read it...  So kludge read_scan ...
	*/
		if (tuner_flag) {
		    read_scan[0] = ORIG_RLE_ROW(img, next_y);
		    for (i=1; i<img->img_channels; i++)
			read_scan[i] = read_scan[i-1] + img->w;
		} else	if (!img->mono_img || img->in_type != RLE)
			read_scan[0] = save_scan[0],
			read_scan[1] = save_scan[1],
			read_scan[2] = save_scan[2];
	} else	save_scan[0] = read_scan[0],
		save_scan[1] = read_scan[1],
		save_scan[2] = read_scan[2];
    }
    if (lines_buffered > 0) {
	y_base = next_y + 1 - (img->in_type==RLE ? 0 : lines_buffered);
	XPutImage(img->dpy, img->pixmap?img->pixmap:img->window, img->gc,
		img->image, 0, y_base, 0, y_base, img->w, lines_buffered);
	if (img->pixmap)
		XCopyArea(img->dpy, img->pixmap, img->window, img->gc, 0, y_base,
			img->w, lines_buffered, 0, y_base);
    }

    if (!previous_img && img->icn_pixmap) {
	XPutImage(img->dpy, img->icn_pixmap, img->icn_gc, img->icn_image,
		  0, 0, 0, 0, img->icn_w, img->icn_h);
	XClearWindow(img->dpy, img->icn_window);
    }

    set_circle_cursor(img->window);
    if (tuner_flag)
	img->hist = nzalloc(HistoSize*3, sizeof(*(img->hist)), "hist");

return (SUCCESS);
}
