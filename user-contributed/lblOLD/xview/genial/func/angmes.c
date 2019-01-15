
/*
 * angmes.c -- find the angle between two vectors (lines)
 */

#include "display.h"
#include "ui.h"
#include "log.h"
#include "reg.h"
#include "common.h"
#include <math.h>

ames_init()
{
    clear_info();
    lab_info(" Please select two lines:", 1);
    lab_info(" To find the angle between them, hit <eval>", 2);

    setrtype(DUBLIN);
    reg_setdom(NONE);
    return 0;
}

ames_eval()
{
    static char msg[80];
    XPoint    p1, p2, p3, p4;
    double    angle, compute_angle();
    struct dlist *dl;

    if (curfunc->reg->r_plen != 4) {
	fprintf(stderr, "Error: 2 lines not yet selected. (%d) \n",
		curfunc->reg->r_plen);
	XBell(display, 0);
	return -1;
    }
    dl = curfunc->reg->r_dlist;
    if (dl == NULL)
	return -1;

    p1.x = dl->points[0].pt.x;
    p1.y = dl->points[0].pt.y;
    p2.x = dl->points[dl->len - 1].pt.x;
    p2.y = dl->points[dl->len - 1].pt.y;

    dl = dl->next;
    if (dl == NULL)
	return -1;

    p3.x = dl->points[0].pt.x;
    p3.y = dl->points[0].pt.y;
    p4.x = dl->points[dl->len - 1].pt.x;
    p4.y = dl->points[dl->len - 1].pt.y;

    angle = compute_angle(p1, p2, p3, p4);

    sprintf(msg, "Angle between lines: %.2f radians ; %.2f degrees",
	    (float) angle, (float) angle * (180 / M_PI));

    xv_set(base_win->infomesg, PANEL_LABEL_STRING, msg, NULL);
    panel_paint(base_win->infomesg, PANEL_CLEAR);

    return 0;
}

/***********************************************************/
double
compute_angle(p1, p2, p3, p4)	/* returns the angle in radians between 2
				 * lines defined by these 4 points */
    XPoint    p1, p2, p3, p4;
{
    struct vect {
	float     x, y;
    };

    struct vect vec1, vec2;	/* two lines in vector form */
    double    angle, mag1, mag2;

    vec1.x = p2.x - p1.x;
    vec1.y = p2.y - p1.y;

    vec2.x = p4.x - p3.x;
    vec2.y = p4.y - p3.y;

    mag1 = sqrt((double) ((vec1.x * vec1.x) + (vec1.y * vec1.y)));
    mag2 = sqrt((double) ((vec2.x * vec2.x) + (vec2.y * vec2.y)));

    /* normalize vector */
    vec1.x /= mag1;
    vec1.y /= mag1;
    vec2.x /= mag2;
    vec2.y /= mag2;

#define ANGLE_0-90 
#ifdef ANGLE_0-90
    /* force angle to be between 0 and 90 degrees */
    angle = fabs(vec1.x * vec2.x + vec1.y * vec2.y);
    if (angle > 1.0)
       angle = 1.0;
    angle = acos(angle);
#else
    /* force angle to be between 0 and 180 degrees */
    /* this doesnt work!! */
    angle = (vec1.x * vec2.x + vec1.y * vec2.y);
    if (angle > 1.0)
       angle = 1.0;
    if (angle < -1.0)
       angle = -1.0;
    if (angle > M_PI)
       angle -= M_PI;
    if (angle < 0.0)
       angle += M_PI;
#endif
    return (angle);
}

/*************************************************************/

ames_change()
{

    printf("ames_change: not yet implemented. \n");

}

/*************************************************************/

ames_clear()
{
    clear_info();

    return 0;
}

ames_reset()
{
    xv_set(base_win->eval,
	   PANEL_INACTIVE, FALSE,
	   NULL);
}
