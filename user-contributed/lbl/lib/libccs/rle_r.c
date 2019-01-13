/*	RLE_Read . C
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

This software is copyright (C) by the Lawrence Berkeley National Laboratory.
Permission is granted to reproduce this software for non-commercial
purposes provided that this notice is left intact.

It is acknowledged that the U.S. Government has rights to this software
under Contract DE-AC03-76SF00098 between the U.S.  Department of Energy
and the University of California.

This software is provided as a professional and academic contribution
for joint exchange. Thus, it is experimental, and is provided ``as is'',
with no warranties of any kind whatsoever, no support, no promise of
updates, or printed documentation. By using this software, you
acknowledge that the Lawrence Berkeley Laboratory and Regents of the
University of California shall have no liability with respect to the
infringement of other copyrights by any part of this software.

For further information about this notice, contact William Johnston,
Bld. 50B, Rm. 2239, Lawrence Berkeley National Laboratory, Berkeley, CA, 94720
(wejohnston@lbl.gov)

For further information about this software, contact:
	Jin Guojun
	Bld. 50B, Rm. 2275
	Lawrence Berkeley National Laboratory, Berkeley, CA, 94720
	g_jin@lbl.gov

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% AUTHOR:	Jin, Guojun - LBL	5/1/91
*/

#include "header.def"
#include "imagedef.h"

#ifndef	StdIconWidth
#define	StdIconWidth	128
#endif

read_rle_image(img, loc_map, icon_factor)
U_IMAGE	*img;
cmap_t	*loc_map[3];
{
bool	spflag = img->channels == 1 && img->color_form == CFM_SEPLANE,
	equal_chan = img->channels == img->dpy_channels || spflag,
	no_scanbuf = equal_chan && img->color_dpy &&
			(img->channels == 1 || img->color_form != CFM_ILC);
byte	*read_scan[3], *save_scan[3];
int	i=0, image_y, ymax=rle_dflt_hdr.ymax, width=img->width,
	scan_size = spflag ? width : width * img->dpy_channels; /* bad to use dpy_c */
rle_pixel**	scanline;

if (!icon_factor)
	icon_factor = width / (img->icon_width ? img->icon_width : StdIconWidth);

if (no_scanbuf)	{
    if (scanline = NZALLOC(4, sizeof(*scanline), No))	{
	if (rle_dflt_hdr.alpha)	scanline++;
	scanline[0] = (byte*) img->src + ymax * scan_size;
	scanline[1] = scanline[0] + width;
	scanline[2] = scanline[1] + width;
    }
} else	i = rle_row_alloc(&rle_dflt_hdr, &scanline);
if (i || !scanline)	return	prgmerr(0, img->name);

if (img->channels==1 && rle_dflt_hdr.ncmap)
	rlemap_to_regmap(loc_map, &rle_dflt_hdr);

while ((image_y=ymax-rle_getrow(&rle_dflt_hdr, scanline)) >= 0)	{
register byte	*obp = (byte*)img->src + (image_y - no_scanbuf) * scan_size;

    if (equal_chan) {	/*	RLE output	*/
	if (img->channels > 1)
	    if (!img->color_dpy)
		ill_to_gray(obp, scanline[0], scanline[1], scanline[2], width);
	    else if (img->color_form == CFM_ILC)
		line_to_cell_color(obp, scanline[0], width, 1);
	    else
		scanline[0] = obp,
		scanline[2] = (scanline[1] = obp + width) + width;
	else {
		if (loc_map[0] && !img->color_dpy)
			map8_gray(obp, *scanline, width, loc_map);
		else	*scanline = obp;
	}
    }
    else if (img->channels == 1)	/* (img->dpy_channels == 3)	*/
	snf_to_rle(obp, *scanline, width, 8, loc_map);
    else
	ill_to_gray(obp, scanline[0], scanline[1], scanline[2], width);
}

if (no_scanbuf)	{
	if (rle_dflt_hdr.alpha)	scanline--;
	CFREE(scanline);
} else	rle_row_free(&rle_dflt_hdr, scanline);
if (!img->color_dpy || img->dpy_depth==1)
	img->channels = 1;
else	img->channels = img->dpy_channels;

return	(ymax - rle_dflt_hdr.ymin - image_y) * width;
}

/* REL en-decoder bug ?	*/
#define	map_pixel(pix, cmaplen, cmap)	(pix > cmaplen ? pix : cmap[pix]>>8)

gray_to_rle(obp, ibp, img, cmaplen, map)
byte	*obp, *ibp;
U_IMAGE	*img;
unsigned short	*map[];
{
register int	width=img->width;
byte	*cp[3];

cp[0] = obp;
cp[1] = cp[0] + width;
cp[2] = cp[1] + width;
ibp += width;

while (width--)	{
register int	v = *ibp--;
	cp[0][width] = map_pixel(v, cmaplen, map[0]);
	cp[1][width] = map_pixel(v, cmaplen, map[1]);
	cp[2][width] = map_pixel(v, cmaplen, map[2]);
}
}
