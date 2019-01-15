/*	PANELSET . C
#
%	Copyright (c)	Jin Guojun
%
%	Initial and Set Colors for Panel Parts
%
% AUTHOR:	Jin Guojun - LBL	12/12/91
*/

#include "panel.h"

int	precision=384, VCTEntry;/* for default cmap	*/
XColor	qcolor[MaxColors];	/* for public uses.	*/
Colormap	lastmap;

void
set_button_color(b, bc, pc, tc)
Button	*b;
{
register int	i;
if (bc >= 0)
    for (i=b->numb, b->vcolor=bc; i--;)
	b->color[i] = bc;
if (pc>=0)
	b->dimcolor = pc;
if (tc>=0)
	b->tcolor = tc;
}

void	set_pressbutton_color(p, bc, pc, tc)
PressButton	*p;
{
if (bc>=0)
	p->color.act.v = bc;
if (pc>=0)
	p->color.act.p = pc;
if (tc>=0)
	p->tcolor = tc;
}

void	set_slider_color(s, rc, bc, tc)
Slider	*s;
{
if (rc>=0)
	s->rcolor = rc;
if (bc>=0) {
	register int	i = s->numb;
	while (i--)
	    if (bc < 128)
		s->scolor[i] = bc + (s->numb-i << 3);
	    else
		s->scolor[i] = bc - (s->numb-i << 3);
	}
if (tc>=0)
	s->tcolor = tc;
}

void
Set_Panel(ip, item, op1, op2, op3)
VType	*ip;
{
switch (item) {
case BUTTON_COLOR:	/* button color, pressed color, title color */
	set_button_color(ip, op1, op2, op3);
	break;
case PRESS_BUTTON_COLOR:
	set_pressbutton_color(ip, op1, op2, op3);
	break;
case SLIDER_COLOR:	/* rail, bar1, bar2 */
	set_slider_color(ip, op1, op2, op3);
	break;
default:	mesg("invalid panel set\n");
}
}

/*===============================================
%	return the 1st available entry in cmap	%
%	if no entry left, then return 0. system	%
%	colormap must use at lease the first	%
%	a couple of entries for blcak and white	%
===============================================*/
PColor
GetVctEntry(dpy, src, cmap, set_gray)
Display*	dpy;
Colormap	cmap;
{
XColor	ccell;

/*	set standard BLACK, WHITE, and GRAYs	*/
Black = XBlackPixel(dpy, src);
White = XWhitePixel(dpy, src);

ccell.flags = DoAll;
ccell.pixel = 0;
if (XAllocColorCells(dpy, cmap, False, 0, 0, &ccell.pixel, 1))	{
	DEBUGMESSAGE("[%d]entry = %u\n", cmap, ccell.pixel);
	XFreeColors(dpy, cmap, &ccell.pixel, 1, 0);
	if (set_gray && ccell.pixel<32) {
		ccell.red = ccell.green = ccell.blue = 80<<8;
		XAllocColor(dpy, cmap, &ccell);	darkGray = ccell.pixel;
		ccell.red = ccell.green = ccell.blue = 128<<8;
		XAllocColor(dpy, cmap, &ccell);	Visible = ccell.pixel;
		ccell.red = ccell.green = ccell.blue = 168<<8;
		XAllocColor(dpy, cmap, &ccell);	lightGray = ccell.pixel;
		ccell.red = ccell.green = ccell.blue = 200<<8;
		XAllocColor(dpy, cmap, &ccell);	Gray = ccell.pixel++;
	}
}
else	message("[%d] no entry available\n", cmap);
return	VCTEntry=ccell.pixel;
}

/*======================================================
%	Find the the closest color in the colormap.
%	Less Pseudo value get more close color but
%	easy to fail. The better value is 256.
======================================================*/
PColor	GetCloseColor(dpy, cmap, ncolors, uct, r, g, b)
Display	*dpy;
Colormap	cmap;
XColor	*uct;
register int	r, g, b;
{
register int	dis, i=MaxColors;
register XColor	*cp = uct;	/* user color table */
PColor	value=-1, Pseudo=precision;

if (! qcolor[i>>1].pixel)	/* initial	*/
	while (i--)
		qcolor[i].pixel = (u_long) i;
if (dpy)	{
	int screen = XDefaultScreen(dpy);
	if (cmap==DefaultColormap(dpy, screen)
	&& ncolors+VCTEntry < 1<<XDisplayPlanes(dpy, screen))
	ncolors += VCTEntry;
}
if (!uct || cmap) {
	cp = qcolor;
	if (lastmap != cmap)	/* system color-map */
		XQueryColors(dpy, lastmap=cmap, cp, ncolors);
}
for (i=0; i < ncolors; i++) {
	dis = abs((cp[i].red >> 8) - r) +
		abs((cp[i].green >> 8) - g) +
		abs((cp[i].blue >> 8) - b);
	if (!dis)
		return (cp[i].pixel);	/* find exact value	*/
	else if (dis < Pseudo) {
	    Pseudo = dis;
	    value = cp[i].pixel;	/* get the closest one	*/
	}
}
return	value;
}
