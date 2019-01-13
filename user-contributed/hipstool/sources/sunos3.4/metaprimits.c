/* metaprimits.c
 * Max Rible
 *
 * Meta-primitives for HIPStool.  Not at the OS level.
 */

#include "hipstool.h"

struct primit_info priminfo[NUM_PRIMITS] = {
/*  { name, loc_size, data_type, drag_create,
    select, modify, display, listflag } */
    { "line", 2, DATA_TRACE, 0,	/* drag_create set to 1 for CLICK_AND_DRAG */
	  line_select, line_modify, primit_display_line, 0 },
    { "box", 2, DATA_BOX, 1,
	  box_select, box_modify, primit_display_box, 0 },
    { "open spline", VARIES, DATA_TRACE, 0,
	  list_select, list_modify, primit_display_open_spline, 1 },
    { "closed spline", VARIES, DATA_NONE, 0,
	  list_select, list_modify, primit_display_closed_spline, 1 },
    { "polygon", VARIES, DATA_NONE, 0,
	  list_select, list_modify, primit_display_polygon, 1 },
    { "point", 1, DATA_CROSS, 1,
	  point_select, point_modify, primit_display_point, 0 },
    { "text", 1, DATA_TEXT, 0,
	  text_select, text_modify, primit_display_text, 0 },
};

/* Call palloc(PRIMIT_FOO, ...).  ..., for anything with loc_type
 * FIXED and loc_size VARIES, is a second argument specifying number.
 * examples:
 * palloc(PRIMIT_LINE) or palloc(PRIMIT_LINE, 1) returns a line
 * palloc(PRIMIT_POINT, 2) returns a pair of point primitives
 */
Primit *
palloc(va_alist)
     va_dcl
{
    int breed;
    unsigned size, num;
    int i, j;
    Primit *new;
    va_list index;

    va_start(index);

    breed = va_arg(index, int);
    if(breed >= NUM_PRIMITS) return(NULL);
    num = va_arg(index, unsigned);
    if(num == 0) num = 1;

    new = Calloc(num, Primit);
    for(i = 0; i < num; i++) {
	new[i].breed = breed;
	new[i].len = 0;		/* Unless loc_size != VARIES... */
	new[i].listp = 0;	/* Not a list... yet */
	new[i].auxiliary = 0;	/* No data present... yet */
	new[i].next = NULL;
    }

    if((size = priminfo[breed].loc_size) != VARIES)
	for(i = 0; i < num; i++) {
	    new[i].loc.coords = (int (*)[2]) calloc(2*size, sizeof(int));
	    for(j = 0; j < size; j++)
		new[i].loc.coords[j][0] = new[i].loc.coords[j][1] = -1;
	    new[i].len = size;
	}

    return(new);
}	 

/* pfree takes a primitive and de-allocates its resources.
 */
void
pfree(primit)
     Primit *primit;
{
    if(primit == NULL) return;

    if(priminfo[primit->breed].loc_size == VARIES) 
	destroy(TYPE_POINTLIST, &(primit->loc.list));
    else
	if(primit->len > 0)
	    Cfree(primit->loc.coords, 2*primit->len, int);

    if(primit->data.txt != NULL)
	free(primit->data.txt);

    Cfree(primit, 1, Primit);
}

void 
primit_display_line(primit, t)
     Primit *primit;
     int t;
{
    (*draw[t])(primit->loc.coords[0][0], primit->loc.coords[0][1],
	       primit->loc.coords[1][0], primit->loc.coords[1][1]);
}

void 
primit_display_box(primit, t)
     Primit *primit;
     int t;
{
    (*draw[t])(primit->loc.coords[0][0], primit->loc.coords[0][1],
	       primit->loc.coords[1][0], primit->loc.coords[0][1]);
    (*draw[t])(primit->loc.coords[0][0], primit->loc.coords[1][1],
	       primit->loc.coords[1][0], primit->loc.coords[1][1]);
    (*draw[t])(primit->loc.coords[0][0], primit->loc.coords[0][1],
	       primit->loc.coords[0][0], primit->loc.coords[1][1]);
    (*draw[t])(primit->loc.coords[1][0], primit->loc.coords[0][1],
	       primit->loc.coords[1][0], primit->loc.coords[1][1]);
}

/* ARGSUSED */
void 
primit_display_open_spline(primit, t)
     Primit *primit;
     int t;
{
    do_spline(primit->loc.list, DO_SPLINE_OPEN, t);
}

/* ARGSUSED */
void 
primit_display_closed_spline(primit, t)
     Primit *primit;
     int t;
{
    do_spline(primit->loc.list, DO_SPLINE_CLOSED, t);
}

/* ARGSUSED */
void 
primit_display_polygon(primit, t)
     Primit *primit;
     int t;
{
    Point tmp;

    for(tmp = primit->loc.list; tmp->next != NULL; tmp = tmp->next) {
	(*draw[t])(tmp->i.x, tmp->i.y, 
		   tmp->next->i.x, tmp->next->i.y);
    }

    (*draw[t])((primit->loc.list)->i.x, 
	       (primit->loc.list)->i.y,
	       (primit->loc.list)->prev->i.x, 
	       (primit->loc.list)->prev->i.y);
}

/* ARGSUSED */
void 
primit_display_point(primit, t)
     Primit *primit;
     int t;
{
    primit->data.cross.x = primit->loc.coords[0][0];
    primit->data.cross.y = primit->loc.coords[0][1];

    do_cross(&(primit->data.cross), ON);
}

/* ARGSUSED */
void 
primit_display_text(primit, t)
     Primit *primit;
     int t;
{
}

static int positions[4*CROSS_RADIUS + 1][2] = { 
    { -1, -1 } };

void
do_cross(target, status)
     Cross *target;
     int status;
{
    int x, y, n, i;

    if((positions[0][0] == -1) && (positions[0][1] == -1)) {
	positions[0][0] = positions[0][1] = 0;
	for(i = 1; i <= CROSS_RADIUS; i++) {
	    positions[4*i-3][0] = -i;	positions[4*i-3][1] = 0;
	    positions[4*i-2][0] = i;	positions[4*i-2][1] = 0;
	    positions[4*i-1][0] = 0;	positions[4*i-1][1] = -i;
	    positions[4*i][0] = 0;	positions[4*i][1] = i;
	}
    }

/* the n = is only to shut lint up. */
#define Put(a, b, v) (n = ((((a) >= 0) && ((a) < base.winfo.width) && \
			    ((b) >= 0) && ((b) < base.winfo.height)) ? \
			   put_pix(&base.winfo, (a), (b), (int)(v)), 0 : 0) )
#define Get(a, b) ((((a) >= 0) && ((a) < base.winfo.width) && \
		    ((b) >= 0) && ((b) < base.winfo.height)) ? \
		   (unsigned char) get_pix(&base.winfo, (a), (b)) : 0)

    x = target->x; y = target->y;

    if(status == OFF) {
	for(i = 0; i < (4*CROSS_RADIUS + 1); i++)
	    Put(positions[i][0] + x, positions[i][1] + y, target->values[i]);
    } else if(status == ON) {
	for(i = 0; i < (4*CROSS_RADIUS + 1); i++) {
	    target->values[i] = Get(x + positions[i][0], y + positions[i][1]);
	    Put(positions[i][0] + x, positions[i][1] + y, STANDOUT);
	}
    }
#undef Put
#undef Get
    n = n;			/* Shut lint up */
}

void
change_line_drawing_mode(mode)
     int mode;
{
    line_drawing_mode = mode;

    switch(mode) {
    case CLICK_AND_CLICK:
	priminfo[PRIMIT_LINE].select = line_select;
	priminfo[PRIMIT_LINE].modify = line_modify;
	priminfo[PRIMIT_LINE].drag_create = 0;
	priminfo[PRIMIT_LINE].listflag = 0;
	break;
    case CLICK_AND_DRAG:
	priminfo[PRIMIT_LINE].select = line_select;
	priminfo[PRIMIT_LINE].modify = line_modify;
	priminfo[PRIMIT_LINE].drag_create = 1;
	priminfo[PRIMIT_LINE].listflag = 0;
	break;
    case LEAST_SQUARES_FIT:
	priminfo[PRIMIT_LINE].select = list_select;
	priminfo[PRIMIT_LINE].modify = list_modify;
	priminfo[PRIMIT_LINE].drag_create = 0;
	priminfo[PRIMIT_LINE].listflag = 1;
	break;
    }
}
