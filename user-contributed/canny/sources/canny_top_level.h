#define NOEDGE 0
#define POSSIBLE_EDGE 254
#define EDGE 255

/*----------------*/
/* CONVOLVE IMAGE */
/*--------------- */

/* POSSIBLE VALUES OF THE FILTERTYPE PARMETER */

#define NOSYMMETRY      0
#define SYMMETRIC       1
#define ANITSYMMETRIC   2

/* POSSIBLE VALUES OF THE BOUNDERY PARAMETER */

#define ZERO      0    /* ZEROES THE REGION OUTSIDE OF THE IMAGE */
#define WRAP      1    /* THE FILTER IS CIRCULAR */
#define MAKESMALL 2    /* RETURNS A SMALLER IMAGE. */
#define EXTEND    3    /* EXTENDS THE BOUNDARY PIXELS */
#define MASKIT    4    /* MASKS THE BOUNDARY TO ZERO */

/* POSSIBLE VALUES OF THE STATUS PARAMETER */

#define SUCCESS        1    /* SUCCESSFULL EXECUTION */
#define NO_SUCH_FILTER 2  /* IF THE FILTER TYPE PARAMETER IS NOT ONE OF THE ALLOWED VALS. */                                                  
#define EVENWINDOWSIZE  3  /* IF THE FILTER IS EVENSIZED */
#define NOSUCHDIRECTION 4 /* Direction is not XDIR of YDIR */
#define NOSUCHBOUNDERY 5  /* Nonexistant boundery option specified */

/* POSSIBLE VALUES OF THE DIRECTION PARAMETER */

#define XDIR  1
#define YDIR  2
