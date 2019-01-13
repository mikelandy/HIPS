/* log_comment.c
 * Max Rible
 *
 * Handling for any special comments in the log.
 * Takes a char *, and a log entry, returns a char *.
 */

#include "hipstool.h"

char string[100];

char *
straight_comment(usercomment)
     char *usercomment;
{
    if(strlen(usercomment) > 0)
	return(Strdup(usercomment));
    else
	return(NULL);
}

char *
angle_comment(usercomment)
     char *usercomment;
{
    if(strlen(usercomment) > 0)
	sprintf(string, "[%.4g rad/%.3g deg]:  %s.", theta,
		360.0*(theta/(2.0*M_PI)), usercomment);
    else
	sprintf(string, "[%.4g radians or %.3g degrees.]", theta, 
		360.0*(theta/(2.0*M_PI)));

    return(Strdup(string));
}

char *
distance_comment(usercomment)
     char *usercomment;
{
    int a, b, c, d;

    a = cur_func->func.primit[0].data.cross.x;
    b = cur_func->func.primit[0].data.cross.y;
    c = cur_func->func.primit[1].data.cross.x;
    d = cur_func->func.primit[1].data.cross.y;

    if(strlen(usercomment) > 0)
	sprintf(string, "[(%d,%d)-(%d,%d)=%d]:  %s.", a, b, c, d,
		(int)sqrt((double)((c-a)*(c-a)) + (double)((d-b)*(d-b))),
		usercomment);
    else
	sprintf(string, "Distance from (%d,%d) to (%d,%d) is %d.",
		a, b, c, d,
		(int)sqrt((double)((c-a)*(c-a)) + (double)((d-b)*(d-b))));

    return(Strdup(string));
}
