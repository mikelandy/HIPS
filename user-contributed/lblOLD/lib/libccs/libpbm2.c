/* libpbm2.c - pbm utility library part 2
*
* Copyright (C) 1988 by Jef Poskanzer.
*
* Permission to use, copy, modify, and distribute this software and its
* documentation for any purpose and without fee is hereby granted, provided
* that the above copyright notice appear in all copies and that both that
* copyright notice and this permission notice appear in supporting
* documentation.  This software is provided "as is" without express or
* implied warranty.
%
% Modified by	Jin, Guojun - LBL	10/1/91
*/

#include "pbm.h"
#include "libpbm.h"

static bit
pbm_getbit( file )
FILE* file;
{
register int	ch;

do {
	ch = pbm_getc( file );
} while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r');

if ( ch != '0' && ch != '1' )
	return	prgmerr(DEBUGANY, "junk in file where bits should be");

return	(ch == '1') ? 1 : 0;
}

pbm_readmagicnumber(file)
FILE* file;
{
register int	ich1 = getc(file),
		ich2 = getc(file);
    if (ich1==EOF || ich2==EOF)
	return	prgmerr(DEBUGANY, "EOF / PNM read error reading magic number");
return	(ich1 << 8) + ich2;
}

void
pbm_readpbminitrest( file, colsP, rowsP )
FILE*	file;
int	*colsP, *rowsP;
{
/* Read size. */
	*colsP = pbm_getint(file);
	*rowsP = pbm_getint(file);
}


pbm_readpbmrow( file, bitrow, cols, format )
register FILE*	file;
register bit*	bitrow;
int	cols, format;
{
register int	col, bitshift;
register unsigned char	item;

	switch (format)
	{
	case PBM_FORMAT:
	    for (col=0; col < cols; ++col)
		*bitrow++ = pbm_getbit( file );
	break;

	case RPBM_FORMAT:
	bitshift = -1;
	for (col=0; col < cols; ++col) {
	    if (bitshift == -1)	{
		item = pbm_getrawbyte( file );
		bitshift = 7;
	    }
	    *bitrow++ = (item >> bitshift--) & 1;
	}
	break;

	default:	return	prgmerr(DEBUGANY, "PBM read ???");
	}
}
