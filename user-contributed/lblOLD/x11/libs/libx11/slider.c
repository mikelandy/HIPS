/*	SLIDER . C
#
%	Copyright (c)	Jin, Guojun
%
% AUTHOR:	Jin Guojun - Lawrence Berkeley Laboratory	4/1/91
*/

#ifndef	PANEL_H
#include "panel.h"
#endif


Slider*
CreateSlider(p, x, y, scale, v0, v1, numb, vert, name, info, bc, rc)
Panel	*p;
char	*name, *info;
{
register Slider	*s;
register int	i, slen;

stry:	slen = (v1 - v0) / scale;
	if (vert && slen+y > p->height || !vert && slen+x > p->width)	{
		scale++;	goto	stry;
	}

	s = ZALLOC(sizeof(*s), 1, name);
	s->pw_type = PW_SLIDER;
	s->defscale = s->scale = scale;
	s->defmin = (s->min = v0) / scale;
	s->defmax = (s->max = v1) / scale;
	s->numb = numb;
	s->rl = slen;
	s->rw = RailWidth;
	s->rx = x;	s->ry = y;
	s->rcolor = rc;
	s->pan = p;
	s->sx = ZALLOC(numb, sizeof(*(s->sx)), "sx");
	s->sy = ZALLOC(numb, sizeof(*(s->sy)), "sy");
	s->vert = vert;
	if (vert)	{
		s->sh = HandleWidth;
		s->sw = HandleHeight;
		s->sy[0] = y;
		s->sx[0] = x - (s->sw-s->rw >> 1);
		if (numb>1)	{
			s->sy[1] = s->sy[0] + s->sh;
			s->sx[1] = s->sx[0];
		}
	}
	else	{	/*	horizontal	*/
		s->sw = HandleWidth;
		s->sh = HandleHeight;
		s->sx[0] = x;
		s->sy[0] = y - (s->sh-s->rw >> 1);
		if (numb>1){
			s->sx[1] = s->sx[0] + s->sw;
			s->sy[1] = s->sy[0];
		}
	}
	s->name = str_save(name);
	s->namex = s->rx - strlen(s->name)*p->font_w - (TextLMargin>>1);
	i = strlen(info)+1;
	s->info = ZALLOC(i, 1, info);
	if (i>1){
		strcpy(s->info, info);
		s->infy = s->sy[0] - (HandleHeight >> 1) - TextVMargin;
	}
	else	s->infy = s->sy[0] - p->font_h;
	s->infx = s->rx + (slen - p->font_w * i >> 1) + (TextLMargin>>1);
	s->scolor = ZALLOC(numb, sizeof(*(s->scolor)), "scolor");
	for (i=numb--; i--;)
	    if (bc < 128)	/* bc is color entry	*/
		s->scolor[i] = bc + (numb-i << 4);
	    else
		s->scolor[i] = bc - (numb-i << 3);
	s->tcolor = Green;
#ifdef	_DEBUG_
message("npos=%d, ipos=%d, rx=%d, rl=%d\n", s->namex, s->infx, s->rx, s->rl);
#endif
	s->valid = True;
return	s;
}


DrawSlider(s)
register Slider	*s;
{
register int	i = s->rl+s->rx-s->namex+TextLMargin;
char	value[32];

if (!s->valid)	return;
if (s->numb>1)	i += s->rx - s->namex;
XClearArea(s->pan->dpy, s->pan->win, s->namex, s->ry-s->pan->font_h-RailWidth,
	i, s->sh+TextVMargin, 0);
	XSetForeground(s->pan->dpy, s->pan->gc, s->rcolor); /* draw rail */
	XFillRectangle(s->pan->dpy, s->pan->win, s->pan->gc, s->rx, s->ry,
		s->rl+s->sw, RailWidth);
	for (i=0; i<s->numb; i++){
		XSetForeground(s->pan->dpy, s->pan->gc, s->scolor[i]);
		XFillRectangle(s->pan->dpy, s->pan->win, s->pan->gc,
			s->sx[i], s->sy[i], s->sw, s->sh);
	}
	XSetForeground(s->pan->dpy, s->pan->gc, s->tcolor);/* write name */
	XDrawString(s->pan->dpy, s->pan->win, s->pan->gc, s->namex,
		s->ry+s->pan->font_h, s->name, strlen(s->name));
XDrawLine(s->pan->dpy, s->pan->win, s->pan->gc, s->namex, s->ry, s->rx-2, s->ry);
	sprintf(value, "%d", ReadSlider(s, 1));
	XDrawString(s->pan->dpy, s->pan->win, s->pan->gc, s->namex,
		s->ry-(s->pan->font_h>>1), value, strlen(value));

if (s->numb>1) {	/* on the top of another number
	XSetForeground(s->pan->dpy, s->pan->gc, s->scolor[1]);
	XDrawLine(s->pan->dpy, s->pan->win, s->pan->gc,
		s->namex, s->ry-s->pan->font_h-RailWidth,
		s->rx-2, s->ry-s->pan->font_h-RailWidth);
	sprintf(value, "%d", ReadSlider(s, 2));
	XDrawString(s->pan->dpy, s->pan->win, s->pan->gc, s->namex,
		s->ry-s->pan->font_h - TextVMargin, value, strlen(value));
}
*/	XSetForeground(s->pan->dpy, s->pan->gc, white1);
	sprintf(value, "%d", ReadSlider(s, 2));
	i = strlen(value);
	XDrawLine(s->pan->dpy, s->pan->win, s->pan->gc, s->rx+s->rl+s->sw+4,
		s->ry, s->rx+s->rl+s->sh+i*s->pan->font_w, s->ry);
	XDrawString(s->pan->dpy, s->pan->win, s->pan->gc, s->rx+s->rl+s->sw+4,
		s->ry-(s->pan->font_h>>1), value, i);
}
SliderInfo(s, Gray);
}


SliderInfo(s, color)
register Slider	*s;
{
char	value[32];
register int	slen = strlen(s->info);
XSetForeground(s->pan->dpy, s->pan->gc, color);
	if (slen)
	    XDrawString(s->pan->dpy, s->pan->win, s->pan->gc, s->infx,
		s->infy, s->info, slen);
	sprintf(value, "%d", s->min);
	slen = strlen(value);
	XDrawString(s->pan->dpy, s->pan->win, s->pan->gc, s->rx,
		s->infy, value, slen);
	sprintf(value, "%d", s->max);
	slen = strlen(value);
	XDrawString(s->pan->dpy, s->pan->win, s->pan->gc,
		s->rx + s->rl - s->pan->font_w*slen, s->infy, value, slen);
XFlush(s->pan->dpy);
}

ChangeSliderScale(s, scale, active)/* scale=0 reset scale to default */
register Slider	*s;
{
	s->scale = scale ? scale : s->defscale;
	s->min = s->defmin * s->scale;
	s->max = s->defmax * s->scale;
	if (active)	{
		EraseSlider(s);
		DrawSlider(s);
	}
}

/*	set Slider Bar in value (p) postion.	*/
SetSBarRPos(s, p, bar)
register Slider	*s;
{
if (bar > s->numb || p < s->min || p > s->max)	return	0;
p -= s->min;
if (s->vert)
	return	s->sy[bar-1] = p / s->scale + s->ry;
else	return	s->sx[bar-1] = p / s->scale + s->rx;
}

/*	set Slider Bar postion by point on panel	*/
SetSBarPos(s, x, y, bar)
register Slider	*s;
{
if (bar > s->numb)	return	0;
if (!s->vert)
	if (x < s->rx || x > s->rx+s->rl)	return	0;
	else	s->sx[bar-1] = x;
else if (y < s->ry || y > s->ry+s->rl)	return	0;
	else	s->sy[bar-1] = y;
#ifdef	_DEBUG_
if (DEBUGANY){
	message("SB(%d): rx=%d, ry=%d, sx1=%d, sy1=%d", bar,
		s->rx, s->ry, s->sx[0], s->sy[0]);
	if (s->numb>1)
		message(" sx2=%d, sy2=%d",s->sx[1], s->sy[1]);
	mesg("\n");
}
#endif
DrawSlider(s);
return	1;
}

/* return Bar ID (number=1, 2,...); 0 is nothing on a Bar	*/
OnSliderBar(s, xbutton)
register Slider	*s;
XButtonEvent	*xbutton;
{
register int	i, x=xbutton->x, y=xbutton->y;
if (!s->valid || xbutton->window != s->pan->win)	return	0;
if (s->vert){	/* vert	*/
	if (x < s->sx[0] || x > s->sx[s->numb-1] + s->sw)	return	0;
	for (i=0; i<s->numb; i++)
		if (y >= s->sy[i] && y < s->sy[i] + s->sh)	return	i+1;
}
else	{
	if (y < s->sy[0] || y > s->sy[s->numb-1] + s->sh)	return	0;
	for (i=0; i<s->numb; i++)
		if (x >= s->sx[i] && x < s->sx[i] + s->sw)	return	i+1;
}
return	0;
}


ReadSlider(s, h)
register Slider	*s;
{
register int	tmp;
register float	scale = ((float)s->max - s->min) / s->rl;
if (h<1 || h>s->numb)	h = s->numb;
h--;
if (s->vert)
	tmp = s->sy[h];
else	tmp = s->sx[h];
return	(tmp - s->rx) * scale + s->min;
}

void
EraseSlider(s)
register Slider	*s;
{
if (s->vert)
    XClearArea(s->pan->dpy, s->pan->win, s->namex - TextLMargin, 0,
	(TextLMargin<<1) + s->rl + s->rx - s->namex + s->sw, 0, 0);
else	/*	correct	*/
    XClearArea(s->pan->dpy, s->pan->win, 0, s->infy-TextVMargin,
	0, s->sy[0] - s->infy + (TextVMargin<<1) + s->sh, 0);
}
