/*	sub_mean . c
%
%	calculate mean value in a sub area for interline 24-bit (RLE) color
%
%	When X0 & y0 zeros, rgb_in & rgb_out point to begining of sub buffer.
%	Otherwise, rgb_in & rgb_out point to begining of original array.
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
        jin@george.lbl.gov

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% AUTHOR:	Jin Guojun - LBL	4/1/92
*/

#include "imagedef.h"

CalcSubWinMean(rgb_in, rgb_out, w, x0, y0, sw, sh)
byte	*rgb_in, *rgb_out;
{
register byte	*pr, *pg, *pb;
register int	c, ir, ig, ib;
int	r;

rgb_in += w*y0*3 + x0;
rgb_out += w*y0*3 + x0;
    for (r=sh, ir=ig=ib=0; r--; rgb_in += w*3)	{
	pr = rgb_in; 	pg = pr + w;	pb = pg + w;
	for (c=sw; c--;)	{
		ir += *pr++;
		ig += *pg++;
		ib += *pb++;
	}
    }
	c = sw * sh;
	ir /= c,	ig /= c,	ib /=c;
    for (r=sh; r--; rgb_out += w*3)	{
	pr = rgb_out; 	pg = pr + w;	pb = pg + w;
	for (c=sw; c--;)	{
		*pr++ = ir;
		*pg++ = ig;
		*pb++ = ib;
	}
    }
}
