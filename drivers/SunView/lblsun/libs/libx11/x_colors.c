/*
* This software is copyrighted as noted below.  It may be freely copied,
* modified, and redistributed, provided that the copyright notices are
* preserved on all copies.
*
* There is no warranty or other guarantee of fitness for this software,
* it is provided solely "as is".  Bug reports or fixes may be sent
* to the author, who may or may not act on them as he desires.
*
* You may not include this software in a program or other software product
* without supplying the source, or without informing the end-user that the
* source is available for no extra charge.
*
* If you modify this software, you should include a notice giving the
* name of the person performing the modification, the date of modification,
* and the reason for such modification.
%
%
* x_colors.c - init_color
*
* Author:	Spencer W. Thomas  (x10)
* 		Computer Science Dept.
* 		University of Utah
* Date:	Thu Feb 20, 1986
* Copyright (c) 1986, University of Utah
*
* Modified:	Andrew F. Vesper (x 11)
*		High Performance Workstations
*		Digital Equipment Corp
* Date:	Fri, Aug 7, 1987
*		Thu, Jan 14, 1988
* Copyright (c) 1987,1988, Digital Equipment Corporation
*
* Modified:	Martin R. Friedmann (better X11, flipbook, mag, pix_info)
* 		Dept of Electrical Engineering and Computer Science
*		University of Michigan
* Date:	Tue, Nov 14, 1989
* Copyright (c) 1989, University of Michigan
*
* Modified:	Jin, Guojun (x11)
*		Control Panel - MapPixWindow(), UnmapPixWindow(),
*				all init_color_s() ... Bugs Fixing
* Date:	Mon, Apr 1, 1991
* Copyright (c) 1991, 1994	Lawrence Berkelry Laboratory
*/

#include "function.h"

extern	LKT	*lkt;
extern	int	ncolors, shift_match_left(), shift_match_right();
extern	void	get_dither_arrays(), get_x_colormap();
extern	Colormap	lastmap;
static x_bool	init_separate_color_rw(), init_color_rw(), init_color_ro(),
		init_separate_color_ro(), init_mono_rw(), init_mono_ro();
Colormap	firstmap;
Pixel	pixel_base;
int	no_color_ref_counts, specified_levels,	Ext11Bottom, tuner_flag;
static	Image*	wait_img;
Image	*rw_set_id;	/* id for whose cc is saved	*/
XColor	*save_1st_cc;	/* save first image's colormap content	*/

#ifndef	MIN_COLOR_LEVELS
#define	MIN_COLOR_LEVELS	7
#endif
#ifndef	X_MIN_LEVELS	/* for mono_ro	*/
#define	X_MIN_LEVELS	16	/* is 32 better ?	*/
#endif
/*	for (ll=1, i=2; i < lvls; i <<= 1)	ll++;	*/
#define	ceiling_log2(i, lvls, ll)	for (ll=1, i=lvls-1; i >>= 1;	ll++)
#define	DefaultCMAP(dpy)	DefaultColormap(dpy, DefaultScreen(dpy))

static void
handle_x_errors(Display *dpy, XErrorEvent *event)
{
XID	xid = event->resourceid;
Image	*img = wait_img;

switch (event->error_code)	{
    case BadAlloc:
	if (img) {
	    if (xid == img->pixmap)
		img->pixmap = NULL;
	    else if (xid == img->icn_pixmap)
		img->icn_pixmap = NULL;
	    else if (xid == img->mag_pixmap)
		img->mag_pixmap = NULL;
	    else	goto	xDefE;
	    img->pixmap_failed = True;
	    break;
	}
    case BadWindow:
    case BadDrawable:
    case BadMatch:
    default:
xDefE:	_XDefaultError(dpy, event);
}
}

void
check_pixmap_allocation(Image	*img)
{
wait_img = img;
XSetErrorHandler(handle_x_errors);
XSync(img->dpy, False);
}

extern	int	log2_levels;	/* this is a relic.	*/

/*
* This routine builds the color map (for X window system, details may
* differ in your system.  Note particularly the two level color
* mapping:  X will give back 216 color map entries, but there is no
* guarantee they are sequential.  The dithering code above wants to
* compute a number in 0 - 215, so we must map from this to the X color
* map entry id.
*/

/*	Initialize the 8 bit color map.  Use gamma corrected map.	*/

#define	IS_BINARY	1
#define	IS_MONO		2
#define	WANT_READ_WRITE	4
#define	NOT_BINARY 	0
#define	NOT_MONO 	0
#define	WANT_READ_ONLY	0

void
init_color(register image_information *img)
{
int	type;
x_bool	done = False, try_ro = !tuner_flag;

	if (img->mono_img && !img->mono_color)
		img->lvls = MIN(img->lvls, 64);
	img->lvls = MIN(img->lvls, (1 << img->dpy_depth));
    while (!done) {
	DPRINTF(stderr, "Cmap type: binary %d, mono %d, read/write %d\n",
		 img->binary_img, img->mono_img, img->rw_cmap);

	type = (((img->binary_img) ? IS_BINARY : NOT_BINARY) |
		((img->mono_img) ? IS_MONO : NOT_MONO) |
		(((img->rw_cmap && img->sep_colors) ||
		(img->rw_cmap && !try_ro)) ?
		WANT_READ_WRITE : WANT_READ_ONLY));

	switch (type) {
	case NOT_BINARY | NOT_MONO | WANT_READ_WRITE:
		done = (img->sep_colors ?
			init_separate_color_rw : init_color_rw)(img);
/*	It is handled in previous section: convert 24 to 8-bit with colormap
	    if (!done && !try_ro) {
		prgmerr(0, "not enough cmap entries available, trying b/w.\n");
		img->mono_img = True;
		img->dpy_channels = 1;
	    }
*/	break;

	case NOT_BINARY | NOT_MONO | WANT_READ_ONLY:
		done = (img->sep_colors ?
			init_separate_color_ro :
			init_color_ro)(img, try_ro);
	    if (!done && !try_ro) {
		prgmerr(0, "not enough cmap entries available, trying b/w.\n");
		img->mono_img = True;
		img->dpy_channels = 1;
	    }
	break;

	case NOT_BINARY | IS_MONO | WANT_READ_WRITE:
		done = (img->sep_colors ?
			init_separate_color_rw : init_mono_rw)(img);
		if (!done & !img->rw_cmap)
			img->binary_img = True;
	break;

	case NOT_BINARY | IS_MONO | WANT_READ_ONLY:
		done = (img->sep_colors ?
			init_separate_color_ro :
			init_mono_ro)(img, try_ro);
		if (!done & !try_ro)	img->binary_img = True;
	break;

	case IS_BINARY | IS_MONO | WANT_READ_ONLY:
	/* make the magic square */
		get_dither_arrays(img);
		make_square(255.0, img->divN, img->modN, img->dm16);

	    if (img->pixel_table == NULL)
		img->pixel_table = (Pixel *) NZALLOC(2, sizeof(Pixel),
					"pixel_table(2)");
	    {
		int	scr = DefaultScreen(img->dpy);
		Pixel	wp = WhitePixel(img->dpy, scr),
			bp = BlackPixel(img->dpy, scr);
		if (bp < 2 && wp < 2) {
		    img->pixel_table[bp] = bp;
		    img->pixel_table[wp] = wp;
		}
		else {	/* really happens ?	*/
		    img->pixel_table[0] = bp;
		    img->pixel_table[1] = wp;
		}
	    }
	    img->lvls = 2;
	    img->lvls_squared = 4;
	    done = True;
	    break;

	default:	prgmerr(1, "Unknown type in init_colors: %d\n", type);
	}
	if (!done)	{
		try_ro = !tuner_flag;
		if (img->rw_cmap)
			get_x_colormap(img);
	}
    }
	if (verbose)	{
		msg("Created colormap with %d entries", img->lvls);
		if (!img->mono_img)
			msg(" per color, %d total", img->entries);
		mesg("\n");
	}
}


void
Setup_rw(XColor	*color_table, Image *img, int sysce, int free_pixels)
{
	Ext11Bottom = sysce;	/* saved sys color entries, not used yet */
	firstmap = 0;	/* force to XQ user color tabel	*/
	ncolors = free_pixels;
	rw_set_id = img;
	if (save_1st_cc)	{
	    firstmap = img->colormap;
	    memcpy(save_1st_cc, color_table, sizeof(XColor)*free_pixels);
	    if (img->mono_color)
		XQueryColors(img->dpy, firstmap, save_1st_cc, ncolors);
	}
}

static	x_bool
init_separate_color_rw (image_information*	img)
{
/* use XAllocColorPlanes to allocate all the cells we need --
* this makes it simple.	*/

#ifdef THIS_CODE_IS_NEVER_CALLED
	pixel_table = map = color_defs = NULL;

	DEBUGMESG("In init_separate_color_rw \n");

    for (log2_num_lvls = log2_levels;
	 log2_num_lvls >= 1;
	 log2_num_lvls--) {

	num_lvls = 1 << log2_num_lvls;
	total_levels =  1 << (log2_num_lvls * 3);

	if (map == NULL &&
		!(map = (int *) NZALLOC(num_lvls, sizeof(*map), No)) ||
	    !color_defs &&
		!(color_defs=(XColor*) NZALLOC(num_lvls, sizeof(XColor), No)))
		continue;

	if (XAllocColorPlanes(img->dpy, img->colormap, 1, &pixel_base, 1,
				log2_num_lvls, log2_num_lvls, log2_num_lvls,
				&red_mask, &green_mask, &blue_mask) == 0)
		continue;

	if (log2_num_lvls == 8) img->dither_img = False;

	bwdithermap(num_lvls, display_gamma, map, divN, modN, dm16);

	red_shift = shift_match_right(red_mask);
	green_shift = shift_match_right(green_mask);
	blue_shift = shift_match_right(blue_mask);

	/*	Set up the color map entries.	*/

	for (i=0; i < num_lvls; i++) {
		color_defs[i].pixel = SHIFT_MASK_PIXEL(i, i, i);

		color_defs[i].red = color_defs[i].green =
		color_defs[i].blue  = map[i] << 8;

		color_defs[i].flags = DoAll;
	}

	XStoreColors(img->dpy, img->colormap, color_defs, num_lvls);

	if (img->lvls > num_lvls) {
	    img->lvls = num_lvls;
	    img->lvls_squared = num_lvls * num_lvls;
	    log2_levels = log2_num_lvls;
	}

	CFREE(map);
	CFREE(color_defs);
	return	True;
    }
#endif	/* THIS_CODE_IS_NEVER_CALLED */
return	False;
}

/*
* Used for allocating the colormap in a PsuedoColor visual or other color
* type.	Used for the standard 8 bit display
*/
static	x_bool
init_color_rw(image_information *img)
{
int	num_lvls, *map;
XColor	*color_defs, *color;
int	need_entries,
	red_index,
	green_index,
	blue_index,
	i, j, free_pixels=0, shift=img->dpy_depth, cmap_size=1<<shift;
Pixel	*pixels;	/* get free pixels from the default colormap */

DEBUGMESG("init_color_rw\n");

/* get all free cells in the colormap in <depth> calls to AllocColor */
    pixels = (Pixel *) NZALLOC(cmap_size, sizeof(*pixels), No);
    color_defs = (XColor *) NZALLOC(cmap_size, sizeof(*color_defs), No);
    if (pixels && color_defs)	{
    	num_lvls = img->lvls;
	need_entries =  num_lvls * num_lvls * num_lvls;
	while (shift--)	{
		if (XAllocColorCells(img->dpy, img->colormap, False, 0, 0,
				&pixels[free_pixels], 1 << shift)) {
			DEBUGMESG("1");
			free_pixels += (unsigned) 1 << shift;
		} else	{
			DEBUGMESG("0");
			if (need_entries > (1 << shift) + free_pixels)
				num_lvls--,
			need_entries =  num_lvls * num_lvls * num_lvls;
		}
		if (free_pixels >= need_entries)	break;
	}
	DPRINTF(stderr, " got %d\n", free_pixels);
    }

    if (free_pixels < 8 || !num_lvls)	{	/* it was a bug here */
	CFREE(color_defs);
	CFREE(pixels);
	return	free_unique_colors(img, pixels, free_pixels);
    }

    if (img->lvls > num_lvls) {
	img->lvls = num_lvls;
	img->lvls_squared = num_lvls * num_lvls;
	ceiling_log2(i, num_lvls, log2_levels);
    }

    if (num_lvls == 256)
	img->dither_img = False;

    if (!img->pixel_table)
	img->pixel_table = (Pixel *) NZALLOC(free_pixels, sizeof(Pixel), "rw_pxl");

    map = (int *) NZALLOC(num_lvls, sizeof(*map), "rw-map");

    get_dither_arrays(img);
    bwdithermap(num_lvls, display_gamma, map,
		img->divN, img->modN, img->dm16);

	/*
	* Use the top free_pixels NOT the bottom ones.  This makes it right for
	* 99% of the DISPLAYs out there which begin using colors at the bottom.
	* We take the top color cells, and copy the bottom ones into the extra
	* spaces, *some* other windows won't be clobbered new colormap.
	*/

	/* set up local colormap indices	*/
	if (shift = free_pixels - need_entries)
	memcpy(img->pixel_table + need_entries, pixels, sizeof(Pixel) * shift);
	memcpy(img->pixel_table, pixels + shift, sizeof(Pixel) * need_entries);

	color = color_defs;
	red_index = green_index = blue_index = 0;

    {	register Colormap	dftcmap = DefaultCMAP(img->dpy);
	if (dftcmap != img->colormap && shift)	{
		for (i=0; i < shift; i++)/* get some global colormap entries */
			color_defs[i].pixel = pixels[i];
		XQueryColors(img->dpy, dftcmap, color_defs, i);
		color += i;	/* i == shift	*/
	} else	need_entries += shift;
    }
	/*	Set up the color map entries.	*/

	for (i=0; i < need_entries; i++, color++) {
		color->pixel = img->pixel_table[i];
		color->red   = map[red_index] << 8;
		color->green = map[green_index] << 8;
		color->blue  = map[blue_index] << 8;
		color->flags = DoAll;

		if (++red_index >= num_lvls) {
		    if (++green_index >= num_lvls) {
			++blue_index;
			green_index = 0;
		    }
		    red_index = 0;
		}
	}

    XStoreColors(img->dpy, img->colormap, color_defs, free_pixels);
	img->entries = free_pixels;
    if (!rw_set_id)
	Setup_rw(color_defs, img, shift, free_pixels);

    CFREE(map);
    CFREE(color_defs);
    CFREE(pixels);
return	True;
}

static	x_bool
init_separate_color_ro(image_information *img, int try_ro)
{
register int	num_lvls, log2_num_lvls;
register int	*map = NULL, i;

DEBUGMESG("init_separate_color_ro\n");

    for (log2_num_lvls = log2_levels; log2_num_lvls > 0; log2_num_lvls--) {

	num_lvls = 1 << log2_num_lvls;

	if (!map && !(map = (int *) NZALLOC(num_lvls, sizeof(*map), No)))
		continue;

	if (num_lvls == 256) img->dither_img = False;

	get_dither_arrays(img);
	bwdithermap(num_lvls, display_gamma, map,
			img->divN, img->modN, img->dm16);

	red_mask   = img->dpy_visual->red_mask;
	green_mask = img->dpy_visual->green_mask;
	blue_mask  = img->dpy_visual->blue_mask;

	red_shift   = shift_match_left(red_mask, log2_num_lvls);
	green_shift = shift_match_left(green_mask, log2_num_lvls);
	blue_shift  = shift_match_left(blue_mask, log2_num_lvls);

	if (img->lvls > num_lvls) {
	    img->lvls = num_lvls;
	    img->lvls_squared = num_lvls * num_lvls;
	    log2_levels = log2_num_lvls;
	}

	/* pack into a pixel the pre shifted and maksed pixel values */
	if (img->mono_color) {
	    img->pixel_table = (Pixel *)NZALLOC(img->cmaplen, sizeof(Pixel), No);
	    if (!img->pixel_table)	continue;
	    for (i=0; i < img->cmaplen; i++)
		img->pixel_table[i] =
			SHIFT_MASK_PIXEL(img->in_cmap[RLE_RED][i],
					img->in_cmap[RLE_GREEN][i],
					img->in_cmap[RLE_BLUE][i]);
	}

	if (map)	CFREE(map);
	return	True;
    }
return	False;
}

static int
pixcompare(Pixel *pixel1, Pixel *pixel2)
{
	return	(*pixel1 - *pixel2);
}

free_unique_colors(image_information *img, Pixel *pixels, int	npixels)
{
Pixel	last, *p;
register int	i, nunique;

    if (no_color_ref_counts) {	/* never used	*/
	qsort(pixels, npixels, sizeof(Pixel), pixcompare);
	p = pixels;

	for (i=1; i<npixels; i++)
		if (pixels[i] != *p)
			*++p = pixels[i];

	nunique = (p - pixels) + 1;

	if (DEBUGANY)	{
		fprintf(stderr, "In free_unique_pixels \n\nPixels: ");
		for (i=0;i<nunique;i++) fprintf(stderr, " %d ",pixels[i]);
		fprintf(stderr, "\n");
	}
    }
    else	nunique = npixels;

    XFreeColors(img->dpy, img->colormap, pixels, nunique, i=0);
return	i;
}

static	x_bool
init_color_ro(image_information *img, int	try_ro)
{
register int	num_lvls,
		need_entries;
XColor		color_def;
register XColor	*color_defs = NULL;
register Status *status = NULL;
register int	red_index,
		green_index,
		blue_index;
register int	*map = NULL;
register int	i, j=MIN((1<<img->lvls), specified_levels);
int	lowest = 6;	/* max 216 in Psuedo color */
	while (1<<lowest > j)	lowest--;	/* same as line 775	*/

    DEBUGMESG("init_color_ro\n");

    for (num_lvls = img->lvls; num_lvls >= 2; num_lvls--) {

	if (try_ro && specified_levels && num_lvls < lowest)	break;

	/* try to dither at lowest levels	*/
	need_entries = num_lvls * num_lvls * num_lvls;

	if (!img->pixel_table &&
	    !(img->pixel_table=(Pixel *)NZALLOC(need_entries, sizeof(Pixel), No))
		||
		!map && !(map = (int *) NZALLOC(num_lvls, sizeof(*map), No)))
		continue;

	if (num_lvls==256) img->dither_img = False;	/* 16M colors	*/

	get_dither_arrays(img);
	bwdithermap(num_lvls>>(VCTEntry>224), display_gamma, map,
			img->divN, img->modN, img->dm16);

	/* try to get a color map entry for each color. */
	red_index = green_index = blue_index = 0;

#ifdef XLIBINT_H_NOT_AVAILABLE
	{	register int	close=0;
	if (VCTEntry>132)	lastmap=close;
	for (i=j=close; i < need_entries; i++) {
	    color_def.red	= map[red_index] << 8;
	    color_def.green	= map[green_index] << 8;
	    color_def.blue	= map[blue_index] << 8;

	/* we use { close || } to enforce all allocated cell in contiguous
	*  and at the beginning of the pixel_table, so we can free them easy.
	*  The drawback is the CloseColor is pointed to, but not refered to
	*  the Colormap Entries. Once the application allocated these entries
	*  quits, then, our color may screwed up by new applications.
	*  Same as the init_mono_ro(). If we want to stay longer, then, ... */
	    if (close || ! XAllocColor(img->dpy, img->colormap, &color_def))
		if ((color_def.pixel = GetCloseColor(img->dpy, img->colormap,
			256, NULL, map[red_index], map[green_index], map[blue_index]))
			>= MaxColors)	break;
		else	close++;
	    else	j++;

	    if (++red_index >= num_lvls) {
		if (++green_index >= num_lvls) {
			++blue_index;	green_index = 0;
		}
		red_index = 0;
	    }
	    img->pixel_table[j + close - 1] = color_def.pixel;
	}

	/* Check if the colors are available */
	if (j+close < need_entries) {	/* Free the colors already obtained */
		free_unique_colors(img, img->pixel_table, j);
		continue;
	}
	img->entries = j	/* + close - no good for freecolor	*/;
	}	/* end close	*/
#else
	if (!color_defs)
	    color_defs = (XColor *) NZALLOC(need_entries, sizeof(XColor), No);
	if (!status)
	    status = (Status *) NZALLOC(need_entries, sizeof(Status), No);
	if (!status || !color_defs)	continue;

	for (i=0; i < need_entries; i++) {
	    color_defs[i].red   = map[red_index] << 8;
	    color_defs[i].green = map[green_index] << 8;
	    color_defs[i].blue  = map[blue_index] << 8;
	    if (++red_index >= num_lvls) {
		if (++green_index >= num_lvls) {
			++blue_index;	green_index = 0;
		}
		red_index = 0;
	    }
	}

	if (XAllocColors(img->dpy, img->colormap, color_defs, need_entries,
			status) > 0)	{
		for (i=j=0; i < need_entries; i++)
			if (status[i])
				img->pixel_table[j++] = color_defs[i].pixel;
		free_unique_colors(img, img->pixel_table, j);
		continue;
	} else	for (i=0; i < need_entries; i++)
		img->pixel_table[i] = color_defs[i].pixel;

#endif
	img->lvls_squared = num_lvls * num_lvls;
	img->lvls = num_lvls;
	ceiling_log2(i, num_lvls, log2_levels);

	if (map)	CFREE(map);
	if (status)	CFREE(status);
	if (color_defs)	CFREE(color_defs);
	return	True;
    }
    if (map)	CFREE(map);
    if (status)	CFREE(status);
    if (color_defs)	CFREE(color_defs);
return	False;
}

static	x_bool
init_mono_rw(image_information	*img)
{
Colormap	dftcmap = DefaultCMAP(img->dpy);
int	i, *map, num_lvls = img->mono_color ? img->cmaplen : img->lvls,
	free_pixels=0, cmap_size, shift=img->dpy_depth;
XColor	*color_defs, *color;
Pixel	*pixels;

    DEBUGMESG("In init_mono_rw\n");

    /* get free pixels from the default colormap */
    cmap_size = (1 << shift);

    /* get all free cells in the colormap in <depth> calls to AllocColor */
    pixels = (Pixel *) NZALLOC(cmap_size, sizeof(*pixels), "mrw-pxl");

	i = min_bits(num_lvls - 1) + 1;
	if (shift > i || img->mono_color)	shift = i;
	while (shift--)	{
		if (XAllocColorCells(img->dpy, img->colormap, False, 0, 0,
				pixels+free_pixels, 1 << shift)) {
			DEBUGMESG("1");
			free_pixels += 1 << shift;
		} else	{
			DEBUGMESG("0");
			if (num_lvls > free_pixels + (1 << shift)
				&& !img->mono_color)
				num_lvls -= (num_lvls > 16) ? i : 1;
		}
		if (free_pixels > num_lvls)	break;
		if (free_pixels == num_lvls)
			if (dftcmap == img->colormap)	break;
			else if (shift > 4)	shift = 4;	/* avg sys CE */
	}
	DPRINTF(stderr, " got %d\n", free_pixels);

    if (img->mono_color) {
	if (free_pixels < img->cmaplen)	/* c_free > 94% c_need for mono C */
	    if (img->cmaplen - free_pixels >= free_pixels>>4)	{
		CFREE(pixels);
		return	free_unique_colors(img, pixels, free_pixels);
	    } else
		prgmerr(0, "Warning: Not enough cells for input cmap\n");
	else	num_lvls = img->cmaplen;
    }

    if (!free_pixels)	return 	False;
/*
    while (num_lvls > free_pixels && num_lvls > 1)
	num_lvls > 16 ? num_lvls >>= 1 : num_lvls--;
*/
    if (num_lvls > free_pixels)
	num_lvls = free_pixels - (dftcmap != img->colormap ? i : 0);
    img->lvls_squared = num_lvls * num_lvls;
    img->lvls = num_lvls;
    ceiling_log2(i, num_lvls, log2_levels);

    if (num_lvls == 256 && img->mono_color)
	img->dither_img = False;

    if (!img->pixel_table)
	img->pixel_table = (Pixel *) NZALLOC(free_pixels, sizeof(Pixel), No);
    map = (int *) NZALLOC(num_lvls, sizeof(*map), No);

    if (!map | !img->pixel_table) {
	mesg("malloc problems in init_mono_rw\n");
	return	False;
    }

    get_dither_arrays(img);
    bwdithermap(num_lvls, display_gamma, map, img->divN, img->modN, img->dm16);

    color_defs = (XColor *) NZALLOC(cmap_size, sizeof(*color_defs), "mrw-xc");
	/*
	*	same as in init_color_rw()
	*/
	color = color_defs;
	shift = free_pixels - num_lvls;

    {	register int	j=i=0;
	if (dftcmap != img->colormap && shift)	{
		for (; j < shift; j++)	/* get some global colormap entries */
			color_defs[j].pixel = pixels[j];
		XQueryColors(img->dpy, dftcmap, color_defs, j);
		color += j;	/* j == shift	*/
	} else	num_lvls += shift;

	if (j)	/*	save system entries; first js	*/
	memcpy(img->pixel_table + num_lvls, pixels, sizeof(*pixels) * j);
	/*	set up local colormap indices	*/
	memcpy(img->pixel_table, pixels + j, sizeof(*pixels) * num_lvls);
    }
	/*	Set up the color map entries	*/

    if (img->mono_color)
	for (; i < num_lvls; i++, color++) {
	    color->pixel = img->pixel_table[i];
	    color->red = img->in_cmap[RLE_RED][i] << 8;
	    color->green = img->in_cmap[RLE_GREEN][i] << 8;
	    color->blue = img->in_cmap[RLE_BLUE][i] << 8;
	    color->flags = DoAll;
	}
    else
	for (; i < num_lvls; i++, color++) {
	    color->pixel = img->pixel_table[i];
	    color->red = color->green = color->blue = map[i] << 8;
	    color->flags = DoAll;
	}

    XStoreColors(img->dpy, img->colormap, color_defs, free_pixels);
	img->entries = free_pixels;
    if (!rw_set_id)
	Setup_rw(color_defs, img, shift, free_pixels);

    CFREE(map);
    CFREE(color_defs);
    CFREE(pixels);
return	True;
}

static	x_bool
init_mono_ro(image_information *img, int try_ro)
{
register int	num_lvls;
XColor		color_def;
register XColor	*color_defs = NULL;
register Status *status = NULL;

register int	*map = NULL;
register int	i, j;
int	lowest = MIN_COLOR_LEVELS;

    DPRINTF(stderr, "In init_mono_ro %s\n", try_ro ? "trying read/only" : "");

	if (img->mono_color)
		img->lvls = img->cmaplen;
	j = MIN(img->lvls, specified_levels);
	at_least_n(img->lvls, 2);

	while (1<<lowest > j)	lowest--;

    for (num_lvls = img->lvls; num_lvls > 1;
	 num_lvls = (num_lvls > 16) ? num_lvls >> 1 : num_lvls - 1) {

	if (num_lvls < 1 << lowest &&
		(try_ro && specified_levels || img->mono_color))	break;

#ifdef	MODIFY_X_MONO_RO
	j = MAX(img->tmp_offset*MIN(num_lvls, 255-VCTEntry), X_MIN_LEVELS);
#else
	j = num_lvls;
#endif
	if (img->pixel_table == NULL &&
	    !(img->pixel_table = (Pixel *) NZALLOC(j, sizeof(Pixel), No))
		|| !map && !(map = (int *) NZALLOC(j, sizeof(*map), No)))
		continue;

	if (num_lvls == 256 || img->mono_color)
	    img->dither_img = False;

	get_dither_arrays(img);
	bwdithermap(j, display_gamma, map, img->divN, img->modN, img->dm16);

	/* try to get a color map entry for each color.  */

#ifdef XLIBINT_H_NOT_AVAILABLE
	{	register int	close = 0;
	if (VCTEntry>132)	lastmap=close;
	for (i=j=close; i < num_lvls; i++) {
	    if (img->mono_color) {
		color_def.red	= img->in_cmap[RLE_RED][i] << 8;
		color_def.green	= img->in_cmap[RLE_GREEN][i] << 8;
		color_def.blue	= img->in_cmap[RLE_BLUE][i] << 8;
	    } else {
		color_def.red	=
		color_def.green	=
		color_def.blue	= map[i] << 8;
	    }

	    if (close || ! XAllocColor(img->dpy, img->colormap, &color_def))
		if ((color_def.pixel = GetCloseColor(img->dpy, img->colormap,
			MaxColors, NULL, color_def.red>>8,
			color_def.green>>8, color_def.blue>>8)) >= MaxColors)
			continue;
		else	close++;
	    else	j++;
	    img->pixel_table[j+close-1] = color_def.pixel;
	}

	/* Check if the colors are available */

	if (j+close < num_lvls) {	/* Free the colors already obtained */
		if (num_lvls-j+close > j+close>>4)	{
			free_unique_colors(img, img->pixel_table, j);
			continue;	/* adjust level & repeat */
		}
		else	prgmerr(0, "Warning: Not enough cells for cmap\n");
	}
	img->entries = j	/* + close; for total colors	*/;
	}	/* end close	*/
#else
	if (!color_defs)
		color_defs = (XColor *) NZALLOC(num_lvls, sizeof(XColor), No);
	if (!status)
		status = (Status *) NZALLOC(num_lvls, sizeof(*status), No);
	if (!status || !color_defs)	continue;

	for (i=0; i < num_lvls; i++) {
	    color_defs[i].pixel = 0;
	    if (img->mono_color) {
		color_defs[i].red   = img->in_cmap[RLE_RED][i] << 8;
		color_defs[i].green = img->in_cmap[RLE_GREEN][i] << 8;
		color_defs[i].blue  = img->in_cmap[RLE_BLUE][i] << 8;
	    } else {
		color_defs[i].red   =
		color_defs[i].green =
		color_defs[i].blue  = map[i] << 8;
	    }
	}

	if (XAllocColors(img->dpy, img->colormap, color_defs, num_lvls, status) > 0)
	{
		for (i=j=0; i < num_lvls; i++)
		    if (status[i])
			img->pixel_table[j++] = color_defs[i].pixel;
		free_unique_colors(img, img->pixel_table, j);
		continue;
	} else	for (i=0; i < num_lvls; i++)
			img->pixel_table[i] = color_defs[i].pixel;

#endif
	img->lvls = num_lvls;
	img->lvls_squared = num_lvls * num_lvls;
	ceiling_log2(i, num_lvls, log2_levels);

	CFREE(map);
	if (status)	CFREE(status);
	if (color_defs)	CFREE(color_defs);
	return	True;

    }
    if (img->pixel_table)
	CFREE(img->pixel_table);
    img->pixel_table = NULL;

    if (map)	CFREE(map);
    if (status)	CFREE(status);
    if (color_defs)	CFREE(color_defs);
return	False;
}

#ifndef XLIBINT_H_NOT_AVAILABLE

#define NEED_REPLIES
#include <X11/Xlibint.h>

static int	counter;
static Status	*st;
static Status	status;

static
dummy_handle_x_errors(Display	*dpy, XErrorEvent *event)
{
	st[counter++] = 0;
	status = False;
}


Status
XAllocColors(register Display *dpy, Colormap	cmap,
		XColor	*defs, int	ndefs, Status	*statuses)
{
xAllocColorReply	rep;
register xAllocColorReq	*req;
int	i, (*function)();
Status	return_status;

    XSync(dpy, False);

    function = XSetErrorHandler( dummy_handle_x_errors );
    st = statuses;

    LockDisplay(dpy);

    for (i = 0; i < ndefs; i++)    {
	GetReq(AllocColor, req);

	req->cmap = cmap;
	req->red = defs[i].red;
	req->green = defs[i].green;
	req->blue = defs[i].blue;
    }

    status = True;
    for (counter = 0; counter < ndefs; counter++)
    {
	statuses[counter] = _XReply(dpy, (xReply *) &rep, 0, xTrue);
	if (statuses[counter]) {
	    defs[counter].pixel = rep.pixel;
	    defs[counter].red = rep.red;
	    defs[counter].green = rep.green;
	    defs[counter].blue = rep.blue;
	}
	else
	    status = False;
    }
    return_status = status;

    UnlockDisplay(dpy);
    SyncHandle();

    XSetErrorHandler(function);

return	return_status;
}
#endif /* !XLIBINT_H_NOT_AVAILABLE */
