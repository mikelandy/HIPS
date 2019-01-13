/*	socket_subr . c
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
	Bld. 50B, Rm. 2275, Lawrence Berkeley Laboratory, Berkeley, CA, 94720
	g_jin@lbl.gov

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%	Scoket subroutines
%
% AUTHOR:	Jin Guojun - LBL	01/01/93
*/

#include <sys/file.h>
#include "net_need.h"

short
get_addr(char* host, long* addr_op)
{
short	family=AF_INET;
union	{
	long	i;
	unsigned char	c[4];
} addr;
if (isdigit(*host))	{
#ifdef	NO_INET
int	a[4];
	sscanf(host, "%d.%d.%d.%d", a, a+1, a+2, a+3);
	addr.c[0] = a[0], addr.c[1] = a[1], addr.c[2] = a[2], addr.c[3] = a[3];
	*addr_op = addr.i;
	message("IP => %d.%d.%d.%d\n", a[0], a[1], a[2], a[3]);
#else
	*addr_op = inet_addr(host);
#endif
}
else	{
struct	hostent	*h_p;	/* host ptr */
	if ((h_p=gethostbyname(host)) == NULL)
		syserr("? host %s", host);
	memcpy(addr_op, h_p->h_addr_list[0], h_p->h_length);
	family = h_p->h_addrtype;
}
#ifdef	_DEBUG_
	message("IP = %s\n", inet_ntoa(
#ifndef	sparc	/*	Sun Bug	*/
			*
#endif
				addr_op));
#endif
return	family;
}

short
get_port(char* s_name, bool udp)
{
struct	servent	*s_p;
if (isdigit(*s_name))	return	htons(atoi(s_name));
	if (!(s_p = getservbyname(s_name, udp ? "udp" : "tcp")))
		syserr("? server %s", s_name);
	return	s_p->s_port;	/* in network short byte order	*/
}


s_buf_size(int s, int buf_size)
{
int	n;
#ifdef	_DEBUG_
int	o_test, test;
	n = sizeof(test);
	n = getsockopt(s, SOL_SOCKET, SO_RCVBUF, &o_test, &n);
#endif
	if (setsockopt(s, SOL_SOCKET, SO_RCVBUF, &buf_size,
		sizeof(buf_size) ) < 0)
		prgmerr(1, "sockbuf size %d", buf_size);
#ifdef	_DEBUG_
	n = sizeof(test);
	n = getsockopt(s, SOL_SOCKET, SO_RCVBUF, &test, &n);
	message("recv size from %d to %d [%d]\n", o_test, test, n);

	n = sizeof(test);
	n = getsockopt(s, SOL_SOCKET, SO_SNDBUF, &o_test, &n);
#endif
	if (setsockopt(s,SOL_SOCKET,SO_SNDBUF, &buf_size,
		sizeof(buf_size) ) < 0)
		prgmerr(1, "sockbuf size %d", buf_size);
#ifdef	_DEBUG_
	n = sizeof(test);
	n = getsockopt(s, SOL_SOCKET,SO_SNDBUF, &test, &n);
	message("send size from %d to %d [%d]\n", o_test, test, n);
#endif
}
