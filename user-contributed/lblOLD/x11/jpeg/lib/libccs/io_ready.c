/*	IO_READY . C
#
%	Copyright (c)	Jin, Guojun -	All rights reserved
%
% Author:	Jin Guojun -	LBL	4/5/95
*/

#include <fcntl.h>
#include <sys/time.h>

io_ready(fd, to_us, ioe)
{
fd_set	ready, *i, *o, *e;
struct timeval	to;
register int	r;

	FD_ZERO(&ready);
	FD_SET(fd, &ready);
	/* slow down if no need to wait	*/
	to.tv_sec = to_us / 100000;	to.tv_usec = to_us % 100000;

	if (!ioe)	i = &ready;
	else if (ioe > 0)	o = &ready;
	else	e = &ready;

	if ((r=select(fd+1, i, o, e, &to)))
		prgmerr(r, "	io_ready %d(%d)	", r, fd);
	else	r = FD_ISSET(fd, &ready);
return	r;	/* True if device is ready	*/
}

