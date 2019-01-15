/*	PANEL . H
#
%	Panel Protocols
%
%	Copyright (c)	1991-1995	Jin Guojun -	All rights reserved
%
% AUTHOR:	Jin Guojun - Lawrence Berkeley Laboratory	2/1/91
*/

#ifndef	PANEL_H
#define	PANEL_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include "stdef.h"


#ifndef MAX
# define MAX(A, B)	((A > B) ? A : B)
#endif
#ifndef ABS
# define ABS(A)		((A < 0) ? -(A) : A)
#endif
#ifndef	Sign
#define	Sign(v)		((v < 0) ? -1 : v != 0)
#endif

typedef	XID	PColor;
typedef	XID	PWID_t;

/*	Setting Codes For PANELSET()	*/
#define	BUTTON_COLOR		1
#define	PRESS_BUTTON_COLOR	2
#define	SLIDER_COLOR		3
#define	PANEL_BACKGROUND	4
#define	PANEL_ACCESSORies	5


#ifdef	SCROLLBAR_on_CANVAS
#define	toREALxy(i, x, y)
#define	toRELATIVExy(i, x, y)
#define	REALxy(i, x, y)		x, y
#define	RELATIVExy(i, x, y)	x, y
#else
#define	toREALxy(i, x, y)	x += i->x0;	y += i->y0
#define	toRELATIVExy(i, x, y)	x -= i->x0;	y -= i->y0
#define	REALxy(i, x, y)		x + i->x0,	y + i->y0
#define	RELATIVExy(i, x, y)	x - i->x0,	y - i->y0
#endif


/* for passing message, it can be used for different purpose */

typedef	struct	{
	int	min, max,
		maxcnt;
	} Mregister;

typedef	struct	{
	int	screen, dpy_depth;
	Visual	*visual;
	Colormap cmap;
	Display	*dpy;
	cookie_t	Mask, reserved;	/* waste space; so, silly osf CC */
	Window	root;
	} WinAttribute;

extern	WinAttribute	Monitor[2];
#define	Dpy		Monitor[0].dpy
#define	Screen		Monitor[0].screen
#define	Dpy1		Monitor[1].dpy
#define	Screen1		Monitor[1].screen
#define	Root_window	Monitor[0].root


typedef	struct	{
	char	*name, *history, *desc;
	Colormap colormap;
	Display	*dpy;
	GC	gc, igc;
	Window	frame, win, icon;
	XEvent	*event;
	int	x0, y0, width, height,
		icon_width, icon_height,
		font_w, font_h, ascent;
	} AnyWindow;


#ifndef	xview_panel_DEFINED	/* used in xview guide	*/

typedef	struct	{
	char	*name, *history, *desc;
	Colormap	colormap;
	Display		*dpy;
	GC		gc, igc;
	Window		frame, win, icon;
	XExposeEvent	*expose;
	int		x0, y0, width, height,
			icon_width, icon_height,
			font_w, font_h, ascent;
	Window		root;
	XEvent		event;
	} Panel;
#endif

typedef	struct	{
	char	*name, *info, *desc;
	PWID_t	pw_type;
	Panel	*pan;
	cookie_t	state, valid;
	PColor	color, vcolor, tcolor;
	VType	*ep;
	int	x, y, w, h;
	} AnyParts;

typedef	struct	{
	char	*name, **bname, *desc;
	PWID_t	pw_type;
	Panel	*pan;
	cookie_t	pressed, valid;
	PColor	dimcolor, vcolor, tcolor;
	VType	*ep;
	int	x, y, rmb, hei;	/* right most bound	*/
	PColor	*color;
	int	namex, *bw, numb;	/* button width & number */
	} Button;

typedef	struct	{
	char	*name, *nouse, *desc;
	PWID_t	pw_type;
	Panel	*pan;
	cookie_t	state, valid;
	union	{
	PColor	s[2];
	struct	{
		PColor	p, v;	/* pressed & regular visible colors */
		} act;
	}	color;
	PColor	tcolor;	/* title color */
	VType	*ep;
	int	x, y, len, hei;
	} PressButton;

typedef	struct	{
	char	*name, **menu, *desc;
	PWID_t	pw_type;
	Display	*dpy;
	GC	gc, igc;
	Window	frame, win, icon;
	VType*	interface;
	int	x0, y0, w, h, items, which,
		font_w, font_h, ascent;
	PColor	bgcolor, fgcolor;
	} PopMenu;

typedef	struct	{
	char	*name, *info, *desc;
	PWID_t	pw_type;
	Panel	*pan;
	cookie_t	vert, valid;
	PColor	rcolor, vcolor, tcolor; /* regular, visible, title colors */
	VType	*ep;
	int	rx, ry, rw, rl;
	PColor	*scolor;
	int	*sx, *sy, sw, sh, numb,	/* scrollbar parameter */
		scale, min, max,
		defscale, defmin, defmax,/* default scale, min and max */
		namex, infx, infy,
		lx, ly;	/* last pos. for SonC	*/
	} Slider;

typedef	struct	{
	char	*name, *vp, *desc;
	PWID_t	pw_type;
	AnyWindow	*awin;
	cookie_t	hvalid, vvalid;
	PColor	rcolor, vcolor, bcolor; /* regular, vert, bar colors.	*/
	VType	*ep;
	int	*rx, *ry, *rw, *rl;
	MType	type;	/* 0 horizontal, 1 vertical, or 2 both.	*/
	int	*bx, *by;
#ifdef	SCROLLBAR_on_CANVAS
	int	lx, ly;
	Window	h_swin, v_swin;
#endif
	} ScrollBar;

typedef	union	{
	Panel		pan;
	AnyParts	aparts;
	AnyWindow	awin;
	Button		button;
	PressButton	pbutton;
	PopMenu		popmenu;
	Slider		slider;
	ScrollBar	scrollbar;
	} PanelParts;

#define	RESETSTATE	0
#define	ButtonState(button)	button->pressed
#define	PressButtonState(pb)	pb->state
#define	ShowPanel(pan)		XMapWindow(pan->dpy, pan->frame)
#define	HidingPanel(pan)	XUnmapWindow(pan->dpy, pan->frame)
#define	HidingPopMenu(pm)	XUnmapWindow(pm->dpy, pm->win)
#define	GetGray(dpy, cmap, entries, gray)	\
	GetCloseColor(dpy, cmap, entries, NULL, gray, gray, gray)

#define	PopButtonMask	Button1


typedef	struct	{
	char	*name, *history, *desc;
	Colormap colormap;
	Display	*dpy;
	GC	gc, igc;
	Window	frame, win, icon;
	XEvent	*event;
	int	x0, y0, width, height,
		icon_width, icon_height,
		font_w, font_h, ascent;
	XImage	*image, *icon_image;
	int	curve, linearlow, linearup, bgrd, fgrd,	/* elastic curve */
		scale, tmp_offset,	/* this 2 fields used for anything */
		mark_x, mark_y, resize_w, resize_h,
		sub_img_x, sub_img_y, sub_img_w, sub_img_h,
		mag_fact, mag_mode,	/* display magnified image	*/
		mag_x, mag_y, mag_w, mag_h,/* subimage currently being viewed */
		save_mag_x, save_mag_y, save_mag_w, save_mag_h,
		save_mag_fact, dpy_depth,
		frames, fn, channels, dpy_channels,
		*hist, *histp;	/* histogram buffer & pointer	*/
	byte	*data, *scan_data,	/* original data & scan buffers	*/
		*cnvt,
		*img_buf, *icon_buf;	/* map buffers for image & icon	*/
	bool	sub_img, sub_img_enh,	/* sub window and enhance	*/
		active, rw_cmap,	/* writable colormap	*/
		color_dpy,		/* False if black/white	*/
		dither_img, update,
		logscale, setscale,	/* use unique / own max histogram count */
		load_all, save_all;
	int	in_type, mid_type, o_type, in_color, color_form,
		in_form, o_form, pxl_in, pxl_out;
	Mregister	mmm, *marray;
	eX_thread	*ethread;	/* func_ps to below are not duplicated	*/
	io_content	i, o;		/* pipe & RTP */
	StdInterface	*eof, *errors, *header_handle, *std_swif;
	TableInterface	(*table_if)(), **table, tables;
	superimpose_elems	**superimpose;
	PanelParts	*parts;
	short	draws, texts, stack_num;/* parts in part stack	*/
	void	(*map_scanline)(), (*MAG_scanline)();
	bool	binary_img,
		mono_img,
		mono_color,
		sep_colors,
		pixmap_failed;	/* as update_header flag in U_IMAGE	*/
	int	entries,	/* number of colors or gray levels	*/
		ncmap, cmaplen;	/* in regular length (not 2 ** cmaplen)	*/
	unsigned *pixel_table;
	cmap_t	**in_cmap;
	int	lvls, lvls_squared, visual_class;
	Visual	*dpy_visual;
#if	defined	EXTENDED_COLOR | defined C_TUNER
	Window	pix_info_window;
	Pixmap	pixmap, icn_pixmap, mag_pixmap, refresh_pixmap;
	float	gamma, dpy_gamma;
	char	*title;
	int	*modN, *divN, **dm16;
#endif
	} Image;


#if	defined	C_TUNER || defined RLE_IMAGE

#include "getx.h"

typedef struct	{
	char	*name, *history, *desc;
	Colormap colormap;
	Display	*dpy;
	GC	gc, icn_gc;
	Window	frame, window, icn_window;
	XEvent	*event;
	int	x, y, w, h,	/* origin, width & height of image	*/
		icn_w, icn_h, font_w, font_h, ascent;
	XImage	*image, *icn_image;
	int	curve, linearlow, linearup, bgrd, fgrd,
		scale, tmp_offset,
		mark_x, mark_y, resize_w, resize_h,
		sub_img_x, sub_img_y, sub_img_w, sub_img_h,
		mag_fact, mag_mode,/* current magnification factor	*/
		mag_x, mag_y, mag_w, mag_h,
		save_mag_x, save_mag_y, save_mag_w, save_mag_h,
		save_mag_fact, dpy_depth,
		img_num, RGB,	/* operated channel	*/
		img_channels,	/* # of color channels in file	*/
		dpy_channels,	/* # of channels can display	*/
		*hist, *histp;
	byte	*data, *scan_data,	/* [h][w][chnl] */
		*cnvt, *img_buf, *icon_buf;
	bool	sub_img, sub_img_enh,
		active, rw_cmap,
		color_dpy, dither_img,	/* dither it? (-a) */
		update, logscale, setscale,
		load_all, save_all;
	int	in_type, mid_type, o_type, in_color, color_form,
		in_form, o_form, pxl_in, pxl_out;
	Mregister	mmm, *marray;
	eX_thread	*ethread;	/* extended event handler	*/
	io_content	i, o;
	StdInterface	*eof, *errors, *header_handle, *std_swif;
	TableInterface	(*table_if)(), **table, tables;
	VType	**superimpose;	/* all SIs can be stored in image headers */
	PanelParts	*parts;
	short	draws, texts, stack_num;
	VOID_FUNCTION	*map_scanline,	/* for scanline routines using	*/
			*MAG_scanline;
	bool	binary_img,	/* make it 2 colors (-W)	*/
		mono_img,	/* make it grey scale (-w)	*/
		mono_color,	/* a one channel color image (mcut)	*/
		sep_colors,	/* is the visual True or DirectColor?	*/
		pixmap_failed;
	int	entries,	/* number of colors or gray levels	*/
		ncmap,		/* rle_hdr.ncmap */
		cmaplen;
	Pixel	*pixel_table;
	rle_pixel**	in_cmap;/* LAST item in the Image structure */

	int	lvls, lvls_squared,
		visual_class;
	Visual	*dpy_visual;
	Window	pix_info_window;
	Pixmap	pixmap, icn_pixmap, mag_pixmap, refresh_pixmap;
	float	gamma, dpy_gamma;
	char	*title;
	int	*modN, *divN;	/* dither arrays, all of them	*/
	array16 *dm16;
	} image_information;

typedef	union	{
	Image	b_w;
	image_information	color;
	} *IMAGESP;

#endif	C_TUNER


typedef	struct	{
	Image	*his;
	int	*hist, *histp, grids, neg, scale;/* is in logarithm */
	bool	change, map;
	void	*lkt;
	} HistoInfo;

#include "p_widget.h"

typedef	struct	{
	char	*f_name,/* name of the file to be appended to by others	*/
		**buf_list;	/* buffers for all frame appending	*/
	Image	*simg;		/* shared image structure for stat I/O	*/
		/* it contains info. of image to be appended to by.	*/
	Panel	*pan;	/* appending control panel&	*/
	PressButton	*add, *finish;
	int	adir, any,	/* if sub region selected for appending	*/
		nbufs,		/* # of pointer buffers in the buf_list	*/
		frames;		/* # of frames to be appened to f_name	*/
	} ED_Append_t;

/*	End of Type Define	*/

	/*	Exports	*/
Image		*CreateImage(/* *ip, name, attr, w, h, i_w, mask, bf */);
Panel		*CreatePanel(/* x0, y0, w, h, name, attr, mask */);
Button		*CreateButton(/* *panel, x, y, n, name, blist, bc, pc */);
PressButton	*CreatePressButton(/* *panel, x, y, name, prscolor */);
PopMenu		*CreatePopMenu(/* *panel, attr, name, menu, num */);
Slider		*CreateSlider();
ScrollBar	*CreateScrollBar();
extern	ScrollBar	**SB_Stack;
char	*ReadButton(), *strcpy();
void	EraseSlider(), DestroyImage(), LoadIcon(),
		PanelSet(/* VType *item, int SettingCode, ... */);
int	CreateWindow(/* *wp, attr, cursor, mask, fixedframe, flag */),
	DrawSlider(), DrawButton(), DrawPressButton(), PopingMenu(),
	ReadSlider(/* *sp, bar_number */),
	SliderInfo(), SetSBarPos(), SetSBarRpos(),
	WhichImage(/* win, *imgp, total_images */),
	TrackSubWin(/* *img, hist_info, X0, Y0 */),
	Draws(/* *img, x_pos, draw_only, shape */),
	DrawVMark(/* *img, x_pos, clean */);
bool	ButtonPressed(), OnSliderBar(), OnButton();
Font	SetFontGetSize();	/* =u_long defined in X.h L42 */
	/* detect the available color map entries */
PColor	GetVctEntry(/* dpy, scr, cmap, set_grays */),
	GetCloseColor(/* dpy, cmap, entries, r, g, b */);
extern	PColor	Black, White, Visible, darkGray, lightGray, Gray,
		Red, Green, Blue, Yellow, black1, white1, gray1, HBGround;

	/*	import */
extern	bool	debug, tuner_flag, OsameI, multi_hd;
extern	char	*DrawShape[];

#ifndef	MaxColors
#define	MaxColors	256
#endif

#define	HistoSize	256	/* histogram bins */
#define	BGround		7	/* pseudo panel background color */
#define	BFRAME		1	/* histogram image frame thickness */

#define	StdIconWidth	128
#define	RailWidth	4	/* slider rail width */
#define	HandleWidth	10	/* sliding bar or scrollbar size */
#define	HandleHeight	20
#define	TextVMargin	15	/* vertical and */
#define	TextLMargin	8	/* horizontal margin */
#define	ButtonHeight	20
#define	IIMargin	4	/* Image Infomation Margin */

#define	DoAll		(DoRed | DoGreen | DoBlue)
#define	ButtonAction	(ButtonPressMask | ButtonReleaseMask)
#define	DrawsArc	1
#define	DrawsLine	2
#define	DrawsRect	3
#define	DrawArc( a,b,c)		Draws(a,b,c,DrawsArc)
#define	DrawCrop(a,b,c)		Draws(a,b,c,DrawsRect)
#define	DrawLine(a,b,c)		Draws(a,b,c,DrawsLine)
#ifndef	CropButton
#define	CropButton	Button3Mask
#endif

#endif	PANEL_H
