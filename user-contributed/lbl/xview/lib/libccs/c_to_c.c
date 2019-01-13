/*	Color_TO_Color . C	color converter
#
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

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
Bld. 50B, Rm. 2239, Lawrence Berkeley National Laboratory, Berkeley, CA, 94720.
(wejohnston@lbl.gov)

For further information about this software, contact:
	Jin Guojun
	Bld. 50B, Rm. 2275
	Lawrence Berkeley National Laboratory, Berkeley, CA, 94720.
	g_jin@lbl.gov

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% AUTHOR:	Jin Guojun - LBL	1/1/90
*/

#include "imagedef.h"


#define	Next_Sample(bit_shift, bitspsmp, p)	\
{	if (!bit_shift)	{	bit_shift = 8,	p++;	}	\
	bit_shift -= bitspsmp;	}

ras8_to_rle(byte *obp, byte *ibp, int w, U_IMAGE *img, cmap_t*	cmap[3], int h)
{
register int	i, c=img->dpy_channels;	/* c === 3	*/
byte	*cp[3];
#define	ccvt(c)	cp[c][i] = cmap[c][ibp[i]]
while (h--)	{
	for (cp[i=0]=obp; ++i<c; cp[i] = cp[i-1] + w);
	obp += (i=w) * c;
	while (i--)	{ ccvt(0);	ccvt(1);	if (c>2) ccvt(2); }
	ibp += w;
}
}


snf_to_rle(obp, ibp, w, bps, cmp)
byte	*obp, *ibp, *cmp[];
{
register int	pxl, shift=8;
byte	*cp[3];
	cp[0] = obp;
	cp[1] = obp + w;
	cp[2] = cp[1] + w;
	if (cmp) while (w--)	{
		Next_Sample(shift, bps, ibp);
		pxl = *ibp >> shift;
		*cp[0]++ = cmp[0][pxl];
		*cp[1]++ = cmp[1][pxl];
		*cp[2]++ = cmp[2][pxl];
	} else	unroll8_bwd(, w, *cp[0]++ = *cp[1]++ = *cp[2]++ = *ibp++);
}


any_ilc_to_rle(byte *obp, byte *ibp, U_IMAGE	*img, int channels, int revs)
{
register int	i, w=img->width, oc=img->dpy_channels;
bool	alpha = channels==4;
byte	*cp[3];

for (cp[i=0] = obp; ++i<oc;)
	cp[i] = cp[i-1] + w;
ibp += channels * w;

if (revs) while (w--)	{
	for (i=0; i<oc; i++)
		cp[i][w] = *(--ibp);
	ibp -= alpha;
} else while (w--)	{
	for (i=oc; i--;)
		cp[i][w] = *(--ibp);
	ibp -= alpha;
	}
}

line_to_cell_color(register char* obp, register char* r, int w, int h)
{
register int	col;
register char*	g, *b=r;
    while (h--) {
	r=b;	g=r+w;	b=g+w;
	unroll8_bwd(col=w, col, *obp++ = *r++; *obp++ = *g++; *obp++ = *b++)
    }
}

ilc_transfer(op, ip, w, inch, rev, och)
char	*op, *ip;
register int	w, rev;
{
register int	oe=0;
if (!rev)	{
	if (inch == och)	return	memcpy(op, ip, w*inch);
	rev = 2;
} else	rev = oe,	oe = 2;
ip += inch > 3;
op += och > 3;
unroll8_bwd(, w,
	*op = ip[oe];	op[1] = ip[1];	op[2] = ip[rev]; ip+=inch; op+=och)
}
