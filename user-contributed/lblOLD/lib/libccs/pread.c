/*
%	pread.c - pipe in read subroutine which waits for its input count
%		or EOF
%
@	For use with pipes, which return on read with that which is available
@		rather than that which is requested.
@
@	10/1/90	-- fixed for multiple machine to use by use MType
*/

#ifdef	_DEBUG_
extern	int	debug;
#endif

#include "header.def"
#include "images.def"

#ifdef	IBMPC
#define	MASK	0x0000FFFF
#else
#define	MASK	0xFFFFFFFF
#endif

PType	pread(fd,buf,count)
int	fd;
PType	count;
char	*buf;
{
static PType	memc;
PType	r, cnt = count;

while (cnt > 0) {
	r = MASK & read(fd,buf,(unsigned)cnt);
#ifdef	_DEBUG_
	if (debug)
		if (r!=memc)	msg("pread in %d\n", memc=r);
		else	mesg(" =*= ");
#endif
	if (r <= 0) break;
	buf += r;
	cnt -= r;
  }
return	(PType)(count - cnt);
}
