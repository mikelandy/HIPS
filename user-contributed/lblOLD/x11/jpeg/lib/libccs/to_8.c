/*	TO_8 . C
#
%	little slow but can handle both ILL & ILC in one
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
% AUTHOR:	Jin Guojun - LBL
*/

#include "header.def"
#include "imagedef.h"

#define	RED	0
#define	GREEN	1
#define	BLUE	2
#define	ADJ_BTM_RIGHT	0
#define	ADJ_BTM_LEFT	1
#define	ADJ_BOTTOM	2
#define	ADJ_NEXT	3
#define truncate(a, b, c)	((a<b) ? a=b : (a>c) ? a=c : a)

static	byte	fsb_table[4][MaxColors];


To_8(img, rg_cmap, quant, nc)
U_IMAGE	*img;
cmap_t	*rg_cmap[];
{
int	w = img->width, h = img->height, is_rle = img->color_form==CFM_ILL;
VType	*tbuf = NZALLOC(w, h, "to_8");

if (verify_buffer_size(rg_cmap, MaxColors, 3*SIZEOF(rg_cmap[0]), "to_8-cmap"))
	rg_cmap[1] = rg_cmap[0] + MaxColors,
	rg_cmap[2] = rg_cmap[1] + MaxColors;

if (!(img->cmaplen=TrueCheck(rg_cmap, img->src, tbuf, w, h, is_rle, nc)))
#ifdef	QUANTIZING_TO_8
    if (quant)
	quant = quant_to_8(img, img->cmaplen=nc, rg_cmap, tbuf);
    else
#endif
	/* for raw input, set img->color_form = img->in_color	*/
	img->cmaplen = MaxColors,
	quant = dither_to8(rg_cmap, img->src, tbuf, w, h, is_rle);

img->color_form = CFM_SCF;
img->in_form = IFMT_SCF;
img->channels = img->mono_img = 1;
CFREE(img->src);
img->src = tbuf;
return	quant;
}

void
init_FS_tables(rg_cmap)	/* initialize Floyd-Steinberg tables */
cmap_t	*rg_cmap[];
{
register int	i=MaxColors;
	while (i--) {
		fsb_table[ADJ_BTM_RIGHT][i] = i >> 4;
		fsb_table[ADJ_BTM_LEFT][i] = 3*i >> 4;
		fsb_table[ADJ_BOTTOM][i] = 5*i >> 4;
		fsb_table[ADJ_NEXT][i] = 7*i >> 4;
		if (!rg_cmap)	continue;
		/* build colormap (RRRGGGBB)	*/
		rg_cmap[RED][i] =  ((i&0xE0) * 255) / 0xE0;
		rg_cmap[GREEN][i]= ((i&0x1C) * 255) / 0x1C;
		rg_cmap[BLUE][i] = ((i&0x03) * 255) / 0x03;
	}
}

static
dither_to8(rg_cmap, rgbp, obuf, w, h, is_rle)
cmap_t	*rg_cmap[];
byte	*rgbp, *obuf;
int	w, h, is_rle;
{
byte	*pp = obuf;
int	pinc, rnext, gnext, bnext, rleft, gleft, bleft, rright, gright, bright,
	i, j, rerr, gerr, berr, rgb_width = w * 3,
	imax = h-1, jmax = w-1;
int	*line1, *line2, *line0=(int *)NZALLOC(rgb_width, sizeof(int)<<1, No);
register int	*cur_p, *dither_p, dr, dg, db;

	init_FS_tables(rg_cmap);

	/* floyd-steinberg dithering.
	%
	% no-effect	x	+ 7/16*err
	% +3/16*err  +5/16*err	+ 1/16*err
	%
	%	err = x mod (2 ** bits)
	*/

	line2 = (line1=line0) + rgb_width;

	rnext = 0;
	if (is_rle)	{
		pinc = 1;	gnext = rnext + w;	bnext = gnext + w;
		rleft = -1;	gleft = gnext + rleft;	bleft = bnext + rleft;
		rright = 1;	gright = gnext+rright;	bright = bnext+rright;
	} else	{
		pinc = 3;	gnext = 1;	bnext = 2;
		rleft = -3;	gleft = -2;	bleft = -1;
		rright = 3;	gright = 4;	bright = 5;
	}

	/*	convert first line to int	*/
	for (j=rgb_width, cur_p=line1; j--;)
		*cur_p++ = *rgbp++;

	for (i=0; i<h; i++) {

	    if (i<imax)   /* convert next line */
		for (j=rgb_width, cur_p=line2; j--;)
			*cur_p++ = *rgbp++;

	    for (j=0, cur_p=line1, dither_p=line2; j<w; j++) {
		dr = cur_p[rnext];
		dg = cur_p[gnext];
		db = cur_p[bnext];
		cur_p += pinc;
		truncate(dr,0,255);  truncate(dg,0,255);  truncate(db,0,255);
		rerr = dr & 0x1F;  gerr = dg & 0x1F;  berr = db & 0x3F;

		*pp++ = dr & 0xE0 | (dg>>3) & 0x1C | (db>>6);

		if (j<jmax) {
			cur_p[rnext] += fsb_table[ADJ_NEXT][rerr];
			cur_p[gnext] += fsb_table[ADJ_NEXT][gerr];
			cur_p[bnext] += fsb_table[ADJ_NEXT][berr];
		}

		if (i<imax) {
		    dither_p[rnext] += fsb_table[ADJ_BOTTOM][rerr];
		    dither_p[gnext] += fsb_table[ADJ_BOTTOM][gerr];
		    dither_p[bnext] += fsb_table[ADJ_BOTTOM][berr];

		    if (j) {
			dither_p[rleft] += fsb_table[ADJ_BTM_LEFT][rerr];
			dither_p[gleft] += fsb_table[ADJ_BTM_LEFT][gerr];
			dither_p[bleft] += fsb_table[ADJ_BTM_LEFT][berr];
		    if (j<jmax) {
			dither_p[rright] += fsb_table[ADJ_BTM_RIGHT][rerr];
			dither_p[gright] += fsb_table[ADJ_BTM_RIGHT][gerr];
			dither_p[bright] += fsb_table[ADJ_BTM_RIGHT][berr];
		    }	}
		    dither_p += pinc;
		}
	    }
	/*	line1 ^= line2;  line2 ^= line1;  line1 ^= line2;	*/
	cur_p = line1;	line1 = line2;	line2 = cur_p;
	}
CFREE(line0);
return	0;
}

static
TrueCheck(rg_cmap, rgbp, obuf, iw, ih, is_rle, maxcol)
cmap_t	*rg_cmap[];
byte	*rgbp, *obuf;
{
longword	colors[MaxColors],col;
int	i, j, nc, low, high, mid, pinc=1;
byte	*pr, *pg, *pb=rgbp, *pix;

	if (maxcol>MaxColors)	maxcol = MaxColors;

	if (!is_rle)
		pr = pb,	pg = pr + 1,	pb = pg + 1,	pinc = 3;
	for (nc=mid=0, i=ih; i--;) {
	    if (is_rle)
		pr = pb,	pg = pr + iw,	pb = pg + iw;
	    for (j=iw; j--;)	{
		col = *pr << 16;	pr += pinc;
		col += *pg << 8;	pg += pinc;
		col += *pb;	pb += pinc;

		/* binary search the 'colors' array to see if it's in there */
		low = 0;	high = nc-1;
		while (low <= high) {
			mid = low + high >> 1;
			if (col < colors[mid]) high = mid - 1;
			else if (col > colors[mid]) low  = mid + 1;
			else	break;
		}

		if (high < low) { /* if not in list, add it in */
			if (nc >= maxcol)	return	0;
			/*	do overlapped copy	*/
#	ifdef	NO_BCOPY
			for (high=nc; high>low; high--)
				colors[high] = colors[high-1];
#	else
			bcopy(colors+low, colors+low+1, (nc-low) * sizeof(*colors));
#	endif
			colors[low] = col;
			nc++;
		}
	    }
	}

	/* run through the data a 2nd time to convert to 8	*/
	pb = rgbp;
	if (!is_rle)
		pr = pb,	pg = pr + 1,	pb = pg + 1;
	for (i=ih, pix=obuf; i--;) {
	    if (is_rle)
		pr = pb,	pg = pr + iw,	pb = pg + iw;
	    for (j=iw; j--;)	{
		col = *pr << 16;	col += *pg << 8;	col += *pb;
		pr += pinc;	pg += pinc;	pb += pinc;

		low = 0;	high = nc-1;
		while (low <= high) {
			mid = low + high >> 1;
			if (col < colors[mid]) high = mid - 1;
			else if (col > colors[mid]) low  = mid + 1;
			else	break;
		}
		if (high < low)
			prgmerr(DEBUGANY, "TrueColor Check: double entry");
		*pix++ = mid;	/* set output */
	    }
	}

	for (i=nc; i--;) {	/* set colormap */
		rg_cmap[RED][i] =  (colors[i]>>16) & 0xFF;
		rg_cmap[GREEN][i] = (colors[i]>>8) & 0xFF;
		rg_cmap[BLUE][i] =  colors[i] & 0xFF;
	}
return	nc;
}
