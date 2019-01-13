/*	tv_empty . c
#
%	patch for timing codes
%
% Author:	Jin Guojun,	LBL -	01/1/92
*/

#include <sys/types.h>	/* it should be included in sys/time.h	*/
#include <sys/time.h>

#ifdef	NO_timelocal
time_t	timelocal(struct tm*	tm)
{
time_t	t0;
register	time_t	t1, tt;
struct	tm	tm0;
	time(&t0);
	memcpy(&tm0, tm, sizeof(tm0));
	tm = localtime(&t0);
	if ((t1 = tm0.tm_sec - tm->tm_sec) < 0)	tm0.tm_min--;
	if ((tt = tm0.tm_min - tm->tm_min) < 0)	tm0.tm_hour--;
	t1 += tt * 60;
	if ((tt = tm0.tm_hour - tm->tm_hour) < 0)	tm0.tm_mday--;
	t1 += tt * 60 * 24;
	tt = tm0.tm_mday - tm->tm_mday;
	t1 += tt * 24 * 60 *24;
return	t0 + t1;
}
#endif


#ifdef	NO_UALARM
time_t	ualarm(start, interval)	/* in micro-seconds.	*/
{
struct itimerval	itm;
	itm.it_value.tv_sec = start / 1000000;
	itm.it_value.tv_usec = start - itm.it_value.tv_sec * 1000000;

	itm.it_interval.tv_sec = interval / 1000000;
	itm.it_interval.tv_usec = interval - itm.it_interval.tv_sec * 1000000;

	setitimer(ITIMER_REAL, &itm, NULL);
}
#endif


#ifdef	NO_usleep
usleep(us)
{
udelay(0, us);
}
#endif
