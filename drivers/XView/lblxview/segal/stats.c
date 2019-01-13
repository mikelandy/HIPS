
/*  stats.c       Brian Tierney, LBL       8/90
 *    for use with segal
 *
 *  These routines are used to compute statistics of objects in the mask image
 */

#include "common.h"

#include "segal_ui.h"
extern segal_stats_win_objects *segal_stats_win;

char    **grid;
long      size;			/* count of number of pixels in an object */
double    pix_total;		/* sum of pixel values in an object */
int       perim_count = 0;	/* count of number of pixels in the perimeter */
int       min, max;		/* minimum and maximum pixel values in the
				 * object */
int       hist[256];		/* histogram array */


/*************************************************************/
void
get_stats(i, j)
    int       i, j;
{
    void      get_object(), show_no_stats();
    static int grid_rows = 0, grid_cols = 0;

    if (verbose)
	fprintf(stderr, " \n show stats for object at loc %d,%d \n", i, j);

    XSetForeground(display, gc, red_standout);
    image_repaint_proc();

    bzero((char *) hist, 256 * sizeof(int));

    if (grid_rows != segal.rows || grid_cols != segal.cols) {	/* if size changes */
	if (grid_rows != 0)	/* not 1st time */
	    free_2d_byte_array(grid);
	grid = (char **) alloc_2d_byte_array(segal.rows, segal.cols);
	grid_rows = segal.rows;
	grid_cols = segal.cols;
    }
    get_object(j, i);

    if (grid != NULL)
	bzero(*grid, segal.rows * segal.cols);
}

/**************************************************************/
void
get_object(i, j)
    int       i, j;
{
    static int tc = 0;
    void      show_stats(), show_no_stats(), get_total_image_stats();

    size = 0L;
    pix_total = 0.;
    perim_count = 0;
    min = 255;
    max = 0;

    if (work_buf[i][j] > 0)
	count_pixels(i, j);
    else
	get_total_image_stats();

    if (verbose)
	fprintf(stdout, " size of object is %d pixels.\n", size);

    if (work_buf[i][j] > 0)
	show_stats(i, j);
    else {
#ifdef NO_STATS
	show_no_stats(i, j);
	fprintf(stderr, " No object at this location. \n");
#else
	show_stats(-1, -1);
#endif
    }
}

/*********************************************************************/
void
get_total_image_stats()
{
    register int i, j;

    size = segal.rows * segal.cols;

    for (i = 0; i < segal.rows; i++)
	for (j = 0; j < segal.cols; j++) {
	    pix_total += (double) himage.data[i][j];
	    if (himage.data[i][j] > max)
		max = himage.data[i][j];
	    if (himage.data[i][j] < min)
		min = himage.data[i][j];
	    hist[himage.data[i][j]]++;
	}
}

/*********************************************************************/
void
show_stats(x, y)		/* display statistics */
    int       x, y;
{
    double    p_ave;
    int       median;
    char      mesg[80];

    p_ave = pix_total / (double) size;
    median = compute_median(size);

    if (verbose) {
	fprintf(stdout, " total pixel intensity: %.0f, (log10: %.3f)\n",
		pix_total, log10(pix_total));
	fprintf(stdout, " mean pixel value: %.2f, (log10: %.3f)\n",
		p_ave, log10(p_ave));
	fprintf(stdout, " number of pixels in object perimeter: %d \n\n",
		perim_count);
	fprintf(stdout, " minimum pixel val: %d, maximum pixel val: %d\n\n",
		min, max);
    }
    /* write into window */
    if (x >= 0)
	sprintf(mesg, "Object Location: (%d,%d)", x, y);
    else
	sprintf(mesg, "Statistics for entire image:");
    (void) xv_set(segal_stats_win->stat_mess0,
		  PANEL_LABEL_STRING, mesg, NULL);

    sprintf(mesg, "Object size (pixels): %d", size);
    (void) xv_set(segal_stats_win->stat_mess1,
		  PANEL_LABEL_STRING, mesg, NULL);

    sprintf(mesg, "Total pixel intensity:  %.0f, (log10: %.3f)",
	    pix_total, log10(pix_total));
    (void) xv_set(segal_stats_win->stat_mess2,
		  PANEL_LABEL_STRING, mesg, NULL);

    sprintf(mesg, "Mean pixel value:  %.2f ",
	    p_ave);
    (void) xv_set(segal_stats_win->stat_mess3,
		  PANEL_LABEL_STRING, mesg, NULL);

    sprintf(mesg, "Median pixel value: %d ", median);
    (void) xv_set(segal_stats_win->stat_mess4,
		  PANEL_LABEL_STRING, mesg, NULL);

    if (x >= 0)
	sprintf(mesg, "Number of pixels in object perimeter:  %d",
		perim_count);
    else
	sprintf(mesg, " ");
    (void) xv_set(segal_stats_win->stat_mess5,
		  PANEL_LABEL_STRING, mesg, NULL);

    sprintf(mesg, "Minimum: %d, Maximum: %d", min, max);
    (void) xv_set(segal_stats_win->stat_mess6,
		  PANEL_LABEL_STRING, mesg, NULL);

    xv_set(segal_stats_win->stats_win, WIN_SHOW, TRUE, NULL);
}

/*********************************************************************/
void
show_no_stats(x, y)		/* if object not found */
    int       x, y;
{
    char      mesg[80];

    /* write into window */
    sprintf(mesg, "Object Location: (%d,%d)", x, y);
    (void) xv_set(segal_stats_win->stat_mess0,
		  PANEL_LABEL_STRING, mesg, NULL);

    sprintf(mesg, "No Object at this location");
    (void) xv_set(segal_stats_win->stat_mess1,
		  PANEL_LABEL_STRING, mesg, NULL);

    sprintf(mesg, " ");
    (void) xv_set(segal_stats_win->stat_mess2,
		  PANEL_LABEL_STRING, mesg, NULL);

    (void) xv_set(segal_stats_win->stat_mess3,
		  PANEL_LABEL_STRING, mesg, NULL);

    (void) xv_set(segal_stats_win->stat_mess4,
		  PANEL_LABEL_STRING, mesg, NULL);

    (void) xv_set(segal_stats_win->stat_mess5,
		  PANEL_LABEL_STRING, mesg, NULL);

    (void) xv_set(segal_stats_win->stat_mess6,
		  PANEL_LABEL_STRING, mesg, NULL);

    xv_set(segal_stats_win->stats_win, WIN_SHOW, TRUE, NULL);
}

/*********************************************************************/

count_pixels(i, j)		/* recursive routine to count the number of
				 * pixels in an object */
    int       i, j;
{
    int       ii, jj;

    size++;
    pix_total += (double) himage.data[i][j];
    if (himage.data[i][j] > max)
	max = himage.data[i][j];
    if (himage.data[i][j] < min)
	min = himage.data[i][j];
    hist[himage.data[i][j]]++;

    grid[i][j] = 1;		/* mark is point as counted */

    XDrawPoint(display, view_xid, gc, j, i);

    if (num_neighbors(i, j) < 4) {	/* if has all 4 neigbors,not on
					 * perimeter */
	perim_count++;
    }
    ii = i + 1;
    if (ii < segal.rows && grid[ii][j] == 0)
	if (work_buf[ii][j] > 0)
	    count_pixels(ii, j);

    ii = i - 1;
    if (ii > 0 && grid[ii][j] == 0)
	if (work_buf[ii][j] > 0)
	    count_pixels(ii, j);

    jj = j + 1;
    if (j + 1 < segal.cols && grid[i][jj] == 0)
	if (work_buf[i][jj] > 0)
	    count_pixels(i, jj);

    jj = j - 1;
    if (jj > 0 && grid[i][jj] == 0)
	if (work_buf[i][jj] > 0)
	    count_pixels(i, jj);

    /* also check diagonals */
    ii = i + 1;
    jj = j + 1;
    if (ii < segal.rows && jj < segal.cols && grid[ii][jj] == 0)
	if (work_buf[ii][jj] > 0)
	    count_pixels(ii, jj);

    ii = i - 1;
    jj = j - 1;
    if (ii > 0 && jj > 0 && grid[ii][jj] == 0)
	if (work_buf[ii][jj] > 0)
	    count_pixels(ii, jj);

    ii = i - 1;
    jj = j + 1;
    if (ii > 0 && j + 1 < segal.cols && grid[ii][jj] == 0)
	if (work_buf[ii][jj] > 0)
	    count_pixels(ii, jj);

    ii = i + 1;
    jj = j - 1;
    if (i + 1 < segal.rows && jj > 0 && grid[ii][jj] == 0)
	if (work_buf[ii][jj] > 0)
	    count_pixels(ii, jj);

}

/***********************************************************/
int
num_neighbors(i, j)		/* counts number of neighbors (max = 4) */
    int
              i, j;
{
    int       cnt = 0;

    if (i > 0)
	if (work_buf[i - 1][j] > 0)
	    cnt++;
    if (j > 0)
	if (work_buf[i][j - 1] > 0)
	    cnt++;
    if (i < segal.rows - 1)
	if (work_buf[i + 1][j] > 0)
	    cnt++;
    if (j < segal.cols - 1)
	if (work_buf[i][j + 1] > 0)
	    cnt++;

    return (cnt);
}

/*********************************************************** */
int
compute_median(npixels)
    int       npixels;
{
    /*
     * finds the gray level value at which 1/2 of the pixels in the window
     * are below this value
     */

    int       total = 0;	/* numb of pixels <= median */
    int       mdn = 0;
    int       half_count = npixels / 2;

    while (total + hist[mdn] <= half_count) {
	total += hist[mdn];
	mdn++;
    }

    return (mdn);
}

/*********************************************************** */
void
stat_close_proc()
{				/* close button in stat window */

    xv_set(segal_stats_win->stats_win, WIN_SHOW, FALSE, NULL);
}
