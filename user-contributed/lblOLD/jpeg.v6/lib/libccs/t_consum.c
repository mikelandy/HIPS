/*	Time_CONSUM . C
#
%	For Performance Analysis
% Usage:
%	consum_time(&ts, &te, 0);	(* initializing	*)
%	...
%	consum_time(&ts, &te, "seg name 1");
%	...
%	consum_time(&ts, &te, "seg name 2");
%
% Author:	Jin Guojun, LBL	- 1/1/94
*/

#define	USE_SYS_TIME_H
#include "stdef.h"

extern	float	tvdiff();
/*	for macros	*/
struct	timeval	global_start_time, loc_start_time, end_time;

float
consum_time(ts, te, m)
register struct timeval	*ts, *te;
char	*m;
{
register float	dv;
	gettimeofday(te, 0);
	if (m)	message("%s %f sec.\t", m, dv=tvdiff(te, ts)),	fflush(stderr);
	gettimeofday(ts, 0);
return	dv;
}
