/*	CTRLPANEL . C
#
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

This software is copyright (C) by the Lawrence Berkeley Laboratory.
Permission is granted to reproduce this software for non-commercial
purposes provided that this notice is left intact.

It is acknowledged that the U.S. Government has rights to this software
under Contract DE-AC03-765F00098 between the U.S.  Department of Energy
and the University of California.

This software is provided as a professional and academic contribution
for joint exchange. Thus, it is experimental, and is provided ``as is'',
with no warranties of any kind whatsoever, no support, no promise of
updates, or printed documentation. By using this software, you
acknowledge that the Lawrence Berkeley Laboratory and Regents of the
University of California shall have no liability with respect to the
infringement of other copyrights by any part of this software.

For further information about this notice, contact William Johnston,
Bld. 50B, Rm. 2239, Lawrence Berkeley Laboratory, Berkeley, CA, 94720.
(wejohnston@lbl.gov)

For further information about this software, contact:
	Jin Guojun
	Bld. 50B, Rm. 2275, Lawrence Berkeley Laboratory, Berkeley, CA, 94720.
	g_jin@lbl.gov

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%	The Panel_init() build a control panel for both color and
%	gray-scale interactive analyst.	Panel wedgets are global
%	and not in the wedget stack, which is for users' wedgets.
%
%	CreateTuner() creates histinfo image, IM and its LKT space;
%	then calls Panel_init()
%
% AUTHOR:	Jin, Guojun - LBL	4/1/1991
*/

#ifndef	ETAButtonColor
#define	ETAButtonColor	3
#endif

PopMenu	*ctrlmenu, *paramenu, *filemenu, *fontmenu;
Panel	*NoteWin, *InfoWin;
Button	*MsgButton;
PressButton	*YesButt, *NoButt, *OkButt, *AbortButt;
char	*BN1[] = {"fore", "back", "linear"},
	*BNF[] = {"FILE", "Load", "Save"},
	*OFFON[] = {"OFF", "ON"},
	*HST[] = {"histogram", "Scale", "Set", "Neg", "Grid"},
	*DUP[] = {"Analysis", "Update"},
	*NOTEMSG[] = {"*   CHANGE ETA SCALE   *"},
#ifdef	C_TUNER
	*FRM[] = {"Red", "Green", "Blue", "Sync"};
#else
	*FRM[] = {"  0/000", "Frame", "Prev", "Next"};
#endif


Panel_init(parent, ela_scale, PBGcolor, map_panel)
WinAttribute	*parent;
{
char	buf[128];

sprintf(buf, "Elasitc TUNER and ANALYZER -- %d %s", ncolors,
#ifdef	C_TUNER
"colors"
#else
"grey levels"
#endif
);
Epanel = CreatePanel(100, 200, 512, 360, buf, parent,
		EnterWindowMask | LeaveWindowMask);
XSetWindowBackground(parent->dpy, Epanel->win, PBGcolor);
if (map_panel)
	ShowPanel(Epanel);

fontWidth = Epanel->font_w;
fontHeight = Epanel->font_h;

	/*	Quantization	*/
QSlider = CreateSlider(Epanel, 192, 270, 5, -300, 100, 1, 0, "Quantizing", "",
		Blue, lightGray);

#ifdef	C_TUNER
QButton = CreateButton(Epanel, 144, 261, 2, "Fixed Panel Color", OFFON,
		Visible, Light);
fButton = CreateButton(Epanel, 64, 51, 4, "Channel", FRM, Visible, Light);
QSlider->valid = False;		/* never used, but easy.	*/
ButtonState(QButton) = True;	/* Change colors with image	*/
#else
QButton = CreateButton(Epanel, 16, 261, 2, "", OFFON, Visible, Light);
fButton = CreateButton(Epanel, 16, 51, 4, "", FRM, Visible, Light);
/* fButton = CreateButton(Epanel, 64, 51, 3, "Frame", FRM, Visible, Gray);*/
strcpy(filelist[MENU3_LDFrm-1], "......");	/* make it invalid...	*/
#endif

ESlider = CreateSlider(Epanel, 75, 174, ela_scale, -200*ela_scale, 200*ela_scale,
	1, 0, "Tuner", "<- Gamma -> | <-  S  -> | <- Gamma ->", White, Visible);
SetSBarRPos(ESlider, 0, 1);
LSlider = CreateSlider(Epanel, 100, 174, 1, 0, 255, 2, 0, "Scale", "Linear",
		White, lightGray);
DButton = CreateButton(Epanel, 50, 303, 2, "DATA", DUP, Visible, Light);
	/*	clip control	*/
CSlider = CreateSlider(Epanel, 312, 72, 2, 0, 254, 2, 0, "Clip", "",
		Blue, lightGray);
SetSBarRPos(CSlider, 254, 2);	/* initial to no clip!	*/
Interpolate = CreateButton(Epanel, 420, 261, 2, "Interpolation", OFFON,
		Visible, Light);
	/* Other Buttons */
ZButton = CreateButton(Epanel, 420, 204, 2, "ZERO", OFFON, Visible, Light);
EButton = CreateButton(Epanel, 100, 204, 3, "Emphasis", BN1,
		lightGray, parent->dpy_depth==8 ? ETAButtonColor : Blue);
RstButt = CreatePressButton(Epanel, 354, 300, "RESET", Gray);
rfsButt = CreatePressButton(Epanel, 435, 300, "ReFresh", Gray);
FButton = CreateButton(Epanel, 16, 12, 3, "", BNF, Visible, Light);
hButton = CreateButton(Epanel, 16, 96, 5, "", HST, Visible, Light);
heqButt = CreatePressButton(Epanel, 435, 96, "histeq", Light);
slider = LSlider;			/* overlayed, init to linear */
ButtonState(EButton) = ETALinear;	/* init to linear */
	/* PULL DOWN MENU */
ctrlmenu = CreatePopMenu(Epanel, parent, "Editer", ctrlist, numctrl, No);
paramenu = CreatePopMenu(Epanel, parent, "Properties", paralist, numpara, No);
filemenu = CreatePopMenu(Epanel, parent, "FILE", filelist, numcomd, No);
fontmenu = CreatePopMenu(Epanel, parent, "Fonts", fontlist, numfont, No);

InfoWin = CreatePanel(10, 10, 512, 256, "Information", parent, NULL);

NoteWin = CreatePanel(10, 10, 256, 128, "MESSAGE", parent, NULL);
MsgButton = CreateButton(NoteWin, 24, 16, 1, "", NOTEMSG, Visible, Gray);
YesButt = CreatePressButton(NoteWin, 48, 80, " Yes ", Light);
NoButt = CreatePressButton(NoteWin, 144, 80, " No ", Light);
OkButt = CreatePressButton(NoteWin, 96, 80, " OK ", Light);
AbortButt = CreatePressButton(NoteWin, 96, 80, " Abort ", Light);

return	map_panel;
}

DestroyPanel(condition)  cookie_t	condition;
{
	PW_DestroyPanel(NoteWin);
	PW_Destroy(ctrlmenu);
	PW_Destroy(filemenu);
	PW_Destroy(paramenu);
	PW_DestroyPanel(Epanel);
	XCloseDisplay(Dpy);	/* clear all others	*/
	if (Dpy1 && Dpy != Dpy1)
		XCloseDisplay(Dpy1);
}

CreateTuner(img, ela_scale, panel_bg, map_panel)  U_IMAGE *img; int	ela_scale; PColor panel_bg; bool map_panel;
{
register int	i = 1<<Monitor[1].dpy_depth, j;
#define	Dpy1	Monitor[1].dpy
#define	Screen1	Monitor[1].screen

	arrow = XCreateFontCursor(Dpy, XC_sb_down_arrow);

#ifdef	_DEBUG_
message("%d graylevel\n", ncolors /* -(~newmap&VCTEntry) */);
message("dpy=%x, dpy1=%x, screen=%d, screen1=%d\n", Dpy, Dpy1, Screen, Screen1);
#endif

/*	must be after color allocated	*/
	if (!precision)	precision = 384;

if (img->dpy_depth < 24)	{
	black1 = XBlackPixel(Dpy1, Screen1);
	white1 = gray1 = XWhitePixel(Dpy1, Screen1);
	if (i == 2)	darkGray = i;
	SetPanelColor(Dpy1, Monitor[1].cmap, graylevel,
		MIN(i, ncolors), start_fast);
}
else	MakeTrueColorPanel();

if (panel_bg < 2)
	panel_bg = darkGray;
if (x_regions < 2 || x_regions > MAX_ITP_LEVEL)
	x_regions = 2;
if (y_regions < 2 || y_regions > MAX_ITP_LEVEL)
	y_regions = 2;
i = MAX_ITP_LEVEL * MAX_ITP_LEVEL;	/* taking 64K memory */
j = MaxColors;
IM = (InterpMap*)NZALLOC(i, sizeof(*IM), "IM");
while (i--)
	IM[i].lkt = (LKT*)NZALLOC(j, sizeof(LKT), "lkts");

lkt = histinfo.lkt = (LKT*)zalloc(j*(img->color_dpy?3:1), sizeof(*lkt), "lkt");
if (img->color_dpy)
    for (i=0; i<MaxColors; i++)
	lkt[i] = lkt[j+i] = lkt[(j<<1)+i] = i;

histinfo.hist = (int*)NZALLOC(HistoSize, sizeof(*(histinfo.hist)), "hist");
CreateImage(&(histinfo.his), "Histogram", Monitor, HistoSize+(BFRAME<<1),
	(HistoSize+(BFRAME<<1)) * (img->color_dpy ? 3 : 1), 32,
	I_Mask | ExposureMask, 1);
histinfo.his->color_dpy = img->mid_type==RLE;
histinfo.his->channels = img->dpy_channels ? img->dpy_channels : 1;

XDefineCursor(Dpy, histinfo.his->win, arrow);
XSetWindowBackground(Dpy, histinfo.his->win, MGray);
XSelectInput(Dpy, histinfo.his->win, ExposureMask | I_Mask);
return	Panel_init(&Monitor[1], ela_scale, panel_bg, map_panel);
}
