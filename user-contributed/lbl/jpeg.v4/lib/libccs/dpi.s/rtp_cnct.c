/*	RTP_ConNeCT . C
#
%	Copyright (c)	Jin Guojun
%
%	RTP Socket Connection Routine
%
% AUTHOR:	Jin Guojun - LBL	1/1/93
*/

#define	USE_RTP
#include "net_need.h"

rtp_connect(int sfd)
{
struct sockaddr_in*	sc_addr;
char*	sc_emsg=0;
rtp_RxTx_ctrl*	rrp = get_rtp_control(sfd);
int	ssof=sizeof(*sc_addr), window, i=sizeof(window),
	trans = rrp->flag & TRANSMITTER_CHANNEL;
rtp_io_ctrl*	riop = rrp->io[trans];

sc_addr = &riop->soaddr;

    if (rrp->flag & RTP_OVERTCP)	{
	if (trans) {
#ifdef	NO_AF_TYPE
		sc_addr->sin_family = AF_UNSPEC;
#endif
		if(connect(sfd, sc_addr, ssof) < 0)
			sc_emsg = "rtp connect";
	} else {
		/* should listen for the connections */
		listen(sfd, 5);   /* allow a queue of 5 */
		if((sfd=accept(sfd, sc_addr, &ssof) ) < 0)
			sc_emsg = "rtp accept";
	}
    } else	{
#ifdef	AUTO_CONNECT
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
#endif
    }
if (sc_emsg)	sfd = prgmerr(0, sc_emsg);
return	sfd;
}
