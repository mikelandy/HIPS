/* ppmtotga.c - read a portable pixmap and produce a TrueVision Targa file
**
** Copyright (C) 1991 by Mark Shand
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include <ctype.h>
#include "ppm.h"
#include "ppmcmap.h"
#include "tga.h"

/* Max number of colors allowed in ppm input. */
#define MAXCOLORS 256

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    pixel** pixels;
    register pixel* pP;
    int argn, rows, cols, ncolors, row, col, i, format, realrow;
    pixval maxval;
    colorhash_table cht;
    colorhist_vector chv;
    char out_name[100];
    char* cp;
    char* usage = "[-name <tganame>] [-mono|-cmap|-rgb] [ppmfile]";
    struct ImageHeader tgaHeader;

    ppm_init( &argc, argv );
    out_name[0] = '\0';

    argn = 1;

    tgaHeader.ImgType = TGA_Null;
    /* Check for command line options. */
    while ( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' )
        {
        if ( pm_keymatch( argv[argn], "-name", 2 ) )
            {
            ++argn;
            if ( argn == argc )
        	pm_usage( usage );
            (void) strcpy( out_name, argv[argn] );
            }
        else if ( pm_keymatch( argv[argn], "-cmap", 2 ) )
            tgaHeader.ImgType = TGA_Map;
        else if ( pm_keymatch( argv[argn], "-mono", 2 ) )
            tgaHeader.ImgType = TGA_Mono;
        else if ( pm_keymatch( argv[argn], "-rgb", 2 ) )
            tgaHeader.ImgType = TGA_RGB;
        else
            pm_usage( usage );
        ++argn;
        }

    if ( argn != argc )
        {
        /* Open the input file. */
        ifp = pm_openr( argv[argn] );

        /* If output filename not specified, use input filename as default. */
        if ( out_name[0] == '\0' )
            {
            (void) strcpy( out_name, argv[argn] );
            cp = index( out_name, '.' );
            if ( cp != 0 )
        	*cp = '\0';	/* remove extension */
            if ( strcmp( out_name, "-" ) == 0 )
        	(void) strcpy( out_name, "noname" );
            }

        ++argn;
        }
    else
        {
        /* No input file specified. */
        ifp = stdin;
        if ( out_name[0] == '\0' )
            (void) strcpy( out_name, "noname" );
        }

    if ( argn != argc )
        pm_usage( usage );

    /* Read in the ppm file. */
    ppm_readppminit( ifp, &cols, &rows, &maxval, &format);
    pixels = ppm_allocarray(cols, rows);
    for (row = 0; row < rows; row++)
	ppm_readppmrow(ifp, pixels[row], cols, maxval, format);
    pm_close( ifp );

    /* Figure out the colormap. */
    switch (PPM_FORMAT_TYPE(format))
    {
        case PPM_TYPE:
            if (tgaHeader.ImgType == TGA_Mono)
                pm_error("input not a graymap, filter through ppmtopgm first");
            if (tgaHeader.ImgType == TGA_Null || tgaHeader.ImgType == TGA_Map)
            {
                pm_message( "computing colormap..." );
                chv = ppm_computecolorhist( pixels, cols, rows, MAXCOLORS, &ncolors );
                if ( chv == (colorhist_vector) 0 )
                {
                    if (tgaHeader.ImgType == TGA_Map)
                        pm_error(
                            "too many colors - try doing a 'ppmquant %d'",
                            MAXCOLORS);
                    else
                        tgaHeader.ImgType = TGA_RGB;
                }
                else
                {
                    pm_message( "%d colors found", ncolors );
                    if (tgaHeader.ImgType == TGA_Null)
                        tgaHeader.ImgType = TGA_Map;
                }
            }
            break;
        case PGM_TYPE:
        case PBM_TYPE:
            if (tgaHeader.ImgType == TGA_Null)
                tgaHeader.ImgType = TGA_Mono;
            else if (tgaHeader.ImgType == TGA_Map)
            {
                pm_message( "computing colormap..." );
                chv = ppm_computecolorhist( pixels, cols, rows, MAXCOLORS, &ncolors );
                if ( chv == (colorhist_vector) 0 )
                    pm_error("can't happen");
                pm_message( "%d colors found", ncolors );
            }
            break;

        default:
            pm_error( "can't happen");
    }

    tgaHeader.IDLength = 0;
    
    if (tgaHeader.ImgType == TGA_Map)
    {
        /* Make a hash table for fast color lookup. */
        cht = ppm_colorhisttocolorhash( chv, ncolors );

        tgaHeader.CoMapType = 1;
        tgaHeader.Index_lo = 0;
        tgaHeader.Index_hi = 0;
        tgaHeader.Length_lo = ncolors%256;
        tgaHeader.Length_hi = ncolors/256;
        tgaHeader.CoSize = 24;
        tgaHeader.PixelSize = 8;
    }
    else
    {
        tgaHeader.CoMapType = 0;
        tgaHeader.Index_lo = 0;
        tgaHeader.Index_hi = 0;
        tgaHeader.Length_lo = 0;
        tgaHeader.Length_hi = 0;
        tgaHeader.CoSize = 0;
        tgaHeader.PixelSize = (tgaHeader.ImgType == TGA_RGB) ? 24 : 8;
    }
    tgaHeader.X_org_lo = tgaHeader.X_org_hi = 0;
    tgaHeader.Y_org_lo = tgaHeader.Y_org_hi = 0;
    tgaHeader.Width_lo = cols%256; tgaHeader.Width_hi = cols/256;
    tgaHeader.Height_lo = rows%256; tgaHeader.Height_hi = rows/256;
    tgaHeader.AttBits = 0;
    tgaHeader.Rsrvd = 0;
    tgaHeader.IntrLve = 0;

    /* Write out the TGA header. */
    writetga(&tgaHeader, NULL);

    if (tgaHeader.ImgType == TGA_Map)
    {
        /* Write out the TGA colormap. */
        for (i = 0; i < ncolors; i++)
            put_map_entry(chv[i].color, tgaHeader.CoSize, maxval);
    }

    /* Write out the pixels */
    for ( row = 0; row < rows; ++row )
        {
            realrow = row;
            if ( tgaHeader.OrgBit == 0 )
                realrow = rows - realrow - 1;
            if (tgaHeader.ImgType == TGA_Map)
                for ( col = 0; col < cols; col++ )
                    putchar(ppm_lookupcolor(cht, pixels[realrow]+col));
            else if (tgaHeader.ImgType == TGA_Mono)
                for ( col = 0; col < cols; col++ )
                {
                    pixel sP;
                    PPM_DEPTH(sP, pixels[realrow][col], maxval, (pixval) 255);
                    putchar((int) PPM_LUMIN(sP));
                }
            else if (tgaHeader.ImgType == TGA_RGB)
                for ( col = 0; col < cols; col++ )
                {
                    pixel sP;
                    PPM_DEPTH(sP, pixels[realrow][col], maxval, (pixval) 255);
                    putchar(PPM_GETB(sP));
                    putchar(PPM_GETG(sP));
                    putchar(PPM_GETR(sP));
                }
        }
    exit( 0 );
    }

writetga( tgaP, id)
struct ImageHeader *tgaP;
char *id;
{
    unsigned char flags;

    putchar(tgaP->IDLength);
    putchar(tgaP->CoMapType);
    putchar(tgaP->ImgType);
    putchar(tgaP->Index_lo);
    putchar(tgaP->Index_hi);
    putchar(tgaP->Length_lo);
    putchar(tgaP->Length_hi);
    putchar(tgaP->CoSize);
    putchar(tgaP->X_org_lo);
    putchar(tgaP->X_org_hi);
    putchar(tgaP->Y_org_lo);
    putchar(tgaP->Y_org_hi);
    putchar(tgaP->Width_lo);
    putchar(tgaP->Width_hi);
    putchar(tgaP->Height_lo);
    putchar(tgaP->Height_hi);
    putchar(tgaP->PixelSize);
    flags = (tgaP->AttBits & 0xf)
            | ((tgaP->Rsrvd & 0x1) << 4)
            | ((tgaP->OrgBit & 0x1) << 5)
            | ((tgaP->OrgBit & 0x3) << 6);
    putchar(flags);
    if (tgaP->IDLength)
        fwrite( id, 1, (int) tgaP->IDLength, stdout);
}
    
put_map_entry(Value, Size, maxval) 
pixel Value;
int Size;
pixval maxval;
{
    int j;
    pixel sP;
    
    switch ( Size )
    {
	case 8:				/* Grey scale. */
	
	PPM_DEPTH(sP, Value, maxval, (pixval) 255);
        putchar((int) PPM_LUMIN(sP));
	break;

	case 16:			/* 5 bits each of red green and blue. */
	case 15:			/* Watch for byte order. */
	PPM_DEPTH(sP, Value, maxval, 31);
	j = (int) PPM_GETB(sP)
	  | ((int) PPM_GETG(sP) << 5)
	  | ((int) PPM_GETR(sP) << 10);
	putchar(j%256);
	putchar(j/256);
	break;

	case 32:
	case 24:			/* 8 bits each of blue green and red. */
	PPM_DEPTH(sP, Value, maxval, (pixval) 255);
	putchar(PPM_GETB(sP));
	putchar(PPM_GETG(sP));
	putchar(PPM_GETR(sP));
	break;

	default:
	pm_error( "unknown colormap pixel size (#2) - %d", Size );
    }
}
