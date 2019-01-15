/* $Header: scaleg.h,v 3.0 88/10/10 13:52:11 ph Locked $ */

/* #define DEBUG  */
#define UNDEF PIXEL_UNDEFINED
#define Calloc(a,b) (b *) calloc((unsigned)(a), sizeof(b))

/* the mapping from discrete dest coord b to continuous source coord: */
#define MAP(b, scale, offset)  ((b)+(offset))/(scale)


typedef struct {	/* SOURCE TO DEST COORDINATE MAPPING */
    double sx, sy, sz;	/* x and y scales */
    double tx, ty, tz;	/* x and y translations */
    double ux, uy, uz;	/* x and y offset used by MAP, private fields */
} Mapping;

typedef struct {	/* ZOOM-SPECIFIC FILTER PARAMETERS */
    double scale;	/* filter scale (spacing between centers in a space) */
    double supp;	/* scaled filter support radius */
    int wid;		/* filter width: max number of nonzero samples */
} Filtpar;

/* see explanation in zoom.c */

extern int zoom_debug;
extern int zoom_coerce;	/* simplify filters if possible? */
extern int zoom_xy;	/* filter x before y (1) or vice versa (0)? */
extern int zoom_trimzeros;	/* trim zeros from filter weight table? */
