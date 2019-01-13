/*	socket_ConNeCT . c
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
%	Socket Connection Server
%
% AUTHOR:	Jin Guojun - LBL	1/1/93
*/

#include "net_need.h"

socket_connect(	int sfd, struct sockaddr_in*	sc_addr,
		int trans, int prot, int options, int *ov, int ol)
{
char*	sc_emsg=0;
int	ssof=sizeof(*sc_addr), window, i=sizeof(window);

    if(options && setsockopt(sfd, SOL_SOCKET, options, ov, ol) < 0)
	sc_emsg = "setsockopt";
    else switch (prot)	{
	case TCP_SOCK:
	if (trans) {
#ifdef	NO_AF_TYPE
		sc_addr->sin_family = AF_UNSPEC;
#endif
		if(connect(sfd, sc_addr, ssof) < 0)
			sc_emsg = "connect";
	} else {
		/* should listen for the connections */
		listen(sfd, 5);   /* allow a queue of 5 */
		if((sfd=accept(sfd, sc_addr, &ssof) ) < 0)
			sc_emsg = "accept";
	}
	break;

	case UDP_SOCK:
	case IP_SOCK:
	break;

	case RTP_SOCK:
	if ((ol=getsockopt(sfd, SOL_SOCKET, trans ? SO_SNDBUF : SO_RCVBUF,
		&window, &i)) >= 0)	{
	PacketProto	pp;
	    if (trans)	{
		struct sockaddr_in	s_other;
		pp.window = window;
		sendto(sfd, &pp, sizeof(pp), 0, sc_addr, sizeof(*sc_addr));
		recvfrom(sfd, &pp, sizeof(pp), 0, &s_other, &ssof);
		if (pp.window < window)
			window = pp.window;
	    }
	    else	{
		recvfrom(sfd, &pp, sizeof(pp), 0, sc_addr, &ssof);
		if (pp.window > window)
			pp.window = window;
		else	window = pp.window;
		sendto(sfd, &pp, sizeof(pp), 0, sc_addr, sizeof(*sc_addr));
	    }
	    sfd = window;
	} else sfd = ol;
	break;

	default:	sc_emsg = "unknown protocol";
    }
if (sc_emsg)	sfd = prgmerr(0, sc_emsg);
return	sfd;
}
