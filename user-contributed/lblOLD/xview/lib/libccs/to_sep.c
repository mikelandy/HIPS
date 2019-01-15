/*	any_to_seplane . c
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

This software is copyright (C) by the Lawrence Berkeley Laboratory.
Permission is granted to reproduce this software for non-commercial
purposes provided that this notice is left intact.

It is acknowledged that the U.S. Government has rights to this software
under Contract DE-AC03-765F00098 between the U.S.  Department of Energy
and the University of California.

This software is provided as a professional and academic contribution
for joint exchange. Thus, it is experimental, and is provided ``as is'',
with no warranties of any kind whatsoever, no support, no promise of
updates, or printed documentation. By using this software, you
acknowledge that the Lawrence Berkeley Laboratory and Regents of the
University of California shall have no liability with respect to the
infringement of other copyrights by any part of this software.

For further information about this notice, contact William Johnston,
Bld. 50B, Rm. 2239, Lawrence Berkeley Laboratory, Berkeley, CA, 94720.
(wejohnston@lbl.gov)

For further information about this software, contact:
	Jin Guojun
	Bld. 50B, Rm. 2275, Lawrence Berkeley Laboratory, Berkeley, CA, 94720.
	g_jin@lbl.gov

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% AUTHOR:	Jin Guojun - LBL	1/1/92
*/

#include "header.def"
#include "imagedef.h"

VType*
any_to_seplane(img, chans, revs, map, rload)
U_IMAGE	*img;
cmap_t*	*map;
{
char	*cnvt;
int	w = img->width, h = img->height;
if (!img->color_dpy)	return	0;
if (img->dpy_channels > 1 || !*map)
#ifdef	JPEG_V4_3
	img->in_color = img->in_type==HIPS && !map[0]
	/* || img->mid_type==HIPS	HIPS color is very complicated */
		? CFM_ILC : CFM_ILL;
#else
	if (img->in_type==HIPS && !map[0])
		img->in_color = CFM_ILC;	/*	Why ??	*/
#endif

cnvt = NZALLOC(w*h, 3L, "to_sep");

switch (img->in_color)	{
case CFM_SCF:	pseudo_to_sep(cnvt, img->src, w * h, map);
	break;
case CFM_ILC:	ilc_to_sep(cnvt, img->src, w * h, chans, revs);
	break;
default:
case CFM_ILL:	ill_to_sep(cnvt, img->src, w, h, chans);
}
if (rload)
	memcpy(img->src, cnvt, w*h*3),
	CFREE(cnvt),	cnvt = 0;
else	CFREE(img->src),
	img->src = cnvt;
return	cnvt;
}


ill_to_sep(r, rle, w, h, chans)
register byte	*r, *rle;
{
register byte	*g=r+w*h, *b=g+w*h;
chans = (chans-3) * w;
    while (h--)	{
	rle += chans;
	memcpy(r, rle, w),	rle += w,	r += w;
	memcpy(g, rle, w),	rle += w,	g += w;
	memcpy(b, rle, w),	rle += w,	b += w;
    }
}

ilc_to_sep(r, rgb, fsize, chans, revs)
register byte	*r, *rgb;
register int	fsize;
{
register byte	*g=r+fsize, *b=g+fsize;
chans = chans - 3;
if (revs)	revs = (int)b,	b = r,	r = (byte*)revs;
    while (fsize--)	{
	rgb += chans;
	*r++ = *rgb++;	*g++ = *rgb++;	*b++ = *rgb++;
    }
}

pseudo_to_sep(r, ibp, fsize, map)
register byte	*r, *ibp;
cmap_t	*map[];
{
register byte	*g=r+fsize, *b=g+fsize;
    while (fsize--)	{
	*r++ = map[0][*ibp];
	*g++ = map[1][*ibp];
	*b++ = map[2][*ibp++];
    }
}

