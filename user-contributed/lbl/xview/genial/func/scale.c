/*
 * scale.c -- routines to label graphs in an easy-to-read way.
 *            ~antony, 6/16/91
 */

#include <stdio.h>
#include <math.h>
#include "scale.h"

/* build_glab() builds an array of graph_lab structures for a graph with range
 *  min->max, with up to maxlab labels and of length p_len.
 * p_len is the length in pixels of the display region and is used in
 * construction of the p_off member of the graph_lab structure.
 *
 * build_glab() returns the number of graph_lab structures allocated in the
 * vector pointed to by *lvec, and 0 upon error.
 */

/* build_glab() should maintain the min and max which are passed in, and just
 * do 'clean' labelling where possible in between the min and max.
 */

int
build_glab(min, max, maxlab, p_len, lvec)
    int       min, max, maxlab, p_len;
    struct graph_lab **lvec;
{
    int       range, order;
    int       start, end;	/* start and end values of intermediate
				 * labels */
    int       nlab;		/* total number of labels */
    int       delta;		/* distance between labels */
    int       val, i;

    range = max - min;
    if (range < 1) {
#ifdef DEBUG
	printf("Ranges must be whole numbers.\n");
#endif
	return 0;
    }
    /* order is the order of ten that the range falls into */
    order = (int) pow(10.,floor(log10((double) range)));

    /*
     * now we need to find what type of labelling to do for the points in
     * between min and max based on order and on maxlab.
     */
    /*
     * We will want to find delta, which is a suitable difference between
     * each of the labels.
     */
    if ((delta = opt_div(order, maxlab, range)) < 0) {
#ifdef DEBUG
	printf("error finding the optimal division.\n");
#endif
	return 0;
    }
    /*
     * find the starting point for the first multiple of delta which is
     * greater than min
     */
    start = lmult(delta, min);
    /*
     * find the ending point for the labels:  highest multiple of delta which
     * is less than max
     */
    end = hmult(delta, max);
    /* find the number of labels.  Add 2 for the min and max */
    nlab = (end - start) / delta + 1 + 2;

    /* and finally:  allocate space for the labels and fill them in */
    *lvec = (struct graph_lab *) malloc(nlab * sizeof(struct graph_lab));
    if (*lvec == NULL) {
	perror("malloc");
	exit(0);
    }
    /* set the first and last lvec structures appropriately */
    (*lvec)[0].val = min;
    sprintf((*lvec)[0].lab, "%d", (*lvec)[0].val);
    (*lvec)[0].p_off = 0;
    (*lvec)[nlab - 1].val = max;
    sprintf((*lvec)[nlab - 1].lab, "%d", (*lvec)[nlab - 1].val);
    (*lvec)[nlab - 1].p_off = p_len;

    /* fill in the labels in between */
    for (val = start; val <= end; val += delta) {
	i = (val - start) / delta + 1;
	(*lvec)[i].val = val;
	sprintf((*lvec)[i].lab, "%d", (*lvec)[i].val);
	(*lvec)[i].p_off = (val - min) * p_len / range;
    }
    return nlab;
}

/* opt_div()  Find a good delta for graph labels based on order and maxlab.
   returns: correct delta on success, -1 on error */
/* we could code this to be a bit more general, but there is no compelling
   reason to do so. A more general version might be more compact, but would
   probably be much less readable as a result. */
/* the way this is coded now is fairly naieve:  just try each of the
   possibilities in turn,  with each succesive attempt representing one that
   will yield a greater number of labels than the previous try.  As soon as
   we pass maxlab, we return the previous try */
int
opt_div(order, maxlab, range)
    int       order, maxlab, range;
{
    int       delta;		/* the delta we are trying */

    /* try a half-decade of the next order up */
    delta = (order * 10) / 2;
    if (max_fit(delta, range) > maxlab) {
	/* couldn't even do half-decades of the next order of magnitude up. */
	/* return an error */
	return -1;
    }
    /* try twentades of the next order up */
    delta = (order * 10) / 5;
    if (max_fit(delta, range) > maxlab) {
	delta = (order * 10) / 2;
	return delta;
    }
    /* try decades of order */
    delta = order;
    if (max_fit(delta, range) > maxlab) {
	delta = (order * 10) / 5;
	return delta;
    }
    /* check if (order==1).  If so, we can't subdivide more than that. */
    if (order == 1) {
	return delta;
    }
    /* try half-decades of order */
    delta = order / 2;
    if (max_fit(delta, range) > maxlab) {
	delta = order;
	return delta;
    }
    /* and last, try twentades of order */
    delta = order / 5;
    if (max_fit(delta, range) > maxlab) {
	delta = order / 2;
	return delta;
    } else {
	/* twentades worked */
	return delta;
    }
}

/* max_fit() -- the max number of labels of size delta which can fit in the
   given range. */
int
max_fit(delta, range)
    int       delta, range;
{
    return range / delta;
}

/* lmult() -- lowest multiple of delta which is larger than min */
int
lmult(delta, min)
    int       delta, min;
{
    return (min / delta + 1) * delta;
}

/* hmult() -- highest multiple of delta which is less than max */
int 
hmult(delta, max)
    int       delta, max;
{
    if ((max % delta) == 0) {	/* divides evenly */
	return (max / delta - 1) * delta;
    }
    /* otherwise natural truncation from integer divides will do the trick */
    return (max / delta) * delta;
}

/* glab_db() -- debug a graph label vector, when passed a vector of
 * graph_lab structures and the number of indices.
 */
glab_db(lvec, len)
    struct graph_lab *lvec;
    int       len;
{
    int       i;

    for (i = 0; i < len; i++) {
	fprintf(stderr, "Value:%d, Label:%s, pixel offset: %d\n", lvec[i].val,
		lvec[i].lab, lvec[i].p_off);
    }
}
