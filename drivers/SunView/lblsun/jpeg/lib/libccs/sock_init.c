/*	socket_init . c
#
%	Copyright (c)	Jin Guojun
%
%	Socket Initialization Server
%
% AUTHOR:	Jin Guojun - LBL	1/1/93
*/

#include "net_need.h"

#ifndef	SERVER
#define	SERVER	"s_master"
#endif
#ifndef	SockWinFrag
#define	SockWinFrag	64
#endif
#ifndef	DEF_PROTO
#define	DEF_PROTO	"ip"
#endif

static	struct	sockaddr_in	sain, sbin;	/* for binding only.	*/
static	int	protocol;
static	struct	protoent* proto;
static	char*	protoname=DEF_PROTO, *proto_name[]={"TCP", "UDP", "RAW"};
int	net_window_scale = 1;

build_socket(char* host, char *serv_o_port, int socktype, bool duplex,
	int* buf_size, struct sockaddr_in* bsin, magic_server*	ms)
{
int	so, /* trans = duplex < 0 ? ~duplex : duplex, /* ~-1 = 0, ~-2 = 1 */
	trans = duplex & TRANSMITTER_CHANNEL,
	udp = socktype==SOCK_DGRAM,
	win_size = Max_WINDOW_SIZE * net_window_scale;

	if (!bsin)	bsin = &sain;
	if (host && trans)
		bsin->sin_family = get_addr(host, &bsin->sin_addr);
	else	bsin->sin_family = AF_INET;
	bsin->sin_port = get_port(serv_o_port ? serv_o_port : SERVER, udp);

	if (!proto && !(proto = getprotobyname(protoname)))
		prgmerr(0, "protocol %s, 0 is used", protoname);
	if (proto)	protocol = proto->p_proto;

    if ((so=socket(bsin->sin_family, socktype, protocol)) >= 0) {
	if (ms)	{
		ms->so = so;
		ms->proto = socktype;
		ms->m_port = bsin->sin_port;
	}
	if (*buf_size > 0)	{
	    if (*buf_size > win_size)
		*buf_size = win_size;
	    if (udp && *buf_size > UDP_BUF_LIMIT)
		*buf_size = UDP_BUF_LIMIT;
	    while (setsockopt(so, SOL_SOCKET, trans ? SO_SNDBUF : SO_RCVBUF,
			buf_size, sizeof(*buf_size) ) < 0)
		if (*buf_size > SockWinFrag)	*buf_size -= SockWinFrag;
		else
	buferr:		return	prgmerr(0, "sockbuf size %d", *buf_size);
	    if (duplex & DUPLEX_CHANNEL &&
		setsockopt(so, SOL_SOCKET, trans ? SO_RCVBUF : SO_SNDBUF,
			buf_size, sizeof(int) ) < 0)	goto	buferr;
	}

	if (trans)			/* bind any way ???	*/
		sbin.sin_port = 0;	/* to get free port choice.	*/
	else	sbin.sin_port = bsin->sin_port;
	if (socktype != SOCK_RAW &&
#ifndef	NEED_TO_BIND
		!trans &&
#endif
			bind(so, &sbin, sizeof(sbin)) < 0)
		close(so),
		so = prgmerr(0, "sock_bind %s %d", proto_name[udp],
					ntohs(sbin.sin_port));
    }
return	so;
}


struct sockaddr_in	*
get_soaddr(int anb, struct sockaddr_in *saddr)
{
struct sockaddr_in *sad;
if (anb)	sad = &sain;
else	sad = &sbin;
if (saddr)	*saddr = *sad;
return	sad;
}

