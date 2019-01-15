/*	MOVIE . C
#
*/

#include "panel.h"

#define	decl_func(func_name)	\
func_name(int start, image_information **img_info, int movie_udelay, int n, longword mask, XEvent *event, x_bool *found_event)

decl_func(action_flip_forward)
{
image_information	*img;
    if (mask) *found_event = False;

    if (start == n-1)
	start = 0;
    while(start < n) {
	set_timer(movie_udelay);
	img = img_info[start++];
	handle_exposure(img, Draws, 0, 0, img->w, img->h, img->h);
	if (img->IN_FP != stdin && movie_udelay) /* as fast as possible */
		XStoreName(img->dpy, img->frame, img->title);
	XFlush(img->dpy);
	if (mask && XCheckMaskEvent(img->dpy, mask, event))	{
	    *found_event = True;
	    break;
	}
	wait_timer();
    }
    if (mask && *found_event && start < n)
	return	start;
    else	return	--start;
}

decl_func(action_flip_backward)
{
image_information	*img;
    if (mask) *found_event = False;

    if (!start++)
	start = n;
    while (start--)	{
	set_timer(movie_udelay);
	img = img_info[start];
	handle_exposure(img, Draws, 0, 0, img->w, img->h, img->h);
	if (img->IN_FP != stdin && movie_udelay) /* same as forward	*/
	    XStoreName(img->dpy, img->frame, img->title);
	XFlush(img->dpy);
	if (mask && XCheckMaskEvent(img->dpy, mask, event))	{
	    *found_event = True;
	    break;
	}
	wait_timer();
    }
    if (mask && *found_event)
	return	start;
    else return	start + 1;
}

action_movie_cycle(start, img_info, n, flip_forward, movie_udelay, bounce_mode)
image_information	**img_info;
int	n, movie_udelay;
x_bool	flip_forward;
{
int	found_event;
XEvent	event;

    do {
	start = (* (flip_forward ? action_flip_forward : action_flip_backward))
	(start, img_info, movie_udelay, n, ButtonPressMask | KeyPressMask,
		&event, &found_event);
	if (bounce_mode && !found_event)	{
		flip_forward = !flip_forward;
		if (n > 1)
		    start = (flip_forward) ? start++ : start--;
	}
	XSync(img_info[0]->dpy, False);
    } while (!found_event);
return	start;
}
