/*	time-out . c
%
%	set time limit for free distribution
*/

#include <time.h>
#include "imagedef.h"

static	struct tm	tl_out;

struct tm *
set_time_limit(y, m, d, h, min)
{

tl_out.tm_sec = 0;
tl_out.tm_min = min;
tl_out.tm_hour = h;
tl_out.tm_mday = d;
tl_out.tm_mon = m-1;
tl_out.tm_year = y;
return	&tl_out;
}

if_time_exp(tm_in)
time_t	tm_in;
{
return	tm_in > timelocal(&tl_out);
}

exit_timeout(omsg, y, m, d, h, min)
char	omsg[];
{
time_t	t0;
	message(omsg, m, d, y);
	set_time_limit(y, m, d, h, min);
	if (if_time_exp(time(&t0)))     exit(0);
}

