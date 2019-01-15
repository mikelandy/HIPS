/* ============= DrawShape . C ==========================
%		Copyright (C)	Jin Guojun		%
%		   All  rights  reserved		%
%	Draw a square in a given image window.		%
%	A Bug in X Window is that in DrawRectangle,	%
%	real width and height are equal to given width	%
%	and height "+ 1".  So if you want to draw a	%
%	10 x 10 rectangle, Draw a 9 x 9 one instead.	%
% AUTHOR:	Jin Guojun - LBL  ===== 4/1/91 ========*/

#include "function.h"

#ifndef	MIN_SUBWINSIDE
#define	MIN_SUBWINSIDE	4
#endif

void
set_superimpose_image(U_IMAGE	*img, superimpose_elems	*si_ep)
{
	img->sub_img = si_ep->e_type;
	img->sub_img_x = si_ep->x0;
	img->sub_img_y = si_ep->y0;
	img->sub_img_w = si_ep->w;
	img->sub_img_h = si_ep->h;
}

void	/* who = 0 for draws, 1 for texts, and -1 for both	*/
superimpose_images(img, who, x0, y0, cw, ch)
U_IMAGE	*img;
{
XRectangle	r;	/*	== Range	*/
char	font_sname[16];	/* font name string	*/
int	sub_img = img->sub_img, x = img->sub_img_x, y = img->sub_img_y,
	h = img->sub_img_h, w = img->sub_img_w,
	font_w = img->font_w, font_h = img->font_h;
register int	i = img->draws;

/* r.x0 = x0;	r.y0 = y0;	r.cs = cw;	r.ls = ch;	*/
r.x = x0;	r.y = y0;	r.width = cw;	r.height = ch;

if (img->superimpose && (i | img->texts))	{
register superimpose_elems*	si_ep = img->superimpose[0];
	XSetClipRectangles(img->dpy, img->gc, 0, 0, &r, 1, No);
    if (i && who<1) while (i--)	{
	XSetForeground(img->dpy, img->gc, si_ep[i].color);
	XSetLineAttributes(img->dpy, img->gc, si_ep[i].elem.draw.line_w,
		LineSolid, CapButt, JoinMiter);
	set_superimpose_image(img, si_ep + i);
	Draws(img, 0, No, img->sub_img);
    }
	XSetLineAttributes(img->dpy, img->gc, 0, 0, CapButt, 0);
    if (who && (i=img->texts))	{
	si_ep = img->superimpose[1];
	while (i--)	{
        	sprintf(font_sname, "%dx%d",
			si_ep[i].elem.text.fw, si_ep[i].elem.text.fh);
		SetFontGetSize(img, font_sname);
		XSetForeground(img->dpy, img->gc, si_ep[i].color);
		set_superimpose_image(img, si_ep + i);
		Draws(img, si_ep[i].elem.text.content, No, img->sub_img);
	}
    }
    XSetClipMask(img->dpy, img->gc, None);
}
img->sub_img = sub_img;
img->sub_img_x = x;	img->sub_img_y = y;
img->sub_img_w = w;	img->sub_img_h = h;
sprintf(font_sname, "%dx%d", font_w, font_h);
SetFontGetSize(img, font_sname);
}

void
superimpose_handle(register Image* img, register int n, int x, int y)
{
if (img->superimpose)	{
int	m=0, mx, my=0;
register superimpose_elems* si_ep = img->superimpose[n<0];

	if (img->sub_img)	Draws(img, 0, 1, img->sub_img);
	if (n < 0)	{
	char	font_sname[16];	/* font name string	*/
		n = ~n;	/* text	*/
		sprintf(font_sname, "%dx%d",
			si_ep[n].elem.text.fw, si_ep[n].elem.text.fh);
		SetFontGetSize(img, font_sname);
	} else	n--,	/* draw	*/
XSetLineAttributes(img->dpy, img->gc, my=si_ep[n].elem.draw.line_w, 0, 0, 0);
XSetForeground(img->dpy, img->gc, si_ep[n].color);
set_superimpose_image(img, si_ep + n);
while (!ImageEvent(img, ButtonAction));/* X Events responding time is 200 ms */
XBell(img->dpy, 50);

do	{	/* move objects	*/
XButtonEvent*	xb = (XButtonEvent *)img->event;
char	*t = img->sub_img<0 ? si_ep[n].elem.text.content : 0;
	if (ImageEvent(img, PointerMotionMask))	{	/* object moved */
		Draws(img, t, 1, img->sub_img);
		img->sub_img_x = si_ep[n].x0 + xb->x - x;
		img->sub_img_y = si_ep[n].y0 + xb->y - y;
		Draws(img, t, 1, img->sub_img);
		img->update = m = True;
	}
} while (!ImageEvent(img, ButtonAction));
RemoveImageEvent(img, ButtonAction);
if (m)	{	/* updated	*/
register int	w = img->sub_img_x,	h = img->sub_img_y;
	m = my;
	x = MIN(si_ep[n].x0, w) - m;
	y = MIN(si_ep[n].y0, h) - m;
	mx = MAX(si_ep[n].x0, w);
	my = MAX(si_ep[n].y0, h);
	si_ep[n].x0 = w,	si_ep[n].y0 = h;
	w = img->sub_img_w;	h = img->sub_img_h;
	if ((n=si_ep[n].h) < 0)	y += n,	my -= n;
	if (n = img->sub_img==DrawsArc)
		x -= w,	y -= h;
	else if (img->sub_img < 0)	/* text	*/
		x -= img->font_w,	y -= img->font_h;
	w = mx + (w+m << n) - x;	mx = x;
	h = my + (h+m << n) - y;	my = y;
	toRELATIVExy(img, x, y);
        if (img->refresh_pixmap)
		XCopyArea(img->dpy, img->refresh_pixmap, img->win, img->gc,
			mx, my, w, h, x, y);
        else	XPutImage(img->dpy, img->win, img->gc, img->image,
			mx, my, x, y, w, h);
	superimpose_images(img, -1, x, y, w, h);
}
img->sub_img = 0;
}
}


/*	Draws Text string, Arc, Line, and Rectangle.
%	img->sub_img_? must be set before this routine called.
*/
Draws(register Image	*img, register int	x, int revs, int tp)
{
int	y, w=img->sub_img_w, h=img->sub_img_h, stp=x;
register float	f = img->mag_fact;
if (f<0)	f = -1/f;
if (x<1)	x = MIN_SUBWINSIDE;

	if (tp>0 && tp != DrawsLine && ((w*=f)<x || (h*=f)<x))
		return	False;

	if (revs)
		XSetFunction(img->dpy, img->gc, revs>1 ? revs : GXinvert);
	x = (img->sub_img_x - img->mag_x) * f;
	y = (img->sub_img_y - img->mag_y) * f;
	toRELATIVExy(img, x, y);

#define	XDX	img->dpy,img->win,img->gc

	if (tp < 0)	{
		w = strlen(stp);	h = img->font_h;
		do {
		register int	i = (int) strchr(stp, CR) - stp;
			w -= abs(i);
			if (i<0)	i = strlen(stp) - 1;
			XDrawString(XDX, x, y, stp, i);
			y += h;	stp += ++i;
		} while (w>0);
	} else if (tp==DrawsLine)	XDrawLine(XDX, x, y, x+w, y+h);
	else if (tp==DrawsArc)
		XDrawArc(XDX, x-w, y-h, w<<1, h<<1, 0, 360<<6);
	else	XDrawRectangle(XDX, x, y, --w, --h);
	XSetFunction(img->dpy, img->gc, GXcopy);
return	tp;
}

	/*=======================================
	%	Draw a vertical line (mark)	%
	%	in a given image window		%
	=======================================*/
DrawVMark(img, x, clean)
Image	*img;
{
if (x == img->mark_x)	return	0;

XSetFunction(img->dpy, img->gc, GXinvert /* GXxor */);
if (clean)	{
	if (img->sub_img)
	    XDrawLine(img->dpy, img->win, img->gc, img->mark_x, 0,
		img->mark_x, img->height);
	img->mark_x = x;
}
XDrawLine(img->dpy, img->win, img->gc, img->mark_x, 0,
	img->mark_x, img->height);
XSetFunction(img->dpy, img->gc, GXcopy);
return	x;
}

exposure_r(img, sub_win, x, y, w, h, cdp)
register Image	*img;
int	(*sub_win)();
{
int	x0 = x,	y0 = y;
	toREALxy(img, x0, y0);
	if (cdp && img->refresh_pixmap)
		XCopyArea(img->dpy, img->refresh_pixmap, img->win, img->gc,
			x0, y0, w, h, x, y);
	else	XPutImage(img->dpy, img->win, img->gc, img->image,
			x0, y0, x, y, w, h);
	if (img->sub_img > 0) {	/* ignore the text patterns	*/
	XRectangle	r;
		r.x = x;	r.y = y;
		r.width = w;	r.height = h;
		XSetClipRectangles(img->dpy, img->gc, 0, 0, &r, 1, Unsorted);
		sub_win(img, 0, 1, img->sub_img);
		XSetClipMask(img->dpy, img->gc, None);
	}
	else if (abs(img->tmp_offset) > img->font_h<<1)	/* ??? */
		img->tmp_offset = 0;
	superimpose_images(img, -1, x, y, w, h);
	Draw_ImageScrollBars(img);
}

win_exposure(register XExposeEvent *expose, Image *img, int (*sub_win)())
{
if (img)	{
register int	modulo = expose->x & 3, w = expose->width;
	if (modulo) {
		expose->x -= modulo;
		w += modulo;
	}
	if (modulo = w & 3)	w += 4 - modulo;
	if (!sub_win)	sub_win = (int(*)())Draws;
	exposure_r(img, sub_win, expose->x, expose->y, w, expose->height,
		img->color_dpy);
}
}

void
superimpose_add_elem(img, cv, color, lw, shape, y, x, rows, cols)
register Image*	img;
color_channel	cv;
{
int	i = shape<0, who = i?img->texts++:img->draws++;
register superimpose_elems*	si_ep;
if (!img->superimpose)	img->superimpose = ZALLOC(2, sizeof(caddr_t), No);
verify_buffer_size(img->superimpose+i, -(who+1), sizeof(*si_ep));
	si_ep = img->superimpose[i] + who;
	si_ep->color = color;
	si_ep->v = cv;
	si_ep->e_type = shape;
toREALxy(img, x, y);
if (i)	{
	si_ep->elem.text.content = str_save((char*)lw);
	si_ep->y0 = y;
	si_ep->x0 = x;
	si_ep->h = (si_ep->rows = rows) * (si_ep->elem.text.fh=img->font_h);
	si_ep->w = (si_ep->cols = cols) * (si_ep->elem.text.fw=img->font_w);
	si_ep->elem.text.ascent = img->ascent;
} else	{
	si_ep->elem.draw.line_w = lw;
	si_ep->elem.draw.join = x;
	si_ep->elem.draw.style = y;
	si_ep->y0 = img->sub_img_y;
	si_ep->x0 = img->sub_img_x;
	si_ep->h = img->sub_img_h;
	si_ep->w = img->sub_img_w;
	}
}

#ifndef	FLUSH_UNIT
#define	FLUSH_UNIT	400000
#endif

void
FlushObject(img, who, ft, freq)
Image*	img;
{
char	*t=NULL;
int	i;
superimpose_elems*	si_ep;
	which_si_elem(i, who);
	si_ep = img->superimpose[i] + who;
	set_superimpose_image(img, si_ep);
	if (i)	t = si_ep->elem.text.content;
	while (ft--)	{
		Draws(img, t, Yes, img->sub_img);
		XBell(img->dpy, 0);	XFlush(img->dpy);
		Delay_Clear(img, win_exposure, &img, 1, freq);
		udelay(0, FLUSH_UNIT / freq);
		Draws(img, t, Yes, img->sub_img);	XFlush(img->dpy);
		Delay_Clear(img, win_exposure, &img, 1, freq);
		udelay(0, FLUSH_UNIT / freq);
	}
}

superimpose_delete_elem(register Image*	img, int who)
{
int	i, l;
superimpose_elems*	si_ep;
	which_si_elem(i, who);
	si_ep = img->superimpose[i] + who;
	if (i)	CFREE(si_ep->elem.text.content),
		l = img->texts--;
	else	l = img->draws--;
	for (l -= who; --l; si_ep++)
		memcpy(si_ep, si_ep+1, sizeof(*si_ep));
	img->update = True;
}

/* returns	pos (1..n+1, draws), or neg (~(0..n), text)	*/
on_superimpose_elem(img, x0, y0)
U_IMAGE*	img;
register int	x0, y0;
{
if (img->superimpose)	{
register superimpose_elems*	si_ep = img->superimpose[0];
register int	i = img->draws;

	toREALxy(img, x0, y0);
	while (i--)	{
	register int	lx, ly=lx=0, lw=si_ep[i].elem.draw.line_w;
	    if (si_ep[i].e_type==DrawsArc)
		lx = si_ep[i].w,	ly = si_ep[i].h;
	    if (si_ep[i].h > 0 &&
		si_ep[i].y0 - ly - lw < y0 && si_ep[i].x0 - lx - lw < x0 &&
		si_ep[i].y0 + si_ep[i].h + lw > y0 &&
		si_ep[i].x0 + si_ep[i].w + lw > x0	||
		si_ep[i].h < 0 &&
		si_ep[i].y0 + si_ep[i].h - lw < y0 && si_ep[i].x0 - lw < x0 &&
		si_ep[i].y0 + lw > y0 &&
		si_ep[i].x0 + si_ep[i].w + lw > x0)	return	++i;
	}
	i = img->texts;
	si_ep = img->superimpose[1];
	while (i--)
	    if (si_ep[i].y0 - si_ep[i].elem.text.fh < y0 &&
		si_ep[i].x0 - si_ep[i].elem.text.fw < x0 &&
		si_ep[i].y0 + si_ep[i].h > y0 &&
		si_ep[i].x0 + si_ep[i].w > x0)	return	~i;
}
return	False;
}
