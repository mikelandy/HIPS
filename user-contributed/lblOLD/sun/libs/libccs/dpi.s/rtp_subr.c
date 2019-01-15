/*	RTP_SUBR . C
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

#define	KERNEL
#include "net_need.h"

#ifndef	RTP_MARGIN
#define	RTP_MARGIN	16
#endif

#define	RTP_OPT_LEN	((sizeof(rtcpsdeschdr_t) + sizeof(rtcp_movie_t)) >> 2)
#define	RTCP_MOVIEHDR_LEN	(sizeof(rtp_hdr_t) >> 2)
#define	RTCP_MOVIEHDR_POS	(sizeof(rtp_hdr_t) + sizeof(rtcpsdeschdr_t))
#define	TOTAL_HEADER_BYTES	TOTAL_RTP_MOVIE_HEADER_BYTES

#define	MIN_FIN_TAILS	2
#ifndef	FIN_TAILS
#define	FIN_TAILS	MIN_FIN_TAILS
#endif

#ifndef	Max_FRAGMENTS
#define	Max_FRAGMENTS	144
#endif

#ifndef	MaxRTPchannel
#define	MaxRTPchannel	64
#endif

#ifdef	RTP_READ_ACCURACY
#define	DEFAULT_TOLERANCE	85
#else
#define	DEFAULT_TOLERANCE	106
#endif

#ifndef	BETTER_RTP_PACKET_DALEY
#define	BETTER_RTP_PACKET_DALEY	410
#endif
#define	RTP_DELAY_INC	4

#ifndef	NUM_IP_BUFFERS
#define	NUM_IP_BUFFERS	4
#endif

static	rtp_RxTx_ctrl	*rtp_resource[MaxRTPchannel];
static	MType	mislist[256];	/* shared resource, so get it right way	*/

extern	struct sockaddr_in *get_soaddr();


/* called at open RTP channel	*/
rtp_request_port(int flow_id, int port, int flags)
{
register int	i=flow_id;
    if (i < 0 || i >= MaxRTPchannel)	return	-1;
    {
    int	use_ip = flags & RTP_OVERIP ? IP_HDR_MINLEN : 0;
    int	n=sizeof(n), packet_size;
    register rtp_RxTx_ctrl	*rrp = rtp_resource[i] = (rtp_RxTx_ctrl*)
	ZALLOC(((sizeof(rtp_io_ctrl) << ((flags & DUPLEX_CHANNEL) >> 1)) +
		sizeof(rtp_RxTx_ctrl)), 1, "flow_ctrl");
    register rtp_io_ctrl	*riop = (rtp_io_ctrl*) (rrp + 1);

    if (flags & DUPLEX_RTP_TRANS)	{	/* transmittor	*/
    rtp_hdr_t	*rtphdrp;
    rtcp_movie_t	*rtcp_mhp;
    rtcpsdeschdr_t	*rtcp_sdp;
	rrp->io[1] = riop;
	rrp->flag = flags;
	riop->len = 0;
	riop->buf_left = NUM_IP_BUFFERS;
/*	riop->port = port;	*/

	if ((i=getsockopt(i, SOL_SOCKET, SO_SNDBUF, &packet_size, &n)) < 0)
		return	i;
	if (!(riop->buf=NZALLOC(packet_size, 1, "rtp")))	return	-1;

	riop->pts = packet_size - use_ip - TOTAL_HEADER_BYTES - RTP_MARGIN;
	get_soaddr(1, &riop->soaddr);
	if (use_ip)	init_ip_header(riop->buf, flow_id, 5);
	rtphdrp = riop->rtphdrp= (rtp_hdr_t *) (riop->buf + use_ip);
	rtcp_sdp = (rtcpsdeschdr_t*)(rtphdrp + 1);
	rtcp_mhp = (rtcp_movie_t *)(rtcp_sdp + 1);
	riop++;

	rtphdrp->rh_ver = RTP_VERSION;
	rtphdrp->rh_channel = flow_id;
	rtphdrp->rh_op = True;
	rtphdrp->rh_format = RTPCONT_MOVIE;
	rtcp_sdp->rtsh_fin = 1;
	rtcp_sdp->rtsh_type = RTPOPT_SDES;
	rtcp_sdp->rtsh_optlen = RTP_OPT_LEN;
	rtcp_sdp->rtsh_class = 0;
	rtcp_sdp->rtsh_msglen = RTCP_MOVIEHDR_LEN;
	rtcp_mhp->group_id = htons(kludgeGID);
	rtcp_mhp->ttl = 25;	/* 25 ms	*/
	rtcp_mhp->linewidth = 0;
	rtcp_mhp->size = htons((short)packet_size);
    }
    if (flags & DUPLEX_RTP_TRANS ^ TRANSMITTER_CHANNEL)	{
	rrp->io[0] = riop;
	rrp->flag = flags;
	rrp->list = mislist;
	riop->len = 0;
	riop->port = port;
	riop->group_id = kludgeGID;

	if ((i=getsockopt(i, SOL_SOCKET, SO_RCVBUF, &packet_size, &n)) < 0)
		return	i;
	if (!(riop->buf=NZALLOC(packet_size,
			(flags&RTP_LOCALBUF ? Max_FRAGMENTS : 1), "rtp")))
		return	-1;
	get_soaddr(0, &riop->soaddr);
	riop->pts = packet_size;
	riop->rtphdrp = (rtp_hdr_t *) (riop->buf + use_ip);
    }

    i = flow_id;
    if (flags & RTP_AUTOCONNECT)	{
#ifdef	LWP
	thread_t	t_cnct;
	lwpcreate(&t_cnct, rtp_connect, CNCT_PRIO, No, lwp_newstk(), 1, i);
#elif	THREAD
	thread_t	t_cnct;
	thr_create(NULL, No, rtp_connect, &i, No, &t_cnct);
#else
	switch (fork())	{
	case	-1:
	case	0:	/* parent	*/
	default:	/* child	*/
		rtp_connect(i);	exit	(0);
	}
#endif
    }
    return	i;	/* should be port # ?	*/
    }
}

void	/*	used for close RTP descriptot	*/
rtp_release_port(register int	fd)
{
register rtp_RxTx_ctrl	*rrp = rtp_resource[fd];
if (fd < 0 || fd >= MaxRTPchannel)
	prgmerr(0, "RTP port %d is out of range", fd);
else	{
    if (rrp->io[0])
	free(rrp->io[0]->buf);
    if (rrp->io[1])
	free(rrp->io[1]->buf);
    free(rrp);	rrp = NULL;
}
}


rtp_write_vec(int s, VType* data, int datalen)
{
if (s < 0 || s >= MaxRTPchannel)	return	-1;
{
fd_set	ready;
struct	timeval	poll;
rtcp_flow_ctrl_t	flc;
rtp_RxTx_ctrl*	rrp = rtp_resource[s];
rtp_io_ctrl*	riop = rrp->io[1];
char	*vp = riop->buf;
rtp_hdr_t	*rtphdrp=riop->rtphdrp;
rtcpsdeschdr_t	*rtcp_sdp=(rtcpsdeschdr_t *)(rtphdrp+1);
rtcp_movie_t	*rtcp_mhp=(rtcp_movie_t *)(rtcp_sdp+1);
int	i=sizeof(i), n, rdelay=0, zero=0,
	hdrlen = (char*)(rtcp_mhp + 1) - vp,
	linewidth=rtcp_mhp->linewidth, packet_size=riop->pts;
bool	use_ip = rrp->flag & RTP_OVERIP;

	rtcp_mhp->total_len = htonl(datalen);
	rtcp_mhp->fragment = htonl((datalen + packet_size - 1) / packet_size);
#ifndef	OLD_RTP
	riop->rtpmsg.msg_name = (caddr_t)&riop->soaddr;
	riop->rtpmsg.msg_namelen = sizeof(riop->soaddr);
	riop->rtpmsg.msg_iov = riop->rtpiov;
	riop->rtpmsg.msg_iovlen = 2;	/* rtphdr & data	*/
	riop->rtpiov[0].iov_base = (caddr_t)vp;
	riop->rtpiov[0].iov_len = hdrlen;
#endif
	time((time_t*)&rtphdrp->rh_ts);
#ifdef	LITTLE_ENDIAN
	rtphdrp->rh_ts = htonl(rtphdrp->rh_ts);
#endif

#ifdef	RTP_CAN_USETCP
	if (rrp->flag & RTP_OVERTCP)	{
		if (write(s, vp, hdrlen) != hdrlen)	return	-1;
		return	write(s, data, datalen);
	}
#endif
	if (use_ip)	set_ip_dsize(vp, hdrlen + packet_size, 0);
	poll.tv_sec = poll.tv_usec = 0;	/* rtcp_flow_ctrl polling time */

	for (rtcp_mhp->fin_p=i=0; datalen > 0; i++)	{
		rtcp_mhp->pkt_id = i;
		n = packet_size * i;
		if (linewidth)	{
		register int	y = n / linewidth;
			rtcp_mhp->offset.cord.y = htons(y);
			rtcp_mhp->offset.cord.x = htons(n - y * linewidth);
		} else	rtcp_mhp->offset.pos = htonl(n);
#ifdef	_DEBUG_
	message("GID %d : offset.pos = %d\n",
		rtcp_mhp->group_id, rtcp_mhp->offset.pos);
#endif
		if (datalen < packet_size)	{
			packet_size = datalen;
			rtphdrp->rh_sync = rtcp_mhp->fin_p = 1;
			if (use_ip)
				set_ip_dsize(vp, hdrlen + packet_size, 0);
		}
		datalen -= packet_size;
#ifdef	OLD_RTP
		memcpy(vp + hdrlen, (byte*)data + n, packet_size);
rtp_retry:	if (sendto(s, vp, packet_size, 0, &riop->soaddr, sizeof(riop->soaddr)) < 0)
#else
		riop->rtpiov[1].iov_base = (caddr_t)data + n;
		riop->rtpiov[1].iov_len = packet_size;
rtp_retry:	if (sendmsg(s, &riop->rtpmsg, 0) < 0)
#endif
		    if (errno == ENOBUFS)	{
			udelay(0, -1);	goto	rtp_retry;
		    } else	return	prgmerr(0, "rtp gwrite");
		if (rrp->flag & RTP_FLOWCTRL)
		{
		riop->buf_left--;
		if_input_ready(s, ready, poll)	{
		    if (recvfrom(s, &flc, sizeof(flc) + 8, 0, NULL, &zero) ==
			sizeof(flc))	{
			if (flc.command == -1)
			riop->buf_left += flc.bufs;
#ifdef	WATCH_RTP_FLOWCTRL
			printf("in ctrl %d (%d) <buf = %d>\n",
				i, flc.data.si.s, riop->buf_left),
#endif
			rdelay = 0;
		    }
		} else if (rdelay)
			rdelay <<= RTP_DELAY_INC;
		else if (riop->buf_left <= 1)
			rdelay = BETTER_RTP_PACKET_DALEY;
		}

		if (datalen && (n=(rrp->pdu+rdelay)) > 0)	{
#ifdef	WATCH_RTP_FLOWCTRL
		printf("delay %d (%d) <buf=%d>\n", n, i, riop->buf_left);
#endif
			n >>= 6;	/* select costs 48-64 us. */
			while (n--)	{
				if_input_ready(s, ready, poll)	{
#ifdef	WATCH_RTP_FLOWCTRL
					printf("in delay %d (%d) <buf = %d>\n",
						n, i, riop->buf_left);
#endif
					break;
				}
			}
		}
		time((time_t*)&rtphdrp->rh_ts);
	}
	rtcp_mhp->command &= htons(~RTP_RESET_GID);
	if (use_ip)	set_ip_dsize(vp, hdrlen, 0);
	for (n=hdrlen, i=FIN_TAILS; i--;)
		sendto(s, vp, n, 0, &riop->soaddr, sizeof(riop->soaddr));
#ifdef	LITTLE_ENDIAN
	rtcp_mhp->group_id = htons(ntohs(rtcp_mhp->group_id) + 1);
#else
	rtcp_mhp->group_id++;
#endif
return	ntohl(rtcp_mhp->total_len);
}
}

rtp_read_vec(int	s, char	*data, int tolerance)
{
static char*	lastbuf;
struct	timeval	tin, tflc;
rtcp_flow_ctrl_t	flc;
rtp_RxTx_ctrl*	rrp = rtp_resource[s];
rtp_io_ctrl*	riop = rrp->io[0];
rtp_hdr_t	*rtphdrp = riop->rtphdrp;
bool	video = lastbuf==data && rrp->flag & RTP_SLOWVIDEO;
int	l, flcped=0, sw_lpp=0,	/*	saitch last packet position	*/
	packet_size = riop->pts, nrcv=rrp->miss=0,
	slen=sizeof(riop->soaddr),
	gid, total_recv=0,
	use_ip=(char*)rtphdrp - riop->buf,
	hdrlen=TOTAL_HEADER_BYTES + use_ip;
register int	n = rrp->flag & RTP_LOCALBUF;
char	*buf,	/* working area	and data pointer */
	*dp = buf = n | video ? riop->buf + hdrlen : data,
	*localbuf = n ? dp : 0;	/* data assembling area	*/

#ifdef	RTP_CAN_USETCP
	if (rrp->flag & RTP_OVERTCP)	{
		if (read(s=rrp->tcp, riop->buf, hdrlen) < hdrlen)
			return	-1;
		rtcp_sdp = 0;
		rtcp_getoptions(rtphdrp, 1, RTPOPT_SDES, &rtcp_sdp);
		if (!rtcp_sdp)	return	-1;
		rmopt = (rtcp_movie_t *)(rtcp_sdp + 1);
		return	read(s, data, ntohl(rmopt->total_len));
	}
#endif
	gettimeofday(&tin, 0);
	/* require at least one buffer */
	if (!((int)localbuf | (int)data)) {
		errno = ENOBUFS;	return	-1;
	}

#ifndef	OLD_RTP
	riop->rtpmsg.msg_name = (caddr_t)&riop->soaddr;
	riop->rtpmsg.msg_namelen = sizeof(riop->soaddr);
	riop->rtpmsg.msg_iov = riop->rtpiov;
	riop->rtpmsg.msg_iovlen = 2;	/* rtphdr & data	*/
	riop->rtpiov[0].iov_base = (caddr_t)rtphdrp;
	riop->rtpiov[0].iov_len = hdrlen;
#endif
	if (tolerance < 25 || tolerance > 100)	/* ? rrp->ignore_err */
		tolerance = DEFAULT_TOLERANCE;
#ifndef	RTP_READ_ACCURACY
	else	tolerance += tolerance >> 2;	/* 1.25 / 1.28	*/
#endif
	if (l=riop->len)	{
#	define	dsp	rrp->header_len
		if (!video) /* move out small saving area */
#ifdef	SLOW_DEBUG
			message("move out %d from %x to %x\n",
				l, riop->ngd, dp),
#endif
			memcpy(dp, riop->ngd, l);
		l += dsp;
		goto	addata;
	}

    Loop	{
	rtcpsdeschdr_t	*rtcp_sdp;
	rtcp_movie_t	*rmopt;

#if defined OLD_RTP | defined MAYBE_FAST
	l = recvfrom(s, rtphdrp, packet_size, 0, &riop->soaddr, &slen);
#else
	riop->rtpiov[1].iov_base = (caddr_t)dp;
	riop->rtpiov[1].iov_len = packet_size;
	l = recvmsg(s, &riop->rtpmsg, 0);
#endif
	if (l < 1)	return	l;

addata:	rtcp_sdp = 0;
	dsp = rtcp_getoptions(rtphdrp, 1, RTPOPT_SDES, &rtcp_sdp);
	if (!rtcp_sdp)	{
		message("strange RTP movie packet");
		continue;
	}
	rmopt = (rtcp_movie_t *)(rtcp_sdp + 1);
#ifdef	LITTLE_ENDIAN
	NTOHS(rmopt->group_id);
	NTOHS(rmopt->size);
	NTOHS(rmopt->linewidth);
	NTOHS(rmopt->command);
	NTOHL(rmopt->total_len);
	/* field pos needs to be converted in cases	*/
#endif
	gid = n = rmopt->group_id;
#ifdef	_DEBUG_
	message("GID %d : rev = %d, dsp = %d\n", n, l, dsp);
#endif
	if (n && n < riop->group_id)	/* n=0 is next gid loop	*/
		continue;
	if (!(l -= dsp))	goto	rend;

	/* if a new group coming in ?	*/
	if (n > riop->group_id || !n & riop->group_id)	{
		riop->group_id = n;	/* == rmopt->group_id;	*/
		riop->len = l;
		riop->ngd = dp;	/* save data pointer	*/

exam:		n = total_recv -
#ifdef	RTP_READ_ACCURACY
				rmopt->total_len * tolerance / 100;
#else
				(rmopt->total_len * tolerance >> 7);
#endif
		if (n >= 0 || data == lastbuf) {
			lastbuf = data;
			if (localbuf && data)	/* require extra copy	*/{
#ifdef	SLOW_DEBUG
				message("copy from localbuf	");
#endif
				memcpy(data, localbuf, rmopt->total_len);
			}
			return	total_recv;
		}
#ifdef	NO_ERROR_RERURN
		nrcv = 0;	/* discard old group	*/
#else
		return	n;
#endif
	}
	if (n=rmopt->linewidth)
		n = ntohs(rmopt->offset.cord.y) * n + ntohs(rmopt->offset.cord.x);
	else	n = ntohl(rmopt->offset.pos);
	total_recv += l;

	if (!video)	{
	    if (!rmopt->fin_p)	{
		if (tolerance < DEFAULT_TOLERANCE && n != dp - buf)
#ifdef	SLOW_DEBUG
			message("move lost packet %d from %x to %x+%d\n",
				l, dp, buf, n),
#endif
			memcpy(buf + n, dp, l);	/* lossing packet, so	*/
	/* we are in the loosing area, and to be moved to the right place */
		/* else	discard frame anyway	*/

		if (n+(l<<1) > rmopt->total_len && !localbuf &&
			rrp->miss && rrp->list[rrp->miss-1] + 4 > gid)
			sw_lpp = l,	/* swap next packet to small local area for */
			dp = riop->buf + hdrlen;	/* either next GRP or out of order */
		else if (n >= dp - buf && nrcv <= rmopt->pkt_id)	{
			dp = buf + n + l;	/* point to next block	*/
#ifdef	_DEBUG_
			if (!localbuf && dp + l > buf + pointer_buffer_size(buf))
				message("warning dp - buf = %d\n", dp - buf);
#endif
		}	/* otherwise, don't move the data pointer	*/
	    }	else if (sw_lpp)	/* sw_lpp used for a flag now	*/
		memcpy(buf + n, dp, l);
	} else	memcpy(data + n, dp, l);	/* slow video copy	*/

#ifdef	_DEBUG_
	message("receive %d : total %d (%d) (pos %d) < %d >\tpktid %d %d\n",
		l, total_recv, rmopt->fin_p, n, dp - buf,
		rmopt->pkt_id, nrcv);
#endif

	{	register int	w = rmopt->pkt_id - nrcv;
	/* ignore case w < 0 which indicates retransmission or
		packet is out of order	*/
	if (w > 0) {	/* missing packet, and packet arrives in order	*/
#ifdef	RTP_INTERPOLATION
		if (rrp->flag & RTP_NOINPOLA)
		    while (w--)	rrp->list[rrp->miss++] = nrcv++;
		else if (localbuf)	{	/* not true anymore	*/
		    while (w--)	nrcv++,
			memcpy(data + n - w*l, localbuf + n - w*l, l);
		} else if (data != lastbuf)	{
		    while (w--)	nrcv++,
			memcpy(data + n - w*l, lastbuf + n - w*l, l);
		}
		/* otherwise, treat it as video which uses same buf	*/
#else
		while (w--)	rrp->list[rrp->miss++] = nrcv++;
#endif
		nrcv++;
	}
	if (!w)	nrcv++;
	}

#ifdef	DO_IT
	if (n=rmopt->size > packet_size) {	/* should be in connection */
			/* and should not happen at last packet !!!	*/
		packet_size = riop->pts = n;
		if (!localbuf)	{
#ifdef	SLOW_DEBUG
		message("verify_buffer_size\n");
#endif
			verify_buffer_size(&riop->buf, n, 1, No);
			if (!riop->buf)
				return	-1;
			rtphdrp = (rtp_hdr_t*)(riop->buf + use_ip);
			if (video)	buf = riop->buf + hdrlen;
		}
		if ((riop=rrp->io[1]) && n < riop->pts)
			riop->pts = n;
		riop = rrp->io[0];
	}	/* end initializing buffer agreement	*/

	n = rmopt->total_len + hdrlen;	/* little overhead	*/
	if (localbuf && n > pointer_buffer_size(riop->buf))	{
#ifdef	SLOW_DEBUG
		message("realloc\n");
#endif
		if (!(riop->buf = realloc(riop->buf, n)))
			return	-1;
		n = dp - buf;	/* save data pointer	*/
		dp = (buf = localbuf = riop->buf + hdrlen) + n;
	}	/* end resizing local buffer	*/
#endif

	if ((rrp->flag & RTP_FLOWCTRL) && !(rmopt->pkt_id & 1))	{
#ifdef	RTP_FLOWCTRL_SEND_TIME
	gettimeofday(&tflc, 0);
	tvsub(&tin, &tflc);
	flc.data.l = tflc.tv_usec;
#else
	flc.pkt_id = rmopt->pkt_id;	/* send packet ID	*/
	flc.bufs = 2;	/* drain out 2 more buf	*/
#endif
	if (!flcped)	{
		memcpy(&flc, rtphdrp, sizeof(flc));
		flc.bufs = ++flcped;
		flc.command = -1;
	}
	sendto(s, &flc, sizeof(flc), 0, &riop->soaddr, sizeof(riop->soaddr));
	}
rend:	if (rmopt->fin_p)	{
		riop->group_id = ++gid;
		riop->len = 0;
		goto	exam;
	}
    }
}


#ifndef	MIN_RCVBUF
#define	MIN_RCVBUF	1024
#endif
#define	EQUAL_NOBUF	512
#define	DEFAULT_RTP_PORT	"1920"

rtp_open(char *host, char *port, int carrier, int mode, int buf_size)
{
int	s, recv = !(mode & TRANSMITTER_CHANNEL);
	mode |= RTP_LOCALBUF;

if (!host && !recv)
	return	prgmerr(0, "no host name for transmission");

if (carrier != SOCK_RAW && carrier != SOCK_STREAM)
	carrier = SOCK_DGRAM;	/* deafult carrier	*/
if (!port)	port = DEFAULT_RTP_PORT;

if (buf_size < MIN_RCVBUF && recv)	{
	if (buf_size < EQUAL_NOBUF)	mode &= ~RTP_LOCALBUF;
	if (carrier != SOCK_RAW)	buf_size = Max_WINDOW_SIZE;
}
if (buf_size < MIN_RCVBUF + TOTAL_HEADER_BYTES ||
	buf_size > UDP_BUF_LIMIT && carrier != SOCK_STREAM)
	buf_size = FRAGMENT_SIZE << recv;

s = build_socket(host, port, carrier, mode, &buf_size, No, NULL);

if (carrier == SOCK_STREAM)	mode = (RTP_OVERTCP | mode) & ~RTP_LOCALBUF;
else if (carrier == SOCK_RAW)	mode |= RTP_OVERIP;
return	rtp_request_port(s, get_soaddr(1, NULL)->sin_port, mode);
}

void
rtp_close(int	fd)
{
rtp_release_port(fd);
close(fd);
}

rtp_setoption(int s, int option, register int op1, int op2)
{
if (s < 0 || s >= MaxRTPchannel)	return	-1;
{
rtp_RxTx_ctrl*	rrp = rtp_resource[s];
rtp_io_ctrl*	riop = rrp->io[1];	/* transmittor	*/

switch (option)	{
case RTP_OPT_UDELAY:
	if (op1 > 0)	/* no negs.	*/
	if (op2)	{
		if (op2 < op1)	break;
		op2 -= op2 - op1 >> 3;	/* comp	*/
		if (op2 < op1)	break;
	/*	rrp->pdu = bits / op1 - bits / op2; /* who is fast?	*/
		rrp->pdu = (op2 - op1) * (riop->pts << 3) / (op1 * op2);
	} else	rrp->pdu = op1;
	break;
case RTP_OPT_HDRLEN: /* user data header need to be repeated in each packet */
	rrp->header_len = op1;	break;
case RTP_OPT_GROUPID:
	if (!op2)	{	/* receiver	*/
		rrp->io[0]->group_id = op1;
	} else	{
	register rtcp_movie_t*	rmopt = (rtcp_movie_t*)
			((char*)riop->rtphdrp + RTCP_MOVIEHDR_POS);
		rmopt->group_id = op1;
	}
	break;
case RTP_OPT_FLAGS:
	rrp->flag = op1;	break;
case RTP_OPT_ADDFLAG:
	rrp->flag |= op1;	break;
case RTP_OPT_QOS:	{
rtcp_movie_t*	opthdt = (rtcp_movie_t*)(riop->buf + sizeof(rtp_hdr_t) +
			sizeof(rtcpsdeschdr_t));
	opthdt->fin_p = False;	break;	/* not right now !!!	*/
}
}

}
return	0;
}

rtp_RxTx_ctrl	*
get_rtp_control(int fd)
{
	if (fd < 0 || fd >= MaxRTPchannel)	return	NULL;
	return	rtp_resource[fd];
}

rtp_receive_cmd(int s, rtcp_movie_t *rmp)
{
if (s < 0 || s >= MaxRTPchannel)    return  0;
{
register rtcp_movie_t*	romp = (rtcp_movie_t*)( (char*)
	(rtp_resource[s]->io[0]->rtphdrp + 1) + sizeof(rtcpsdeschdr_t) );
	if (rmp)	*rmp = *romp;
	else	rmp = romp;
return	rmp->command;
}
}
