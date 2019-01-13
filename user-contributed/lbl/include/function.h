/*	FUNCTION . H
%
%	Copyright (c)	Jin Guojun	1991
%
%	functions including:
%
%		create color lookup table, get closest color,
%		grey level quantization, quicksort, create panel.
%		integrated histogram stretching algorithms.
%		Histogram ploting subroutine, and exposure handling.
%	Remember:
%		The functions above require the X window library.  So, if
%	any window program is invoked, please use X_WINDOW_DEP or C_TUNER
%	directive. Otherwise, this header file is only a general definition
%	file for regular filters.
%
% AUTHOR:	Guojun Jin - LBL	4/1/91
*/

#ifndef	FUNC_H
#define	FUNC_H

#ifndef	LKT
#define	LKT	int
#endif			/* for lookup-table and interpolation */

#include "header.def"	/* for different images */

#define	I_Mask	PointerMotionMask | EnterWindowMask |	\
		ButtonPressMask | ButtonReleaseMask | ExposureMask

#ifndef	MAX_ITP_LEVEL
#define	MAX_ITP_LEVEL	8	/* maximum interpolation level, 64K mem */
#endif


/* X_WINDOW dependency OR color "tuner" applications. */

#if	defined	X_WINDOW_DEP | defined C_TUNER

#include "panel.h"

typedef	struct	{
	char	*copyarea;
	Image	*src, *dst, *copy;
	bool	fill, cut;	/* cut or copy */
	int	x0, y0, w, h, dstx, dsty;
	} EditorSpace;

#define	ClearParameterWin(imginfo, y0)	\
	XClearArea (imginfo->dpy, imginfo->win, 0, y0,	\
	0, abs(imginfo->tmp_offset), 1); /* imginfo->offset = img->font_h<<1 */

#define	ImageEvent(img, Ievent)	\
	XCheckMaskEvent(img->dpy, Ievent, img->event)
#define	RemoveImageEvent(img, Xevent)	\
	while(ImageEvent(img, Xevent))

/*	Export variables	*/

extern	Panel	*Epanel;
extern	Slider	*ESlider, *LSlider, *slider, *CSlider, *QSlider;
extern	Button	*EButton, *FButton, *fButton, *hButton,
		*QButton, *ZButton, *maxButt, *DButton, *Interpolate;
extern	PressButton	*RstButt, *heqButt, *rfsButt;
extern	HistoInfo	histinfo;

/*	import variables	*/

extern	PColor	MGray, Light;
extern	QSCell	QSArray[];
extern	XColor	graylevel[];

#define	GetUserColor(color_table, entries, r, g, b)	\
	GetCloseColor(0, 0, entries, color_table, r, g, b)

#define	GetUserGray(color_table, entries, gray)	\
	GetCloseColor(0, 0, entries, color_table, gray, gray, gray)

#endif	X_WINDOW_DEP | C_TUNER


#include "imagedef.h"

extern	char	*Help_message_array1;
extern	bool	cntz, top, neg, verbose;
extern	int	topv, *dgt, VCTEntry, precision, start_fast,
		fontWidth, fontHeight, ncolors;

#define	ELALINFO	1	/* Slider events */
#define	CLIPINFO	2
#define	QUANINFO	3
#define	HMARKPOS	4
#define	IMAGEINFO	5
#define	MOVESCROLLBAR	6

	/* ETA function codes */

#define	ETAForeGD	0	/* elastic on foreground */
#define	ETABackGD	1	/* elastic on background */
#define	ETALinear	2	/* linear scale, it effects all others */
#define	ETAQuant	4	/* quantization on */
#define	ETAHistoEq	0x80	/* histo equalization */

#define	OnETASlider	1
#define	OnClipSlider	2
#define	OnQuanSlider	3
#define	OnHistButton	5
#define	OnHEQButton	6
#define	OnZcntButton	7
#define	OnQuanButton	8
#define	OnFrameButton	9
#define	OnChannel	9	/* for color version */
#define	OnDataButton	10
#define	OnInterpolate	11
#define	OnETAButton	12
#define	OnMaxButton	13	/* not used */
#define	OnResetButton	14
#define	OnRefresh	15


#define	fb_format	"%3d/%3d"

#define	PosFRAME	0
#define	NumFRAME	1	/* frame change button */
#define	PrevFRAME	2
#define	NextFRAME	3

#define	FileLabel	0
#define	FileLoad	1	/* file access button */
#define	FileSave	2
#define	FileReLoad	-1	/* for FileLoad and FileAccess. Not FButton */

#define	DataAnalys	0	/* data update button. boolean value */
#define	DateUpdate	1	/* never change these 2 definetions */

#ifndef	HistBottom
#define	HistBottom	1
#endif
#define	HistScaleSelf	0+HistBottom	/* hButton (SelfScale/Set button) */
#define	HistScaleSet	1+HistBottom
#define	HistBgNeg	2+HistBottom
#define	HistBgGrid	3+HistBottom

#define	ButtonRed	0	/* for color version */
#define	ButtonGreen	1
#define	ButtonBlue	2
#define	ButtonSync	3	/* 3 channel function together */

#endif	FUNC_H
