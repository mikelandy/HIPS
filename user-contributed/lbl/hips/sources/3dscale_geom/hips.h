/* $Header$ */

#include "header.def"
#include "imagedef.h"
#include "pixel.h"

typedef struct {
	char	name[25];	/* picture name */
	short	nchan;	/* number of channels (1=monochrome, 3=RGB) */
	short	ox, oy;	/* origin (upper left corner) of screen */
	short	dx, dy;	/* width and height of picture in pixels */
	Pixel1	*ibuf,	/* pointer to the image buffer */ 
		*lbuf;	/* loading buffer for multi-thread	*/
	int	mt;	/* true if multi-thread	*/
} Hips;

Hips	*hips_open(/* file, mode */), *hips_dup(/* hips */);
void	hips_close(/* p */);

char	*hips_get_name(/* p */);
void	hips_clear(/* p, pv */);
void	hips_clear_rgba(/* p, r, g, b, a */);

void	hips_set_nchan(/* p, nchan */);
void	hips_set_box(/* p, ox, oy, dx, dy */);
void	hips_write_pixel(/* p, x, y, pv */);
void	hips_write_pixel_rgba(/* p, x, y, r, g, b, a */);
void	hips_write_row(/* p, y, x0, nx, buf */);
void	hips_write_row_rgba(/* p, y, x0, nx, buf */);

int	hips_get_nchan(/* p */);
void	hips_get_box(/* p, ox, oy, dx, dy */);
Pixel1	hips_read_pixel(/* p, x, y */);
void	hips_read_pixel_rgba(/* p, x, y, pv */);
void	hips_read_row(/* p, y, x0, nx, buf */);
void	hips_read_row_rgba(/* p, y, x0, nx, buf */);

#ifndef	XMAX
#define XMAX 8192	/* 3584 */
#define YMAX 8192	/* 3072 */
#endif

