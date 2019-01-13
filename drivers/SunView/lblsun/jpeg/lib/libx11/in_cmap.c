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
*
* in_cmap.c - Jack with the Input colormap from the rle files.
*
* Author:	Martin R. Friedmann
* 		Dept of Electrical Engineering and Computer Science
*		University of Michigan
* Date:	Tue, April 12, 1990
* Copyright (c) 1990, University of Michigan
*
* Modified:	For CCS and not depend on RLE buildmap.
% Date:		Fri, May 31, 1991
*		Jin, Guojun - Lawrence Berkeley Laboratory
*/

#include "panel.h"
#include <math.h>

/* uses global command line args (display_gamma) */
/* sets in_cmap, cmaplen, ncmap and mono_color */
#define	R_G(s)	rle_getcom(s, &rle_dflt_hdr)

void
get_dither_colors(img)
register image_information *img;
{
register int	i, j;

    if (img->ncmap ||	/* dpy channels == 3 is for TureColor.	*/
	img->dpy_channels < 3 && (img->ncmap = rle_dflt_hdr.ncmap))	{
	if (img->in_type == RLE)	{
	char * v = R_G("color_map_length");
	    if (v)	img->cmaplen = atoi(v);
	    else	img->cmaplen = 1 << rle_dflt_hdr.cmaplen;
	/* Protect against bogus information */
	    if (img->cmaplen < 0)	img->cmaplen = 0;
	    if (img->cmaplen > 256)	img->cmaplen = 256;
	    if (img->ncmap < rle_dflt_hdr.ncolors)
		img->ncmap = rle_dflt_hdr.ncolors;
	    if (!img->gamma &&
		((v = R_G("image_gamma")) || (v = R_G("display_gamma"))))
		img->gamma = atof(v);
	}
	/*	Input map at least 3 channels.
		If not using color map directly, neither apply display gamma.
	== if (!(img->sep_colors ||
		(img->img_channels == 1 && img->ncmap == 3 && img->cmaplen)))
		display_gamma = 1.0;
	*/
	if (!img->sep_colors &&
		!(img->img_channels == 1 && img->ncmap == 3 && img->cmaplen))
		display_gamma = 0.0;

	img->in_cmap = alloc_2d_array(img->cmaplen * sizeof(cmap_t), 3);
#ifndef	NO_CONCURRENCY
	if (!reg_cmap[0])	/*	rle only	*/
		rlemap_to_regmap(reg_cmap, &rle_dflt_hdr);
#endif
	apply_3c_gamma(img, reg_cmap, display_gamma);

#ifdef	_DEBUG_
	if (DEBUGANY) for (i=0; i < 3; i++) {
		message("Input image colormap channel %d:\n", i);
		if (i > 0)
		    for (j=0; j < img->cmaplen; j++)
			if (img->in_cmap[i-1][j] != img->in_cmap[i][j])
				break;
		if (i > 0 && j == img->cmaplen)
			message("\tSame as channel %d\n", i - 1);
		else	for (j=0; j < img->cmaplen; j += 16) {
			int	k;
			message("%3d: ", j);
			for (k = 0; k < 16 ; k++)
			    if (j + k < img->cmaplen)
				message("%3d ", img->in_cmap[i][j+k]);
			mesg("\n");
		    }
	}
#endif
	/*	The mono_color flag means that a single input channel is being
	*	pseudocolored by a multi-channel colormap.
	*/
	img->mono_color = (img->img_channels == 1 && img->ncmap == 3 &&
				img->in_cmap && img->cmaplen);

	/* make colormap monochrome...   Whatahack! */
	if (img->mono_color && !img->color_dpy) {
	    for (j=0; j < 256; j++)
		img->in_cmap[0][j] = (rle_pixel)
			((30 * img->in_cmap[0][j] + 59 * img->in_cmap[1][j] +
			  11 * img->in_cmap[2][j]) / 100);

	    img->mono_color = False;
	}
    }
}

eq_cmap(cm1, len1, cm2, len2)
register rle_pixel **cm1, **cm2;
{
register int	i, j;

    if (cm1 && cm2) {
	if (len1 != len2)
		return 0;
	for (i=0; i < 3; i++)
	    for (j=0; j < 256; j++)
		if (cm1[i][j] != cm2[i][j])
			return 0;
    } else
	return	!(cm1 || cm2);
return	1;
}
