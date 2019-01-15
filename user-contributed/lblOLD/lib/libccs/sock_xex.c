/*	sock_xex . c
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
%	Socket X-extended Server
%	If RTP is initialized, and it is not impelmented in kernel method;
%	then, we need a special rtp_read routine to read incoming data.
%
% AUTHOR:	Jin Guojun - LBL	01/01/93
*/

#include <fcntl.h>
#define	KERNEL
#include "net_need.h"

#ifndef	SERVER
#define	SERVER	"s_master"
#endif

#define	TOTAL_HEADER_BYTES	TOTAL_RTP_MOVIE_HEADER_BYTES


static	rtpmsg_hd	msg_hdr;
static	char	buf[1024], **plist;
static	int	pnum, as, so;
static	magic_server	ss[2];	/* 0 for TCP, and 1 for RTP or others	*/
static	struct sockaddr_in	sain;

build_arg_list(char *buf, char **tpp[])
{
register int	i, n, p=n=0;
char*	*lp;
	Loop	{
		n++;
		if (!(i = strchr(buf+p, Space)))	break;
		i -= (int)buf;
		buf[i++] = 0;	/* add terminator	*/
		p = i;
	}	n++;
	verify_buffer_size(tpp, n--, sizeof(tpp), "tpp");
	lp = *tpp;
	lp[p=0] = "socket";
	for (i=p; i++ < n;)	{	/* start from second ap	*/
		lp[i] = buf + p;
		while (buf[p++]);
	}
return	n;
}

get_arg_list(int *rpp)
{
if (rpp)
	*rpp = (int)plist;
return	pnum;
}

x_extender_init(char *server_name, bool rtp, int buf_size, int carrier)
{
	if (!server_name)
		sprintf(server_name=buf, "%s", SERVER);
	sain.sin_addr.s_addr = INADDR_ANY;
retryexi:
	if (rtp)	{
		carrier = carrier ? SOCK_DGRAM : SOCK_RAW;
		so = rtp_open(NULL, server_name, carrier, 0, buf_size);
		rtp_setoption(ss[rtp].so= so, RTP_OPT_ADDFLAG, RTP_FLOWCTRL);
		ss[rtp].m_port = get_soaddr(0, NULL)->sin_port;
	} else	{
		if (buf_size < 4096 || buf_size > Max_WINDOW_SIZE)
			buf_size=Max_WINDOW_SIZE;
		if (carrier != SOCK_DGRAM)	carrier = SOCK_STREAM;
		so = build_socket(NULL, server_name, carrier,
			False, &buf_size, &sain, ss);
	}
	if (so < 0)	{
		if (errno != EADDRINUSE || sain.sin_port == htons(SV_PORT))
			return	prgmerr(0, "%d:bind[%d] -> port = %d",
				carrier, so, ntohs(sain.sin_port));
		sprintf(server_name=buf, "%d", SV_PORT);
		goto	retryexi;
	}
	if (!rtp)	listen(so, 4);	/* TCP	*/
return	htons(sain.sin_port);	/* sin_port is net-byte_order	*/
}

FILE*
x_extender(bool rtp)
{
fd_set	ready;
int	l;
short	psize;
register int	i;

struct timeval	to;
	FD_ZERO(&ready);
	FD_SET(so = ss[rtp].so, &ready);
	to.tv_sec = 0;	to.tv_usec = 100000;
	if ((i=select(so+1, &ready, 0, 0, &to)) < 0)
		prgmerr(i, "	select %d(%d)	", i, so);
	else if (FD_ISSET(so, &ready))	{
	    if (!ss[rtp].connected)	{
		l = sizeof(sain);
		ss[rtp].as = as = ss[rtp].proto != SOCK_STREAM ?
			so : accept(so, &sain, &l);
#ifdef	_DEBUG_
		msg("select returns %d for [%d]{%X}\t", i, so, as);
#endif
		if (as >= 0)	{
#ifdef	F_SETOWN
		    if (fcntl(as, F_SETOWN,  getpid()) < 0)
			syserr("F_SETOWN %d", getpid());
#endif
#ifdef	FASYNC
		    if (fcntl(as, F_SETFL, FASYNC)<0)
			syserr("F_SETEL FASYNC");
#endif
		    ss[rtp].connected++;
		    ss[rtp].stat = ss[rtp].dp = pnum = 0;

		    if (rtp)	{
			i = rtp_read(as, NULL, 78);
			printf("RTP return %d\n", i);
			if (i < 0)	goto	disc;
			{
			rtp_RxTx_ctrl*	rrp = get_rtp_control(as);
			rtcp_movie_t*	rmopt;
			register BUFFER	*bp;
			if (!rrp)	goto	disc;
			rrp->io[0]->group_id--;	/* keep GID constant	*/
			if (!i || rrp->miss && !rrp->list[0]) /* miss hdr */
				goto	disc;
			if (!(bp=buffer_create(0, TOTAL_HEADER_BYTES)))
				return	0;
			ss[1].fp = (FILE*)bp;
			bp->ptr = (bp->base = rrp->io[0]->buf) + bp->offset;
/* not needed ?!	bp->bsize = pointer_buffer_size(rrp->io[0]->buf);	*/
			i = rtcp_getoptions(rrp->io[0]->buf, 0);	/* = rtphdrp */
			rmopt = (rtcp_movie_t*)(rrp->io[0]->buf + i) - 1;
			bp->dlen = rmopt->total_len;
			if (buffer_seek(bp, 0, SEEK_PEEK) == '#' &&
				!buffer_seek(bp, 0, SEEK_PEEK))	{
				buffer_seek(bp, 2, SEEK_CUR);
				buffer_read(&psize, 1, sizeof(psize), bp);
				i = htons(psize) + 1;
				buffer_read(&msg_hdr, 1, sizeof(msg_hdr), bp);
				buffer_read(buf, 1, i, bp);
				goto	common_sub;
			}
			goto	common_out;
			}
		    } else if ((ss[rtp].fp=fdopen(as, "rb")))	{
			if ((i=getc(ss[rtp].fp)) == '#')
			    if (!(l=getc(ss[rtp].fp)))	{
				fread(&psize, 1, sizeof(short), ss[rtp].fp);
				i = htons(psize) + 1;	/* NULL terminator */
				fread(&msg_hdr, 1, sizeof(msg_hdr), ss[rtp].fp);
				fread(buf, 1, i, ss[rtp].fp);

common_sub:			gettimeofday(&ss[rtp].ts, 0);
				msg_hdr.tsr_s = ss[rtp].ts.tv_sec;
				msg_hdr.tsr_us = ss[rtp].ts.tv_usec;
				pnum = build_arg_list(buf, &plist);
#ifdef	_DEBUG_
				msg("params size %d {# %d}\n", i, pnum);
#endif
			    }	else	ungetc(l, ss[rtp].fp),
					ungetc('#', ss[rtp].fp);
			else	ungetc(i, ss[rtp].fp);
common_out:
#ifdef	_DEBUG_
			msg("#1 = %c [%d], #2 = %d\n", i, i, l);
#endif	/* params start with "#" ('#'\0)	*/
			return	i<0 ? NULL : ss[rtp].fp;
		    } else	return	(FILE*)prgmerr(-1, "fdopen");
		}	/* never out from here, buf might ?	*/
	    }	/* already connected	*/
		if (rtp)	goto	disc;
		message("%d: something happened %d\n", ss[rtp].connected, as);
	} else if (ss[rtp].connected)	{	/* fd not set	*/
		FD_ZERO(&ready);
		FD_SET(as, &ready);
		i = select(as+1, &ready, 0, 0, &to);
#ifdef	_DEBUG_
		msg("%d {%X} %d\n", i, FD_ISSET(as, &ready), to.tv_usec);
#endif
		if (i && FD_ISSET(as, &ready))	{
			/* close(as);	/* prepare to reconnect */
disc:			if (!rtp && ss[rtp].proto == SOCK_STREAM)
				fclose(ss[rtp].fp);
			ss[rtp].connected = 0;
/*			if (rtp)
				x_extender_init(0, rtp, 0, ss[rtp].proto); */
#ifdef	_DEBUG_
			fprintf(stderr, "\t%d(%d): disconnect at {%X}\n",
				i, as, FD_ISSET(as, &ready));
#endif
		}
	}
	fflush(stderr);
return	NULL;
}
