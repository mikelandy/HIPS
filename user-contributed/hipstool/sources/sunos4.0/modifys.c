/* modifys.c
 * Max Rible
 *
 * Modify procedures for HIPStool functions.
 * All modify procedures return void and take arguments of x and y
 * position and whether the mouse event is up or not.
 */

#include "hipstool.h"

static char message[100];

void
modify_search(x, y, up, command)
     int x, y, up;
     Command *command;
{
    static int cidx = -1, pidx = -1;
    int m, n;
    enum primit breed;
    Primit *primit;

    if(command == NULL || command->func.primit == NULL) return;

    breed = command->func.primit->breed;

    if(up) { 
	cidx = -1; pidx = -1; 
	if(priminfo[breed].modify == list_modify)
	    (*priminfo[breed].modify)(0, 0, NULL, -1);
	return; 
    }

    if(priminfo[breed].modify == list_modify) {
	for(m = 0, n = 0; m < command->func.len && n == 0; m++)
	    n = (*priminfo[breed].modify)(x, y, command->func.primit + m, -1);
    } else {
	if(pidx == -1) 
	    for(m = 0, primit = command->func.primit; 
		m < command->func.len; 
		m++, primit = command->func.primit + m) {
		for(n = 0; n < primit->len; n++) {
		    if((abs(primit->loc.coords[n][0] - x) < SNAPDIST) &&
		       (abs(primit->loc.coords[n][1] - y) < SNAPDIST)) {
			pidx = m; cidx = n; goto breakout;
		    }
		}
	    }
    breakout:
	if(pidx >= 0)
	    (*priminfo[breed].modify)(x, y, command->func.primit + pidx, cidx);
    }
}

/* line_modify for LEAST_SQUARES_FIT is the same as list_modify. */

int
line_modify(x, y, primit, idx)
     int x, y, idx;
     Primit *primit;
{
    undoline2d(&(primit->data.trace));
    primit->loc.coords[idx][0] = x;
    primit->loc.coords[idx][1] = y;
    doline2d(primit->loc.coords[0][0], primit->loc.coords[0][1],
	     primit->loc.coords[1][0], primit->loc.coords[1][1],
	     &(primit->data.trace), 0, ON);
    return(0);
}

int
box_modify(x, y, primit, idx)
     int x, y;
     Primit *primit;
{
    int i;

    if(primit->loc.coords[1][0] >= 0)
	for(i = 0; i < 4; i++)
	    undoline2d(primit->data.traces + i);

    primit->loc.coords[idx][0] = x; primit->loc.coords[idx][1] = y;

    doline2d(primit->loc.coords[0][0], primit->loc.coords[0][1],
	     primit->loc.coords[1][0], primit->loc.coords[0][1],
	     primit->data.traces, 0, ON);
    doline2d(primit->loc.coords[0][0], primit->loc.coords[1][1],
	     primit->loc.coords[1][0], primit->loc.coords[1][1],
	     primit->data.traces+1, 0, ON);
    doline2d(primit->loc.coords[0][0], primit->loc.coords[0][1]+1,
	     primit->loc.coords[0][0], primit->loc.coords[1][1]-1,
	     primit->data.traces+2, 0, ON);
    doline2d(primit->loc.coords[1][0], primit->loc.coords[0][1]+1,
	     primit->loc.coords[1][0], primit->loc.coords[1][1]-1,
	     primit->data.traces+3, 0, ON);

    sprintf(message, "Box: selected (%d,%d)-(%d,%d).", 
	    primit->loc.coords[0][0], primit->loc.coords[0][1],
	    primit->loc.coords[1][0], primit->loc.coords[1][1]);
    Update_info(message);
    return(0);
}

/* ARGSUSED */
int
list_modify(x, y, primit, idx)
     int x, y, idx;
     Primit *primit;
{
    if(primit != NULL)
	return(move_point(primit->loc.list, x, y));
    else
	return(move_point(NULL, 0, 0));	/* Reset */
}

/* ARGSUSED */
int
point_modify(x, y, primit, idx)
     int x, y, idx;
     Primit *primit;
{
    do_cross(&(primit->data.cross), OFF);
    primit->data.cross.x = primit->loc.coords[0][0] = x;
    primit->data.cross.y = primit->loc.coords[0][1] = y;
    do_cross(&(primit->data.cross), ON);

    return(0);
}

/* ARGSUSED */
int
text_modify(x, y, primit, idx)
     int x, y, idx;
     Primit *primit;
{
    return(0);
}
