/* libpgm1.c - pgm utility library part 1 - reading
*
* Copyright (C) 1989 by Jef Poskanzer.
*
* Permission to use, copy, modify, and distribute this software and its
* documentation for any purpose and without fee is hereby granted, provided
* that the above copyright notice appear in all copies and that both that
* copyright notice and this permission notice appear in supporting
* documentation. This software is provided "as is" without express or
* implied warranty.
*/

#include "pgm.h"
#include "libpgm.h"
#include "libpbm.h"

pgm_readpgminitrest(fileP, colsP, rowsP, maxvalP)
FILE*	fileP;
int	*colsP, *rowsP;
gray*	maxvalP;
{
int maxval;

	/* Read size. */
    *colsP = pbm_getint(fileP);
    *rowsP = pbm_getint(fileP);

    /* Read maxval. */
    maxval = pbm_getint(fileP);
    if (maxval > PGM_MAXMAXVAL)
	return	prgmerr(0, "maxval too large: %d > %d", maxval, PGM_MAXMAXVAL);
    else	*maxvalP = maxval;
return	0;
}


#ifdef __STDC__
pgm_readpgmrow(FILE* fileP, gray* grayrow, int cols, gray maxval, int format)
#else
pgm_readpgmrow(fileP, grayrow, cols, maxval, format)
FILE*	fileP;
gray*	grayrow;
int	cols, maxval, format;
#endif	__STDC__
{
register int	col;
register gray*	gP;
register bit*	bP, *bitrow;

switch (format)
{
case PGM_FORMAT:
	for (col=0, gP=grayrow; col < cols; ++col, ++gP)
	{
	    *gP = pbm_getint(fileP);
#ifdef _DEBUG_
	    if (*gP > maxval)
		message("value out of bounds (%u > %u)", *gP, maxval);
#endif
	}
	break;
case RPGM_FORMAT:
	for (col=0, gP=grayrow; col < cols; ++col, ++gP)
	{
	    *gP = pbm_getrawbyte(fileP);
#ifdef _DEBUG_
	    if (*gP > maxval)
		message("value out of bounds (%u > %u)", *gP, maxval);
#endif
	}
	break;
case PBM_FORMAT:
case RPBM_FORMAT:
	pbm_readpbmrow(fileP, bitrow, cols, format);
	for (col=0, gP=grayrow, bP=bitrow; col < cols; ++col, ++gP, ++bP)
	    *gP = (*bP == PBM_WHITE) ? maxval : 0;
	break;
default:	col = prgmerr(0, "can't happen");
}
return	col;
}

