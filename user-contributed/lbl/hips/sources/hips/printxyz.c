
/* printxyz.c                                 Brian Tierney, LBL   4/90
 *
 *   usage:   printxyz [outfile] < binary_image > outfile
 *
 *  to compile:  cc -O -o printxyz printxyz.c -lhips
 *
 *  this program print x,y,z coordinates of non-zero objects in
 *  a sequence of hips images.
 *
 * only works for byte images
 *
 * if the outfile name is present, output files are created for each
 *  frame in a sequence, otherwise output goes to stdout
 *
 */


#include <stdio.h>
#include <sys/types.h>

#include <hipl_format.h>

#define Calloc(a,b) (b *)calloc((unsigned)(a), sizeof(b))
#define Fread(a,b,c,d) fread((char *)(a), b, (int)(c), d)

u_char  **bimage;
char    **grid;
int       nrow, ncol, nf;	/* number of rows, columns in the image */
int       fr;  /* current frame # */
int       obj_cnt = 0;
int       pix_cnt = 0;
int       object_size = 0;

FILE *fp;
int verbose, ndim, outflag;
char *out_base;
void	usageterm(), find_objects();


/******************************************************/
main(argc, argv)
    int       argc;
    char    **argv;
{
    struct header hd;  /* hips header */
    char out_name[80];

    u_char  **alloc_2d_byte_array();

    Progname = strsave(*argv);

    parse_args(argc, argv);

    read_header(&hd);
    if (hd.pixel_format != PFBYTE)
	perr(HE_MSG,"image must be in byte format");

    out_base = argv[1];

    nrow = hd.orows;		/* y axis */
    ncol = hd.ocols;		/* x axis */
    nf = hd.num_frame;
    if ( nf == 1)
	ndim = 2;
    else
	ndim = 3;

    if (!outflag)
	fp = stdout;

    fprintf(stderr, "\n rows: %d,  cols: %d,  frames: %d \n", nrow, ncol, nf);

    grid = (char **) alloc_2d_byte_array(hd.orows, hd.ocols);
    bimage = alloc_2d_byte_array(hd.orows, hd.ocols);

    for (fr = 0; fr < nf; fr++) { /* process each frame in the sequence */
	if (outflag) {
	    (void)fclose(fp);
	    sprintf(out_name,"%s.%d",out_base,fr);
	    if ((fp = fopen(out_name, "w")) == NULL) {
		fprintf(stderr, " Error opening output file %s \n", out_name);
		exit(-1);
	    }
	}
	if (verbose)
	    fprintf(fp, "\n   frame #   column #     row #  \n");

	if (nf > 1)
	    fprintf(stderr, "\n processing frame: %d ", fr);

	bzero(grid[0], nrow * ncol);

	read_2d_byte_array(stdin, bimage, hd.orows, hd.ocols);

	find_objects();

    }				/* for each frame  */

    fprintf(stderr, "\n\n total number of objects: %d \n",obj_cnt);
    fprintf(stderr, " total number of pixels: %d \n",pix_cnt);
    fprintf(stderr, "\n finished. \n\n");
    return (0);
}

/***************************************************************/
void
find_objects()
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

    if (num_neighbors(i, j) > 2) {	/* ignore small objects */
	obj_cnt++;
	object_size = 0;
	get_object_size(i,j);
	if (verbose)
	    fprintf(fp, "\n\nobject %d \n", obj_cnt);
	fprintf(fp, "\n %8d %6d \n",object_size, ndim );
	mark_object(i, j);
    }
}

/*********************************************************************/

get_object_size(i, j)		/* recursive routine to count all pixels in an
				 * object */
    int       i, j;
{

    int       ii, jj;

    grid[i][j] = -1;		/* mark is point as counted */

    object_size++;

    ii = i + 1;
    if (ii < nrow && grid[ii][j] == 0)
	if (bimage[ii][j] > 0)
	    get_object_size(ii, j);

    ii = i - 1;
    if (ii > 0 && grid[ii][j] == 0)
	if (bimage[ii][j] > 0)
	    get_object_size(ii, j);

    jj = j + 1;
    if (j + 1 < ncol && grid[i][jj] == 0)
	if (bimage[i][jj] > 0)
	    get_object_size(i, jj);

    jj = j - 1;
    if (jj > 0 && grid[i][jj] == 0)
	if (bimage[i][jj] > 0)
	    get_object_size(i, jj);

    /* also check diagonals */
    ii = i + 1;
    jj = j + 1;
    if (ii < nrow && jj < ncol && grid[ii][jj] == 0)
	if (bimage[ii][jj] > 0)
	    get_object_size(ii, jj);

    ii = i - 1;
    jj = j - 1;
    if (ii > 0 && jj > 0 && grid[ii][jj] == 0)
	if (bimage[ii][jj] > 0)
	    get_object_size(ii, jj);

    ii = i - 1;
    jj = j + 1;
    if (ii > 0 && j + 1 < ncol && grid[ii][jj] == 0)
	if (bimage[ii][jj] > 0)
	    get_object_size(ii, jj);

    ii = i + 1;
    jj = j - 1;
    if (i + 1 < nrow && jj > 0 && grid[ii][jj] == 0)
	if (bimage[ii][jj] > 0)
	    get_object_size(ii, jj);

}

/*********************************************************************/

mark_object(i, j)		/* recursive routine to mark all pixels in an
				 * object */
    int       i, j;
{
    int       ii, jj;

    grid[i][j] = 1;		/* mark is point as counted */

    if ( ndim == 2)
	fprintf(fp, "%10d %10d \n", j,i);
    else
	fprintf(fp, "%10d %10d %10d \n", fr,j,i);
    pix_cnt++;

    ii = i + 1;
    if (ii < nrow && grid[ii][j] <= 0)
	if (bimage[ii][j] > 0)
	    mark_object(ii, j);

    ii = i - 1;
    if (ii > 0 && grid[ii][j] <= 0)
	if (bimage[ii][j] > 0)
	    mark_object(ii, j);

    jj = j + 1;
    if (j + 1 < ncol && grid[i][jj] <= 0)
	if (bimage[i][jj] > 0)
	    mark_object(i, jj);

    jj = j - 1;
    if (jj > 0 && grid[i][jj] <= 0)
	if (bimage[i][jj] > 0)
	    mark_object(i, jj);

    /* also check diagonals */
    ii = i + 1;
    jj = j + 1;
    if (ii < nrow && jj < ncol && grid[ii][jj] <= 0)
	if (bimage[ii][jj] > 0)
	    mark_object(ii, jj);

    ii = i - 1;
    jj = j - 1;
    if (ii > 0 && jj > 0 && grid[ii][jj] <= 0)
	if (bimage[ii][jj] > 0)
	    mark_object(ii, jj);

    ii = i - 1;
    jj = j + 1;
    if (ii > 0 && j + 1 < ncol && grid[ii][jj] <= 0)
	if (bimage[ii][jj] > 0)
	    mark_object(ii, jj);

    ii = i + 1;
    jj = j - 1;
    if (i + 1 < nrow && jj > 0 && grid[ii][jj] <= 0)
	if (bimage[ii][jj] > 0)
	    mark_object(ii, jj);

}

/***********************************************************/
int
num_neighbors(i, j)
    int       i, j;
{
    int       cnt = 0;

    if (bimage[i][j] > 0)
	cnt++;
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

    return (cnt);
}
/****************************************************************/

parse_args(argc, argv)
    int       argc;
    char     *argv[];
{

    verbose = outflag = 0;
    out_base = NULL;

    /* Interpret options  */
    while (--argc > 0 && (*++argv)[0] == '-') {
        char     *s;
        for (s = argv[0] + 1; *s; s++)
            switch (*s) {
            case 'v':
                verbose++;
                break;
            case 'o':
                if (argc < 2)
                    usageterm();
                out_base = *++argv;
                fprintf(stderr, " creating output files: %s.N \n", out_base);
                argc--;
		outflag++;
                break;
            case 'h':
                usageterm();
                break;
            default:
                usageterm();
                break;
            }
    }                           /* while */
}

/******************************************************/
void
usageterm()
{
    fprintf(stderr, "Usage: printxyz [-v][-o outfile] < binary_image   \n ");
    fprintf(stderr, "   [-v] verbose: more descriptive text is output \n");
    fprintf(stderr, "   [outfile]: if specified, each frame is written to a new file named outfile.N \n\n");
    exit(0);
}



