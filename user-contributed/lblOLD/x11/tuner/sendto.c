/*	sendto.c
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
%	Send images with options to an extended X server on a given host
%
% AUTHOR:	Jin Guojun - LBL	1/1/93
*/
char
#ifdef	OLD_SENDTO
Usage[] =
"\nusage: %s host [+udp] [+port #] [+rtp] [-d #] [-w #] [[| <] input] [-... options]",
#endif
	*Progname, *s_name = "s_master";

#include <sys/file.h>
#include "imagedef.h"
#define	USE_RTP
#include "net_need.h"

#ifndef	BufSize
#define BufSize 32768
#endif

int	window=Max_WINDOW_SIZE, total, udp, rtp,
	/* in micro sec. */ Delay, mbps, params;
U_IMAGE	uimg;

#define	hdlen	MAGIC_HEADER_LEN + sizeof(rtpmsg_hd)
#define	CNT	" connect"

arg_fmt_list_string	arg_fmt[] =	{
	{" host name", NULL, 0, 0, 0, },
	{".-", "%+ %%", 0, 2, 1, "add parameter to header"},
	{"+w", "%d", Max_WINDOW_SIZE, 1, 1, "window size"},
	{"+u", "%N %1", SOCK_DGRAM, 2, 0, "udp (default is TCP)"},
	{"+r", "%1 %i", 1<<20, 2, 0, "rtp [data size = %.f]"},
	{"+p", "%s", 0, 1, 0, "server name or port"},
	{"+f", "%N", RTP_FLOWCTRL, 1, 0, "flow control on rtp"},
	{"+d", "%d", 0, 1, 1, "packet delay in micro sec."},
	{"+R", "%d", 64, 1, 0, "RTP Tx rate in mbps."},
	{" [<] input", NULL, 0, 0, 0, "end of usage"},	NULL	};

main(int ac, char* av[])
{
struct	timeval	te, ts;
struct	sockaddr_in	server;
char	*buf=NZALLOC(sizeof(*buf), BufSize, CNT),
	*hostp=buf+hdlen, **hostlist;
int	s=0, n=2, buf_size=BufSize, carrier=SOCK_STREAM, flc, f_size;
rtpmsg_hd*	msg_hd;

in_fp = stdin;
Progname = *av;
strcpy(buf, CNT);

#ifdef	OLD_SENDTO
#define	isarg(s)	!strncmp(av[n], s, 2)
if (ac < n--)
serr:	prgmerr(n, Usage, *av);

	while (++n < ac)
	    if (*av[n] == '+')	{
		if (av[n][1]=='p' && n+1<ac)
			s_name = av[++n];
		else if (isarg("+u"))	udp = True,
		window = UDP_BUF_LIMIT,	buf_size = 28672;
		else if (isarg("+r"))	rtp = True;
		else if (isarg("+d") && n+1<ac)
			Delay = atoi(av[++n]);
		else if (isarg("+w") && n+1<ac)
			window = atoi(av[++n]);
		else	goto	serr;
	    } else if (*av[n] == '-')	{
		strcat(hostp, av[n]);
		strcat(hostp, " ");
		if (n+1<ac && *av[n+1] != '-' && *av[n+1] != '+')
			strcat(hostp, av[++n]),
			strcat(hostp, " ");
		strcpy(buf, "#");	/* arg is set	*/
	    }	else	s = n;	/* file name position	*/

	if (s && !(in_fp=fopen(av[s], "rb")))
		goto	serr;
	n = strlen(hostp);	hostp = av[1];
#else
	if ((s=parse_argus(&hostlist, ac, av, arg_fmt, &params, buf+hdlen,
		&window, &carrier, &udp, &rtp, &f_size, &s_name,
		&flc, &Delay, &mbps)) < 0)	exit(0);
	if (s < 1 || s > 1 && !(in_fp=fopen(hostlist[1], "rb")))	{
		prgmerr(0, s ? s > 1 ? hostlist[1] : "input" : "host");
serr:		parse_usage(arg_fmt);	exit(0);
	}
	hostp = *hostlist;
	if (rtp)
		buf_size = 16384,	buf = realloc(buf, f_size);
	else if (udp)
		buf_size=Max_UDP_MESG,	window = UDP_BUF_LIMIT;
#endif

io_test(fileno(in_fp),
		if (!(ac=iset) && strlen(buf+hdlen) < 2)	goto	serr);

	s = rtp ? rtp_open(hostp, s_name, SOCK_DGRAM, True, buf_size) :
		build_socket(hostp, s_name, carrier,
			True, &window, &server, 0);
	if (s < 0)	syserr("socket");

	if (!(udp | rtp))
	    if (connect(s, &server, sizeof(server)) /* for write, but sendto */
		< 0)	prgmerr(1, buf);
	    else message("%s: connected to %s, port=%d <window %d>\n",
		getenv("HOSTNAME"), hostp, server.sin_port, window);
	else	message("send %s to %s\n", rtp ? "RPT" : "UDP", hostp);

	if (params || !strcmp(buf, "#")) {	/* send parameter header */
#ifndef	OLD_SENDTO
		strcpy(buf, "#");	/* arg is set	*/
		n = strlen(buf+hdlen);
#endif
		msg_hd = (rtpmsg_hd*) &buf[MAGIC_HEADER_LEN];
		((short*)buf)[1] = htons(n);	/* params len	*/
		n += hdlen + 1;	/* for a NULL ending */

		gettimeofday(&ts, 0);
		msg_hd->tss_s = htonl(ts.tv_sec);
		msg_hd->tss_us = htonl(ts.tv_usec);

		if (!rtp && (total = !udp ? write(s, buf, n) :
			sendto(s, buf, n, 0, &server, sizeof(server))) != n)
			syserr("sendto header to");
		else	message("send params {%s} to %s\n", buf+hdlen, hostp);
	}
	if (buf_size > window)
		buf_size = window;
	window = buf_size;
	gettimeofday(&ts, 0);

	if (ac)
	if (rtp)	{
		n = params ? hdlen : 0;
		if (mbps)	rtp_TxRATE(s, mbps);
		else	rtp_setoption(s, RTP_OPT_UDELAY, Delay, 0);
		rtp_setoption(s, RTP_OPT_ADDFLAG, flc);
#ifdef	USE_READ
		total = read(fileno(in_fp), buf + n, f_size);
#else
		total = fread(buf + n, 1, f_size, in_fp);
#endif
		gettimeofday(&ts, 0);	/* reset RTP timer	*/
		total = rtp_write(s, buf, total + n);
	} else do {	/* send data	*/
		buf_size = fread(buf, 1, window, in_fp);
		if ((n = !udp ? write(s, buf, buf_size) :
		    sendto(s, buf, buf_size, 0, &server, sizeof(server))) < 1)
			syserr("sento");
		total += n;
		if (Delay)	{
			ts.tv_sec = 0;	ts.tv_usec = Delay;
			select(1, 0, 0, 0, &ts);
		}
#ifdef	_DEBUG_
		message("\r%d Bytes send to %s", total, hostp);
#endif
	} while (n == window || (rtp && n > 0));
gettimeofday(&te, 0);
{
	register float	td = tvdiff(&te, &ts);
	message("\rTotal %d Bytes send to %s in %f sec. (%f Mbps)\n",
		total, hostp, td, total * 8e-6 / td);
}
exit(0);
}
