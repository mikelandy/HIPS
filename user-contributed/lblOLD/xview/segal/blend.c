/*
 *  blend.c            -Brian Tierney,  LBL
 */

#include "common.h"

#include "display_control.h"
#include "mask_control.h"
#include "pixedit.h"

/* code for halftone blend is included, but not really useful,
    because when the halftone is zoomed it looses the halftone
    effect */

#ifdef HALFTONE_BLEND
/* used for halftoning */
#define	RANDOMVALS	1201
short    *error, *rtab;
#endif

u_char gray_lut[256], blend_lut[256];

/**************************************************************/
void
slider_proc()
{				/* procedure called when blend sliders are
				 * adjusted */
    void      blend(), zoom(), edit_repaint_proc(), make_blend_lut();
    void      map_image_to_lut(), image_repaint_proc();
    static int old_s1 = -1, old_s2 = -1;

    segal.slider1 = (int) xv_get(display_control_win->bg_slider, PANEL_VALUE, 0);
    segal.slider2 = 100 - (int) xv_get(display_control_win->fg_slider, PANEL_VALUE, 0);

    if (segal.slider1 == old_s1 && segal.slider2 == old_s2)	/* no change */
	return;

    if (himage.fp == NULL || blend_image->data == NULL)
	return;

    set_watch_cursor();

    make_blend_lut();

    if (segal.display_type != 2) {	/* set display to blend */
	xv_set(display_control_win->display_type, PANEL_VALUE, 2, NULL);
	segal.display_type = 2;
    }
    blend(himage.data[0], work_buf[0], (u_char *) blend_image->data,
	  segal.rows * segal.cols);

    if ((int) xv_get(edit_win->win, XV_SHOW, NULL) == TRUE) {
	zoom();
	edit_repaint_proc();
    }
    image_repaint_proc();

    unset_watch_cursor();
    old_s1 = segal.slider1;
    old_s2 = segal.slider2;
}

/****************************************************************/
void
make_blend_lut()
{
    register int i, k;
    float     scale1, scale2;

    /* compute scale factor for the color map and slider */
/* easier to understand (maybe ?) but slower
   j = (int) ((float) img[i] * (s1 / 100.));
   k = ((j * NGRAY) / 257.);
*/

    scale1 = ((segal.slider1 / 100.) * (float) NGRAY) / 257.;
    scale2 = ((segal.slider2 / 100.) * (float) NGRAY) / 257.;

    for (i = 0; i < 256; i++) {
	k = i * scale1;
	if (k >= NGRAY)
	    k = NGRAY - 1;
	gray_lut[i] = k;
	k = i * scale2;
	if (k >= NGRAY)
	    k = NGRAY - 1;
	blend_lut[i] = NGRAY + k;
    }
}

/**************************************************************/
void
blend(img, mask, out, size)	/* maps directly to colormap */
    u_char   *img, *mask, *out;
    int       size;
{
    register int i, k;
#ifdef HALFTONE_BLEND
    u_char    halftone_blend();
#endif

    set_watch_cursor();

    segal.blend_type = (int) xv_get(display_control_win->blend_type, PANEL_VALUE, 0);
    segal.mask_type = (int) xv_get(display_control_win->mask_type, PANEL_VALUE, 0);

    if (verbose)
	fprintf(stderr, "blending images...  %d, %d \n",
		segal.slider1, segal.slider2);

    for (i = 0; i < size; i++) {
	if ((mask[i] == 0 && segal.mask_type == 1) ||
	    (mask[i] > 0 && segal.mask_type == 0)) {
	    k = gray_lut[img[i]]; 
	    out[i] = (u_char) colors[k];
	} else {
	    if (segal.blend_type == 1)
		out[i] = (u_char) colors[NGRAY - 1];	/* white */
	    else if (segal.blend_type == 0) {
		k = blend_lut[img[i]];
		out[i] = (u_char) colors[k];
	    }
#ifdef HALFTONE_BLEND
	    else {
		/* halftone */
		k = halftone_blend(img[i], i % segal.cols);
		if (k > 0) {
		    out[i] = (u_char) colors[NGRAY - 1];
		} else {	/* use image value */
		    k = gray_lut[himage.data[y][x]];
		    out[i] = (u_char) colors[k];
		}
	    }
#endif
	}
    }
    unset_watch_cursor();
}

/*************************************************/
u_long
get_blend_pixel(x, y, val)	/* used in painting to get pixel
					 * values */
    int       x, y, val;
{
    int       k;
    u_long    rval;

    if (himage.fp == NULL)
	return (-1);		/* shouldn't be here */

    if ((val == 0 && segal.mask_type == 1) ||
	(val > 0 && segal.mask_type == 0)) {	/* erasing */
	k = gray_lut[himage.data[y][x]];
	rval = colors[k];

    } else {			/* painting */
	if (segal.blend_type == 1)
	    rval = (u_char) colors[NGRAY - 1];
	else if (segal.blend_type == 0) {
	    k = blend_lut[himage.data[y][x]];
	    rval = (u_char) colors[k];
	}
#ifdef HALFTONE_BLEND
	else {
	    /* halftone */
	    k = halftone_blend(himage.data[y][x], x);
	    if (k > 0) {
		rval = (u_char) colors[NGRAY - 1];
	    } else {		/* use image value */
		k = gray_lut[himage.data[y][x]];
		rval = (u_char) colors[k];
	    }
	}
#endif
    }

    return (rval);
}

/*************************************************/
#ifdef HALFTONE_BLEND
u_char
halftone_blend(inval, col)
    u_char    inval;
    int       col;
{
    static int rptr = 0;
    int       k;
    u_char    outval;
    static int init_done = 0;
    void      halftone_init();

    if (!init_done) {
	halftone_init();
	init_done = 1;
    }
    if (col >= segal.cols)
	col = segal.cols - 1;

    k = error[col] + (inval & 0377);
    if (k > rtab[rptr++]) {
	outval = 255;
	k -= 255;
    } else
	outval = 0;

    if (rptr >= RANDOMVALS)
	rptr = 0;

    /* propogate half the error down, half right */
    k /= 2;
    error[col] = k;
    error[col + 1] += k;

    return (outval);
}

/*************************************************/
void
halftone_init()
{
    register int i;
    long      state[32];

    error = (short *) halloc(segal.cols + 1, sizeof(short));
    rtab = (short *) halloc(RANDOMVALS, sizeof(short));

    for (i = 0; i <= segal.cols; i++)
	error[i] = 0;

    /*
     * initialize table of random numbers in the range of pixel values
     */

    H__SRANDOM((unsigned) time((int *) 0));
    for (i = 0; i < RANDOMVALS; i++)
	rtab[i] = H__RANDOM() % 255;
}

#endif
/**********************************************************/
#ifdef OR_BLEND
void
or_images(im1, im2, size)	/* or's 2 images for blend */
 /* not being used */
    u_char   *im1, *im2;
    int       size;
{
    register int i;
    u_char   *blend_buf;

    blend_buf = Calloc(segal.rows * segal.cols, u_char);

    for (i = 0; i < size; i++)
	blend_buf[i] = im1[i] | im2[i];

    map_image_to_lut(blend_buf, blend_image->data, segal.rows * segal.cols);
}

#endif
