/* spline.c
 * Max Rible
 *
 * Spline routines and weirdness for hipstool.
 */

#include "hipstool.h"

#define THRESHOLD 3.0		/* 1.0 is better, but superplot uses 3.0 */
#define round(q) ((int)((q) + .5))

static int lastx, lasty;
static void spline(), Cspline();

/* Spline routines in here are stolen outright from 
 * superplot, which is
/*************************************************************/
/*                                                           */
/*  Copyright (c) 1986                                       */
/*  Marc S. Majka - UBC Laboratory for Computational Vision  */
/*                                                           */
/*  Permission is hereby granted to copy all or any part of  */
/*  this program for free distribution.   The author's name  */
/*  and this copyright notice must be included in any copy.  */
/*                                                           */
/*************************************************************/


void
do_spline(points, closed, drawmode)
     Point points;
     int closed;
     int drawmode;
{
    Point tmp;
    int len, *vector, idx;

    len = depth(points);
    vector = Calloc(2*len, int);

    for(idx = 0, tmp = points; tmp != NULLPOINT; tmp = tmp->next) {
	vector[idx++] = tmp->i.x;
	vector[idx++] = tmp->i.y;
    }

    spline((closed == DO_SPLINE_CLOSED ? 1 : 0), len, vector, drawmode);

    Cfree(vector, 2*len, int);
}

/* "move":  put point at x, y
 * "draw":  draw a line to x, y
 * "dblmove":  move, takes doubles as input
 * "dbldraw":  move, takes doubles as input
 */

/* This comment is from Superplot:
/* based on the spline algorithms in:
 * FIG : Facility for Interactive Generation of figures
 * by Supoj Sutanthavibul (supoj@sally.UTEXAS.EDU)
 * used with permission from the author.
 */

/* spline():  draw splines
/* Apparently n is the number of points, and v is a vector of
 * numbers, length 2*n, with the even entries being x and the
 * odd entries being y.  k seems to be a boolean flag:
 * it seems to determine whether the spline is open or 
 * closed.
 */
static void
spline(k, n, v, t)
     int k;			/* Flags */
     int n, *v;
     int t;
{
    double cx_1, cy_1, cx_2, cy_2, cx_3, cy_3, cx_4, cy_4;
    double x_1, y_1, x_2, y_2;
    int i;
    
    x_1 = v[0];
    y_1 = v[1];
    x_2 = v[2];
    y_2 = v[3];

    cx_1 = (x_1 + x_2) / 2.0;
    cy_1 = (y_1 + y_2) / 2.0;
    cx_2 = (x_1 + 3.0 * x_2) / 4.0;
    cy_2 = (y_1 + 3.0 * y_2) / 4.0;
    
    (*draw[t])(round(x_1), round(y_1), 
	       lastx = round(cx_1), lasty = round(cy_1));

    for (i = 2; i < n; i++) {
	x_1 = x_2;
	y_1 = y_2;
	x_2 = v[i*2];
	y_2 = v[i*2 + 1];
	cx_3 = (3.0 * x_1 + x_2) / 4.0;
	cy_3 = (3.0 * y_1 + y_2) / 4.0;
	cx_4 = (x_1 + x_2) / 2.0;
	cy_4 = (y_1 + y_2) / 2.0;
	
	Cspline(cx_1, cy_1, cx_2, cy_2, cx_3, cy_3, cx_4, cy_4, t);
	
	cx_1 = cx_4;
	cy_1 = cy_4;
	cx_2 = (x_1 + 3.0 * x_2) / 4.0;
	cy_2 = (y_1 + 3.0 * y_2) / 4.0;
    }
    if (k == 0) {
	(*draw[t])(round(cx_1), round(cy_1),
		   lastx = round(x_2), lasty = round(y_2));
    } else {
	x_1 = x_2;
	y_1 = y_2;
	x_2 = v[0];
	y_2 = v[1];
	cx_3 = (3.0 * x_1 + x_2) / 4.0;
	cy_3 = (3.0 * y_1 + y_2) / 4.0;
	cx_4 = (x_1 + x_2) / 2.0;
	cy_4 = (y_1 + y_2) / 2.0;
	Cspline(cx_1, cy_1, cx_2, cy_2, cx_3, cy_3, cx_4, cy_4, t);
	(*draw[t])(lastx, lasty, 
		   round(x_2), round(y_2));
    }
}

/****************************************************************************

	The following spline drawing routine is from 

	"An Algorithm for High-Speed Curve Generation" 
	by George Merrill Chaikin,
	Computer Graphics and Image Processing, 3, Academic Press, 
	1974, 346-349.

	and

	"On Chaikin's Algorithm" by R. F. Riesenfeld,
	Computer Graphics and Image Processing, 4, Academic Press, 
	1975, 304-310.

*****************************************************************************/

static void
Cspline(x_1, y_1, x_2, y_2, x_3, y_3, x_4, y_4, t)
     double x_1, y_1, x_2, y_2, x_3, y_3, x_4, y_4;
     int t;
{
    double xmid, ymid, fabs();
    
    xmid = (x_2 + x_3) / 2.0;
    ymid = (y_2 + y_3) / 2.0;

    if (fabs(x_1 - xmid) < THRESHOLD && fabs(y_1 - ymid) < THRESHOLD) {
	(*draw[t])(round(x_1), round(y_1), 
		   lastx = round(xmid), lasty = round(ymid));
    } else {
	Cspline(x_1, y_1, ((x_1+x_2)/2.0), ((y_1+y_2)/2.0),
		((x_2+xmid)/2.0), ((y_2+ymid)/2.0), xmid, ymid, t);
    }
    
    if (fabs(xmid - x_4) < THRESHOLD && fabs(ymid - y_4) < THRESHOLD) {
	(*draw[t])(round(xmid), round(ymid), 
		   lastx = round(x_4), lasty = round(y_4));
    } else {
	Cspline(xmid, ymid, ((xmid+x_3)/2.0), ((ymid+y_3)/2.0),
		((x_3+x_4)/2.0), ((y_3+y_4)/2.0), x_4, y_4, t);
    }
}
