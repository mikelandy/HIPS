/* XWD Read . C
#
%	Copyright (c)	1995	Jin, Guojun
%
% AUTHOR:	Jin Guojun - LBL	7/25/95
*/

#include <X11/X.h>
#include <X11/XWDFile.h>
#include "header.def"
#include "imagedef.h"

static XWDFileHeader	xwd_hd;
/* SunOS 4.1.x has a bug in /usr/openwin/include/X11/XWDFile.h
struct	{
	...
} XWDColor;
--- should be -----
typedef	struct  {
	...
} XWDColor;
*/
static XWDColor *	xwd_cp;	/* same as XColor *	*/
static char	*xwd_name, *xwd_nosup="xwd_r: %s is not support yet",
		*xwd_type[] = {"XYBitmap", "XYPixmap", "ZPixmap"};


#ifndef	XWD_PADS
#define	XWD_PADS	4	/* must be 2^* , so we can make a mask	*/
#endif
#define	XWD_PADMASK	(XWD_PADS - 1)	/* a fast pad mask !	*/

xwd_header_handle(int job, U_IMAGE* img, int ac, char **av, VType *assist)
{
int	i, rev=0;

switch (job) {
case HEADER_READ:
case HEADER_FREAD:
	if (av)	return	EOF;	/* no second frame	*/
	(*img->r_seek)(img->IN_FP, 0, 0);
	(*img->i_read)(&xwd_hd, sizeof(xwd_hd), 1, img->IN_FP);
	i = ntohl(xwd_hd.header_size);
	if (i < sizeof(xwd_hd))
		return	prgmerr(0, "strange xwd header size %d", i);
	if (xwd_hd.file_version != XWD_FILE_VERSION)	{
		rev = ntohl(xwd_hd.file_version) == XWD_FILE_VERSION;
		if (!rev)	message("XWD ver = %d\n", xwd_hd.file_version);
	}
	i -= sizeof(xwd_hd);	/* covers NULL	*/
	if (i)	{
		verify_buffer_size(&xwd_name, sizeof(char*), i, "xwd_name");
		(*img->i_read)(xwd_name, i, 1, img->IN_FP);
#ifdef	_DEBUG_
		message("XWD trailor: %s\n", xwd_name);
#endif
	}
	img->in_type = XWD;
	img->frames = img->channels = 1;
	i = ntohl(xwd_hd.bits_per_pixel) >> 3;
	if (!i)	{
	    switch (ntohl(xwd_hd.pixmap_depth))	{
	    case 1:
		img->in_color = CFM_BITMAP;
		img->in_form = IFMT_BITMAP;
		break;
	    case 8:
		img->in_color = CFM_SCF;
		img->in_form = IFMT_BYTE;
		break;
	    default:	prgmerr(0, xwd_nosup, "XY > 8");
	    }
		img->pxl_in = 1;
	} else	{
		img->in_form = IFMT_BYTE;
		img->pxl_in = i;	/* forcing MIN(3, i) ?	*/
		switch (ac=ntohl(xwd_hd.visual_class))	{
		case StaticGray:	case GrayScale:
			img->in_color = CFM_SGF;	break;
		case StaticColor:	case PseudoColor:
			img->in_color = CFM_SCF;	break;
		case DirectColor:	case TrueColor:
			img->in_color = CFM_ILC;
			img->in_form = IFMT_ILC;
			if (i == 4)	/* to Alpha	*/
				img->in_color++,	img->in_form++;
			img->channels = 3;
		}
	}
	img->cmaplen = 0;
	if (i=ntohl(xwd_hd.ncolors)) {
		verify_buffer_size(&xwd_cp, -sizeof(*xwd_cp), i, "xwd_cp");
		(*img->i_read)(xwd_cp, sizeof(*xwd_cp), i, img->IN_FP);
		if (verify_buffer_size(reg_cmap, sizeof(cmap_t)*3, i, "rcmap"))
			reg_cmap[1] = reg_cmap[0] + i,
			reg_cmap[2] = reg_cmap[1] + i;
		img->cmaplen = i;
		while (i--)	/* ignore xwd_cp[i].pixel ?	*/
			reg_cmap[0][i] = ntohs(xwd_cp[i].red) >> 8,
			reg_cmap[1][i] = ntohs(xwd_cp[i].green) >> 8,
			reg_cmap[2][i] = ntohs(xwd_cp[i].blue) >> 8;
	}
	img->width = ntohl(xwd_hd.pixmap_width);
	img->height = ntohl(xwd_hd.pixmap_height);

	break;
default:message("unknown job %d\n", job);	return	job;
}
return	0;
}

char *	get_xwd_type(form)
{
return	xwd_type[ntohl(xwd_hd.pixmap_format)];
}

#define	unroll_BO8(ops)	\
	for (p=w; p>=8; p-=8)	{	\
		v = *ip++;	\
		repeat_8(ops)	\
	}	v = *ip++;	\
	spread_8(p, ops, p=0)

/*	Only ZPixmap supported at this time.	*/
read_xwd_image(U_IMAGE *img, void *aux, int flags)
{
int	i, w = img->width, i_c = img->pxl_in, o_c = img->dpy_channels,
		p = ntohl(xwd_hd.bytes_per_line) / i_c - w,
		pad=ntohl(xwd_hd.bitmap_pad) >> 3;

	if (ntohl(xwd_hd.bits_per_pixel) == 1)	{
	int	bo = ntohl(xwd_hd.byte_order),
		lw = ntohl(xwd_hd.bytes_per_line);
	/* int	bpu = ntohl(xwd_hd.bitmap_unit), bitp, bop;	*/
	char*	buf = NZALLOC(1, lw, "xwd_xy"), *op=img->src;
	register int	v;
	    switch (ntohl(xwd_hd.pixmap_depth))	{
#ifdef	XWD_XYBitmap
	    case	1:
		bo = ntohl(xwd_hd.bitmap_bit_order);
		for (i=img->height; i--;)	{
		register char*	ip;
			(*img->i_read)(ip=buf, 1, lw, img->IN_FP);
			if (bo)	{
				unroll_BO8(*op++ = v&0x80?255:0; v<<=1)
			} else	{
				unroll_BO8(*op++ = v&1?255:0; v>>=1)
			}
		}
		break;
#endif
	    case	8:	{
		register char*	ip;
		int	pln;
	/*
		if (bo)	bitp = 7,	bop = -1;
		else	bitp = 0,	bop = 1;
	*/
		for (pln=8; pln--; /*bitp+=bop*/)	{
		int	bmask = 1 << pln;
			op = img->src;
		    for (i=img->height; i--;)	{
			(*img->i_read)(ip=buf, 1, lw, img->IN_FP);
			if (bo)	{
				unroll_BO8(if (v&0x80) *op |= bmask; v<<=1; op++)
			} else	{
				unroll_BO8(if (v&1) *op |= bmask; v>>=1; op++)
			}
		    }
		}
	    }	break;
	    default:	CFREE(buf);
			return	prgmerr(0, xwd_nosup, get_xwd_type(1));
	    }
	    CFREE(buf);	i = w * img->height;
	} else if (p)	{
	char*	op = img->src;
	register int	c=img->height, r=i=0,
			inc=(img->color_form==CFM_ILL && i_c>1) ? 0 : i_c * w;
	register char*	bp = inc ? op : (char*)NZALLOC(i_c, w, "xwd_pc");
#ifdef	XWD_PAD_CONFIRM
		pad -= w % pad;	/* waste time to confirm :-) */
		if (p != pad)	pad = p;
	    for (;c--; i+=r, bp+=inc)	{
		r = (*img->i_read)(bp, i_c, w, img->IN_FP);
#else
		pad = p + w;
	    for (;c--; i+=r, bp+=inc)	{
		r = (*img->i_read)(bp, i_c, pad, img->IN_FP);
#endif
		if (r < 1)	break;
		switch (ntohl(xwd_hd.visual_class))	{
		case StaticColor:	case PseudoColor:
			if (!img->color_dpy)
				map8_gray(bp, bp, r, reg_cmap);
			break;
		case DirectColor:	case TrueColor:
			if (o_c < 3)
				return	prgmerr(0, xwd_nosup, "Pad to Sseudo");
			if (!inc)
				any_ilc_to_rle(op, bp, img, i_c, 0),
				op += o_c * w;
			else if (img->color_form == CFM_SGF)
				ilc_to_gray(img->src, img->src, w, i_c==4,
					img->cmaplen ? reg_cmap : NULL, No);
			else if (o_c != i_c)
				ilc_transfer(bp, bp, w, i_c, 0, o_c);
		}
#ifdef	XWD_PAD_CONFIRM
		for (p=pad; p--;)	getc(img->IN_FP);
#else
		if (!c)	pad = w;
#endif
	    }
		if (!inc)	CFREE(bp);
	} else	{
	int	l = img->height;
		i = (*img->i_read)(img->src, i_c, w * l, img->IN_FP);
		switch (ntohl(xwd_hd.visual_class))	{
		case StaticColor:	case PseudoColor:
			if (!img->color_dpy)
				map8_gray(img->src, img->src, i, reg_cmap);
			break;

		case StaticGray:	case GrayScale:
		case DirectColor:	case TrueColor:
			if (img->color_form == CFM_ILL)	{
			char*	bp = NZALLOC(o_c, w * l, "xwd_c"),
				*ip = img->src;	img->src = bp;	img->cnvt = ip;
				while (l--)	{
					any_ilc_to_rle(bp, ip, img, i_c, 0);
					bp += o_c * w;
					ip += i_c * w;
				}
				CFREE(img->cnvt);
			} else if (img->color_form == CFM_SGF)
				ilc_to_gray(img->src, img->src, w * l, i_c==4,
					img->cmaplen ? reg_cmap : NULL, No);
			else if (o_c != i_c)
			ilc_transfer(img->src, img->src, w * l, i_c, 0, o_c);
		}
	}
if ((img->channels=o_c) == 3 && img->in_color==CFM_ALPHA &&
	img->color_form != CFM_ILL)	img->in_color = CFM_ILC;
return	i;
}

