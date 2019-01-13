/*
 * reg.h -- include file for region selection
 *
 */

#include "plist.h"

/* IDs for the different types of regions */
#define NOREG 0
#define LINE 1
#define SPLINE 2
#define CLSPLINE 3
#define POLYGON 4
#define BOX 5
#define DUBLIN 6		/* double line for angle measure. */
#define ENTIRE_IMAGE 7

/* all the following regions types refer to annotation of the image */
#define AN_TEXT 8		/* textual label */
#define AN_VEC 9		/* vector (arrow-shaped thingy) */
#define MAXREG 10

/* IDs for different type of region "groups" */
#define LINEAR 1
#define AREA 2
#define ANOT 3
#define TRACE 4
#define NONE 10

/* structure of a region */
struct region {
    int       r_type;		/* type of region
				 * (LINE,SPLINE,POLYGON,BOX,etc) */
    int       r_flags;		/* special region modes (e.g. least-squares) */
    /*
     * plist is now a vector of linked lists to allow for compound regions
     * while still preserving record boundaries
     */
    struct plist *r_plist;	/* points which the user supplied */
    int       r_plen;		/* number of entries in above list */
    struct dlist *r_dlist;	/* points which form the region */
    struct strbs *r_sbs;	/* for region type AN_TEXT */
};

extern struct region *interp_reg(), *newreg();
