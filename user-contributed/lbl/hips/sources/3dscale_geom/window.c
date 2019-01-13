
#include "simple.h"
#include "window.h"

static char rcsid[] = "$Header: window.c,v 2.0 88/10/10 13:46:29 ph Locked $";

window_set(x0, y0, z0, x1, y1, z1, a)
register Window *a;
{
    a->x0 = x0;
    a->y0 = y0;
    a->z0 = z0;
    a->x1 = x1;
    a->y1 = y1;
    a->z1 = z1;
}

window_clip(a, b)		/* a=intersect(a,b), return overlap bit */
register Window *a, *b;
{
    int overlap;

    overlap = window_overlap(a, b);
    window_intersect(a, b, a);
    return overlap;
}

window_intersect(a, b, c)	/* c = intersect(a,b) */
register Window *a, *b, *c;
{
    c->x0 = MAX(a->x0, b->x0);
    c->y0 = MAX(a->y0, b->y0);
    c->z0 = MAX(a->z0, b->z0);
    c->x1 = MIN(a->x1, b->x1);
    c->y1 = MIN(a->y1, b->y1);
    c->z1 = MIN(a->z1, b->z1);
}

window_overlap(a, b)
register Window *a, *b;
{
    return a->x0<=b->x1 && a->x1>=b->x0
	&& a->y0<=b->y1 && a->y1>=b->y0
	&& a->z0<=b->z1 && a->z1>=b->z0;
}

window_print(str, a)
char *str;
Window *a;
{
    fprintf(stderr,"%s{%d,%d,%d,%d,%d,%d}%d,%d,%d",
	str, a->x0, a->y0, a->z0, a->x1, a->y1, a->z1,
	a->x1-a->x0+1, a->y1-a->y0+1, a->z1-a->z0+1);
}

/*----------------------------------------------------------------------*/

window_box_intersect(a, b, c)
register Window_box *a, *b, *c;
{
    c->x0 = MAX(a->x0, b->x0);
    c->y0 = MAX(a->y0, b->y0);
    c->z0 = MAX(a->z0, b->z0);
    c->x1 = MIN(a->x1, b->x1);
    c->y1 = MIN(a->y1, b->y1);
    c->z1 = MIN(a->z1, b->z1);
    window_box_set_size(c);
}

window_box_print(str, a)
char *str;
Window_box *a;
{
    fprintf(stderr,"%s{%d,%d,%d,%d,%d,%d}%d,%d,%d",
	str, a->x0, a->y0, a->z0, a->x1, a->y1, a->z1, a->nx, a->ny, a->nz);
}

window_box_set_max(a)
register Window_box *a;
{
    a->x1 = a->x0+a->nx-1;
    a->y1 = a->y0+a->ny-1;
    a->z1 = a->z0+a->nz-1;
}

window_box_set_size(a)
register Window_box *a;
{
    a->nx = a->x1-a->x0+1;
    a->ny = a->y1-a->y0+1;
    a->nz = a->z1-a->z0+1;
}
