/*	IP-HEADER . C
#
%	Copyright (c)	Jin, Guojun
%
%	Set or Init IP header
%
% AUTHOR:	Jin Guojun - LBL	01/01/93
*/

#define	INCLUDE_IP_HDRS
#include "net_need.h"

#ifndef	FRAGMENT_SIZE
#define	FRAGMENT_SIZE	(4328 - (20 + 8 + 40 /* ip + udp + rtp	*/))
#endif

set_ip_dsize(register struct ip	*ip, int datalen, register int fragsize)
{
if (fragsize < 512)	fragsize = FRAGMENT_SIZE;
	ip->ip_len = datalen;	/* total ip hdr + data	*/
	ip->ip_off = fragsize > datalen ? IP_DF : IP_MF;
#ifdef	COMPARE_IPSUM
	ip->ip_sum = 0;
	ip->ip_sum = checksum(ip, IP_HDR_MINLEN);
#endif
}

void
init_ip_header(register struct ip *ip, int id, int ttl, char *protoname)
{
struct	protoent* proto;
	ip->ip_v = IPVERSION;
	ip->ip_hl = BASE_IP_HDR_WSIZE;	/* 5 words	*/
	ip->ip_tos = ip->ip_sum = ip->ip_p = 0;
	ip->ip_len = 0;
	ip->ip_id = id;
	ip->ip_off = IP_DF;
	ip->ip_ttl = ttl;
	if (protoname && (proto = getprotobyname(protoname)))
		ip->ip_p = proto->p_proto;
}
