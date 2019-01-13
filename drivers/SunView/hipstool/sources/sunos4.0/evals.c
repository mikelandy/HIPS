/* evals.c
 * Max Rible
 *
 * Setup and Eval procedures for HIPStool functions.
 * Setup procedures take the current command and ON or OFF.
 * All eval procedures take the current command.
 */

#include "hipstool.h"

static char string[100];

void
box_eval(command)
     Command *command;
{
    Primit *primit = command->func.primit;
    int dx, dy;

    dx = primit->loc.coords[1][0] - primit->loc.coords[0][0] + 1;
    dy = primit->loc.coords[1][1] - primit->loc.coords[0][1] + 1;
    sprintf(string, "%dx%d image selected (%d pixels).", dx, dy, dx*dy);
    save_menu_funcs[SAVE_BOX_SUBIMAGE].active = 1;
    update_save_menu();
    Update_info(string);
}

void
angle_eval(command)
     Command *command;
{
    int (*loc[2])[2];
    double a, b;

    loc[0] = command->func.primit[0].loc.coords;
    loc[1] = command->func.primit[1].loc.coords;

    a = atan2( (float) (loc[0][1][1] - loc[0][0][1]), 
	       (float) (loc[0][1][0] - loc[0][0][0]) );
    b = atan2( (float) (loc[1][1][1] - loc[1][0][1]), 
	       (float) (loc[1][1][0] - loc[1][0][0]) );

    theta = a - b;

    /* Convert to reasonable orientation-- these are lines, not vectors */
    theta = fmod(theta, M_PI);
    theta = fabs(theta);
    if(theta > (M_PI/2.0)) theta = theta - M_PI;
    theta = fabs(theta);

    sprintf(string, "The angle is %.4g radians or %.4g degrees.",
	    theta, 360.0*(theta/(2.0*M_PI)));
    Update_info(string);
}

void
distance_eval(command)
     Command *command;
{
    Primit *primit = command->func.primit;
    double a, b;

    a = (double) (primit[1].loc.coords[0][0] - primit[0].loc.coords[0][0]);
    b = (double) (primit[1].loc.coords[0][1] - primit[0].loc.coords[0][1]);
    sprintf(string, "Distance from (%d,%d)-(%d,%d) is %d.",
	    primit[0].loc.coords[0][0], primit[0].loc.coords[0][1],
	    primit[1].loc.coords[0][0], primit[1].loc.coords[0][1],
	    (int) sqrt(a*a+b*b));
    Update_info(string);
}
