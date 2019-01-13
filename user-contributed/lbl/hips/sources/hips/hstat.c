
/* hstat.c                                 Brian Tierney, LBL   3/90
 *
 *   using the binary image as a mask, this program displays the follow
 *    information for each object in the binary image.
 *
 *   object #, location,  # of pixels,  average pixel value, log of ave val
 *
 * currently only works for byte images
 *
 *  Usage: hstat -m mask_file [-l][-s][-p] <  image > output
 *   [-m mask_file]  filename of binary image mask (required)
 *   [-l] create script for labeling points
 *   [-s] output file is stripped (no labels)
 *   [-p] output file contains x,y locations of the object perimeters
 *
 *
 * the -s option can be used to list the object stats with no labels
 *   to be used as input to some other program.
 * the -l option can be used to create a c-shell script to call the
 *    'label' program to number each option with a label number.
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

#define Calloc(a,b) (b *)calloc((unsigned)(a), sizeof(b))

u_char  **image;
u_char  **bimage;
char    **grid;
int       nrow, ncol, nf;	/* number of rows, columns in the image */

long      size;			/* count of number of pixels in an object */
double    pix_total;		/* sum of pixel values in an object */
int       obj_cnt = 0;
int       perim_count = 0;	/* count of number of pixels in the perimeter */

/* global command line arguments */
int	make_script, perim_locations, stripped;
char	*mask_file;
void	stat_objects(), usageterm();

FILE	*fp2;

/******************************************************/
main(argc, argv)
    int       argc;
    char    **argv;
{
    int       fr;
    FILE     *fp;

    struct header hd, bhd;	/* hips header */

    void      write_script_head(), write_script_tail();
    u_char  **alloc_2d_byte_array();

    Progname = strsave(*argv);
    parse_args(argc, argv);

    read_header(&hd);
    if (hd.pixel_format != PFBYTE)
	perr(HE_MSG, "image must be in byte format");

    nrow = hd.rows;		/* y axis */
    ncol = hd.cols;		/* x axis */
    nf = hd.num_frame;
    fprintf(stderr, "\n rows: %d,  cols: %d,  frames: %d \n", nrow, ncol, nf);

    grid = (char **) alloc_2d_byte_array(hd.rows, hd.cols);

    if ((fp = fopen(mask_file, "r")) == NULL) {
	fprintf(stderr, "\n error opening binary image file \n\n");
	exit(-1);
    }
    if (make_script) {
	if ((fp2 = fopen("make_label.sh", "w")) == NULL) {
	    fprintf(stderr, "\n error opening shell script file \n\n");
	    exit(-1);
	}
	/* write starting commands for script file */
	write_script_head();
    }
    fread_header(fp, &bhd, mask_file);
    if (bhd.pixel_format != PFBYTE ||
	bhd.rows != hd.rows || bhd.cols != hd.cols)
	perr(HE_MSG, "binary mask image not correct format or size");

    image = alloc_2d_byte_array(hd.rows, hd.cols);
    bimage = alloc_2d_byte_array(hd.rows, hd.cols);

    for (fr = 0; fr < nf; fr++) {	/* process each frame in the sequence */

	if (nf > 1)
	    fprintf(stderr, "\n processing frame: %d ", fr);

	bzero(grid[0], nrow * ncol);

#ifdef OLD
	read_2d_byte_array(stdin, image, hd.rows, hd.cols);
	read_2d_byte_array(fp, bimage, hd.rows, hd.cols);
#else
	hd.image = image[0];
	if (read_image(hd, fr) == HIPS_ERROR)
	    return (HIPS_ERROR);
	bhd.image = bimage[0];
	if (fread_image(fp, bhd, fr, mask_file) == HIPS_ERROR)
	    return (HIPS_ERROR);
#endif

	stat_objects();

    }				/* for each frame  */

    fclose(fp);
    /* write ending commands for script file */
    if (make_script) {
	write_script_tail();
	fclose(fp2);
	fprintf(stderr, "\nTo execute script, type: csh make_label.sh in_file \n");
    }
    fprintf(stderr, "\n\n finished. \n\n");
    return (0);
}

/***************************************************************/
void
stat_objects()
{
    register int i, j;
    void      get_object();

    for (i = 0; i < nrow; i++)
	for (j = 0; j < ncol; j++)
	    if (bimage[i][j] > 0 && grid[i][j] == 0)
		get_object(i, j);
}

/**************************************************************/
void
get_object(i, j)
    int       i, j;
{
    double    p_ave;
    static int tc = 0;

    if (num_neighbors(i, j) < 3)
	return;			/* ignore very small objects */

    obj_cnt++;
    if (stripped)
	fprintf(stdout, "%d, (%d,%d) \n", obj_cnt, j, i);
    else
	fprintf(stdout, "object %d at location (x,y) : (%d,%d) \n",
		obj_cnt, j, i);

    size = 0L;
    pix_total = 0.;
    perim_count = 0;
    count_pixels(i, j);

    p_ave = pix_total / (double) size;

    if (stripped) {
	if (pix_total > 0)
	    fprintf(stdout, "%d, %.0f, %.3f, %.2f, %.3f, %d \n",
		    size, pix_total, log10(pix_total),
		    p_ave, log10(p_ave), perim_count);
	else
	    fprintf(stdout, "%d, %.0f, %.3f, %.2f, %.3f, %d \n",
		    size, 0, 0, 0, 0, perim_count);

    } else {
	fprintf(stdout, " size of object is %d pixels.\n", size);
	if (pix_total > 0) {
	    fprintf(stdout, " total pixel intensity: %.0f, (log10: %.3f)\n",
		    pix_total, log10(pix_total));
	    fprintf(stdout, " average pixel value: %.2f, (log10: %.3f)\n",
		    p_ave, log10(p_ave));
	    fprintf(stdout, " number of pixels in object perimeter: %d \n",
		    perim_count);
	} else {
	    fprintf(stdout, " total pixel intensity = 0 \n");
	    fprintf(stdout, " average pixel value = 0 \n");
	}
    }
    fprintf(stdout, "\n\n");

    if (make_script) {		/* write shell script file command */
	fprintf(fp2, " | label %d -a 6 -b 1 -x %d -y %d \\\n",
		obj_cnt, 2 * j, 2 * i);
	if ((obj_cnt % 5) == 0) {
	    tc++;
	    fprintf(fp2, " > temp%d \n", tc);
	    if (tc > 1)
		fprintf(fp2, "rm -f temp%d \n", tc - 1);
	    fprintf(fp2, "cat < temp%d \\\n", tc);
	}
    }
}

/*********************************************************************/

count_pixels(i, j)
    int       i, j;
{
    int       ii, jj;

    size++;
    pix_total += (double) image[i][j];
    grid[i][j] = 1;		/* mark is point as counted */

    if (num_neighbors(i, j) < 4) {	/* if has all 4 neigbors,not on
					 * perimeter */
	perim_count++;
	if (perim_locations) {
	    if (stripped)
		fprintf(stdout, " (%d,%d) \n", j, i);
	    else
		fprintf(stdout, " perimeter pixel: (%d,%d) \n", j, i);
	}
    }
    ii = i + 1;
    if (ii < nrow && grid[ii][j] == 0)
	if (bimage[ii][j] > 0)
	    count_pixels(ii, j);

    ii = i - 1;
    if (ii > 0 && grid[ii][j] == 0)
	if (bimage[ii][j] > 0)
	    count_pixels(ii, j);

    jj = j + 1;
    if (j + 1 < ncol && grid[i][jj] == 0)
	if (bimage[i][jj] > 0)
	    count_pixels(i, jj);

    jj = j - 1;
    if (jj > 0 && grid[i][jj] == 0)
	if (bimage[i][jj] > 0)
	    count_pixels(i, jj);

    /* also check diagonals */
    ii = i + 1;
    jj = j + 1;
    if (ii < nrow && jj < ncol && grid[ii][jj] == 0)
	if (bimage[ii][jj] > 0)
	    count_pixels(ii, jj);

    ii = i - 1;
    jj = j - 1;
    if (ii > 0 && jj > 0 && grid[ii][jj] == 0)
	if (bimage[ii][jj] > 0)
	    count_pixels(ii, jj);

    ii = i - 1;
    jj = j + 1;
    if (ii > 0 && j + 1 < ncol && grid[ii][jj] == 0)
	if (bimage[ii][jj] > 0)
	    count_pixels(ii, jj);

    ii = i + 1;
    jj = j - 1;
    if (i + 1 < nrow && jj > 0 && grid[ii][jj] == 0)
	if (bimage[ii][jj] > 0)
	    count_pixels(ii, jj);

}

/***********************************************************/
int
num_neighbors(i, j)
    int       i, j;
{
    int       cnt = 0;

    if (i > 0)
	if (bimage[i - 1][j] > 0)
	    cnt++;
    if (j > 0)
	if (bimage[i][j - 1] > 0)
	    cnt++;
    if (i < nrow - 1)
	if (bimage[i + 1][j] > 0)
	    cnt++;
    if (j < ncol - 1)
	if (bimage[i][j + 1] > 0)
	    cnt++;

/* don't need to check diagonals
    if (i > 0 && j > 0)
	if (bimage[i - 1][j - 1] > 0)
	    cnt++;
    if (i < nrow - 1 && j < ncol - 1)
	if (bimage[i + 1][j + 1] > 0)
	    cnt++;
    if (i < nrow - 1 && j > 0)
	if (bimage[i + 1][j - 1] > 0)
	    cnt++;
    if (i > 0 && j < ncol - 1)
	if (bimage[i - 1][j + 1] > 0)
	    cnt++;
*/
    return (cnt);
}

/****************************************************************/

parse_args(argc, argv)
    int       argc;
    char     *argv[];
{

    mask_file = NULL;
    make_script = perim_locations = stripped = 0;

    /* Interpret options  */
    while (--argc > 0 && (*++argv)[0] == '-') {
	char     *s;
	for (s = argv[0] + 1; *s; s++)
	    switch (*s) {
	    case 'm':
		if (argc < 2)
		    usageterm();
		mask_file = *++argv;
		fprintf(stderr, " using mask file: %s\n", mask_file);
		argc--;
		break;
	    case 'l':
		make_script++;
		break;
	    case 's':
		stripped++;
		break;
	    case 'p':
		perim_locations++;
		break;
	    case 'h':
		usageterm();
		break;
	    default:
		usageterm();
		break;
	    }
    }				/* while */

    if (mask_file == NULL)
	usageterm();
}

/******************************************************/
void
usageterm()
{
    fprintf(stderr, "Usage: hstat -m mask_file [-l][-s][-p] <  image > output  \n");
    fprintf(stderr, "   [-m mask_file]  filename of binary image mask (required) \n");
    fprintf(stderr, "   [-l] create script for labeling points \n");
    fprintf(stderr, "   [-s] output file is stripped (no labels) \n");
    fprintf(stderr, "   [-p] output file contains x,y locations of the object perimeters \n\n ");

    exit(0);
}

/******************************************************/
void
write_script_head()
{
    fprintf(fp2, "#! /bin/csh \n");
    fprintf(fp2, "# shell script file for adding labels to images \n");
    fprintf(fp2, "unset noclobber \n");
    fprintf(fp2, "unalias rm \n");
    fprintf(fp2, "if ($#argv == 0 ) then \n");
    fprintf(fp2, "        echo -n \" enter input file name: \" \n");
    fprintf(fp2, "        set inname = $< \n");
    fprintf(fp2, "        echo \" \" \n");
    fprintf(fp2, "else \n");
    fprintf(fp2, "        set inname = $argv[1] \n");
    fprintf(fp2, "endif \n");
    fprintf(fp2, "set outname = $inname.labeled \n");
    fprintf(fp2, "# check if output file already exists \n");
    fprintf(fp2, "if (-e $outname ) then \n");
    fprintf(fp2, "        echo -n \" output file \"$outname \"already exists. OK to remove (Y/N)\" \n");
    fprintf(fp2, "        set ans = $< \n");
    fprintf(fp2, "        if ($ans == \"y\" || $ans == \"Y\") then \n");
    fprintf(fp2, "                'rm' -f $outname \n");
    fprintf(fp2, "        else \n");
    fprintf(fp2, "                exit \n");
    fprintf(fp2, "        endif \n ");
    fprintf(fp2, "endif \n");
    fprintf(fp2, "echo \" \" \n");
    fprintf(fp2, "echo \" adding labels to file \"$inname \n");
    fprintf(fp2, "set echo \n");
    fprintf(fp2, "cat < $inname \\\n");
    fprintf(fp2, " | scale_geom -2 \\\n");
}

/******************************************************/
void
write_script_tail()
{
    /* fprintf(fp2, " | scale_geom -dc %d -dr %d \\\n", ncol, nrow ); */
    fprintf(fp2, " > $outname \n");
    fprintf(fp2, "rm -f temp* \n");
    fprintf(fp2, "echo \"done\" \n");
}
