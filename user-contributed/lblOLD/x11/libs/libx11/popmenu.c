/*	POP MENU . C
#
%	Copyright (c)	Jin, Guojun
%
% AUTHOR:	Jin Guojun - Lawrence Berkeley Laboratory	4/1/91
*/

#include "panel.h"

int	PopLineHeight;
#define	PmMg	2

PopMenu*
CreatePopMenu (Panel* pan, WinAttribute	*parent,
	char*	name, char *menu[], int num, bool freemenu)
{
register int	i, w=0, sl;
register PopMenu*	pm = ZALLOC(sizeof(*pm), 1, name);
	pm->pw_type = freemenu ? PW_POPMENU | PW_FREEMEM : PW_POPMENU;
	pm->bgcolor = White;
	pm->fgcolor = Black;
	pm->items = i = num;
	while (i--)
	    if ((sl=strlen(menu[i])) > w)	w = sl;
	if ((sl=strlen(name)+4) > w)	w = sl;
	pm->name = str_save(name);
	pm->dpy = pan->dpy;
	pm->x0 = pan->x0;
	pm->y0 = pan->y0 + pan->height + 8;
	PopLineHeight = 15 + PmMg;
	pm->w = w * 9 + TextLMargin;
	pm->h = num * PopLineHeight;
	CreateWindow(pm, parent, 0, PointerMotionMask | ButtonAction, True, 0);
	XSetWindowBackground(parent->dpy, pm->win, pm->bgcolor);
	SetFontGetSize(pm, "9x15");
	if (pm->font_h > 15 || pm->font_w > 9)	{
		PopLineHeight = pm->font_h + PmMg;
		XResizeWindow(pm->dpy, pm->frame,
			pm->w = w * pm->font_w + TextLMargin,
			pm->h = num * PopLineHeight);
	}
pm->menu = menu;
return	pm;
}

/*	reset menus for popmenu	*/
ChangePopMenu(PopMenu *pm, char *menu[], int num)
{
    if (pm)	{
	register int	i, sl, w = (pm->w - TextLMargin) / pm->font_w;
	pm->items = i = num;
	while (i--)
	    if ((sl=strlen(menu[i])) > w)	w = sl;
	XResizeWindow(pm->dpy, pm->frame,
		pm->w = w * pm->font_w + TextLMargin,
		pm->h = num * PopLineHeight);
	pm->menu = menu;
	return	0;
    }
return	EOF;
}


#ifdef	__STDC__
#define	PrepColor(c)	XSetForeground(pm->dpy, pm->gc, pm->##c##gcolor);
#else
#define	PrepColor(c)	XSetForeground(pm->dpy, pm->gc, pm->/**/c/**/gcolor);
#endif
#define	FillItem(i)	XFillRectangle(pm->dpy, pm->win, pm->gc,\
				0, (i)*PopLineHeight, pm->w, PopLineHeight);
#define	WriteItem(i)	{	register char*	p=pm->menu[i];	\
	XDrawString(pm->dpy, pm->win, pm->gc,	\
		PmMg, (i)*PopLineHeight+pm->font_h, p, strlen(p));	}
#define	DrawItem(i, g)	{	FillItem(i);	PrepColor(g);	WriteItem(i); }


PopingMenu(PopMenu* pm, int n_th /* place cursor on n_th item */,
	void	(*expos)(),	Image	**imgp,	int	ni)
{
Window	rw;
int	cx, cy, bw, dp;
XEvent	event;
register int	i;

pm->interface = (VType*)&event;
TopWindow(pm,NULL, True);
XGetGeometry(pm->dpy, pm->win, &rw, &pm->x0, &pm->y0, &cx, &cy, &bw, &dp);
if (n_th < 0)
	n_th = - n_th;	/* kludge way to bypass waiting	*/
else	while(!XCheckMaskEvent(pm->dpy, ButtonAction, &event));
XWarpPointer(pm->dpy, event.xbutton.window, pm->win, 0, 0, 0, 0,
	pm->w>>1, --n_th * PopLineHeight);
PrepColor(b);
XFillRectangle(pm->dpy, pm->win, pm->gc, 0, 0, pm->w, pm->h);
PrepColor(f);
for (i=pm->items; i--;)
	WriteItem(i);

while(!XCheckMaskEvent(pm->dpy, ButtonReleaseMask, &event)){
    if (XCheckMaskEvent(pm->dpy, PointerMotionMask, &event))	{
	cx = event.xmotion.x;
	cy = event.xmotion.y;
	pm->which = cy / (pm->font_h+PmMg) + 1;
	if (cx>0 && cx<pm->w && cy>0 && cy<pm->h && i != pm->which)	{
		if (i-- > 0)
			DrawItem(i, f);
		i = pm->which;	/* high light on which item	*/
		DrawItem(i-1, b);
	}
    }
    if (expos && XCheckMaskEvent(pm->dpy, ExposureMask, &event))	{
	register Image*	ip=NULL;
	register int	in;
	if (imgp && (in=WhichImage(event.xany.window, imgp, ni)) >= 0)
		ip = imgp[in];
	expos(&event, ip);
    }
}
while(!XCheckMaskEvent(pm->dpy, ButtonAction, &event));
if (!(pm->pw_type & PW_FREEMEM)) /* don't hide it if createed dynamically */
	HidingPopMenu(pm),
	pm->which = 0;
return	i;
}

#undef	PmMg	/* for included in panel.c	*/
#undef	PrepColor
#undef	FillItem
#undef	WriteItem
#undef	DrawItem
