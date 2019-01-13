/*	RTP . H
#
% Constants and structures based on the 10/22/93 draft of the RTP protocol
% Internet Draft.	This information is still subject to change.
%
%	LBL
*/

#include <stdef.h>
#ifdef	DEF_ENDIAN
#include "endian.h"
#endif
#ifndef	MIN_IM_FLUSHSIZE
#define	MIN_IM_FLUSHSIZE	384	/* 992 maybe better for flush	*/
#endif

#define	ushort	unsigned short

#ifndef _rtp_h
#define _rtp_h

#ifdef	BIG_ENDIAN
# undef	LITTLE_ENDIAN
#endif

#define RTP_VERSION	1
#define	OLD_RTCP_OPTIONS

#ifndef	RECEIVER_CHANNEL
# define RECEIVER_CHANNEL	0
# define TRANSMITTER_CHANNEL	1
# define DUPLEX_CHANNEL	(1<<1)	/* same as DUPLEX_RTP_RECV	*/
#endif
#define	DUPLEX_RTP_RECV		(DUPLEX_CHANNEL | RECEIVER_CHANNEL)
#define DUPLEX_RTP_TRANS	(DUPLEX_CHANNEL | TRANSMITTER_CHANNEL)

	/*	RTP control flags only	*/
#define	RTP_AUTOCONNECT	(1<<2)
#define RTP_LOCALBUF	(1<<3)
#define	RTP_NOINPOLA	(1<<4)
#define	RTP_SLOWVIDEO	(1<<5)	/* almost 30% slower, but may have higher Q */
#define	RTP_FLOWCTRL	(1<<6)
#define	RTP_SPEEDFRAME	(1<<7)	/* used for busy process between trans frames */
#define	RTP_QUICKSTART	RTP_SPEEDFRAME	/* another name	*/
	/* RTP commands & control flags	*/
#define	RTP_OVERIP	(1<<8)
#define	RTP_OVERTCP	(1<<9)	/* !(bit8 | bit9) => default use UDP	*/
#define RTP_RETRANS	(1<<10)
#define RTP_REFILL	(1<<11)	/* 12-15 only for commands */

/*	RTP OPTION FLAGS	*/
#define	RTP_OPT_UDELAY	1
	/* toggle send sub-headers with each RTP packet	*/
#define	RTP_OPT_SUBHEADERS	2	/* True for sending sub-headers once */
#define	RTP_OPT_TTL	3	/* set rtp_ttl; the default is 35 ms	*/
#define	RTP_OPT_QOS	4
#define	RTP_OPT_HDRLEN	5
#define	RTP_OPT_GROUPID	6
#define	RTP_OPT_ADDFLAG	7
#define	RTP_OPT_DELFLAG	8
#define	RTP_OPT_FLAGS	9	/* set rtp option flags	*/
#define	RTP_OPT_DEBUG	10	/* set debug level (1-3)	*/
#define	RTP_OPT_ADDVBUF	11
#define	RTP_OPT_ON_WAN	12

#ifndef	IP_RATE
#define	IP_RATE	50	/* average UDP/IP Tx speed (Mbps)	*/
#endif
#define	rtp_TxRATE(s, mbps)	rtp_setoption(s, RTP_OPT_UDELAY, mbps, IP_RATE)
#define	rtp_ADJ_Tx_4_NODROP(s)	rtp_setoption(s, RTP_OPT_TTL, 0)
#define	rtp_MIN_TxFRAMES(s,fps)	rtp_setoption(s, RTP_OPT_TTL, 1000/fps)

#define	rtp_read(s, buf, tol)	rtp_read_vec(s, buf, tol, NULL)
#define	rtp_write(s, buf, len)	rtp_write_vec(s, buf, len, NULL)

typedef	enum	{
	RTP_CSRC =	0,	/* bridge only	*/
	RTP_SSRC,		/* every one	*/
	RTP_SDST,
	RTP_BOS,
	RTP_ENC =	8,
	RTP_MIC,
	RTP_MICA,
	RTP_MICK,
	RTP_MICS,
			/* start RTCP	*/
	RTP_FMT	=	32,	/* any one between 0-63; similar to RTP_APP */
	RTP_SDES =	34,
	RTP_BYE,
	RTP_QOS,
			/* start format	*/
	RTP_MINFMT =	96,
	RTP_MAXFMT =	126,

	RTP_APP =	127
	} rtp_option_t;

/* Offset from UNIX's epoch to the NTP epoch in seconds (NTP's JAN_1970) */
#define RTP_EPOCH_OFFSET	2208988800

/* Basic RTP header */
typedef	struct	rtphdr {
#ifndef LITTLE_ENDIAN
	short	rh_ver:2;	/* version */
	short	rh_channel:6;	/* channel (flow) id */
	short	rh_op:1;	/* options present */
	short	rh_sync:1;	/* end of synchronization unit */
	short	rh_format:6;	/* content id */
#else
	short	rh_format:6;
	short	rh_sync:1;
	short	rh_op:1;
	short	rh_channel:6;
	short	rh_ver:2;
#endif
	ushort	rh_seq;		/* sequence number */
	longword rh_ts;		/* time stamp (middle of NTP timestamp) */
} rtp_hdr_t;

/* Basic RTP option header */
typedef	struct	rtpopthdr {
#ifndef LITTLE_ENDIAN
	byte	roh_fin:1;	/* final option flag */
	byte	roh_type:7;	/* option type */
#else
	byte	roh_type:7;	/* option type */
	byte	roh_fin:1;	/* final option flag */
#endif
	byte	roh_optlen;	/* option len */
} rtpopthdr_t;

#ifdef	OLD_RTCP_OPTIONS	/* Normal_RTP_options	*/
#define RTPOPT_CSRC	0	/* Content source */
#define RTPOPT_SSRC	1	/* Synchronization source */
#define RTPOPT_BOP	2	/* Beginning of playout unit */
#endif

/* RTP source (CSRC, SSRC) option header */
typedef	struct	rtpsrchdr {
#ifndef LITTLE_ENDIAN
	byte	rsh_fin:1;	/* final option flag */
	byte	rsh_type:7;	/* option type */
#else
	byte	rsh_type:7;	/* option type */
	byte	rsh_fin:1;	/* final option flag */
#endif
	byte	rsh_optlen;	/* option len (== 2) */
	ushort	rsh_uid;	/* unique id within host */
	longword rsh_addr;	/* IP address of host */
} rtpsrchdr_t;

/* RTP BOP option header */
typedef	struct rtpbophdr	{
#ifndef LITTLE_ENDIAN
	byte	rbh_fin:1;	/* final option flag */
	byte	rbh_type:7;	/* option type */
#else
	byte	rbh_type:7;	/* option type */
	byte	rbh_fin:1;	/* final option flag */
#endif
	byte	rbh_optlen;	/* option len (== 1) */
	ushort	rbh_seq;	/* sequence number of BOP */
} rtpbophdr_t;

#ifdef	OLD_RTCP_OPTIONS	/* RTP forward direction options */
#define RTPOPT_CDES	32	/* Content description */
#define RTPOPT_SDES	33	/* Source description */
#define RTPOPT_FDES	34	/* Flow description */
#define RTPOPT_BYE	35	/* Conference exit notification */
#endif

/* RTCP CDESC option header */
typedef	struct	rtcpcdeschdr {
#ifndef LITTLE_ENDIAN
	byte	rtch_fin:1;	/* final option flag */
	byte	rtch_type:7;	/* option type */
#else
	byte	rtch_type:7;	/* option type */
	byte	rtch_fin:1;	/* final option flag */
#endif
	byte	rtch_optlen;	/* option len */
#ifndef LITTLE_ENDIAN
	byte	rtch_x1:2;	/* reserved (must be 0) */
	byte	rtch_content:6;	/* content id */
#else
	byte	rtch_content:6;	/* content id */
	byte	rtch_x1:2;	/* reserved (must be 0) */
#endif
	byte	rtch_x2;	/* reserved (must be 0) */
	ushort	rtch_rport;	/* return port */
	byte	rtch_cqual;	/* clock quality */
	byte	rtch_x3;	/* reserved (must be 0) */
	longword rtch_cdesc;	/* content descriptor */
} rtcpcdeschdr_t;

/* RTCP SDESC option header */
typedef	struct	rtcpsdeschdr {
#ifndef LITTLE_ENDIAN
	byte	rtsh_fin:1;	/* final option flag */
	byte	rtsh_type:7;	/* option type */
#else
	byte	rtsh_type:7;	/* option type */
	byte	rtsh_fin:1;	/* final option flag */
#endif
	byte	rtsh_optlen;	/* total option len	*/
	ushort	rtsh_uid;	/* unique id within host */
	ushort	rtsh_class;		/* option class	*/
	byte	rtsh_msglen;	/* user option length	*/
	byte	data;		/* uses its address	*/
} rtcpsdeschdr_t;

/* RTCP BYE option header */
typedef	struct	rtcpbyehdr {
#ifndef LITTLE_ENDIAN
	byte	rtbh_fin:1;	/* final option flag */
	byte	rtbh_type:7;	/* option type */
#else
	byte	rtbh_type:7;	/* option type */
	byte	rtbh_fin:1;	/* final option flag */
#endif
	byte	rtbh_optlen;	/* option len */
	ushort	rtbh_uid;	/* unique id within host */
	longword rtbh_addr;	/* IP address of host */
} rtcpbyehdr_t;

#ifdef	OLD_RTCP_OPTIONS	/* RTCP reverse direction options */
#define RTPOPT_QOS	64	/* Quality of service */
#define RTPOPT_RAD	65	/* Raw application data */
#endif

/* Basic RTCP reverse packet header */
typedef	struct	rtcprevhdr {
	byte	rtrh_flow;	/* flow id */
	byte	rtrh_x1;	/* reserved (must be 0) */
	byte	rtrh_x2;	/* reserved (must be 0) */
	byte	rtrh_x3;	/* reserved (must be 0) */
} rtcprevhdr_t;

/* RTCP QOS option header */
typedef	struct	rtcpqoshdr {
#ifndef LITTLE_ENDIAN
	byte	rtqh_fin:1;	/* final option flag */
	byte	rtqh_type:7;	/* option type */
#else
	byte	rtqh_type:7;	/* option type */
	byte	rtqh_fin:1;	/* final option flag */
#endif
	byte	rtqh_optlen;	/* option len (== 5) */
	ushort	rtqh_uid;	/* unique id within host */
	longword rtqh_addr;	/* IP address of host */
	ushort	rtqh_precv;	/* packets received */
	ushort	rtqh_seqrange;	/* sequence number range */
	ushort	rtqh_mindel;	/* minimum delay */
	ushort	rtqh_maxdel;	/* maximum delay */
	ushort	rtqh_avgdel;	/* average delay */
	ushort	rtqh_x;		/* reserved (must be 0) */
} rtcpqoshdr_t;

/* LBL MOVIE & NETEST	*/
typedef	struct	rtcp_movie	{
	ushort	group_id,	/* frame ID	*/
		size;		/* optimized packet window size	*/
	byte	fragment;	/* number of fragments in this group (frame) */
#ifndef LITTLE_ENDIAN
	byte	fin_p:1;	/* final packet; assemble now	*/
	byte	ttl:7;		/* time to live; max is 128 ms.	*/
#else
	byte	ttl:7;
	byte	fin_p:1;
#endif
	short	command;	/* RTP_RETRANS ..., see top and below	*/
#	define	RTP_RESET_GID	(1 << 12)	/* force to change group_id */
#	define	RTP_REQUESTBUF	(1 << 13)
#	define	RTP_DROPFRAME	(1 << 14)
#	define	RTP_SYNC	(1 << 15)	/* a special command	*/

	byte	type;	/* protocol. can be used for extended header	*/
	/*	number bytes for more options that follows this header	*/
	byte	pkt_id;		/* fragment (packet) ID	*/
	ushort	linewidth;	/* image (scan line) width	*/
	union	{
		struct	{
			ushort	x, y;	/* used for linewidth presented */
		} cord;
		longword	pos;	/* absolute offset	*/
	} offset;		/* fragment start offset	*/
	longword	total_len;	/* total data in this group (frame)	*/
	ushort	frag_len;	/* for bad IP imp.	*/
	char	fs_sent,	/* more frags can be sent, included this one */
		tp_sent;	/* total packets sent from last sync	*/
} rtcp_movie_t;
#define	TOTAL_RTP_MOVIE_HEADER_BYTES	\
	(sizeof(rtp_hdr_t) + sizeof(rtcpsdeschdr_t) + sizeof(rtcp_movie_t))

typedef	struct	rtcp_flow_ctrl	{
	rtp_hdr_t	mhdr;
	rtcpsdeschdr_t	shdr;
	/*	control header; compatble to _movie_t (OVERLAYING)	*/
	ushort	group_id,	/* frame ID	*/
		size;		/* acceptable receiver buffer size	*/
	byte	fragment,
		bufs;		/* ACK number of buffers consumed	*/
	short	command;
	byte	type, pkt_id;
	short	last_fd_id;	/* for last dropped frame ID	*/
	longword	saved_rdelay	/* was pos	*/, total_len;
	ushort	cont_f_drops;	/* counts continue dropped frames	*/
	char	fs_sent, tp_sent;	/* END of OVERLAYING	*/
	ushort	pad;
	byte	virtual_bufs;	/* take advantage of WAN as bufs	*/
	char	adj_f_c;	/* for adjust_FRQ count	*/
	char	arriving[MIN_IM_FLUSHSIZE];
} rtcp_flow_ctrl_t;

#define	if_input_ready(s, ready, pt)	\
	FD_ZERO(&ready);	FD_SET(s, &ready);	\
	if (select(s +1, &ready, 0, 0, &pt) < 0)	\
		prgmerr(0, "	select %d	", s);	\
	else if (FD_ISSET(s, &ready))

/* RTP standard content encodings for audio */
#define RTPCONT_PCMU		0	/* 8kHz PCM mu-law mono */
#define RTPCONT_1016		1	/* 8kHz CELP (Fed Std 1016) mono */
#define RTPCONT_G721		2	/* 8kHz G.721 ADPCM mono */
#define RTPCONT_GSM		3	/* 8kHz GSM mono */
#define RTPCONT_G723		4	/* 8kHz G.723 ADPCM mono */
#define RTPCONT_DVI		5	/* 8kHz Intel DVI ADPCM mono */
#define RTPCONT_L16_16		6	/* 16kHz 16-bit linear mono */
#define RTPCONT_L16_44_2	7	/* 44.1kHz 16-bit linear stereo */

/* RTP standard content encodings for video */
#define	RTPCONT_CUSEEME		27	/* Cornell CU-SeeMe */
#define RTPCONT_NV		28	/* Xerox PARC nv */
#define RTPCONT_DVC		29	/* BBN dvc */
#define RTPCONT_BOLT		30	/* Bolter */
#define RTPCONT_H261		31	/* CCITT H.261 */

#define	RTPCONT_MOVIE		48	/* LBL MOVIE	*/
#define	RTPCONT_TEST		49	/* RTP TEST MODE	*/

#endif	/* _rtp_h */
