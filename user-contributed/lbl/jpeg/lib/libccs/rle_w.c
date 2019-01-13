/*	RLE_Write . C
#
%	Copyright (c)	Jin Guojun
%
% AUTHOR:	Jin, Guojun - LBL	12/1/91
*/

#ifndef	RLE_IMAGE
#define	RLE_IMAGE
#endif

#if	!defined LKT && !defined FLOAT_LKT_REQUIRED
#define	LKT	int	/*	for window application	*/
#endif

#include "header.def"
#include "imagedef.h"

write_rle(img, lkt, mapped /* from src to dest */)
U_IMAGE	*img;
LKT	*lkt;
{
register int	i, j, ch;
byte	*pp[3], *dp[3];
int	w, h, X0=0, rle_y = img->height;

	if (img->sub_img)
		w = img->sub_img_w,
		h = img->sub_img_h,
		X0 = img->sub_img_x;
	else	w =img->width,	h = rle_y;

	rle_dflt_hdr.rle_file = img->OUT_FP;
	if (lkt) {
		dp[0] = (byte*) zalloc(w*3, sizeof(*dp[0]), "dp");
		dp[1] = dp[0] + w;
		dp[2] = dp[1] + w;
	}
	rle_dflt_hdr.xmin = rle_dflt_hdr.ymin = 0;
	rle_dflt_hdr.xmax = w - 1;
	rle_dflt_hdr.ymax = h - 1;
	rle_dflt_hdr.ncolors = img->dpy_channels;
	if (img->color_form==CFM_SCF) /* if not mapped, cmap is in reg */
		regmap_to_rlemap(img->cmap ? img->cmap : reg_cmap,
			img->cmaplen, 3, &rle_dflt_hdr);
	else	rle_dflt_hdr.ncmap = rle_dflt_hdr.cmaplen =
			(int)(rle_dflt_hdr.cmap = NULL); /* must be 0 !	*/
	rle_put_setup(&rle_dflt_hdr);

	for (i=0; i++ < h;) {
	    if (mapped)
		pp[0] = SAVED_RLE_ROW(img, (img->sub_img ?
		img->sub_img_y+img->sub_img_h :	rle_y) - i) + X0;
	    else
		pp[0] = ORIG_RLE_ROW(img, (img->sub_img ?
		img->sub_img_y+img->sub_img_h :	rle_y) - i) + X0;
	    pp[1] = pp[0] + img->width;
	    pp[2] = pp[1] + img->width;
	    if (lkt) {
		for (ch=0; ch<img->channels; ch++)	{
		register LKT	*lktp = lkt + ch * MaxColors;
			if (img->update && img->sub_img)
				memcpy(dp[ch], pp[ch], w);
			else
			    for (j=0; j<w; j++)
				dp[ch][j] = lktp[pp[ch][j]];
		}
		rle_putrow(dp, w, &rle_dflt_hdr);
	    }
	    else	rle_putrow(pp, w, &rle_dflt_hdr);
	}
	rle_puteof(&rle_dflt_hdr);	/* for multi-frame images only	*/

	if (lkt)	CFREE(dp[0]);
	img->update = False;
}
