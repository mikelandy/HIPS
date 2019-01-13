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
*/
/*
* map_scan.c - Put RLE images on X display.
*
* Author:	Spencer W. Thomas  (x10)
*		Computer Science Dept.
*		University of Utah
* Date:	Thu Feb 20 1986
* Copyright (c) 1986, University of Utah
*
* Modified:	Andrew F. Vesper (x 11)
*		High Performance Workstations
*		Digital Equipment Corp
* Date:	Fri, Aug 7, 1987
*		Thu, Jan 14, 1988
* Copyright (c) 1987,1988, Digital Equipment Corporation
*
* Modified:	Martin R. Friedmann (better X11, flipbook, MAG, info)
* 		Dept of Electrical Engineering and Computer Science
*		University of Michigan
* Date:	Tue, Nov 14, 1989
* Copyright (c) 1989, University of Michigan
*
* Modified:	Jin, Guojun (x11)
*		IMAGE Technology Group
*		Lawrence Berkeley Laboratory
* Date: Mon, Oct 1, 1990
#		change dpy from global to local.
#		It is also made in entire libpanel.
$	Thu, Feb 6, 1992
#		Add start_col {x} in all map_XXX_??dither_??table_#()
#		routines for fast painting in small region.
#		Add shrink (-MAG)
* Copyright (c)	1990, 1992, Lawrence Berkeley Laboratory
*/

#include "panel.h"

#ifndef	DEFAULT_GAMMA
#define	DEFAULT_GAMMA	1.0	/*	no gamma correction	*/
#endif

double	display_gamma = DEFAULT_GAMMA;	/* 2.5 fits most NTSC monitors	*/
int	red_shift,
	green_shift,
	blue_shift;
Pixel	red_mask,
	green_mask,
	blue_mask;

static void	map_scanline_generic();
static void	map_1_dither_table_1(),
		map_1_dither_table_8(),
		map_1_nodither_table_8(),
		map_2or3_dither_table_8(),
		map_1_nodither_notable_32(),
		map_2or3_nodither_table_8(),
		map_2or3_dither_notable_32(),
		map_2or3_nodither_notable_32();
static void	map_1_mono_color_8(),
		map_1_mono_color_32();
static void	MAG_scanline_generic();
static void	MAG_1_dither_table_1(),
		MAG_1_dither_table_8(),
		MAG_1_nodither_table_8(),
		MAG_1_nodither_notable_32();
static void	MAG_2or3_dither_table_8(),
		MAG_2or3_nodither_table_8(),
		MAG_2or3_dither_notable_32(),
		MAG_2or3_nodither_notable_32();
static void	MAG_1_mono_color_8(),
		MAG_1_mono_color_32();

static byte	LSBMask[9] = {	0x0, 0x1, 0x3, 0x7,
				0xf, 0x1f, 0x3f, 0x7f, 0xff };
#if EVEN_NEVER_CALLED	/* Not actually used	*/
static byte	MSBMask[9] = {	0x00, 0x80, 0xc0, 0xe0,
				0xf0, 0xf8, 0xfc, 0xfe, 0xff };
#endif	EVEN_NEVER_CALLED


/*
* Figure out which scan line routine to use.
*/

void
choose_scanline_converter(img)
register Image	*img;
{
static struct mst_t	{
	x_bool	dither,
		table_present;
	int	bpp;
	void	(*routine)(),
		(*mag_routine)();
} map_scanline_table[] =  {
    /* dithr, Table, b/p, routine ptr */
    {	True, True, 1, map_1_dither_table_1,	MAG_1_dither_table_1},
    {	True, True, 8, map_1_dither_table_8,	MAG_1_dither_table_8},
    {	False,True, 8, map_1_nodither_table_8,	MAG_1_nodither_table_8},
    {	False,False,32,map_1_nodither_notable_32,	MAG_1_nodither_notable_32},
    {	True, True, 8, map_2or3_dither_table_8,	MAG_2or3_dither_table_8},
    {	False,True, 8, map_2or3_nodither_table_8, MAG_2or3_nodither_table_8},
    {	True, False,32,map_2or3_dither_notable_32,MAG_2or3_dither_notable_32},
    {	False,False,32,map_2or3_nodither_notable_32,MAG_2or3_nodither_notable_32}
  };

    img->map_scanline = map_scanline_generic;
    img->MAG_scanline = MAG_scanline_generic;

    if (img->mono_color)
	switch (img->image->bits_per_pixel) {
	case 32:
	    img->map_scanline = map_1_mono_color_32;
	    img->MAG_scanline = MAG_1_mono_color_32;
	    break;
	case 8:
	    img->map_scanline = map_1_mono_color_8;
	    img->MAG_scanline = MAG_1_mono_color_8;
	    break;
	default:	prgmerr(0, "Warning: Bits per pixel = %d\n",
				img->image->bits_per_pixel);
	} else	{
	register x_bool table_present = (img->pixel_table != NULL);
	register struct mst_t*	mstp = map_scanline_table;
	register int	i = (!img->mono_img) << 2;

	    for (; i < COUNT_OF(map_scanline_table); i++) {
		if (mstp[i].dither == img->dither_img &&
		    mstp[i].table_present == table_present &&
		    mstp[i].bpp == img->image->bits_per_pixel) {
		    img->map_scanline = mstp[i].routine;
		    img->MAG_scanline = mstp[i].mag_routine;

		    if (DEBUGANY) {
			message("Special map_scanline routine used: map_");

			fputs(img->mono_img ? "1_" : "2or3", stderr);
			fputs(img->dither_img ? "dither_" : "no", stderr);

			if (! table_present) fputs ("no", stderr);
			message("table_%d (index %d)\n",
				img->image->bits_per_pixel, i);
		    }
		    break;
		}
	    }
	}
}

/*
 * Map a scanline through the dither matrix.
 *
 * Inputs:
 *	rgb:		Pointers to buffers containing the red, green,
 *			and blue color rows.
 *	n:		Length of row.
 *	s:		Skip between pixels in original image.
 *	y:		Y position of row (necessary for dither)
 *	line:		Pointer to output buffer for dithered color data.
 */

#define DMAP(v,x,y)	(divN[v] + (modN[v]>dm16[x][y] ? 1 : 0))
#define NDMAP(v)	(divN[v])

#define DMAP_SETUP(img) \
	register int *divN = img->divN;\
	register int *modN = img->modN;\
	register array16 *dm16 = img->dm16

#define NDMAP_SETUP(img)	register int *divN = img->divN

#define IN_CMAP_SETUP(img)	register rle_pixel **in_cmap = img->in_cmap

#define LEVELS_SETUP(img) \
	register int levels = img->lvls; \
	register int levels_squared = img->lvls_squared


static void
map_scanline_generic (img, rgb, ncolors, given_width, stride, y, x, image)
image_information *img;
byte	*rgb[3];
int	ncolors,
	given_width,
	stride, y;
register int	x;
XImage	*image;
{
register Pixel *pixel_table = img->pixel_table;
DMAP_SETUP(img);
LEVELS_SETUP(img);
register int	col, row;
register byte *r;
register long	pixel;
register int	width = given_width;

    row = y & 0xF;
    r = rgb[col=0];

    if (ncolors == 1) {

	if (img->dither_img) {
	    for (; width-- > 0; r += stride, col = ((col + 1) & 15), x++) {
		pixel = pixel_table [DMAP(*r, col, row)];
		if (pixel) pixel = 0xffffffff;
		XPutPixel (image, x, y, pixel);
	    }
	}
	else {
	    for (; width--; r += stride, x++) {
		pixel = pixel_table [NDMAP(*r)];
		XPutPixel (image, x, y, pixel);
	    }
	}
    }

    else {
	register byte *g, *b;

	g = b = rgb[1];
	if (ncolors >= 3) b = rgb[2];

	if (img->dither_img) {
	    for (; width-- > 0; r += stride, g += stride, b += stride,
		 col = ((col + 1) & 15), x++)
	    {
		if (pixel_table != NULL)
		    pixel = pixel_table
			[DMAP(*r, col, row) +
			 DMAP(*g, col, row) * levels +
			 DMAP(*b, col, row) * levels_squared];

		else pixel = SHIFT_MASK_PIXEL (DMAP(*r, col, row),
					       DMAP(*g, col, row),
					       DMAP(*b, col, row));

		XPutPixel (image, x, y, pixel);
	    }

	}
	else {

	    for (; width-- > 0; r += stride, g += stride, b += stride, x++) {
		if (pixel_table != NULL) {
		    pixel = pixel_table
			[NDMAP(*r) +
			 NDMAP(*g) * levels +
			 NDMAP(*b) * levels_squared];
		}
		else {
		    pixel = SHIFT_MASK_PIXEL (NDMAP(*r), NDMAP(*g), NDMAP(*b));
		}
		XPutPixel (image, x, y, pixel);
	    }
	}
    }	
}

static void
MAG_scanline_generic(img, rle_x, rle_y, mag_size, x, y, width, height, image)
image_information *img;
int	rle_x, rle_y,
	width, height,
	mag_size,
	x, y;
XImage	*image;

{
register Pixel *pixel_table = img->pixel_table;
DMAP_SETUP(img);
LEVELS_SETUP(img);
register int 	col;
register byte	*r;
int	save_x = x,
	mag_y, mag_f = abs(mag_size),
	x_mod_16 = (mag_f * rle_x) & 0xF,
	rgb_line_stride = img->w * img->dpy_channels;
register int 	mag_x, w, row = (mag_f * rle_y - 1) & 0xF;
byte	*rgb_line = SAVED_RLE_ROW(img, rle_y) + rle_x;
register unsigned long	pixel;
    while (height-- > 0) {
	x = save_x;
	w = width;
	row = (row + 1) & 0xF;	/* = % 16; change through this file */
	col = x_mod_16;
	r = rgb_line;
	mag_x = mag_f;

	if (img->dpy_channels == 1) {

	    if (img->dither_img)
		while (w-- > 0)	{
		    pixel = pixel_table [DMAP(*r, col++, row)];
		    XPutPixel (image, x, y, pixel);
		    if (mag_size < 0)
			r += mag_x,	w -= mag_x - 1;
		    else if (--mag_x == 0)
			r++,	mag_x = mag_f;
		    col &= 15;	x++;
		}
	    else while (w-- > 0)	{
		    pixel = pixel_table [NDMAP(*r)];
		    XPutPixel (image, x, y, pixel);
		    if (mag_size < 0)
			r += mag_x,	w -= mag_x - 1;
		    else if (--mag_x == 0)
			r++,	mag_x = mag_f;
		    x++;
		}
	} else	{
	register byte	*g, *b;

	    g = b = r + img->w;
	    if (img->dpy_channels >= 3)	b += img->w;

	    if (img->dither_img)
		while (w-- > 0)
		{
		    if (pixel_table != NULL)
			pixel = pixel_table
			    [DMAP(*r, col, row) +
			     DMAP(*g, col, row) * levels +
			     DMAP(*b, col, row) * levels_squared];

		    else pixel =
			SHIFT_MASK_PIXEL (DMAP(*r, col, row),
					  DMAP(*g, col, row),
					  DMAP(*b, col, row));
		    XPutPixel (image, x++, y, pixel);
		    if (mag_size < 0)
			r += mag_x,	g += mag_x,	b += mag_x,
			w -= mag_x - 1;
		    else if (--mag_x == 0)	{
			r++, g++, b++;
			mag_x = mag_f;
		    }
		    col++;
		    col &= 15;
		}
	    else while (w-- > 0)	{
		    if (pixel_table != NULL)
			pixel = pixel_table
			    [NDMAP(*r) +
			     NDMAP(*g) * levels +
			     NDMAP(*b) * levels_squared];
		    else pixel = SHIFT_MASK_PIXEL (NDMAP(*r),
						   NDMAP(*g),
						   NDMAP(*b));

		    XPutPixel (image, x++, y, pixel);
		    if (mag_size < 0)
			r += mag_x,	g += mag_x,	b += mag_x,
			w -= mag_x - 1;
		    else if (--mag_x == 0)
		    {
			r++, g++, b++;
			mag_x = mag_f;
		    }
		}
	}
	if (mag_size < 0)
		rgb_line += rgb_line_stride * mag_x,
		height -= mag_x - 1;
	else if (--mag_y == 0)	{
	    rgb_line += rgb_line_stride;
	    mag_y = mag_f;
	}
	y++;
    }
}

/*
 * map_1_dither_table_8
 *
 * Inputs:
 *	rgb:		Pointers to buffers containing the red, green,
 *			and blue color rows.
 *	n:		Length of row.
 *	s:		Skip between pixels in original image.
 *	y:		Y position of row (necessary for dither)
 *	line:		Pointer to output buffer for dithered color data.
 */

static	void
map_1_dither_table_8(img, rgb, ncolors, given_width, stride, y, start_col, image)
image_information *img;
byte	*rgb[3];
int	ncolors,
	given_width,
	stride, y, start_col;
XImage	*image;
{
register Pixel	*pixel_table = img->pixel_table;
DMAP_SETUP(img);
register int	col, row;
register byte	*r, *pixel_ptr;
register int	width = given_width;

    row = y & 0xF;
    col = 0;
    r = rgb[0];

    pixel_ptr = ((byte *) image->data) + y * image->bytes_per_line + start_col;

    while (width-- > 0) {
	*pixel_ptr++ = pixel_table [ DMAP(*r, col, row) ];
	
	r += stride;
	col = ((col + 1) & 15);
    }
}

static	void
MAG_1_dither_table_8(img, rle_x, rle_y, mag_size, x, y, width, height, image)
image_information *img;
int	rle_x, rle_y,
	width, height,
	mag_size,
	x, y;
XImage	*image;
{
register Pixel	*pixel_table = img->pixel_table;
DMAP_SETUP(img);
register int 	col;
register byte	*r, *pixel_ptr;
register int 	mag, w;
int 		mag_f = abs(mag_size), wd = width / mag_f;
register int 	row = (mag_f * rle_y - 1) & 0xF;
int 	x_mod_16 = (mag_f * rle_x) & 0xF,
	rgb_line_stride = img->w * img->dpy_channels;
byte	*rgb_line = SAVED_RLE_ROW(img, rle_y) + rle_x,
	*line_ptr = ((byte *) image->data)+(y * image->bytes_per_line) + x;

	height /= mag_f;

    while (height--) {
	pixel_ptr = line_ptr;
	w = wd;
	row = (row + 1) & 0xF;
	col = x_mod_16;
	r = rgb_line;

	while (w--) {
		*pixel_ptr++ = pixel_table [DMAP(*r, col, row) ];
		mag = mag_f;
		if (mag_size > 0)	{
			r++;
			while (--mag)
				*pixel_ptr++ = pixel_ptr[-1];
		} else	r += mag;
		col = (col + mag_f) & 0xF;
	}
	mag = mag_f;
	while (--mag)
	    if (mag_size < 0)
		rgb_line += rgb_line_stride;
	    else	{
		line_ptr += image->bytes_per_line;
		memcpy(line_ptr, line_ptr - image->bytes_per_line, width);
	    }
	rgb_line += rgb_line_stride;
	line_ptr += image->bytes_per_line;
    }
}

/*
 * map_2or3_dither_table_8
 *
 * Dither three colors into an eight bit table...  This is faster than the
 * map_scanline_generic routine.
 *
 * Inputs:
 *	rgb:		Pointers to buffers containing the red, green,
 *			and blue color rows.
 *	n:		Length of row.
 *	s:		Skip between pixels in original image.
 *	y:		Y position of row (necessary for dither)
 *	line:		Pointer to output buffer for dithered color data.
 */

static	void
map_2or3_dither_table_8 (img, rgb, ncolors, given_width, stride, y, x, image)
image_information *img;
byte	*rgb[3];
int	ncolors,
	given_width,
	stride, y, x;
XImage	*image;

{
register Pixel *pixel_table = img->pixel_table;
DMAP_SETUP(img);
LEVELS_SETUP(img);
register int	col, row;
register byte	*r, *g, *b;
register byte	*pixel_ptr;
register int	width = given_width;

    row = y & 0xF;
    r = rgb[col=0];
    g = b = rgb[1];
    if (ncolors >= 3) b = rgb[2];

    pixel_ptr = (byte *) image->data + y * image->bytes_per_line + x;

    while (width-- > 0) {
	*pixel_ptr++ = pixel_table[DMAP(*r, col, row) +
				DMAP(*g, col, row) * levels +
				DMAP(*b, col, row) * levels_squared];
	r += stride;
	g += stride;
	b += stride;
	col++;	 col &= 0xF;
    }
}

static	void
MAG_2or3_dither_table_8 (img, rle_x, rle_y, mag_size, x, y, width, height, image)
image_information *img;
int	rle_x, rle_y,
	width, height,
	mag_size,
	x, y;
XImage	*image;

{
register Pixel	*pixel_table = img->pixel_table;
DMAP_SETUP(img);
LEVELS_SETUP(img);
int	mag_y, mag_f = abs(mag_size),
	x_mod_16 = (mag_f * rle_x) & 0xF,
	rgb_line_stride = img->w * img->dpy_channels;
register byte	*r, *g, *b, *pixel_ptr;
register int 	col, mag_x, w, row = (mag_f * rle_y - 1) & 0xF;
byte	*rgb_line = SAVED_RLE_ROW(img, rle_y) + rle_x,
	*line_ptr = (byte *) image->data + y * image->bytes_per_line + x;

	width /= (mag_y = mag_f);
	if (mag_size < 0)
		height /= mag_f;

    while (height--) {
	pixel_ptr = line_ptr;
	w = width;
	row = (row + 1) & 0xF;
	col = x_mod_16;

	r = rgb_line;
	g = b = r + img->w;
	if (img->dpy_channels >= 3) b += img->w;

	while (w--) {
	    *pixel_ptr++ = pixel_table [DMAP(*r, col, row) +
					DMAP(*g, col, row) * levels +
					DMAP(*b, col, row) * levels_squared];
	    col++;	col &= 0xF;
	    mag_x = mag_f;
	    if (mag_size < 0)	{
		r += mag_x,	g += mag_x,	b += mag_x;
	    } else	{
		while (--mag_x)	{
			*pixel_ptr++ = pixel_table [DMAP(*r, col, row) +
					DMAP(*g, col, row) * levels +
					DMAP(*b, col, row) * levels_squared];
			col++;
			col &= 0xF;
		}
		r++, g++, b++;
	    }
	}
	line_ptr += image->bytes_per_line;
	if (mag_size < 0)
		rgb_line += rgb_line_stride * mag_x;	/* mag_x = mag_f */
	else if (--mag_y == 0)	{
		rgb_line += rgb_line_stride;
		mag_y = mag_f;
	}
    }
}

/*
 * map_1_dither_table_1
 *
 * Dither three colors into an eight bit table...  This is faster than the
 * map_scanline_generic routine.
 *
 * Inputs:
 *	rgb:		Pointers to buffers containing the red, green,
 *			and blue color rows.
 *	n:		Length of row.
 *	s:		Skip between pixels in original image.
 *	y:		Y position of row (necessary for dither)
 *	line:		Pointer to output buffer for dithered color data.
 */

static	void
map_1_dither_table_1 (img, rgb, ncolors, given_width, stride, y, x, image)
image_information *img;
byte	*rgb[3];
int	ncolors, given_width;
register int	stride;
int		y, x;
XImage		*image;

{
register Pixel *pixel_table = img->pixel_table;
DMAP_SETUP(img);
register int	col, row;
register byte	*r;
register byte	*pixel_ptr;
register int	width = given_width;
int	bit;
    row = y & 0xF;
    col = bit = 0;
    r = rgb[0];

    pixel_ptr = ((byte *) image->data) + y * image->bytes_per_line + x;

    if (BitmapBitOrder(img->dpy) == MSBFirst) {
	    /* we don't want to trash those good bits */
	    *pixel_ptr >>= (8 - bit) & 7;

	    /* do first byte fragment */
	    while (col & 7)
	    {
		*pixel_ptr <<= 1;
		if (pixel_table[DMAP(*r, col++, row)])
		    *pixel_ptr |= (byte) 0x1;
		width--;
		r += stride;
	    }
	    if (bit)

		pixel_ptr++, col &= 15;

	    while ((width -= 8) >= 0) {
		*pixel_ptr = 0;
		if (pixel_table[DMAP(*r, col++, row)])
		    *pixel_ptr = (byte) 0x80;
		r += stride;
		if (pixel_table[DMAP(*r, col++, row)])
		    *pixel_ptr |= (byte) 0x40;
		r += stride;
		if (pixel_table[DMAP(*r, col++, row)])
		    *pixel_ptr |= (byte) 0x20;
		r += stride;
		if (pixel_table[DMAP(*r, col++, row)])
		    *pixel_ptr |= (byte) 0x10;
		r += stride;
		if (pixel_table[DMAP(*r, col++, row)])
		    *pixel_ptr |= (byte) 0x8;
		r += stride;
		if (pixel_table[DMAP(*r, col++, row)])
		    *pixel_ptr |= (byte) 0x4;
		r += stride;
		if (pixel_table[DMAP(*r, col++, row)])
		    *pixel_ptr |= (byte) 0x2;
		r += stride;
		if (pixel_table[DMAP(*r, col++, row)])
		    *pixel_ptr |= (byte) 0x1;
		r += stride;
		col &= 15;
		pixel_ptr++;
	    }

	    /* if w is non-zero then we have to finish up... */
	    width = 8 + width;
	    if (width)
	    {
		byte savebits;

		savebits = *pixel_ptr & LSBMask[8-width];
		bit = width;
		
		while (--width >= 0) {
		    *pixel_ptr <<= 1;
		    if (pixel_table [DMAP(*r, col++, row)])
			*pixel_ptr |= (byte) 0x1;
		    r += stride;
		}
		*pixel_ptr <<= 8 - bit;
		*pixel_ptr |= savebits;
	    }
	}
    else
	while (--width >= 0) {
	    *pixel_ptr = (byte)(*pixel_ptr >> 1) | (byte)
		((pixel_table[DMAP(*r, col, row)]!= 0) ? 0x80: 0);
	    
	    r += stride;
	    col = ((col + 1) & 15);
	    bit = ((bit + 1) & 7);
	    if (!bit) pixel_ptr++;
	}
}

#define INC_RGB(stmt)	if (mag_size < 0)	{ r += mag_x; w -= mag_x-1; }\
		else	if (--mag_x == 0) {stmt; mag_x = mag_f; }
static	void
MAG_1_dither_table_1 (img, rle_x, rle_y, mag_size, x, y, width, height, image)
image_information *img;
int	rle_x, rle_y,
	width, height,
	mag_size,
	x, y;
XImage	*image;
{
register Pixel *pixel_table = img->pixel_table;
DMAP_SETUP(img);
int	mag_f = abs(mag_size);
register int	col, bit;
register byte	*r;
register byte	*pixel_ptr;
register int	mag_x, w;
register int	row = (mag_f * rle_y - 1) & 0xF;
int	rgb_line_stride = img->w * img->dpy_channels,
	x_mod_16 = (mag_f * rle_x) & 0xF,
	x_mod_8 = x & 0x7,
	mag_y = mag_f,
	byte_order = BitmapBitOrder(img->dpy);
byte	*rgb_line = SAVED_RLE_ROW(img, rle_y) + rle_x,
	*line_ptr = (byte *) image->data + y * image->bytes_per_line + (x>>3);

    while (height-- > 0) {
	pixel_ptr = line_ptr;
	w = width;
	r = rgb_line;
	mag_x = mag_f;

	row = (row + 1) & 0xF;
	col = x_mod_16;
	bit = x_mod_8;

	if (byte_order == MSBFirst) {
	    /* we don't want to trash those good bits */
	    *pixel_ptr >>= (8 - bit) & 7;

	    /* do first byte fragment */
	    while (bit & 7)
	    {
		*pixel_ptr <<= 1;
		*pixel_ptr |= pixel_table[DMAP(*r, col++, row)] != 0;
		w--;
		INC_RGB(r++);
		bit++;
		col &= 15;
	    }
	    if (x_mod_8)
		pixel_ptr++;

	    /* do the bulk of the line fast in eight bit chunks Gee I hope all
	     * this fits into your instruction cache...  Or else we are
	     * forked..   You can get rid of 7 (col &= 15)'s if you make dm16
	     * a 32x16 array with duplicates in the second half of the columns.
	     * Then we don't have to worry about col overflowing in 8 ++'s
	     */
	    while ((w -= 8) >= 0) {
		*pixel_ptr = (byte) 0;
		if (pixel_table[DMAP(*r, col++, row)])
		    *pixel_ptr = (byte) 0x80;
		INC_RGB(r++);	col &= 15;
		if (pixel_table[DMAP(*r, col++, row)])
		    *pixel_ptr |= (byte) 0x40;
		INC_RGB(r++);	col &= 15;
		if (pixel_table[DMAP(*r, col++, row)])
		    *pixel_ptr |= (byte) 0x20;
		INC_RGB(r++);	col &= 15;
		if (pixel_table[DMAP(*r, col++, row)])
		    *pixel_ptr |= (byte) 0x10;
		INC_RGB(r++);	col &= 15;
		if (pixel_table[DMAP(*r, col++, row)])
		    *pixel_ptr |= (byte) 0x8;
		INC_RGB(r++);	col &= 15;
		if (pixel_table[DMAP(*r, col++, row)])
		    *pixel_ptr |= (byte) 0x4;
		INC_RGB(r++);	col &= 15;
		if (pixel_table[DMAP(*r, col++, row)])
		    *pixel_ptr |= (byte) 0x2;
		INC_RGB(r++);	col &= 15;
		if (pixel_table[DMAP(*r, col++, row)])
		    *pixel_ptr |= (byte) 0x1;
		INC_RGB(r++);	col &= 15;
		pixel_ptr++;
	    }

	    /* if w is non-zero then we have to finish up... */
	    w = 8 + w;
	    if (w)
	    {
		byte	savebits;

		savebits = *pixel_ptr & LSBMask[8-w];
		bit = w;

		while (w-- > 0) {
		    *pixel_ptr <<= 1;
		    *pixel_ptr |= (byte) pixel_table
			[DMAP(*r, col++, row)] ? 0x1 : 0;
		    INC_RGB(r++); col &= 15;
		}
		*pixel_ptr <<= 8 - bit;
		*pixel_ptr |= savebits;
	    }
	}
	else {
	    /* we don't want to trash those good bits */
	    *pixel_ptr <<= (8 - bit) & 7;

	    /* do first byte fragment */
	    while (col & 7) {
		*pixel_ptr >>= 1;
		if (pixel_table[DMAP(*r, col++, row)])
		    *pixel_ptr |= (byte) 0x80;
		w--;
		INC_RGB(r++);	col &= 15;
	    }
	    if (x_mod_8)
		pixel_ptr++;

	    /* do the bulk of the line fast in eight bit chunks.. */
	    while ((w -= 8) >= 0) {
		*pixel_ptr = (byte) 0x0;
		if (pixel_table[DMAP(*r, col++, row)])
		    *pixel_ptr = (byte) 0x1;
		INC_RGB(r++);	col &= 15;
		if (pixel_table[DMAP(*r, col++, row)])
		    *pixel_ptr |= (byte) 0x2;
		INC_RGB(r++);	col &= 15;
		if (pixel_table[DMAP(*r, col++, row)])
		    *pixel_ptr |= (byte) 0x4;
		INC_RGB(r++);	col &= 15;
		if (pixel_table[DMAP(*r, col++, row)])
		    *pixel_ptr |= (byte) 0x8;
		INC_RGB(r++);	col &= 15;
		if (pixel_table[DMAP(*r, col++, row)])
		    *pixel_ptr |= (byte) 0x10;
		INC_RGB(r++);	col &= 15;
		if (pixel_table[DMAP(*r, col++, row)])
		    *pixel_ptr |= (byte) 0x20;
		INC_RGB(r++);	col &= 15;
		if (pixel_table[DMAP(*r, col++, row)])
		    *pixel_ptr |= (byte) 0x40;
		INC_RGB(r++);	col &= 15;
		if (pixel_table[DMAP(*r, col++, row)])
		    *pixel_ptr |= (byte) 0x80;
		INC_RGB(r++);	col &= 15;
		pixel_ptr++;
	    }

	    /* if w is negative then we have to finish up... */
	    w = 0 - w;

	    if (w)
	    {
		byte	savebits = (byte)(*pixel_ptr >> 8 - w);
		savebits <<= (byte)8 - w;
		bit = w;

		while (w-- > 0) {
		    *pixel_ptr >>= 1;
		    if (pixel_table[DMAP(*r, col++, row)])
			*pixel_ptr |= (byte) 0x80;
		    INC_RGB(r++); col &= 15;
		}
		*pixel_ptr >>= 8 - bit;
		*pixel_ptr |= savebits;
	    }
	}
	if (mag_size < 0)
		rgb_line += rgb_line_stride * mag_y,
		height -= mag_y - 1;
	else if (--mag_y == 0)	{
	    rgb_line += rgb_line_stride;
	    mag_y = mag_f;
	}
	line_ptr += image->bytes_per_line;
    }
}

/*
 * map_2or3_nodither_table_8
 *
 * Dither three colors into an eight bit table...  This is faster than the
 * map_scanline_generic routine.
 *
 * Inputs:
 *	rgb:		Pointers to buffers containing the red, green,
 *			and blue color rows.
 *	n:		Length of row.
 *	s:		Skip between pixels in original image.
 *	y:		Y position of row (necessary for dither)
 *	line:		Pointer to output buffer for dithered color data.
 */

static	void
map_2or3_nodither_table_8 (img, rgb, ncolors, given_width, stride, y, x, image)
image_information *img;
byte	*rgb[3];
int	ncolors,
	given_width,
	stride,
	y, x;
XImage	*image;

{
register Pixel	*pixel_table = img->pixel_table;
NDMAP_SETUP(img);
LEVELS_SETUP(img);
register byte	*r, *g, *b;
register byte	*pixel_ptr;
register int	width = given_width;

    r = rgb[0];
    g = b = rgb[1];
    if (ncolors >= 3) b = rgb[2];

    pixel_ptr = ((byte *) image->data) + y * image->bytes_per_line + x;
    
    while (width-- > 0) {
	*pixel_ptr++ = pixel_table [  NDMAP(*r)
				    + NDMAP(*g) * levels
				    + NDMAP(*b) * levels_squared];
	r += stride;
	g += stride;
	b += stride;
    }
}
static	void
MAG_2or3_nodither_table_8(img, rle_x, rle_y, mag_size, x, y, width, height, image)
image_information *img;
int	rle_x, rle_y,
	width, height,
	mag_size,
	x, y;
XImage	*image;

{
register Pixel	*pixel_table = img->pixel_table;
NDMAP_SETUP(img);
LEVELS_SETUP(img);
register byte	*r, *g, *b;
register byte	*pixel_ptr;
register byte	table_value;
register int	mag, w;
int	mag_f = abs(mag_size), wd = width / mag_f,
	rgb_line_stride = img->w * img->dpy_channels;
byte	*rgb_line = SAVED_RLE_ROW(img, rle_y) + rle_x,
	*line_ptr = (byte *) image->data + y * image->bytes_per_line + x;

	height /= mag_f;
    while (height--) {
	pixel_ptr = line_ptr;
	w = wd;
	r = rgb_line;

	g = b = r + img->w;
	if (img->dpy_channels >= 3) b += img->w;

	while (w--) {
		*pixel_ptr++ =
		table_value = pixel_table[NDMAP(*r)
					+ NDMAP(*g) * levels
					+ NDMAP(*b) * levels_squared];
		mag = mag_f;
		if (mag_size < 0)
			r += mag,	g += mag,	b += mag;
		else	{
			while (--mag)
				*pixel_ptr++ = table_value;
			r++, g++, b++;
		}
	}
	mag = mag_f;
	if (mag_size < 0)
		rgb_line += rgb_line_stride * mag;
	else	{
	    while (--mag)	{
		memcpy(line_ptr+image->bytes_per_line, line_ptr, width);
		line_ptr += image->bytes_per_line;
	    }	rgb_line += rgb_line_stride;
	}
	line_ptr += image->bytes_per_line;
    }
}

/*
 * map_1_nodither_table_8
 *
 * Inputs:
 *	rgb:		Pointers to buffers containing the red, green,
 *			and blue color rows.
 *	n:		Length of row.
 *	s:		Skip between pixels in original image.
 *	y:		Y position of row (necessary for dither)
 *	line:		Pointer to output buffer for dithered color data.
 */

static	void
map_1_nodither_table_8 (img, rgb, ncolors, given_width, stride, y, x, image)
image_information *img;
byte	*rgb[3];
int	ncolors,
	given_width,
	stride, y, x;
XImage	*image;

{
register Pixel	*pixel_table = img->pixel_table;
NDMAP_SETUP(img);
register byte 	*r;
register byte	*pixel_ptr;
register int	width = given_width;

    r = rgb[0];

    pixel_ptr = (byte *)image->data + y * image->bytes_per_line + x;

    while (width-- > 0) {
	*pixel_ptr++ = pixel_table [ NDMAP(*r) ];
	r += stride;
    }

}
static	void
MAG_1_nodither_table_8(img, rle_x, rle_y, mag_size, x, y, width, height, image)
image_information *img;
int	rle_x, rle_y,
	width, height,
	mag_size,
	x, y;
XImage	*image;
{
register Pixel	*pixel_table = img->pixel_table;
NDMAP_SETUP(img);
register byte	*r;
register byte	*pixel_ptr;
register byte	table_value;
register int	mag, w;
int		mag_f = abs(mag_size), wd = width / mag_f;
byte	*line_ptr, *rgb_line;
int	rgb_line_stride = img->w * img->dpy_channels;

	height /= mag_f;
	rgb_line = SAVED_RLE_ROW(img, rle_y) + rle_x;
	line_ptr = ((byte *) image->data) + y * image->bytes_per_line + x;

    while (height--) {
	pixel_ptr = line_ptr;
	w = wd;
	r = rgb_line;

	while (w-- > 0) {
		*pixel_ptr++ = table_value = pixel_table[NDMAP(*r)];
		mag = mag_f;
		if (mag_size < 0)
			r += mag;
		else for (r++; --mag;)
				*pixel_ptr++ = table_value;
	}
	mag = mag_f;
	if (mag_size < 0)
		rgb_line += rgb_line_stride * mag;
	else	{
	    while (--mag)	{
		memcpy(line_ptr+image->bytes_per_line, line_ptr, width);
		line_ptr += image->bytes_per_line;
	    }	rgb_line += rgb_line_stride;
	}
	line_ptr += image->bytes_per_line;
    }
}

/*
 * map_1_mono_color_8
 *
 * Inputs:
 *	rgb:		Pointers to buffers containing the red, green,
 *			and blue color rows.
 *	n:		Length of row.
 *	s:		Skip between pixels in original image.
 *	y:		Y position of row (necessary for dither)
 *	line:		Pointer to output buffer for dithered color data.
 */

static	void
map_1_mono_color_8 (img, rgb, ncolors, given_width, stride, y, x, image)
image_information *img;
byte	*rgb[3];
int	ncolors,
	given_width,
	stride, y, x;
XImage	*image;

{
register Pixel	*pixel_table = img->pixel_table;
register byte 	*r;
register byte	*pixel_ptr;
register int	width = given_width;

    r = rgb[0];

    pixel_ptr = ((byte *) image->data) + y * image->bytes_per_line + x;

    while (width-- > 0) {
	*pixel_ptr++ = pixel_table [ *r ];
	r += stride;
    }

}
static	void
MAG_1_mono_color_8 (img, rle_x, rle_y, mag_size, x, y, width, height, image)
image_information *img;
int	rle_x, rle_y,
	width, height,
	mag_size,
	x, y;
XImage	*image;
{
byte	*line_ptr, *rgb_line;
int	mag_f = abs(mag_size), wd = width / mag_f,
	rgb_line_stride = img->w * img->dpy_channels;
register Pixel	*pixel_table = img->pixel_table;
register byte	*r, *pixel_ptr;
register byte	table_value;
register int	mag=mag_f, w;

	height /= mag;
	rgb_line = SAVED_RLE_ROW(img, rle_y) + rle_x;
	line_ptr = ((byte *) image->data)+(y * image->bytes_per_line) + x;

    while (height--) {
	pixel_ptr = line_ptr;
	w = wd;
	r = rgb_line;

	while (w--)	{
		*pixel_ptr++ = table_value = pixel_table [ *r ];
		if (mag_size < 0)
			r += mag;
		else for (r++, mag=mag_f; --mag;)
			*pixel_ptr++ = table_value;
	}
	mag = mag_f;
	if (mag_size < 0)
		rgb_line += rgb_line_stride * mag;
	else	{
	    while (--mag)	{
		memcpy(line_ptr+image->bytes_per_line, line_ptr, width);
		line_ptr += image->bytes_per_line;
	    }	rgb_line += rgb_line_stride;
	}
	line_ptr += image->bytes_per_line;
    }
}

/*
* map_2or3_dither_table_32
*
* Inputs:
*	rgb:		Pointers to buffers containing the red, green,
*			and blue color rows.
*	n:		Length of row.
*	s:		Skip between pixels in original image.
*	y:		Y position of row (necessary for dither)
*	line:		Pointer to output buffer for dithered color data.
*/

#if EVEN_NEVER_CALLED				/* Never called. */
static	void
map_2or3_dither_table_32 (img, rgb, ncolors, given_width, stride, y, x, image)
image_information *img;
byte	*rgb[3];
int	ncolors,
	given_width,
	stride, y;
XImage	*image;

{
register Pixel *pixel_table = img->pixel_table;
DMAP_SETUP(img);
LEVELS_SETUP(img);
register int	col, row;
register byte	*r, *g, *b;
register unsigned long	pixval;
register byte	*pixel_ptr;
register int	width = given_width;
int	byte_order = ImageByteOrder(img->dpy);

    row = y & 0xF;
    r = rgb[col=0];

    g = b = rgb[1];
    if (ncolors >= 3) b = rgb[2];

    pixel_ptr = (byte *)image->data + y * image->bytes_per_line + (x<<2);

    while (width-- > 0) {

	pixval = pixel_table [DMAP(*r, col, row) +
				DMAP(*g, col, row) * levels +
				DMAP(*b, col, row) * levels_squared];
	if (byte_order == MSBFirst)
	{
		pixel_ptr += 3;
		*pixel_ptr-- = pixval & 0xff;
		pixval >>= 8;
		*pixel_ptr-- = pixval & 0xff;
		pixval >>= 8;
		*pixel_ptr-- = pixval & 0xff;
		pixval >>= 8;
		*pixel_ptr = pixval & 0xff;
		pixel_ptr += 4;
	}
	else
	{
		*pixel_ptr++ = pixval & 0xff;
		pixval >>= 8;
		*pixel_ptr++ = pixval & 0xff;
		pixval >>= 8;
		*pixel_ptr++ = pixval & 0xff;
		pixval >>= 8;
		*pixel_ptr++ = pixval & 0xff;
	}
	r += stride;
	g += stride;
	b += stride;
	col = ((col + 1) & 15);
    }
}

/*	#if EVEN_NEVER_CALLED		Never called. */
static	void
MAG_2or3_dither_table_32 (img, rle_x, rle_y, mag_size, x, y, width, height, image)
image_information *img;
int	rle_x, rle_y,
	width, height,
	mag_size,
	x, y;
XImage	*image;
{
register Pixel	*pixel_table = img->pixel_table;
DMAP_SETUP(img);
LEVELS_SETUP(img);
int	mag_y, mag_f = abs(mag_size),	x_mod_16 = (mag_f * rle_x) & 0xF,
	rgb_line_stride = img->w * img->dpy_channels,
	byte_order = ImageByteOrder(img->dpy);
register int 	col, mag_x;
register unsigned long	pixval;
register byte	*pixel_ptr;
register byte	*r, *g, *b;
register int	w, row = (mag_f * rle_y - 1) & 0xF;
byte	*rgb_line = SAVED_RLE_ROW(img, rle_y) + rle_x,
	*line_ptr = (byte *)image->data + y * image->bytes_per_line + (x << 2);

    while (height-- > 0) {
	pixel_ptr = line_ptr;
	w = width;
	row = (row + 1) & 0xF;
	col = x_mod_16;
	r = rgb_line;
	mag_x = mag_f;

	g = b = r + img->w;
	if (img->dpy_channels >= 3) b += img->w;

	while (w-- > 0) {
	    pixval = pixel_table [DMAP(*r, col, row) +
					DMAP(*g, col, row) * levels +
					DMAP(*b, col, row) * levels_squared];
	    if (byte_order == MSBFirst)
	    {
		pixel_ptr += 3;
		*pixel_ptr-- = pixval & 0xff;
		pixval >>= 8;
		*pixel_ptr-- = pixval & 0xff;
		pixval >>= 8;
		*pixel_ptr-- = pixval & 0xff;
		pixval >>= 8;
		*pixel_ptr = pixval & 0xff;
		pixel_ptr += 4;
	    }
	    else
	    {
		*pixel_ptr++ = pixval & 0xff;
		pixval >>= 8;
		*pixel_ptr++ = pixval & 0xff;
		pixval >>= 8;
		*pixel_ptr++ = pixval & 0xff;
		pixval >>= 8;
		*pixel_ptr++ = pixval & 0xff;
	    }
	    if (mag_size < 0)
		r += mag_x,	g += mag_x,	b += mag_x,	w -= mag_x - 1;
	    else if (--mag_x == 0)	{
		r++, g++, b++;
		mag_x = mag_f;
	    }
	    col++;
	    col &= 15;
	}
	if (mag_size < 0)
		rgb_line += rgb_line_stride * mag_x,
		height -= mag_x;
	else if (--mag_y == 0)	{
		rgb_line += rgb_line_stride;
		mag_y = mag_f;
	}
	line_ptr += image->bytes_per_line;
    }
}
#endif	EVEN_NEVER_CALLED

/*
* map_2or3_dither_notable_32
*
* Inputs:
*	rgb:		Pointers to buffers containing the red, green,
*			and blue color rows.
*	n:		Length of row.
*	s:		Skip between pixels in original image.
*	y:		Y position of row (necessary for dither)
*	line:		Pointer to output buffer for dithered color data.
*/
static	void
map_2or3_dither_notable_32 (img, rgb, ncolors, given_width, stride, y, x, image)
image_information *img;
byte	*rgb[3];
int	ncolors,
	given_width,
	stride, y;
XImage	*image;

{
DMAP_SETUP(img);
register int	col, row;
register byte	*r, *g, *b;
register unsigned long	pixval;
register byte	*pixel_ptr;
register int	width = given_width;
int	byte_order = ImageByteOrder(img->dpy);

    row = y & 0xF;
    col = 0;
    r = rgb[0];
    g = rgb[1];
    if (ncolors > 2) b = rgb[2];
    else b = rgb[1];

    pixel_ptr = (byte *)image->data + y * image->bytes_per_line + (x<<2);

    while (width-- > 0) {

	pixval = SHIFT_MASK_PIXEL_32 (DMAP(*r, col, row),
					DMAP(*g, col, row),
					DMAP(*b, col, row));
	if (byte_order == MSBFirst)
	{
		pixel_ptr += 3;
		*pixel_ptr-- = pixval & 0xff;
		pixval >>= 8;
		*pixel_ptr-- = pixval & 0xff;
		pixval >>= 8;
		*pixel_ptr-- = pixval & 0xff;
		pixval >>= 8;
		*pixel_ptr = pixval & 0xff;
		pixel_ptr += 4;
	}
	else	{
		*pixel_ptr++ = pixval & 0xff;
		pixval >>= 8;
		*pixel_ptr++ = pixval & 0xff;
		pixval >>= 8;
		*pixel_ptr++ = pixval & 0xff;
		pixval >>= 8;
		*pixel_ptr++ = pixval & 0xff;
	}
	r += stride;
	g += stride;
	b += stride;
	col = ((col + 1) & 15);
    }
}
static	void
MAG_2or3_dither_notable_32 (img, rle_x, rle_y, mag_size, x, y, width, height, image)
image_information *img;
int	rle_x, rle_y,
	width, height,
	mag_size,
	x, y;
XImage	*image;
{
DMAP_SETUP(img);
int	mag_f = abs(mag_size), wd = width / mag_f,
	x_mod_16 = (mag_f * rle_x) & 0xF,
	rgb_line_stride = img->w * img->dpy_channels,
	byte_order = ImageByteOrder(img->dpy);
register byte	*r, *g, *b;
register unsigned long	pixval, *pixel_ptr;
register int	col, row = (mag_f * rle_y - 1) & 0xF;
register int 	mag=mag_f, w;
byte	*rgb_line = SAVED_RLE_ROW(img, rle_y) + rle_x,
	*line_ptr = (byte *)image->data + y * image->bytes_per_line + (x<<2);

	height /= mag;
    while (height-- > 0) {
	pixel_ptr = line_ptr;
	w = wd;
	row = (row + 1) & 0xF;
	col = x_mod_16;
	r = rgb_line;
	g = b = r + img->w;
	if (img->dpy_channels > 2)	b += img->w;

	while (w-- > 0) {
	    *pixel_ptr++ = pixval = SHIFT_MASK_PIXEL_32 (DMAP(*r, col, row),
							 DMAP(*g, col, row),
							 DMAP(*b, col, row));
	    if (mag_size < 0)
		r += mag,	g += mag,	b += mag;
	    else for (r++, g++, b++, mag=mag_f; --mag;)
			*pixel_ptr++ = pixval;
	    col++;
	    col &= 15;
	}
	mag = mag_f;
	if (mag_size < 0)
		rgb_line += rgb_line_stride * mag;
	else	{
	    while (--mag)	{
		memcpy(line_ptr+image->bytes_per_line, line_ptr, width << 2);
		line_ptr += image->bytes_per_line;
	    }	rgb_line += rgb_line_stride;
	}
	line_ptr += image->bytes_per_line;
    }
}


/*
* map_2or3_nodither_notable_32
*
* Inputs:
*	rgb:		Pointers to buffers containing the red, green,
*			and blue color rows.
*	n:		Length of row.
*	s:		Skip between pixels in original image.
*	y:		Y position of row (necessary for dither)
*	line:		Pointer to output buffer for dithered color data.
*/

static	void
map_2or3_nodither_notable_32 (img, rgb, ncolors, given_width, stride, y, x, image)
image_information *img;
byte	*rgb[3];
int	ncolors,
	given_width,
	stride, y;
XImage	*image;

{
NDMAP_SETUP(img);
register byte	*r, *g, *b;
register unsigned long	pixval;
register byte	*pixel_ptr;
register int	width = given_width;
int	byte_order = ImageByteOrder(img->dpy);

    r = rgb[0];
    g = rgb[1];
    if (ncolors > 2)	b = rgb[2];
    else b = rgb[1];

    pixel_ptr = (byte *)image->data + y * image->bytes_per_line + (x<<2);

    while (width-- > 0) {

	pixval = SHIFT_MASK_PIXEL_32(NDMAP(*r), NDMAP(*g), NDMAP(*b));
	if (byte_order == MSBFirst)
	{
		pixel_ptr += 3;
		*pixel_ptr-- = pixval & 0xff;
		pixval >>= 8;
		*pixel_ptr-- = pixval & 0xff;
		pixval >>= 8;
		*pixel_ptr-- = pixval & 0xff;
		pixval >>= 8;
		*pixel_ptr = pixval & 0xff;
		pixel_ptr += 4;
	}
	else
	{
	    *pixel_ptr++ = pixval & 0xff;
	    pixval >>= 8;
	    *pixel_ptr++ = pixval & 0xff;
	    pixval >>= 8;
	    *pixel_ptr++ = pixval & 0xff;
	    pixval >>= 8;
	    *pixel_ptr++ = pixval & 0xff;
	}

	r += stride;
	g += stride;
	b += stride;

    }
    
}
static	void
MAG_2or3_nodither_notable_32 (img, rle_x, rle_y, mag_size, x, y, width, height, image)
image_information *img;
int	rle_x, rle_y,
	width, height,
	mag_size,
	x, y;
XImage	*image;
{
NDMAP_SETUP(img);
int	mag_f = abs(mag_size), wd = width / mag_f,
	rgb_line_stride = img->w * img->dpy_channels,
	byte_order = ImageByteOrder(img->dpy);
register byte	*r, *g, *b;
register unsigned long	pixval, *pixel_ptr;
register int	mag=mag_f, w;
byte	*rgb_line = SAVED_RLE_ROW(img, rle_y) + rle_x,
	*line_ptr = (byte *)image->data + y * image->bytes_per_line + (x<<2);

	height /= mag;
    while (height-- > 0) {
	pixel_ptr = (unsigned long*)line_ptr;
	w = wd;
	r = rgb_line;
	g = b = r + img->w;
	if (img->dpy_channels > 2)	b += img->w;

	while (w-- > 0) {
		*pixel_ptr++ = pixval =
			SHIFT_MASK_PIXEL_32(NDMAP(*r), NDMAP(*g), NDMAP(*b));
		if (mag_size < 0)
		r += mag,	g += mag,	b += mag;
		else for (r++, g++, b++, mag=mag_f; --mag;)
			*pixel_ptr++ = pixval;
	}
	mag = mag_f;
	if (mag_size < 0)
		rgb_line += rgb_line_stride * mag;
	else	{
	    while (--mag)	{
		memcpy(line_ptr+image->bytes_per_line, line_ptr, width << 2);
		line_ptr += image->bytes_per_line;
	    }	rgb_line += rgb_line_stride;
	}
	line_ptr += image->bytes_per_line;
    }
}

/*
* map_1_nodither_notable_32
*
* Inputs:
*	rgb:		Pointers to buffers containing the red, green,
*			and blue color rows.
*	n:		Length of row.
*	s:		Skip between pixels in original image.
*	y:		Y position of row (necessary for dither)
*	line:		Pointer to output buffer for dithered color data.
*/

static	void
map_1_nodither_notable_32 (img, rgb, ncolors, given_width, stride, y, x, image)
image_information *img;
byte	*rgb[3];
int	ncolors, given_width;
register int	stride;
int	y;
XImage	*image;

{
NDMAP_SETUP(img);
register byte	*r;
register unsigned long	pixval;
register byte	*pixel_ptr;
register int	width = given_width;
int	byte_order = ImageByteOrder(img->dpy);

    r = rgb[0];

    pixel_ptr = (byte *)image->data + y * image->bytes_per_line + (x<<2);

    while (width-- > 0) {
	register int bw_value = NDMAP(*r);

	pixval = SHIFT_MASK_PIXEL_32(bw_value, bw_value, bw_value);
	if (byte_order == MSBFirst)
	{
		pixel_ptr += 3;
		*pixel_ptr-- = pixval & 0xff;
		pixval >>= 8;
		*pixel_ptr-- = pixval & 0xff;
		pixval >>= 8;
		*pixel_ptr-- = pixval & 0xff;
		pixval >>= 8;
		*pixel_ptr = pixval & 0xff;
		pixel_ptr += 4;
	}
	else
	{
		*pixel_ptr++ = pixval & 0xff;
		pixval >>= 8;
		*pixel_ptr++ = pixval & 0xff;
		pixval >>= 8;
		*pixel_ptr++ = pixval & 0xff;
		pixval >>= 8;
		*pixel_ptr++ = pixval & 0xff;
	}

	r += stride;
    }

}
static	void
MAG_1_nodither_notable_32 (img, rle_x, rle_y, mag_size, x, y, width, height, image)
image_information *img;
int	rle_x, rle_y,
	width, height,
	mag_size,
	x, y;
XImage	*image;

{
NDMAP_SETUP(img);
int	mag_f = abs(mag_size), wd = width / mag_f,
	rgb_line_stride = img->w * img->dpy_channels,
	byte_order = ImageByteOrder(img->dpy);
register unsigned long	pixval, *pixel_ptr;
register byte	*r;
register int	bw_value, mag=mag_f, w;
byte	*rgb_line = SAVED_RLE_ROW(img, rle_y) + rle_x,
	*line_ptr = (byte *)image->data + y * image->bytes_per_line + (x<<2);

	height /= mag_f;
    while (height-- > 0) {
	pixel_ptr = line_ptr;
	w = wd;
	r = rgb_line;

	while (w-- > 0) {
		bw_value = NDMAP(*r);
		*pixel_ptr++ = pixval = SHIFT_MASK_PIXEL_32(bw_value, bw_value, bw_value);
		if (mag_size < 0)
			r += mag;
		else for (r++, mag=mag_f; --mag;)
			*pixel_ptr++ = pixval;
	}
	mag = mag_f;
	if (mag_size < 0)
		rgb_line += rgb_line_stride * mag;
	else	{
	    while (--mag)	{
		memcpy(line_ptr+image->bytes_per_line, line_ptr, width << 2);
		line_ptr += image->bytes_per_line;
	    }	rgb_line += rgb_line_stride;
	}
	line_ptr += image->bytes_per_line;
    }
}
/*
 * map_1_mono_color_32
 *
 * Inputs:
 *	rgb:		Pointers to buffers containing the red, green,
 *			and blue color rows.
 *	n:		Length of row.
 *	s:		Skip between pixels in original image.
 *	y:		Y position of row (necessary for dither)
 *	line:		Pointer to output buffer for dithered color data.
 */

static	void
map_1_mono_color_32 (img, rgb, ncolors, given_width, stride, y, x, image)
image_information *img;
byte	*rgb[3];
int	ncolors,
	given_width;
register int	stride;
int	y;
XImage	*image;
{
register byte	*r;
register unsigned long	pixval;
register byte	*pixel_ptr;
register int	width = given_width;
int	byte_order = ImageByteOrder(img->dpy);

    r = rgb[0];

    pixel_ptr = (byte *)image->data + y * image->bytes_per_line + (x<<2);

    while (width-- > 0) {
	pixval = img->pixel_table[ *r ];
	if (byte_order == MSBFirst)
	{
		pixel_ptr += 3;
		*pixel_ptr-- = pixval & 0xff;
		pixval >>= 8;
		*pixel_ptr-- = pixval & 0xff;
		pixval >>= 8;
		*pixel_ptr-- = pixval & 0xff;
		pixval >>= 8;
		*pixel_ptr = pixval & 0xff;
		pixel_ptr += 4;
	}
	else
	{
		*pixel_ptr++ = pixval & 0xff;
		pixval >>= 8;
		*pixel_ptr++ = pixval & 0xff;
		pixval >>= 8;
		*pixel_ptr++ = pixval & 0xff;
		pixval >>= 8;
		*pixel_ptr++ = pixval & 0xff;
	}
	r += stride;
    }

}
static	void
MAG_1_mono_color_32 (img, rle_x, rle_y, mag_size, x, y, width, height, image)
image_information *img;
int	rle_x, rle_y,
	width, height,
	mag_size,
	x, y;
XImage	*image;

{
int	mag_f = abs(mag_size), wd = width / mag_f,
	rgb_line_stride = img->w * img->dpy_channels,
	byte_order = ImageByteOrder(img->dpy);
register byte	*r;
register unsigned long	pixval, *pixel_ptr;
register int	mag=mag_f, w;
byte	*rgb_line = SAVED_RLE_ROW(img, rle_y) + rle_x,
	*line_ptr = (byte *)image->data + y * image->bytes_per_line + (x<<2);

	height /= mag;
    while (height-- > 0) {
	pixel_ptr = line_ptr;
	w = wd;
	r = rgb_line;

	while (w-- > 0) {
		*pixel_ptr++ = pixval = img->pixel_table[ *r ];
		if (mag_size < 0)
			r += mag;
		else for (r++, mag=mag_f; --mag;)
			*pixel_ptr++ = pixval;
	}
	mag = mag_f;
	if (mag_size < 0)
		rgb_line += rgb_line_stride * mag;
	else	{
	    while (--mag)	{
		memcpy(line_ptr+image->bytes_per_line, line_ptr, width << 2);
		line_ptr += image->bytes_per_line;
	    }	rgb_line += rgb_line_stride;
	}
	line_ptr += image->bytes_per_line;
    }
}

/*****************************************************************
* TAG(map_rgb_to_bw)
*
* Convert RGB to black and white through NTSC transform, but map
* RGB through a color map first.
* Inputs:
*	red_row, green_row, blue_row:	Given RGB pixel data.
*	map:		Array[3] of pointers to pixel arrays,
*			representing color map.
*	rowlen:		Number of pixels in the rows.
* Outputs:
*	bw_row:	    Output B&W data.  May coincide with one of the
*		    inputs.
* Algorithm:
*	BW = .35*map[0][R] + .55*map[1][G] + .10*map[2][B]
*/
void
map_rgb_to_bw(img, rows, bw_row, rowlen)
image_information	*img;
rle_pixel		**rows;
register rle_pixel	*bw_row;
{
register rle_pixel	*red, *green, *blue;

rle_pixel	**map;
int		ncolors;

    map = img->in_cmap;
    ncolors = img->img_channels;
    if (rowlen < 1)
	rowlen = img->w;

    if (ncolors < 1)
	prgmerr('c', "map_rgb_to_bw given %d colors\n", ncolors);

    switch (ncolors) {
    case 1:
	red = rows[0];
	if (!map || img->mono_color)
		duff8(rowlen, *bw_row++ = *red++)
	else {
	register rle_pixel *cmap = map[0];
		duff8(rowlen, *bw_row++ = cmap[*red++])
	}
	break;

    case 2:
	red = rows[0];
	green = rows[1];
	if (!map)
		duff8(rowlen, *bw_row++ = (*red++ + *green++) >> 1)
	else {
	register rle_pixel **cmap = map;
		duff8(rowlen, *bw_row++ = (cmap[0][*red++] +
					cmap[1][*green++]) >> 1)
	}
	break;

    default:
    case 3:
	red = rows[0];
	green = rows[1];
	blue = rows[2];
	if (!map)
		duff8(rowlen, *bw_row++ = (35 * *red++ + 55 * *green++ +
					10 * *blue++) / 100)
	else {
	register rle_pixel **cmap = map;
		duff8(rowlen, *bw_row++ = (35 * cmap[0][*red++] +
					55 * cmap[1][*green++] +
					10 * cmap[2][*blue++]) / 100)
	}
	break;
    }
}

/*****************************************************************
 * TAG(map_rgb_to_rgb)
 *
 * Convert RGB to RGB through a colormap
 * Inputs:
 * 	in_rows:	Given RGB pixel data.
 *	map:		Array[3] of pointers to pixel arrays,
 *			representing color map.
 *	rowlen:		Number of pixels in the rows.
 * Outputs:
 * 	out_rows:       Output data.  May coincide with one of the inputs.
 */
void
map_rgb_to_rgb(img, in_rows, out_rows, rowlen)
image_information *img;
rle_pixel	**in_rows, **out_rows;
{
register rle_pixel	*in, *out, *cmap;
register int	w;
rle_pixel	**map=img->in_cmap;
int	ncolors = img->img_channels;
	if (rowlen < 1)
		rowlen = img->w;

	if (ncolors < 1)
		prgmerr('c', "map_rgb_to_rgb given %d colors\n", ncolors);

	if (map)
	    while (ncolors--)	{
		in = in_rows[0];
		out = out_rows[0];
		cmap = map[0];
		w = rowlen;

		duff8(w, *out++ = cmap[*in++]);

		in_rows++;
		out_rows++;
		map++;
	    }
	else
	    while (ncolors--)	{
		if (in_rows[0] != out_rows[0])
			memcpy(out_rows[0], in_rows[0], rowlen);

		in_rows++;
		out_rows++;
	    }
}


void
get_dither_arrays(img)
register image_information *img;
{
	if (!img->divN)
		img->divN = (int *) NZALLOC(256, sizeof(int), No);
	if (!img->modN)
		img->modN = (int *) NZALLOC(256, sizeof(int), No);
	if (!img->dm16)
		img->dm16 = (array16 *) NZALLOC(16 * 16, sizeof(int), No);

	if (!img->divN || !img->modN || !img->dm16)
		syserr("malloc error getting dither arrays");
}


shift_match_left (mask, high_bit_index)
Pixel	mask;
int	high_bit_index;
{
register int	shift;
register Pixel	high_bit;

	if (!mask) return	0;

	high_bit = 0x80000000;

	for (shift=(32 - high_bit_index); !(high_bit & mask); shift--)
		high_bit >>= 1;

return	shift;
}

shift_match_right (mask)
Pixel	mask;
{
register int	shift=0;
register Pixel	low_bit;

if (mask)
	for (low_bit = 1; (low_bit & mask) == 0; shift++)
		low_bit <<= 1;
return (shift);
}
