/*	C_MAP1 . C
#
%	Copyright (c)	Jin Guojun
%
% AUTHOR:	Jin Guojun - LBL	10/01/91
*/

#include "header.def"
#include "imagedef.h"

/*	return	0 in failure when reading color maps	*/
bool
ReadRGBMap(U_IMAGE *img, color_cell *cmbuf, int mlen, bool OsameI)
{
	if ((*img->i_read)(cmbuf, sizeof(*cmbuf), mlen, img->IN_FP) != mlen)
		return	prgmerr(-1, "bad colormap");
	rgbmap_to_othermap(cmbuf, mlen, True /* to reg_cmap */);
return	True;
}


read_hex_rgbmap(U_IMAGE *img, register color_cell *cmbuf, int mlen)
{
register int	i;
for (i=0; i < mlen; i++)	{
#ifndef	NO_ALIGN
MType	align;	/* works for both Endian machines	*/
	if (fscanf(img->IN_FP, "%x ", &align) < 1)
#else	/* for BGR cmap only since %x format reverses reading order	*/
	if (fscanf(img->IN_FP, "%x ", cmbuf + i) < 1)
#endif
		return	prgmerr(-1, "hex colormap");
#ifndef	NO_ALIGN
	cmbuf[i].b = align & 0xFF;
	cmbuf[i].g = (align >> 8) & 0xFF;
	cmbuf[i].r = (align >> 16)& 0xFF;
#endif
}
rgbmap_to_othermap(cmbuf, mlen, True);
return	True;
}

read_ras_cmap(U_IMAGE*	img, register int mlen, int chans)
{
	verify_buffer_size(reg_cmap, mlen * 3, sizeof(cmap_t), "rast_cmap");
	reg_cmap[1] = reg_cmap[0] + mlen;
	reg_cmap[2] = reg_cmap[1] + mlen;
return	((*img->i_read)(reg_cmap[0], sizeof(cmap_t)*3, mlen, img->IN_FP) ==
		(img->cmaplen = mlen));
}


/*	return	color fromat	*/
imageform_to_colorform(register int	iform)
{
	switch (iform)	{
	case IFMT_SCF:	iform = CFM_SCF;	break;
	case IFMT_ILC:	iform = CFM_ILC;	break;
	case IFMT_ILL:	iform = CFM_ILL;	break;
	case IFMT_ALPHA:	iform = CFM_ALPHA;	break;
	case IFMT_SEPLANE:	iform = CFM_SEPLANE;	break;
	case IFMT_BITMAP:	iform = CFM_BITMAP;	break;
	case IFMT_SGF:
	default:	iform = CFM_SGF;	break;
	}
return	iform;
}
