/*	RAST_R.C - read a Sun rasterfile and produce raw data
#
%	Copyright (c)	1991, 1995	Jin Guojun -	All rights reserved
%
% AUTHOR:	Jin Guojun - LBL	10/1/91
*/

#include "header.def"
#include "imagedef.h"

#ifndef	TOP_BITMAP_VALUE
#define	TOP_BITMAP_VALUE	255
#endif

/*	colormap_t	pr_colormap;
	{ int type, lenght; char* map[3] }	SUN raster color map */

struct rasterfile	srhd;
static	bool	grayscale;

RastSWidth(struct rasterfile*	srhd)
{
register int	w = srhd->ras_width;
	if (srhd->ras_depth == 1)	w = w+7 >> 3;
return	w;
}

rast_header_handle(int job, U_IMAGE* img, int ac, char **av, VType *assist)
{
    switch (job) {
    case HEADER_READ:
    case HEADER_FREAD:
	if (!av)
		(*img->r_seek)(img->IN_FP, 0, 0);
	if ((*img->i_read)(&srhd, sizeof(srhd), 1, img->IN_FP) < 1 ||
		srhd.ras_magic != RAS_MAGIC) {
		if (srhd.ras_magic == RAS_MAGIC_REV)	{
		register long_32*	lp = (long_32*)	&srhd;
		for (ac=1; ac<8; ac++)
			lp[ac] = LongSwap(lp[ac]);
		} else	{
		if (DEBUGANY)	prgmerr(0, "not a raster header");
		return	EOF;
		}
	}
	if (srhd.ras_width < 1 || srhd.ras_height < 1)
		return	prgmerr(0, "invalid cols: %d, or rows: %d",
			srhd.ras_width, srhd.ras_height);
	grayscale = 1;
	if (srhd.ras_maplength) {	/* If there are colormaps, read them. */
	register int	i=srhd.ras_maplength / 3;
		if (!read_ras_cmap(img, i, 3))
			return	prgmerr(0, "unable to read colormap data");
		while (i-- && grayscale)
		    grayscale = (reg_cmap[0][i]==reg_cmap[1][i] &&
				reg_cmap[1][i]==reg_cmap[2][i]);
		img->mono_img = (grayscale &= srhd.ras_depth < 24);
	}
	/*========================================================
	byte	red[CMAPSIZE],	// red colormap entries
		green[CMAPSIZE], blue[CMAPSIZE];
	switch(rashdr.ras_maptype) {
	case RMT_RAW:	RAW_COLOR_MAP
		for (i=shdr.ras_maplength; i--;)
			getc(rasfile);	// true color ?
	case RMT_NONE:
		for (i=256; i--;)	red[i]=green[i]=blue[i] = i;
		break;
	case RMT_EQUAL_RGB:	RGB_COLOR_MAP
		i = srhd.ras_maplength/3;
		fread(red, i, 1, rastfile);
		fread(green, ...);	fread(blue, ...);	// ditto
		break;
	default:
	EX_DATAERR, "Unknown cmap type (%d)", rashdr.ras_maptype
	}
	============================================================*/

	img->width = srhd.ras_width;
	img->height = srhd.ras_height;
	img->pxl_in = 1;
	img->in_type = RAS;
	img->in_form = IFMT_SGF;
	img->channels = 3;
	switch(srhd.ras_depth)	{
	case 1:	img->in_color = CFM_BITMAP;
		img->in_form = IFMT_BITMAP;
		img->binary_img = 1;
		goto	next18;
	case 8:	if (grayscale)
			img->in_color = CFM_SGF;
		else	img->in_color = CFM_SCF,
			img->in_form = IFMT_SCF;
next18:		if (grayscale || !img->color_dpy || assist)	{
		register int	i = srhd.ras_maplength;	img->cmaplen = i/3;
		    if (assist && i)
			img->color_form = img->in_color;
		    img->channels = img->mono_img = 1;
		}
		break;
	case 24:
		img->in_color = CFM_ILC;
		img->in_form = IFMT_ILC;
		break;
	case 32:img->in_color = CFM_ALPHA;
		img->in_form = IFMT_ALPHA;
	break;
	default:
	return	prgmerr(0, "invalid depth: %d; Only handle 1, 8, 24, or 32.",
			srhd.ras_depth);
	}
	/* loading depends on depth, so set up dpy_channels for conversion */
	break;
    case HEADER_TO:
	srhd.ras_width = img->width;
	srhd.ras_height = img->height;
	if ((srhd.ras_depth = img->channels << 3) == 8)
		srhd.ras_maplength = img->cmaplen * img->channels,
		srhd.ras_maptype = RMT_EQUAL_RGB,
		srhd.ras_type = ac ? ac : RT_BYTE_ENCODED;
	else	srhd.ras_type = RT_FORMAT_RGB;
	break;
    case HEADER_WRITE:
    default:	return	prgmerr(0, "no such a job %d\n", job);
    }
return	0;
}


#ifndef	RAS_RLE_ECODE
#define	RAS_RLE_ECODE	128
#endif
#define	GETC(fp)	(*img->r_seek)(fp, 0, SEEK_GETB)

load_rastfile(byte	*buf,
	struct rasterfile*	srhd,
	register int	len,
	register U_IMAGE*	img)
{
FILE	*fp = img->IN_FP;
double	junk;
register long_32	p = srhd->ras_depth>>3,
	pad = RAST_ODD_PAD(srhd->ras_width),
	i = srhd->ras_width*srhd->ras_height;
if (!len)	len = i;
if (! p)	p++;

if (srhd->ras_type==RT_BYTE_ENCODED) {
int	pad_len = srhd->ras_width * p;
register int	c, w;
    for (w=pad_len, i *= p; i-- > 0 && len--;)	{
	p = GETC(fp);
	if (p==RAS_RLE_ECODE) {
	    c = GETC(fp);	len--;
	    if (c)	{
		p = GETC(fp);	len--;
		if (c > i)	c = i;
		i -= c;	w -= c;
		while (c--)	*buf++ = p;
	    }
	}
	*buf++ = p;
	if (pad && --w <= 0)	{
#if	(RAST_8PAD == 2)
		buf--;
#else
		c = GETC(fp);
#endif
		w = pad_len;
	}
    }
}
else if (srhd->ras_width & p & 1)
	for (p *= RastSWidth(srhd), len=srhd->ras_height; len--; buf += p) {
		if ((*img->i_read)(buf, 1, p, img->IN_FP) != p)
			return	prgmerr(0, "load rastline %d", len);
		(*img->i_read)(&junk, 1, pad, img->IN_FP);
	}
    else if ((p=(*img->i_read)(buf, 1, len, img->IN_FP)) != len)
	return	prgmerr(0, "read rastfile %d", p);
    else if (srhd->ras_maplength && grayscale)	{
	/* stupid map only for RT_ST (1) ? */
	while (p--)	buf[p] = reg_cmap[0][buf[p]];
	CFREEnNULL(reg_cmap[0]);
    }
return	srhd->ras_width * srhd->ras_height;
}


#ifndef	V_ASSIGN
#define	V_ASSIGN(p, r,g,b)	(p) = ((MType)(r)<<20) | ((MType)(g)<<10) | (MType)(b)
#endif

read_rast_image(U_IMAGE	*img, struct rasterfile	*srhd, bool OsameI8)
{
int	dc, depth=srhd->ras_depth, cols, mask, icf=img->color_form,
	zero, one, swap=srhd->ras_type!=RT_FORMAT_RGB && depth>8,
	row=one=0, rows = srhd->ras_height;
register int	col = cols=srhd->ras_width;

switch(depth) {	/* Check the depth and color map */
case 1:
	if (srhd->ras_maptype==RMT_NONE && !srhd->ras_maplength)
		zero = TOP_BITMAP_VALUE;
	else if (srhd->ras_maptype == RMT_EQUAL_RGB && srhd->ras_maplength == 6)
	    if (grayscale)
		zero = reg_cmap[0][0],
		one = reg_cmap[0][1];
	    else
		V_ASSIGN(zero, reg_cmap[0][0], reg_cmap[1][0], reg_cmap[2][0]),
		V_ASSIGN(one, reg_cmap[0][1], reg_cmap[1][1], reg_cmap[2][1]);
	else	goto	cmaperr;
	break;
case 8:
	if (grayscale || srhd->ras_maptype==RMT_EQUAL_RGB)	break;
cmaperr:
	return	prgmerr(0,
		"depth-%d has a non-standard colormap-type %d length %d",
		depth, srhd->ras_maptype, srhd->ras_maplength);
case 24:
case 32:
	if (srhd->ras_maptype != RMT_NONE || srhd->ras_maplength)
		prgmerr(0, "strange: depth-%d rasterfile with colormaps", depth);
	break;
	default:return	prgmerr(0,
		"invalid depth: %d.  Can only handle 1, 8, 24, or 32",	depth);
}

/*	converting	*/
#define	Next_Sample(mask, ibp)	\
	if (!(mask >>= 1))	{	mask = 0x80;	ibp++;	}

    if (depth==8 && (OsameI8 || !srhd->ras_maplength ||
			img->mid_type==COLOR_PS || icf == CFM_SEPLANE))
	return	load_rastfile(img->src, srhd, srhd->ras_length, img);

    else {
	byte*	ibuf = nzalloc(img->width*img->height, depth==1 ? 1 : depth>>3,
				"ras-ibuf");
	register byte	*ibp = ibuf, *obp = (byte*)img->src;

	if ((dc=load_rastfile(ibuf, srhd, srhd->ras_length, img)) < 0)
		return	dc;
	dc = img->dpy_channels;
	if (!swap && img->in_color == icf)	{
		img->src = ibp;	ibuf = obp;
	}
	else for (row=rows; row--;) {
	switch (depth) {
	case 1:
		mask = 0x80;
		for (col=0; col < cols; col++)	{
			*obp++ = (*ibp & mask) ? one : zero;
			Next_Sample(mask, ibp);
		}
		if (mask & 0x7F)	ibp++;
		break;
	case 8:
		if (grayscale)
		    unroll8_fwd(col=0, col, cols, *obp++ = reg_cmap[0][*ibp++])
		else{
		    if (icf != CFM_SGF)
			ras8_to_rle(obp, ibp, col, img, reg_cmap, rows),
			row=0;
		    else
			map8_gray(obp, ibp, col, reg_cmap);
		    ibp += col;
		    obp += col * dc;
		}
		break;
	case 24:
	case 32:
		if (icf==CFM_ILC || icf==CFM_SEPLANE)	/* BGR -> RGB */
			ilc_transfer(obp, ibp, cols, depth>>3, swap, 3);
		else if (icf!=CFM_SGF || img->mid_type==COLOR_PS)
			any_ilc_to_rle(obp, ibp, img, depth>>3, swap);
		else
			ilc_to_gray(obp, ibp, col, depth==32,
				srhd->ras_maplength ? reg_cmap : NULL, swap);
		ibp += col * (depth>>3);
		obp += col * dc;
		break;
	default:prgmerr(cols=row=0, "cannot happen");
	}
	}	/* end for */
	CFREE(ibuf);
    }
if (!img->color_dpy || img->dpy_depth == 1)
	img->channels = 1;
else	img->channels = dc;
return	cols * (rows-row);
}
