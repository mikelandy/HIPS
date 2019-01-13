/*
 * log.h -- include file for all functions which involve logging
 *
 */

#include <X11/Xlib.h>
#include "trace.h"
#include "histo.h"
#include "zoom.h"

struct strbs {			/* backing store for a string */
    /*
     * only stores enough information to be able to redraw the text and to be
     * able to refresh the image.  Doesn't store any image backgrounds
     */
    int       x, y;		/* same as x and y passed to XDrawString(). */
    char     *string;		/* the actual text */
    XCharStruct metr;		/* the metrics of the string as returned from
				 * XTextExtents */
};


struct logent {
    struct logent *next, *prev;
    int       opcode;		/* Op-Code representing the type of
				 * analytical function */
    int       id;		/* id which uniquely identifies this entry
				 * into the log */
    struct region *reg;		/* region on which this function operates */
    struct region *pvreg;	/* preview region pointer */
    int       fnum;		/* frame number for this log entry */

    unsigned char *auxdata;	/* auxiliary data, specific to the function */
    unsigned long auxdsize;	/* size of auxdata, also function dependent */
    struct strbs *sbs;		/* string associated with the log entry. This
				 * will be the log id, usually */

    struct trcontext *trace;	/* only one of these will be used at a time */
    struct hcontext *hist;
    struct zcontext *zoom;
};

/* a pointer to the current log entry */
extern struct logent *loghead, *logtail, *curfunc, *lastfunc, *newfunc();

extern XFontStruct *xfs;

/* a global variable signalling that the log is being loaded. */
extern int load_log;
