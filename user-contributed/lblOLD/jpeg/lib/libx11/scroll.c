/*	SCROLL . C
#
%	Copyright (c)	1992-1995	Jin Guojun -	All rights reserved
%
% AUTHOR:	Jin Guojun - LBL	4/1/92
*/

#include "panel.h"

#ifdef	SCROLLBAR_on_CANVAS
#define	SonC
#endif	/* short drc name	*/

ScrollBar*
CreateScrollBar(aw, x, y, xv, yv, sbw, sblh, sblv, type, name, bc, rc)
AnyWindow*	aw;
char*	name;
{
register int	i=2;
register ScrollBar*	sb = zalloc(sizeof(*sb) + 12 * sizeof(i), 1, name);
	sb->pw_type = PW_SCROLLBAR;
	sb->hvalid = !((sb->vvalid = sb->type = type) & 1);
	sb->awin = aw;	sb->name = name;
	sb->rx = (int*)(sb + 1);
	sb->ry = sb->rx + i;
	sb->rw = sb->ry + i;
	sb->rl = sb->rw + i;
	sb->bx = sb->rl + i;
	sb->by = sb->bx + i;
	i--;	/* initialize vertical sb	*/
	sb->rx[i] = sb->bx[i] = xv;
	sb->ry[i] = sb->by[i] = yv;
	sb->rl[i] = sblv;
	*sb->rw = sb->rw[i] = sbw ? sbw : HandleWidth;
	i--;	/* init horizontal sb	*/
	sb->rx[i] = sb->bx[i] = x;
	sb->ry[i] = sb->by[i] = y;
	sb->rl[i] = sblh;
	sb->rcolor = sb->vcolor = rc;
	sb->bcolor = bc;
#ifdef	SonC
#define	GetSW(aw,x,y,w,h)	\
		XCreateSimpleWindow(aw->dpy, aw->win, x,y,w,h, 0, None, Black);
	sb->h_swin = GetSW(aw, 0 ,sblv, sblh, sb->rw[0]);
	sb->v_swin = GetSW(aw, sblh, 0, sb->rw[1], sblv);
	sb->lx = sb->ly = 0;	/* dir	*/
#endif
return	sb;
}


static	void
DrawScrollBar(sb, w, l, vert)
register ScrollBar*	sb;
{
register AnyWindow*	awin=sb->awin;
register int	x=sb->rx[vert],	y=sb->ry[vert];
XGCValues	values;
Window	dw = awin->win;
Display	*dpy = awin->dpy;
#define	SetGC(v)	XSetForeground(dpy, awin->gc, v)
	XGetGCValues(dpy, awin->gc, GCForeground, &values);

#ifdef	SonC
	dw = vert ? sb->v_swin : sb->h_swin;
	XMoveResizeWindow(dpy, dw, x+awin->x0, y+awin->y0, w, l);
	XMapWindow(dpy, dw);
	x = y = 0;
#endif
	XClearArea(dpy, dw, x, y, w, l, 0);
	SetGC(sb->rcolor);
	XFillRectangle(dpy, dw, awin->gc, x, y, w, l);	/* draw rail	*/
	if (!vert)	w = l;
	SetGC(sb->bcolor);
	x = sb->bx[vert] - !vert;	y = sb->by[vert] - vert;
#ifdef	SonC
	if (vert)	x=0;
	else	y=0;
#endif
	/* draw bar	*/
	XFillRectangle(dpy, dw, awin->gc, x, y, w + (1<<!vert), w + (vert<<1));
	SetGC(values.foreground);
XFlush(dpy);
}

void
DrawScrollBars(sb, type)
register ScrollBar*	sb;
{
register int	w, l;

if (type && sb->vvalid)	{
	w = sb->rw[1];	l = sb->rl[1] + sb->hvalid * w;
	DrawScrollBar(sb, w, l, 1 /* vertical */);
}
if (!(type & 1) && sb->hvalid)	{
	w = *sb->rw;	l = *sb->rl;
	DrawScrollBar(sb, l, w, 0);
}
}


OnScrollBar(sb, xbutton)
register ScrollBar*	sb;
XButtonEvent	*xbutton;
{
register int	x=xbutton->x, y=xbutton->y, i=1;
#ifdef SonC
	x -= sb->awin->x0;	y -= sb->awin->y0;
#endif
	if (xbutton->window != sb->awin->win)	return	False;
	if (x < sb->bx[i] || x > sb->bx[i] + sb->rw[i] ||
	    y < 0 || y > sb->by[i] + sb->rl[i]) {
		i--;
		if (x < 0 || x > sb->bx[i] + sb->rl[i] ||
			y < sb->by[i] || y > sb->by[i] + sb->rw[i])
		i--;
	}
return	++i;
}


#define	repositon_X0(I, railen, x)	/* resize_? === railen	*/	\
	I->x0 = (I->width - railen) * (x) / I->resize_w
#define	repositon_Y0(I, railen, y)	\
	I->y0 = (I->height - railen) * (y) / I->resize_h

void
SetScrollBar(sb, x, y, type)
register ScrollBar*	sb;
{
register Image*	I = (Image*)sb->awin;
register int	w = -sb->rw[0], resize = sb->rl[1] + w, m=x;
#ifdef	SonC
	x -= I->x0;
	if (!(type&1))
		if (Sign(m - sb->lx) != Sign(x - *sb->bx))	return;
		else	sb->lx = m;
	m = y;
	y -= I->y0;
	if (type)
		if (Sign(m - sb->ly) != Sign(y - sb->by[1]))	return;
	else	sb->ly = m;
#endif

	if (type && y > w && y < resize)
		repositon_Y0(I, resize, sb->by[1] = y);
	resize = *sb->rl;
	if (!(type & 1) && x > w && x < resize)
		repositon_X0(I, resize, *sb->bx = x);

#ifdef	SonC
	if (I->frame != I->win)
		XMoveWindow(I->dpy, I->win, -I->x0, -I->y0),	XFlush(I->dpy);
#endif

DrawScrollBars(sb, type);
}

void
SetScrollBarLength(sb, vert, len)
register ScrollBar *sb;
register int	vert, len;
{
register bool	hv=sb->hvalid;
register Image	*i=(Image*)sb->awin;

	sb->rl[vert] = len + (vert & hv) * *sb->rw;
	if (vert = !vert) {	/* if horizotal, set vertical rail pos */
		sb->rx[vert] = sb->bx[vert] = len;
		len = i->width - len;
		if (!hv)	/* if horizon is not valid, reset x0	*/
			i->x0 = 0;	/* but keeping bar positions	*/
#ifdef	OLD_SCROLLBAR_SSB	/* how much time can we save here ?	*/
		else if (i->x0 > len)	i->x0 = len;
#else
		else	repositon_X0(i, *sb->rl, *sb->bx);	/* good! */
#endif
	} else	{
		sb->ry[vert] = sb->by[vert] = len;
		len = i->height - len;
		if (!sb->vvalid)	i->y0 = 0;
#ifdef	OLD_SCROLLBAR_SSB
		else if (i->y0 > len)	i->y0 = len;
#else
		else	repositon_Y0(i, sb->rl[1], sb->by[1]);
#endif
	}
}


void
Draw_ImageScrollBars(img)
Image	*img;
{
register int	A = img->resize_h < img->height,
		B = img->resize_w < img->width;
	if (img->parts && (A | B))
		DrawScrollBars(img->parts, (A&B) + A);
}

ResizeWindow(img, cevent)
register Image	*img;
register XConfigureEvent*	cevent;
{
    if (cevent->window == img->frame)	{
	register int	offset=0, offsetx=
			img->parts ? *img->parts->scrollbar.rw : offset;
	if (offsetx)	{
		if (img->parts->scrollbar.hvalid = img->width > cevent->width)
			offset = offsetx;
		if (!(img->parts->scrollbar.vvalid=img->height>cevent->height))
			offsetx = 0;
	}
	img->resize_w = cevent->width - offsetx;
	img->resize_h = cevent->height - offset;
	if (img->parts)	{
		SetScrollBarLength(img->parts, 0, img->resize_w);
		SetScrollBarLength(img->parts, 1, img->resize_h);
	}
#ifdef	SonC
	if (img->frame != img->win)
		XMoveWindow(img->dpy, img->win, -img->x0, -img->y0),
		Draw_ImageScrollBars(img);
#endif
	return	1;
    }
return	0;
}
