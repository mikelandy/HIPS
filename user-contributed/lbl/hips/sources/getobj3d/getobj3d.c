
/*
 *  getobj3d.c - 3d version of getobj
 *
 *  written 6/90 by Brian Tierney, Lawrence Berkeley Laboratory
 *
   Usage: getobj3d [-t NN][-b NN][-m NN][-g N][-c NN NN NN][-s]
              [-a][-o][-v][-f fname] < inseq > outseq

  Options: [-t NN]  Surface threshold value (default = 50)
           [-b NN]  background level (default = 0)
           [-m NN]  minimum size object to keep (default = 10)
           [-g N]  type of connectivity bridge: default = 0)
                  ( 0 = none, 1 = 2D, 2 = 3D weak, 3 = 3D strong )
           [-s]  computes stats on each object, creating file 'getobj.stat'
           [-a]  find all objects in the image
           [-o]  output a binary mask of selected objects.
           [-v]  verbose mode
           [-c NN NN NN] frame,row,col location of object
           [-f fname ] file with list of seed values
 */

#include "getobj.h"

/****************************************************************/
main(argc, argv)
    int       argc;
    char    **argv;
{
    int       i, num_seed_vals;
    int      *sx_list, *sy_list, *sz_list;
    struct header hd;		/* hips header */

    void      clear_background_grid(), parse_args(), init_grid();
    void      clear_background_image(), locate_surfaces(), show_slices();
    u_char ***alloc_3d_byte_array();
    u_short ***alloc_3d_short_array();
    u_int  ***alloc_3d_int_array();

    Progname = strsave(*argv);
    parse_args(argc, argv);

    read_header(&hd);
    if (hd.pixel_format != PFSHORT && hd.pixel_format != PFBYTE
	&& hd.pixel_format != PFINT)
	perr(HE_MSG,"image must be in int, short or byte format");

    ncol = hd.ocols;		/* x axis */
    nrow = hd.orows;		/* y axis */
    nframe = hd.num_frame;	/* z axis */
    nvoxels = ncol * nrow * nframe;  /* total number of voxels */

    pix_format = hd.pixel_format;	/* byte, short , or int */

    if (nframe == 1) {
	fprintf(stderr, "Error: only 1 frame, use getobj instead. \n\n");
	exit(-1);
    }
    if (stats) {
	if ((df = fopen("getobj.stats", "w")) == NULL) {
	    fprintf(stderr, "\n error opening stats output file \n\n");
	    exit(-1);
	}
    }
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
    if ((sz_list = Calloc(num_seed_vals, int)) == NULL)
	perror("calloc error");

    if (val_file) {		/* read seed location file */
	for (i = 0; i < num_seed_vals; i++) {
	    sx_list[i] = get_value(vf);
	    sy_list[i] = get_value(vf);
	    sz_list[i] = get_value(vf);
	}
    } else {
	sx_list[0] = isx;
	sy_list[0] = isy;
	sz_list[0] = isz;
    }

    for (i = 0; i < num_seed_vals; i++)	/* check if valid values */
	if (sx_list[i] > ncol || sy_list[i] > nrow || sz_list[i] > nframe) {
	    fprintf(stderr, "\n Invalid seed value!(%d,%d,%d) \n\n",
		    sx_list[i], sy_list[i], sz_list[i]);
	    exit(-1);
	}
    fprintf(stderr, "\n rows: %d,  cols: %d,  frames: %d \n", nrow, ncol, nframe);

    if (verbose)
	fprintf(stderr, "\n reading image... \n");

    grid = alloc_3d_byte_array(nframe, nrow, ncol);
    if (hd.pixel_format == PFSHORT) {
	sht_image = alloc_3d_short_array(nframe, nrow, ncol);
	read_3d_short_array(stdin, sht_image, nframe, nrow, ncol);
    } else if (hd.pixel_format == PFINT) {
	int_image = alloc_3d_int_array(nframe, nrow, ncol);
	read_3d_int_array(stdin, int_image, nframe, nrow, ncol);
    } else {
	image = alloc_3d_byte_array(nframe, nrow, ncol);
	read_3d_byte_array(stdin, image, nframe, nrow, ncol);
    }

/* #define DEBUG */
#ifdef DEBUG
    show_slices();
#endif

    if (verbose)
	fprintf(stderr, " initializing grid... \n");

    init_grid(0);

    if (verbose)
	fprintf(stderr, " locating objects... \n");

    stack_size = identify_objects(tval);	/* label objects '1' */
    alloc_stack(stack_size);	/* stack shouldn't be larger than the number
				 * of pixels found by identify_objects */

    locate_surfaces();		/* label edges of object '2' */

    if (find_all) {
	i = 1;
	while (get_object(i, 0, 0, 0) >= 0)	/* label grid with '3' */
	    i++;
    } else {
	for (i = 0; i < num_seed_vals; i++) {	/* process each object */
	    (void) get_object(i + 1, sx_list[i], sy_list[i], sz_list[i]);
	}
    }

    if (verbose)
	fprintf(stderr, "\n zeroing non-objects... ");

    clear_background_grid();	/* grid = 0 if grid != 3 */

    clear_background_image(bg);	/* set image to bg if grid != 3 */

    if (verbose)
	fprintf(stderr, "\n writing image... ");

    if (output_binary_mask) {
	init_header(&hd, "", "", nframe, "", nrow, ncol, PFBYTE,1, "");
	update_header(&hd, argc, argv);
	write_header(&hd);
	write_3d_byte_array(stdout, grid, nframe, nrow, ncol);
    } else {
	update_header(&hd, argc, argv);
	write_header(&hd);
	if (pix_format == PFSHORT)
	    write_3d_short_array(stdout, sht_image, nrow, ncol, nframe);
	else if (pix_format == PFINT)
	    write_3d_int_array(stdout, int_image, nframe, nrow, ncol);
	else
	    write_3d_byte_array(stdout, image, nframe, nrow, ncol);
    }

    if (pix_format == PFSHORT)
	free_3d_short_array(sht_image);
    else if (pix_format == PFINT)
	free_3d_int_array((int) int_image);
    else
	free_3d_byte_array(image);
    free_3d_byte_array(grid);
    free((char *) stack);

    fprintf(stderr, "\n\n finished. \n\n");
    return (0);
}

/************************************************************/
int
get_object(num, sx, sy, sz)	/* locates and labels objects */
    int       num;
    int       sx, sy, sz;
{
    int       rval;
    int       object_fill(), get_seed(), get_seed_guess();
    void      reset_grid(), check_object_size();

    if (sx == 0 && sy == 0 && sz == 0)
	rval = get_seed_guess(&sx, &sy, &sz);
    else
	rval = get_seed(&sx, &sy, &sz);

    if (rval < 0) {		/* didn't find an object */
	if (find_all) {
	    if (verbose)
		fprintf(stderr, "\n No more objects. \n");
	} else
	    fprintf(stderr, "\n Object not found. \n");
	return (-1);		/* give up */
    }
    pix_total = 0.;
    pcnt = 0L;
    sp = 0;			/* initialize stack pointer */
    push(-1, -1, -1);		/* null stack */

    check_object_size(sx, sy, sz);
    if (pcnt < min_size) {
	if (verbose)
	    fprintf(stderr, "   skipping object %d: location %d,%d,%d; size of %d pixels \n",
		    num, sx, sy, sz, pcnt);
	return (0);
    } else {
	reset_grid();
    }

    pcnt = object_fill(sx, sy, sz);	/* grid = 3 if desired object */

    if (verbose)
	fprintf(stderr, " Object: %d, location: (%d,%d,%d);  %d pixels \n",
		num, sx, sy, sz, pcnt);

    if (stats) {
	fprintf(stderr, "\n STATS: object %d at (%d,%d,%d) is %d pixels.",
		num, sx, sy, sz, pcnt);
	fprintf(df, "\n STATS: object at (%d,%d,%d) is %d pixels.",
		sx, sy, sz, pcnt);
	fprintf(stderr, "\n STATS: Total pixel intensity: %e,   log10: %f ",
		pix_total, log10((double) pix_total));
	fprintf(df, "\n STATS: Total pixel intensity:  %e,   log10: %f ",
		pix_total, log10((double) pix_total));
	fprintf(stderr, "\n STATS: average pixel value of this object is %.1f\n",
		(double) pix_total / (double) pcnt);
	fprintf(df, "\n STATS: average pixel value of this object is %.1f\n",
		(double) pix_total / (double) pcnt);
    }
    return (0);
}

/***************************************************************/
void
check_object_size(c, r, f)	/* recursive routine to check if an object is
				 * big enough */
    int       c, r, f;
 /*
  * Note: this routine doesnt count edge pixels, only pixels labeled '1'. It
  * would require extra memory to store the 1 and the 2's (edges), and just
  * counting the 1's is a good enough to get an idea of the objects size.
  */
{

    grid[f][r][c] = 9;
    push(c, r, f);		/* add location to the stack */

    pcnt++;
    if (pcnt >= min_size)
	return;

    if (c < (ncol - 1))
	if (grid[f][r][c + 1] == 1)
	    check_object_size(c + 1, r, f);

    if (c > 0)
	if (grid[f][r][c - 1] == 1)
	    check_object_size(c - 1, r, f);

    if (r < (nrow - 1))
	if (grid[f][r + 1][c] == 1)
	    check_object_size(c, r + 1, f);

    if (r > 0)
	if (grid[f][r - 1][c] == 1)
	    check_object_size(c, r - 1, f);

    if (f < (nframe - 1))
	if (grid[f + 1][r][c] == 1)
	    check_object_size(c, r, f + 1);

    if (f > 0)
	if (grid[f - 1][r][c] == 1)
	    check_object_size(c, r, f - 1);

    return;
}

/********************************************************/
void
reset_grid()
{				/* restore grid to original state before
				 * check_object_size routine */
    int       c, r, f;

    pop(&c, &r, &f);
    while (c >= 0) {
	grid[f][r][c] = 1;
	pop(&c, &r, &f);
    }
}

/***************************************************************/
void
sum_pixel(c, r, f)		/* used of stats flag is set */
    int       c, r, f;
{
    if (pix_format == PFSHORT)
	pix_total += (double) sht_image[f][r][c];
    else if (pix_format == PFINT)
	pix_total += (double) int_image[f][r][c];
    else
	pix_total += (double) image[f][r][c];
}

/***************************************************************/
void
show_slices()
{				/* for debugging  */
    register int c, r, f;

    for (f = 0; f < nframe; f++) {
	fprintf(stderr, "\n frame #: %d \n", f);
	for (r = 0; r < nrow; r++) {
	    for (c = 0; c < ncol; c++)
		fprintf(stderr, "%3d", image[f][r][c]);
	    fprintf(stderr, "\n");
	}
    }
}

/******************************************************/
static int
get_value(vfile)		/* read seed values from file */
    FILE     *vfile;
{
    int       val, tmp;

    if ((tmp = fscanf(vfile, "%d\n", &val)) == EOF) {
	fprintf(stderr, "Out of values in seed file!\n\n");
	exit(-1);
    } else if (tmp != 1) {
	fprintf(stderr, " Bad seed value file!\n\n");
	exit(-1); 
    }
    return (val);
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
    bg = isx = isy = isz = stats = verbose = find_all = 0;
    output_binary_mask = bridges = 0;
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
	    case 'g':
		if (argc < 2)
		    usageterm();
		sscanf(*++argv, "%d", &iv);
		bridges = iv;
		argc--;
		break;
	    case 'm':
		if (argc < 2)
		    usageterm();
		sscanf(*++argv, "%d", &iv);
		min_size = iv;
		argc--;
		break;
	    case 's':
		stats = 1;
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
		if (argc < 4)
		    usageterm();
		sscanf(*++argv, "%d", &iv);
		isx = iv;
		sscanf(*++argv, "%d", &iv);
		isy = iv;
		sscanf(*++argv, "%d", &iv);
		isz = iv;
		argc -= 3;
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
    fprintf(stderr, "Usage: getobj3d [-t NN][-b NN][-m NN][-g N][-c NN NN NN][-s][-a][-o][-v][-f fname] < inseq > outseq \n ");
    fprintf(stderr, " Options: [-t NN]  Surface threshold value (default = 50)\n");
    fprintf(stderr, "           [-b NN]  background level (default = 0)\n");
    fprintf(stderr, "           [-m NN]  minimum size object to keep (default = 10)\n");
    fprintf(stderr, "           [-g N]  type of connectivity bridge: default = 0) \n");
    fprintf(stderr, "                  ( 0 = none, 1 = 2D, 2 = 3D weak, 3 = 3D strong ) \n");
    fprintf(stderr, "           [-s]  computes stats on each object, creating file 'getobj.stat' \n");
    fprintf(stderr, "           [-a]  find all objects in the image \n");
    fprintf(stderr, "           [-o]  output a binary mask of selected objects. \n");
    fprintf(stderr, "           [-v]  verbose mode \n");
    fprintf(stderr, "           [-c NN NN NN] col,row,frame location of object \n");
    fprintf(stderr, "           [-f fname ] file with list of seed values \n\n");
    exit(0);
}
