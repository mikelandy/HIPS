
/* isobuild.h */

/* This program implements the marching cubes surface tiler described by
 * Lorensen & Cline in the Siggraph 87 Conference Proceedings.
 *
 *   Author:  Brian L. Tierney
 *            Lawrence Berkeley Laboratory
 *            Imaging Technologies Group
 *            email: bltierney@lbl.gov
 *
 * Portions developed at the National Center for Supercomputing Applications at
 * the University of Illinois at Urbana-Champaign.
 * Written by Mike Krogh, NCSA, Feb.  2, 1990
 *
 */

/*   This program is copyright (C) 1991, Regents  of  the
University  of  California.   Anyone may reproduce this software,
in whole or in part, provided that:
(1)  Any copy  or  redistribution  must  show  the
     Regents  of  the  University of California, through its
     Lawrence Berkeley Laboratory, as the source,  and  must
     include this notice;
(2)  Any use of this software must reference this  distribu-
     tion,  state that the software copyright is held by the
     Regents of the University of California, and  that  the
     software is used by their permission.

     It is acknowledged that the U.S. Government has  rights
to this software under  Contract DE-AC03-765F00098 between the U.S.
Department of Energy and the University of California.

     This software is provided as a professional  academic  contribu-
tion  for  joint exchange.  Thus it is experimental, is pro-
vided ``as is'', with no warranties of any kind  whatsoever,
no  support,  promise  of updates, or printed documentation.
Bug reports or fixes may be sent to the author, who may or may
not act on them as he desires.
*/

/* $Id: isobuild.h,v 1.5 1992/01/31 02:06:50 tierney Exp $ */

/* $Log: isobuild.h,v $
 * Revision 1.5  1992/01/31  02:06:50  tierney
 * *** empty log message ***
 *
 * Revision 1.4  1992/01/30  20:06:20  davidr
 * prior to Brian
 *
 * Revision 1.3  1992/01/10  01:59:31  davidr
 * works with triserv now
 *
 * Revision 1.2  1991/12/19  01:42:20  davidr
 * added RCS identification markers
 * */


/*   #define SPEED */			/* show execution speed */

#include <stdio.h>
#include <math.h>
#include <sys/types.h>
#include <hipl_format.h>
#include <sys/time.h>
#ifdef SPEED
#include <sys/timeb.h>
#endif

/*************************************************************/
/* defines section */
/* #define DEBUG */
/* #define TESTING */


/* define this for floating point input: no reason to do this in
   most cases, it just uses more memory
*/

/*#define FLOAT_INPUT  *//* internal data set type float */

#define SHORT_NORMALS   /* see normal.c for explanation */

/* some usefull macros */
#define Fread(a,b,c,d) fread((char *)(a), b, (int)(c), d)
#define Calloc(x,y) (y *)calloc((unsigned)(x), sizeof(y))
#define Fwrite(a,b,c,d) fwrite((char *)(a), b, (int)(c), d)
#define Memset(a,b,c) memset((char *)(a), b, (int)(c))

/****************************************************************/
#ifdef FLOAT_INPUT
typedef float Data_type;
#define	FakeFrames	999999
#else
typedef unsigned char Data_type;
#define	FakeFrames	255
#endif

typedef unsigned char Grid_type;

/* Norm_type may be short or float */
#ifdef FLOAT_INPUT
typedef float Norm_type;
#else
#ifdef SHORT_NORMALS
typedef short Norm_type;
#else
typedef float Norm_type;
#endif
#endif

typedef struct {
    u_char       nverts;		/* possible values: 0-7 */
    u_char       verts[8];
    u_char       nedges;		/* possible values: 0-12 */
    u_char       edges[12];
    u_char       npolys;		/* possible values: 0-10 */
    u_char       polys[30];	/* 10 polys * 3 vertices = 30 */
}         CELL_ENTRY;

typedef struct {		/* structure for detecting regions to skip */
    int       xloc, yloc;
    int       width, height;
    Data_type min, max;
    Data_type **dslice;		/* pointer to a 2D data slice */
    Grid_type **grid;		/* pointer to a 2D grid slice */
}         BLOCK_INFO, *BINFO_PTR;

typedef struct norm {
    Norm_type x, y, z;
}         NORMAL_VECT;

/* output data structure */
typedef struct plist {
    u_char    x, y, z;
/* CHANGE to something better -- davidr */
    Norm_type nx, ny, nz;
}         POINT, *POINT_PTR;

typedef struct vlist {
    float     x, y, z;
}         VERTEX, FLOAT_VECT, *VERT_PTR;

typedef struct norm_list {
    Norm_type nx, ny, nz;
}         NORMAL, *NORM_PTR;

typedef struct {		/* used to detect duplicate vertices */
    int       edge_index;
    int       num_edges;
}         CUBE_TRIANGLES;

#ifdef MAIN

/*********************************************************************/

 /* set command line arguement defaults */
char      MY_NAME[120];		/* name of this program (for error messages ) */
int       PRE_NORMALS = 0;	/* pre-compute all normal values */
Data_type TVAL = 50.;		/* threshold value */
int       VERBOSE = 0;		/* print status messages */
int       VERBOSE2 = 0;		/* print even more status messages */
int       MARCH_NORMALS = 0;	/* compute vertex normals flag */
int       OUTPUT_TYPE = 0;	/* type of output file: binary or ASCII */
int       DIVIDING = 0;		/* marching or dividing cubes method */
int       SERVER = 0;		/* run in server mode */
int       DUP_CHECK = 1;	/* check for duplicate vertices */
short     PORT_NUM = 10000;	/* socket port number for server */
int       SEG_METHOD = 0;	/* data segmentation method: threshold (0) or
				 * flood fill (1) */
int       BRIDGE_TYPE = 3;	/* in flood_fill segmentation: check strength
				 * of connecting bridges before continuing
				 * the fill */
int       SMOOTH_GRID = 0;	/* apply filter to smooth binary mask */
int       SKIP_BLOCKS = 1;	/* create block min/max map for speed */
int       SMALLER_BOX = 0;	/* flag indicating that the entire data set
				 * is not being examined */
int       BLOCK_SIZE = 0;	/* use xdim / 12  */
int       DO_CUBE_CENTER_NORMALS = 1;  /* more accurate normals for dividing 
					  cubes version */
int       SMALL_CHUNKS = 1;     /* output is done in small chunks, not all 
				   at once */
int       SX = 0, SY = 0, SZ = 0;	/* starting and ending locations to
					 * examine from the original data
					 * set. */
int       EX = 9999, EY = 9999, EZ = 9999;

char     *IN_FNAME = NULL;
char     *OUT_FNAME = NULL;
char     *OUT_MASK_FNAME = NULL;
char     *IN_MASK_FNAME = NULL;

FILE     *poly_fp = NULL;
FILE     *out_mask_fp = NULL;
FILE     *in_mask_fp = NULL;


/* global data /segmentation arrays */

Data_type ***data = NULL;	/* hold input data */

NORMAL_VECT ***normals = NULL;	/* holds normals */
Grid_type ***grid = NULL;	/* segmentation grid */
BLOCK_INFO **block_info_array = NULL;
FLOAT_VECT **gradient_curr_slice=NULL, **gradient_next_slice=NULL;

int       xdim, ydim, zdim;	/* size of input data volume */
int       xmax, ymax, zmax;	/* xmax = xdim -1, etc */
int       num_blocks;		/* number of blocks in a slice */

/* minimum and maximun values in the input data set */
Data_type data_min = FakeFrames, data_max = 0;

/* used to eliminate duplicate verts */
CUBE_TRIANGLES **prevslice = NULL, **currslice = NULL;

#else

extern char MY_NAME[];
extern int PRE_NORMALS;
extern int VERBOSE;
extern int VERBOSE2;
extern int MARCH_NORMALS;
extern int OUTPUT_TYPE;
extern int DIVIDING;
extern int SEG_METHOD;
extern int BRIDGE_TYPE;
extern int SMALLER_BOX;
extern short PORT_NUM;
extern int SX, SY, SZ, EX, EY, EZ;
extern Data_type TVAL;		/* threshold value */
extern int SERVER;
extern int SKIP_HOLES;
extern int SKIP_BLOCKS;
extern int BLOCK_SIZE;
extern int DUP_CHECK;
extern int    SMOOTH_GRID;
extern int    SMALL_CHUNKS;
extern int    DO_CUBE_CENTER_NORMALS;

extern char *IN_FNAME;
extern char *IN_MASK_FNAME;

extern FILE *poly_fp;
extern FILE *in_mask_fp;
extern FILE *out_mask_fp;

extern Data_type ***data;
extern NORMAL_VECT ***normals;
extern Grid_type ***grid;
extern BLOCK_INFO **block_info_array;
extern FLOAT_VECT **gradient_curr_slice, **gradient_next_slice;
extern int xdim, ydim, zdim, xmax, ymax, zmax, num_blocks;

extern Data_type data_min, data_max;
extern CUBE_TRIANGLES **prevslice, **currslice;

#endif

void      Error(), Status(), exit_handler();
