/*	RTP_group . C
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
% Author:	Jin Guojun - LBL	3/1/93
*/

#include "net_need.h"
#include "rtp.h"

#define	RTP_OPT_LEN	((sizeof(rtcpsdeschdr_t) + sizeof(rtcp_movie_t)) >> 2)
#define	RTCP_MOVIEHDR_LEN	((sizeof(rtp_hdr_t) >> 2) + RTP_OPT_LEN)

#ifndef	MaxRTPChannel
#define	MaxRTPChannel	64
#endif

typedef	struct	{
	char	*buf;	/* buffer saves data for next group	*/
	int	alias,	/* channel ID does not match its index	*/
		len;	/* data in buffer	*/
	char	used;	/* for each communication channel	*/
	char	protocol;
	char	retrans;
	char	align;
	} flow_ctrl;

static	flow_ctrl	rtp_resource[MaxRTPChannel];


/* called either before start receiving or in receiving	*/
rtp_request_channel(flow_id, buf_size, protocol, retrans)
{
register int	i=flow_id;
register flow_ctrl	*rrp = rtp_resource;
	if (!i || i>MaxRTPChannel
		|| rrp[i].used && rrp[i].alias != i) /* need to use others ? */
			/* it will replace old channel setting	*/
	    for (i=0; i<MaxRTPChannel; i++)
		if (!rrp[i].used)	break;
	if (i >= MaxRTPChannel)	return	EOF;	/* no channel available	*/
	verify_buffer_size(&rrp[i].buf, buf_size, 1, "rtp");	/* can be 0 */
	rrp[i].alias = rrp[i].len = 0;
	if (flow_id && flow_id != i)
		rrp[i].alias = flow_id;
	rrp[i].protocol = protocol;
	rrp[i].retrans = retrans;
	rrp[i].used++;
return	i;	/* return flow_id	*/
}

void
rtp_release_channel(register int chan /* it's the flow_id */)
{
register flow_ctrl	*rrp = rtp_resource;
    if (!rrp[chan].alias)	{
	free(rrp[chan].buf);
	rrp[chan].used = False;
    } else {
	register int	i=MaxRTPChannel;
	while (i--) if (rrp[i].alias == chan)	{
		free(rrp[i].buf);
		rrp[i].used = rrp[i].alias = 0;
		break;
	}
	if (i < 0)	prgmerr(0, "RTP channel %d is not released", chan);
    }
}


rtp_sendbygroup(s, data, datalen, group_id, sender_id, hdr, hdr_len, linewidth)
int	s;
VType	*data, *hdr;
{
int	packet_size, i=sizeof(i), n;
rtp_hdr_t	*rtphdrp;
rtcpsdeschdr_t	*rtcp_sdp;
rtcp_movie_t	*rtcp_mhp;
long	*vp;

	if ((n=getsockopt(s, SOL_SOCKET, SO_SNDBUF, &packet_size, &i)) < 0)
		return	n;
	if (!(vp = (long*)malloc(packet_size)))
		return	prgmerr(0, "rtp_sg mem");
	rtphdrp = (rtp_hdr_t *) vp;
	rtcp_sdp = (rtcpsdeschdr_t*)(rtphdrp + 1);
	rtcp_mhp = (rtcp_movie_t *)(rtcp_sdp + 1);

	rtphdrp->rh_ver = 1;
	rtphdrp->rh_channel = sender_id;
	rtphdrp->rh_op = True;
	rtphdrp->rh_format = RTPCONT_MOVIE;
	rtcp_sdp->rtsh_type = RTPOPT_SDES;
	rtcp_sdp->rtsh_optlen = RTCP_MOVIEHDR_LEN;
	rtcp_sdp->rtsh_class = 0;
	rtcp_sdp->rtsh_msglen = RTP_OPT_LEN;
	rtcp_mhp->group_id = group_id;
	rtcp_mhp->ttl = 33;	/* 33 ms	*/
	rtcp_mhp->linewidth = linewidth;
	rtcp_mhp->total_len = datalen;

	time((time_t*)&rtphdrp->rh_ts);

	for (i=0; datalen > 0; i++)	{
		rtcp_mhp->fragment = i;
		n = packet_size * i;
		if (linewidth)	{
		register int	y = n / linewidth;
			rtcp_mhp->offset.cord.y = y;
			rtcp_mhp->offset.cord.x = n - y * linewidth;
		} else	rtcp_mhp->offset.pos = n;
		if (datalen < packet_size)
			packet_size = datalen,
			rtphdrp->rh_sync = 1;
		datalen -= packet_size;
		memcpy(vp + RTCP_MOVIEHDR_LEN, (byte*)data + n, packet_size);
		if (write(s, vp, packet_size) < 0)
			return	prgmerr(0, "rtp gwrite");
	}
}

rtp_recvbygroup(s, data, datalen, group_id, tolerance, ignore_err)
int	s;
char	*data;
int	*datalen, *group_id;
{
int	dsp, packet_size, i=sizeof(i), n, nrcv=0;
char	*buf;

    if ((n=getsockopt(s, SOL_SOCKET, SO_RCVBUF, &packet_size, &i)) < 0)
	return  n;
    if (!(buf=(char *)malloc(packet_size)))
	return	prgmerr(0, "rtp_rg mem");
    Loop	{
rtp_hdr_t	*rtphdrp = (rtp_hdr_t *)buf;
rtcpsdeschdr_t	*rtcp_sdp;
rtcp_movie_t	*rmopt;
	i = read(s, buf, packet_size);
	if (i < 0)	return	i;
	dsp = rtcp_getoptions(buf, 1, RTPOPT_SDES, &rtcp_sdp);
	if (!rtcp_sdp)	{
		message("strange RTP group packet");
		continue;
	}
	rmopt = (rtcp_movie_t*)(rtcp_sdp + 1);
	if (rmopt->group_id < *group_id)
		continue;
	if (rmopt->group_id > *group_id)	{
		*group_id = rmopt->group_id;
		if (nrcv * packet_size * 100 > rmopt->total_len ||
			!ignore_err) {
			free(buf);
			return	nrcv * packet_size;
		}
		nrcv = 0;
	}
	if (n=rmopt->linewidth)
		n = rmopt->offset.cord.y * n + rmopt->offset.cord.x;
	else	n = rmopt->offset.pos;
	memcpy(data + n, buf + dsp, i - dsp);
	nrcv++;
    }
}

