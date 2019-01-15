/*
 * filter.c - Routines to apply the Hips Filter to the Image, the Mask, or
 *		  both.
 *
 *	Bryan Skene, LBL, 6/91
 *	-for use with Segal
 *
 *	Note: This needs to be updated to work with segal3d.  I would add
 *	stuff to allow the source buffer to be any region, consistent with
 *	the rest of segal3d's philosophy on routines.
 *
 */

#include "common.h"

#define Realloc(x,y,z) (z *)realloc((char *)x,(unsigned)(y * sizeof(z)))

/*****************************************************/
void
apply_filter()
{
	void redisplay_all();
	int filter_buf();

	int which_in, which_out, result;
	u_char **in_buf, **out_buf;

	which_in = xv_get(filter_win->set_source_buf, PANEL_VALUE, NULL);
	if(which_in == 0) in_buf = himage.data;
	else in_buf = m[segal.edit_m].data;

	which_out = xv_get(filter_win->set_dest_buf, PANEL_VALUE, NULL);
	if(which_out == 0) out_buf = himage.data;
	else out_buf = m[segal.edit_m].data;

	result = filter_buf(in_buf, out_buf, "temp",
		xv_get(filter_win->filter_name, PANEL_VALUE, NULL),
		xv_get(filter_win->filter_parameters, PANEL_VALUE, NULL));

	if(result == -1) fprintf(stderr, "Filter_buf failed\n");
	
	if(which_out == 1) {
		bcopy(m[segal.edit_m].data[0], work_buf[0],
			segal.rows * segal.cols);
		segal.changed = 1;
	}
	redisplay_all();
}

/****************************************************/
int
filter_buf(in_buf, out_buf, filename, filter_name, param_list)
u_char **in_buf, **out_buf;
char *filename;
char *filter_name;
char *param_list;
{
/* execs a shell command and utilizes a temporary file to filter one frame */
/* NOTE: if parameter is -h then help info goes to stderr ... no buffer
 *	 modifications.
 */
        u_char  **alloc_2d_byte_array();
	int	free_2d_byte_array();

	int i, j, i1, j1, fd;
	int x, y, x1, y1, x2, y2, temp_x1, temp_y1, temp_x2, temp_y2;
	FILE *fp;
	char fname[MAXPATHLEN], temp_fname[MAXPATHLEN], command[MAXPATHLEN];
	u_char **temp_buf;
	struct header hd;

	/* only help wanted ... */
	if(strcmp("-h", param_list) == 0) {
		sprintf(command, "%s -h", filter_name);
		system(command);
		return(0);
	}

	/* create a temp file to store the frame buffer */
	sprintf(temp_fname, "%s.tmp", filename);
	if((fp = fopen(temp_fname, "w")) == NULL) {
		fprintf(stderr, "Cannot create temporary file\n");
		return;
	}
	write_2d_byte_array(fp, in_buf, segal.rows, segal.cols);
	fclose(fp);

        /* put a header on it */
        sprintf(command, "genheader -B -s %d %d -f 1 -nc 1 -p < %s > %s.hd",
                segal.rows, segal.cols, temp_fname, temp_fname);
        system(command);
 
        /* piece together the info */
        sprintf(command, "%s %s < %s.hd > %s.out",
                filter_name, param_list, temp_fname, temp_fname);
 
        /* do the actual filtering */
        system(command);
	
	/* get back the result into temp_buf */
	sprintf(fname, "%s.out", temp_fname);
        fp = hfopenr(fname);
        if (fp == NULL) {
                fprintf(stderr, "\n*** error opening file %s\n\n", fname);
                return (-1);
        }

        fread_hdr_a(fp, &hd, fname);
	temp_buf = alloc_2d_byte_array(hd.orows, hd.ocols);
        if (read_2d_byte_array(fp, temp_buf, hd.orows, hd.ocols) == -1)
                return (-1);

	/* now, if size of buf changed from segal.rows & segal.cols,
	 * figure out what to do about it
	 */

	/* Get it from the middle of the result */

	if(hd.ocols < segal.cols) {
		x = segal.cols - hd.ocols;
		x1 = x / 2;
		x2 = segal.cols - 1 - (x/2);
		temp_x1 = 0;
		temp_x2 = hd.ocols - 1;
	}
	else if(hd.ocols > segal.cols) {
		x = hd.ocols - segal.cols;
		x1 = 0;
		x2 = segal.cols - 1;
		temp_x1 = x / 2;
		temp_x2 = hd.ocols - 1 - (x/2);
	}
	else if(segal.cols == hd.ocols) {
		x1 = temp_x1 = 0;
		x2 = temp_x2 = segal.cols - 1;
	}

	if(hd.orows < segal.rows) {
		y = segal.rows - hd.orows;
		y1 = y / 2;
		y2 = segal.rows - 1 - (y/2);
		temp_y1 = 0;
		temp_y2 = hd.orows - 1;
	}
	else if(hd.orows > segal.rows) {
		y = hd.orows - segal.rows;
		y1 = 0;
		y2 = segal.rows - 1;
		temp_y1 = y / 2;
		temp_y2 = hd.orows - 1 - (y/2);
	}
	else if(segal.rows == hd.orows) {
		y1 = temp_y1 = 0;
		y2 = temp_y2 = segal.rows - 1;
	}

	for(j = 0; j < segal.rows; j++)
	for(i = 0; i < segal.cols; i++)
		if(j < y1 || j > y2 || i < x1 || i > x2) out_buf[j][i] = 0;
		else {
			j1 = j - y1 + temp_y1;
			RANGE(j1, 0, (hd.orows-1));
			i1 = i - x1 + temp_x1;
			RANGE(i1, 0, (hd.ocols-1));
			out_buf[j][i] = temp_buf[j1][i1];
		}

	fclose(fp);

	/* clean house */
	free_2d_byte_array(temp_buf);
	unlink(temp_fname);
	unlink(strcat(temp_fname, ".hd"));
	unlink(strcat(temp_fname, ".out"));

	return(0);
}
