/*	NET_NEED . H
#
%	Copyright (c)	Jin, Guojun
%
%	for network users
%
% Author:	Jin Guojun - LBL	01/1/92
*/

#ifndef	_net_need_h
#define	_net_need_h

#ifdef vax11c
#include "multinet_root:[multinet.include]errno.h"
#include "multinet_root:[multinet.include.sys]types.h"
#include "multinet_root:[multinet.include.sys]socket.h"
#include "multinet_root:[multinet.include.netinet]in.h"
#include "multinet_root:[multinet.include]netdb.h"
#include "multinet_root:[multinet.include.sys]time.h"   /* struct timeval */
#ifdef errno
#undef errno
#endif
#define errno socket_errno
#define perror socket_perror
#else
#include <errno.h>
#include <sys/types.h>
#if	defined	KERNEL | defined USE_RTP
# include <sys/uio.h>
#endif
# if	(defined ultrix || defined __osf__) && defined KERNEL
# define	DEC_MESS
# undef	KERNEL
# endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>           /* struct timeval */
#endif

#include <signal.h>
#ifdef	DEC_MESS
# undef	DEC_MESS
# define	KERNEL
#endif

#ifdef	INCLUDE_IP_HDRS
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#endif

#include "stdef.h"

#ifndef	SV_PORT
#define	SV_PORT	1991	/*	second server port for extended X server */
#endif			/*	first one is called SERVER	*/

#define	UNKNOWN_SOCK	-1
#define	TCP_SOCK	0	/* SOCK # are equal to SOCK_protocol - 1 */
#define	UDP_SOCK	1
#define	RTP_SOCK	2
#define	IP_SOCK		3

#define RECEIVER_CHANNEL	0
#define TRANSMITTER_CHANNEL	1
#define DUPLEX_CHANNEL		2

#ifndef	nttype

#define	FIELD_SIZE_BITS	16
#define	htonhdr	htons
#define	ntohhdr	ntohs
#define	nttype	unsigned short
#define	BORROWED_BITS	2
#define	REST_BITS	(FIELD_SIZE_BITS - BORROWED_BITS)

#elif	(nttype == long_32)

#define	FIELD_SIZE_BITS	32
#define	htonhdr	htonl
#define	ntohhdr	ntohl

#endif


typedef	struct	{
	char	ctrl_word,
		bitmap_seq;
	short	TID;
	}	atp_udp_hd;	/* atp_udp header */

typedef struct	{
	int	as, so;		/* accepted socket & original socket fd	*/
	FILE	*fp;		/* fdopen()	*/
	byte	connected,
		proto;		/* SOCK_?????	*/
	short	m_port;
	long	dp, stat,
		total, packet_size;
	char	*bp;
	struct	timeval ts, te;
        }	magic_server;

typedef	struct	{
	nttype	seq_num, size;	/* sequence # and packet size	*/
	short	t_frame, type;	/* time interval, protocol	*/
	longword	tss_s, tss_us,	/* start sending	*/
			tes_s, tes_us,	/* end of sending	*/
			tsr_s, tsr_us,	/* start receiving	*/
			ter_s, ter_us,	/* end of receiving	*/
			rss_s, rss_us,	/* start sending back	*/
			res_s, res_us,	/* end of sending back	*/
			rsr_s, rsr_us,	/* start receiving back	*/
			rer_s, rer_us;	/* end of r.b.		*/
	long	t_err_s, t_err_us;	/* clock diff	*/
	char	name[16];	/* must be aligned to 8-Byte boundary	*/
	union	{
		char	*bufp;
		struct	sl	{
			short	pkt_id, pkt_dir;
		} pkt_info;
	} udp_nif;
	} rtpmsg_hd;	/* round trip message	*/

#define	pkt_num	udp_nif.pkt_info.pkt_id
#define	pkt_dir	udp_nif.pkt_info.pkt_dir
#define	pkt_buf	udp_nif.bufp

typedef	struct	{	/* overlaid on rtpmsg_hd structure	*/
	float	min_s, min_r;
	struct	timeval	tss, tes,
			tsr, ter,
			rss, res,
			rsr, rer;
	union	{
		double	t_err;
		long	t_err_s[2];
		} clock_dif;
	char	name[16];
	float	max_tt;	/* max transmitting time */
	} pt_inf;

typedef	struct	{		/* total must 8 bytes long	*/
	nttype	npackets,	/* maximum packets per group	*/
		window;		/* maximum bytes per packet	*/
	char	clock_tick,	/* clock quality, not used	*/
#if defined LITTLE_ENDIAN || defined (vax) | defined (i386)
		pp_hl:4,	/* in words.	*/
		flow_id:4;	/* EOF and 0 mean same (end)	*/
#else
		flow_id:4,
		pp_hl:4;
#endif
	short	servtype;
	} PacketProto;

#define PROT_CORRECT_TIME	1
#define	MAGIC_HEADER_LEN	4

#if	defined	KERNEL | defined USE_RTP
#include "rtp.h"
#ifndef	RTP_IOVEC_DEPTH
#define	RTP_IOVEC_DEPTH	4
#endif	/* max data sets can be used by rtp_read_vec() or rtp_write_vec() */

typedef	struct	{
	rtp_hdr_t	*rtphdrp;
	char	*buf,	/* rtp kernel buffer	*/
		*ngd;	/* buffer saves data for next group	*/
	short	buf_left; /* number of ip buffers. SGI char is unsigned */
	short	noused_s;
unsigned short	port,	/* communication port	*/
		pts;	/* packet transmitting size	*/
	long_32	len;	/* data in buffer	*/
#define	kludgeGID	1	/* has to be non-zero	*/
unsigned short	group_id,
		command;	/* communication command	*/
	struct sockaddr_in	soaddr;
	struct msghdr	rtpmsg;
	struct iovec	rtpiov[RTP_IOVEC_DEPTH];
	} rtp_io_ctrl;

typedef	struct	{
	rtp_io_ctrl	*io[2];
	short	flag;	/* control falgs (both)	*/
	char	ignore_err, header_len;	/* only for reading	*/
unsigned short	miss,	/* number of missing packets (read)	*/
		pdu;	/* packet delay in us. (write)	*/
	int	*list,	/* missing list (read)	*/
		tcp;	/* accepted socket descriptor (write)	*/
	int	reserved;
	/* flcped can only be reset by sending request when frame changes */
	bool	flcped;	/* fill flc with sender rtp mv header	*/
	rtcp_flow_ctrl_t	flc;
	} rtp_RxTx_ctrl;

rtp_RxTx_ctrl	*	get_rtp_control();

#endif

#include <math.h>

typedef	struct Packet {
	long_32	npackets;
	long_32	nbytes;
} *PBuffer;	/* packet buffer */

#ifdef	DEC
#undef	fabs
#define	fabs(x)	(x<0 ? -x : x)
#endif

#ifndef	Default_Sleep
#define	Default_Sleep	5000	/*	millisec.	*/
#endif

#define	UDP_BD	sizeof(PacketProto)

#ifdef	IPVERSION
#define	IP_HDR_MINLEN	sizeof(struct ip)
#else
#define IP_HDR_MINLEN	(5 << 2)	/* 5 words	*/
#endif
#define	BASE_IP_HDR_WSIZE	(IP_HDR_MINLEN >> 2)

#define UDP_HDR_LENGTH_BYTE	UDP_BD
#define SYS_HDR_LENGTH		(IP_HDR_MINLEN + UDP_BD)
#define HDR_BYTE_LENGTH		SYS_HDR_LENGTH
#define	WINDOW_SIZE		32768	/* 0x8000	*/
#ifndef	Max_WINDOW_SIZE
#define	Max_WINDOW_SIZE		52428	/* 0xCCCC	*/
#endif

#ifndef	FRAGMENT_SIZE
#define	FRAGMENT_SIZE	(4328 - SYS_HDR_LENGTH)
#endif
#ifndef	UDP_BUF_LIMIT	/* WINDOW_SIZE - SYS_HDR_LENGTH	*/
# ifdef	SOLARIS
#define	UDP_BUF_LIMIT	Max_WINDOW_SIZE
# else
#define	UDP_BUF_LIMIT	32739	/* 0x7FE3, MAX for BSD	*/
# endif
#endif
#define	MAX_IP_PACKET	28752	/* 0x7050, MAX for BSD	*/
#define	Max_UDP_MESG	28672	/* 0x7000	*/

struct sockaddr_in* get_soaddr(/*int rot, struct sockaddr_in *saddr*/);
float	tvdiff(/* struct timeval *end, struct timeval *start */);
extern	char	*Progname;

#endif	/* _net_need_h	*/
