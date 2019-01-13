/*
 * mframe.c -- support for managing multiple frames
 *
 */

#include "display.h"
#include "ui.h"

adv_frame()
{
    set_watch_cursor();
    orig_img->cframe++;
    /* entire data already loaded, so just need to move pointer */
    orig_img->data = orig_img->data + (orig_img->width * orig_img->height *
				       orig_img->dsize);

    extrema(orig_img);

    make_lut(orig_img, NCOLORS);

    orig_ximg = mk_x_img(orig_img, orig_ximg, 0);
    disp_ximg = mk_x_img(orig_img, disp_ximg, 1);
    printf("frame %d loaded\n", orig_img->cframe);

    label_img(orig_img);
    disp_img();
    unset_watch_cursor();

#ifdef DEBUG
    printf("adv_frame\n");
#endif
}

rev_frame()
{
    set_watch_cursor();
    orig_img->cframe--;
    orig_img->data = orig_img->data - (orig_img->width * orig_img->height *
				       orig_img->dsize);
    extrema(orig_img);

    make_lut(orig_img, NCOLORS);

    orig_ximg = mk_x_img(orig_img, orig_ximg, 0);
    disp_ximg = mk_x_img(orig_img, disp_ximg, 1);
    printf("frame %d loaded\n", orig_img->cframe);

    label_img(orig_img);
    disp_img();
    unset_watch_cursor();

#ifdef DEBUG
    printf("rev_frame\n");
#endif
}

/* get the current frame number */
curframe()
{
    if (orig_img != NULL)
	return orig_img->cframe;
    else
	return 0;
}

/* set the frame buttons based on the position in the image */
set_frame_buttons(img)
    struct img_data *img;
{
    if (img->cframe > 1)
	xv_set(base_win->prev,
	       PANEL_INACTIVE, FALSE,
	       NULL);
    else
	xv_set(base_win->prev,
	       PANEL_INACTIVE, TRUE,
	       NULL);
    if (img->cframe < img->nframes)
	xv_set(base_win->next,
	       PANEL_INACTIVE, FALSE,
	       NULL);
    else
	xv_set(base_win->next,
	       PANEL_INACTIVE, TRUE,
	       NULL);
}
