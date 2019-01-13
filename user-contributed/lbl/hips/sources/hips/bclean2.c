
/*  bclean.c                                      Brian Tierney,  LBL  3/90
 *
 *  removes small 8-connected objects from binary images.
 *
 *  usage: bclean [-s NN] < infile > outfile
 *    where -s NN remove all objects smaller than NN (default = 4)
 *
 *  2 versions are included:
 *  use #define RECURSIVE for recursive version, otherwise defaults
 *   to a non-recursive version which uses a stack.
 *  The stack version is a lot faster, but also uses a lot more memory
 *   ( twice as much )
 *
 *  Note: pixels in an object are considered connected if they are 8-connected
 *     ( diagonal neighbors )
 */

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

     This software is provided as a professional  academic  contribu-
tion  for  joint exchange.  Thus it is experimental, is pro-
vided ``as is'', with no warranties of any kind  whatsoever,
no  support,  promise  of updates, or printed documentation.
Bug reports or fixes may be sent to the author, who may or may
not act on them as he desires.
*/

/*   Author:  Brian L. Tierney
 *            Lawrence Berkeley Laboratory
 *            Imaging and Distributed Computing Group
 *            email: bltierney@lbl.gov
*/

#include <stdio.h>
#include <sys/types.h>

#include <hipl_format.h>

/* #define RECURSIVE */
/* #define DEBUG */

#define MIN_SIZE 4

#define Calloc(x,y) (y *)calloc((unsigned)(x), sizeof(y))
#define Realloc(x,y,z) (z *)realloc((char *)x,(unsigned)(y * sizeof(z)))

#ifndef RECURSIVE
typedef struct s_item {
    short     i, j;
}         STACK_ITEM;

STACK_ITEM *stack, *point_list;

int       sp, sp2;		/* global stack pointer */
int       stack_size;
#define STACK_CHUNK 5000
#endif

u_char  **pic, **newpic, **grid;
static int nrow, ncol, size;
static int min_size;
static int obj_cnt = 0;

int       c1, c2;
void      get_size(), get_size_r(), mark_object(), mark_object_r();
char     *realloc();

/* #define DEBUG */

/*************************************************************/
main(argc, argv)
    int       argc;
    char     *argv[];
{
    register int f;
    struct header hd;

    void      find_object();
    u_char  **alloc_2d_byte_array();

    register int i, j;

    Progname = strsave(*argv);

#ifdef DEBUG
    malloc_debug(1);
#endif

    min_size = MIN_SIZE;	/* default */
    if (argc > 1) {
	if (strcmp(argv[1], "-s") == 0)
	    min_size = atoi(argv[2]);
	else
	    usageterm();
    }
    read_header(&hd);
    if (hd.pixel_format != PFBYTE)
	perr(HE_MSG, "image pixel format must be byte");
    update_header(&hd, argc, argv);
    write_header(&hd);

    pic = alloc_2d_byte_array(hd.rows, hd.cols);
    newpic = alloc_2d_byte_array(hd.rows, hd.cols);
    grid = alloc_2d_byte_array(hd.rows, hd.cols);
    fprintf(stderr, " looking for objects of at least %d pixels \n", min_size);

    nrow = hd.orows;
    ncol = hd.ocols;
/*    stack_size = nrow * ncol; */
    stack_size = STACK_CHUNK;

    alloc_stack(stack_size);
    alloc_point_list(stack_size);

    for (f = 0; f < hd.num_frame; f++) {
#ifdef OLD
	read_2d_byte_array(stdin, pic, hd.rows, hd.cols);
#else
	hd.image = pic[0];
	if (read_image(hd, f) == HIPS_ERROR)
	    return (HIPS_ERROR);
#endif
	bzero((char *) grid[0], nrow * ncol);

	for (i = 0; i < nrow; i++) {
	    for (j = 0; j < ncol; j++) {

		if (pic[i][j] > 0 && grid[i][j] == 0) {
		    size = 0;
		    c1 = c2 = 0;
#ifdef RECURSIVE
		    get_size_r(i, j);
		    if (size > min_size)
			mark_object_r(i, j);
#else
		    get_size(i, j);
		    if (size > min_size)
			mark_object();
#endif
		    if (size > min_size) {
			obj_cnt++;
#ifdef DEBUG
			fprintf(stderr, "Object: %d, size %d \n",
				obj_cnt, size);
			fprintf(stderr, "\n # counted: %d, # marked: %d \n\n",
				c1, c2);
#endif
		    }
		}
	    }
	}
#ifdef OLD
	write_2d_byte_array(stdout, newpic, hd.rows, hd.cols);
#else
	hd.image = newpic[0];
	if (write_image(hd, f) == HIPS_ERROR)
	    return (HIPS_ERROR);
#endif
	bzero((char *) newpic[0], hd.rows * hd.cols);
    }
    fprintf(stderr, "%s done. \n\n", argv[0]);
    return (0);
}

/**********************************************************/
void
get_size_r(i, j)		/* recursive version  */
    int       i, j;
{
    int       ii, jj;

    size++;
    grid[i][j] = 1;		/* mark point as counted */
    c1++;

    ii = i + 1;
    if (ii < nrow && pic[ii][j] > 0 && grid[ii][j] == 0)
	get_size_r(ii, j);

    ii = i - 1;
    if (ii > 0 && pic[ii][j] > 0 && grid[ii][j] == 0)
	get_size_r(ii, j);

    jj = j + 1;
    if (jj < ncol && pic[i][jj] > 0 && grid[i][jj] == 0)
	get_size_r(i, jj);

    jj = j - 1;
    if (jj > 0 && pic[i][jj] > 0 && grid[i][jj] == 0)
	get_size_r(i, jj);

    /* diagonals */
    ii = i + 1;
    jj = j + 1;
    if (ii < nrow && jj < ncol && pic[ii][jj] > 0 && grid[ii][jj] == 0)
	get_size_r(ii, jj);

    ii = i - 1;
    jj = j - 1;
    if (ii > 0 && jj > 0 && pic[ii][jj] > 0 && grid[ii][jj] == 0)
	get_size_r(ii, jj);

    ii = i + 1;
    jj = j - 1;
    if (ii < nrow && jj > 0 && pic[ii][jj] > 0 && grid[ii][jj] == 0)
	get_size_r(ii, jj);

    ii = i - 1;
    jj = j + 1;
    if (ii > 0 && jj < ncol && pic[ii][jj] > 0 && grid[ii][jj] == 0)
	get_size_r(ii, jj);

    return;
}

/**********************************************************/
void
mark_object_r(i, j)		/* recursive version */
    int       i, j;
{
    int       ii, jj;

    newpic[i][j] = pic[i][j];
    c2++;

    ii = i + 1;
    if (ii < nrow && pic[ii][j] > 0 && newpic[ii][j] == 0)
	mark_object_r(ii, j);

    ii = i - 1;
    if (ii > 0 && pic[ii][j] > 0 && newpic[ii][j] == 0)
	mark_object_r(ii, j);

    jj = j + 1;
    if (jj < ncol && pic[i][jj] > 0 && newpic[i][jj] == 0)
	mark_object_r(i, jj);

    jj = j - 1;
    if (jj > 0 && pic[i][jj] > 0 && newpic[i][jj] == 0)
	mark_object_r(i, jj);

    /* diagonals */
    ii = i + 1;
    jj = j + 1;
    if (ii < nrow && jj < ncol && pic[ii][jj] > 0 && newpic[ii][jj] == 0)
	mark_object_r(ii, jj);

    ii = i - 1;
    jj = j - 1;
    if (ii > 0 && jj > 0 && pic[ii][jj] > 0 && newpic[ii][jj] == 0)
	mark_object_r(ii, jj);

    ii = i + 1;
    jj = j - 1;
    if (ii < nrow && jj > 0 && pic[ii][jj] > 0 && newpic[ii][jj] == 0)
	mark_object_r(ii, jj);

    ii = i - 1;
    jj = j + 1;
    if (ii > 0 && jj < ncol && pic[ii][jj] > 0 && newpic[ii][jj] == 0)
	mark_object_r(ii, jj);

    return;
}

/*********************************************************************/

usageterm()
{
    fprintf(stderr, "Usage: bclean [-s nn] < inseq > outseq \n ");
    fprintf(stderr, " Options: [-s nn] romove objects smaller than NN ( default = %d) \n", MIN_SIZE);

    exit(0);
}

/*********************************************************************/
#ifndef RECURSIVE
void
get_size(i, j)			/* non-recursive */
    int       i, j;
{
    int       ii, jj;

    sp = sp2 = 0;		/* initialize stack pointer */
    push(-1, -1);		/* null stack */

    do {
      start:
	grid[i][j] = 1;		/* mark grid */
	add_to_point_list(i, j);
	c1++;
	size++;

	ii = i + 1;
	if (ii < nrow && pic[ii][j] > 0 && grid[ii][j] == 0) {
	    push(i, j);
	    i++;
	    goto start;
	}
	ii = i - 1;
	if (ii > 0 && pic[ii][j] > 0 && grid[ii][j] == 0) {
	    push(i, j);
	    i--;
	    goto start;
	}
	jj = j + 1;
	if (jj < ncol && pic[i][jj] > 0 && grid[i][jj] == 0) {
	    push(i, j);
	    j++;
	    goto start;
	}
	jj = j - 1;
	if (jj > 0 && pic[i][jj] > 0 && grid[i][jj] == 0) {
	    push(i, j);
	    j--;
	    goto start;
	}
	/* diagonals */
	ii = i + 1;
	jj = j + 1;
	if (ii < nrow && jj < ncol && pic[ii][jj] > 0 && grid[ii][jj] == 0) {
	    push(i, j);
	    i++;
	    j++;
	    goto start;
	}
	ii = i - 1;
	jj = j - 1;
	if (ii > 0 && jj > 0 && pic[ii][jj] > 0 && grid[ii][jj] == 0) {
	    push(i, j);
	    i--;
	    j--;
	    goto start;
	}
	ii = i + 1;
	jj = j - 1;
	if (ii < nrow && jj > 0 && pic[ii][jj] > 0 && grid[ii][jj] == 0) {
	    push(i, j);
	    i++;
	    j--;
	    goto start;
	}
	ii = i - 1;
	jj = j + 1;
	if (ii > 0 && jj < ncol && pic[ii][jj] > 0 && grid[ii][jj] == 0) {
	    push(i, j);
	    i--;
	    j++;
	    goto start;
	}
	pop(&i, &j);

    } while (i >= 0);		/* neg i indicates empty stack */
    if (sp != 0)
	fprintf(stderr, "Error: stack not empty \n");

    return;
}

/***************************************************************/
void
mark_object()
{
    int       i, j;

    while (sp2 > 0) {
	i = point_list[sp2].i;
	j = point_list[sp2].j;

	newpic[i][j] = pic[i][j];
	sp2--;

	c2++;
    }
}

/***************************************************************/
alloc_stack(st_size)		/* allocation stack for non-recursive
				 * flood-fill alg */
    int       st_size;
{
    if ((stack = Calloc(st_size, STACK_ITEM)) == NULL)
	perror("calloc: stack");
}

/**************************************************************/
alloc_point_list(st_size)	/* allocation stack for non-recursive
				 * flood-fill alg */
    int       st_size;
{
    if ((point_list = Calloc(st_size, STACK_ITEM)) == NULL)
	perror("calloc: point list");
}

/***************************************************************/
push(i, j)
    int       i, j;
{
    sp++;

    if (sp >= stack_size) {
	stack_size += STACK_CHUNK;
#ifdef DEBUG
	fprintf(stderr, " increasing stack size to %d.. \n", stack_size);
#endif
	if ((stack = Realloc(stack, stack_size, STACK_ITEM)) == NULL)
	    perror("realloc");
	if ((point_list = Realloc(point_list, stack_size, STACK_ITEM)) == NULL)
	    perror("realloc");
    }
    stack[sp].i = i;
    stack[sp].j = j;
}

/***************************************************************/
add_to_point_list(i, j)
    int       i, j;
{
    sp2++;
    if (sp2 >= stack_size) {
	stack_size += STACK_CHUNK;
#ifdef DEBUG
	fprintf(stderr, " increasing stack size to %d.. \n", stack_size);
#endif
	if ((stack = Realloc(stack, stack_size, STACK_ITEM)) == NULL)
	    perror("realloc");
	if ((point_list = Realloc(point_list, stack_size, STACK_ITEM)) == NULL)
	    perror("realloc");

    }
    point_list[sp2].i = i;
    point_list[sp2].j = j;

}

/***************************************************************/
pop(i, j)
    int      *i, *j;
{
    *i = stack[sp].i;
    *j = stack[sp].j;
    sp--;
}

#endif
/***************************************************************/
show_grid()
{				/* for debugging */
    int       i, j;

    fprintf(stderr, " \n grid is: \n");
    for (i = 0; i < nrow; i++) {
	fprintf(stderr, "\n");
	for (j = 0; j < ncol; j++) {
	    fprintf(stderr, " %1d", grid[i][j]);
	}
    }
    fprintf(stderr, " \n\n");
}

/***************************************************************/
show_out_image()
{				/* for debugging */
    int       i, j;

    fprintf(stderr, " \n out image is: \n");
    for (i = 0; i < nrow; i++) {
	fprintf(stderr, "\n");
	for (j = 0; j < ncol; j++) {
	    if (newpic[i][j] == 0)
		fprintf(stderr, " %1d", 0);
	    else
		fprintf(stderr, " %1d", 1);
	}
    }
    fprintf(stderr, " \n\n");
}
