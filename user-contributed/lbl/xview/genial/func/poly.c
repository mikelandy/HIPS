/*
 * poly.c -- routines for drawing polygons
 *
 */

#include "reg.h"
#include "common.h"
#include "llist.h"
#include "display.h"
#include "ui.h"
#include <X11/Xlib.h>
#include <stdio.h>

draw_polygon(reg)
    struct region *reg;
{
    poly_interp(reg->r_dlist, reg->r_plist);
    draw_dlist(img_win->d_xid, reg->r_dlist);
}

poly_interp(dlist, ptlist)
    struct dlist *dlist;
    struct plist *ptlist;
{
    struct plist *org, *dst;
    struct dlist *seg;

    seg = dlist;
    for (org = ptlist, dst = org->next; dst != NULL; org = dst, dst = org->next) {
	ras_line(org->pt, dst->pt, seg, orig_ximg, orig_img);
	seg = (struct dlist *) malloc(sizeof(struct dlist));
	llist_add((llist *) seg, (llist **) & dlist, (llist **) NULL);
    }
    ras_line(org->pt, ptlist->pt, seg, orig_ximg, orig_img);
}

/*****************************************************************/

char     *
polygon_info(reg)		/* create string containing message giving
				 * info on spline */
    struct region *reg;
{
    char      mesg[80];
    int       xmax, xmin, ymax, ymin, width, height, area;
    struct plist *tr;

    xmax = xmin = reg->r_plist->pt.x;
    ymax = ymin = reg->r_plist->pt.y;

    /* find min and max of points defining the spline */

    for (tr = reg->r_plist; tr != NULL; tr = tr->next) {
	if (tr->pt.x > xmax)
	    xmax = tr->pt.x;
	if (tr->pt.x < xmin)
	    xmin = tr->pt.x;
	if (tr->pt.y > ymax)
	    ymax = tr->pt.y;
	if (tr->pt.y < ymin)
	    ymin = tr->pt.y;
    }
    width = xmax - xmin;
    height = ymax - ymin;
    area = convex_poly_area(reg->r_plist);

    sprintf(mesg, "Polygon width: %d pixels;  height: %d pixels;  area: %d ",
	    width, height, area);

    return (mesg);
}

/**************************************************************/
int
convex_poly_area(verts)
    struct plist *verts;
{
    typedef struct PointStruct {
	int    x, y;
    }         Point;

    /* This routine is adapted from the method used in Graphics Gems,
     but simplifed for the 2D case. I dont completely understand the
     math here, so I hope this gives the correct results.... -BT
     */

    float     area_sum = 0., area;
    Point    p0, p1, p2;
    struct plist *pinfo;

    /* compute areas of the sub-triangles of a polygon */

    pinfo = verts;
    p0.x = pinfo->pt.x;
    p0.y = pinfo->pt.y;
    pinfo = pinfo->next;
    p1.x = pinfo->pt.x;
    p1.y = pinfo->pt.y;
    while (pinfo->next != NULL) {
	pinfo = pinfo->next;
        p2.x = pinfo->pt.x;
        p2.y = pinfo->pt.y;

	area = (.5 * (float)abs(((p1.x - p0.x) * (p2.y - p0.y)) - 
		     ((p1.y - p0.y) * (p2.x - p0.x))));

        area_sum += area;

	p1.x = p2.x;
	p1.y = p2.y;
    }
    return ((int) (area_sum + .5));
}

/**************************************************************/

#ifdef OLD_METHOD
/* adapted from "Graphics Gems", Academic Press, 1990
 *
 * original routines were for 3D polygons, so as a quick hack
 * I just set the z-component equal to 1. There is most certainly
 * a 2D version that would be faster
 */

typedef struct PointStruct {
    double    x, y, z;
}         Point3;
typedef Point3 Vector3;

int
convex_poly_area(verts)
    struct plist *verts;
{
    int       i;
    float     area_sum = 0;
    Vector3   v1, v2, v3, *V3Cross(), *V3Sub();
    Point3    p0, p1, p2;
    double    V3Length();
    struct plist *pinfo;

    /* compute relative areas of the sub-triangles of polygon */

    pinfo = verts;
    p0.x = pinfo->pt.x;
    p0.y = pinfo->pt.y;
    p0.z = p1.z = p2.z = 1.;
    pinfo = pinfo->next;
    p1.x = pinfo->pt.x;
    p1.y = pinfo->pt.y;
    while (pinfo->next != NULL) {
	pinfo = pinfo->next;
        p2.x = pinfo->pt.x;
        p2.y = pinfo->pt.y;
	V3Sub(&p1, &p0, &v1);
	V3Sub(&p2, &p0, &v2);
	v1.z = v2.z = v3.z = 1.0;
	V3Cross(&v1, &v2, &v3);
	area_sum += (.5 * V3Length(&v3));

	p1.x = p2.x;
	p1.y = p2.y;
    }
    return ((int) (area_sum + .5));
}

/* return the cross product c = a cross b */
Vector3  *
V3Cross(a, b, c)
    Vector3  *a, *b, *c;
{
    c->x = (a->y * b->z) - (a->z * b->y);
    c->y = (a->z * b->x) - (a->x * b->z);
    c->z = (a->x * b->y) - (a->y * b->x);
    return (c);
};

/* return vector difference c = a-b */
Vector3  *
V3Sub(a, b, c)
    Vector3  *a, *b, *c;
{
    c->x = a->x - b->x;
    c->y = a->y - b->y;
    c->z = a->z - b->z;
    return (c);
};

/* returns squared length of input vector */
double
V3SquaredLength(a)
    Vector3  *a;
{
    return ((a->x * a->x) + (a->y * a->y) + (a->z * a->z));
};

/* returns length of input vector */
double
V3Length(a)
    Vector3  *a;
{
    return (sqrt(V3SquaredLength(a)));
};

#endif
