/*	PGM_READ_WRITE . C
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
% AUTHOR:	Jin Guojun - LBL	8/1/91
*/

#include "header.def"
#include "imagedef.h"
#include "pgm.h"

read_pgm_image(img, maxval, format)
U_IMAGE	*img;
{
register int	row, col;
int	total = 0;
gray	*gP, *grayrow = pgm_allocrow(img->width);
/*
verify_buffer_size(&img->src, img->width * img->height, img->pxl_in, "pgsrc");
*/
#ifndef	NO_F_CHECK_on_PGM
if ((col=getc(img->IN_FP)) == EOF)	return	col;
putc(col, img->IN_FP);
#endif
if (img->o_form != IFMT_SHORT)	{
char	*obp=img->src;
    for (row=0; row < img->height; row++){
	total+=pgm_readpgmrow(img->IN_FP, grayrow, img->width, maxval, format);
	if (sizeof(*grayrow) > 1)
	    for (col=img->width, gP=grayrow; col--; gP++)
		*obp++ = *gP++;	/* convert to byte */
	else	memcpy(obp, grayrow, img->width),
		obp += img->width;
    }
}
else	{
short	*obp = (short *)img->src;
    for (row=0; row < img->height; row++) {
	total+=pgm_readpgmrow(img->IN_FP, grayrow, img->width, maxval, format);
	if (img->in_form != img->o_form)
	    for (col=img->width, gP=grayrow; col--; gP++)
		*obp++ = *gP++;	/* convert to short */
	else	memcpy(obp, grayrow, img->width<<1),
		obp += img->width;
    }
}
return	total;
}
