/* list.c
 * Max Rible
 *
 * Manipulation of lists of points.
 */

#include "hipstool.h"

void
add_point(list, x, y)
     Point *list;
     int x, y;
{
    Point tmp;

    if(*list == NULLPOINT) {
	tmp = *list = Calloc(1, struct point);
	tmp->prev = NULLPOINT;
    } else {
	for(tmp = *list; tmp->next != NULL; tmp = tmp->next) ;
	tmp->next = Calloc(1, struct point);
	tmp = tmp->next;
	(*list)->prev = tmp;
    }
    tmp->next = NULLPOINT;
    tmp->i.x = x; tmp->i.y = y;
    do_cross(&tmp->i, ON);
}

int
move_point(list, x, y)
    Point list;
    int x, y;
{
    static Point current;
    Point tmp;

    if(list == NULLPOINT) {
	current = NULLPOINT;
	return(0);
    }

    if(current == NULLPOINT)
	for(tmp = list; tmp != NULLPOINT; tmp = tmp->next)
	    if((abs(tmp->i.x - x) < SNAPDIST) &&
	       (abs(tmp->i.y - y) < SNAPDIST)) {
		current = tmp;
		break;
	    }
    if(current != NULLPOINT) {
	do_cross(&current->i, OFF);
	current->i.x = x; current->i.y = y;
	do_cross(&current->i, ON);
	return(1);		/* Success */
    } else
	return(0);
}

int
depth(list)
     Point list;
{
    Point tmp;
    int i;

    for(i = 0, tmp = list; tmp != NULLPOINT; i++, tmp = tmp->next) ;
    return(i);
}
