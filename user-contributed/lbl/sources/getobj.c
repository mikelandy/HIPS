
/*
 *  getobj.c - Selects objects in an image which are larger than
 *      a given size, and sets the rest of the image to a
 *      given background value. This program works on byte, short, or
 *      int images.
 *
 *  written 9/89 by Brian Tierney, Lawrence Berkeley Laboratory
 *  modified 6/90 by Brian Tierney, Lawrence Berkeley Laboratory
 *
Usage: getobj [-t NN][-b NN][-m NN][-a][-o][-v][-c NN NN][-f fname] < in > out

  Options: [-t NN]  threshold value (default = 50)
           [-b NN]  background level (default = 0)
           [-m NN]  minimum size object to keep (default = 10)
           [-a]  find all objects in the image
           [-o]  output a binary mask of selected objects.
           [-v]  verbose mode
           [-c NN NN] row,col of object to be located
           [-f fname ] file with list of seed values

 *
 */

/*
This program selects an arbitrary number of distinct objects from an
image, and sets the remaining image to a uniform background value
(usually zero). This can be used when only certain objects are
of interest, and can be used to eliminate background noise
around a central object.  The program can also be helpful in that
by making the entire background a constant value, data
compression programs will perform better.
*/

/* algorithm:
   1. identify all the objects in the image based on a given threshold value.
   2. Using the given seed points, us a flood fill to locate all points
        in an object
   3. Set all unselected locations to zero
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
#include <math.h>
#include <hipl_format.h>

#define Calloc(x,y) (y *)calloc((unsigned)(x), sizeof(y))

u_char  **image;
u_short **sht_image;
u_int   **int_image;
u_char  **grid;

/* input image info */
int       nrow, ncol;		/* number of rows, columns in the image */
int       npixels;
int       pix_format;

/* for statistics */
long      pcnt;			/* count of number of pixels selected by
				 * object_fill  */
/* command line args */
int       tval, bg, verbose, find_all, output_binary_mask, min_size;
int       isx, isy;		/* input seed point */
int       val_file;		/* value file flag */
char     *valfile;		/* name of file of seed values */

/* used in stack to simulate recursion */
typedef struct s_item {
    short     i, j;
}         STACK_ITEM;
STACK_ITEM *stack;

int       sp;			/* global stack pointer */
int       stack_size;
static int get_value();
int identify_objects(),get_object(),count_neighbors();
void alloc_stack(),push(),pop(),free_2d_short_array(),free_2d_int_array();
void free_2d_byte_array(),show_slice();

/*****************************************************/
int main(argc, argv)
    int       argc;
    char    **argv;
{
    int       nf, fr;
    FILE     *vf;
    int       i, num_seed_vals;
    struct header hd;		/* hips header */
    int      *sx_list, *sy_list;/* list of seed locations */

    void      clear_background_grid();
    void      parse_args(), init_grid(), clear_background_image();
    u_char  **alloc_2d_byte_array();
    u_short **alloc_2d_short_array();
    u_int   **alloc_2d_int_array();

    Progname = strsave(*argv);

    parse_args(argc, argv);

    read_header(&hd);
    if (hd.pixel_format != PFSHORT && hd.pixel_format != PFBYTE
	&& hd.pixel_format != PFINT)
	perr(HE_MSG, "image must be in int, short or byte format");

    nrow = hd.orows;		/* y axis */
    ncol = hd.ocols;		/* x axis */
    nf = hd.num_frame;
    pix_format = hd.pixel_format;
    npixels = nrow * ncol;

    fprintf(stderr, "\n rows: %d,  cols: %d,  frames: %d \n", nrow, ncol, nf);

    if (val_file) {
	if ((vf = fopen(valfile, "r")) == NULL) {
	    fprintf(stderr, "\n error opening seed value file \n\n");
	    exit(-1);
	}
	num_seed_vals = get_value(vf);
    } else
	num_seed_vals = 1;

    if ((sx_list = Calloc(num_seed_vals, int)) == NULL)
	perror("calloc error");
    if ((sy_list = Calloc(num_seed_vals, int)) == NULL)
	perror("calloc error");

    if (val_file) {		/* read seed value file */
	for (i = 0; i < num_seed_vals; i++) {
	    sx_list[i] = get_value(vf);
	    sy_list[i] = get_value(vf);
	}
    } else {
	sx_list[0] = isx;
	sy_list[0] = isy;
    }

    for (i = 0; i < num_seed_vals; i++)	/* check if valid values */
	if (sx_list[i] > ncol || sy_list[i] > nrow) {
	    fprintf(stderr, "\n Invalid seed value!(%d,%d) \n\n",
		    sx_list[i], sy_list[i]);
	    exit(-1);
	}
    grid = alloc_2d_byte_array(nrow, ncol);
    if (hd.pixel_format == PFSHORT) {
	sht_image = alloc_2d_short_array(nrow, ncol);
    } else if (hd.pixel_format == PFINT) {
	int_image = alloc_2d_int_array(nrow, ncol);
    } else {
	image = alloc_2d_byte_array(nrow, ncol);
    }

    if (output_binary_mask) {
	init_header(&hd, "", "", nf, "", nrow, ncol, PFBYTE, 1, "");
	update_header(&hd, argc, argv);
	write_header(&hd);
    } else {
	update_header(&hd, argc, argv);
	write_header(&hd);
    }

    for (fr = 0; fr < nf; fr++) {	/* process each frame in the sequence */

	if (nf > 1)
	    fprintf(stderr, "\n processing frame: %d ", fr);

	init_grid(0);		/* initialize grid to all zeros */

	if (verbose)
	    fprintf(stderr, "\n loading image... \n ");

#ifdef OLD
	if (pix_format == PFSHORT)
	    read_2d_short_array(stdin, sht_image, nrow, ncol);
	else if (pix_format == PFINT)
	    read_2d_int_array(stdin, int_image, nrow, ncol);
	else
	    read_2d_byte_array(stdin, image, nrow, ncol);
#else
	if (pix_format == PFSHORT)
	    hd.image = (byte *) sht_image;
	else if (pix_format == PFINT)
	    hd.image = (byte *) int_image;
	else
	    hd.image = (byte *) image;
	if (read_image(hd, fr) == HIPS_ERROR)
	    return (HIPS_ERROR);
#endif

	stack_size = identify_objects(tval);	/* # of pixels marked is a
						 * good upper limit for the
						 * stack */
	alloc_stack(stack_size);

	if (find_all) {
	    i = 1;
	    while (get_object(i, 0, 0) >= 0)
		i++;
	} else {
	    for (i = 0; i < num_seed_vals; i++) {	/* process each object */

		fprintf(stderr, "\n Looking for object at (%d,%d),  surface threshold value: %d...",
			sx_list[i], sy_list[i], tval);
		(void) get_object(i + 1, sx_list[i], sy_list[i]);
	    }
	}

	clear_background_grid();/* grid = 0 if grid != 3 */

	clear_background_image(bg);


	if (output_binary_mask) {
	    if (verbose)
		fprintf(stderr, " writing binary image... \n");
#ifdef OLD
	    write_2d_byte_array(stdout, grid, nrow, ncol);
#else
	    hd.image = grid[0];
	    if (write_image(hd, fr) == HIPS_ERROR)
		return (HIPS_ERROR);
#endif
	} else {
	    if (verbose)
		fprintf(stderr, " writing image... \n");
#ifdef DEBUG
	    show_slice(fr);
#endif

#ifdef OLD
	    if (pix_format == PFSHORT)
		write_2d_short_array(stdout, sht_image, nrow, ncol);
	    else if (pix_format == PFINT)
		write_2d_int_array(stdout, int_image, nrow, ncol);
	    else
		write_2d_byte_array(stdout, image, nrow, ncol);
#else
	    if (pix_format == PFSHORT) {
		hd.image = (byte *) sht_image;
		if (write_image(hd, fr) == HIPS_ERROR)
		    return (HIPS_ERROR);
	    } else if (pix_format == PFINT) {
		hd.image = (byte *) int_image;
		if (write_image(hd, fr) == HIPS_ERROR)
		    return (HIPS_ERROR);
	    } else {
		hd.image = (byte *) image;
		if (write_image(hd, fr) == HIPS_ERROR)
		    return (HIPS_ERROR);
	    }
#endif
	}

	free((char *) stack);
    }				/* for each frame  */

    if (pix_format == PFSHORT)
	free_2d_short_array(sht_image);
    else if (pix_format == PFINT)
	free_2d_int_array((int) int_image);
    else
	free_2d_byte_array(image);
    free_2d_byte_array(grid);

    fprintf(stderr, "\n\n finished. \n\n");
    return (0);
}

/************************************************************/
int
get_object(num, sx, sy)
    int       num;
    int       sx, sy;
{
    int       rval;
    int       object_fill(), get_seed(), get_seed_guess();
    void      reset_grid(), check_object_size();

    if (sx == 0 && sy == 0)
	rval = get_seed_guess(&sx, &sy);
    else
	rval = get_seed(&sx, &sy);

    if (rval < 0) {		/* didn't find an object */
	if (find_all) {
	    if (verbose)
		fprintf(stderr, "\n No more objects. \n");
	} else
	    fprintf(stderr, "\n Object not found. \n");
	return (-1);		/* give up */
    }
    pcnt = 0L;
    sp = 0;			/* initialize stack pointer */
    push(-1, -1);		/* null stack */
    check_object_size(sx, sy);
    if (pcnt < min_size) {
	if (verbose)
	    fprintf(stderr, "   skipping object %d: size of %ld pixels \n",
		    num, pcnt);
	return (0);
    } else {
	reset_grid();
    }

    pcnt = object_fill(sx, sy);	/* grid = -1 if desired object */
    if (verbose)
	fprintf(stderr, "\n Object: %d, location: (%d,%d);  %ld pixels \n",
		num, sx, sy, pcnt);

    return (0);
}

/***************************************************************/
void
check_object_size(c, r)		/* recursive routine to check if an object is
				 * big enough */
    int       c, r;
{

    grid[r][c] = 9;
    push(r, c);			/* add location to the stack */

    pcnt++;
    if (pcnt >= min_size)
	return;

    if (r < (nrow - 1))
	if (grid[r + 1][c] == 1)
	    check_object_size(c, r + 1);

    if (r > 0)
	if (grid[r - 1][c] == 1)
	    check_object_size(c, r - 1);

    if (c < (ncol - 1))
	if (grid[r][c + 1] == 1)
	    check_object_size(c + 1, r);

    if (c > 0)
	if (grid[r][c - 1] == 1)
	    check_object_size(c - 1, r);

    return;
}

/********************************************************/
void
reset_grid()
{				/* restore grid to original state before
				 * check_object_size routine */
    int       r, c;

    pop(&r, &c);
    while (r >= 0) {
	grid[r][c] = 1;
	pop(&r, &c);
    }
}


/*************************************************/
void
parse_args(argc, argv)
    int       argc;
    char     *argv[];
{
    int       iv;
    void      usageterm();

    /* set defaults */
    tval = 50;
    min_size = 10;
    bg = 0;
    isx = isy = verbose = find_all = 0;
    output_binary_mask = 0;
    valfile = NULL;

    /* Interpret options  */
    while (--argc > 0 && (*++argv)[0] == '-') {
	char     *s;
	for (s = argv[0] + 1; *s; s++)
	    switch (*s) {
	    case 't':
		if (argc < 2)
		    usageterm();
		sscanf(*++argv, "%d", &iv);
		tval = iv;
		argc--;
		break;
	    case 'b':
		if (argc < 2)
		    usageterm();
		sscanf(*++argv, "%d", &iv);
		bg = iv;
		argc--;
		break;
	    case 'm':
		if (argc < 2)
		    usageterm();
		sscanf(*++argv, "%d", &iv);
		min_size = iv;
		argc--;
		break;
	    case 'a':
		find_all = 1;
		fprintf(stderr, " Looking for all objects \n");
		break;
	    case 'o':
		output_binary_mask = 1;
		break;
	    case 'v':
		verbose = 1;
		break;
	    case 'c':
		if (argc < 3)
		    usageterm();
		sscanf(*++argv, "%d", &iv);
		isx = iv;
		sscanf(*++argv, "%d", &iv);
		isy = iv;
		argc -= 2;
		break;
	    case 'f':
		if (argc < 2)
		    usageterm();
		valfile = *++argv;
		val_file++;
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
}

/******************************************************/
void
usageterm()
{
    fprintf(stderr, "Usage: getobj [-t NN][-b NN][-m NN][-a][-o][-v][-c NN NN][-f fname] < inseq > outseq \n ");
    fprintf(stderr, " Options: [-t NN]  threshold value (default = 50)\n");
    fprintf(stderr, "           [-b NN]  background level (default = 0)\n");
    fprintf(stderr, "           [-m NN]  minimum size object to keep (default = 10)\n");
    fprintf(stderr, "           [-a]  find all objects in the image \n");
    fprintf(stderr, "           [-o]  output a binary mask of selected objects. \n");
    fprintf(stderr, "           [-v]  verbose mode \n");
    fprintf(stderr, "           [-c NN NN] row,col of object to be located \n");
    fprintf(stderr, "           [-f fname ] file with list of seed values \n\n");
    exit(0);
}

/******************************************************/
static int
get_value(vfile)
    FILE     *vfile;
{
    int       val, tmp;

    if ((tmp = fscanf(vfile, "%d\n", &val)) == EOF) {
	fprintf(stderr, "Out of values in seed file!\n\n");
	exit(-1);
    } else if (tmp != 1) {
	fprintf(stderr, "Bad seed value file!\n\n");
	exit(-1);
    }
    return (val);
}

/***************************************************************/

void show_slice(sn)			/* for debugging  */
    int       sn;
{
    register int c, r;

    fprintf(stderr, "\n frame #: %d \n", sn);

    for (r = 0; r < nrow; r++) {
	for (c = 0; c < ncol; c++)
	    fprintf(stderr, "%3d", image[r][c]);
	fprintf(stderr, "\n");
    }
}

/*********************************************************/
/*  routines for identifing objects using the grid       */
/*********************************************************/

void
init_grid(val)
    int       val;
 /* sets the entire grid to 'val' */
{
    register int i;
    register u_char *gptr;	/* pointer to 1D part of grid array */

    gptr = *grid;

    for (i = 0; i < npixels; i++)
	gptr[i] = val;

}

/***************************************************************/
void
clear_background_image(bg_val)
    int       bg_val;
 /* sets the image pixels to zero based on the grid value */

{
    register int i;
    register u_char *bptr, *gptr;	/* pointers to seqential part of 2D
					 * arrays */
    register u_short *sptr;
    register u_int *iptr;

    gptr = *grid;
    if (pix_format == PFBYTE)
	bptr = *image;
    else if (pix_format == PFSHORT)
	sptr = *sht_image;
    else
	iptr = *int_image;

    for (i = 0; i < npixels; i++)
	if (gptr[i] == 0) {
	    if (pix_format == PFBYTE)
		bptr[i] = (u_char) bg_val;
	    else if (pix_format == PFSHORT)
		sptr[i] = (u_short) bg_val;
	    else
		iptr[i] = bg_val;
	}
}

/***************************************************************/
void
clear_background_grid()
 /*
  * at this point, all points in the desired object should be -1, so set any
  * other points to 0
  */
{
    register int i;
    register u_char *gptr;

    gptr = *grid;

    for (i = 0; i < npixels; i++) {
	if (gptr[i] == 3)
	    gptr[i] = 255;
	else
	    gptr[i] = 0;
    }
}

/***************************************************************/
int
identify_objects(threshold_value)	/* 1 identifies any objects */
    int       threshold_value;
{
    register int r, c;
    int       num_on = 0, check_val;

    /* change all data to 0 or 1, depending on the threshold value */
    for (r = 0; r < nrow; r++)
	for (c = 0; c < ncol; c++) {
	    if (pix_format == PFSHORT)
		check_val = (int) sht_image[r][c];
	    else if (pix_format == PFINT)
		check_val = image[r][c];
	    else
		check_val = (int) image[r][c];

	    if (check_val < threshold_value)
		grid[r][c] = 0;
	    else {
		grid[r][c] = 1;
		num_on++;
	    }
	}
    return (num_on);
}

/***************************************************************/
int
get_seed(sx, sy)
    int      *sx, *sy;
/* looks for an object at the given seed point +-10 pixels:
 * Starts at given seed, first look up and right 10 pixels, then
 * looks down and left 10 pixels
*/
{
    register int r, c;

    int       bx, by;

    bx = *sx;
    by = *sy;

    /* look at 4 edge neighbors to determine if good seed */
    for (r = by; r < by + 10; r++)
	for (c = bx; c < bx + 10; c++)
	    if (grid[r][c] == 1) {
		if (count_neighbors(c, r) >= 3) {
		    *sx = c;
		    *sy = r;
		    return (0);
		}
	    }
    /* try again */
    for (r = by - 10; r < by; r++)
	for (c = bx - 10; c < bx; c++)
	    if (grid[r][c] == 1) {
		if (count_neighbors(c, r) >= 3) {
		    *sx = c;
		    *sy = r;
		    return (0);
		}
	    }
    /* if get thru without returning */

    return (-1);
}

/***************************************************************/

int
get_seed_guess(sx, sy)
    int      *sx, *sy;
{
/* returns a seed for an object larger than 8 pixels */
    register int r, c;

    /*
     * if the user does not specify a seed value, then this routine located
     * reasonable guesses for seeds. If the -a (find all objects) flag is
     * specified, it begins the search from location (1,1). Otherwise it
     * starts near the center of the data set, and if it doesn't find
     * anything, then looks from location (1,1).
     */

    static int bx = 0, by = 0;

    if (bx == 0 || by == 0) {
	if (find_all)
	    bx = by = 1;	/* start at upper corner */
	else {
	    bx = (ncol / 2) - 2;/* start near center of data set */
	    by = (nrow / 2) - 2;
	    if (bx < 1)
		bx = 1;
	    if (by < 1)
		by = 1;
	}
    }
    if (!find_all) {
	if (verbose) {
	    fprintf(stderr, "\n Looking for a seed value starting at: %d,%d \
n",
		    bx, by);
	}
	for (r = by; r < nrow - 1; r++)
	    for (c = bx; c < ncol - 1; c++)
		if (grid[r][c] == 1) {
		    if ((grid[r][c - 1] == 1) &&
			(grid[r][c + 1] == 1) &&
			(grid[r - 1][c] == 1) &&
			(grid[r + 1][c] == 1) &&
			(grid[r - 1][c - 1] == 1) &&
			(grid[r + 1][c + 1] == 1) &&
			(grid[r + 1][c - 1] == 1) &&
			(grid[r - 1][c + 1] == 1)) {
			*sx = c;
			*sy = r;
			return (0);
		    }
		}
	bx = by = 1;		/* didn't find a seed, so try again from
				 * upper corner */
    }
    if (verbose) {
	fprintf(stderr, "\n Looking for a seed value starting at: %d,%d \n",
		bx, by);
    }
    for (r = by; r < nrow - 1; r++)
	for (c = bx; c < ncol - 1; c++)
	    if (grid[r][c] == 1) {
		if ((grid[r][c - 1] == 1) &&
		    (grid[r - 1][c] == 1) &&
		    (grid[r][c + 1] == 1) &&
		    (grid[r + 1][c] == 1) &&
		    (grid[r - 1][c - 1] == 1) &&
		    (grid[r + 1][c + 1] == 1) &&
		    (grid[r + 1][c - 1] == 1) &&
		    (grid[r - 1][c + 1] == 1)) {
		    *sx = c;
		    *sy = r;
		    bx = c + 1;
		    by = r;
		    return (0);
		}
	    }
    /* if get thru without returning */
    if (verbose)
	fprintf(stderr, " object not found \n");

    return (-1);
}

/***************************************************************/
int
count_neighbors(c, r)
    register int c, r;		/* max value returned is 4 */
{
    /*
     * counts number of primary neighbors, used by 'get_seed'
     */

    int       neighbors = 0, tx, ty;

    tx = ncol - 1;
    ty = nrow - 1;

    if (r > 0)
	if (grid[r - 1][c] == 1)
	    neighbors++;
    if (r < ty)
	if (grid[r + 1][c] == 1)
	    neighbors++;

    if (c > 0)
	if (grid[r][c - 1] == 1)
	    neighbors++;
    if (c < tx)
	if (grid[r][c + 1] == 1)
	    neighbors++;

    return (neighbors);
}

/***************************************************************/
/* routines for handling stack-based recursion                 */
/***************************************************************/
int
object_fill(c, r)
    int       c, r;
{
    /* fill the objects with (3) */
    int       rr, cc, pt_count;

    pt_count = 0;
    sp = 0;			/* initialize stack pointer */
    push(-1, -1);		/* null stack */

    do {
      start:
	pt_count++;
	grid[r][c] = 3;		/* mark grid */

	rr = r + 1;
	if (rr < nrow && grid[rr][c] == 1) {
	    push(c, r);
	    r++;
	    goto start;
	}
	rr = r - 1;
	if (rr > 0 && grid[rr][c] == 1) {
	    push(c, r);
	    r--;
	    goto start;
	}
	cc = c + 1;
	if (cc < ncol && grid[r][cc] == 1) {
	    push(c, r);
	    c++;
	    goto start;
	}
	cc = c - 1;
	if (cc > 0 && grid[r][cc] == 1) {
	    push(c, r);
	    c--;
	    goto start;
	}
	pop(&c, &r);

    } while (r >= 0);		/* neg r indicates empty stack */

    if (sp != 0)
	fprintf(stderr, "Error: stack not empty \n");

    return (pt_count);
}

/***************************************************************/
void push(i, j)
    int       i, j;
{
    sp++;
    if (sp >= stack_size) {
	fprintf(stderr, "recursive stack overflow!! ");
	fprintf(stderr, " stack was allocated %d slots \n", stack_size);
	exit(-1);
    }
    stack[sp].i = i;
    stack[sp].j = j;
}

/***************************************************************/
void pop(i, j)
    int      *i, *j;
{
    *i = stack[sp].i;
    *j = stack[sp].j;
    sp--;
}

/***************************************************************/
void alloc_stack(st_size)	/* allocation of stack for non-recursive
				 * flood-fill alg */
    int       st_size;
{
    if ((stack = Calloc(st_size, STACK_ITEM)) == NULL)
	perror("calloc: stack");
}

/***************************************************************/
void
object_fill_recursive(c, r)	/* slower, so not used */
    int       c, r;
{
    /* fill the objects with (-1) */

    grid[r][c] = 3;
    pcnt++;
    if (pcnt > 80000) {
	fprintf(stderr, "\n Error: object too large, stack overflow. Try a higher surface value! \n\n");
	exit(-1);
    }
    if (r < (nrow - 1))
	if (grid[r + 1][c] == 1)
	    object_fill_recursive(c + 1, r);

    if (r > 0)
	if (grid[r - 1][c] == 1)
	    object_fill_recursive(c - 1, r);

    if (c < (ncol - 1))
	if (grid[r][c + 1] == 1)
	    object_fill_recursive(c, r + 1);

    if (c > 0)
	if (grid[r][c - 1] == 1)
	    object_fill_recursive(c, r - 1);

    return;
}
