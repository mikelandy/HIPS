/*	IMAGE . C
#
%	Copyright (c)	Jin Guojun -	All rights reserved
%
%	image & window creating and handling
%
% AUTHOR:	Jin Guojun - LBL	4/1/91
*/

#include "panel.h"

#ifndef	ScrollBarSIZE
#define	ScrollBarSIZE	HandleWidth
#endif
#ifndef	MaxWindowSIZE
#define	MaxWindowSIZE	768
#endif

WinAttribute	Monitor[2];
ScrollBar	**SB_Stack;
int	SB_SP;

XImage*
get_XImage(img, w, h, buf_need, name)
Image	*img;
caddr_t	name;
{
XImage	*image = XCreateImage(img->dpy, img->dpy_visual,
			(img->binary_img ? 1 : img->dpy_depth),
			(img->binary_img ? XYBitmap : ZPixmap),
			0, NULL, w, h, img->dpy_depth==8 ? 8 : 32, 0);

	if (image && buf_need &&
	    !(image->data = NZALLOC(image->bytes_per_line, h, No))) {
		perror("malloc image->data");
		XDestroyImage(image);	image = NULL;
	}
	DEBUGMESSAGE("%s = %d x %d\n", name, w, h);
return	image;
}

get_iconsize(Image	*img, int width)
{
XIconSize	*icon_sizes;
int	count, factor, height = DESIRED_ICON_HEIGHT;

	if (width<32 || width>384)
		width = DESIRED_ICON_WIDTH;

    if (XGetIconSizes(img->dpy, Root_window, &icon_sizes, &count) && count > 0){
	while (count--)
	    if (icon_sizes[count].min_width <= width
		&& icon_sizes[count].max_width >= width
		&& icon_sizes[count].min_height <= height
		&& icon_sizes[count].max_height >= height)
		break;

	if (count < 0) {
		width = icon_sizes[0].max_width;
		height= icon_sizes[0].max_height;
	}
    }

	factor = img->width / width;
	if (factor < img->height / height)
	    factor = img->height / height;
	factor += factor & 1 | !factor;

/* icon_height need plus 1, otherwise get BadValue or BadGC problem	*/
	img->icon_height = img->height / factor + 1;
	img->icon_width = img->width / factor;
return	factor;
}

void	/*	initializing Maximum window size	*/
MaximumWindowSize(register Image *I,	XSetWindowAttributes	*wattr,
	int cw, int dpy_depth)
{
register int	l, v;
if (!I->event)	return;	/* no image	*/
if (!I->resize_h)	/* ensure resize_? to be set	*/
	I->resize_h = MIN(I->height, MaxWindowSIZE);
if (!I->resize_w)
	I->resize_w = MIN(I->width, MaxWindowSIZE);
l = I->width != I->resize_w,	v = I->height != I->resize_h;
    if (l | v)	{
	v += l & v;
	l = ScrollBarSIZE;
	XResizeWindow(I->dpy, I->frame, I->resize_w + l, I->resize_h + l);
#ifdef	SCROLLBAR_on_CANVAS
	wattr->override_redirect = True;
	I->win = XCreateWindow(I->dpy, I->frame, 0, 0,
		I->width + l, I->height + l, 0, dpy_depth, InputOutput,
		I->dpy_visual, cw | CWOverrideRedirect, wattr);
#endif
	I->stack_num = SB_SP++;
	verify_buffer_size(&SB_Stack, sizeof(*SB_Stack), SB_SP, "sb_stack");
	I->parts = (PanelParts *) (SB_Stack[I->stack_num] =
		CreateScrollBar(I, 0, I->resize_h, I->resize_w, 0, l,
		I->resize_w, I->resize_h, v, I->name,
		darkGray ? darkGray : Black, Gray ? Gray : White));
    }
}

Font
SetFontGetSize(AnyWindow* aw, char* fontname)
{
/* Font = u_long defined in L42 X.h
int	cdir, ascent, descent;
Font	fid = XLoadFont(aw->dpy, fontname);
XCharStruct	XCharS;
XSetFont(dpy, gc, fid);
XQueryTextExtents(aw->dpy, FontP->fid, "H", 1, &cdir, &ascent, &descent, &XCharS);
*fw = XCharS.width;
*fh = ascent + descent;
*/
XFontStruct	*FontP = XLoadQueryFont(aw->dpy, fontname);

if (!FontP)
	FontP = XLoadQueryFont(aw->dpy, "fixed");
XSetFont(aw->dpy, aw->gc, FontP->fid);
aw->font_w = FontP->max_bounds.width;
aw->font_h = (aw->ascent=FontP->ascent) + FontP->descent;
DEBUGMESSAGE("GC[%d] fontW=%d, fontH=%d\n", aw->gc, aw->font_w, aw->font_h);
XFreeFont(aw->dpy, FontP);
return	FontP->fid;	/* use freed memory Yh	*/
}

#define	BaseCW	CWBackPixel | CWBorderPixel | CWEventMask
#define	GC_Mask	GCFunction | GCPlaneMask | GCForeground | GCBackground

CreateWindow(AnyWindow*	W, WinAttribute	*parent, Cursor	cursor,
		int mask, bool fixedframe, bool wm_flag)
{
XGCValues	gc_val;		/* p4 Xlib.h	*/
XSetWindowAttributes	wattr;	/* for window self	*/
XSizeHints	szhints;	/* for window manager	*/
XWMHints	wmhints;
Display	*dpy = parent->dpy;
int	scr = parent->screen, dpy_depth = XDefaultDepth(dpy, scr),
	cw = BaseCW | CWCursor | CWColormap,
	ww = W->width, wh = W->height;

if (!cursor)
	cursor = XCreateFontCursor(dpy, XC_heart);
/* Get window attributes, these wattr.{ebc} MUST be set up here */
wattr.event_mask = mask | ExposureMask;
wattr.background_pixel = Black;
wattr.border_pixel = White;
wattr.colormap = parent->cmap;
wattr.cursor = cursor;
/*=======================================================
%	set window manager hints for normal window	%
%	If size not been set, icon will not work ??	%
%	Any flag is set, set corresponding properties	%
%	also. Otherwise, window manager will dead.	%
=======================================================*/
szhints.flags = PPosition | PSize | PMinSize | PMaxSize;
if (!W->frame | !W->win)	{
	if (!(W->frame = W->win = XCreateWindow(dpy, parent->root,
		W->x0, W->y0, ww, wh, 5,
		dpy_depth, InputOutput, parent->visual, cw, &wattr)))
	return	prgmerr(0, "%s -> win", W->name);

szhints.max_width = szhints.min_width = szhints.width = ww;
szhints.max_height= szhints.min_height= szhints.height = wh;
if (!fixedframe)	{
	szhints.min_width = MIN(ww, 512);
	szhints.min_height = MIN(wh, 256);
	szhints.max_width = MAX(MIN(DisplayWidth(dpy, scr), (ww+16)), 512);
	szhints.max_height = MAX(MIN(DisplayHeight(dpy, scr), (wh+16)), 256);
}
szhints.x = szhints.y = 0;
XSetStandardProperties(dpy, W->frame, W->name, W->name, None, NULL, 0, &szhints);
}
MaximumWindowSize(W, &wattr, cw, dpy_depth);

if (W->icon_width > 7 && W->icon_height > 7)	{
	wattr.event_mask = ExposureMask;
	W->icon = XCreateWindow(dpy, parent->root, 0, 0,
		W->icon_width, W->icon_height, 0, dpy_depth,
		InputOutput, parent->visual, BaseCW, &wattr);
	if (!W->icon)	return	prgmerr(0, "%s -> icon", W->name);
}
/* set hints for iconified window */
if (W->icon)	{
	szhints.flags = PSize;
	szhints.width = szhints.max_width = W->icon_width;
	szhints.height = szhints.max_height = W->icon_height;
	szhints.min_height = szhints.min_width = 32;
	{
	char	ibuf[64];	/* make Icon name */
	sprintf(ibuf, "Icon-%s", W->name);
	XSetStandardProperties(dpy, W->icon, ibuf, ibuf,
			None, NULL, 0, &szhints);
	}
}
wmhints.icon_window = W->icon;
wmhints.icon_x = szhints.max_width - 64;
wmhints.icon_y = szhints.max_height >> 1;
wmhints.flags = InputHint | wm_flag;
wmhints.input = True;
XSetWMHints(dpy, W->win, &wmhints);

gc_val.function = GXcopy;
gc_val.plane_mask = AllPlanes;
gc_val.foreground = fixedframe ? black1 : White;
gc_val.background = fixedframe ? white1 : Black;
W->gc = XCreateGC(dpy, W->win, GC_Mask, &gc_val);
if (W->icon)
	W->igc = XCreateGC(dpy, W->icon, GC_Mask, &gc_val);

return	SetFontGetSize(W, "8x13");
}

/*===============================================================
%	Create Window, GC and Image Copy for given image data	%
===============================================================*/
Image*
CreateImage(ip, name, parent,	width, height, icon_width, WEMask, need)
Image	**ip;
char	*name;
WinAttribute	*parent;
{
Image	*I;
int	bsize=height*width;

if (*ip==NULL)	*ip = ZALLOC(sizeof(**ip), 1, "Image");
I = *ip;
I->resize_w = MIN((I->width = width), MaxWindowSIZE);
I->resize_h = MIN((I->height = height), MaxWindowSIZE);
I->dpy = parent->dpy;
I->colormap = parent->cmap;
if (!I->dpy_visual)
	I->dpy_visual = parent->visual;
if (!I->dpy_depth)
	I->dpy_depth = parent->dpy_depth;
if (!name || !*name)	name = "standard-in";
I->name = str_save(name);

if (icon_width)	{
	get_iconsize(I, icon_width);
	I->icon_image = get_XImage(I, I->icon_width, I->icon_height, Yes, "icon");
	I->icon_buf = (byte *) I->icon_image->data;
}
I->image = get_XImage(I, I->width, I->height, need, name);
I->img_buf = (byte *) I->image->data;
I->event++;	/* flag to generate canvas	*/
u_window_api(I, parent, 0);	/* inherit windows!	*/
CreateWindow(I, parent, 0, WEMask | StructureNotifyMask, 0,
		IconWindowHint | IconPositionHint);
return	I;
}

#define	freable(i)	if (i)	CFREE(i)

void
DestroyImage(Image	*img)	/* can be for window only!	*/
{
	freable(img->icon_buf);
	freable(img->img_buf);	/* === img->image->data	*/
	freable(img->hist);
	if (img->image)		XDestroyImage(img->image),
				XFreeColormap(img->dpy, img->colormap);
	if (img->gc)	XFreeGC(img->dpy, img->gc),
			XDestroyWindow(img->dpy, img->win);
	if (img->frame && img->frame != img->win)
		XDestroyWindow(img->dpy, img->frame);
	if (img->igc)	XFreeGC(img->dpy, img->igc),
			XDestroyWindow(img->dpy, img->icon);
	if (img->icon_image)	XDestroyImage(img->icon_image);
	img->frame = img->win = img->icon = img->colormap = 0;
	img->gc = img->igc = (GC)(img->image = NULL);
}
/*===============================
%	Destroy Color Image	%
===============================*/
#define	freablePixmap(i, p)	\
	if (i->p)	XFreePixmap(i->dpy, i->p),	i->p = 0
Window
DestroyColorImage(Image *img)
{
Window	window=img->win;
XFreeColors(img->dpy, img->colormap, img->pixel_table, img->entries, 0);
	CFREEnNULL(img->pixel_table);
	DestroyImage(img);
	freable(img->title);
	freable(img->scan_data);
	freablePixmap(img, pixmap);
	freablePixmap(img, mag_pixmap);
	freablePixmap(img, icn_pixmap);
return	window;
}

/*=======================================================
%	RETURN image array position if image found	%
%		Otherwise, return EOF (-1)		%
=======================================================*/
WhichImage(Window	win, Image	**imgp, register int	num)
{
while (num--)
if (win==imgp[num]->frame || win==imgp[num]->win || win==imgp[num]->icon)
	break;
return	num;
}

/*===============================================================
%	Load Icon buffer dynamically for both b/w & color	%
===============================================================*/
void
LoadIcon(register Image	*I)
{
register int	h, w, fact = I->width / I->icon_width;
register char	*dp = I->icon_image->data;

for (h=0; h < I->icon_height; h++)	{
register char	*bp = I->image->data + h * I->width * fact;
    for (w=0; w < I->icon_width; w++, bp+=fact)
	*dp++ = *bp;
}
}
