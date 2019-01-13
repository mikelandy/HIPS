/*	TUNER . C
#
%	Copyright (c)	Jin Guojun	-	All Rights Reserved
%
%	The ELASTIC TUNER main routine Tuner(), editor routines.
%
%	There are 2 places (case fButton & Button3) re-using histogram().
%	Of course, if there are more memory in system, this time can be saved.
%	Two versions are set up here:
%		direct color mapping and quantizing mode.
%
% compile:	-DDIRECT will generate fast version, but will not share
%		color map with other images runing with it. And this fast
%		version is mainly used for image viewing, not editing.
%
% Author:	Jin Guojun - Lawrence Berkeley Laboratory	4/1/91
*/

#include "tuner.h"
#include "info_any.c"

#define	Put_Image(img)	XPutImage(img->dpy,	\
		img->refresh_pixmap ? img->refresh_pixmap : img->win,	\
		img->gc, img->image, 0, 0, 0, 0, img->width, img->height);

bool	clickon, moved,	/* for pointer motion	*/
	newmap, cquire, cca, verbose, quant;
int	BackGD, num_images,
	x_regions=2, y_regions=2,
	fontWidth, fontHeight;	/* public font size */
LKT	*lkt;
MType	gf_size;
Image	**pic;
Cursor	arrow;
EditorSpace	I_ED;
Image	cmn_hd;
static	char	lbuf[128], *EmptySheet="empty frame";
#define	lmesgbuf	lbuf + 64
#define	Get_Note_Input(spl, info, icolor, show_pw)	\
	InputFrom_Panel(MsgButton, lbuf, sizeof(lbuf), spl,	\
			lmesgbuf, info, icolor, show_pw)

#ifndef	HISTO_BACKGROUND
#define	HISTO_BACKGROUND	lightGray
#endif

static	int	MBUTTON_ACTION[3][3]	= {
	CONTROL_EVENT,	MAGNIFY_EVENT,	MENU1,
	HISTO_EVENT,	MAGNIFY_SHIFT,	MENU2,
	MOVIE_EVENT,	UNMAGNIFY,	MENU3
	};

Update_imageNpanel(img)
Image	*img;
{
	gf_size =	GetImageBufNSize(img);
	img->mmm.min = img->marray[img->fn].min;
	img->mmm.max = img->marray[img->fn].max;
	if (!img->setscale)
		img->mmm.maxcnt = img->marray[img->fn].maxcnt,
		ResetORange(img);
	if (histinfo.map)	{
		histogram(img->cnvt, gf_size, histinfo.histp, 0);
		HistoHandle(img, &histinfo, HISTO_BACKGROUND);
		sprintf(lbuf, "%d", img->mmm.maxcnt);
		DispInfo(Epanel, 240, lbuf, Green);
		SetShowFramePos(img, fButton, 0);
	new_curve(lkt, &histinfo, &img->mmm, img->curve, 0,
		img, gf_size);	/* maybe always needed	*/
	}
#	ifdef	DIRECT
	frmchange++;
	MapColor(img, img->cnvt, gf_size,
		graylevel, MAX(VCTEntry, img->mmm.min));
#	else
	MapColor(img, img->cnvt, gf_size);
#	endif
}

#ifdef	C_TUNER

#define	ImageBackup(img)	if (img->refresh_pixmap)	\
	XCopyArea(img->dpy, img->refresh_pixmap, img->win, img->gc,	\
		0, 0, img->width, img->height, 0, 0);

Tuner(imgp, max_images, active)
Image	**imgp;
bool	*active;
{
register MType	i;
Image	*imginfo, *img;
int	sb;
XEvent		event;	/* p16 Xlib.h	*/
XExposeEvent	*expose;/* p11 Xlib.h	*/
XCrossingEvent	*xcrossing;
expose = (XExposeEvent *) &event;
xcrossing = (XCrossingEvent *) &event;

for (i=0; i<num_images; i++)
	imgp[i]->event = &event;
for (i=0; i<num_images; i++)
	if (imgp[i]->active)	break;
if (i >= num_images)
	i = 0;
imginfo = img = imgp[i];
PanelMessage(FButton, img->name, 0, 0, 0);
if (img->marray)
	memcpy(&img->mmm, img->marray, sizeof(img->mmm));
img->cnvt = img->data;

if (*active<0)	ShowPanel(Epanel);

#else	Gray_Scale_tuner


main(argc, argv)
int	argc;
char	**argv;
{
register int	i;
int	VCTolerance=0, max_images=12, ela_scale, nf, xy_ext=0;
float	thdscale=0.005000001;
char	**fl;
#define	force_dpy	xy_ext

format_init(&cmn_hd, IMAGE_INIT_TYPE, HIPS, HIPS, *argv, "Oct1-4");
if ((nf=parse_argus(&fl, argc, argv, arg_list,
#ifdef	DIRECT
	&VCTolerance,
#else
	&ncolors,
#endif
	&cca, &cquire,
	&DEBUGANY, &DEBUGANY,
	&display_name, &display_name,
	&thdscale, &ela_scale,
	&max_images,
	&newmap, &precision,
	&start_fast, &verbose,
	&xy_ext, &x_regions,	/*&y_regions,*/
	&x_regions, &y_regions)) < 0)	exit(nf);

if (!xy_ext)	y_regions = x_regions;
if (!VCTolerance)
	VCTolerance = (ncolors>>ToleranceFactor)+16;
if (precision<128)	/* small value may get more close color */
	precision = 256;	/* but easy to failure */

	/* Open the display & set defaults */
if ((Dpy1 = XOpenDisplay(display_name)) == NULL)
	syserr("Can't open display '%s'", XDisplayName(display_name));
if (force_dpy = !display_name)
	display_name = XDisplayName(display_name);
i = strlen(display_name);
msg("display = %s\n", display_name);

if (force_dpy && display_name[i-1] != '0') {	/* number 0 {48H in ASCII} */
char	str[64];
	strcpy(str, display_name);
	str[i-1] = '0';	/* force image on main screen	*/
	if ((Dpy = XOpenDisplay(str)) == NULL)
		syserr("Can't open display '%s'", str);
	msg("display = %s\n", str);
}
else	Dpy = Dpy1;
Set_Monitor(NULL, Dpy, Dpy1, NULL);

if (XDisplayPlanes(Dpy1, Monitor[1].screen) == 1)
	mesg("control panel is on a monochrome screen\n");

dgt = (int*) ZALLOC(MaxColors, sizeof(*dgt), "dgt");
if (Monitor[0].dpy_depth < 24)
	GetVctEntry(Dpy, Screen, Monitor[0].cmap,
		start_fast<2 && Monitor[1].dpy_depth>1 ||
		Dpy==Dpy1 && Monitor[1].dpy_depth==1);
else	VCTEntry = 1;

#ifdef	DIRECT
if (VCTEntry<1 || ncolors+VCTEntry-MaxColors > VCTolerance)
	newmap--;
CreateCLT(graylevel, ncolors, DoAll, 0, False, &histinfo);
Monitor[0].cmap = SetColormap(&Monitor[0], graylevel, ncolors, &newmap,
	VCTEntry);
#else
if (VCTEntry < 1)	newmap--;
else if (ncolors+VCTEntry-MaxColors > VCTolerance)
	ncolors >>= 1;
CreateCLT(graylevel, ncolors, DoAll, 0, True, &histinfo);
Monitor[0].cmap = SetColormap(&Monitor[0], graylevel, &ncolors, &newmap, 0);
#endif
if (Dpy == Dpy1)
	Monitor[1].cmap = Monitor[0].cmap;
if (max_images<1 || max_images>256)
	max_images = 64;

cmn_hd.dpy = Dpy;
cmn_hd.dpy_depth = Monitor[0].dpy_depth;
CreateTuner(&cmn_hd, ela_scale, darkGray, True);

pic = (Image**) ZALLOC(max_images, sizeof(**pic), "pic");

	/* if stdin is ready, load image. */
i = True;
io_test(fileno(stdin), i=iset);
if (i)	LoadImage(stdin, pic, fname);
else  while (i < nf)	{
FILE*	fp = zreopen(fname=fl[i], No /* no clean for /tmp/ztmp */, NULL);
	if (!fp)
		prgmerr(i < nf, "input %s", fname);
	else	LoadImage(fp, pic+i, fname);
	i++;
}
Tuner(pic, max_images);		/* analyse images */
}	/* End of Main	*/


Tuner(imgp, max_images)
Image	**imgp;
{
Image	*img,	/* = imgp[0], only set by load, map, enter.
			So, it always represents the current image */
	*imginfo=NULL;	/* temp image pointer, parameter window	*/
int	sb;
XEvent		event;	/* p16 Xlib.h	*/
XExposeEvent	*expose;/* p11 Xlib.h	*/
XCrossingEvent	*xcrossing;
register MType	i=0;

while (img=imgp[i]) {
	img->cnvt = img->data;	i++;
	img->event = &event;
}
if (num_images = i)
	(img=imgp[--i])->active = True;
XSelectInput(histinfo.his->dpy, histinfo.his->win, I_Mask);
expose = (XExposeEvent *) & event;
xcrossing = (XCrossingEvent *) & event;

I_ED.copy = (Image*) NZALLOC(sizeof(*I_ED.copy), 1, "I_copy");

#endif 	C_TUNER


/*	public tuner()	*/

Loop	{	/* the loop to maintain the images.	PUBLIC entry */
#ifdef	_DEBUG_
#include <time.h>
time_t	t0, t1;
#endif

int	mevent, y0, new_colors=ncolors, ps=1;
Window	wp;	/* long, defined in X.h */
KeySym	keysym;
XComposeStatus	stat;
	if (event.type != MotionNotify) {
	    sprintf(lbuf,
		"(q)uit   {%d}%-8d BUTTON => [1]Ctrl   [2]Hist   [3]Movie",
		histinfo.scale, img?img->mmm.maxcnt:0);
	    DispInfo(Epanel, 0, lbuf, white1);
	}
	if (Dpy == Dpy1)
		XNextEvent(Dpy, &event);/* waiting for event */
	else	{
		i = 0;
		while (!XEventsQueued(Dpy, QueuedAfterFlush) &&
			!(i=XEventsQueued(Dpy1, QueuedAfterFlush)));
		if (i)	XNextEvent(Dpy1, &event);
		else	XNextEvent(Dpy, &event);
	}

	wp = event.xany.window;
	switch ((int) event.type) {
	case Expose:
		if ((i=WhichImage(wp, imgp, num_images))>=0)
			imginfo = imgp[i];
		Exposure_handler(expose, imginfo, NULL);
	XFlush(Dpy);
	break;

	case ButtonPress:
	    i = event.xbutton.button;
	    mevent = event.xbutton.state & (ShiftMask | ControlMask);
	    if (mevent>2)	mevent = 2;
	    mevent = MBUTTON_ACTION[i-Button1][mevent];
	    if (OnButton(FButton, &event.xbutton) > 0 &&
		(i=FileAccess(imgp, max_images, num_images))) {
			if (img)	img->active = False;
			if (i == EOF)	{	/* SUCCESSful LOADing	*/
			/* not a loading process. What about i==0 ?	*/
				if (!(img = imgp[num_images]))	break;
				img->event = &event;
				num_images++;
			}
			else	img = imgp[i];
			img->active = True;
#	ifdef	DIRECT
			frmchange++;
#	endif
			img->cnvt = img->data;
			break;
	    }
	    else if (mevent==MENU2) {
		switch (i=PopingMenu(paramenu, numpara, NULL)) {
		case MENU2_FITSType:	{
#ifdef	FITS_IMAGE
		extern	int	FTy;
			sprintf(lmesgbuf, "[%c] Unix Vax Pc T->unix_vax", FTy);
			Get_Note_Input(7, "TYPE ", Yellow, No);
			FTy = lbuf[0];
			if (FTy != 'u' && FTy != 'v' && FTy != 'p' && FTy != 't')
				FTy = 'u';
#endif
		}	break;
		case MENU2_ETAScale:
			if (!img)	break;
			sprintf(lmesgbuf, "\n\nlevel = %d (Max : 65)", img->scale);
			Get_Note_Input(8, "scale ", Yellow, Yes);
			img->scale = i = atoi(lbuf);
			ChangeSliderScale(ESlider, i, slider==ESlider);
			break;
		case MENU2_ITPRange:
			sprintf(lmesgbuf, "level = %d, Max level = 8", x_regions);
			Get_Note_Input(8, "level ", Green, No);
			i = atoi(lbuf);
			if (i<2 || i>MAX_ITP_LEVEL)	i = 2;
			x_regions = y_regions = i;
			break;
		case MENU2_RGBScale:
			sprintf(lmesgbuf, "R-G-B = %d %d %d",
				RED_to_GRAY, GREEN_to_GRAY, BLUE_to_GRAY);
			Get_Note_Input(0, "R.G.B factor ", Green, No);
			sscanf(lbuf, "%d %d %d",
				&RED_to_GRAY, &GREEN_to_GRAY, &BLUE_to_GRAY);
			break;
		case MENU2_BackGD:
			sprintf(lmesgbuf, "Background = %d", BackGD);
			Get_Note_Input(0, "new back ground ", Green, No);
			BackGD = atoi(lbuf);
			break;
		case MENU2_DPYMode:
			if (vroot_dpy)	{
				vroot_dpy = 0;
				strcpy(paralist[i-1], dpy_root_mode);
				ResetWindowBackground(cmn_hd.win, 0);
			} else	{
				vroot_dpy++;
				strcpy(paralist[i-1], clr_root_mode);
			}
			CopyToRootWindow(&cmn_hd, vroot_dpy ? img : NULL,
					cmn_hd.win);
			break;
		default:	XBell(Epanel->dpy, 10);
		}
		HidingPanel(NoteWin);
		break;
	    }
	    else if (mevent==MENU3)	{
		char	errbuf[128];
		switch(i = PopingMenu(filemenu, numcomd, NULL))	{
		case MENU3_BLKFrm:	{
			int	w, h;
			strcpy(lmesgbuf, "new frame size:\n width height");
			Get_Note_Input(0, NULL, Green, No);
			sscanf(lbuf, "%d %d", &w, &h);
			if (w < 64 || h < 64)
				WaitOk(AbortButt, "size to small", 0);
			else {
			register int	chan = cmn_hd.color_dpy ? 3 : 1;
			register byte	*clean;
				if (num_images >= max_images)
					break;
#	ifdef	C_TUNER
				if (chan==3) {
				int	icn_f;
					img = imgp[num_images] = (Image*)ZALLOC(1,
						sizeof(image_information), EmptySheet);
					init_img_info(img, Dpy, RLE, cmn_hd.color_dpy);
					img->active = img->pixmap_failed = 1;
					init_img_flag(img);
					img->width = w,	img->height = h;
					img->channels = img->dpy_channels = chan;
					img->name = EmptySheet;
					find_appropriate_visual(img);/* may copy from parent */
					BuildColorImage(img, NULL, NULL, &icn_f);
				} else
#	endif
				{   	imgp[num_images] = NULL;
					BuildImage(imgp+num_images, "tmp", w, h,
						1, NULL, chan==1 ? HIPS : RLE, True);
					img = imgp[num_images];
					img->colormap = Monitor[0].cmap;
					img->channels = img->dpy_channels = chan;
				}
				num_images++;
				img->color_form = chan==3 ? CFM_ILL : CFM_SCF;
#	ifdef	SCROLLBAR_on_CANVAS	/* just for fun ??	*/
		img->in_type = img->o_type = RLE;
#	endif
				SetImageEvent(img, &event);
				img->data = clean = nzalloc(w *= h, chan, "blk-data");
				if (chan > 1)
					img->scan_data = nzalloc(w, chan, "blk-scan");
				for (chan *= w; chan--;)
					clean[chan] = -1;
			}
		}	break;
		case MENU3_CDIR:
		/*	sprintf(lmesgbuf, "%s\nChange directory to",
				getcwd(NULL, 256));
			Get_Note_Input(0, NULL, Green, No);
		*/	GetFilenameFromX(MsgButton, lbuf, 0 /* dirs only */);
			if (chdir(lbuf)) {
				sprintf(errbuf, "wrong directory:\n  %s", lbuf);
				WaitOk(NULL, errbuf, 0);
			}
			break;
#	ifdef	C_TUNER
		case MENU3_MAP123:
			OsameI = YesOrNo("Map 1 to 3 channels for input ?", 0);
			break;
		case MENU3_LDFrm:
			sprintf(lmesgbuf, "frame = %d", img->fn);
			Get_Note_Input(0, NULL, 0, No);
			img->fn = atoi(lbuf);
			break;
#	endif
		case MENU3_OType:
			sprintf(lmesgbuf, "OUTPUT TYPE : %s\nHIPS, RAS, RLE",
				ITypeName[img->o_type]);
			Get_Note_Input(0, NULL, 0, No);
			if ((i=available_type(lbuf)) > 0)
				img->o_type = i;
			break;
		case MENU3_QUIT:	goto	MEXIT;
		}
		HidingPanel(NoteWin);
		break;
	    }
	    else if (!img)	/* start image related events	*/
		break;
	    if (general_PW_manager(img, &event, NULL))
		break;	/* performed user wedget event control	*/

	    if ((wp==img->win || wp==histinfo.his->win) &&
		(mevent==CONTROL_EVENT || mevent==MOVIE_EVENT))	{
		if (wp==img->win) {
			XDefineCursor(img->dpy, wp, 0);
			imginfo = img;
		}
		else{	imginfo = histinfo.his;
			imginfo->mmm = img->mmm;
		}
		y0 = SetParameterWin(imginfo, &event, fontHeight, i==Button3);
	    }
	    switch (mevent) {
	    case CONTROL_EVENT:	/* on Botton1 */
		if (i=Button1_On(&event.xbutton, &sb))
		switch (i)	{
		    case OnETASlider:
			clickon=ELALINFO;	break;
		    case OnClipSlider:
			clickon=CLIPINFO;	break;
		    case OnQuanSlider:
			clickon=QUANINFO;	break;
		    case OnZcntButton:
			cntz = ButtonState(ZButton);	break;
#	ifndef	DIRECT
		    case OnQuanButton:
			quant = ButtonState(QButton);
			if (!quant)	{
				moved = new_colors = ncolors;
				CreateCLT(graylevel, ncolors, DoAll, quant, 1, &histinfo);
				SetColormap(&Monitor[0], graylevel, &new_colors, &newmap, !quant);
			}
#	endif
			break;
		    case OnFrameButton:
#ifdef	C_TUNER
			img->fn = ButtonState(fButton);
			i = img->fn % 3;
			img->mmm.min = img->marray[img->fn%img->channels].min;
			img->mmm.max = img->marray[img->fn%img->channels].max;
			img->curve = ButtonState(EButton) = cer[i].curve;
			if (img->curve != ETALinear)
			    SetSBarPos(ESlider, img->curve ?
				cer[i].bgrd : cer[i].fgrd, 1);
			img->linearlow = cer[i].lower;
			img->linearup = cer[i].upper;
			ChangeSlider(img, &slider, ESlider, LSlider, heqButt, EButton);
			PressButtonState(heqButt) = RESETSTATE;
			DrawPressButton(heqButt);
#else
			if (img->frames>1) switch(ButtonState(fButton))	{
			case NumFRAME:
				sprintf(lbuf, "Input frame# <= %d", img->frames);
				i = (strlen(lbuf)+2) * Epanel->font_w;
				DispInfo(Epanel, 0, lbuf, white1);
				lbuf[0] = 0;	/* clean buf */
				TextLine(fButton, lbuf, sizeof(lbuf), i,
					Exposure_handler, imgp, num_images);
				i = atoi(lbuf);
				if (i>0 && i<=img->frames)	img->fn = i-1;
				break;
			case PrevFRAME:	if (img->fn)	img->fn--;
				break;
			case NextFRAME:	if (img->fn<img->frames-1)
						img->fn++;
			}
			ButtonState(fButton)=RESETSTATE;
			sprintf(lbuf, fb_format, img->fn+1, img->frames);
			if (moved=strcmp(lbuf, fButton->bname[PosFRAME])) {
				gf_size = GetImageBufNSize(img);
				i = histogram(img->cnvt, gf_size, histinfo.histp, 0);
				img->mmm.min = img->marray[img->fn].min;
				img->mmm.max = img->marray[img->fn].max;
				if (!img->setscale)
					img->mmm.maxcnt = i,
					ResetORange(img);
				strcpy(fButton->bname[PosFRAME], lbuf);
#	ifdef	DIRECT
				frmchange++;
#	endif
			}
			DrawButton(fButton);
#	endif
			break;
		    case OnDataButton:	/* update image->data */
			Update_ImageData(img, lbuf);
		    case OnInterpolate:
			cer[0].intp = i==OnInterpolate;
			if (cer[0].intp && img->color_form == CFM_SCF)	{
				ButtonState(Interpolate) = RESETSTATE;
				break;
			}
			Find_min_max(img, histinfo.histp, img->cnvt, Yes, True);
		    case OnETAButton:
			img->curve = ButtonState(EButton);
#	ifdef	C_TUNER
			i = img->fn % 3;
			cer[i].curve = img->curve;
			img->bgrd = cer[i].bgrd;
			img->fgrd = cer[i].fgrd;
			img->linearlow = cer[i].lower;
			img->linearup  = cer[i].upper;
#	endif
			ChangeSlider(img, &slider, ESlider, LSlider, heqButt, EButton);
			moved++;
			break;

		    case OnHistButton:
			Histo_Setting(img, lbuf);
			break;
		    case OnResetButton:
		    {
			moved++;
			ResetORange(img);
		    }	break;
		    case OnRefresh:
			moved++;
			img->curve &= ~ETAHistoEq;
			break;
		    case OnHEQButton:
			moved = img->curve |= ETAHistoEq;
			break;
		    default:	/* a lazy job ! should do something better */
			if ((i=WhichImage(wp, imgp, num_images))>=0 &&
				img->parts && (sb=OnScrollBar(img->parts,
				&event.xbutton)))
				clickon = MOVESCROLLBAR;
			else if (wp != Epanel->win)	{
#	ifdef	C_TUNER
			    if (wp != histinfo.his->win)
				ColorImageInfo(img, !y0);
			    else
#	endif
				ParameterWin(imginfo, &histinfo,
					event.xbutton.x, event.xbutton.y,
					y0, wp==histinfo.his->win),
				clickon = IMAGEINFO;
			}
		} break;	/* end of Button1 */

	    case HISTO_EVENT:	/* on Button2	*/
		if (img)	Map_HistoWin(img);
		break;

	    case MOVIE_EVENT:	/* on Button3	*/
		XBell(Dpy, 0);
#	ifdef	_DEBUG_
		time(&t0);
#	endif
#	ifndef	C_TUNER
		if(OnButton(fButton, &event.xbutton) > 0 &&
			event.xbutton.window == Epanel->win)	{
		char	val[8];
			DispInfo(Epanel, 0, "s => stop", Yellow);
		   if (img->curve & ETAHistoEq)
			strcpy(lbuf, "Histo Equalization");
		   else if(img->curve<ETALinear)
			strcpy(lbuf, "Elastic");
		   else	strcpy(lbuf, "Linear");
		   DispInfo(Epanel, 100, lbuf, Yellow);
		   Loop	{
			if (ButtonState(fButton) == PrevFRAME)
			    if (img->fn)	img->fn--;
			    else	break;
			else if(img->fn<img->frames-1)	img->fn++;
				else	break;
			if (XCheckMaskEvent(Dpy, KeyPressMask, &event))	{
				XLookupString(&event, lbuf, sizeof(lbuf),
					&keysym, &stat);
				if (lbuf[0] == 's')	break;
			}
			Update_imageNpanel(img);
		   }	/* end of Loop	*/
		    SetShowFramePos(img, fButton, True);
		    if (img->sub_img)
			DrawCrop(img, 0, 1);
#	ifdef	_DEBUG_
		time(&t1);
		message("time = %d\n", t1-t0);
#	endif
		} else
#	endif
		if (event.xany.window == img->win)
			TrackSubWin(img, &histinfo, event.xbutton.x,
				event.xbutton.y, DrawsRect, CropButton, y0);
		else if (event.xany.window == histinfo.his->win)	{
			DrawVMark(histinfo.his, event.xmotion.x,
				histinfo.his->sub_img);
			histinfo.his->sub_img = clickon = HMARKPOS;
		}
		break;	/* end of MOVIE_EVENT	*/

	    case MENU1:	/* case ACTION_OBJECT:	*/
		if ((i=WhichImage(event.xany.window, imgp, num_images))>=0 &&
			(i=on_superimpose_elem(imginfo=imgp[i],
				event.xbutton.x, event.xbutton.y)))
			superimpose_handle(imginfo, i,
				event.xbutton.x, event.xbutton.y);
		else switch (i=PopingMenu(ctrlmenu, 2, Exposure_handler,
				imgp, num_images))	{
		case MENU1_INFO:
			Toggle_Info(Help_message_array1);
			break;
		case MENU1_APPEND:
			create_append(img);
			break;
		case MENU1_ANOT:
			if ((i=PrepareToEdit(imgp, num_images, PaintMesg,
				lbuf, sizeof(lbuf), &event)) >= 0)
				Annotation(imginfo=imgp[i], &y0, &keysym, &stat);
			break;
		case MENU1_CUT:	/* img == imginfo */
			img->update |= img->sub_img;
		case MENU1_COPY:
			if (img->sub_img)
				CopyOrCutSubImage(img, i==MENU1_CUT, BackGD);
			else	XBell(img->dpy, 0);
			break;
		case MENU1_CROP:
			sprintf(lmesgbuf, "size: %dH x %dW\ncrop => X0 Y0 height width",
				img->height, img->width);
			Get_Note_Input(8, "sub-image ", Yellow, No);
			sscanf(lbuf, "%d %d %d %d", &img->sub_img_x,
				&img->sub_img_y,&img->sub_img_h,&img->sub_img_w);
			bound_check(img->sub_img_h, img->sub_img_y, img->height);
			bound_check(img->sub_img_w, img->sub_img_x, img->width);
			img->sub_img = (img->height>4 && img->width>4);
			break;
		case MENU1_DELETE:
			if (!(img->texts | img->draws))	break;
			DisplayMessage(NoteWin, "click on OBJECT to delete it", 0, 0);
			XMaskEvent(Dpy, ButtonPressMask, &event);
			if ((i=WhichImage(event.xany.window, imgp, num_images))
				>= 0 && (i=on_superimpose_elem(imginfo=imgp[i],
				event.xbutton.x, event.xbutton.y)))
				XDeleteObject(imginfo, i);
			break;
		case MENU1_DRAW:
			if ((i=PrepareToEdit(imgp, num_images, PaintMesg,
				lbuf, sizeof(lbuf), &event)) >= 0)
				DrawInImage(imginfo=imgp[i], &y0, PaintMesg, lbuf);
			break;
		case MENU1_PASTE:
			if (!I_ED.fill)	break;
			DisplayMessage(NoteWin, PasteMesg, 0, 0);
			XMaskEvent(Dpy, ButtonPressMask, &event);
		    if ((i=WhichImage(event.xany.window, imgp, num_images)) >= 0)
			PasteImage(imginfo=imgp[i], &y0);
			break;
		case MENU1_PAINT:
			sprintf(lmesgbuf, "brush size = %d", ps);
			Get_Note_Input(8, "size ", Green, No);
			sscanf(lbuf, "%d", &ps);
			if ((i=PrepareToEdit(imgp, num_images, PaintMesg,
				lbuf, sizeof(lbuf), &event)) >= 0)
				PaintImage(imginfo=imgp[i], &y0, ps,
					fnt_r, fnt_g, fnt_b);
			break;
		case MENU1_MEAN:
#	ifdef	C_TUNER
			if (img->dpy_channels > 1 && img->sub_img)	{
			byte	*scan[3], *smap[3];
			register int	fact = get_iconsize(img, 0),
				x = img->sub_img_x, y = img->sub_img_y,
				iw = img->width;
			    CalcSubWinMean(img->data, img->data, iw,
				x, y, img->sub_img_w, img->sub_img_h);
			    for (i=img->sub_img_h; i--;)	{
				scan[0] = ORIG_RLE_ROW(img, y + i) + x;
				scan[1] = scan[0] + iw;
				scan[2] = scan[1] + iw;
				smap[0] = SAVED_RLE_ROW(img, y + i) + x;
				smap[1] = smap[0] + iw;
				smap[2] = smap[1] + iw;
				Map_Scanline(img, scan, smap, x, y + i,
					img->sub_img_w, fact);
			    }
			    if (img->refresh_pixmap)
				XPutImage(img->dpy, img->refresh_pixmap,
					img->gc, img->image, x, y, x, y,
					img->sub_img_w, img->sub_img_h);
			    handle_exposure(img, Draws,
		/* convert real pos. to relative pos.	*/
				RELATIVExy(img, x, y),
				img->sub_img_w, img->sub_img_h,
				img->height, img->update=True);
			}
#	endif
			break;
		case MENU1_SNAP:	SnapWindow(imgp, img, &event);
		}	/* end switch(popmenu) */
		HidingPanel(NoteWin);
		break;
	    default:	msg("uninstalled event %d\n", mevent);
	}	/* end of switch(mevent) */
	break;	/* end of ButtonPress */

	case ButtonRelease:
	    if (moved)	{
		switch(clickon)	{
		case NULL:
		case ELALINFO:	/* color() REG color=RGB%channels */
			Fresh_ImageScreen(img, img->cnvt, &new_colors);
			break;
		case CLIPINFO:	topv = ReadSlider(CSlider, 2);
				top = ReadSlider(CSlider, 1);
			break;
		case QUANINFO:
			if (!quant || ncolors > 64)	break;
#	ifndef	DIRECT
			new_colors = CreateCLT(graylevel, ncolors, DoAll, quant,
					1, &histinfo);
			SetColormap(&Monitor[0], graylevel,
					&new_colors, &newmap, quant);
			MapColor(img, img->cnvt, gf_size);
#	endif
#	ifdef	SCROLLBAR_on_CANVAS
		case MOVESCROLLBAR:
/*			XMoveWindow(img->dpy, img->win, -img->x0, -img->y0);
			XMoveResizeWindow(img->dpy,img->parts->scrollbar.h_swin,
				img->x0+img->parts->scrollbar.rx[0],
				img->y0+img->parts->scrollbar.ry[0],
				img->parts->scrollbar.rl[0],
				img->parts->scrollbar.rw[0]);
			XMoveResizeWindow(img->dpy,img->parts->scrollbar.v_swin,
				img->x0+img->parts->scrollbar.rx[1],
				img->y0+img->parts->scrollbar.ry[1],
				img->parts->scrollbar.rw[1],
				img->parts->scrollbar.rl[1]);	*/
#	else
			goto	DrawOthers;
		case MOVESCROLLBAR:	/* this is relative pos. (0, 0)	*/
			exposure_r(img, Draws, 0, 0,
				img->resize_w, img->resize_h, No);
#	endif
DrawOthers:		Draw_ImageScrollBars(img);
			if (img->sub_img)	DrawCrop(img, 0, 1);
		}/* end switch */
		moved = False;
		PressButtonState(RstButt)=PressButtonState(rfsButt)=RESETSTATE;
		if (PressButtonState(heqButt) && img->curve!=(ETAHistoEq|ETALinear)){
			PressButtonState(heqButt) = RESETSTATE;
			img->curve &= ~ETAHistoEq;
		}
		DrawPanel(); /* for reset pressbutton */
		HistoHandle(img, &histinfo, HISTO_BACKGROUND);
	    }
	    else if (img)	/* check for undo loading file and default */
		switch (clickon)	{
		case HMARKPOS:
		case IMAGEINFO:
		    if (wp==img->win)	/* for image only */
			XDefineCursor(img->dpy, wp, cursor);
		    if (wp != Epanel->win)
			ClearParameterWin(imginfo, y0);
	    }
	    clickon = 0;
	break;

	case KeyPress:	/* color() return *active = -1 */
	{
	char	*str, *XKeysymToString();
	int	len;
	bool	CTRL_Key;
		len = XLookupString(&event, lbuf, sizeof(lbuf), &keysym, &stat);
		lbuf[len] = 0;
		str = XKeysymToString(keysym);	/* useful? */
		CTRL_Key = event.xkey.state & ControlMask;
		if (len == 1) {
		    if (*str == 'q' || *str == 'Q') {
MEXIT:			if (!YesOrNo("Quit ?", 0))
				break;
			if (num_images) {
			    for (i=0; i<num_images; i++) {
				if (imgp[i]->update)
					SaveImage(imgp[i]);
#ifdef	C_TUNER
				DestroyColorImage(imgp[i]);
#else
				DestroyImage(imgp[i]);
#endif
			    }
			}
			if (vroot_dpy > 1 || vroot_dpy && cmn_hd.win &&
					YesOrNo("Keep Root Display ?", 0))
				RetainWindowProperty(&cmn_hd);
			if (newmap)
			    XInstallColormap(Dpy, XDefaultColormap(Dpy, Screen));
			DestroyPanel(NULL);
			exit(0);
		    }
		    if (tolower(*str) == 'c') {
#	ifdef	C_TUNER
			if (newmap)
			    XInstallColormap(Dpy, XDefaultColormap(Dpy, Screen));
			HidingPanel(Epanel);
			return	*active = -1;
#	else
			for (i=0; i<num_images; i++)
			    if (imgp[i]->win == event.xany.window)	{
				if (imgp[i]->update)
					SaveImage(imgp[i]);
				DestroyImage(imgp[i]);
				while (++i < num_images)
					imgp[i-1] = imgp[i];
				imgp[--num_images] = NULL;
				if (num_images)	{
					img = imginfo = imgp[num_images-1];
					goto	ReSume;/* several lines down */
				}
				else	img = NULL;
			    }/* end close image */
#endif
		    }
		    else if (tolower(*str) == 'h')
			Toggle_Info(Help_message_array1);
		}
	}break;

	case EnterNotify:
	    if ((i=WhichImage(event.xany.window, imgp, num_images)) >= 0) {
		imginfo = imgp[i];
		if (imginfo == img)	break;
ReSume0:	img->active = False;
		img = imginfo;
ReSume:		img->active = True;
		gf_size = GetImageBufNSize(img);
		Panel_Image(img, lkt);
#if	defined DIRECT & !defined C_TUNER
		MapColor(img, img->cnvt, gf_size, graylevel,
			MAX(VCTEntry, img->marray[img->fn].min));
#endif
	    }
	    if ((newmap || cca) && (xcrossing->mode != NotifyUngrab))	{
		XInstallColormap(Dpy, Monitor[0].cmap);
		XInstallColormap(Dpy1, Monitor[1].cmap);
	    }
#	ifdef	_DEBUG_
	    else	if (verbose)	mesg("enter notified\n");
#	endif
	break;

	case LeaveNotify:
	    if((newmap || cca) && (xcrossing->mode != NotifyGrab))
		XInstallColormap(Dpy, XDefaultColormap(Dpy, Screen));
#	ifdef	C_TUNER
		if (!clickon)	{
			*active = 1;
			return	num_images;
		}
#	endif
	break;

	case ColormapNotify:
#ifdef	_DEBUG_
	{	static	int	CN=0;
		msg("color change happened %d\r", CN++);
	}
#endif
	break;

	case MapNotify:
		if ((i=WhichImage(event.xany.window, imgp, num_images))>=0)	{
			imginfo = imgp[i];
			if (event.xmap.window == imginfo->win)	{
				if (imginfo->icon)
				XUnmapWindow(imginfo->dpy, imginfo->icon);
#ifndef	C_TUNER
				goto	ReSume0;
#endif
			}
			else if (event.xmap.window == imginfo->icon)
				XUnmapWindow(imginfo->dpy, imginfo->frame);
		}
		while (XCheckMaskEvent(Dpy, StructureNotifyMask, &event));
	break;

	case UnmapNotify:
		if ((i=WhichImage(event.xany.window, imgp, num_images))>=0){
			imginfo = imgp[i];
			if (event.xunmap.window == imginfo->win && imginfo->icon){
				if (imginfo->channels == 1)
					LoadIcon(imginfo);
				XMapWindow(imginfo->dpy, imginfo->icon);
			}
			else if (event.xunmap.window == imginfo->icon)	{
				XMapWindow(imginfo->dpy, imginfo->frame);
#ifndef	C_TUNER
				goto	ReSume0;
#endif
			}
		}
		while (XCheckMaskEvent(Dpy, StructureNotifyMask, &event));
	break;

	case ConfigureNotify:	/* any window moving, changing size, ... */
		/* we are only interested in window size change	*/
		ResizeWindow(img, &event);
	break;

	case MotionNotify:
	switch (clickon)	{
	    case ELALINFO:
		SetSBarPos(slider, event.xbutton.x, event.xbutton.y, sb);
		moved++;
		break;
	    case CLIPINFO:
		SetSBarPos(CSlider, event.xbutton.x, event.xbutton.y,sb);
		moved++;
		break;
	    case QUANINFO:
		SetSBarPos(QSlider, event.xbutton.x, event.xbutton.y,sb);
		moved++;
		break;
	    case MOVESCROLLBAR:
		SetScrollBar(img->parts, event.xbutton.x, event.xbutton.y, sb-1);
		moved++;
		break;
	    case HMARKPOS:
		DrawVMark(histinfo.his, event.xmotion.x, True);
	    case IMAGEINFO:
		ParameterWin(imginfo, &histinfo, event.xbutton.x, event.xbutton.y,
				y0, wp==histinfo.his->win);
		/* end of IMAGEINFO */
	}break;	/* end of clickon */
	default:    if (verbose)
		msg("%d Not a Panel Event\n", event.type);
	}
    }	/* end while */
}	/* End of Tuner */




void
SetImageEvent(img, event)
Image	*img;
XEvent	*event;
{
	img->event = event;
	img->color_dpy = cmn_hd.color_dpy;
	img->hist = NZALLOC(HistoSize*3, sizeof(*(img->hist)), "hist");
}

void
Panel_Image(img, lkt)
Image	*img;
LKT	*lkt;
{
	SetSBarRPos(LSlider, img->linearlow, 1);
	SetSBarRPos(LSlider, img->linearup, 2);
	SetSBarRPos(ESlider, img->bgrd, 1);
	ChangeSliderScale(ESlider, img->scale, ESlider==slider);
	ChangeSlider(img, &slider, ESlider, LSlider, heqButt, EButton);
/*	ButtonState(maxButt of hButton) = img->setscale;	*/
#ifdef	C_TUNER
	ButtonState(fButton) = img->fn;
	ChangePanelCmap(img);
#else
	SetShowFramePos(img, fButton, 0);
#endif
	DrawPanel();
	PanelMessage(FButton, img->name, 0, 0, 0);
	histinfo.histp = img->hist;	/* point to image histogram */
#ifndef	DIRECT
	if (histinfo.map || img->frames==1)
#endif
		new_curve(lkt, &histinfo, img->marray, img->curve, 0,
			img, img->width*img->height);
	HistoHandle(img, &histinfo, HISTO_BACKGROUND);
}

void
Fresh_ImageScreen(img, rp, new_colors)
Image	*img;
byte	*rp;
int	*new_colors;
{
int	map_flag = 1;
register byte	*mtmp = rp;	/* here, rp is for interpolation only */
register int	i;
	img->linearlow = ReadSlider(LSlider, 1);
	img->linearup = ReadSlider(LSlider, 2);
	if (img->curve == ETABackGD)
		img->bgrd = ReadSlider(ESlider, 1);
	if (img->curve == ETAForeGD)
		img->fgrd = ReadSlider(ESlider, 1);
#ifdef	C_TUNER
	i = img->fn % 3;
	cer[i].lower = img->linearlow;
	cer[i].upper = img->linearup;
	cer[i].bgrd = img->bgrd;
	cer[i].fgrd = img->fgrd;
#endif
	if (PressButtonState(RstButt) || ButtonState(Interpolate))
	{
	    ResetLKT(lkt, img);
	    if (ButtonState(Interpolate)){
		sprintf(lbuf, "Interpolate %s[f%d]", img->name, img->fn);
		DispInfo(Epanel, 0, lbuf, white1);
#ifdef	C_TUNER
		if (img->channels > 1)
		{ /*  memcpy(&cmn_hd, img, sizeof(cmn_hd));	*/
		    row *= img->channels;
		    interpolation(img->data, img->scan_data,
			x_regions, y_regions, IM, img, &histinfo);
		} else
#endif
		interpolation(rp, mtmp=(VType*)img->image->data,
		    x_regions, y_regions, IM, img, &histinfo);
		Find_min_max(img, histinfo.histp, mtmp, Yes, False);
	    }
	}
	else new_curve(lkt, &histinfo, img->marray + (img->color_dpy ?
		img->fn%img->channels : (img->frames > 1 ? img->fn : 0)),
		img->curve, /* 0? */ -1, img, img->width*img->height);
	if (img->color_dpy && img->fn == ButtonSync)	{
	    cer[2] = cer[1] = cer[0];
	    for (i=1; i<3; i++)
		memcpy(lkt+MaxColors*i, lkt, sizeof(*lkt)*MaxColors);
	}
#ifdef	DIRECT
#	ifdef	C_TUNER
	map_flag = img->dpy_depth < 24 &&
			((image_information*)img)->visual_class < TrueColor;
	if(cer[0].intp || ButtonState(Interpolate) || !map_flag)	{
		eta_scan_map(img, ButtonState(Interpolate));
		mtmp = PtrCAST img->image->data;
		if (!map_flag)	{
		register LKT	*lktr=lkt, *lktg=lktr+MaxColors, *lktb=lktg+MaxColors;
		  if (ImageByteOrder(img->dpy) != LSBFirst)
		    for (i=img->image->bytes_per_line*img->height>>2; i--;) {
			mtmp++;
			*mtmp++ = lktb[*mtmp];
			*mtmp++ = lktg[*mtmp];
			*mtmp++ = lktr[*mtmp];
		    }
		  else for (i=img->image->bytes_per_line*img->height>>2; i--;){
			*mtmp++ = lktr[*mtmp];
			*mtmp++ = lktg[*mtmp];
			*mtmp++ = lktb[*mtmp];
			mtmp++;
		    }
		}
		histogram(img->data, img->width*img->height, histinfo.histp, img);
		cer[0].intp = 0;
		Put_Image(img);
		ImageBackup(img);
	}
#	endif	C_TUNER
	if (map_flag)
	    MapColor(img, mtmp, gf_size, graylevel,
		MAX(VCTEntry, img->marray[img->fn].min));
#else
	if (quant && img->curve==ETALinear)	{
		*new_colors = CreateCLT(graylevel, ncolors, DoAll, quant, 1, &histinfo);
		SetColormap(&Monitor[0], graylevel, new_colors, &newmap, quant);
	}
	MapColor(img, mtmp, gf_size);
	Draw_ImageScrollBars(img);
	if (img->sub_img)
		DrawCrop(img, 0, 1);
#endif
}

Update_ImageData(img, lbuf)
Image	*img;
VType	*lbuf;
{
int	i;
	sprintf(lbuf, "Update %s[f%d]", img->name, img->fn);
	DispInfo(Epanel, 0, lbuf, white1);
	img->update = True;
	(*img->std_swif)(FI_DESC_ETA, img, lbuf, cer);
#ifdef	C_TUNER	/* update RGB image only */
	if (img->dpy_channels > 1)	{
	register int	j, ch, *lktp;
	byte	*pp[3], *dp[3];
	int	my,
		s_w = img->sub_img ? img->sub_img_w : img->width,
		s_h = img->sub_img ? img->sub_img_h : img->height,
		X0 = img->sub_img ? img->sub_img_x : 0,
		Y0 = img->sub_img ? img->sub_img_y : 0,
		icon_factor = get_iconsize(img, 0);
	    for (i=0; i<s_h; i++)	{
		my = Y0 + s_h - i /*- GETX_OFFSET*/;
		pp[0] = ORIG_RLE_ROW(img, my);
		pp[1] = pp[0] + img->width;
		pp[2] = pp[1] + img->width;
		dp[0] = SAVED_RLE_ROW(img, my);
		dp[1] = dp[0] + img->width;
		dp[2] = dp[1] + img->width;
		for (ch=0; ch<img->dpy_channels; ch++)	{
		register int	min = img->marray[ch].min;
		    lktp = lkt + ch*MaxColors;
		    for (j=X0; j<s_w+X0; j++)
			pp[ch][j] = lktp[pp[ch][j] - min];
		}
		MapRGB(0, 0, &img, pp, dp, my, img->width,
			my, icon_factor);
	    }
	    histogram(img->data, img->width*img->height, histinfo.histp, img);
	    Put_Image(img);	ImageBackup(img);
	}
	else
#endif
	if (ButtonState(Interpolate))
	    interpolation(img->cnvt, img->cnvt, x_regions, y_regions,
		IM, img, &histinfo);
	else	{
	register byte	*bp = img->cnvt;
	register int	min = img->mmm.min;
	    if (img->sub_img){
	    register int	j;
		bp += img->sub_img_y * img->width + img->sub_img_x;
		for (i=0; i<img->sub_img_h; i++){
		    for (j=0; j<img->sub_img_w; j++)
			bp[j] = lkt[bp[j] - min];
		    bp += img->width;
		}
	    }
	    else for (i=gf_size; i--;)
			bp[i] = lkt[bp[i] - min];
	}
	if (verbose)	sleep(2);
	ButtonState(EButton) = ETALinear;
	ButtonState(DButton) = RESETSTATE;
	DrawButton(DButton);
}

void
PasteImage(img, y0)
Image	*img;
int	*y0;
{
	TopWindow(img, No, img->sub_img=0);
	/* handle exposure generated from TopWindow 1st.	*/
	do	{
		Exposure_handler(img->event, img, NULL);
		XFlush(img->dpy);
	} while (ImageEvent(img, ExposureMask));
	memcpy(I_ED.copy, img, sizeof(Image));
	XDefineCursor(img->dpy, img->win, 0);
	I_ED.copy->sub_img_w = I_ED.w;
	I_ED.copy->sub_img_h = I_ED.h;
	while (!ImageEvent(img, ButtonReleaseMask))
	if (ImageEvent(img, PointerMotionMask))	{
	register int	yo;
		if (I_ED.copy->sub_img)
			DrawCrop(I_ED.copy, 0, 1);
#ifdef	SCROLLBAR_on_CANVAS
		I_ED.dstx = I_ED.copy->sub_img_x = img->event->xbutton.x;
		I_ED.dsty = I_ED.copy->sub_img_y = img->event->xbutton.y;
#else
		I_ED.dstx = I_ED.copy->sub_img_x = img->event->xbutton.x + img->x0;
		I_ED.dsty = I_ED.copy->sub_img_y = img->event->xbutton.y + img->y0;
#endif
		yo = SetParameterWin(img, img->event, img->font_h, 0);
		if (yo != *y0) {
			ClearParameterWin(img, *y0=yo);
			while (ImageEvent(img, ExposureMask))
				Exposure_handler(img->event, img, NULL);
		}
		ParameterWin(img, &histinfo, I_ED.dstx, I_ED.dsty, yo, 0);
		I_ED.copy->sub_img = DrawCrop(I_ED.copy, 0, 1);
	}
	if (I_ED.copy->sub_img)	{
	register int	i, Isize = img->width * img->height,
			w = MIN(I_ED.w, img->width - I_ED.dstx),
			h = MIN(I_ED.h, img->height - I_ED.dsty);
	byte* cpbuf = (img->data + (img->color_dpy || img->frames<2
			? 0 : img->fn * Isize) ),
		*src_area = (I_ED.src->channels < img->channels) ? (byte*)
			map_1_to_3(I_ED.copyarea, NULL, I_ED.src->in_cmap,
				I_ED.w, I_ED.h) : (byte*)I_ED.copyarea;
	    if (img && w>0 && h>0 && I_ED.dstx>=0 && I_ED.dsty>=0)	{
		{
		register byte	*srcp = src_area, *cp = cpbuf + I_ED.dstx +
				I_ED.dsty*img->width*img->channels;
		register int	factors=I_ED.src->channels/img->channels;
		if (factors)	factors--;	/* color quant will be here */
		i = MAX(MaxColors, I_ED.src->cmaplen);	/* tricky of buildmap */
		    if (I_ED.src != img && img->dpy_channels == 1 &&
			I_ED.src->dpy_channels == img->dpy_channels &&
				I_ED.src->color_dpy) {
			cmap_t	transf[MaxColors];
			register cmap_t	*cr=I_ED.src->in_cmap[0], *cg=cr+i, *cb=cg+i;
			for (i=I_ED.src->cmaplen; i--;)
				transf[i] = CloseColor_in_Map(img->in_cmap,
					img->cmaplen, cr[i], cg[i], cb[i], 384);
			for (i=I_ED.w * I_ED.h; i--;)
				srcp[i] = transf[srcp[i]];
		    }
		    for (i=h; i--; srcp += I_ED.w*factors)	{
			register int	chan = img->channels;
			    while (chan--)
				memcpy(cp, srcp, w),
				srcp += I_ED.w,
				cp += img->width;
		    }
		}
		if (src_area != (byte*)I_ED.copyarea)
			CFREE(src_area);
		I_ED.dst = img;
		img->sub_img_x = I_ED.dstx;
		img->sub_img_y = I_ED.dsty;
		img->sub_img_w = w;
		img->sub_img_h = h;
		img->update = True;
#	ifndef	C_TUNER
		XClearArea(img->dpy, img->win,
			I_ED.dstx, I_ED.dsty, I_ED.w, I_ED.h, 1);
#	endif
#	ifndef	DIRECT
		Find_min_max(img, histinfo.histp, cpbuf, Yes, True);
#	endif
#	ifndef	DIRECT
		ResetORange(img);
		ResetLKT(lkt, img);
		MapColor(img, cpbuf, Isize);
#	elif	C_TUNER
		{
		byte	*scan_line[3], *dp[3];
		register int	l, my, icon_factor=get_iconsize(img, 0);
		    for (l=0; l<h; l++)	{
			my = I_ED.dsty + l /* - GETX_OFFSET */;
			scan_line[0] = ORIG_RLE_ROW(img, my);
			dp[0] = SAVED_RLE_ROW(img, my);
			for (i=1; i<img->channels; i++)
				scan_line[i] = scan_line[i-1] + I_ED.dst->width,
				dp[i] = dp[i-1] + I_ED.dst->width;
			MapRGB(0, NULL, &img, scan_line, dp,
				my, I_ED.dst->width, my, icon_factor);
		    }
		}
		PlaceArea(img, I_ED.dstx, I_ED.dsty, I_ED.w, I_ED.h);
#	endif
	    }	/* end if */	else	DrawCrop(I_ED.copy, 0, 1);
	}	/* end section */
Find_min_max(img, histinfo.histp=img->hist, img->data, Yes, True);
}

PaintArea(img, rp, x, y, r, g, b, gs, err)
Image	*img;
byte	*rp;
{
byte	*scan[3];
register int	dy=img->sub_img_h, dx=img->sub_img_w, rt_x, rt_y;

#ifdef	SCROLLBAR_on_CANVAS
	img->sub_img_x = (x -= err);
	img->sub_img_y = (y -= err);
#else
	img->sub_img_x = (x += img->x0 - err);
	img->sub_img_y = (y += img->y0 - err);
#endif
	err <<= 1;


    if (x >= 0 && x < img->width-err && y >= 0 && y < img->height-err)
	for (rt_y=y+dy; rt_y-- > y;)	{
	byte	*p[3];
	err = rt_y*img->width*img->channels + x;
	p[0] = rp + err;
	p[1] = p[0] + img->width;
	p[2] = p[1] + img->width;
	    for (rt_x=dx; rt_x--;) {
		if (img->channels == 1)
			p[0][rt_x] = gs;
		else	{
			p[0][rt_x] = r;
			p[1][rt_x] = g;
			p[2][rt_x] = b;
		}
	    }
#	ifdef	C_TUNER
	    scan[0] = img->scan_data + err;
	    scan[1] = scan[0] + img->width;
	    scan[2] = scan[1] + img->width;
	    Map_Scanline(img, p, scan, x, rt_y, dx, get_iconsize(img, 0));
#	endif
	}
#	ifndef	C_TUNER
	img->sub_img++;
	MapColor(img, rp, img->width * img->height);
#	endif
	PlaceArea(img, x, y, dx, dy);
img->sub_img = 0;
}

SnapWindow(imgp, img, event)
Image*	*imgp, *img;
XEvent	*event;
{
extern	XColor	qcolor[MaxColors];
register XImage	*ip;
Image	*info=NULL;
Window	win;
XWindowAttributes	wa;
register int	i, j = 256;
#if	defined SNAP_WARNING & !defined	SINGLE_WINDOW_SNAP
	/* this may corrupt the other windows	*/
WaitOk(0, "Warning:\n\
If window is partially covered by VWM, move it to the middle to avoid crash", 0);
#endif
	XMaskEvent(Dpy, ButtonPressMask, event);
	win = event->xany.window;
	if ((i=WhichImage(win, imgp, num_images)) >= 0)
		info = imgp[i];
#ifdef	SINGLE_WINDOW_SNAP
	XMoveWindow(Dpy, win, 1, 1);	/* x, y of XGetWA() broken on X11R5 */
	TopWindow(img, Exposure_handler, No);	/* So, move win to Left-Top */
#endif						/* & make XGetImage() happy */
	XGetWindowAttributes(Dpy, win, &wa);	/* but cannot snap overlay! */
	XSync(img->dpy, No);
	while (ImageEvent(img, ExposureMask))	{
		if ((i=WhichImage(img->event->xany.window, imgp, num_images)) >= 0)
			img = imgp[i];
		Exposure_handler(img->event, img, Draws);
	}
	if (info)
		wa.width = info->resize_w,	wa.height = info->resize_h;
	ip  = XGetImage(Dpy, win, 0, 0, wa.width, wa.height, AllPlanes, ZPixmap);
	img = imgp[num_images++] = ZALLOC(1, sizeof(*img), "snap");
	init_img_info(img, Dpy, cmn_hd.color_dpy ? RLE : HIPS, cmn_hd.color_dpy);
	img->resize_w = img->width = wa.width;
	img->resize_h = img->height = wa.height;
	img->colormap = wa.colormap;
	img->dpy_channels = img->channels = 1;
	img->color_form = cmn_hd.color_dpy ? CFM_SCF : CFM_SGF;
	img->name = "snap window";
	CreateWindow(img, Monitor, 0, I_Mask | KeyPressMask,
		0, IconWindowHint | IconPositionHint);
	img->image = ip;
	img->dpy_depth = Monitor[0].dpy_depth;
	img->data = img->scan_data = img->img_buf = ip->data;
	SetImageEvent(img, event);
	XMapWindow(img->dpy, img->frame);
	if (info)	{
		if (!(i = info->entries))
			i = img->entries = ncolors;
#ifdef	SNAP_COPY_OK
		memcpy(&img->entries, &info->entries, sizeof(int)<<2);
#endif
	} else	{
		GetCloseColor(Dpy, wa.colormap, j, 0, 240, 240, 240);
		img->cmaplen = i = j;
	}
	if (verify_buffer_size(reg_cmap, i * 3, sizeof(cmap_t), "snap_map")) {
		img->in_cmap = reg_cmap;
		reg_cmap[1] = reg_cmap[0] + i;
		reg_cmap[2] = reg_cmap[1] + i;
	}
	while (i--)	{
		reg_cmap[0][i] = qcolor[i].red >> 8;
		reg_cmap[1][i] = qcolor[i].green>>8;
		reg_cmap[2][i] = qcolor[i].blue>>8;
	}
#ifdef	C_TUNER
	if (r_cmap)	CFREE(r_cmap);
	i = img->cmaplen;
	rle_dflt_hdr.cmap = (rle_map*)(r_cmap = (sht_cmap_t*)
		zalloc(i*(img->ncmap=3), sizeof(*r_cmap), "r_cmap"));
	rle_dflt_hdr.cmaplen = 8;
	while (i--)	{
		r_cmap[i] = qcolor[i].red;
		r_cmap[i + j] = qcolor[i].green;
		r_cmap[i + (j<<1)] = qcolor[i].blue;
	}
#else
	img->data = nzalloc(i=img->width, img->height, "");
	for (i *= img->height; i--;)
		img->data[i] = reg_cmap[0][ip->data[i]];
#endif
	Find_min_max(img, histinfo.histp, img->data, 1, True);
}
