/*	zoom.c - for use with segal
 *
 *	zooms an area of an image
 */

#include "common.h"

/************************************************************/
void
zoom_to_ximage(win_id)
int win_id;
{
/* Replicates data in the upper left corner of the ximage to fill the entire
 * ximage.
 */
	int i;
	int x1, y1;
	int sx, sy, y0;

	if(win[win_id].zoom_mag <= 1) {
		bcopy(win[win_id].z_data[0],
			(u_char *) win[win_id].ximg->data,
			win[win_id].img_size);
		return;
	}

	/* Magnify in the X direction */
	/* start in the upper right corner of the data and expand */
	for(x1 = win[win_id].img_c - 1; x1 >= 0; x1--)
	for(y1 = 0; y1 < win[win_id].img_r; y1++) {
		sx = x1 * win[win_id].zoom_mag;
		sy = y1 * win[win_id].ximg->bytes_per_line;
		for(i = sx; i < sx + win[win_id].zoom_mag; i++)
			win[win_id].ximg->data[i + sy] = win[win_id].z_data[y1][x1];
	}

	/* Magnify in the Y direction */
	/* start with the lower left corner of the data and expand */
	for(x1 = 0; x1 < win[win_id].ximg->width; x1++)
	for(y1 = win[win_id].img_r - 1; y1 > 0; y1--) {
		y0 = y1 * win[win_id].ximg->bytes_per_line;
		sy = y1 * win[win_id].zoom_mag;
		for(i = sy; i < sy + win[win_id].zoom_mag; i++)
			win[win_id].ximg->data[x1 + i * win[win_id].ximg->bytes_per_line] = win[win_id].ximg->data[x1 + y0];
	}
}
