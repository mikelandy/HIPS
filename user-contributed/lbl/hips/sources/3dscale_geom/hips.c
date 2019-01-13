/*
*	hips: subroutine package to read and write hips images
*/

#include "hips.h"
#include "simple.h"
#include "pixel.h"

extern U_IMAGE	uimg;


Hips*	hips_open(file, mode)
char	*file, *mode;
{
Hips	*p;

    ALLOC_ZERO(p, Hips, 1);
    strcpy(p->name, file);

    if (str_eq(mode, "r")) {		/* input file	*/
	(*uimg.header_handle)(HEADER_READ, &uimg, 0, 0, No);
	if (!uimg.color_dpy)	{	/* forcing SGF	*/
		uimg.o_form = IFMT_SGF;
		uimg.pxl_out = 1;
	} else	{
		uimg.o_form = uimg.in_form==IFMT_ALPHA ? IFMT_ILC : uimg.in_form;
		uimg.pxl_out = MIN(3, uimg.pxl_in);
	}
	if (uimg.dpy_channels==3)
		uimg.color_form = CFM_SEPLANE;
	uimg.frames += !uimg.frames;
	if (uimg.in_type == HIPS)	{
		if (uimg.in_form != IFMT_BYTE)
			prgmerr(0, "warning: pixel format may not be byte\n");
		uimg.o_type = HIPS;
		uimg.o_form = IFMT_BYTE;
	}
	p->dx = uimg.width;
	p->dy = uimg.height;
	p->nchan = 1;		/* used for byte filter	*/
    } else {
	p->dx = XMAX;
	p->dy = YMAX;
	p->nchan = uimg.dpy_channels;
    }
	p->ox = p->oy = 0;
return p;
}

Hips*	hips_dup(stp)
Hips*	stp;
{
Hips	*p;
	ALLOC_ZERO(p, Hips, 1);
	*p = *stp;
return	p;
}

/**********************************************/
void
hips_close(p)
Hips     *p;
{
prgmerr(0, "Not needed: using stdin and stdout \n");
}

char     *
hips_get_name(p)
Hips     *p;
{
return	p->name;
}

/****************************************/
void
hips_clear(p, pv)
Hips     *p;
Pixel1    pv;
{
}

void
hips_clear_rgba(p, r, g, b, a)
Hips	*p;
Pixel1	r, g, b, a;
{
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
Hips	*p;
int	ox, oy, dx, dy;
{
    if (dx > XMAX || dy > YMAX)
	prgmerr(2, "\nMaximum destination size is %d x %d \n\n",
		 XMAX, YMAX);
    p->ox = ox;
    p->oy = oy;
    p->dx = dx;
    p->dy = dy;
    if (!p->ibuf)
	p->ibuf = nzalloc(dx*dy, p->nchan, "h_set_box");
}

/****************************************/
static	void
unimp_err(nm)
char	*nm;
{
prgmerr(1, "hips_%s: unimplemented\n", nm);
}

void
hips_write_pixel(p, x, y, pv)
Hips	*p;
int	x, y;
Pixel1	pv;
{
unimp_err("write_pixel");
}

void
hips_write_pixel_rgba(p, x, y, r, g, b, a)
Hips	*p;
int	x, y;
Pixel1	r, g, b, a;
{
unimp_err("write_pixel_rgba");
}

/****************************************/
void
hips_write_row(p, y, x0, nx, buf)
Hips	*p;
int	y, x0, nx;
register Pixel1	*buf;
{

register int	i, offset = y * p->dx + x0;

    for (i = 0; i < nx; i++) {
	p->ibuf[offset + i] = buf[i];
/*	fprintf(stderr, " %d", buf[i]); */
    }
}

/****************************************/
void
hips_write_row_rgba(p, y, x0, nx, buf)
Hips	*p;
int	y, x0, nx;
register Pixel1	*buf;
{
unimp_err("write_row_rgba");
}

/****************************************/
void
hips_get_box(p, ox, oy, dx, dy)
Hips	*p;
int	*ox, *oy, *dx, *dy;
{
	*ox = p->ox;
	*oy = p->oy;
	*dx = p->dx;
	*dy = p->dy;
}

/*-------------------- file reading routines ------------------*/

hips_get_nchan(p)
Hips	*p;
{
	return p->nchan;
}

Pixel1
hips_read_pixel(p, x, y)
Hips	*p;
int	x, y;
{
unimp_err("read_pixel");
}

void
hips_read_pixel_rgba(p, x, y, pv)
Hips	*p;
int	x, y;
Pixel1	*pv;
{
unimp_err("read_pixel_rgba");
}

/****************************************/
void
hips_read_row(p, y, x0, nx, buf)
Hips	*p;
int	y, x0, nx;
Pixel1	*buf;
{
register int	i, offset = y * p->dx + x0;

    for (i = 0; i < nx; i++) {
	buf[i] = p->ibuf[offset + i];
	/* fprintf(stderr, " %d", buf[i]); */
    }

}

/****************************************/
void
hips_read_row_rgba(p, y, x0, nx, buf)
Hips	*p;
int	y, x0, nx;
Pixel1	*buf;
{
unimp_err("read_row_rgba");
}

/****************************************/
/* routines to load and save hips file */
/****************************************/
load_hips_frame(img, p)
U_IMAGE	*img;
Hips	*p;
{
char	**bp = p->mt > 0 ? &p->lbuf : &p->ibuf;	/* for multi-thread	*/
int	w = img->width, h = img->height, tl;
	img->load_all = 1;
	img->width = p->dx;
	img->height = p->dy;
	img->src = *bp;
	tl = (*img->std_swif)(FI_LOAD_FILE, img, 0, 0);
	*bp = img->src;
	img->width = w,	img->height = h;
	if (tl > 0)	tl = p->dx * p->dy;	/* avoid errors	*/
return	tl;
}

/****************************************/
void	store_hips_frame(p)
Hips     *p;
{
int	len = (uimg.width=p->dx) * (uimg.height=p->dy) * sizeof(*p->ibuf);

if (uimg.o_type == RLE)	{
register Pixel1	*r=p->ibuf, *g=r+len, *b=g+len, *obp;
register int	w = p->dx, ch3 = p->nchan==3;
uimg.src = obp = nzalloc(len, p->nchan, "store_rle");
	for (len=p->dy; len--;)	{
		memcpy(obp, r, w);	obp += w;	r += w;
		if (!ch3)	continue;
		memcpy(obp, g, w);	obp += w;	g += w;
		memcpy(obp, b, w);	obp += w;	b += w;
	}
	(*uimg.std_swif)(FI_SAVE_FILE, &uimg, 0, 0);
free(uimg.src);
uimg.src = 0;
}
else if (fwrite(p->mt>0 ? p->lbuf : p->ibuf, p->nchan, len, out_fp) != len)
	syserr("during write\n");
}
