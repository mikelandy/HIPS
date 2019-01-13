/*	PANEL . C	A library of the Main Control Panel
#
%	Copyright (c)	Jin, Guojun
%
%	This file includes SLIDER, Multiple-Key BUTTON and Press BUTTON
%	routines for creating, drawing, reading, setting, and controling
%	their key or position. TextLine() is used for window input with
%	a multiple-key button. It is better to keep it right side empty.
%	When button or slider created, its valid=1;	If valid=0, it is
%	not active at all.
%	Use GetCloseColor() and GetGray() to set colors & grays from colormap.
%	They must be set by other program after the color-map initialized.
%
%	Memory allocation:
%		ZALLOC() will zero all cells but is slower than nzalloc().
%
% AUTHOR:	Jin Guojun - Lawrence Berkeley Laboratory	4/1/91
*/

#include "panel.h"

#ifndef	ButtonTColor
#define	ButtonTColor	Green
#endif
#ifndef	PB_Fill
#define	PB_Fill	lightGray
#endif

PColor	Black, White, Visible, darkGray, lightGray, Gray,
	Red, Green, Blue, Yellow,
	black1, white1, gray1, HBGround=BGround;
XColor	graylevel[MaxColors];	/* locally calculate or store(load) to VCT */


/*=======================================================
%	Following routines are to build panel & parts	%
=======================================================*/
Panel*
CreatePanel(x, y, w, h, name, parent, WEMask)
char	*name;
WinAttribute	*parent;
{
register Panel	*p=ZALLOC(sizeof(*p), 1, name);

p->expose = PtrCAST &p->event;
p->root = parent->root;
p->dpy = parent->dpy;
p->colormap = parent->cmap;
p->name = str_save(name);
p->x0 = x;	p->y0 = y;
p->width = w;	p->height = h;
p->icon_width = 0;	/* no icon	*/

WEMask |= ButtonAction | KeyPressMask | PointerMotionMask;
CreateWindow(p, parent, XCreateFontCursor(p->dpy, XC_top_left_arrow), WEMask,
	True, 0);
SetFontGetSize(p, "8x13bold");
XSelectInput(p->dpy, p->win, WEMask | ExposureMask);
return	p;
}

Button*
CreateButton(p, x, y, numb, name, bname, bc, pc)
Panel	*p;
char	*name, **bname;
{
register Button	*b;
register int	i, w;

/*	4 is rough estimation for average words in each button	*/
if (x + p->font_w * 4 * numb > p->width || y+ButtonHeight > p->height)
	prgmerr(numb, "Button  %s out of panel", name);
b = ZALLOC(sizeof(*b), 1, name);
b->pw_type = PW_BUTTON;
b->name = str_save(name);
b->namex = x - strlen(name)*p->font_w - 4;
b->bname = ZALLOC(sizeof(*(b->bname)), numb, "bname");
b->bw = ZALLOC(sizeof(*(b->bw)), numb, "bwidth");
b->x = x;
b->y = y;
b->hei = ButtonHeight;
b->numb = numb;
for (i=w=0; i<numb; i++) {
	w += b->bw[i] = strlen(bname[i])*p->font_w + (TextLMargin<<1);;
	b->bname[i] = str_save(bname[i]);
}
b->rmb = w;
b->color = ZALLOC(numb, sizeof(*(b->color)), "Bcolor");
for (i=numb; i--;)
	b->color[i] = bc;	/* button color */
b->dimcolor = pc;		/* pressed color */
b->tcolor = ButtonTColor;
b->pan = p;
b->valid = True;
return	b;		/* bw & bname will be set by user	*/
}

PressButton*
CreatePressButton(p, x, y, name, pc)
Panel	*p;
char	*name;
{
register int	len = strlen(name)+1;
register PressButton	*pb;
if (x<0 || x + len > p->width || y<0 || y + ButtonHeight > p->height)
	prgmerr(len, "PButton %s not fit in panel", name);
pb = ZALLOC(sizeof(*pb), 1, name);
pb->pw_type = PW_PRESSBUTTON;
pb->x = x;
pb->y = y;
pb->hei = ButtonHeight;
pb->len = len * p->font_w + TextLMargin;
pb->name = str_save(name);
pb->color.act.p = pc;	/* pressed color */
pb->color.act.v = PB_Fill;
pb->tcolor = black1;
pb->pan = p;
pb->valid = True;
return	pb;
}


/*=======================================================
%	Following routines are to control panel parts	%
=======================================================*/
DrawButton(b)  register Button*	b;
{
register int	i, p=0;
if (b->valid)
    for (i=0; i<b->numb; i++) {

	XSetForeground(b->pan->dpy, b->pan->gc,
		i==ButtonState(b) ? b->dimcolor : b->color[i]);
	XFillRectangle(b->pan->dpy, b->pan->win, b->pan->gc, b->x+p, b->y,
		b->bw[i]-2, b->hei);
	XSetForeground(b->pan->dpy, b->pan->gc,
		i==ButtonState(b) ? Black : b->tcolor);
	XDrawString(b->pan->dpy, b->pan->win, b->pan->gc, b->x + p + TextLMargin,
		b->y + TextVMargin, b->bname[i], strlen(b->bname[i]));
	p += b->bw[i];
    }
else
    for (i=0; i<b->numb; i++) {
	
	XSetForeground(b->pan->dpy, b->pan->gc,
		i==ButtonState(b) ? b->color[i] : b->dimcolor);
	XDrawRectangle(b->pan->dpy, b->pan->win, b->pan->gc, b->x+p, b->y,
		b->bw[i]-2, b->hei);
	XSetForeground(b->pan->dpy, b->pan->gc, darkGray); /* buttom title */
	XDrawString(b->pan->dpy, b->pan->win, b->pan->gc, b->x + p + TextLMargin,
		b->y + TextVMargin, b->bname[i], strlen(b->bname[i]));
	p += b->bw[i];
    }
XSetForeground(b->pan->dpy, b->pan->gc, white1); /* title name */
XDrawString(b->pan->dpy, b->pan->win, b->pan->gc, b->namex, b->y + TextVMargin,
	b->name, strlen(b->name));
XFlush(b->pan->dpy);
}

DrawPressButton(pb)  register PressButton*	pb;
{
if (pb->valid) {
XSetForeground(pb->pan->dpy, pb->pan->gc, pb->color.s[pb->state]);
XFillArc(pb->pan->dpy, pb->pan->win, pb->pan->gc, pb->x, pb->y, pb->len, pb->hei,
	0, 360<<6);
XSetForeground(pb->pan->dpy, pb->pan->gc, Yellow);
XDrawArc(pb->pan->dpy, pb->pan->win, pb->pan->gc,
	pb->x, pb->y, pb->len, pb->hei, 0, 360<<6);
}
else	{
XSetForeground(pb->pan->dpy, pb->pan->gc, Yellow);
XDrawArc(pb->pan->dpy, pb->pan->win, pb->pan->gc,
	pb->x, pb->y, pb->len, pb->hei, 0, 360<<6);
}
XSetForeground(pb->pan->dpy, pb->pan->gc, pb->tcolor);
XDrawString(pb->pan->dpy, pb->pan->win, pb->pan->gc,
	pb->x + TextLMargin, pb->y + TextVMargin, pb->name, strlen(pb->name));
XFlush(pb->pan->dpy);
}


OnButton(b, xbutton)  register Button*	b; XButtonEvent	*xbutton;
{
register int	i, w, x=xbutton->x, y=xbutton->y;

if (xbutton->window != b->pan->win || !b->valid)	return	False;

#ifdef	_DEBUG_
DEBUGMESSAGE("OnB: x=%d, y=%d, bx=%d, by=%d, rmb=%d\n", x,y,b->x,b->y,b->rmb);
#endif
x -= b->x;
if (y < b->y || y > b->y + b->hei || x < 0 || x > b->rmb)
	return	-1;
for (i=w=0; i<b->numb; i++)	{
	w += b->bw[i];
	if (x < w)
	    if (i == ButtonState(b))	return	0;
	    else{
		ButtonState(b) = i++;
		DrawButton(b);
		return	i;
	    }
}
}

bool
ButtonPressed(pb, xbutton)  PressButton*	pb; XButtonEvent	*xbutton;
{
register bool	state;
register int	x=xbutton->x, y=xbutton->y;
if (!pb->valid || pb->state || xbutton->window != pb->pan->win)
	return	False;
state = (x>=pb->x && x<pb->x+pb->len && y>=pb->y && y<pb->y+pb->hei);
if (state){
	PressButtonState(pb) = state;
	DrawPressButton(pb);
}
return	state;
}

void
ResetPressButton(pb)  register PressButton	*pb;
{
	PressButtonState(pb) = RESETSTATE;
	DrawPressButton(pb);
}



/*=======================================================
%	return currently pressed button name(label)	%
%	It may be defined as a macro define		%
=======================================================*/
char*
ReadButton(b)  Button	*b;
{
return	b->bname[ButtonState(b)];
}


/*===============================================
%	A text line editor for panel input	%
%	if x0 = 0, use right side area of b	%
%	else use information area of panel	%
===============================================*/
TextLine(b, buf, len, x0, exp_hd, imgp, imgs)  Button	*b; char*	buf;
	int	len; int x0; int (*exp_hd)();
	Image	*imgp;	/* for exposure handle	*/
	int	imgs;
{
int	bufl=strlen(buf), chars_line, max_field, /* maximum text filed */
	cursor_p, sl, ys, y0; /* top of the text line */
char	str[16];
bool	nodone=1, new=bufl;

if (!x0){
	x0 = b->x + b->rmb;
	y0 = b->y;
}
else	y0 = b->pan->height - (b->pan->font_h-1<<1);

ys = y0 + (b->hei>>1);	/* string height */

chars_line = (b->pan->width - x0) / b->pan->font_w - 1;
max_field = MIN(len*b->pan->font_w, b->pan->width - x0);
XClearArea(b->pan->dpy, b->pan->win, x0, y0, max_field, b->hei, 0);
XSetForeground(b->pan->dpy, b->pan->gc, white1);
XDrawLine(b->pan->dpy, b->pan->win, b->pan->gc, x0, y0+b->hei,
	x0+max_field, y0+b->hei);
cursor_p = MIN(bufl, chars_line);
if (cursor_p)
	XDrawString(b->pan->dpy, b->pan->win, b->pan->gc, x0, ys,
	buf + (bufl<chars_line ? 0 : bufl-cursor_p), cursor_p);
XBell(b->pan->dpy, 0);	/* beep to start input */

do	{
XComposeStatus	status;
KeySym	keysym;
	FlushingCursor(b->pan,	exp_hd, imgp, imgs, x0 + cursor_p*b->pan->font_w,
		y0, b->pan->font_w, b->pan->font_h, 15, 0);
	sl = XLookupString(&b->pan->event, str, 16, &keysym, &status);

	str[sl] = 0; /* set end of string */
	if (str[0] == BS | *str == 0x7F){	/* ctrl-H or DEL */
		if (!bufl)	continue;
		if (bufl-- <= chars_line){
		    XClearArea(b->pan->dpy, b->pan->win,
			x0+cursor_p*b->pan->font_w, y0, b->pan->font_w, b->hei, 0);
		    cursor_p = bufl;
		}
		else	{
		    XClearArea(b->pan->dpy, b->pan->win, x0, y0, max_field, b->hei, 0);
		    XDrawString(b->pan->dpy, b->pan->win, b->pan->gc, x0, ys,
			buf + bufl-cursor_p, cursor_p);
		}
		buf[bufl] = new = 0;
	}
	else if (str[0] == Esc)	return	0;	/* Undo */
	else if (str[0] == Tab){	/* Cat. So set new to False */
		new = 0;	continue;
	}
	else if (nodone = str[0] - '\r'){
	    if (new){	/* clear all to start a new line */
		XClearArea(b->pan->dpy, b->pan->win, x0,y0,max_field,b->hei,0);
		XSetForeground(b->pan->dpy, b->pan->gc, white1);
		XDrawLine(b->pan->dpy, b->pan->win, b->pan->gc, x0, y0+b->hei,
			x0+max_field, y0+b->hei);
		buf[0] = new = cursor_p = bufl = 0;
	    }
	    strcat(buf, str);
	    bufl += sl;
	    if (bufl <= chars_line){
		XDrawString(b->pan->dpy, b->pan->win, b->pan->gc,
			x0+cursor_p*b->pan->font_w, ys, str, sl);
		cursor_p = bufl;	/* set cursor position */
	    }
	    else{
		XClearArea(b->pan->dpy, b->pan->win, x0, y0, max_field, b->hei, 0);
		XDrawString(b->pan->dpy, b->pan->win, b->pan->gc,
			x0, ys, buf + bufl-cursor_p, cursor_p);
		}
	}
}while (nodone && strlen(buf)<len);
return	strlen(buf);
}

/*=======================================================
%	display message at bottom area of the panel	%
=======================================================*/
DispInfo(p, xoffset, str, color)  Panel	*p; int xoffset; char*	str; int color;
{
XClearArea(p->dpy, p->win, xoffset, p->height-(p->font_h<<1),
	p->width-xoffset, p->font_h<<1, 0);
XSetForeground(p->dpy, p->gc, color);
XDrawString(p->dpy, p->win, p->gc, xoffset+p->font_w, p->height-p->font_h,
	str, strlen(str));
XFlush(p->dpy);
}

/*=======================================================
%	display message onto right area of panel parts	%
%	or onto given place. If any of x0, y0 and len	%
%	is zero, then it will be set to parts->x,	%
%	parts->y and length of the right area of parts	%
=======================================================*/
PanelMessage(pp, pmsg, x0, y0, len)  AnyParts*	pp; char	*pmsg;
	register int	x0; register int	y0; register int	len;
{
if (!x0)	x0 = pp->x+pp->w;
if (!y0)	y0 = pp->y-1;
if (!len)	len = pp->pan->width - pp->x - pp->pan->font_w;
XClearArea(pp->pan->dpy, pp->pan->win, x0, y0, len, pp->pan->font_h+2, 0);
XSetForeground(pp->pan->dpy, pp->pan->gc, pp->vcolor);
XDrawString(pp->pan->dpy, pp->pan->win, pp->pan->gc,
	x0, y0+pp->pan->font_h, pmsg, strlen(pmsg));
XFlush(pp->pan->dpy);
}

#ifndef	PANEL_SEPARATED_WLIB
# include "popmenu.c"
#endif

#include "slider.c"

