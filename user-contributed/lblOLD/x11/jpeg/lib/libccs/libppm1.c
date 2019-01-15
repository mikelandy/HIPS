/* libppm1.c - ppm utility library part 1
**
** Copyright (C) 1989 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include "ppm.h"
#include "libppm.h"
#include "pgm.h"
#include "libpgm.h"
#include "pbm.h"
#include "libpbm.h"

void
ppm_init( argcP, argv )
    int* argcP;
    char* argv[];
    {
    pm_init( argcP, argv );
    }

ppm_readppminitrest( file, colsP, rowsP, maxvalP )
FILE* file;
int	*colsP, *rowsP;
pixval*	maxvalP;
{
int	maxval;

    /* Read size. */
    *colsP = pbm_getint( file );
    *rowsP = pbm_getint( file );

    /* Read maxval. */
    maxval = pbm_getint( file );
    if ( maxval > PPM_MAXMAXVAL )
	return	prgmerr(DEBUGANY, "maxval too large %d > %d", maxval, PPM_MAXMAXVAL);
    if (maxvalP)	*maxvalP = maxval;
return	maxval;
}

pixval ppm_pbmmaxval = 1;

void
ppm_readppminit( file, colsP, rowsP, maxvalP, formatP )
    FILE* file;
    int* colsP;
    int* rowsP;
    int* formatP;
    pixval* maxvalP;
    {
    /* Check magic number. */
    *formatP = pbm_readmagicnumber( file );
    switch ( PPM_FORMAT_TYPE(*formatP) )
	{
	case PPM_TYPE:
	ppm_readppminitrest( file, colsP, rowsP, maxvalP );
	break;

	case PGM_TYPE:
	pgm_readpgminitrest( file, colsP, rowsP, maxvalP );
	break;

	case PBM_TYPE:
	pbm_readpbminitrest( file, colsP, rowsP );
	*maxvalP = ppm_pbmmaxval;
	break;

	default:
	pm_error( "bad magic number - not a ppm, pgm, or pbm file" );
	}
    }

#ifdef __STDC__
ppm_readppmrow( FILE* file, pixel* pixelrow, int cols, pixval maxval, int format )
#else /*__STDC__*/
ppm_readppmrow( file, pixelrow, cols, maxval, format )
FILE* file;
pixel* pixelrow;
int cols, format;
pixval maxval;
#endif /*__STDC__*/
{
register int col;
register pixel* pP;
register pixval r, g, b;
register gray* gP;
register bit* bP;
static gray* grayrow;
static bit* bitrow;

    switch ( format )
	{
	case PPM_FORMAT:
	for ( col = 0, pP = pixelrow; col < cols; ++col, ++pP )
	    {
	    r = pbm_getint( file );
#ifdef _DEBUG_
	    if ( r > maxval )
		message("r value out of bounds (%u > %u)", r, maxval);
#endif /*DEBUG*/
	    g = pbm_getint( file );
#ifdef _DEBUG_
	    if ( g > maxval )
		message("g value out of bounds (%u > %u)", g, maxval);
#endif /*DEBUG*/
	    b = pbm_getint( file );
#ifdef _DEBUG_
	    if ( b > maxval )
		message("b value out of bounds (%u > %u)", b, maxval);
#endif /*DEBUG*/
	    PPM_ASSIGN( *pP, r, g, b );
	    }
	break;

	case RPPM_FORMAT:
	for ( col = 0, pP = pixelrow; col < cols; ++col, ++pP )	{
	    r = pbm_getrawbyte( file );
#ifdef _DEBUG_
	    if ( r > maxval )
		message("r value out of bounds (%u > %u)", r, maxval);
#endif /*DEBUG*/
	    g = pbm_getrawbyte( file );
#ifdef _DEBUG_
	    if ( g > maxval )
		message("g value out of bounds (%u > %u)", g, maxval);
#endif /*DEBUG*/
	    b = pbm_getrawbyte( file );
#ifdef _DEBUG_
	    if ( b > maxval )
		message("b value out of bounds (%u > %u)", b, maxval);
#endif /*DEBUG*/
	    PPM_ASSIGN( *pP, r, g, b );
	}
	break;

	case PGM_FORMAT:
	case RPGM_FORMAT:
	if (!grayrow)	grayrow = NZALLOC(cols, sizeof(*grayrow), "gr");
	pgm_readpgmrow( file, grayrow, cols, maxval, format );
	for ( col = 0, gP = grayrow, pP = pixelrow; col < cols; ++col, ++gP, ++pP )
	    {
	    r = *gP;
	    PPM_ASSIGN( *pP, r, r, r );
	    }
	CFREEnNULL(grayrow);
	break;

	case PBM_FORMAT:
	case RPBM_FORMAT:
	if (!bitrow)	bitrow = NZALLOC(cols, sizeof(*bitrow), "br");
	pbm_readpbmrow( file, bitrow, cols, format );
	for ( col = 0, bP = bitrow, pP = pixelrow; col < cols; ++col, ++bP, ++pP )
	    {
	    r = ( *bP == PBM_WHITE ) ? maxval : 0;
	    PPM_ASSIGN( *pP, r, r, r );
	    }
	CFREEnNULL(bitrow);
	break;

	default:return	prgmerr(DEBUGANY, "PPM read ???");
	}
return	cols;
}

pixel**
ppm_readppm( file, colsP, rowsP, maxvalP )
    FILE* file;
    int* colsP;
    int* rowsP;
    pixval* maxvalP;
    {
    pixel** pixels;
    int row;
    int format;

    ppm_readppminit( file, colsP, rowsP, maxvalP, &format );

    pixels = ppm_allocarray( *colsP, *rowsP );

    for ( row = 0; row < *rowsP; ++row )
	ppm_readppmrow( file, pixels[row], *colsP, *maxvalP, format );

    return pixels;
    }
