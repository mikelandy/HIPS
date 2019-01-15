
#include <sys/types.h>	/* it should be included in sys/time.h	*/
#include <sys/time.h>


#ifdef	NO_Random
random(seed)
{
struct timeval	tr;
	gettimeofday(&tr, 0);
	return	(tr.tv_sec ^ tr.tv_usec) & (seed - 1);
}
#endif
