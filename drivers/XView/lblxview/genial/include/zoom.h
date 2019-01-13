/*
 * zoom.h -- definitions for zoom functions in genial
 *
 */

#include "zoom_ui.h"

/* zcontext is the context of one zoom operation */
struct zcontext {
  zoom_zmwin_objects *display; /* pointer to the GUIDE-generated structure */
  Xv_Window paintwin; /* paintwin associated with this zoom window */
  XID zxid;            /* XID for zoom */

  XImage *zim; /* XImage of the zoom */
  XPoint p1, p2; /* the 2 points which define the rectangle for this zoom*/
  int zmfac; /* zoom scaling factor */
  int can_width, can_height;  /* size of zoom window canvas */
};

extern struct zcontext *newzoom(), *zoom_by_win(), *zoom_by_lid();

/* direction the user can be scroll in the zoom window */
#define LEFT 1
#define RIGHT 2
#define UP 3
#define DOWN 4
