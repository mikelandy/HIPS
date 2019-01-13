
/* getobj.h    file for getobj3d        Brian Tierney, LBL   */

#include <stdio.h>
#include <sys/types.h>
#include <math.h>
#include <hipl_format.h>

#define Calloc(x,y) (y *)calloc((unsigned)(x), sizeof(y))

char     *Progname;

u_char ***image;
u_short ***sht_image;
u_int  ***int_image;
u_char ***grid;

int       nrow, ncol, nframe;	/* number of rows, columns in the image */
int       pix_format, nvoxels;

/* for statistics */
long      pcnt;			/* count of number of pixels selected by
				 * object_fill  */
double    pix_total;		/* sum of pixel values in an object */

/* global command line args */
int       stats;		/* flag to indicate if should display
				 * statistics on each object */
int       tval, bg, verbose, find_all, bridges, output_binary_mask, min_size;
int       isx, isy, isz;	/* input seed point */
int       val_file;		/* value file flag */
char     *valfile;		/* name of file of seed values */

/* file descriptors */
FILE     *vf, *df;

/* used in stack to simulate recursion */
typedef struct s_item {
    short     i, j, k;
}         STACK_ITEM;
STACK_ITEM *stack;

int       sp;			/* global stack pointer */
int       stack_size;


/***********************************************************************/
/*  COPYRIGHT NOTICE         *******************************************/
/***********************************************************************/

/*   This program is copyright (C) 1990, Regents  of  the
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

     This software is provided as a professional  academic
contribution for  joint exchange.  Thus it is experimental, is
provided ``as is'', with no warranties of any kind  whatsoever,
no  support,  promise  of updates, or printed documentation.
Bug reports or fixes may be sent to the author, who may or may
not act on them as he desires.
*/

/*   Author:  Brian L. Tierney
 *            Lawrence Berkeley Laboratory
 *            Imaging and Distributed Computing Group
 *            email: bltierney@lbl.gov
*/
