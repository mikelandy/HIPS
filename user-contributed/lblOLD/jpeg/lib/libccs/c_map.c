/*	C_MAP . C
%
%	Copyright (c)	Jin Guojun
%
% AUTHOR:	Jin Guojun - LBL	10/1/91
*/

#include "header.def"
#include "imagedef.h"

#ifndef	MaxColors
#define	MaxColors	256
#endif

sht_cmap_t	*r_cmap;	/* for matching rle_map */
cmap_t*		reg_cmap[3];	/* for global usage */


min_bits(val)	/* minimum bits=(log2(v) + 1) for integer value  */
{
register int	i=1;
if (val > 0)
	while (val>>=1)	i++;
return	i;
}

VType*
rgbmap_to_othermap(cmap, number, reg_or_rle)
color_cell	*cmap;
{
register int	i=number;
    if (reg_or_rle) {	/* to regular cmap */
	if (verify_buffer_size(reg_cmap, i, 3*sizeof(cmap_t), "rgb-reg")) {
		reg_cmap[1] = reg_cmap[0] + i;
		reg_cmap[2] = reg_cmap[1] + i;
	}
	for (i=0; i < number; ++i)
		reg_cmap[0][i] = cmap[i].r,
		reg_cmap[1][i] = cmap[i].g,
		reg_cmap[2][i] = cmap[i].b;
	return	(VType *) reg_cmap[0];
    }
    else {	/* to RLE cmap, always be 2**min_bits */
	register int	j=1<<min_bits(i-1);
	verify_buffer_size(&r_cmap, j, 3*sizeof(*r_cmap), "rgb-rle");
	for (i=0; i < number; ++i) {
		r_cmap[i] = cmap[i].r << 8;
		r_cmap[j+i] = cmap[i].g << 8;
		r_cmap[(j<<1)+i] = cmap[i].b << 8;
	}
	return	(VType *)r_cmap;
    }
}

sht_cmap_t*
regmap_to_rlemap(cmap, number, channels, rle_hd)
cmap_t	*cmap[3];
rle_hdr	*rle_hd;
{
register int	i=min_bits(MIN(MaxColors, number)-1), j=1<<i;
/*	rle_hd->ncolors = 3;	is always 3 color channels	*/
	rle_hd->cmaplen = i;
	if (!(rle_hd->ncmap = channels))
		return	NULL;
	verify_buffer_size(&r_cmap, j, channels*sizeof(*r_cmap), "reg-rle");
	rle_hd->cmap = (rle_map *)r_cmap;
	for (i=0; i < number; ++i) {
		r_cmap[i] = cmap[0][i] << 8;
		r_cmap[j+i] = cmap[1][i] << 8;
		r_cmap[(j<<1)+i] = cmap[2][i] << 8;
	}
return	r_cmap;
}

cmap_t*
rlemap_to_regmap(cmap, rle_hd)
cmap_t	*cmap[3];
rle_hdr	*rle_hd;
{
register rle_map*	rmap = rle_hd->cmap;
register int	i = 1 << rle_hd->cmaplen;

    if (verify_buffer_size(cmap, i, 3*sizeof(cmap_t), "rle-reg")) {
	cmap[1] = cmap[0] + i;
	cmap[2] = cmap[1] + i;
    }
#ifndef	EXACT_256_RLE_TO_REG
    for (i *= rle_hd->ncmap; i--;)
	cmap[0][i] = rmap[i] >> 8;
#else
    {	register int	c=i
	while (c--)
		cmap[0][c] = rlemap[c] >> 8,
		cmap[1][c] = rlemap[i+c] >> 8,
		cmap[2][c] = rlemap[(i<<1)+c] >> 8;
    }
#endif
return	cmap[0];
}

CloseColor_in_Map(cmap, ncolors, r, g, b, Pseudo /* better is 384 */)
register cmap_t	*cmap[];
register int	r, g, b;
{
register int	dis, i;
int	value=0;

for (i=0; i < ncolors; i++) {
	dis = abs(cmap[0][i] - r) + abs(cmap[1][i] - g) + abs(cmap[2][i] - b);
	if (!dis)
		return	i;	/* find exact value	*/
	else if (dis < Pseudo) {
	    Pseudo = dis;
	    value = i;	/* get the closest one	*/
	}
}
return	value;
}

isColorImage(cfmt)
register int	cfmt;
{
register isc =	cfmt != CFM_SGF && cfmt != CFM_BITMAP;
if (isc && cfmt != CFM_SCF)	isc = 3;
return	isc;
}

select_color_form(img, osamei8)
register U_IMAGE*	img;
{	/* no change, special in a TiFF case. May change later on */
int	cfm=img->in_color, is_hips=img->mid_type == HIPS;
if (img->in_type != TiFF || img->color_form != CFM_BITMAP)
	if (img->color_dpy && cfm != CFM_SGF)
	    switch(img->mid_type)	{
	    case RLE:	case COLOR_PS:	/* no SEPLANE for RLE	*/
		if (img->color_dpy > 0 || cfm == CFM_SEPLANE) {
		    if (isColorImage(cfm))
			if (img->channels > 1 || !osamei8)
				img->in_form = IFMT_ILL,
				cfm = CFM_ILL;
			else	cfm = CFM_SCF;
		    else	cfm = CFM_SGF;
		    break;
		}
	    case HIPS:	/* no CFM_ILL | CFM_BITMAP output in HIPS	*/
		if (is_hips && cfm == CFM_BITMAP)	cfm = CFM_SGF;
		if (img->o_form == IFMT_SEPLANE)	cfm = CFM_SEPLANE; /* ?? */
		if (! is_hips || cfm != CFM_ILL)	break;
	    default:	cfm = CFM_ILC;
	    }
	else	cfm = CFM_SGF;

	if (is_hips)	/* for color only */
		at_least_1(img->frames),	img->in_form = IFMT_BYTE;
	if (cfm == CFM_ALPHA)	cfm--;	/* get rid of the alpha channel	*/
	img->dpy_channels = isColorImage(img->color_form = cfm);
	at_least_1(img->dpy_channels);
	img->mono_img = img->dpy_channels==1;
}
