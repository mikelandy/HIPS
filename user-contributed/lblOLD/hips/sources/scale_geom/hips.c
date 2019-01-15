/*
 * hips: subroutine package to read and write hips images
 */

static char rcsid[] = "$Header$ ";

#include "hips.h"
#include "simple.h"
#include "pixel.h"

#define Calloc(a,b) (b *) calloc((unsigned)(a), sizeof(b))

extern struct header hd;
/****************************************/
Hips
* hips_open(file, mode)
    char     *file, *mode;
{
    Hips     *p;
    int       sz;

    ALLOC_ZERO(p, Hips, 1);

    strcpy(p->name, file);
    p->nchan = 1;
    p->ox = 0;
    p->oy = 0;

    if (str_eq(mode, "r")) {	/* input file */
	read_header(&hd);
	if (hd.pixel_format != PFBYTE) {
	    fprintf(stderr, "scale_geom: pixel format must be byte\n");
	    exit(1);
	}
	p->dx = hd.ocols;
	p->dy = hd.orows;
	sz = hd.orows * hd.ocols;
    } else {			/* maximum output file */
	p->dx = XMAX;
	p->dy = YMAX;
	sz = XMAX * YMAX;
    }

    if ((p->ibuf = Calloc(sz, Pixel1)) == NULL) {
	fprintf(stderr, "memory allocation error \n");
	exit(1);
    }
    return p;
}

/**********************************************/
void
hips_close(p)
    Hips     *p;
{
    fprintf(stderr, "Not needed: using stdin and stdout \n");
    exit(1);
}

/****************************************/
char     *
hips_get_name(p)
    Hips     *p;
{
    return p->name;
}

/****************************************/
void
hips_clear(p, pv)
    Hips     *p;
    Pixel1    pv;
{
    return;
}

/****************************************/
void
hips_clear_rgba(p, r, g, b, a)
    Hips     *p;
    Pixel1    r, g, b, a;
{
    return;
}

/*-------------------- file writing routines --------------------*/
void
hips_set_nchan(p, nchan)
    Hips     *p;
    int       nchan;
{
    p->nchan = nchan;
}

/****************************************/
void
hips_set_box(p, ox, oy, dx, dy)
    Hips     *p;
    int       ox, oy, dx, dy;
{
    p->ox = ox;
    p->oy = oy;
    if (dx > XMAX || dy > YMAX) {
	fprintf(stderr, "\n Error: Maximum destination size if %d x %d \n\n",
		 XMAX, YMAX);
	exit(-1);
    }
    p->dx = dx;
    p->dy = dy;
}

/****************************************/
void
hips_write_pixel(p, x, y, pv)
    Hips     *p;
    int       x, y;
    Pixel1    pv;
{
    fprintf(stderr, "hips_write_pixel: unimplemented\n");
    exit(1);
}

/****************************************/
void
hips_write_pixel_rgba(p, x, y, r, g, b, a)
    Hips     *p;
    int       x, y;
    Pixel1    r, g, b, a;
{
    fprintf(stderr, "hips_write_pixel_rgba: unimplemented\n");
    exit(1);
}

/****************************************/
void
hips_write_row(p, y, x0, nx, buf)
    Hips     *p;
    int       y, x0, nx;
    register Pixel1 *buf;
{

    int       offset, i;

    offset = (y * p->dx) + x0;

    for (i = 0; i < nx; i++) {
	p->ibuf[offset + i] = buf[i];
/*	fprintf(stderr, " %d", buf[i]); */
    }
}

/****************************************/
void
hips_write_row_rgba(p, y, x0, nx, buf)
    Hips     *p;
    int       y, x0, nx;
    register Pixel1 *buf;
{
    fprintf(stderr, "hips_write_row_rgba: unimplemented\n");
    exit(1);
}

/*-------------------- file reading routines --------------------*/

int
hips_get_nchan(p)
    Hips     *p;
{
    return p->nchan;
}

/****************************************/
void
hips_get_box(p, ox, oy, dx, dy)
    Hips     *p;
    int      *ox, *oy, *dx, *dy;
{
    *ox = p->ox;
    *oy = p->oy;
    *dx = p->dx;
    *dy = p->dy;
}

/****************************************/
Pixel1
hips_read_pixel(p, x, y)
    Hips     *p;
    int       x, y;
{
    fprintf(stderr, "hips_read_pixel: unimplemented\n");
    exit(1);
}

/****************************************/
void
hips_read_pixel_rgba(p, x, y, pv)
    Hips     *p;
    int       x, y;
    Pixel1   *pv;
{
    fprintf(stderr, "hips_read_pixel_rgba: unimplemented\n");
    exit(1);
}

/****************************************/
void
hips_read_row(p, y, x0, nx, buf)
    Hips     *p;
    int       y, x0, nx;
    Pixel1   *buf;
{
    int       offset, i;

    offset = (y * p->dx) + x0;

    for (i = 0; i < nx; i++) {
	buf[i] = p->ibuf[offset + i];
	/* fprintf(stderr, " %d", buf[i]); */
    }

}

/****************************************/
void
hips_read_row_rgba(p, y, x0, nx, buf)
    Hips     *p;
    int       y, x0, nx;
    Pixel1   *buf;
{
    fprintf(stderr, "hips_read_row_rgba: unimplemented\n");
    exit(1);
}

/****************************************/
/* routines to load and save hips file */
/****************************************/
void
load_hips_frame(p)
    Hips     *p;
{
    int       length;

    length = p->dx * p->dy * sizeof(Pixel1);

    if (fread(p->ibuf, length,1,stdin) != 1) {
	fprintf(stderr, "error during read\n");
	exit(1);
    }
}

/****************************************/
void
store_hips_frame(p)
    Hips     *p;
{
    int       length;

    length = p->dx * p->dy * sizeof(Pixel1);

    if (fwrite(p->ibuf, length,1,stdout) != 1) {
	fprintf(stderr, "error during write\n");
	exit(1);
    }
}
