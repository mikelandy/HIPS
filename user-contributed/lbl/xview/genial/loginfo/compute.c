/*
 * compute.c
 */

#include "display.h"
#include "ui.h"
#include "log.h"
#include "reg.h"
#include "common.h"
#include <math.h>


/***********************************************************/
double
compute_angle(p1, p2, p3, p4)
    XPoint    p1, p2, p3, p4;
{
    XPoint    vec1, vec2;	/* two lines in vector form */
    double    d1, d2;		/* magnitude of bothe vectors */
    double    cosa;		/* cosine of angle between them */
    double    angle;		/* the angle itself */

    vec1.x = p2.x - p1.x;
    vec1.y = p2.y - p1.y;

    vec2.x = p4.x - p3.x;
    vec2.y = p4.y - p3.y;

    p1.x = 0;
    p1.y = 0;
    d1 = distance(p1, vec1);
    d2 = distance(p1, vec2);
    cosa = (double) (vec1.x * vec2.x + vec1.y * vec2.y) / (d1 * d2);
    angle = acos(cosa);
    return (angle);
}

double
distance(pt1, pt2)
    XPoint    pt1, pt2;
{
    register double x1, y1, x2, y2, dsq;

    x1 = (double) pt1.x;
    y1 = (double) pt1.y;
    x2 = (double) pt2.x;
    y2 = (double) pt2.y;

    dsq = ((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
    return (sqrt(dsq));
}
