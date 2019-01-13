/*	Create_Window . c
#
%	extracted from x11_stuff.c
*/

#include "panel.h"

static	Pixel	black_pixel, white_pixel;	/* for GC only */

#define	SelectedMask	ExposureMask | StructureNotifyMask


void	/* must start after colormap allocated to get correct GC color	*/
create_windows(img, window_geometry, stingy)
register image_information *img;
char	*window_geometry;
{
static XClassHint	class_hint = {"getx", "tuner"};
char		default_geometry[32];
int		width, height, x, y, icn_alloc = False;
XSizeHints	size_hints;
unsigned int	mask;
unsigned long	gc_mask, xswa_mask;
XGCValues	gc_values;
XWMHints	wm_hints;
XSetWindowAttributes xswa;
x_bool	new_window = ! (img->window | img->icn_window),
	new_pixmaps = ((img->pixmap == NULL || img->icn_pixmap == NULL)
		&& !img->pixmap_failed);

    if (!img->window) {
	sprintf(default_geometry, "=%dx%d", img->w, img->h);
	mask = XGeometry(Dpy, Screen, window_geometry, default_geometry,
			 IMAGE_BORDERWIDTH, 1, 1, 0, 0,
			 &x, &y, &width, &height);

	size_hints.flags = 0;

	if (mask & (XValue | YValue)) {
	    size_hints.flags |= USPosition;
	    size_hints.x = x;
	    size_hints.y = y;
	} else {
	    size_hints.flags |= PPosition;
	    size_hints.x = x = (DisplayWidth(Dpy, Screen) - width) >> 1;
	    size_hints.y = y = (DisplayHeight(Dpy, Screen) - height) >> 1;
	}

	if (mask & (WidthValue | HeightValue)) {
	    size_hints.flags |=	USSize;
	    size_hints.width = width;
	    size_hints.height = height;
	}
	size_hints.flags |= PMaxSize;
	size_hints.max_width = MAX(MIN(DisplayWidth(Dpy, Screen), (width+16)), 512);
	size_hints.max_height = MAX(MIN(DisplayHeight(Dpy, Screen), (height+16)), 256);

	wm_hints.flags = InputHint;
	wm_hints.input = True;

	DEBUGMESSAGE("window = (%dx%d) %dW x %dH\n", x, y, width, height);

	if (img->colormap==DefaultColormap(Dpy, Screen))
		black_pixel = BlackPixel(Dpy, Screen),
		white_pixel = WhitePixel(Dpy, Screen);
	else	black_pixel = GetGray(Dpy, img->colormap, img->entries, 0),
		white_pixel = GetGray(Dpy, img->colormap, img->entries, 248);

	xswa_mask = CWBackPixel | CWEventMask | CWBorderPixel;
	xswa.background_pixel = black_pixel;
	xswa.border_pixel = white_pixel;
	xswa.event_mask = EnterWindowMask | KeyPressMask | PointerMotionMask
			| ButtonAction | SelectedMask;
	if (img->colormap) {
	    xswa.colormap = img->colormap;
	    xswa_mask |= CWColormap;
	}

	img->frame = img->window = XCreateWindow(Dpy, Root_window, x, y,
			width, height, IMAGE_BORDERWIDTH, img->dpy_depth,
			InputOutput, img->dpy_visual, xswa_mask, &xswa);

	XSetNormalHints(Dpy, img->frame, &size_hints);
	XSetClassHint(Dpy, img->frame, &class_hint);
	XSetWMHints(Dpy, img->frame, &wm_hints);
	XSetIconName(Dpy, img->frame, img->title);
	img->event++;	/* Make image and icon windows of the right size. */
	MaximumWindowSize(img, &xswa, xswa_mask, img->dpy_depth);
    }
    if (img->frame)
	XStoreName(Dpy, img->frame, img->title);

    if (!img->pixmap && new_pixmaps)
	img->pixmap = XCreatePixmap(Dpy, img->window, img->w, img->h,
					img->dpy_depth);
    img->refresh_pixmap = img->pixmap;

    if (!img->gc) {
	gc_mask = 0;
	gc_values.function = GXcopy;		gc_mask |= GCFunction;
	gc_values.plane_mask = AllPlanes;	gc_mask |= GCPlaneMask;
	gc_values.foreground = white_pixel;	gc_mask |= GCForeground;
	gc_values.background = black_pixel;	gc_mask |= GCBackground;

	img->gc = XCreateGC(Dpy, img->window, gc_mask, &gc_values);
    }

    if (!stingy)	{	/* for icon only!	*/
	if (!img->icn_window) {
		xswa.event_mask = SelectedMask;
		xswa_mask = CWEventMask;
		if (img->colormap) {
			xswa.colormap = img->colormap;
			xswa_mask |= CWColormap;
		}
		img->icn_window = XCreateWindow(Dpy, Root_window, 0, 0,
			 img->icn_w, img->icn_h, 0, img->dpy_depth,
			 InputOutput,img->dpy_visual, xswa_mask, &xswa);

		size_hints.flags = PMinSize | PMaxSize;
		size_hints.min_width = img->icn_w;
		size_hints.max_width = img->icn_w;
		size_hints.min_height = img->icn_h;
		size_hints.max_height = img->icn_h;

		XSetNormalHints(Dpy, img->icn_window, &size_hints);
		XStoreName(Dpy, img->icn_window, img->title);
		XSelectInput(Dpy, img->icn_window, SelectedMask);
	}

	if (!img->icn_pixmap) {
		img->icn_pixmap = XCreatePixmap(Dpy, img->icn_window,
				img->icn_w, img->icn_h, img->dpy_depth);
		img->icn_gc = XCreateGC(Dpy, img->icn_window, gc_mask, &gc_values);
		icn_alloc = True;
	}
    }

    if (new_pixmaps || icn_alloc)
	check_pixmap_allocation(img);

    if (new_window) {
	if (img->icn_pixmap && img->icn_window) {
		wm_hints.flags = (StateHint | IconPixmapHint | IconMaskHint |
				IconWindowHint);
		wm_hints.initial_state = NormalState;
		wm_hints.icon_pixmap = img->icn_pixmap;
		wm_hints.icon_window = img->icn_window;
		wm_hints.icon_mask = img->icn_pixmap;

		XSetWMHints(Dpy, img->window, &wm_hints);
	XSetWindowBackgroundPixmap(img->dpy, img->icn_window, img->icn_pixmap);
	}
	SetFontGetSize(img, "8x13");
	get_cursors(img->window);
    }
}
