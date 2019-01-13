
/*  linesegs.c                             Brian Tierney,  LBL  12/90
 *
 *    This program creates straight line segments of minimum length N
 *     from thinned (single pixel line) binary HIPS images.
 *
 *  Usage: linesegs [-l N][-a R][-f[F] fname] < inseq > outseq
 *    Options: [-l N]: minimun line segment size N ( default = 10)
 *          [-a R]: combine lines if difference in angle < R (default = 1.5)
 *         [-f line file name]: create file with angles andlengths of lines
 *         [-F line file name]: file with headers and line endpoint locations
 */

/*   Author:  Brian L. Tierney
 *            Lawrence Berkeley Laboratory
 *            Imaging and Distributed Computing Group
 *            email: bltierney@lbl.gov
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

#include <stdio.h>
#include <math.h>
#include <sys/types.h>

#include <hipl_format.h>

typedef struct endpt {
    int       x, y;
    struct endpt *next;
    struct endpt *prev;
}         ENDPT, *ENDPTR;

#define SEG_SIZE 10
#define SLOPE_DIFF 1.5
#define ANGLE_DIFF 4.0

#define PVAL 255		/* endpoint */
#define PVAL2 160		/* lines */


ENDPTR    list_head;

/* #define DEBUG */

u_char  **image, **out_image;
int       nrow, ncol;
int       seg_size, long_report;
float     angle_diff;
char     *fname;

/*************************************************************/
main(argc, argv)
    int       argc;
    char     *argv[];
{
    register int f;
    int       x, y;
    struct header hd;
    FILE     *fp;

    u_char  **alloc_2d_byte_array();
    void      init_list(), check_for_unneeded_points(), draw_output_image();
    void      write_vertex_list();
    ENDPTR    make_node();

    Progname = strsave(*argv);

    parse_args(argc, argv);

    read_header(&hd);
    if (hd.pixel_format != PFBYTE)
	perr(HE_MSG, "image pixel format must be byte");
    update_header(&hd, argc, argv);
    write_header(&hd);

    nrow = hd.orows;
    ncol = hd.ocols;

    init_list();

    image = alloc_2d_byte_array(nrow, ncol);
    out_image = alloc_2d_byte_array(nrow, ncol);

    if (fname != NULL) {
	if ((fp = fopen(fname, "w")) == NULL) {
	    fprintf(stderr, " Error opening output file %s \n", fname);
	    exit(-1);
	}
    }
    for (f = 0; f < hd.num_frame; f++) {
#ifdef OLD
	read_2d_byte_array(stdin, image, nrow, ncol);
#else
	hd.image = image[0];
	if (read_image(hd, f) == HIPS_ERROR)
	    return (HIPS_ERROR);
#endif

	x = y = 0;
	while (x >= 0) {

	    find_endpoint(&x, &y);

	    if (x >= 0) {
#ifdef DEBUG
		fprintf(stderr, " starting at point: %d, %d \n", x, y);
#endif

		init_list();
		list_head = make_node(x, y);

		follow_line(list_head, x, y);

		check_for_unneeded_points(list_head);

		draw_output_image(list_head);

		/* write out line segment list */
		if (fname != NULL)
		    write_vertex_list(fp, list_head);
	    }
	}

#ifdef OLD
	write_2d_byte_array(stdout, out_image, nrow, ncol);
#else
	hd.image = out_image[0];
	if (write_image(hd, f) == HIPS_ERROR)
	    return (HIPS_ERROR);
#endif
    }
    fprintf(stderr, "%s done. \n\n", argv[0]);
    return (0);
}

/*******************************************************/
find_endpoint(x, y)		/* find an end of a line in the image */
    int      *x, *y;
{
    register int i, j;

    for (i = 0; i < nrow; i++)
	for (j = 0; j < ncol; j++)
	    if (image[i][j] > 0) {
		if (count_neighbors(j, i) == 1) {	/* line end has only one
							 * neighbor */
		    *x = j;
		    *y = i;
		    return;
		}
	    }
    *x = *y = -1;		/* line end not found */
    return;
}

/********************************************************/

follow_line(tail, x, y)
    ENDPTR    tail;
    int       x, y;

/* this routine looks in a square 2 pixels away from the current
 * location, and continues the fill from the first pixel it finds.
 * If there are more than one line starting from within that square,
 * the other lines will be located during a later pass through the image. */
{
    int       x1, y1, x2, y2;
    int       i;
    static int lcnt = 0;
    ENDPTR    make_node();
    void      add_node();

    if ((x <= 1) || (x >= ncol - 1) || (y <= 1) || (y >= nrow - 1)) {
	image[y][x] = 0;
	return;			/* ignore points near the edges if the image */
    }
    lcnt += 2;

    if (lcnt > seg_size) {
	lcnt = 0;
#ifdef DEBUG
	fprintf(stderr, " point at: %d, %d \n", x, y);
#endif
	add_node(tail, make_node(x, y));
	tail = tail->next;
    }
    /* zero region around the pixel */
    image[y][x] = 0;
    image[y][x - 1] = image[y][x + 1] = image[y - 1][x] = image[y + 1][x] = 0;
    image[y - 1][x - 1] = image[y + 1][x + 1] = 0;
    image[y - 1][x + 1] = image[y + 1][x - 1] = 0;

    x1 = x - 2;
    x2 = x + 2;
    y1 = y - 2;
    y2 = y + 2;

    for (i = y1; i <= y2; i++) {
	if (image[i][x1] > 0) {
	    follow_line(tail, x1, i);
	    return;
	}
	if (image[i][x2] > 0) {
	    follow_line(tail, x2, i);
	    return;
	}
    }

    for (i = x1; i <= x2; i++) {
	if (image[y1][i] > 0) {
	    follow_line(tail, i, y1);
	    return;
	}
	if (image[y2][i] > 0) {
	    follow_line(tail, i, y2);
	    return;
	}
    }


    if (lcnt > 1) {		/* if any left */
#ifdef DEBUG
	fprintf(stderr, "extra point at: %d, %d \n", x, y);
#endif
	add_node(tail, make_node(x, y));
	tail = tail->next;
    }
}


/********************************************************************/
void
check_for_unneeded_points(head)
    ENDPTR    head;
{

    void      delete_node();
    ENDPTR    curr;
    int       x1, x2, y1, y2, x, y;
    float     sl1, sl2, a1, a2, a1_deg, a2_deg, diff, val;
    int       calc_s1 = 1;	/* calculate slope of first line segment flag */

#ifdef DEBUG
    fprintf(stderr, " removing unneeding points \n");
#endif

    curr = head->next;

    if (curr == NULL || curr->next == NULL || curr->prev == NULL)
	return;

    while (curr->next != NULL) {

	x1 = curr->prev->x;
	x2 = curr->next->x;
	y1 = curr->prev->y;
	y2 = curr->next->y;

	x = curr->x;
	y = curr->y;

	if (calc_s1) {
	    if (x1 != x) {
		sl1 = (float) (y1 - y) / (float) (x1 - x);
		a1 = atan(sl1);
		a1_deg = (180.0 / M_PI) * a1;
	    } else
		a1_deg = 90.0;
	}
	if (x2 != x) {
	    sl2 = (float) (y2 - y) / (float) (x2 - x);
	    a2 = atan(sl2);
	    a2_deg = (180.0 / M_PI) * a2;
	} else
	    a2_deg = 90.0;

	diff = fabs((double) (a2_deg - a1_deg));

	if (diff < angle_diff) {
	    delete_node(curr);
	    calc_s1 = 0;	/* use slope of first line next time */
#ifdef DEBUG
	    fprintf(stderr, "deleting node: %d, %d \n", x, y);
#endif
	} else
	    calc_s1 = 1;

	curr = curr->next;
    }
}

/********************************************************************/
void
draw_output_image(head)
    ENDPTR    head;
{

    void      line_fill();
    ENDPTR    curr;
    int       x1, x2, y1, y2;

#ifdef DEBUG
    fprintf(stderr, " writing lines into out_image buffer\n");
#endif

    curr = head->next;

    if (curr == NULL || curr->prev == NULL)
	return;

    while (curr != NULL) {
	x1 = curr->prev->x;
	y1 = curr->prev->y;
	x2 = curr->x;
	y2 = curr->y;

	line_fill(out_image, x1, y1, x2, y2);
	out_image[y1][x1] = PVAL;
	out_image[y2][x2] = PVAL;

	curr = curr->next;
    }

}

/********************************************************************/
void
write_vertex_list(fp, head)
    FILE     *fp;
    ENDPTR    head;
{

    ENDPTR    curr;
    int       x1, x2, y1, y2;
    double    length, theta_rad, theta_deg;
    static int cnt = 0;

#ifdef DEBUG
    fprintf(stderr, " writing line info into file \n");
#endif

    curr = head->next;

    if (curr == NULL || curr->prev == NULL)
	return;

    cnt++;
    if (long_report)
	fprintf(fp, "\n\n line # %d \n\n", cnt);

    while (curr != NULL) {
	x1 = curr->prev->x;
	y1 = curr->prev->y;
	x2 = curr->x;
	y2 = curr->y;

	length = sqrt(pow((double) (x2 - x1), 2.0) +
		      pow((double) (y2 - y1), 2.0));

	if (length > 0.0) {

	    if (x1 != x2) {
		/* angle = arctan of slope */
		theta_rad = atan((double) (y2 - y1) / (double) (x2 - x1));
#ifdef NEED
		if (theta_rad <= 0)	/* force positive angle */
		    theta_rad = (2 * M_PI) + theta_rad;
#endif
		theta_deg = (180.0 / M_PI) * theta_rad;
	    } else {
		theta_rad = M_PI / 2.0;
		theta_deg = 90.0;
	    }

	    if (long_report)
		fprintf(fp, "%4d,%4d  to  %4d,%4d   length = %4.4f   angle = %4.4f rad, %4.4f deg\n",
			x1, y1, x2, y2, length, theta_rad, theta_deg);

	    else		/* format for use with gnu_plot */
		fprintf(fp, " %f %f \n", theta_rad, length);

	}			/* length > 0.0 */
	curr = curr->next;
    }

}

/*********************************************************/
parse_args(argc, argv)
    int       argc;
    char     *argv[];
{
    seg_size = SEG_SIZE;
    angle_diff = ANGLE_DIFF;
    long_report = 0;
    fname = NULL;

    /* Interpret options  */
    while (--argc > 0 && (*++argv)[0] == '-') {
	char     *s;
	for (s = argv[0] + 1; *s; s++)
	    switch (*s) {
	    case 'l':
		if (argc < 2)
		    usageterm();
		sscanf(*++argv, "%d", &seg_size);
		argc--;
		break;
	    case 'a':
		if (argc < 2)
		    usageterm();
		sscanf(*++argv, "%f", &angle_diff);
		argc--;
		break;
	    case 'f':
		if (argc < 2)
		    usageterm();
		fname = *++argv;
		argc--;
		break;
	    case 'F':
		if (argc < 2)
		    usageterm();
		fname = *++argv;
		long_report++;
		argc--;
		break;
	    case 'h':
		usageterm();
		break;
	    default:
		usageterm();
		break;
	    }
    }				/* while */

    fprintf(stderr, "Minimum line segment length: %d \n", seg_size);
    fprintf(stderr, "Combining lines with angle difference < %.2f degrees\n",
	    angle_diff);

}

/*********************************************************************/

usageterm()
{
    fprintf(stderr, "Usage: linesegs [-l N][-a R][-f[F] fname] < inseq > outseq \n ");
    fprintf(stderr, " Options: [-l N]: minimum line segment size N ( default = %d) \n", SEG_SIZE);
    fprintf(stderr, "           [-a R]: combine lines if difference in angles < R deg. (default = %.1f)\n", ANGLE_DIFF);
    fprintf(stderr, "           [-f line file name]: create file with angles and lengths of lines\n");
    fprintf(stderr, "           [-F line file name]: file with headers and line endpoint locations \n\n");
    exit(0);
}


/*********************************************************************/
/*********************************************************************/
/*********************************************************************/


/*
 *  routines for handling the linked list
 */

/********************************************************/

ENDPTR
make_node(x, y)
    int       x, y;
{
    ENDPTR    nd;

    nd = (ENDPTR) malloc(sizeof(ENDPT));

    nd->x = x;
    nd->y = y;
    nd->next = nd->prev = NULL;

    return (nd);
}

/**************************************************/
void
add_node(n1, n2)		/* insert node n2 after n1 */
    ENDPTR    n1, n2;
{
    if (n1 == NULL) {
	n2 = list_head;
	return;
    }
    n2->next = n1->next;
    if (n1->next != NULL)	/* if not end of list */
	n1->next->prev = n2;
    n1->next = n2;
    n2->prev = n1;

}

/***************************************************/
void
delete_node(nd)			/* remove node from the linked list */
    ENDPTR    nd;
{
    if (nd == list_head) {
	list_head = nd->next;
	if (!list_is_empty())
	    list_head->prev = NULL;
    } else {
	nd->prev->next = nd->next;
	if (nd->next != NULL)	/* if not end of list */
	    nd->next->prev = nd->prev;
    }
}

/************************/
int
list_is_empty()
{
    if (list_head == NULL)
	return (1);		/* true */
    else
	return (0);		/* false */

}

 /****************************/
void
init_list()
{
    list_head = NULL;
}

/*********************************************************************/
/*********************************************************************/
/*********************************************************************/


/* utility routines
 */

/***************************************************************/
int
count_neighbors(c, r)
    register int c, r;		/* max value returned is 8 */
{
    int       neighbors = 0, tx, ty;

    tx = ncol - 1;
    ty = nrow - 1;

    if (r > 0)
	if (image[r - 1][c] > 0)
	    neighbors++;
    if (r < ty)
	if (image[r + 1][c] > 0)
	    neighbors++;

    if (c > 0)
	if (image[r][c - 1] > 0)
	    neighbors++;
    if (c < tx)
	if (image[r][c + 1] > 0)
	    neighbors++;

    if (r < ty && c < tx)
	if (image[r + 1][c + 1] > 0)
	    neighbors++;

    if (r > 0 && c > 0)
	if (image[r - 1][c - 1] > 0)
	    neighbors++;

    if (r > 0 && c < tx)
	if (image[r - 1][c + 1] > 0)
	    neighbors++;

    if (r < ty && c > 0)
	if (image[r + 1][c - 1] > 0)
	    neighbors++;

    return (neighbors);
}

/********************************************************************/
void
line_fill(buf, x1, y1, x2, y2)	/* Bresenhams's scan conversion algorithm */
    u_char  **buf;
    int       x1, y1, x2, y2;
/* this code adapted from:   Digital Line Drawing
 *                           by Paul Heckbert
 * from "Graphics Gems", Academic Press, 1990
 */
{
    int       d, x, y, ax, ay, sx, sy, dx, dy;

/* absolute value of a */
#ifndef ABS
#define ABS(a)          (((a)<0) ? -(a) : (a))
#endif

/* take binary sign of a, either -1, or 1 if >= 0 */
#define SGN(a)          (((a)<0) ? -1 : 1)

    if (x1 == x2 && y1 == y2) {
	/* single point, don 't need to scan convert */
	buf[y1][x1] = PVAL2;
	return;
    }
    dx = x2 - x1;
    ax = ABS(dx) << 1;
    sx = SGN(dx);

    dy = y2 - y1;
    ay = ABS(dy) << 1;
    sy = SGN(dy);

    x = x1;
    y = y1;
    if (ax > ay) {		/* x dominant */
	d = ay - (ax >> 1);
	for (;;) {
	    buf[y][x] = PVAL2;
	    if (x == x2)
		return;
	    if (d >= 0) {
		y += sy;
		d -= ax;
	    }
	    x += sx;
	    d += ay;
	}
    } else {			/* y dominant */
	d = ax - (ay >> 1);
	for (;;) {
	    buf[y][x] = PVAL2;
	    if (y == y2)
		return;
	    if (d >= 0) {
		x += sx;
		d -= ay;
	    }
	    y += sy;
	    d += ax;
	}
    }
}
