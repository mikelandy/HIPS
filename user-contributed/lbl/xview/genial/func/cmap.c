/*
 * cmap.c -- routines for manipulating colormap
 *
 * general steps involved in creating ximage with proper color:
 *   1) read data
 *   2) create buffer of gray-level lut indices (make_lut)
 *   3) create colormap of these colors (build_colormap)
 *   4) create buffer of colormap lut indices (map_image_to_colors)
 *   5) create ximage (mk_x_img)
 */

#include <math.h>
#include "ui.h"
#include "display.h"

char     *palnames[PALSIZE] =
{"red", "yellow", "green", "cyan", "blue", "magenta", "black", "white"};

XColor    pallet[PALSIZE];
u_long    standout;
u_long   *colors;

Window    mainw;
byte      red[256], green[256], blue[256];

void      make_lut();
u_long *build_colormap();

/*************************************************************/
void
cmap_init()
{
    Colormap  cmap;
    int       i, val;

    mainw = (Window) xv_get(img_win->d_win, XV_XID, NULL);

    cmap = XDefaultColormap(display, DefaultScreen(display));
    /* set up standout colors (read-only colors) */
    for (i = 0; i < PALSIZE; i++) {
	if (XParseColor(display, cmap, palnames[i], &pallet[i]) == 0)
	    fprintf(stderr, "%s: Error parsing color \n", Progname);
	else if (XAllocColor(display, cmap, &pallet[i]) == 0)
	    fprintf(stderr, "%s: couldn't allocate color: %s.\n",
		    Progname, palnames[i]);
    }
    standout = pallet[GREEN].pixel;

    for (i = 0; i < 256; i++) {
	val = (int) ((i * 256.0 / (float) NCOLORS) + .5);
	if (val > 255)
	    val = 255;
	red[i] = green[i] = blue[i] = (byte) val;
    }
}

/*************************************************************/
void
build_cmap(image)
    struct img_data *image;
{
    make_lut(image, NCOLORS);	/* creates image->lut from image->data */

    /* sorted lut is returned im image_lut */
    colors = build_colormap(display, winv, mainw,
			    (byte *)image->lut, image->width * image->height,
			    red, green, blue, 256,
			    0, 0, NCOLORS, 0, 1, 0, 1, Progname);

    return;

}
/************************************************/
void
make_lut(image, ncols)
    struct img_data *image;
    int       ncols;
{
    register int i, j;
    u_char   *c_ptr;
    u_short  *s_ptr;
    u_int    *i_ptr;
    float     delta = (float) (ncols - 1) / (float) (image->maxv - image->minv);

    switch (image->dsize) {
    case 1:
	c_ptr = (u_char *) image->data;
	for (i = 0; i < image->height * image->width; i++) {
	    j = (int) c_ptr[i];
	    image->lut[i] = (byte) (((j - image->minv) * delta));
	}
	break;
    case 2:
	s_ptr = (u_short *) image->data;
	for (i = 0; i < image->height * image->width; i++) {
	    j = (int) s_ptr[i];
	    image->lut[i] = (byte) (((j - image->minv) * delta));
	}
	break;
    case 4:
	i_ptr = (u_int *) image->data;
	for (i = 0; i < image->height * image->width; i++) {
	    j = (int) i_ptr[i];
	    image->lut[i] = (byte) (((j - image->minv) * delta));
	}
	break;
    }
    return;
}

/*************************************************************/
int
get_gamma(val, gam)
    byte      val;
    float     gam;
{
    float     newval;

    newval = 256. * pow((double) val / 256., (double) (1.0 / gam));
    return (MIN((int) (newval + .5), 255));	/* dont allow values > 255 */
}
