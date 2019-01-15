
/* main.c  */

/* This program implements the Marching Cubes surface tiler described by
 * Lorensen & Cline in the Siggraph 87 Conference Proceedings, as well as
 * a version of their Dividing Cubes Algorithm.
 *
 * by Brian Tierney, LBL  12/90
 *            Lawrence Berkeley Laboratory
 *            Imaging Technologies Group
 *            email: bltierney@lbl.gov
 *
 *    see usage routine below for command line options
 *
 * Parts of this code from a program written by Mike Krogh, NCSA, Feb.  2, 1990
 * Portions developed at the National Center for Supercomputing Applications at
 * the University of Illinois at Urbana-Champaign.
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
#define MAIN

/* $Id: main.c,v 1.4 1992/01/31 02:05:45 tierney Exp $ */

/* $Log: main.c,v $
 * Revision 1.4  1992/01/31  02:05:45  tierney
 * y
 *
 * Revision 1.3  1992/01/30  20:05:03  davidr
 * prior to Brian's changes
 *
 * Revision 1.2  1991/12/19  01:42:20  davidr
 * added RCS identification markers
 * */

static char rcsid[] = "$Id: main.c,v 1.4 1992/01/31 02:05:45 tierney Exp $";

#include "isobuild.h"		/* define MAIN must come before this!! */

char      GEOM_TYPE[32];	/* used in program status messages */

/*****************************************************/
void
main(argc, argv)
    int       argc;
    char     *argv[];
{
    struct header hd;		/* HIPS file header */
    FILE     *fp1;
    int       parse_options();	/* parse the command line options */
    Data_type ***alloc_3d_data_array();
    NORMAL_VECT ***alloc_3d_normal_array();
    FLOAT_VECT **alloc_2d_vector_array();
    Grid_type ***alloc_3d_grid_array();
    CUBE_TRIANGLES **alloc_2d_cube_array();
    void      tessellate_volume(), tessellate_blocks(), compute_normal_array();
    void      get_max_min();
#ifdef SPEED
    struct timeb start_time, end_time;
    float     elapsed_time;
#endif

    Progname = strsave(*argv);
    strcpy(MY_NAME, argv[0]);

    if (parse_options(argc, argv) == -1)
	Error("error from parse_options");

    if (DIVIDING)		/* used in program status messages */
	strcpy(GEOM_TYPE, "points");
    else
	strcpy(GEOM_TYPE, "triangles");

    if (SERVER) {
	if (IN_MASK_FNAME != NULL && !DIVIDING) {
	    if ((in_mask_fp = fopen(IN_MASK_FNAME, "r")) == NULL)
		Error("Error opening mask file");
	}
	surfserv();
    } else {

	if (IN_FNAME != NULL) {	/* open input file */
	    if ((fp1 = fopen(IN_FNAME, "r")) == NULL)
		Error("Input data file not found");
	} else
	    fp1 = stdin;

	if (OUT_FNAME != NULL) {/* open output file */
	    if ((poly_fp = fopen(OUT_FNAME, "w")) == NULL)
		Error("opening outfile file");
	} else
	    poly_fp = stdout;

	if (get_size(fp1) < 0)	/* get size of input data (sets globals xdim,
				 * ydim, zdim  */
	    exit_handler();

	if (IN_MASK_FNAME != NULL && !DIVIDING) {
	    if ((in_mask_fp = fopen(IN_MASK_FNAME, "r")) == NULL)
		Error("Error opening mask file");
	}
	if (OUT_MASK_FNAME != NULL) {
	    if ((out_mask_fp = fopen(OUT_MASK_FNAME, "w")) == NULL)
		Error("Error opening mask file");
	    init_header(&hd, MY_NAME, "", zdim, "", ydim, xdim, PFBYTE, 1, "");
	    update_header(&hd, argc, argv);
	    fwrite_header(out_mask_fp, &hd, OUT_MASK_FNAME);
	}
	if (data == NULL)
	    data = alloc_3d_data_array(zdim, ydim, xdim);	/* alloc data array */

	if (PRE_NORMALS) {	/* pre-computing normals saves time later,
				 * but take LOTS of memory */
	    if (normals == NULL)
		normals = alloc_3d_normal_array(zdim, ydim, xdim);
	}
	if (DUP_CHECK && prevslice == NULL) {	/* for checking for dup verts */
	    prevslice = alloc_2d_cube_array(ydim, xdim);
	    currslice = alloc_2d_cube_array(ydim, xdim);
	}
	/* a separate grid array is used for segmenting the data */
	if (grid == NULL) {
	    grid = alloc_3d_grid_array(zdim, ydim, xdim);
	}
	if (DIVIDING && DO_CUBE_CENTER_NORMALS) {
	    gradient_curr_slice = alloc_2d_vector_array(ydim, xdim);
	    gradient_next_slice = alloc_2d_vector_array(ydim, xdim);
	}
	/* read in entire data set */
	if (read_hips_data(fp1, data, xdim * ydim * zdim) < 0)
	    exit_handler();

	if (IN_MASK_FNAME != NULL && !DIVIDING) {
	    /* read in mask data */
	    if (read_mask_file(xdim * ydim * zdim) < 0)
		exit_handler();
	}
	if (SKIP_BLOCKS)
	    num_blocks = block_setup();
	else {
	    get_max_min(xdim * ydim * zdim, &data_max, &data_min);
	    if (VERBOSE)
		fprintf(stderr, "Minimum data value: %.1f, Maximum data value: %.1f \n",
			(float) data_min, (float) data_max);
	}

	if (PRE_NORMALS)	/* compute normals for entire data set */
	    compute_normal_array();

#ifdef SPEED
	ftime(&start_time);
#endif
	if (SKIP_BLOCKS)
	    tessellate_blocks();
	else
	    tessellate_volume();

#ifdef SPEED
	ftime(&end_time);
	elapsed_time = ((((double) end_time.time * 1000.) +
			 (double) end_time.millitm) -
			(((double) start_time.time * 1000.) +
			 (double) start_time.millitm)) / 1000.;
	fprintf(stderr, "\nTime for tessellation: %f seconds\n", elapsed_time);
#endif

/* #define SHOW_MEM_SIZE */
#ifdef SHOW_MEM_SIZE
	get_mem_size();
#endif

	fclose(fp1);
	fclose(poly_fp);
	if (OUT_MASK_FNAME != NULL)
	    fclose(out_mask_fp);
	if (IN_MASK_FNAME != NULL)
	    fclose(in_mask_fp);

	Status("Done.");
	exit(0);
    }
}

/***************************************************************/
void
tessellate_volume()
{				/* tessellate entire volume */
    int       npoints = 0;
    int       num_slices;
    int      *num_voxels;
    register int slice;

    if (TVAL < data_min || TVAL > data_max) {
	fprintf(stderr, "Error: threshold value %.1f is not between %.2f and %.2f \n",
		(float) TVAL, (float) data_min, (float) data_max);
	return;
    }
    if (SX > 0 || SY > 0 || SZ > 0 || EX < xdim || EY < ydim || EZ < zdim) {
	SMALLER_BOX = 1;	/* global flag indication using reduced
				 * volume */
	fprintf(stderr, "Using partial input data set: from %d,%d,%d to %d,%d,%d\n",
		SX, SY, SZ, EX, EY, EZ);
    }
    if (EX > xdim)		/* set-up for handling partial volumes */
	EX = xdim;
    if (EY > ydim)
	EY = ydim;
    if (EZ > zdim)
	EZ = zdim;

    num_voxels = Calloc(zdim, int);

    if (DUP_CHECK)
	init_dup_vertex_check_arrays();
    if (DIVIDING && DO_CUBE_CENTER_NORMALS) {
	memset(gradient_curr_slice[0], 0, sizeof(FLOAT_VECT) * xdim * ydim);
	memset(gradient_next_slice[0], 0, sizeof(FLOAT_VECT) * xdim * ydim);
    }
    if (SEG_METHOD == 4) {
	fprintf(stderr, "\nisobuild: Using mask file for segmentation (at isoval %.2f) ...\n", (float) TVAL);
    } else if (SEG_METHOD == 0) {
	fprintf(stderr, "\nisobuild: Segmenting data at isoval %.2f ...\n",
		(float) TVAL);
	for (slice = SZ; slice < EZ; slice++) {
	    num_voxels[slice] = seg_slice_thresh(TVAL, data[slice],
						 grid[slice], SX, SY,
						 EX, EY, slice);
	    if (SMOOTH_GRID)
		smooth_slice(slice);
	}
    } else {
	fprintf(stderr, "\nisobuild: Performing flood fill segmentation at isoval %.2f ...\n",
		(float) TVAL);
	flood_fill_segmentation(TVAL, BRIDGE_TYPE);
    }

    Status("Tessellating data...");

    num_slices = EZ - 1;	/* dont want to do last slice */
    for (slice = SZ; slice < num_slices; slice++) {
	if (VERBOSE2)
	    fprintf(stderr, "tessellating slice: %d \n", slice);
	if (slice % 10 == 0) {
	    fprintf(stderr, "Tessellating slice %d,  %d %s so far \n",
		    slice, npoints, GEOM_TYPE);
	}
	if (SEG_METHOD > 0 || num_voxels[slice] > 0) {
	    if (!DIVIDING)
		npoints += iso_march(SX, SY, EX, EY, slice);
	    else
		npoints += iso_divide(SX, SY, EX, EY, slice);
	}
    }

    if (out_mask_fp != NULL)
	write_mask_slices(zdim);

    if (!DIVIDING) {
	dump_polys(0);		/* make sure all vertices have been written */
    } else {
	dump_points(0);
    }
    fprintf(stderr, "\n%s: %d %s generated\n\n", MY_NAME, npoints, GEOM_TYPE);


    return;
}

/***************************************************************/
void
tessellate_blocks()
{				/* tessellate block by block */
    int       npoints = 0, prev_xloc;
    int       num_slices, skip_cnt = 0;
    BLOCK_INFO *binfo_ptr, *binfo_ptr2;
    register int i, slice, block, xloc, yloc;

    if (TVAL < data_min || TVAL > data_max) {
	fprintf(stderr, "Error: threshold value %.1f is not between %.2f and %.2f \n",
		(float) TVAL, (float) data_min, (float) data_max);
	return;
    }
    if (SX > 0 || SY > 0 || SZ > 0 || EX < xdim || EY < ydim || EZ < zdim) {
	fprintf(stderr, "Using partial input data set: from %d,%d,%d to %d,%d,%d\n",
		SX, SY, SZ, EX, EY, EZ);
    }
    if (EX > xdim)		/* set-up for handling partial volumes */
	EX = xdim;
    if (EY > ydim)
	EY = ydim;
    if (EZ > zdim)
	EZ = zdim;

    if (DUP_CHECK)
	init_dup_vertex_check_arrays();
    if (DIVIDING && DO_CUBE_CENTER_NORMALS) {
	memset(gradient_curr_slice[0], 0, sizeof(FLOAT_VECT) * xdim * ydim);
	memset(gradient_next_slice[0], 0, sizeof(FLOAT_VECT) * xdim * ydim);
    }
    if (SEG_METHOD == 1) {
	SEG_METHOD = 0;
	Status("Flood fill segmentation not allow with skip slocks option!");
    }
    fprintf(stderr, "\nisobuild: Segmenting data at isoval %.2f ...\n",
	    (float) TVAL);
    for (slice = SZ; slice < EZ; slice++) {
	/* initialize grid to 0 */
	memset((char *) grid[slice][0], 0, xdim * ydim * sizeof(Grid_type));
	for (block = 0; block < num_blocks; block++) {
	    binfo_ptr = &block_info_array[slice][block];
	    yloc = binfo_ptr->yloc;
	    xloc = binfo_ptr->xloc;
	    if (xloc > 0)
		prev_xloc = xloc - binfo_ptr->width;
	    else
		prev_xloc = 0;
	    if (prev_xloc >= SX && yloc >= SY &&
		prev_xloc <= EX && yloc <= EY) {

		if (binfo_ptr->max < TVAL)
		     /* do nothing, slice already set to 0 */ ;
		else if (binfo_ptr->min > TVAL) {

		    /* set grid block to 1 */
		    for (i = 0; i < binfo_ptr->height; i++) {
			memset((char *) &binfo_ptr->grid[yloc++][xloc], 1,
			       (binfo_ptr->width) * sizeof(Grid_type));
		    }
		} else
		    threshold_block(binfo_ptr, TVAL);
	    }			/* if check bounds */
	}
    }
    if (SMOOTH_GRID) {
	for (slice = SZ; slice < EZ; slice++)
	    smooth_slice(slice);
    }
    skip_cnt = 0;
    num_slices = EZ - 1;	/* dont want to do last slice */

    Status("Tessellating data...");

    for (slice = SZ; slice < num_slices; slice++) {
	if (slice % 10 == 0) {
	    fprintf(stderr, "Tessellating slice %d,  %d %s so far \n",
		    slice, npoints, GEOM_TYPE);
	}
	for (block = 0; block < num_blocks; block++) {
	    binfo_ptr = &block_info_array[slice][block];
	    binfo_ptr2 = &block_info_array[slice + 1][block];

	    yloc = binfo_ptr->yloc;
	    xloc = binfo_ptr->xloc;
	    if (xloc > 0)
		prev_xloc = xloc - binfo_ptr->width;
	    else
		prev_xloc = 0;
	    if (prev_xloc >= SX && yloc >= SY &&
		prev_xloc <= EX && yloc <= EY) {
		if (((binfo_ptr->max < TVAL) &&
		     (binfo_ptr2->max < TVAL)) ||
		    ((binfo_ptr->min > TVAL) &&
		     (binfo_ptr2->min > TVAL)))
		    skip_cnt++;
		else {
		    if (!DIVIDING)
			npoints += iso_march_block(binfo_ptr->xloc,
						   binfo_ptr->yloc, slice,
						   binfo_ptr->width,
						   binfo_ptr->height,
						   SX, SY, EX - 1, EY - 1);
		    else
			npoints += iso_divide_block(binfo_ptr->xloc,
						    binfo_ptr->yloc, slice,
						    binfo_ptr->width,
						    binfo_ptr->height,
						    binfo_ptr->grid,
						    binfo_ptr2->grid,
						    SX, SY, EX - 1, EY - 1);
		}
	    }
	}
    }
    if (VERBOSE)
	fprintf(stderr, "Skipped %d blocks out of %d \n", skip_cnt,
		num_blocks * num_slices);


    if (!DIVIDING)
	dump_polys(0);		/* make sure all vertices have been written */
    else
	dump_points(0);
    fprintf(stderr, "\n%s: %d %s generated\n", MY_NAME, npoints, GEOM_TYPE);

    return;
}

/****************************************************************/
 /*
  * this routine can be used to check the results of the segmentation by
  * creating a file containing the segmentation mask
  */
write_mask_slices(num_slices)
    int       num_slices;
{

    register int i, j, k;

    if (VERBOSE)
	Status("Writing mask file...");

    for (i = 0; i < num_slices; i++) {
	for (j = 0; j < ydim; j++)
	    for (k = 0; k < xdim; k++)
		if (grid[i][j][k] > 0)
		    grid[i][j][k] = 255;
		else
		    grid[i][j][k] = 0;

	if ((Fwrite(grid[i][0], sizeof(Grid_type), xdim * ydim,
		    out_mask_fp)) != xdim * ydim)
	    perror("error writing mask image");
    }
}

/**************************** parse_options ****************************/
int
parse_options(argc, argv)
    int       argc;
    char     *argv[];
{
    /* see isovis.h for default values */

    void      usage();
    int       val;

    /* Interpret options  */
    while (--argc > 0 && (*++argv)[0] == '-') {
	char     *s;
	for (s = argv[0] + 1; *s; s++)
	    switch (*s) {
	    case 'b':
		if (argc < 2)
		    usage();
		sscanf(*++argv, "%d", &BRIDGE_TYPE);
		argc--;
		break;
	    case 'd':
		DUP_CHECK = abs(DUP_CHECK - 1);
		break;
	    case 'D':
		SERVER = abs(SERVER - 1);
		break;
	    case 'E':
		if (argc < 4)
		    usage();
		sscanf(*++argv, "%d", &EX);
		sscanf(*++argv, "%d", &EY);
		sscanf(*++argv, "%d", &EZ);
		argc -= 3;
		break;
	    case 'f':
		SKIP_BLOCKS = abs(SKIP_BLOCKS - 1);
		break;
	    case 'F':
		SMOOTH_GRID = abs(SMOOTH_GRID - 1);
		break;
	    case 'g':
		MARCH_NORMALS = abs(MARCH_NORMALS - 1);
		break;
	    case 'h':
		usage();
		break;
	    case 'i':
		if (argc < 2)
		    usage();
		IN_FNAME = *++argv;
		fprintf(stderr, " using input data file: %s\n", IN_FNAME);
		argc--;
		break;
	    case 'm':
		if (argc < 2)
		    usage();
		IN_MASK_FNAME = *++argv;
		fprintf(stderr, " input segmentation mask file: %s\n",
			IN_MASK_FNAME);
		argc--;
		break;
	    case 'M':
		if (argc < 2)
		    usage();
		OUT_MASK_FNAME = *++argv;
		fprintf(stderr, " writing segmentation mask to file: %s\n",
			OUT_MASK_FNAME);
		argc--;
		break;
	    case 'n':
		PRE_NORMALS = abs(PRE_NORMALS - 1);
		break;
	    case 'N':
		DO_CUBE_CENTER_NORMALS = abs(DO_CUBE_CENTER_NORMALS - 1);
		break;
	    case 'o':
		if (argc < 2)
		    usage();
		OUT_FNAME = *++argv;
		fprintf(stderr, " using output data file: %s\n", OUT_FNAME);
		argc--;
		break;
	    case 'O':
		if (argc < 2)
		    usage();
		sscanf(*++argv, "%d", &OUTPUT_TYPE);
		argc--;
		break;
	    case 'p':
		DIVIDING = abs(DIVIDING - 1);
		break;
	    case 'P':
		if (argc < 2)
		    usage();
		sscanf(*++argv, "%d", &PORT_NUM);
		argc--;
		break;
	    case 's':
		if (argc < 2)
		    usage();
		sscanf(*++argv, "%d", &SEG_METHOD);
		argc--;
		break;
	    case 'S':
		if (argc < 4)
		    usage();
		sscanf(*++argv, "%d", &SX);
		sscanf(*++argv, "%d", &SY);
		sscanf(*++argv, "%d", &SZ);
		argc -= 3;
		break;
	    case 't':
		if (argc < 2)
		    usage();
		sscanf(*++argv, "%d", &val);
		TVAL = (Data_type) val;
		argc--;
		break;
	    case 'u':
		usage();
		break;
	    case 'v':
		VERBOSE = abs(VERBOSE - 1);
		break;
	    case 'V':
		VERBOSE2 = abs(VERBOSE2 - 1);
		if (VERBOSE2)
		    VERBOSE = 1;
		break;
	    default:
		usage();
		break;
	    }
    }				/* while */

    if (IN_FNAME == NULL && OUT_FNAME == NULL)
	SERVER = 1;

    if (!SERVER && (IN_FNAME == NULL || OUT_FNAME == NULL)) {
	fprintf(stderr, "\n ERROR: Input and Output file names must be specified. \n");
	usage();
    }
    if (PRE_NORMALS && !DIVIDING) {
	fprintf(stderr, "Pre-computing normals not allowed with marching cubes. \n");
	PRE_NORMALS = 0;	/* this option for dividing cubes only */
    }
    if (OUTPUT_TYPE > 4) {
	fprintf(stderr, "\n ERROR: Invalid output type. \n");
	usage();
    }
    if (OUTPUT_TYPE == 2) {
	fprintf(stderr, "Output file will be movie.BYU format. \n");
	DUP_CHECK = 1;		/* must produce edge list for movie.byu files */
	DIVIDING = 0;		/* must use marching cubes method */
	SERVER = 0;		/* movie.byu server not supported */
	MARCH_NORMALS = 0;	/* movie.byu doesn't handle normals */
	SMALL_CHUNKS = 0;	/* chunks not allowed in move.byu */
    }
    if (OUTPUT_TYPE == 3) {
	fprintf(stderr, "Output file will be wavefront format. \n");
	DUP_CHECK = 1;		/* must produce edge list for wavefront files */
	DIVIDING = 0;		/* must use marching cubes method */
	SERVER = 0;		/* wavefront server not supported */
	SMALL_CHUNKS = 0;	/* chunks not implemented in wavefront */
    }
    if (OUTPUT_TYPE == 4) {
	fprintf(stderr, "Output file will be sunvision.vff format. \n");
	DUP_CHECK = 1;
	DIVIDING = 0;
	SERVER = 0;
	SMALL_CHUNKS = 0;	/* chunks not allowed in sunvision.vff */
    }
    if (SEG_METHOD > 0 && SKIP_BLOCKS) {
	/* ERROR: Can't use skip-block option with flood fill segmentation */
	SKIP_BLOCKS = 0;
    }
    if (SEG_METHOD > 2) {
	fprintf(stderr, "\n ERROR: Invalid segmentation method. \n");
	usage();
    }
    if (IN_MASK_FNAME != NULL && !DIVIDING) {
	/* ERROR: Can't use skip-block option with input mask (no advantage) */
	SKIP_BLOCKS = 0;
	SMOOTH_GRID = 0;	/* not implemented with mask input */
	SEG_METHOD = 4;		/* no segmentation at all, just use mask */
    }
    if (!DIVIDING && IN_MASK_FNAME != NULL) {
	fprintf(stderr, "\n ERROR: Mask segmentation allowed with dividing cubes only. \n");
	SEG_METHOD = 0;
    }
    if (SEG_METHOD == 1) {
	fprintf(stderr, "Using flood fill segmentation method. \n");
    }
    if (!DIVIDING && DUP_CHECK)
	fprintf(stderr, "Checking for duplicate vertices. \n");

    if (!DIVIDING)
	fprintf(stderr, "Output will be triangle vertices. \n");
    else
	fprintf(stderr, "Output will be points with normals. \n");

    if (SX < 0 || SY < 0 || SZ < 0 || EX < SX || EY < SY || EY < SZ) {
	fprintf(stderr, "\n ERROR: Invalid starting or ending grid locations.\n");
	usage();
    }
    return 0;
}

/**************************** usage ****************************/

void
usage()
{
    fprintf(stderr, "\nusage: %s [-i 3d_data.hips][-o outfile][-O N][-t NN][-p][-s N][-N][-v] \n", MY_NAME);

    fprintf(stderr, "  -i file   HIPS file name (default = stdin) \n");
    fprintf(stderr, "  -o file   output file name \n");
    fprintf(stderr, "  -O N      output type (default = %d) \n", OUTPUT_TYPE);
    fprintf(stderr, "             ( 0=LBL, 1=ASCII, 2=byu, 3=wavefront, 4=sunvision (.vff) )\n");
    fprintf(stderr, "  -t NN     surface threshold value (default = %d) \n", (int) TVAL);
    if (DIVIDING)
	fprintf(stderr, "  -p        output points, not triangles (Dividing Cubes) (default = points) \n");
    else
	fprintf(stderr, "  -p        output points, not triangles (Dividing Cubes) (default = triangles) \n");
    fprintf(stderr, "  -s N      segmentation method   (default = %d) \n", SEG_METHOD);
    fprintf(stderr, "             ( 0 = thresholding, 1 = flood fill, 2 = flood with gradient ) \n");
    fprintf(stderr, "  -D        run as a server, if no file names, this is assumed \n");
    fprintf(stderr, "  -m file   HIPS file to use as a segmentation mask\n");


    fprintf(stderr, "\n other options:\n");
    fprintf(stderr, "  -v[V]     verbose mode(s) \n");
    if (MARCH_NORMALS)
	fprintf(stderr, "  -g        compute normals for gouraud shading. (default = yes) \n");
    else
	fprintf(stderr, "  -g        compute normals for gouraud shading. (default = no) \n");
    if (SMOOTH_GRID)
	fprintf(stderr, "  -F        smooth segmentation grid after threshold (default = yes)\n");
    else
	fprintf(stderr, "  -F        smooth segmentation grid after threshold (default = no)\n");
    fprintf(stderr, "  -P NN     server port number (default = %d)\n", PORT_NUM);
    fprintf(stderr, "  -b N      type of connectivity bridge: (default = %d) \n", BRIDGE_TYPE);
    fprintf(stderr, "             ( 0 = none, 1 = 2D, 2 = 3D weak, 3 = 3D strong ) \n");
    if (SKIP_BLOCKS)
	fprintf(stderr, "  -f        pre-pass to compute min/max of data blocks (default = prepass)\n");
    else
	fprintf(stderr, "  -f        pre-pass to compute min/max of data blocks (default = no prepass)\n");
    if (PRE_NORMALS)
	fprintf(stderr, "  -n        pre-compute all normals (default = yes) \n");
    else
	fprintf(stderr, "  -n        pre-compute all normals (default = no) \n");
    if (DO_CUBE_CENTER_NORMALS)
	fprintf(stderr, "  -N        fast, less accurate normals for dividing cubes (default = slow)\n");
    else
	fprintf(stderr, "  -N        fast, less accurate normals for dividing cubes (default = fast)\n");
    if (DUP_CHECK)
	fprintf(stderr, "  -d        check for duplicate triangle vertices (default = check) \n");
    else
	fprintf(stderr, "  -d        check for duplicate triangle vertices (default = no check) \n");
    fprintf(stderr, "  -M file   write segmentation mask to HIPS file\n");
    fprintf(stderr, "  -S NN NN NN starting x,y,z data grid location (default = 0,0,0)\n");
    fprintf(stderr, "  -E NN NN NN ending x,y,z data grid location (default = xmax, ymax, zmax)\n\n");
    exit(-1);
}
