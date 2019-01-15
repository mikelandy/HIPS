/*	BuffeRnW . C
#
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
%	Buffer Operations for Virtual I/O and Piping.
%
% Author:	Jin Guojun - LBL	01/01/1993
*/

#include	"stdef.h"


buffer_read(char *buf, int bsize, register int nblock, register BUFFER *bp)
{
register int	dsz = bp->base + bp->offset + bp->dlen - bp->ptr;

	nblock *= bsize;
	if (dsz > nblock)	dsz = nblock;
	else if (dsz < 1)	return	EOF;

	memcpy(buf, bp->ptr, dsz);
	bp->ptr += dsz;
return	dsz;
}

buffer_write(char *buf, int bsize, register int nblock, register BUFFER *bp)
{
register int	dsz = bp->base + bp->bsize - bp->ptr;

	nblock *= bsize;
	if (dsz < nblock)	return	EOF;

	memcpy(bp->ptr, buf, nblock);
	bp->ptr += nblock;
return	nblock;
}

buffer_seek(register BUFFER *bp, register int n, int job)
{
	switch (job)	{
	case SEEK_GETB:
		if (beof(bp))	break;
		return	*bp->ptr++ & 0xFF;
	case SEEK_UGETB:
		if (bp->base + bp->offset < bp->ptr)
			return	*--bp->ptr = n;
		break;
	case SEEK_PEEK:
		return	bp->ptr[n];
	case SEEK_SET:
		if (0 > n || n > bp->dlen)	break;
		bp->ptr = bp->base + bp->offset + n;
		return	n;
	case SEEK_CUR:
		if (n + bp->ptr - (bp->base + bp->offset) > bp->dlen)	break;
		return	(bp->ptr += n) - (bp->base + bp->offset);
	case SEEK_END:
		if (-bp->dlen > n || n > 0)	break;
		bp->ptr = bp->base + bp->offset + bp->dlen + n;
		return	bp->dlen + n;
	}
return	EOF;
}

BUFFER	*
buffer_create(bsize, hsize)
{
BUFFER	*bp = ZALLOC(1, sizeof(*bp), No);
if (bp)	{
	bp->flags = BUFFER_MAGIC;
	bp->offset = hsize;	/* header size	*/

	if (bsize && !(bp->base = NZALLOC(1, bsize, No)))
		CFREE(bp),	bp = NULL;
	else	bp->bsize = bsize,	bp->ptr = bp->base + hsize;
}
return	bp;
}

void
buffer_close(BUFFER *bp)
{
if (bp->flags == BUFFER_MAGIC)	{
	if (bp->bsize)	CFREE(bp->base);
	CFREE(bp);
}
}
