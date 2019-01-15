
/*  fill3d.c:  routines to perform a 3d flood fill using  a stack
               to simulate recursion.

   Brian Tierney,   LBL

*/

#include "getobj.h"
/***************************************************************/
int
object_fill(c, r, f)		/* label all points within the item 1 */
    int       c, r, f;

/* this routine uses a stack to do a 3D recursive flood fill starting
   at location (c,r,f)  */
{
    void      sum_pixel();

    int       cnt = 0, skip = 0;/* count number of points in object */
    sp = 0;			/* initialize stack pointer */

    push(-1, -1, -1);		/* null stack */
    do {

start:
	grid[f][r][c] = 3;
	cnt++;
	if (stats)
	    sum_pixel(c, r, f);
	if (bridges > 0 && cnt > 1) {	/* check strength of connecting
					 * bridges */

	    if (bridges == 1) {	/* 2D bridges */
		cnt += mark_edge_neighbors(c, r, f, 0);
		if (count_diag_neighbor_edges(c, r, f, 0) >= 2) {
		    skip++;
		    goto w_stop;
		}
	    }
	    if (bridges == 2) {	/* 3D weak bridges */
		cnt += mark_edge_neighbors(c, r, f, 1);
		if (count_diag_neighbor_edges(c, r, f, 1) >= 4) {
		    skip++;
		    goto w_stop;
		}
	    }
	    if (bridges == 3) {	/* 3D strong bridges */
		cnt += mark_edge_neighbors(c, r, f, 1);
		if (count_diag_neighbor_edges(c, r, f, 1) >= 2) {
		    skip++;
		    goto w_stop;
		}
	    }
	} else {
	    cnt += mark_edge_neighbors(c, r, f, 0);	/* include edges */
	}

	if ((f < nframe - 1) && (grid[f + 1][r][c] == 1)) {
	    push(c, r, f);
	    f++;
	    goto start;
	}
	if ((f > 0) && (grid[f - 1][r][c] == 1)) {
	    push(c, r, f);
	    f--;
	    goto start;
	}
	if ((r < nrow - 1) && (grid[f][r + 1][c] == 1)) {
	    push(c, r, f);
	    r++;
	    goto start;
	}
	if ((r > 0) && (grid[f][r - 1][c] == 1)) {
	    push(c, r, f);
	    r--;
	    goto start;
	}
	if ((c < ncol - 1) && (grid[f][r][c + 1] == 1)) {
	    push(c, r, f);
	    c++;
	    goto start;
	}
	if ((c > 0) && (grid[f][r][c - 1] == 1)) {
	    push(c, r, f);
	    c--;
	    goto start;
	}
w_stop:
	pop(&c, &r, &f);

    } while (f >= 0);		/* neg i indicates empty stack */

    if (sp != 0)
	fprintf(stderr, "Error: stack not empty \n");

    if (verbose)
	if (bridges)
	    fprintf(stderr, "   fill stopped %d times due to weak bridges \n",
		    skip);

    return (cnt);
}

/***************************************************************/
push(i, j, k)			/* add location to stack */
    int       i, j, k;
{
    sp++;
    if (sp >= stack_size) {
	fprintf(stderr, "recursive stack overflow!! ");
	fprintf(stderr, " stack was allocated %d slots \n", stack_size);
	exit(-1);
    }
    stack[sp].i = i;
    stack[sp].j = j;
    stack[sp].k = k;
}

/***************************************************************/
pop(i, j, k)			/* remove item from stack */
    int      *i, *j, *k;
{
    *i = stack[sp].i;
    *j = stack[sp].j;
    *k = stack[sp].k;
    sp--;
}

/***************************************************************/
alloc_stack(st_size)		/* allocate stack for non-recursive
				 * flood-fill alg */
    int       st_size;
{
    if ((stack = Calloc(st_size, STACK_ITEM)) == NULL)
	perror("calloc: stack");
}
