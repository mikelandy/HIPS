/*	PipeRead . C
#
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

This software is copyrighted (C) by the Lawrence Berkeley Laboratory.
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
Bld. 50B, Rm. 2239, Lawrence Berkeley Laboratory, Berkeley, CA, 94720
(wejohnston@lbl.gov)

For further information about this software, contact:
	Jin Guojun
	Bld. 50B, Rm. 2275, Lawrence Berkeley Laboratory, Berkeley, CA, 94720
	g_jin@lbl.gov

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%	Reading procedure for piping operation
%
% Author:	Jin Guojun - LBL	01/01/1993
*/

#include	"imagedef.h"
#if defined	TC_Need || defined	__STDIO_DEF_
#define	_ptr	curp
#define	_cnt	level
#elif	defined	_linux_
#define	_ptr	_IO_read_ptr
#define	_cnt	_IO_read_end	/* fake	one	*/
#elif	defined	BSD44 | defined	__BSD44_
#define	_ptr	_p
#define	_cnt	_r
#endif

#ifndef	MAX_FILES
#define	MAX_FILES	12
#endif
#ifndef	OVHEAD
#define	OVHEAD	16
#endif

typedef	struct	{
	MType	bleft, bsize, cur, pos;
	short	set, flag;	/* be usable & for what job */
	byte	*inpbuf;
} fpfd;

static	fpfd	ffd[MAX_FILES];
#define	FPT	register fpfd*	ffp = ffd + fileno((FILE*)fp)
static	byte	garbage[OVHEAD];
static	bool	bad_pipe =
#if defined NASI | defined	TC_Need
	False;
#else
	True;
#endif

pipe_read(byte *buf, int block, int nblock, FILE *fp)
{
register int	t = block * nblock;
FPT;
register byte*	temp = ffp->inpbuf;
ffp->inpbuf = buf;	/* use new buf	*/

if (t < 0)	{	/* block < 0, push data back to pseudo-pipe	*/
	ffp->bleft = nblock;
	return	ffp->cur = 0;
}
if (!t && !block)	{	/* block == 0, setup buf_size	*/
	t = nblock;	block = 1;
	ffp->bsize = pointer_buffer_size(buf);
}
if (!ffp->bleft)
	return	fread(buf, block, nblock, fp);
else	{
register int	ret;
	if (ffp->cur == ffp->bsize)	ffp->cur = 0;
	memcpy(buf, temp+ffp->cur, ffp->bleft);
	t -= ffp->bleft;
	ret = fread(buf+ffp->cur, 1, t, fp);
	ffp->bleft = ffp->cur = 0;
	if (t == ret)	return	nblock;
	else	return	ret / block + 1;
}
}

getbyte(FILE	*fp)
{
FPT;

if (ffp->bleft)	{
	if (ffp->cur == ffp->bsize)	ffp->cur = 0;
	ffp->bleft--;	return	ffp->inpbuf[ffp->cur++];
} else	{
	pipe_read(ffp->inpbuf+ffp->cur, 1, 1, fp);
	return	ffp->inpbuf[ffp->cur++];
}
}

ungetbyte(int b, FILE *fp)	/* compatible with ungetc	*/
{
FPT;

if (!ffp->cur)	ffp->cur = ffp->bsize;
ffp->bleft++;	ffp->cur--;
return	ffp->inpbuf[ffp->cur] = b;
}

/*	compatible with fseek when job in 0..3.
	If job < 0, it peek a byte from inpbuf.
*/

peekbyte(FILE *fp, int n, int job)
{
FPT;
if (job < 0)	{
bool	peeked = ffp->flag == SEEK_PEEK;
ffp->flag = job;
	switch (job)	{
	case SEEK_GETB:
pcross:		return	ffp->set ? getbyte(fp) : getc(fp);
	case SEEK_UGETB:
		if (peeked)	return	0;
		return	ffp->set ? ungetbyte(n, fp) : ungetc(n, fp);
	case SEEK_PEEK:
	default:if (!ffp->set)	{
			if (fp->_cnt>0)	return	*fp->_ptr;
			ffp->flag = 0;	goto	pcross;
		}
		if (!ffp->bleft)	{
			pipe_read(ffp->inpbuf+ffp->cur, 1, 1, fp);
			ffp->bleft++;
		}
		return	ffp->inpbuf[ffp->cur];
	}
}
/*	else	*/
if (ffp->set && job == SEEK_CUR && n>0)	{
	while(n--)	getbyte(fp);
	return	n;
}
/*	else	*/
return	fseek(fp, n, job);
}


static	/*	EOF routine for macro feof and beof	*/
ccs_eof(FILE*	fp)
{
register BUFFER	*bp = (BUFFER*)fp;
if (bp->flags == BUFFER_MAGIC)
	return	beof(bp);
return	(feof(fp))
#ifdef	ANY_VALUE_EOF
	!= 0
#endif	/* guarantee to return	True or False	*/
;
}


com_io_sw_t	isys_bio[] = {
#define	ifunc	int(*)()
#define	i_fopen	(ifunc)fopen
	{	(ifunc)buffer_create, (ifunc)buffer_close,
		buffer_read, buffer_seek, ccs_eof	},
	{	(ifunc)buffer_create, (ifunc)buffer_close,
		buffer_write, buffer_seek, ccs_eof	},
	{	i_fopen, fclose, (ifunc)fread, peekbyte, ccs_eof},
	{	i_fopen, fclose, (ifunc)fwrite, fseek, ccs_eof	},
	{	i_fopen, fclose, pipe_read, peekbyte, ccs_eof	},
};

link_buffer(U_IMAGE	*img, bool w)
{
if (w)	img->o.bio = isys_bio + 1;
else	img->i.bio = isys_bio;
}

set_pipe_read(U_IMAGE	*img, int job)
{
img->eof = ccs_eof;

if (fileno(img->IN_FP) < MAX_FILES)	{
register fpfd*	ffp = ffd + fileno(img->IN_FP);

    if (job & bad_pipe)	{	/*	initial for bad pipe	*/
	img->i.bio = isys_bio + 4;
	ffp->cur = ffp->bleft = ffp->flag = ffp->pos = 0;
	ffp->inpbuf = garbage;
	ffp->bsize = OVHEAD;
	return	++ffp->set;
    }	else	{	/*	reset to regular f_io	*/
	img->i.bio = isys_bio + 2;
	img->o.bio = isys_bio + 3;
	return	ffp->set = 0;
    }
}
return	EOF;
}
