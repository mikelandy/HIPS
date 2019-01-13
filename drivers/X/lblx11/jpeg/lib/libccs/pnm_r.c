/* pnm_read.c - read a portable anymap
#
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
@
@ AUTHOR:	Jin Guojun - LBL	8/1/91
*/

#include "header.def"
#include "imagedef.h"

#ifndef	TOP_BITMAP_VALUE
#define	TOP_BITMAP_VALUE	240
#endif

void
#ifdef __STDC__
pnm_readpnmrow(FILE* file, xel* linep, int cols, xelval maxval, int format)
#else
pnm_readpnmrow(file, linep, cols, maxval, format)
FILE*	file;
xel*	linep;
xelval	maxval;
#endif
{
	switch (PNM_FORMAT_TYPE(format))	{
	case PPM_TYPE:
	ppm_readppmrow(file, (pixel*)linep, cols, (pixval) maxval, format);
	break;

	case PGM_TYPE:
	pgm_readpgmrow(file, (gray*)linep, cols, (gray) maxval, format);
	break;

	case PBM_TYPE: {
	register int	col=0;
	register bit	*bP, *bitrow = pbm_allocrow(cols);
		pbm_readpbmrow(file, bitrow, cols, format);
		for (bP = bitrow; col < cols; ++col, ++linep, ++bP)
			PNM_ASSIGN1(*linep, *bP==PBM_BLACK ? 0: maxval);
		pbm_freerow(bitrow);
	}	break;

	default:	prgmerr(format, "can't happen");
	}
}

/*	PNM type !!!
pixel => structure { byte r,g,b } when PPM_MAXMAXVAL != 1023
otherwise	pixel => long
ifdef	PPM
	xel => pixel
else	xel => gray { byte }
*/

read_pnm_image(img, maxval, format, xels, cht, cmap)
U_IMAGE	*img;
xel**	xels;
colorhash_table	cht;	/* a structure pointer */
cmap_t	*cmap[3];
{
register int	row;
xel*	xtmp=NULL;
int	i, cols=img->width, rows=img->height, cfm=img->color_form;
bool	toill = img->channels==1 && cfm==CFM_ILL;

if (cht && !cmap[0])
	rlemap_to_regmap(cmap, &rle_dflt_hdr);
if (!xels | toill)
	xtmp = NZALLOC(cols, img->channels * sizeof(xel), "pnm-tmp");
	/*
	*	The variables at this point are:
	*		img->channels 1, 3
	*	mid_type:	HIPS, RLE, COLOR_PS (include RAS & TIFF)
	*/
    for (row=0; row < rows; ++row) {
	register int	col=0;
	register byte*	obp = (byte*)img->src + row*cols*img->dpy_channels,
		*ptmp = toill ? (byte*) xtmp : obp;	/* slow down <--> powerful */
	register xel*	xP;
	if (xels)	xP = xels[row];
	else	xP = xtmp,
		pnm_readpnmrow(img->IN_FP, xP, cols,
			maxval>1 ? maxval : TOP_BITMAP_VALUE, format);
	switch (img->channels)	{
	case 1:
		switch (PNM_FORMAT_TYPE(format))	{
		case PPM_TYPE:
		    for (; col < cols; ++col, ++xP)	{
			register int	color;
			if (maxval != 255)
				PPM_DEPTH(*xP, *xP, maxval, 255);
			color = ppm_lookupcolor(cht, xP);
			if (color == -1)	return	prgmerr( 0,
			    "color not found ?  row=%d col=%d  r=%d g=%d b=%d",
				row, col, PPM_GETR(*xP), PPM_GETG(*xP),
				PPM_GETB(*xP) );
			ptmp[col] = color;
		    }
		break;
		default:
#	ifdef	PPM
			for (col=cols; col--;)
				obp[col] = xP[col].b;
#	else	/* PGM and PBM, xel = byte */
			memcpy(obp, xP, cols);
#	endif
		}
		if (cht && cfm == CFM_SGF)
			map8_gray(obp, obp, cols, cmap);
		else if (toill)
			ras8_to_rle(obp, ptmp, cols, img, reg_cmap, 1);
	break;
	case 3:	if (cfm == CFM_SGF)
			ilc_to_gray(obp, xP, cols, NULL, NULL, True);
		else if (cfm != CFM_ILC && img->dpy_channels == 3)
			any_ilc_to_rle(obp, xP, img, img->channels, No);
		else	memcpy(obp, xP, cols * img->channels);
/*			for (; col++ < cols; ++xP) {	similar to memcpy
				*obp++ = PPM_GETR( *xP );
				*obp++ = PPM_GETG( *xP );
				*obp++ = PPM_GETB( *xP );
			}
*/	    break;

	    default:	return	prgmerr(DEBUGANY, "PNM read ??");
	    }
	}
if (xtmp)	CFREE(xtmp);
if (img->mid_type == RLE || img->dpy_depth == 1)
	img->channels = img->dpy_channels;	/* for RLE map_scan()	*/
return	rows * cols;
}
