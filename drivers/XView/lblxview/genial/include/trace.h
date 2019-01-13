/*
 * trace.h -- include file for traces and trace contexts
 *
 */

#include "trace_ui.h"

/* data structures for trace integration: */

#define NINT 10			/* max number of integrations for a trace */

#define STARTY 10

struct integ_value {
    int       min, max;		/* min and max values from trace */
    int       isize;		/* number of points in ival */
    int      *ival;		/* array of integration values */
};

struct integ_context {
    struct integ_value ireg[NINT];
    int       cnum;		/* # of current integration (0-NINT) */
    Xv_window ipntw, iwin, icanv;	/* handles for integral display */
    XID       ixid;		/* XID of above */
    struct graph_lab *yglab;
    struct cbstore *cbl;	/* backing store of the left cross  */
    struct cbstore *cbr;	/* backing store of the right cross */
};

struct tropts {			/* structure containing modifiable trace
				 * options */
    int       avgrad;		/* radius to be used for averaging in order
				 * to make smooth traces. */
    int       scale_type;	/* type of scaling to do */
    int       axmin, axmax;	/* min and max values to be used in creating
				 * the scaled axis labels */
    /*
     * we may wish to create an array of (XPoint, string) doubles which
     * contains the appropriate information necessary to refresh the display
     * of each trace window QUICKLY
     */
    /*
     * may also wish to have some integer indicating # of labels and allow
     * this to be modified.
     */
};

struct trcontext {
    trace_trwin_objects *win_info;
    Xv_window trpntw;		/* XView window handles for a trace win */
    XID       trxid;		/* XID of above */
    struct pval *pbuf;		/* point buffer */
    int       npts;		/* number of indices into above */
    int       x, y;		/* coordinate pair of last mouse event */
    int       flags;		/* a mask of flags */
    struct cbstore *cb;		/* backing store of the cross in the original
				 * window */
    struct tropts t_attr;	/* localized, modifiable trace attributes */
    struct graph_lab *yglab;	/* graph labels along y axis */
    struct integ_context integ;	/* used for integration */
};

#define SMPHT .056		/* the amount of the display determined to be
				 * reasonable for the display of a sample bar
				 * of what the grey values really look like.
				 * On a 270 pixel high display(the default),
				 * this is 15 pixels. */

/* to be used with the flags member of a trcontext: */
#define RECT 0x01		/* trace is rectangular */


extern struct trcontext *ctx_by_lid();

extern unsigned trace_value();

void      trcanv_repaint_proc();
