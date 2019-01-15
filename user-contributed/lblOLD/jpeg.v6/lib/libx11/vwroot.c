/*	VWROOT . C
#
%	Copyright (c)	Jin Guojun
%
%	Display Images on the Root Window.
%
% Author:	Jin Guojun -	Lawrence Berkeley Laboratory
*/

#include "panel.h"
#include <X11/Xatom.h>

static Atom	DPY_VROOT = None;


Window
get_vroot_win(Image* img)
{
Atom	a_type;
Window	rootRet, parentRet, *children, *dupRoot=NULL;
XWindowAttributes	xwa;
int	a_format, i, nchildren;

	/* property __SWM_VROOT would be somehow defined	*/
	DPY_VROOT = XInternAtom(img->dpy, "__SWM_VROOT", False);
	XQueryTree(img->dpy, Root_window, &rootRet, &parentRet,
			&children, &nchildren);

	for (i=0; i < nchildren; i++) {
	word	nitems, left;
	    if (XGetWindowProperty (img->dpy, children[i], DPY_VROOT, 0, 1,
		False, XA_WINDOW, &a_type, &a_format, &nitems,
		&left, (byte **) &dupRoot) == Success && dupRoot)
		break;
	}
	img->win = dupRoot ? *dupRoot : Root_window;
	XGetWindowAttributes(img->dpy, img->win, &xwa);
	img->pixmap = XCreatePixmap(img->dpy, img->win,
		img->width = xwa.width, img->height = xwa.height,
		img->dpy_depth = xwa.depth);
#ifdef	VWROOT_GC
	img->icn_pixmap = img->pixmap;	/* sets for create GC	*/
	img->icon = img->frame = img->win;
	create_window(img, NULL, Yes);	/* creates GC only!	*/
#endif
return	img->win;
}


#define	gray_width	4
#define	gray_height	4
static byte	gray_bits[] = {
	0xf8, 0x1f, 0xe3, 0xc7, 0xcf, 0xf3, 0x9f, 0xf9,
	0xbf, 0xfd, 0x33, 0xcc, 0x7f, 0xfe, 0x7f, 0xfe,
	0x7e, 0x7e, 0x7f, 0xfe, 0x37, 0xec, 0xbb, 0xdd,
	0x07, 0x0d, 0x0b, 0x0e, 0xe3, 0xc7, 0xf8, 0x1f
	};

void
ResetWindowBackground(Window win, int wbg)
{
GC gc;
XGCValues gc_init;
int	ps = wbg & 3;
if (ps == 3 && wbg < 4)	ps--;
	if (win == Root_window)
		XSetWindowBackgroundPixmap(Dpy, win, None);
	else {
	register int	xscale = gray_width << ps,
			yscale = gray_height << ps;
		Pixmap	pix, bfd = XCreateBitmapFromData(Dpy, win,
				gray_bits + 24 - 8 * (wbg&3), xscale, yscale);

		gc_init.foreground = BlackPixel(Dpy, Screen);
		gc_init.background = WhitePixel(Dpy, Screen);
		gc = XCreateGC(Dpy, win, GCForeground|GCBackground, &gc_init);
		pix = XCreatePixmap(Dpy, win, xscale, yscale,
			DefaultDepth(Dpy, Screen));

		XCopyPlane(Dpy, bfd, pix, gc, 0, 0, xscale, yscale, 0, 0, 1L);
		XSetWindowBackgroundPixmap(Dpy, win, pix);

		XFreeGC(Dpy, gc);
		XFreePixmap(Dpy, bfd);
		XFreePixmap(Dpy, pix);
	}
XClearWindow(Dpy, win);
}

/* if (img == NULL)	Map old image up	*/
CopyToRootWindow(Image *parent, Image *img, x_bool map_only)
{
    if (img)	{
	register int	i=img->width, j=img->height;

	if (!parent->pixmap)	get_vroot_win(parent);
	if (!map_only && (i < parent->width || j < parent->height))	{
	register int	w, h;
	Pixmap	pmp = img->pixmap;
	    if (!pmp)	{
		pmp = parent->pixmap;
		XPutImage(img->dpy, pmp, img->gc, img->image, 0,0, 0,0, i, j);
	    }
	    for (w=0; w < parent->width; w+=i)
		for (h=0; h < parent->height; h+=j)
			XCopyArea(img->dpy, pmp, parent->pixmap, img->gc,
				0, 0, i, j, w, h);
	}
	XUnmapWindow(img->dpy, img->win);
    }
    {	register Image	*info = (Image*)parent->histp;
	/* save for alternative images which want to be in Root window.	*/
	parent->histp = (int*) img;
	if (info && info != img)
		XMapWindow(info->dpy, info->win);
    }
XSetWindowBackgroundPixmap(parent->dpy, parent->win, parent->pixmap);
XClearWindow(parent->dpy, parent->win);
}


#define	RetainProName	"RetainRootPro"

RetainWindowProperty(Image *img)
{
if (img->dpy_visual->class & 1)	{
	if (!img->pixmap)
		img->pixmap = XCreatePixmap(img->dpy, img->win, 1, 1, 1);
	if (img->pixmap)	{
	Atom	prop = XInternAtom(img->dpy, RetainProName, False);
	    if (prop)	{	/* PropModeReplace => XSetProperty()	*/
		XChangeProperty(img->dpy, img->win, prop, XA_PIXMAP, 32,
			PropModeReplace, (byte *) &img->pixmap, 1);
		XSetCloseDownMode(img->dpy, RetainPermanent);
		return	0;
	    }
	}
}
return	-1;
}

void
KillRetainedProperty(AnyWindow *aw)
{
/* get the pixmap ID from the RetainProName property, and kill it */
Atom	prop, type;
int	format;
long	n, bafter;
byte *data;

	/* look for retained pixmap to kill */
    if ((prop = XInternAtom(aw->dpy, RetainProName, True)) != None)	{
	if (XGetWindowProperty(aw->dpy, aw->win, prop, 0L, 1L, True,
		 AnyPropertyType, &type, &format, &n, &bafter, &data) == Success
		&& type==XA_PIXMAP && format==32 && n==1 && !bafter && data) {
		XKillClient(aw->dpy, *((Pixmap *)data));
		XDeleteProperty(aw->dpy, aw->win, prop);
	}
    }
}
