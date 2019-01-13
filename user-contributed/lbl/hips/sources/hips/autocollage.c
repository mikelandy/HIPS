
/* autocollage.c                                 Brian Tierney, LBL   4/90
 *
 *   usage:   autocollage [-b NN] [-o NN NN] < imseq > outimage
 *
 *  creates a single frame from a sequence of images
 *
 *  Works with data types: Byte only
 *
 *  to compile: cc  -o autocollage autocollage.c -lhipsh -lhips -lm -lcary
 */

/*   Author:  Brian L. Tierney
 *            Lawrence Berkeley Laboratory
 *            Imaging Technologies Group
 *            email: bltierney@lbl.gov
*/

#include <stdio.h>
#include <sys/types.h>

#include <hipl_format.h>

#define Calloc(a,b) (b *)calloc((unsigned)(a), sizeof(b))

#define MAX_COLS 2048		/* size for kodak printer: 8 1/2 x 11 */
#define MAX_ROWS 1536

#define GAP 50			/* default number of pixels between the
				 * images */

#define DEBUG

void      usageterm();

/* command line args */
int       bgval, outx, outy, hgap, vgap;

/******************************************************/
main(argc, argv)
    int       argc;
    char    **argv;
{
    struct header hd;		/* hips header */
    int       nframe, nrow, ncol, startx, starty, rw_size;
    int       pic_across, pic_down, out_rows, out_cols, rcnt;
    register int fr, i, j;
    FILE     *fp;
    u_char   *image, *iptr;
    u_char  **obuf;

    u_char  **alloc_2d_byte_array();

    Progname = strsave(*argv);
    parse_args(argc, argv);

    fp = hfopenr("<stdin>");
    fread_header(fp, &hd, "<stdin>");
    if (hd.pixel_format != PFBYTE) {
	fprintf(stderr, " input must be type byte \n");
	exit(0);
    }
    nframe = hd.num_frame;
    ncol = hd.ocols;
    nrow = hd.orows;
    fprintf(stderr, "input file is %d frames of size %d x %d \n",
	    nframe, ncol, nrow);

    pic_across = outx / (ncol + hgap);
    pic_down = outy / (nrow + vgap);


    if (pic_across * pic_down < nframe) {
	fprintf(stderr, " Error: too many frame in the sequence \n\n");
	exit(0);
    }
/*    while (pic_across * (pic_down + 1) > nframe)
	pic_down--;
    pic_down++; */
    pic_down = (nframe + pic_across-1)/pic_across;

    out_cols = (pic_across * (ncol + hgap)) + MIN(hgap, vgap);
    out_cols = ((out_cols / 8) + 1) * 8;	/* round up to multile of 8 */

    out_rows = (pic_down * (nrow + vgap));
    out_rows = ((out_rows / 8) + 1) * 8;

    fprintf(stderr, "Creating an image of size %dx%d which has  \n",
	    out_cols, out_rows);
    fprintf(stderr, " %d images across and %d images down; \n",
	    pic_across, pic_down);
    fprintf(stderr, "Space between images: %d pixels horizontal and %d vertical \n", hgap, vgap);


#ifdef OLD
    image = (u_char *) halloc(nrow * ncol, sizeof(u_char));
#else
    if (alloc_image(&hd) == HIPS_ERROR)
	return (HIPS_ERROR);
#endif
    obuf = alloc_2d_byte_array(out_rows, out_cols);

    /* initialize to bgval */
    for (i = 0; i < out_rows; i++)
	for (j = 0; j < out_cols; j++)
	    obuf[i][j] = bgval;


    rw_size = nrow * ncol * sizeof(u_char);
    startx = starty = MIN(hgap, vgap);
    rcnt = 0;

    for (fr = 0; fr < nframe; fr++) {

#ifdef DEBUG
	fprintf(stderr, "frame %d: at location %d, %d \n", fr, startx, starty);
#endif
#ifdef OLD
	if ((fread(image, rw_size, 1, fp)) != 1)
	    perr("error during read");
	iptr = image;
#else
	if (read_image(&hd, fr) == HIPS_ERROR)
	    return (HIPS_ERROR);
	iptr = hd.image;
#endif

	for (i = starty; i < starty + nrow; i++)
	    for (j = startx; j < startx + ncol; j++)
		obuf[i][j] = *iptr++;

	rcnt++;
	if (rcnt < pic_across) {
	    startx = j + hgap;
	} else {
	    rcnt = 0;
	    starty = i + vgap;
	    startx = MIN(hgap, vgap);
	}
    }

    hd.num_frame = 1;
    hd.ocols = hd.cols = out_cols;
    hd.orows = hd.rows = out_rows;
#ifndef OLD
    hd.sizeimage = hd.numpix = hd.ocols * hd.orows;
#endif
    update_header(&hd, argc, argv);
    write_header(&hd);
    
    fprintf(stderr, " writing new collaged image... \n");
#ifdef OLD
    write_2d_byte_array(stdout, obuf, out_rows, out_cols);
#else
    hd.image = obuf[0];
    if (write_image(&hd, 1) == HIPS_ERROR)
	return (HIPS_ERROR);
#endif

    fprintf(stderr, "autocollage done. \n");
    return (0);
}

/****************************************************************/

parse_args(argc, argv)
    int       argc;
    char     *argv[];
{

    /* set defaults */
    bgval = 255;		/* white */
    outx = MAX_COLS;
    outy = MAX_ROWS;
    hgap = vgap = GAP;

    /* Interpret options  */
    while (--argc > 0 && (*++argv)[0] == '-') {
	char     *s;
	for (s = argv[0] + 1; *s; s++)
	    switch (*s) {
	    case 'b':
		if (argc < 2)
		    usageterm();
		sscanf(*++argv, "%d", &bgval);
		argc--;
		break;
	    case 'g':
		if (argc < 2)
		    usageterm();
		sscanf(*++argv, "%d", &hgap);
		argc--;
		break;
	    case 'G':
		if (argc < 2)
		    usageterm();
		sscanf(*++argv, "%d", &vgap);
		argc--;
		break;
	    case 'o':
		if (argc < 3)
		    usageterm();
		sscanf(*++argv, "%d", &outx);
		argc--;
		sscanf(*++argv, "%d", &outy);
		argc--;
		break;
	    case 'h':
		usageterm();
		break;
	    default:
		usageterm();
		break;
	    }
    }
}

/******************************************************/
void
usageterm()
{
    fprintf(stderr, "Usage: autocollage [-b NN] [-o NN NN] < inseq > outimage  \n");
    fprintf(stderr, "       [-b NN] gray value for background between images (default = 255) \n");
    fprintf(stderr, "       [-g NN] number of pixels between images (horizontal) (default = 50) \n");
    fprintf(stderr, "       [-G NN] number of pixels between images (vertical) (default = 50) \n");
    fprintf(stderr, "       [-o NN NN] maximum size of output image (default = 2048x1536) \n\n");
    exit(0);
}
