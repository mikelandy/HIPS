/* selects.c
 * Max Rible
 *
 * Select procedures for HIPStool functions.
 * All select procedures return void and take arguments of x and y
 * position and whether the mouse event is up or not.
 */

#include "hipstool.h"

static char message[100];

void
select_search(x, y, up, command)
     int x, y, up;
     Command *command;
{
    static int pidx = -1, cidx = -1;
    int m, n;
    enum primit breed;
    Primit *primit;

    if(command == NULL || command->func.primit == NULL) return;
    if(up) { pidx = -1; cidx = -1; return; }

    breed = command->func.primit->breed;

    if(priminfo[breed].select == list_select) {
	if(cidx == -1) {
	    (*priminfo[breed].select)(x, y, command->func.primit, -1);
	    cidx = 0;
	}
    } else {
	if(pidx >= 0)
	    if(priminfo[breed].drag_create) {
		if(!(cidx & 1)) pidx = -1; /* Drag stuff around */
	    } else {
		return;		/* Don't create new stuff on drag! */
	    }
	if(pidx == -1)
	    for(m = 0, primit = command->func.primit;
		m < command->func.len;
		m++, primit = command->func.primit + m) {
		for(n = 0; n < primit->len; n++) {
		    if(primit->loc.coords[n][0] == -1) {
			pidx = m; cidx = n; goto breakout;
		    }
		}
	    }
    breakout:
	if(pidx >= 0)
	    (*priminfo[breed].select)(x, y, command->func.primit + pidx, cidx);
	else
	    complete = 1;
    }
}

void
line_select(x, y, primit, idx)
     int x, y, idx;
     Primit *primit;
{
    if(primit->loc.coords[1][0] >= 0)
	undoline2d(&(primit->data.trace));

    primit->loc.coords[idx][0] = x;
    primit->loc.coords[idx][1] = y;

    if(primit->loc.coords[1][0] >= 0) {
	doline2d(primit->loc.coords[0][0], primit->loc.coords[0][1],
		 primit->loc.coords[1][0], primit->loc.coords[1][1],
		 &(primit->data.trace), 0, ON);
	Update_info("Line selected.");
    } else {
	Update_info("Terminate line.");
    }
}

void
box_select(x, y, primit, idx)
     int x, y, idx;
     Primit *primit;
{
    int i;

    if(primit->loc.coords[1][0] >= 0)
	for(i = 0; i < 4; i++)
	    undoline2d(primit->data.traces + i);

    primit->loc.coords[idx][0] = x; 
    primit->loc.coords[idx][1] = y;

    if(idx == 1) {
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
    } else {
	sprintf(message, "Box: start (%d,%d).", x, y);
    }

    Update_info(message);
}

/* ARGSUSED */
void
list_select(x, y, primit, idx)
     int x, y, idx;
     Primit *primit;
{
    add_point(&(primit->loc.list), x, y);
    primit->len++;
}

/* ARGSUSED */
void
point_select(x, y, primit, idx)
     int x, y, idx;
     Primit *primit;
{
    primit->data.cross.x = primit->loc.coords[0][0] = x;
    primit->data.cross.y = primit->loc.coords[0][1] = y;

    do_cross(&(primit->data.cross), ON);
}

/* ARGSUSED */
void
text_select(x, y, primit, idx)
     int x, y, idx;
     Primit *primit;
{
}
