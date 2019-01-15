/*	tuner . h
#
%	Copyright (c)	1991-1995	Jin Guojun
%
%	global definitions for tuner handle routines.
%
% AUTHOR:	Guojun Jin - LBL	4/1/91
% Date of last modification:	10/1/94
*/

#ifndef	X_WINDOW_DEP
#define	X_WINDOW_DEP
#endif	X_WINDOW_DEP_FOR_INCLUDE_PANEL.H

#include "function.h"
#include <fcntl.h>
#include <errno.h>
#ifndef	__STDC__
extern	char	*sys_errlist[];
#endif

#if	!defined sparc | !defined BSD4
#include <sys/stat.h>
#endif

#ifdef	DIRECT
#	define	NCOLOR	MaxColors
#	define	IBNeed	False
extern	bool	frmchange;
#else
#	define	NCOLOR	64	/* good enough for eyes	*/
#	define	IBNeed	True
#endif

#ifndef	MAX_IMAGE_PTRS
#define	MAX_IMAGE_PTRS	64	/* default maximum image handlers	*/
#endif

#ifndef	ToleranceFactor
#	define	ToleranceFactor	3
#endif

#define	MENU1_INFO	1	/* must match names in info_any.c	*/
#define	MENU1_ANOT	2
#define	MENU1_APPEND	3
#define	MENU1_COPY	4
#define	MENU1_CROP	5
#define	MENU1_CUT	6
#define	MENU1_DELETE	7
#define	MENU1_DRAW	8
#define	MENU1_MEAN	9
#define	MENU1_PAINT	10
#define	MENU1_PASTE	11
#define	MENU1_SNAP	12
#define	MENU1_UNDO	13

#define	MENU2_FITSType	1
#define	MENU2_ETAScale	2
#define	MENU2_ITPRange	3
#define	MENU2_RGBScale	4
#define	MENU2_BackGD	5
#define	MENU2_DPYMode	6
#define	MENU2_UNDO	7
#define	vroot_dpy	cmn_hd.frame

#define	MENU3_BLKFrm	1
#define	MENU3_CDIR	2
#define	MENU3_LDFrm	3
#define	MENU3_MAP123	4
#define	MENU3_OType	5
#define	MENU3_QUIT	6
#define	MENU3_UNDO	7

#define	MENUF_WHITE	0
#define	MENUF_BLACK	1
#define	MENUF_PKCOLOR	2
#define	MENUF_1stFONT	3
#define	MENUF_lastFONT	10
#define	MENUF_UNDO	MENUF_lastFONT+2

#define	numctrl	MENU1_UNDO
#define	numpara	MENU2_UNDO
#define	numcomd	MENU3_UNDO
#define	numfont	MENUF_UNDO


extern	Image*	*pic, cmn_hd;
extern	Button	*MsgButton;
extern	CENR	cer[3];		/* color editor registers */
extern	Colormap	firstmap;
extern	EditorSpace	I_ED;
extern	InterpMap	*IM;	/* interpolation matrix */
extern	LKT	*lkt;
extern	Panel	*NoteWin, *InfoWin;
extern	PopMenu	*ctrlmenu, *paramenu, *filemenu, *fontmenu;
extern	PressButton	*YesButt, *NoButt, *OkButt, *AbortButt;
extern	char	*filelist[], *ctrlist[], *paralist[], *fontlist[],
		*PaintMesg, *ITypeName[];
extern	bool	clickon,	/* for pointer motion	*/
		newmap, moved,
		cquire, cca, verbose, quant,
		start_fast, YesOrNo();
extern	int	ncolors, VCTEntry, fnt_r, fnt_g, fnt_b, num_images,
		x_regions, y_regions,
		fontWidth, fontHeight,	/* public font size */
		RED_to_GRAY, GREEN_to_GRAY, BLUE_to_GRAY;
extern	Cursor	arrow, cursor;		/* for any one to use */
extern	WinAttribute	Monitor[2];

#define	frm	cmn_hd.frames
#define	row	cmn_hd.height
#define	cln	cmn_hd.width

int	Panel_init(), MapColor(), FileAccess(), LoadFile(),
	HistoHandle(), Find_min_max(),
	CreateTuner(/* U_IMAGE *img, ela_scale, panel_bg, map_panel */),
	Get_Note_Input(/*buf, buf_size, sp_limit, info, color, dmsg */),
	Button1_On(/* XButton *xbutton, int* feedback */),
	SaveImage(/* U_IMAGE* */), WaitOk();
void	ShowMessageWin(), DisplayMessage(),
	TopWindow(), Toggle_Info(),
	DrawPanel(), Exposure_handler(),
	PaintImage(), PasteImage(), SetImageEvent(),
	Panel_Image(), Fresh_ImageScreen(), ResetORange(),
	ChangePanelCmap(/* U_IMAGE *img */), Set_Monitor(),
	ResetLKT(/* LKT* lkt, U_IMAGE* img */);
Colormap	SetColormap();
Window	DestroyColorImage();

#define	CONTROL_EVENT	0
#define	HISTO_EVENT	1
#define	MOVIE_EVENT	2
#define	MAGNIFY_EVENT	3
#define	MAGNIFY_SHIFT	4
#define	UNMAGNIFY	5
#define	MENU1		6
#define	MENU2		7
#define	MENU3		8
#define	DEFAULT_ACTION  9	/*	do nothing	*/

#define ACTION_MAGNIFY		0	/*	ACTION_MOVIE_FORWARD	*/
#define ACTION_UNMAGNIFY	1	/*	ACTION_MOVIE_STEP	*/
#define ACTION_PAN		2	/*	ACTION_MOVIE_BACKWARD	*/
#define ACTION_PIXEL_INFO	3	/*	ACTION_MOVIE_SPEED	*/
#define	ACTION_SUB_WINDOW	4	/*	ACTION_CHDIR	*/
#define ACTION_SWITCH_MAG_MODE	5	/*	ACTION_CYCLE	*/
#define ACTION_OBJECT		6
#define ACTION_TUNER		7
#define ACTION_DEFAULT		ACTION_PAN
