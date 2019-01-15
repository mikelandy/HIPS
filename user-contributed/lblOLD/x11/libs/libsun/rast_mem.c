/* rast_mem.c - allocate a Sun rasterfile header
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
% AUTHOR:	Jin Guojun - LBL	10/1/91
*/

#include "imagedef.h"

struct	pixrect*
mem_create(int w, int h, int depth)
{
register struct	pixrect	*prt = ZALLOC(1, sizeof(*prt), 0);
if (prt) {
	prt->pr_size.x = w;
	prt->pr_size.y = h;
	prt->pr_depth = depth;
	prt->pr_data = NZALLOC(sizeof(*prt->pr_data), 1, "pr_data");
	depth >>= 3;	depth += !depth;
	prt->pr_data->md_image = NZALLOC(w*h, depth, "md_image");
	prt->pr_data->md_linebytes = depth * w;
	prt->pr_data->md_primary = 1;
}
return	prt;
}

