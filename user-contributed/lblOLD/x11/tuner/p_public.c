/*	Panel_PUBLIC . C
#
%	Copyright (c)	Jin Guojun - All rights reserved
%
%		initial & run time panel public handlers
%	Including:
%		Button1_On(), SaveImage(), YesOrNo(), WaitOk(),
%		ChangePanelCmap(), DrawPanel(), InputFrom_Panel(),
%		Exposure_handler(),
%		Histo_Setting(), PickUpColor(), Toggle_Info(),
%
% AUTHOR:	Jin, Guojun - LBL	4/1/1991
*/

#include "tuner.h"

XWindowAttributes	gwattrs;
extern int	fnt_r, fnt_g, fnt_b;

#define	Dpy1		Monitor[1].dpy
#define	Screen1		Monitor[1].screen
/*===============================================
%	public routines for color and b/w	%
===============================================*/
void
Exposure_handler(XExposeEvent	*expose, Image	*img, int (*sub_win)())
{
register Window	exp_win=expose->window;

	if (expose->type != Expose)
		return;	/*	generated by XCopyArea	*/
	if (img && exp_win == img->win)
		win_exposure(expose, img, sub_win);
	else if (img && exp_win == img->icon)
		XPutImage(img->dpy, img->icon, img->igc, img->icon_image,
			0, 0, 0, 0, img->icon_width, img->icon_height);
	else if (exp_win == Epanel->win)	DrawPanel();
	else if (exp_win == InfoWin->win)	Toggle_Info(0);
	else if (exp_win == histinfo.his->win)
		win_exposure(expose, histinfo.his, DrawVMark);
	else if (exp_win == NoteWin->win)
		DisplayMessage(NoteWin, NULL, 0, True);
	else if (!general_PW_manager(img, expose, exp_win))
		message("strange exposure win %d\n", exp_win);
	DEBUGMESSAGE("expose event x= %d y= %d width= %d height= %d\n",
			expose->x, expose->y, expose->width, expose->height);
}


void
ChangeSlider(Image* img, Slider	**slider, Slider *s1, Slider *s2,
	Button	*equ_b, Button *rel_b)
{
EraseSlider(*slider);
switch (img->curve) {
case ETAForeGD:
case ETABackGD:
	*slider = s1;	/* ESlider */
	SetSBarRPos(*slider, img->curve ? img->bgrd : img->fgrd, 1);
	break;
case ETALinear:
case ETALinear | ETAHistoEq:
	*slider = s2;	/* LSlider */
	SetSBarRPos(*slider, img->linearlow, 1);
	SetSBarRPos(*slider, img->linearup, 2);
	ButtonState(equ_b /*heqButt*/) = img->curve & ETAHistoEq;
	break;
default:msg("ETA %d\n", img->curve);
}
DrawSlider(*slider);
SliderInfo(*slider, lightGray);
ButtonState(rel_b) = img->curve & 0x7;
DrawButton(rel_b);	/* EButton */
}

void
SetShowFramePos(Image *img, Button *f_b, bool reset)
{
	sprintf(f_b->bname[PosFRAME], fb_format, img->fn+1, img->frames);
	if (reset)	ButtonState(f_b)=RESETSTATE;
	DrawButton(f_b /*fButton*/);
}

WaitOk(PressButton*	b, char *sp, int line_h)
{
register XEvent	*xe;

if (!b)	b = OkButt;
xe = &b->pan->event;

DisplayMessage(NoteWin, sp, line_h, 0);
DrawPressButton(b);
while (!XCheckMaskEvent(b->pan->dpy, ButtonPressMask, xe))
    if (XCheckMaskEvent(b->pan->dpy, ExposureMask, xe))
	Exposure_handler(xe, NULL, No),	DrawPressButton(b);
HidingPanel(NoteWin);
return	(b==OkButt);
}

YesOrNo(char *sp, int line_h)
{
XEvent	*xe = &YesButt->pan->event;
int	state = 2;

DisplayMessage(NoteWin, sp, line_h, 0);
PressButtonState(YesButt) = PressButtonState(NoButt) = RESETSTATE;

DrawPressButton(YesButt);
DrawPressButton(NoButt);
while (abs(state) >> 1) {
	while (!XCheckMaskEvent(YesButt->pan->dpy, ButtonPressMask, xe))
	    if (XCheckMaskEvent(YesButt->pan->dpy, ExposureMask, xe))
		Exposure_handler(xe, NULL, No),
		DrawPressButton(YesButt),
		DrawPressButton(NoButt);
	if (ButtonPressed(YesButt, xe))		state = 1;
	else if (ButtonPressed(NoButt, xe))	state = 0;
}
HidingPanel(NoteWin);
return	state;
}

/*	A general Panel input routine, but requires a gadget on panel	*/
InputFrom_Panel(AnyParts *app, char *buf, int buf_size,	int sp_limit,
		char *dmesg, char *info, int i_color, bool show_p)
{
	DisplayMessage(app->pan, dmesg ? dmesg : buf, sp_limit, 0);
	if (info)
		DispInfo(app->pan, 4, info, i_color);
	if (show_p)
		DrawButton(app);
	buf[0] = 0;
	return	TextLine(app, buf, buf_size,
			info ? (strlen(info)+1) * fontWidth : fontWidth,
		Exposure_handler, pic, num_images); /* only global usage */
}

void
Toggle_Info(char* s)
{
static	int	state;
static	char*	sp;
register int	old = !s;

if (!old)
	sp = s;
if (old || (state = !state))
	DisplayMessage(InfoWin, sp, 8, old);
else	HidingPanel(InfoWin);
}

void
SetPanelItemColor()
{
Set_Panel(CSlider, SLIDER_COLOR, Green, Red, lightGray);
Set_Panel(ESlider, SLIDER_COLOR, Blue, white1, Visible);
Set_Panel(LSlider, SLIDER_COLOR, Red, white1, lightGray);
Set_Panel(QSlider, SLIDER_COLOR, Red, Green, white1);
Set_Panel(QButton, BUTTON_COLOR, Visible, Light, Green);
Set_Panel(DButton, BUTTON_COLOR, Visible, Light, Green);
Set_Panel(EButton, BUTTON_COLOR, lightGray, Yellow, Green);
Set_Panel(FButton, BUTTON_COLOR, Visible, Light, Green);
Set_Panel(fButton, BUTTON_COLOR, Visible, Light, Green);
Set_Panel(hButton, BUTTON_COLOR, Visible, Light, Green);
Set_Panel(Interpolate, BUTTON_COLOR, Visible, Light, Yellow);
Set_Panel(ZButton, BUTTON_COLOR, Visible, Light, Yellow);
Set_Panel(RstButt, PRESS_BUTTON_COLOR, Gray, lightGray, -1);
Set_Panel(rfsButt, PRESS_BUTTON_COLOR, lightGray, Gray, Red);
Set_Panel(heqButt, PRESS_BUTTON_COLOR, Light, lightGray, -1);
Set_Panel(YesButt, PRESS_BUTTON_COLOR, Light, lightGray, -1);
Set_Panel(NoButt, PRESS_BUTTON_COLOR, Light, lightGray, -1);
ctrlmenu->bgcolor=paramenu->bgcolor=filemenu->bgcolor = Light;	/*?*/
}

void
ChangePanelCmap(Image *img)
{
register int	i = Dpy != Dpy1;	/* point default cmap */
if (i || !rw_set_id || img->dpy_depth == 24)	return;

	/* To ensure XStoreColors to store existing image colormap.
	If gwattrs.colormap is not equal to img->colormap, then
	the img->colormap needs to be installed.
	If an image quited but it did not set firstmap to zero, then
	gwattrs.colormap will be same as Epanel->colormap if image's
	colormap was not installed, or gwattrs.colormap will be zero if its
	colormap was installed.
	*/
    XGetWindowAttributes(Epanel->dpy, Epanel->win, &gwattrs);
    i = img != rw_set_id;
    if (i || !firstmap || gwattrs.colormap != img->colormap) {
#ifdef	RELIABLE_CMAP_FOR_ALL
	register int	BaseGL=Monitor[0].cmap==img->colormap ? VCTEntry : 0;
#endif
#ifdef	_DEBUG_
    msg("\nsave colormap [%u] %X\n", firstmap, gwattrs.colormap);
	msg("gl77 [p%d] = %04x %04x %04x\n", graylevel[77].pixel,
		graylevel[77].red, graylevel[77].green, graylevel[77].blue);
#endif
	if (firstmap && gwattrs.colormap &&
			gwattrs.colormap != Epanel->colormap | i)
		XStoreColors(Epanel->dpy, firstmap, graylevel, ncolors);

	for (i=img->entries; i--;)
#ifdef	RELIABLE_CMAP_FOR_ALL
		graylevel[i].pixel = i + BaseGL;
#else
		graylevel[i].pixel = img->pixel_table[i];
#endif
	XQueryColors(img->dpy, firstmap=img->colormap,
		graylevel, ncolors=img->entries);
#ifdef	_DEBUG_
	msg("new colormap = %X [%u]\n", img->colormap, firstmap);
	msg("gl77 [p%d] = %04x %04x %04x\n", graylevel[77].pixel,
		graylevel[77].red, graylevel[77].green, graylevel[77].blue);
#endif
	rw_set_id = img;
	XSetWindowColormap(Epanel->dpy, Epanel->win, firstmap);
	XSetWindowColormap(NoteWin->dpy, NoteWin->win, firstmap);
	XSetWindowColormap(InfoWin->dpy, InfoWin->win, firstmap);
	XSetWindowColormap(ctrlmenu->dpy, ctrlmenu->win, firstmap);
	XSetWindowColormap(paramenu->dpy, paramenu->win, firstmap);
	XSetWindowColormap(filemenu->dpy, filemenu->win, firstmap);
	XInstallColormap(Epanel->dpy, firstmap);
	if (ButtonState(QButton) && img->dpy_depth < 24) {
		Visible = darkGray;
		Light = GetGray(img->dpy, img->colormap, img->entries, 224);
		white1 = GetGray(img->dpy, img->colormap, img->entries, 248);
		SetPanelColor(img->dpy, img->colormap, graylevel, img->entries, 2);
		SetPanelItemColor();
	}
    }
}

void
DrawPanel()	/* it will be changed. */
{
XGCValues	values;
XGetGCValues(slider->pan->dpy, slider->pan->gc, GCForeground, &values);
	DrawSlider(slider);
	DrawSlider(CSlider);
	DrawSlider(QSlider);
	DrawButton(EButton);
	DrawButton(FButton);
	DrawButton(fButton);
	DrawButton(hButton);
	DrawButton(ZButton);
	DrawButton(QButton);
	DrawButton(DButton);
	DrawButton(Interpolate);
	DrawPressButton(heqButt);
	DrawPressButton(rfsButt);
	DrawPressButton(RstButt);
XSetForeground(slider->pan->dpy, slider->pan->gc, values.foreground);
}

Button1_On(XButtonEvent	*xbutton, int	*fb)
{
if (ButtonPressed(RstButt, xbutton))	return	OnResetButton;
if (*fb=OnSliderBar(slider, xbutton))	return	OnETASlider;
if (*fb=OnSliderBar(CSlider, xbutton))	return	OnClipSlider;
if (*fb=OnSliderBar(QSlider, xbutton))	return	OnQuanSlider;
if (OnButton(hButton, xbutton)>0)	return	OnHistButton;
if (OnButton(ZButton, xbutton)>0)	return	OnZcntButton;
if (OnButton(QButton, xbutton)>0)	return	OnQuanButton;
if (OnButton(fButton, xbutton)>0)	return	OnFrameButton;
if (OnButton(EButton, xbutton)>0)	return	OnETAButton;
if (OnButton(DButton, xbutton)>0)	return	OnDataButton;
if (OnButton(Interpolate, xbutton)>0)	return	OnInterpolate;
if (ButtonPressed(rfsButt, xbutton))	return	OnRefresh;
if (ButtonPressed(heqButt, xbutton))	return	OnHEQButton;
return	EOF;
}


SaveImage(Image	*img)
{
char	buf[128];
register int	len;

TopWindow(Epanel, No, Yes);
DrawPanel();	XFlush(Epanel->dpy);
sprintf(buf, "SAVE %s before Close it ? y/n", img->name);
len = (strlen(buf)+2) * Epanel->font_w;
DispInfo(Epanel, 0, buf, white1);
buf[0] = 0;
TextLine(fButton, buf, sizeof(buf), len, Exposure_handler, pic, num_images);

if (buf[0] != 'n')	{
	ButtonState(FButton) = FileSave;
	FileAccess(&img, 1, 1);
	}
}

Histo_Setting(U_IMAGE*	img, char *lbuf)
{
register int	i;
	if (ButtonState(hButton) > HistScaleSet)
		histchange(hButton, img, &histinfo);
	else {
	    sprintf(lbuf, "maximum histo = %d, set %s", img->mmm.maxcnt,
		ButtonState(hButton)==HistScaleSelf ? "sacle" :
				"maximum value");
	    i = (strlen(lbuf)+2) * Epanel->font_w;
	    DispInfo(Epanel, 0, lbuf, white1);
	    histinfo.scale = lbuf[0] = 0;
	    TextLine(hButton, lbuf, sizeof(lbuf), i,
		Exposure_handler, pic, num_images);
	    i = atoi(lbuf);
	    img->setscale = ButtonState(hButton)==HistScaleSet;
	    switch (ButtonState(hButton))	{
	    case HistScaleSelf:
		if (img->color_dpy)
			img->mmm.maxcnt = img->marray[img->fn %
					img->channels].maxcnt;
		else
			img->mmm.maxcnt = img->marray[img->fn].maxcnt;
		if (i)	histinfo.scale = i;
		break;
	    case HistScaleSet:
		if (i < 0)	img->mmm.maxcnt = img->marray[0].maxcnt;
		else if (i>row)	img->mmm.maxcnt = i;
	    }
	    HistoHandle(img, &histinfo, lightGray);
	}
	ButtonState(hButton) = RESETSTATE;
	DrawButton(hButton);
}


GetNew_Img_Font(Image	*img)
{
int	i = PopingMenu(fontmenu, MENUF_PKCOLOR, Exposure_handler, &img, 1);

	/* the list number in this popmenu is start from 0 rather 1	*/
	if (--i < MENUF_1stFONT)
	    if (i == MENUF_PKCOLOR)
		return	PickUpColor(img, &fnt_r, &fnt_g, &fnt_b);
	    else	return	i;
	else if (i < MENUF_UNDO)
		SetFontGetSize(img, fontlist[i]);
return	~i;
}