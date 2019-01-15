/* pic: device-independent picture package */

static char rcsid[] = "$Header: pic.c,v 2.1 88/11/01 21:09:56 ph Locked $";

#include "simple.h"
#include "pic.h"

int pic_npic = -1;
extern Pic pic_hips;

Pic *pic_list[PIC_LISTMAX] = { &pic_hips, 0};

void pic_init()
{
    /* count the pic device types to set npic */
    for (pic_npic=0; pic_npic<PIC_LISTMAX && pic_list[pic_npic]; pic_npic++);
}

Pic *pic_open(file, mode)
char *file, *mode;
{
    return pic_open_dev("hips", file, mode);
}

Pic*	pic_dup(pic)
Pic*	pic;
{
Pic*	p;
	ALLOC(p, Pic, 1);
	*p = *pic;
	p->data = (*p->procs->dup)(p->data);
return	p;
}

Pic *pic_open_dev(dev, name, mode)
char *dev, *name, *mode;
{
    int i;
    char *data;
    Pic *p, *q;

    if (pic_npic<0) pic_init();
    if (!dev) {			/* probably comes from pic_file_dev */
	fprintf(stderr, "unknown pic device on %s\n", name);
	return 0;
    }
    for (i=0; i<pic_npic && !str_eq(dev, pic_list[i]->dev); i++);
    if (i>=pic_npic) {
	fprintf(stderr, "unknown pic device: %s\n", dev);
	return 0;
    }
    q = pic_list[i];
    data = (*q->procs->open)(name, mode);
    if (!data) return 0;

    /* copy the Pic structure before modifying it */
    ALLOC(p, Pic, 1);
    *p = *q;
    p->data = data;
    return p;
}

void pic_close(p)
Pic *p;
{
    (*p->procs->close)(p->data);
    free(p);
}

/* pic_catalog: print list of known (linked) device libraries */

void pic_catalog()
{
    int i;

    if (pic_npic<0) pic_init();
    fprintf(stderr,"picture devices/file formats known:");
    for (i=0; i<pic_npic; i++)
	fprintf(stderr," %s", pic_list[i]->dev);
    fprintf(stderr,"\n");
}

Pic *pic_load(name1, name2)
char *name1, *name2;
{
    Pic *p, *q;

    p = pic_open(name1, "r");
    if (!p) {
	fprintf(stderr, "pic_load: can't open %s\n", name1);
	return 0;
    }
    q = pic_open(name2, "w");
    if (!q) {
	fprintf(stderr, "pic_load: can't open %s\n", name2);
	pic_close(p);
	return 0;
    }
    pic_copy(p, q);
    pic_close(p);
    return q;
}

void pic_save(p, name)
Pic *p;
char *name;
{
    Pic *q;

    q = pic_open(name, "w");
    if (!q) {
	fprintf(stderr, "pic_save: can't create %s\n", name);
	return;
    }
    pic_copy(p, q);
    pic_close(q);
}

void pic_copy(p, q)
register Pic *p, *q;
{
    int nchan, dx, y;
    Window w;
    Pixel1 *buf;
    Pixel1_rgba *buf4;

    nchan = pic_get_nchan(p);
    pic_set_nchan(q, nchan);
    pic_set_window(q, pic_get_window(p, &w));
    dx = w.x1-w.x0+1;
    switch (nchan) {
	case 1:
	    ALLOC(buf, Pixel1, dx);
	    break;
	case 3:
	case 4:
	    ALLOC(buf4, Pixel1_rgba, dx);
	    break;
	default:
	    fprintf(stderr, "pic_copy: can't handle nchan=%d\n", nchan);
	    return;
    }
    for (y=w.y0; y<=w.y1; y++)
	switch (nchan) {
	    case 1:
		pic_read_row(p, y, w.x0, dx, buf);
		pic_write_row(q, y, w.x0, dx, buf);
		break;
	    case 3:
	    case 4:
		pic_read_row_rgba(p, y, w.x0, dx, buf4);
		pic_write_row_rgba(q, y, w.x0, dx, buf4);
		break;
	}
    if (nchan==1) free(buf); else free(buf4);
}

void pic_set_window(p, win)
Pic *p;
Window *win;
{
    pic_set_box(p, win->x0, win->y0, win->x1-win->x0+1, win->y1-win->y0+1);
}

void pic_write_block(p, x0, y0, nx, ny, buf)
Pic *p;
int x0, y0, nx, ny;
Pixel1 *buf;
{
    int y;

    for (y=0; y<ny; y++, buf+=nx)
	pic_write_row(p, y0+y, x0, nx, buf);
}

void pic_write_block_rgba(p, x0, y0, nx, ny, buf)
Pic *p;
int x0, y0, nx, ny;
Pixel1_rgba *buf;
{
    int y;

    for (y=0; y<ny; y++, buf+=nx)
	pic_write_row_rgba(p, y0+y, x0, nx, buf);
}

Window *pic_get_window(p, win)
Pic *p;
Window *win;
{
    int dx, dy;

    if (!win) ALLOC(win, Window, 1);
    pic_get_box(p, &win->x0, &win->y0, &dx, &dy);
    win->x1 = win->x0+dx-1;
    win->y1 = win->y0+dy-1;
    return win;
}

void pic_read_block(p, x0, y0, nx, ny, buf)
Pic *p;
int x0, y0, nx, ny;
Pixel1 *buf;
{
    int y;

    for (y=0; y<ny; y++, buf+=nx)
	pic_read_row(p, y0+y, x0, nx, buf);
}

void pic_read_block_rgba(p, x0, y0, nx, ny, buf)
Pic *p;
int x0, y0, nx, ny;
Pixel1_rgba *buf;
{
    int y;

    for (y=0; y<ny; y++, buf+=nx)
	pic_read_row_rgba(p, y0+y, x0, nx, buf);
}
