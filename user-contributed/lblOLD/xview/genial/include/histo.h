/*
 * histo.h -- definitions for histograms
 *
 */

#include "histo_ui.h"

extern histo_histo_ctrl_objects	*histo_ctrl;

#define NUM_BUCKETS 512         /* number of histogram buckets */
#define NUM_SUB 8               /* number of sub intervals */

#define GRWIDTH 512
#define GRHGT 280
#define SX 10
#define SY 10
 
/* the following structure defined by antony to facilitate histogram
   'lookback' to the original image */

struct lbent {
  XPoint *parr; /*the dynamically sized vector of points which compose this
                  histogram bucket */
  int asize; /* size of parr.  Never decremented.  We never free() parr, we
                just let it fault itself in */
  int npts; /* reset to zero for every histogram, number of points in this
               bucket.  same hdata[index] above... */
};


/* the following is just to store upper and lower bounds for each of the
   six lookback colors. */
struct subrange {
  int isactive;   /* flag to indicate that this subrange is in use */
  int lower, upper; /*each of these is a bucket */
  int islit; /* 0 if unlit, 1 if lit */
};

struct hcontext {
  int rtype; /* indicates the region type of histogram.  One of BOX, POLYGON,
		or CLSPLINE.  NOREG is used to indicate that this is a free
	       context which may be recycled */
  int minv, maxv; /* local extrema for this histogram only.  Fairly painful to
		     search for them when called, but means there is less
		     wasted bucket space.  Could be controllable by a control
		     panel item */
  XPoint *pbuf; /* buffer of the points associated with this histogram.  A
		   necessary evil */
  int pbsize, pblen; /* size of above and length of above.  Size only changes
		        when we realloc(), len is number of actual points */
  int countvec[NUM_BUCKETS]; /* the actual counts for histogram information */
  struct lbent lbtab[NUM_BUCKETS]; /* table to look back at the original
				      image.  Built when histogram is
				      computed.  */
  int maxc, minc; /* max and min counts for vertical labelings */

  histo_display_objects *histo_display; /* window containing the histogram */
  Xv_window paintwin; /* xid of the paintwin associated with this window */
  XID hxid; /* xid of the paintwin associated with this window */
  struct subrange interval[NUM_SUB]; /* a selected subinterval */
  int subnum; /* number of the presently active subrange */

  int minb, maxb; /* min and max. bucket.  Used for magnification.  Default is
		     0 and NUM_BUCKETS, obviously */
  struct graph_lab *xglab, *yglab; /* the graph labels along the X and Y
				      axes. */
  int xglen, yglen; /* number of entries in the above glab vectors. */
};

