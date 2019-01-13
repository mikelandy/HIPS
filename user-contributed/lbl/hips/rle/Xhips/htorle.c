/*
 * Altered by:	Fritz Renema
 * 		Advanced Development Projects Group
 * 		Lawrence Berkeley Laboratory
 *
 * reads a byte per pixel HIPS file (with a header)
 * from stdin and converts it to rle format.
 *
 *  updated to use new rle library routines, Brian Tierney,  7/91
 *  converted to HIPS2:  Brian Tierney, 9/91
 */

#include <stdio.h>

#include <rle.h>
#include <hipl_format.h>

/***************************************************************/

main(argc, argv)
    int       argc;
    char     *argv[];

{
    struct header hd;		/* HIPS header structure */
    char     *iinfo[5];		/* important information */
    char      cols[100], rows[100];	/* number of cols and rows */

    if (argc > 1)
	usage(argv[0]);

    read_header(&hd);
    if (hd.pixel_format != PFBYTE)
	fatal("%s: input must be byte/pixel format.\n", argv[0]);

/* initialize array of important information to be passed to htorle */

    sprintf(cols, "%d", hd.ocols);
    sprintf(rows, "%d", hd.orows);

    iinfo[0] = "htorle";
    iinfo[1] = cols;
    iinfo[2] = rows;
    iinfo[3] = "HIPS2";
    iinfo[4] = "";

    if (htorle(4, iinfo) < 0)
	fatal("Error trying to convert from HIPS to rle.\n");

    exit(0);
}

usage(prgrm)
    char     *prgrm;
{
    fatal("Usage: %s < infile.hips > outfile.rle", prgrm);
}

fatal(msg, ar1, ar2, ar3, ar4, ar5)
    char     *msg;
    int       ar1, ar2, ar3, ar4, ar5;
{
    fprintf(stderr, msg, ar1, ar2, ar3, ar4, ar5);
    exit(-1);
}

int
htorle(argc, argv)
    int       argc;
    char     *argv[];
{
/*
 * converted from:
 *    graytorle.c - Create an RLE image from gray pixels.
 *
 * Author:	Michael J. Banks
 * 		Computer Science Dept.
 * 		University of Utah
 * Date:	Wed Jun 22 1988
 * Copyright (c) 1988, University of Utah
 */

    int       xsize, ysize;	/* Image size. */
    int       hsize, hflag = 0;	/* Image header size. */
    int       aflag = 0;	/* Alpha channel flag. */
    int       oflag = 0;	/* Output file flag. */
    int       files;		/* Number of files. */
    char    **fname;		/* List of input file names. */
    char     *oname;		/* Output file name. */
    rle_pixel **outrow;		/* Output buffer. */
    register int row;
    char     *trash;

    if (!scanargs(argc, argv,
		  "% xsize!d ysize!d h%-hdrsize!d o%-outfile!s a%- files!*s",
		  &xsize, &ysize,
		  &hflag, &hsize,
		  &oflag, &oname,
		  &aflag,
		  &files, &fname))
	return (-1);

    /* Initialize the_hdr and allocate image row storage. */

    rle_dflt_hdr.xmax = xsize - 1;
    rle_dflt_hdr.ymax = ysize - 1;

    rle_dflt_hdr.ncolors = 1;
    rle_dflt_hdr.ncmap = 0;
    rle_dflt_hdr.cmap = NULL;
    rle_dflt_hdr.cmaplen = 0;

    rle_dflt_hdr.rle_file = stdout;

    rle_put_setup(&rle_dflt_hdr);

    if (rle_row_alloc(&rle_dflt_hdr, &outrow)) {
	fprintf(stderr, "rle row allocation failed!\n");
	return (-2);
    }
    for (row = 0; row < ysize; row++) {
	fread(outrow[0], 1, xsize, stdin);
	rle_putrow(outrow, xsize, &rle_dflt_hdr);
    }

    rle_puteof(&rle_dflt_hdr);

    return (1);
}
