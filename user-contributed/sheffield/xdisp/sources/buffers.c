/* Buffering code for X writes */

#include <X11/Intrinsic.h>
#include <xdisp.h>

#define	BUFFER_DEPTH	512

struct {
	XPoint	data[MAXCOLOR][BUFFER_DEPTH];
	int	npoints[MAXCOLOR];
	} points;

/* There are MAXCOLOR buffers each of size BUFFER_DEPTH. Each buffer entry
   buffers an X point (x,y) under the appropriate color. The buffer is
   flushed when it fills, or on expicit request at the end of a sequence
   of X writes.
*/

/*********************************
 * init_buffer()
 *********************************/

void init_buffer()
{
  int i;

    for (i = 0; i < MAXCOLOR; i++)
	points.npoints[i] = 0;
}



/*********************************
 * buffer_point()
 *********************************/

void buffer_point(pixmap,gc,color,x,y)
  Pixmap	pixmap;
  GC		gc;
  int		color;
  int		x;
  int		y;
{
    if (points.npoints[color] == BUFFER_DEPTH-1) {
	
	/*
	 * Buffer is full. Set foreground color of graphics context
	 * and draw points in pixmap
	 */

	XSetForeground(dpy,gc,color);
	XDrawPoints(dpy,pixmap,gc,points.data[color],
			points.npoints[color],CoordModeOrigin);

	/*
	 * Reset buffer
	 */

	points.npoints[color] = 0;
	
	}

    /*
     * Store point in buffer according to color
     */

    points.data[color][points.npoints[color]].x = x;
    points.data[color][points.npoints[color]].y = y;
    points.npoints[color]+= 1;
}



/*********************************
 * flush_buffer()
 *********************************/

void flush_buffer(pixmap,gc)
  Pixmap	pixmap;
  GC		gc;
{
  int i;

    for (i = 0; i < MAXCOLOR; i++) 
	if (points.npoints[i]) {
	    XSetForeground(dpy,gc,i);
	    XDrawPoints(dpy,pixmap,gc,points.data[i],
	 		points.npoints[i],CoordModeOrigin);
	    points.npoints[i] = 0;
	    }
}



