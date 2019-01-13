/* libpbm4.c - pbm utility library part 4
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
% Modified by	Jin, Guojun - LBL	10/01/91
*/

#include "pbm.h"
#include "libpbm.h"

char
pbm_getc(file)
FILE*	file;
{
register int	ich = getc( file );
	if (ich == EOF)
		return	prgmerr(DEBUGANY, "EOF - pbm_getc");

	if (ich == '#')	{
	    do {
		ich = getc( file );
		if (ich == EOF)
			return	prgmerr(DEBUGANY, "EOF - pbm_getc#");
	    }	while (ich != '\n');
	}

return	(char)ich;
}

unsigned char
pbm_getrawbyte( file )
FILE* file;
{
register int	iby = getc( file );
	if (iby == EOF)
		return	prgmerr(DEBUGANY, "EOF - pbm_getrawbyte");
return	(unsigned char) iby;
}

pbm_getint(file)
FILE* file;
{
register char	ch;
register int	i=0;

    do	ch = pbm_getc(file);	while (ch == ' ' || ch == '\t' || ch == '\n');

    if (ch < '0' || ch > '9')
	return	prgmerr(DEBUGANY, "junk in file where an integer should be");

    do	{
	i = i * 10 + ch - '0';
	ch = pbm_getc(file);
        }
    while (ch >= '0' && ch <= '9');

return	i;
}
