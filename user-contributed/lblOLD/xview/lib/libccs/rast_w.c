/*	rast_w . c
#
%	Copyright (c)	1991, 1994	Jin Guojun
%
% AUTHOR:	Jin Guojun - LBL	12/1/1991
*/

#include "header.def"
#include "imagedef.h"

#define	cols	img->width
#define	rows	img->height

colormap_t*
create_pr_colormap(register int	l)
{
colormap_t*	pr_cmap = (colormap_t *)ZALLOC(1, sizeof(colormap_t), "pr-map");
	pr_cmap->type = RMT_EQUAL_RGB;
	pr_cmap->length = l;
	pr_cmap->map[0] = (byte *) ZALLOC(l, 3, "0");
	pr_cmap->map[1] = pr_cmap->map[0] + l;
	pr_cmap->map[2] = pr_cmap->map[1] + l;
return	pr_cmap;
}

void
free_pr_colormap(colormap_t* pr_cmap)
{
if (pr_cmap)
	CFREE(pr_cmap->map[0]),
	CFREE(pr_cmap);
}

colormap_t*
set_pr_colormap(cmap_t*	rg_map[3], register int	colors)
{
colormap_t* pr_cmap = create_pr_colormap(colors);

	memcpy(pr_cmap->map[0], rg_map[0], colors);
	memcpy(pr_cmap->map[1], rg_map[1], colors);
	memcpy(pr_cmap->map[2], rg_map[2], colors);
return	pr_cmap;
}

colormap_t*
set_gs_colormap()
{
colormap_t* pr_cmap = create_pr_colormap(MaxColors);
register int	i;
    for (i=0; i < MaxColors; ++i)
	pr_cmap->map[0][i] = pr_cmap->map[1][i] =
	pr_cmap->map[2][i] = i;
return	pr_cmap;
}

write_rast(U_IMAGE *img, bool alpha, int pr_t)
{
int	depth=8, linesize, nobuf,
	pr_type=RT_BYTE_ENCODED, row, colw=cols;
colormap_t	*pr_cmap=NULL;
struct pixrect	*pr;
byte*	data;
register byte	*inbp, *obp;

	switch (img->color_form) {
	case CFM_ALPHA:
	case CFM_ILL:
	case CFM_ILC:
		depth = alpha ? 32 : 24;
		pr_type = RT_STANDARD;
		break;
	case CFM_SCF:
		if (!reg_cmap[0] && rle_dflt_hdr.ncmap)
			rlemap_to_regmap(reg_cmap, &rle_dflt_hdr),
			img->cmaplen = 1 << rle_dflt_hdr.cmaplen;
		pr_cmap = set_pr_colormap(reg_cmap, img->cmaplen);
		break;
	case CFM_SGF:
#ifdef	PIXRECT_NEED_CMAP
		pr_cmap = set_gs_colormap();	/* for Sun pixrect lib	*/
#endif
		break;
	case CFM_BITMAP:
        	depth = 1;
		if (img->in_color == CFM_BITMAP)
			colw = (colw + 7) >> 3;
		break;
	default:prgmerr(-1, "format error %d", img->color_form);
	}

	if (pr_t)	pr_type = pr_t;

	message("create %d-bit Sun-Raster image\n", depth);

	if (!(pr=mem_create(cols, rows, depth)))
		return	prgmerr(0, "create new pixrect");
	data = pr->pr_data->md_image;
	linesize = pr->pr_data->md_linebytes;
	if (colw < cols)	depth = 8;	/* transfer by coping	*/

	if (nobuf = (linesize == colw && depth == 8))
		pr->pr_data->md_image = img->src;
	else for (row=0, inbp=(byte*)img->src; row < rows; ++row) {
	register int	col=colw;
	obp = data;
	switch (depth) {
	case 1: {
	register int	bitcount = 7;
	    for (*obp=col=0; col < cols; col++) {
		if (! inbp[col])
			*obp |= 1 << bitcount;
		if (--bitcount < 0) {
			*++obp = 0;
			bitcount = 7;
		}
	    }
	}	break;
	case 8:
		memcpy(obp, inbp, col);	break;
	case 24:
	case 32:
		if (img->color_form==CFM_ILC)
			ilc_transfer(obp, inbp, colw, 3, Yes, depth>>3),
			inbp += colw << 1;
	else	{	/* ILL	*/
	register byte	*g=inbp+colw, *b=g+colw;
		while (col--) {
			if (alpha)
				*obp++ = (*b | *g | *inbp) ? 255 : 0;
			*obp++ = *b++,
			*obp++ = *g++,
			*obp++ = *inbp++;
		}
		inbp = g;
	}	break;
	default:	return	prgmerr(-'n', "not implemented");
	}

	data += linesize;
	inbp += colw;
	}	/* end of for	*/

	depth = pr_dump(pr, img->OUT_FP, pr_cmap, pr_type, 0);
	free_pr_colormap(pr_cmap);
	if (nobuf)	pr->pr_data->md_image = data;
	CFREE(pr->pr_data->md_image);	/* mem_destroy	*/
	CFREE(pr->pr_data);
	if (depth==PIX_ERR)
		return	prgmerr(0, "RAS writing error %d", depth);
return	depth;
}
