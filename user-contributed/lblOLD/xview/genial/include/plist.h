/*
 * plist.h -- include file for point lists chosen by the user
 *
 */

#include <X11/Xlib.h>
#define CRSIZE 16		/* width of a cross */
#define MAXPOINTS 200		/* max # of points in any list */
#define NODRAW 1		/* for dlists which need not be redrawn */

/* cbstore is a structure for storing the points underneath a cross indicating
   a user selection */
struct cbstore {
    XPoint    top;
    short     cht;		/* height of cross. */
    unsigned char vvals[(CRSIZE * 2) + 1];
    XPoint    left;
    short     cwt;		/* width of cross. */
    unsigned char hvals[(CRSIZE * 2) + 1];
};

/* plists are linked lists of points the user chose */
struct plist {
    struct plist *next, *prev;
    XPoint    pt;
    struct cbstore cb;
};

/* pbstore is used to store point/value vectors for fast and easy access */

struct pval {
    XPoint    pt;		/* x,y coordinate pair */
    unsigned char val;		/* XImage value */
    unsigned long oval;		/* original value off of image. */
};

struct dlist {
    struct dlist *next, *prev;
    int       flags;		/* special flags (line NODRAW) */
    int       len;		/* length of segment (number of points) in
				 * the following array. */
    struct pval *points;
};


extern struct plist *pl_find(), *pfind(), *pfind_all();

extern struct dlist *dlist_new();
