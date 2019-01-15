/* tvmath.c -- functions for calculating time value by structures
%
%	struct	timeval	{
%		long	tv_sec;		/ seconds /
%		long	tv_usec;	/ and microseconds /
%	};
% Usage:
%	tvadd(tv1, tv2)
%		tv1 = tv1 + tv2
%
%	tvsub(tv1, tv2)
%		tv1 = tv1 - tv2
%
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
% Author:	Jin Guojun - LBL
*/

#include <sys/time.h>
#include <stdef.h>

#define	argus	register struct timeval *tv1, register struct timeval *tv2

void
tvadd(register struct timeval *tv1, register struct timeval *tv2)
{
	if( (tv1->tv_usec += tv2->tv_usec) > 1000000 )   {
		tv1->tv_sec++;
		tv1->tv_usec -= 1000000;
	}
	tv1->tv_sec += tv2->tv_sec;
}


void
tvsub(register struct timeval *tv1, register struct timeval *tv2)
{
	if( (tv1->tv_usec -= tv2->tv_usec) < 0 )   {
		tv1->tv_sec--;
		tv1->tv_usec += 1000000;
	}
	tv1->tv_sec -= tv2->tv_sec;
}

float
tvdiff(register struct timeval *tv1, register struct timeval *tv2)
{
register time_t	usec=tv1->tv_usec, sec=tv1->tv_sec;
	if( (usec -= tv2->tv_usec) < 0 )   {
		sec--;
		usec += 1000000;
	}
	sec -= tv2->tv_sec;
return	sec + usec * 0.000001;
}

longword
timetoms(register struct timeval* t)
{
return	t->tv_sec * 1000 + t->tv_usec / 1000;
}

/*	avg min delay is 2 ms. on Sparc10/40 when 0 < us < 1000000.
	if us <= 0 or us > 1000000, no delay at all,
	but we add 50 us to it on Sparc10/40, or 100 us on Sparc2.
*/
#define MIN_DELAY	2000
#ifndef	TS_COST
#define	TS_COST	35.
#endif

float
udelay(s, us)
{
struct timeval	tv, tt;
	gettimeofday(&tt, 0);
#ifdef vax11c
	{
	int status, delta_time[2];

	delta_time[0] = -10 * us;
	delta_time[1] = -s;
	status = sys$setimr(0, delta_time, 0, 0);
/*	if (!(status&1)) sys$exit(status);	*/
	sys$waitfr(0);
	}
#else
	tv.tv_sec = s;
	tv.tv_usec = us - MIN_DELAY;
	select(1, 0, 0, 0, &tv);
#endif
	if (us < 0)	{
		gettimeofday(&tv, 0);
		return	tvdiff(&tv, &tt);
	}
return	TS_COST;
}
