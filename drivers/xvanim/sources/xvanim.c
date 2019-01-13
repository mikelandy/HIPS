/*
 * xvanim: XView-based HIPS-format image displayer/animator
 *
 * Copyright (c) 1989 Michael Landy
 *
 * Written by Mike Landy - 6/88
 * (initial display code by Pat Flynn, 1/88)
 * HIPS 2 - msl - 1/16/91
 * converted to XView - msl - 4/26/92
 *
 * By default, on an 8-or-more-bit console, this program uses 64 slots in
 * the colormap for gray-scale.  This 64-color mode lets the tool coexist
 * more peacefully with other color-piggish applications.  The low 2 bits
 * in the 8-bit gray value are ignored.  For most images, and given some
 * poor monitor performance, one can barely tell the difference.  If
 * multiple copies of the tool are running, they do not use the same 64
 * entries, so after the main colormap is filled up (after 3 invocations
 * or so), color flashing will occur.  This behavior may also be specified
 * using -r (e.g. in order to ignore a color map stored with the image
 * itself). If you want a full 256 color map with a ramp color lookup
 * table, specify -f.  If the image has its own colormap (in parameter
 * `cmap'), and if -r, -f, -g, -sg and -c are not specified, it will be
 * used.  Finally, the user may provide a color map with -c, -g or -sg.
 * The switch -c is used to input a color map from a file which is
 * formatted as follows:
 *
 * 		#-of-entries
 * 		r(0) g(0) b(0)
 * 		r(1) g(1) b(1)
 * 		      .
 * 		      .
 * 		      .
 * 		r(n-1) g(n-1) b(n-1)
 *
 * The image should not contain any pixel values greater than or equal to
 * n.  -g generates an inverse gamma correction lookup table where gammar
 * defaults to 2, gammag defaults to gammar and gammab defaults to
 * gammag.  -sg is like -g except that only the 6 most-significant bits of
 * each pixel are used, and only 64 colormap entries are required as for
 * -r.  For the full 256-color colormaps (-f, -g, -c, or a colormap stored
 * in a header when -r and -sg aren't specified), the default behavior is
 * to use the full colormap unaltered.  This results in the scrollbars and
 * window frame having the appearance governed by that colormap even when
 * the cursor is over the scrollbars (often causing them to disappear).
 * If the user specifies -C, then the control colors (needed for the
 * scrollbars and window frame) are altered to be identical to those
 * required for the scrollbars.  Any pixels in the displayed image with
 * those values are replaced with values which result in a displayed color
 * which is as close as possible to the desired color.  Afterward, if the
 * user changes the colormap using the window buttons, these substituted
 * pixels will continue to result in the same display colors as indicated
 * by the previously-made pixel substitution which occurred when the image
 * was initially loaded.  The choice of 64-color, 256-color, or
 * 256-color-with-saved-console-colors is specified in the command line,
 * and may not be changed once the tool is run.  In any of these modes the
 * program is able to display binary images as well.
 *
 * The window defaults to be just large enough to contain the entire
 * image.  The image may be resized by the user in the usual way.  The
 * user may also specify that the window be of a different size with the
 * -S flag.  This specifies the amount of the image that will fit in the
 * initially displayed canvas (and number of columns defaults to be the
 * same as the number of rows).  In order to avoid an XView bug, any time
 * the image is resized, the scrollbars are reset to be at the top-left
 * corner of the image.  Note:  the scrollbars are splittable, allowing
 * multiple views of the image.
 *
 * Compile this with:
 *
 * cc -DULORIG -O -I/usr/openwin/include -o xvanim xvanim.c \
 * 	-L/usr/openwin/lib -lhipsh -lhips -lm -lxview -lolgx -lX11
 *
 * Calling format:
 *
 *    cat HIPSfile | xvanim [-s n] [-f | -r | -c mapfile |
 * 		-g gammar gammag gammab | -sg gammar gammag gammab]
 * 		[-C] [-S viewrows [viewcols]]
 *
 * n is the height/width of the screen rectangle corresponding to
 * a single pixel in the input file (equivalent to the HIPS command
 * "enlarge -s n".  n defaults to 1, meaning
 * an MxM image file will appear in an MxM window on the screen
 * (not counting the title bar, etc.).
 *
 * On one-bit displays, input images which are not bit-packed will be
 * halftoned.
 */

#include <stdio.h>
#include <xview/xview.h>
#include <xview/canvas.h>
#include <xview/panel.h>
#include <xview/seln.h>
#include <xview/textsw.h>
#include <xview/cms.h>
#include <xview/svrimage.h>
#include <xview/screen.h>
#include <xview/icon.h>
#include <xview/scrollbar.h>
/* #include <pixrect/pixrect_hs.h> */
#include <X11/Xutil.h>
#include <sys/time.h>
#include <hipl_format.h>
#include <math.h>

static short HIPSicon_image[]={
/* Format_version=1, Width=64, Height=64, Depth=1, Valid_bits_per_item=16
 */
	0x0000,0x0000,0x0000,0x0000, 0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000, 0x0000,0x0000,0x0000,0x0000,
	0x0FFC,0x0000,0x0000,0x0000, 0x0F80,0x0000,0x0000,0x0000,
	0x0F80,0x0000,0x0000,0x0000, 0x0F00,0x0000,0x0000,0x0000,
	0x0E00,0x0000,0x0000,0x0000, 0x0800,0x0000,0x0000,0x0000,
	0x0820,0x0000,0x0000,0x0000, 0x0810,0x0000,0x0000,0x0000,
	0x0808,0x0000,0x0000,0x0000, 0x0804,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000, 0x0001,0x0000,0x0000,0x0000,
	0x0000,0x8000,0x0000,0x0000, 0x0000,0x4000,0x0000,0x0000,
	0x0000,0x2000,0x0000,0x0000, 0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0FFF,0xFFFF,0xFFFE, 0x0000,0x0800,0x0000,0x0002,
	0x0000,0x0800,0x0000,0x0002, 0x0000,0x0800,0x0000,0x0002,
	0x0000,0x0800,0x0000,0x0002, 0x0000,0x0800,0x0000,0x0002,
	0x0000,0x0800,0x0000,0x0002, 0x0000,0x0800,0x0000,0x0002,
	0x0000,0x0800,0x0000,0x0002, 0x0000,0x0800,0x0000,0x0002,
	0x0000,0x0800,0x0000,0x0002, 0x0000,0x0800,0x0000,0x0002,
	0x0000,0x0800,0x0000,0x0002, 0x0000,0x0800,0x0000,0x0002,
	0x0000,0x0800,0x0000,0x0002, 0x0000,0x0800,0x0000,0x0002,
	0x0000,0x0800,0x0000,0x0002, 0x0000,0x0800,0x0000,0x0002,
	0x0000,0x0A24,0x4210,0x931A, 0x0000,0x0A24,0x4218,0x931A,
	0x0000,0x0944,0x4514,0x92AA, 0x0000,0x0882,0x8512,0x92EA,
	0x0000,0x0942,0x8F92,0x924A, 0x0000,0x0A21,0x0891,0x924A,
	0x0000,0x0A21,0x0890,0x920A, 0x0000,0x0800,0x0000,0x0002,
	0x0000,0x0800,0x0000,0x0002, 0x0000,0x0800,0x0000,0x0002,
	0x0000,0x0800,0x0000,0x0002, 0x0000,0x0800,0x0000,0x0002,
	0x0000,0x0800,0x0000,0x0002, 0x0000,0x0800,0x0000,0x0002,
	0x0000,0x0800,0x0000,0x0002, 0x0000,0x0800,0x0000,0x0002,
	0x0000,0x0800,0x0000,0x0002, 0x0000,0x0800,0x0000,0x0002,
	0x0000,0x0800,0x0000,0x0002, 0x0000,0x0800,0x0000,0x0002,
	0x0000,0x0800,0x0000,0x0002, 0x0000,0x0800,0x0000,0x0002,
	0x0000,0x0800,0x0000,0x0002, 0x0000,0x0800,0x0000,0x0002,
	0x0000,0x0FFF,0xFFFF,0xFFFE, 0x0000,0x0000,0x0000,0x0000
};

#define ITIMER_NULL ((struct itimerval *) 0)

static void speed_proc(),frame_proc(),setrange(),newfile_proc();
static void nextframe(),displayit(),run_proc(),stop_proc(),step_proc();
static void rev_step_proc(),setdir(),runonce_proc(),done_proc();
static void header_proc(),quit_proc(),gammar_proc(),gammag_proc(),gammab_proc();
static void setcmapchoice(),loadcmap_proc(),resize_proc(),gang_proc();
static void sizereset_proc();
static Panel_setting text_proc();
static Notify_value catchclose();
struct itimerval frame_timer;
Icon icon;
Server_image icon_image;
Display *dpy;
Visual *visual;
Frame base_frame,header_frame,image_frame;
Canvas canvas;
Scrollbar scrollh,scrollv;
Xv_Screen xvscreen;
Window win;
Xv_window xvwin;
GC imagegc;
XColor colors[256],*conscolors,*savecolors;
Panel filepanel,controlpanel,header_panel1;
Panel_item speedslider,frameslider,loadcmapbutton,newfilebutton,headerbutton;
Panel_item runbutton,runoncebutton,stopbutton,stepbutton;
Panel_item currfile,message,fileitem,maptypeitem,gangeditem,cflagmsg;
Panel_item hdrfname,redslider,greenslider,blueslider,numgls;
Textsw header_panel2;
int pixelsize,blackpixel,whitepixel,nconscolors;
h_boolean ibinary,fflag,rflag,cflag,gflag,sgflag,Cflag;
h_boolean resized = FALSE;		/* True if user resizes the canvas */
int screen,ourpackedtype=PFLSBF;	/* Using bitmaps which assume LSBF */
h_boolean color2=FALSE,color64=TRUE,color256=FALSE,wantcolor256;
unsigned long *substtable;
byte bytelut[256],**imagemem;
int fps = 15;
int spf = 1;
int mode = 0;		/* 0=fast, 1=slow */
int resizecount = 0;	/* if above 1, user has resized */
h_boolean greydisplay,obinary;
h_boolean runonce = FALSE;
h_boolean image_shown = FALSE;
h_boolean cmap_loaded = FALSE;
int currfr = 0;
int dir = 0;		/* 0=forward, 1=backward, 2=palindromic-forward,
				3=palindromic-backward */
int cmaptype = 0;	/* 0=ramp, 1=gamma, 2=file, 2/3=hdr (3 if cmapvalid) */
h_boolean running = TRUE;
h_boolean iconic = FALSE;
struct header hd,hd2,hd3,hd4,*chd;
double gammar,gammag,gammab;
h_boolean cmapvalid = FALSE;	/* TRUE if file colormap has been read */
h_boolean hmapvalid = FALSE;	/* TRUE if header cmap has been found */
byte cred[256],cgreen[256],cblue[256],gred[256],ggreen[256],gblue[256];
byte sgred[64],sggreen[64],sgblue[64],*hred,*hgreen,*hblue;
char cmapname64[100],cmapname256[100];
int cnumcol,hnumcol;
h_boolean ganged = TRUE;		/* TRUE if gang the color sliders */

static Flag_Format flagfmt[] = {
    {"f",{"r","c","g","sg",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
    {"r",{"f","c","g","sg",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
    {"c",{"f","r","g","sg",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},
	{PTFILENAME,"","colormapfile"},LASTPARAMETER}},
    {"g",{"f","r","c","sg",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},
	{PTDOUBLE,"2","gammar"},
	{PTDOUBLE,"-1","gammag"},{PTDOUBLE,"-1","gammab"},LASTPARAMETER}},
    {"sg",{"f","r","c","g",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},
	{PTDOUBLE,"2","gammar"},
	{PTDOUBLE,"-1","gammag"},{PTDOUBLE,"-1","gammab"},LASTPARAMETER}},
    {"s",{LASTFLAG},0,{{PTINT,"1","sizepixel"},LASTPARAMETER}},
    {"C",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
    {"S",{LASTFLAG},1,{{PTINT,"-1","winrows"},{PTINT,"-1","wincols"},
	LASTPARAMETER}},
    LASTFLAG};
int types[] = {PFBYTE,LASTTYPE};

char str[100];
int nrows,ncols,nframes,fpanelhgt,fpanelwdth,cpanelhgt;
int viewrows,viewcols,smallhgt,bighgt;
h_boolean validheader;

main(argc,argv)

int argc;
char **argv;

{
	int i;
	char c;
	char *dummy;
	Filename filename,mapfile;
	FILE *fp;

	Progname = strsave(*argv);
	xv_init(XV_INIT_ARGC_PTR_ARGV,&argc,argv,NULL);
	parseargs(argc,argv,flagfmt,&fflag,&rflag,&cflag,&mapfile,&gflag,
		&gammar,&gammag,&gammab,&sgflag,&gammar,&gammag,&gammab,
		&pixelsize,&Cflag,&viewrows,&viewcols,FFONE,&filename);
	if (viewcols <= 0)
		viewcols = viewrows;
	if (gammar <= 0) {
		perr(HE_IMSG,"red gamma was nonpositive, reset to one");
		gammar = 1;
	}
	if (gammag < 0)
		gammag = gammar;
	if (gammab < 0)
		gammab = gammag;
	if (gammag == 0) {
		perr(HE_IMSG,"green gamma was zero, reset to one");
		gammag = 1;
	}
	if (gammab == 0) {
		perr(HE_IMSG,"blue gamma was zero, reset to one");
		gammab = 1;
	}
	if (gammag != gammab || gammag != gammar || gammab != gammar)
		ganged = FALSE;
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	if (findparam(&hd,"cmap") != NULLPAR) {
		getparam(&hd,"cmap",PFBYTE,&hnumcol,&hred);
		if (hnumcol % 3 == 0)
			hmapvalid = TRUE;
	}
	sprintf(cmapname64,"xvanim64_%d",getpid());
	sprintf(cmapname256,"xvanim256_%d",getpid());
	if (rflag || fflag)
		cmaptype = 0;
	else if (gflag || sgflag)
		cmaptype = 1;
	else if (cflag) {
		readcmap(mapfile,256,&cnumcol,cred,cgreen,cblue);
		cmapvalid = TRUE;
		cmaptype = 2;
	}
	else if (hmapvalid)
		cmaptype = 2;
	else
		cmaptype = 0;
	init_windows(argc,argv);
	gammalut();
	if ((fflag || cflag || gflag || sgflag) && !greydisplay) {
		perr(HE_IMSG,
		    "full color map for binary monitors is not meaningful");
		wantcolor256 = Cflag = fflag = cflag = gflag = sgflag = FALSE;
	}
	fileinit(fp,filename,TRUE);
	notify_interpose_event_func(image_frame,catchclose,NOTIFY_SAFE);
	settimer();
	hipserrprt = hipserrlev = HEL_SEVERE;
	window_main_loop(base_frame);
	return(0);
}

init_windows(argc,argv)

int argc;
char **argv;

{
	Colormap defcolormap;
	Cms hdr_cms;
	XColor cdef;
	Xv_cmsdata *cmsdata;
	int i,vismatch,rw,gw,bw,mw,marg;
	XVisualInfo vistemp,*vislist;
	h_boolean foundb,foundw;

	base_frame = (Frame) xv_create(NULL,
		FRAME,XV_LABEL,"xvanim",
		FRAME_INHERIT_COLORS,TRUE,
		NULL);
	dpy = (Display *) xv_get(base_frame,XV_DISPLAY);
	xvscreen = (Xv_Screen) xv_get(base_frame,XV_SCREEN);
	screen = DefaultScreen(dpy);
	imagegc = DefaultGC(dpy,screen);
	blackpixel = BlackPixel(dpy,screen);
	whitepixel = WhitePixel(dpy,screen);
	XSetForeground(dpy,imagegc,whitepixel);
	XSetBackground(dpy,imagegc,blackpixel);
	vistemp.screen = screen;
	vistemp.class = PseudoColor;
	vistemp.depth = 8;
	vislist = XGetVisualInfo(dpy,VisualClassMask | VisualDepthMask,
		&vistemp,&vismatch);
	if (vismatch == 0) {
		greydisplay = FALSE;
		visual = DefaultVisual(dpy,screen);
	}
	else {
		visual = vislist[0].visual;
		greydisplay = TRUE;
	}
	XFree((caddr_t)vislist);
	wantcolor256 = (fflag || gflag || cflag || (hmapvalid && !rflag))
		&& greydisplay;
	color2 = ibinary || !greydisplay;
	color256 = wantcolor256 && !color2;
	color64 = !(color2 || color256);
	if (Cflag && !color256)
	    perr(HE_IMSG,
		"-C is only relevant for 256-cell colormaps, ignored");
	icon_image = (Server_image) xv_create(NULL,SERVER_IMAGE,
		XV_WIDTH,64,
		XV_HEIGHT,64,
		SERVER_IMAGE_BITS,HIPSicon_image,NULL);
	icon = (Icon) xv_get(base_frame,FRAME_ICON);
	xv_set(icon,ICON_IMAGE,icon_image,NULL);
	xv_set(base_frame,FRAME_ICON,icon,NULL);
	image_frame = (Frame) xv_create(base_frame,FRAME,NULL);
	canvas = xv_create(image_frame,CANVAS,
		WIN_DYNAMIC_VISUAL,greydisplay,
		CANVAS_AUTO_SHRINK,FALSE,
		CANVAS_AUTO_EXPAND,FALSE,
		CANVAS_X_PAINT_WINDOW,TRUE,
		CANVAS_REPAINT_PROC,displayit,NULL);
	scrollv = (Scrollbar) xv_create(canvas,SCROLLBAR,
		SCROLLBAR_DIRECTION,SCROLLBAR_VERTICAL,
		SCROLLBAR_SPLITTABLE,TRUE,
		NULL);
	scrollh = (Scrollbar) xv_create(canvas,SCROLLBAR,
		SCROLLBAR_DIRECTION,SCROLLBAR_HORIZONTAL,
		SCROLLBAR_SPLITTABLE,TRUE,
		NULL);
	xv_set((Xv_window) xv_get(canvas,OPENWIN_NTH_VIEW,0),
		WIN_EVENT_PROC,resize_proc,
		WIN_CONSUME_EVENTS,WIN_RESIZE,NULL,
		NULL);
	xvwin = canvas_paint_window(canvas);
	win = xv_get(xvwin,XV_XID);
	if (greydisplay) {
		defcolormap = DefaultColormap(dpy,screen);
		cmsdata = (Xv_cmsdata *) xv_get(scrollv,WIN_CMS_DATA);
		nconscolors = cmsdata->size;
		conscolors = (XColor *) halloc(nconscolors+2,sizeof(XColor));
		savecolors = (XColor *) halloc(nconscolors+2,sizeof(XColor));
		foundb = foundw = FALSE;
		for (i=0;i<nconscolors;i++) {
			cdef.red = cmsdata->red[i]<<8;
			cdef.green = cmsdata->green[i]<<8;
			cdef.blue = cmsdata->blue[i]<<8;
			XAllocColor(dpy,defcolormap,&cdef);
			conscolors[i] = cdef;
			if (cdef.pixel == blackpixel)
				foundb = TRUE;
			if (cdef.pixel == whitepixel)
				foundw = TRUE;
		}
		if (!foundb) {
			conscolors[nconscolors].red =
				conscolors[nconscolors].green =
				conscolors[nconscolors].blue = 0;
			conscolors[nconscolors++].pixel = blackpixel;
		}
		if (!foundw) {
			conscolors[nconscolors].red =
				conscolors[nconscolors].green =
				conscolors[nconscolors].blue = 255 << 8;
			conscolors[nconscolors++].pixel = whitepixel;
		}

	}
	filepanel = xv_create(base_frame,PANEL,NULL);
	newfilebutton = (Panel_item) xv_create(filepanel,PANEL_BUTTON,
		PANEL_LABEL_STRING,"New File",
		PANEL_NOTIFY_PROC,newfile_proc,
		NULL);
	headerbutton = (Panel_item) xv_create(filepanel,PANEL_BUTTON,
		PANEL_LABEL_STRING,"Header",
		PANEL_NOTIFY_PROC,header_proc,
		NULL);
	marg = xv_get(headerbutton,XV_X) - xv_get(newfilebutton,XV_WIDTH);
	xv_create(filepanel,PANEL_BUTTON,
		PANEL_LABEL_STRING,"Reset Size",
		PANEL_NOTIFY_PROC,sizereset_proc,
		NULL);
	xv_create(filepanel,PANEL_BUTTON,
		PANEL_LABEL_STRING,"Quit",
		PANEL_NOTIFY_PROC,quit_proc,
		NULL);
	currfile = xv_create(filepanel,PANEL_MESSAGE,
		XV_X,0,
		XV_Y,xv_row(filepanel,1),
		PANEL_LABEL_STRING,"",
		NULL);
	fileitem = xv_create(filepanel,PANEL_TEXT,
		XV_X,0,
		XV_Y,xv_row(filepanel,2),
		PANEL_LABEL_STRING,"Filename:",
		PANEL_VALUE,"",
		PANEL_VALUE_DISPLAY_LENGTH,30,
		PANEL_NOTIFY_LEVEL,PANEL_SPECIFIED,
		PANEL_NOTIFY_PROC,text_proc,
		NULL);
	message = xv_create(filepanel,PANEL_MESSAGE,
		XV_X,0,
		XV_Y,xv_row(filepanel,3),
		PANEL_LABEL_STRING,"",
		NULL);
	if (greydisplay) {
		redslider = xv_create(filepanel,PANEL_SLIDER,
			XV_X,0,
			XV_Y,xv_row(filepanel,4),
			PANEL_LABEL_STRING,"Red Gamma*100",
			PANEL_VALUE,(int) (100*gammar),
			PANEL_MIN_VALUE,1,
			PANEL_MAX_VALUE,500,
			PANEL_NOTIFY_PROC,gammar_proc,
			NULL);
		greenslider = xv_create(filepanel,PANEL_SLIDER,
			XV_X,0,
			XV_Y,xv_row(filepanel,5),
			PANEL_LABEL_STRING,"Green Gamma*100",
			PANEL_VALUE,(int) (100*gammag),
			PANEL_MIN_VALUE,1,
			PANEL_MAX_VALUE,500,
			PANEL_NOTIFY_PROC,gammag_proc,
			NULL);
		blueslider = xv_create(filepanel,PANEL_SLIDER,
			XV_X,0,
			XV_Y,xv_row(filepanel,6),
			PANEL_LABEL_STRING,"Blue Gamma*100",
			PANEL_VALUE,(int) (100*gammab),
			PANEL_MIN_VALUE,1,
			PANEL_MAX_VALUE,500,
			PANEL_NOTIFY_PROC,gammab_proc,
			NULL);
		maptypeitem = xv_create(filepanel,PANEL_CHOICE_STACK,
			XV_X,0,
			XV_Y,xv_row(filepanel,7),
			PANEL_LABEL_STRING,"Cmap Type:",
			PANEL_NOTIFY_PROC,setcmapchoice,
			NULL);
		setcmapchoices();
		gangeditem = xv_create(filepanel,PANEL_CHECK_BOX,
			PANEL_LABEL_STRING,"Ganged Color Gammas:",
			PANEL_CHOICE_STRINGS,"",NULL,
			PANEL_VALUE,ganged ? 1 : 0,
			PANEL_NOTIFY_PROC,gang_proc,
			XV_SHOW,cmaptype==1,
			NULL);
		if (color256)
			loadcmapbutton = (Panel_item) xv_create(filepanel,
				PANEL_BUTTON,
				PANEL_LABEL_STRING,"Load Cmap",
				PANEL_NOTIFY_PROC,loadcmap_proc,
				NULL);
		numgls = xv_create(filepanel,PANEL_MESSAGE,
			XV_X,0,
			XV_Y,xv_row(filepanel,8),
			PANEL_LABEL_STRING,"GLs: 256",
			NULL);
		if (color256) {
			cflagmsg = xv_create(filepanel,PANEL_MESSAGE,
				PANEL_LABEL_STRING,Cflag ?
					"Console Colors Saved: Yes" :
					"Console Colors Saved: No",
				NULL);
		}
		rw = xv_get(redslider,XV_WIDTH);
		gw = xv_get(greenslider,XV_WIDTH);
		bw = xv_get(blueslider,XV_WIDTH);
		mw = (rw > gw) ? rw : gw;
		if (bw > mw)
			mw = bw;
		if (mw > rw)
			xv_set(redslider,XV_X,mw-rw,NULL);
		if (mw > gw)
			xv_set(greenslider,XV_X,mw-gw,NULL);
		if (mw > bw)
			xv_set(blueslider,XV_X,mw-bw,NULL);
	}
	window_fit(filepanel);
	smallhgt = xv_get(filepanel,XV_HEIGHT);
	controlpanel = xv_create(base_frame,PANEL,
		WIN_BELOW,filepanel,
		WIN_X,0,
		NULL);
	runbutton = (Panel_item) xv_create(controlpanel,PANEL_BUTTON,
		XV_X,0,
		XV_Y,0,
		PANEL_LABEL_STRING,"Run",
		PANEL_NOTIFY_PROC,run_proc,
		NULL);
	runoncebutton = (Panel_item) xv_create(controlpanel,PANEL_BUTTON,
		XV_X,xv_get(runbutton,XV_WIDTH)+marg,
		XV_Y,0,
		PANEL_LABEL_STRING,"Run once",
		PANEL_NOTIFY_PROC,runonce_proc,
		NULL);
	stopbutton = (Panel_item) xv_create(controlpanel,PANEL_BUTTON,
		XV_X,xv_get(runbutton,XV_WIDTH)+xv_get(runoncebutton,XV_WIDTH)
			+2*marg,
		XV_Y,0,
		PANEL_LABEL_STRING,"Stop",
		PANEL_NOTIFY_PROC,stop_proc,
		NULL);
	stepbutton = (Panel_item) xv_create(controlpanel,PANEL_BUTTON,
		XV_X,xv_get(runbutton,XV_WIDTH)+xv_get(runoncebutton,XV_WIDTH)
			+xv_get(stopbutton,XV_WIDTH)+3*marg,
		XV_Y,0,
		PANEL_LABEL_STRING,"Step",
		PANEL_NOTIFY_PROC,step_proc,
		NULL);
	frameslider = xv_create(controlpanel,PANEL_SLIDER,
		XV_X,0,
		XV_Y,xv_row(controlpanel,1),
		PANEL_LABEL_STRING,"Frame number",
		PANEL_NOTIFY_LEVEL,PANEL_ALL,
		PANEL_NOTIFY_PROC,frame_proc,
		NULL);
	xv_create(controlpanel,PANEL_CHOICE_STACK,
		XV_X,0,
		XV_Y,xv_row(controlpanel,2),
		PANEL_LABEL_STRING,"Direction:",
		PANEL_CHOICE_STRINGS,"forward","reverse","palindromic",0,
		PANEL_NOTIFY_PROC,setdir,
		NULL);
	speedslider = xv_create(controlpanel,PANEL_SLIDER,
		XV_X,0,
		XV_Y,xv_row(controlpanel,3),
		PANEL_LABEL_STRING,"Frames/sec",
		PANEL_VALUE,15,
		PANEL_MIN_VALUE,1,
		PANEL_MAX_VALUE,60,
		PANEL_NOTIFY_LEVEL,PANEL_DONE,
		PANEL_NOTIFY_PROC,speed_proc,
		NULL);
	xv_create(controlpanel,PANEL_CHOICE_STACK,
		XV_X,0,
		XV_Y,xv_row(controlpanel,4),
		PANEL_LABEL_STRING,"Range:",
		PANEL_CHOICE_STRINGS,"fast","slow",0,
		PANEL_NOTIFY_PROC,setrange,
		NULL);
	window_fit(controlpanel);
	bighgt = smallhgt + xv_get(controlpanel,XV_HEIGHT);
	if (xv_get(controlpanel,XV_WIDTH) > xv_get(filepanel,XV_WIDTH))
		xv_set(filepanel,XV_WIDTH,xv_get(controlpanel,XV_WIDTH),NULL);
	else
		xv_set(controlpanel,XV_WIDTH,xv_get(filepanel,XV_WIDTH),NULL);
	if (greydisplay) {
		i = xv_get(filepanel,XV_WIDTH);
		if (color256) {
			xv_set(cflagmsg,XV_X,i-xv_get(cflagmsg,XV_WIDTH),NULL);
			xv_set(loadcmapbutton,XV_X,
				i-xv_get(loadcmapbutton,XV_WIDTH),NULL);
		}
	}
	window_fit(base_frame);
	hdr_cms = (Cms) xv_create(NULL,CMS,
		CMS_NAME,"header",
		CMS_CONTROL_CMS,TRUE,
		CMS_SIZE,CMS_CONTROL_COLORS,
		NULL);
	header_frame = xv_create(base_frame,FRAME,XV_SHOW,FALSE,NULL);
	header_panel1 = xv_create(header_frame,PANEL,
		NULL);
	xv_create(header_panel1,PANEL_BUTTON,
		PANEL_LABEL_STRING,"Done",
		PANEL_NOTIFY_PROC,done_proc,
		NULL);
	hdrfname = xv_create(header_panel1,PANEL_MESSAGE,
		PANEL_LABEL_STRING,"",
		NULL);
	window_fit_height(header_panel1);
	header_panel2 = xv_create(header_frame,TEXTSW,
		WIN_BELOW,header_panel1,
		WIN_X,0,
		WIN_ROWS,20,
		WIN_COLUMNS,60,
		TEXTSW_BROWSING,TRUE,
		TEXTSW_LINE_BREAK_ACTION,TEXTSW_WRAP_AT_CHAR,
		TEXTSW_DISABLE_CD,TRUE,
		TEXTSW_DISABLE_LOAD,TRUE,
		WIN_CMS,hdr_cms,
		WIN_BACKGROUND_COLOR,CMS_CONTROL_COLORS-4,
		WIN_FOREGROUND_COLOR,CMS_CONTROL_COLORS-2,
		NULL);
	xv_set(header_panel1,XV_WIDTH,xv_get(header_panel2,XV_WIDTH),NULL);
	window_fit(header_frame);
}

fileinit(fp,fname,firsttime)

FILE *fp;
h_boolean firsttime;
Filename fname;

{
	char *glmsg;
	int method,herror,k,l;
	register int i,j,fr;
	struct hips_roi saveroi;
	char tmpname[100];

	running = TRUE;
	validheader = FALSE;
	msg("");
	if (!firsttime) {
		if (fread_hdr_a(fp,&hd,fname) == HIPS_ERROR) {
			fclose(fp);
			free_image(&hd);
			msg(hipserr);
			return(0);
		}
	}
	if (image_shown) {
		for (i=0;i<nframes;i++) {
			if (imagemem[i] != (byte *) NULL)
				free(imagemem[i]);
		}
		free(imagemem);
		image_shown = FALSE;
		xv_set(currfile,PANEL_LABEL_STRING,"Current File: <none>",NULL);
	}
	if (hd.numcolor > 1 || type_is_col3(&hd)) {
		if (firsttime)
			perr(HE_MSG,"can't handle color images");
		else {
			fclose(fp);
			free_image(&hd);
			msg("can't handle color images");
			xv_set(image_frame,XV_SHOW,FALSE,NULL);
			return(0);
		}
	}
	ibinary = (hd.pixel_format == PFMSBF || hd.pixel_format == PFLSBF);
	color2 = ibinary || !greydisplay;
	color256 = wantcolor256 && !color2;
	color64 = !(color2 || color256);
	hmapvalid = FALSE;
	if (findparam(&hd,"cmap") != NULLPAR) {
		hnumcol = 768;
		getparam(&hd,"cmap",PFBYTE,&hnumcol,&hred);
		if (hnumcol % 3) {
			if (firsttime)
			    perr(HE_MSG,"colormap length not a multiple of 3");
			else
			    msg("colormap length not a multiple of 3");
		}
		else {
			hnumcol /= 3;
			hgreen = hred + hnumcol;
			hblue = hred + 2*hnumcol;
			hmapvalid = TRUE;
		}
	}
	setcmapchoices();
	nrows=hd.orows; ncols=hd.ocols; nframes = hd.num_frame;
	hd2.imdealloc = hd3.imdealloc = hd4.imdealloc = FALSE;
	if ((imagemem = (byte **) malloc(nframes*sizeof(byte *))) == 0) {
		if (firsttime)
			perr(HE_MSG,"can't allocate image pointers");
		else {
			fclose(fp);
			free_image(&hd);
			msg("can't allocate image pointers");
			xv_set(image_frame,XV_SHOW,FALSE,NULL);
			return(0);
		}
	}
	for (i=0;i<nframes;i++)
		imagemem[i] = (byte *) NULL;
	getroi(&hd,&saveroi);
	clearroi(&hd);
	herror = 0;
	for (fr=0;fr<nframes;fr++) {
		if (hd.pixel_format == ourpackedtype && pixelsize == 1) {
			if (!hd.imdealloc)
				if (alloc_image(&hd) == HIPS_ERROR)
					{herror++;break;}
			if (fread_image(fp,&hd,fr,fname) == HIPS_ERROR)
				{herror++;break;}
			chd = &hd;
		}
		else {
			if (fr == 0)
				if ((method = fset_conversion(&hd,&hd2,types,
				    fname)) == HIPS_ERROR)
					{herror++;break;}
			if (!hd2.imdealloc)
				if (alloc_image(&hd2) == HIPS_ERROR)
					{herror++;break;}
			chd = &hd2;
			if (fread_imagec(fp,&hd,&hd2,method,fr,fname)
			    == HIPS_ERROR)
				{herror++;break;}
			if (color64)
				if (h_shift_b(chd,chd,-2) == HIPS_ERROR)
					{herror++;break;}
			if (pixelsize > 1) {
			    if (fr == 0) {
			        dup_headern(chd,&hd3);
				if (setsize(&hd3,(nrows * pixelsize),
					(ncols * pixelsize)) == HIPS_ERROR)
					    {herror++;break;}
			    }
			    if (!hd3.imdealloc)
				if (alloc_image(&hd3) == HIPS_ERROR)
					{herror++;break;}
			    if (h_enlarge_b(chd,&hd3,pixelsize,pixelsize)
				== HIPS_ERROR)
					{herror++;break;}
			    chd = &hd3;
			}
			if (color2) {
				if (!ibinary)
					if (h_halftone(chd,chd) == HIPS_ERROR)
						{herror++;break;}
				if (fr == 0) {
					dup_headern(chd,&hd4);
					if (setformat(&hd4,ourpackedtype) ==
					    HIPS_ERROR)
						{herror++;break;}
				}
				if (!hd4.imdealloc)
					if (alloc_image(&hd4) == HIPS_ERROR)
						{herror++;break;}
				if (ourpackedtype == PFMSBF) {
					if (h_btomp(chd,&hd4) == HIPS_ERROR)
						{herror++;break;}
				}
				else {
					if (h_btolp(chd,&hd4) == HIPS_ERROR)
						{herror++;break;}
				}
				chd = &hd4;
			}
		}
#ifdef LLORIG
		h_invert(chd,chd);
#endif
		imagemem[fr] = chd->image;
		chd->imdealloc = FALSE;
	}
	fclose(fp);
	free_image(&hd);
	free_image(&hd2);
	free_image(&hd3);
	free_image(&hd4);
	if (herror) {
		msg(hipserr);
		for (i=0;i<fr;i++) {
			if (imagemem[i] != (byte *) NULL)
				free(imagemem[i]);
		}
		free(imagemem);
		xv_set(image_frame,XV_SHOW,FALSE,NULL);
		return(0);
	}
	currfr = 0;
	validheader = image_shown = TRUE;
	sprintf(str,"Current File: %s",fname);
	xv_set(currfile,PANEL_LABEL_STRING,str,NULL);
	resizecount = 0;
	if (!resized) {
		i = viewrows;
		j = viewcols;
		if (viewrows <= 0) {
			i = pixelsize*ncols;
			j = pixelsize*nrows;
		}
		xv_set(canvas,
			CANVAS_WIDTH,ncols*pixelsize,
			CANVAS_HEIGHT,nrows*pixelsize,
			XV_WIDTH,i+xv_get(scrollv,XV_WIDTH)+2,
			XV_HEIGHT,j+xv_get(scrollh,XV_HEIGHT)+2,NULL);
	}
	else {
		xv_set(canvas,
			CANVAS_WIDTH,ncols*pixelsize,
			CANVAS_HEIGHT,nrows*pixelsize,
			NULL);
	}
	setcmap(TRUE);
	setroi2(&hd,&saveroi);	/* after any substitution done by setcmap */
	xv_set(frameslider,
		PANEL_VALUE,0,
		PANEL_MIN_VALUE,0,
		PANEL_MAX_VALUE,nframes-1,
		NULL);
	if (greydisplay) {
		if (color2)
			glmsg = "GLs: 2";
		else if (color256)
			glmsg = "GLs: 256";
		else
			glmsg = "GLs: 64";
		xv_set(numgls,PANEL_LABEL_STRING,glmsg,NULL);
	}
	xv_set(controlpanel,
		XV_SHOW,(nframes>1) ? TRUE : FALSE,
		NULL);
	xv_set(base_frame,XV_HEIGHT,(nframes>1) ? bighgt : smallhgt);
	if (nframes == 1) {
		running = FALSE;
		display1();
	}
	j = strlen(fname);
	k = 0;
	for (i=0;i<j;i++)
		if (fname[i]=='/')
			k = i+1;
	sprintf(tmpname,"xvanim: %s",fname+k);
	xv_set(image_frame,FRAME_LABEL,tmpname,NULL);
	xv_set(header_frame,FRAME_LABEL,tmpname,NULL);
	window_fit(image_frame);
	xv_set(scrollv,SCROLLBAR_VIEW_START,0,NULL);
	xv_set(scrollh,SCROLLBAR_VIEW_START,0,NULL);
	xv_set(image_frame,XV_SHOW,TRUE,NULL);
	settimer();
}

display1()

{
	displayit(canvas,xvwin,dpy,win,(Rectlist *) 0);
}

static void displayit(canv,pw,display,win,repaint_area)

Canvas canv;
Xv_window pw;
Display *display;
Window win;
Rectlist *repaint_area;

{
	XImage *image;
	Pixmap pxmap;

	if (image_shown) {
		if (color2) {
			pxmap = XCreatePixmapFromBitmapData(dpy,win,
			    (char *) imagemem[currfr],chd->ocols,chd->orows,
			    1,0,1);
			XCopyPlane(dpy,pxmap,win,imagegc,0,0,ncols*pixelsize,
				nrows*pixelsize,0,0,1);
			XFreePixmap(dpy,pxmap);
		}
		else {
			image = XCreateImage(dpy,visual,8,ZPixmap,
				0,(char *) imagemem[currfr],
				ncols*pixelsize,nrows*pixelsize,8,0);
			XPutImage(dpy,win,imagegc,image,0,0,0,0,
				ncols*pixelsize,nrows*pixelsize);
			image->data = (char *) 0;
			XDestroyImage(image);
		}
	}
}

static void speed_proc(item,value,event)

Panel_item item;
unsigned int value;
Event *event;

{
	msg("");
	if (mode)
		spf = value;
	else
		fps = value;
	settimer();
}

static void frame_proc(item,value,event)

Panel_item item;
unsigned int value;
Event *event;

{
	msg("");
	if (image_shown) {
		currfr = value;
		display1();
	}
}

static void run_proc(item,value,event)

Panel_item item;
unsigned int value;
Event *event;

{
	msg("");
	if (image_shown) {
		running = TRUE;
		runonce = FALSE;
		settimer();
	}
}

static void runonce_proc(item,value,event)

Panel_item item;
unsigned int value;
Event *event;

{
	msg("");
	if (image_shown) {
		running = TRUE;
		runonce = TRUE;
		if (dir == 1)
			currfr = nframes-1;
		else
			currfr = 0;
		if (dir == 3)
			dir = 2;
		display1(); setfrm();
		settimer();
	}
}

static void stop_proc(item,value,event)

Panel_item item;
unsigned int value;
Event *event;

{
	msg("");
	if (image_shown) {
		running = FALSE;
		settimer();
	}
}

static void step_proc(item,value,event)

Panel_item item;
unsigned int value;
Event *event;

{
	msg("");
	if (image_shown) {
		running = FALSE;
		if (dir == 0)
			currfr = (currfr + 1) % nframes;
		else if (dir == 1) {
			currfr--;
			if (currfr < 0)
				currfr = nframes-1;
		}
		else if (dir == 2) {
			currfr = currfr + 1;
			if (currfr == nframes) {
				dir = 3;
				currfr = nframes-2;
				if (currfr < 0)
					currfr = 0;
			}
		}
		else {
			currfr = currfr - 1;
			if (currfr < 0) {
				dir = 2;
				currfr = 1;
				if (nframes == 1)
					currfr = 0;
			}
		}
		display1(); setfrm();
		settimer();
	}
}

static void setdir(item,value,event)

Panel_item item;
unsigned int value;
Event *event;

{
	msg("");
	switch(value) {
	case 0: 		/* forward */
		dir = 0;
		break;
	case 1:			/* backward */
		dir = 1;
		break;
	case 2:			/* palindromic (initially forward) */
		dir = 2;
		break;
	default:
		hipserrprt = hipserrlev = HEL_ERROR;
		perr(HE_MSG,"invalid dirchoice value, help!!?!?");
	}
}

static void nextframe(client,itimer_type)

Notify_client client;
int itimer_type;

{
	if (image_shown) {
		if (runonce && ((dir==0 && currfr == nframes-1) ||
				(dir==1 && currfr == 0) ||
				(dir==3 && currfr == 0))) {
			running = FALSE;
			runonce = FALSE;
			settimer();
			return;
		}
		if (dir == 0)
			currfr = (currfr + 1) % nframes;
		else if (dir == 1) {
			currfr--;
			if (currfr < 0)
				currfr = nframes-1;
		}
		else if (dir == 2) {
			currfr = currfr + 1;
			if (currfr == nframes) {
				dir = 3;
				currfr = nframes-2;
				if (currfr < 0)
					currfr = 0;
			}
		}
		else {
			currfr = currfr - 1;
			if (currfr < 0) {
				dir = 2;
				currfr = 1;
				if (nframes == 1)
					currfr = 0;
			}
		}
		display1(); setfrm();
	}
	else
		settimer();	/* to clear the timer, just in case */
}

setfrm()

{
	xv_set(frameslider,PANEL_VALUE,currfr,NULL);
}

settimer()

{
	if (!image_shown)
		running = FALSE;
	if ((!running) || iconic) {
		frame_timer.it_interval.tv_usec = 0;
		frame_timer.it_interval.tv_sec = 0;
		frame_timer.it_value.tv_usec = 0;
		frame_timer.it_value.tv_sec = 0;
	}
	else if (mode) {
		frame_timer.it_interval.tv_usec = 0;
		frame_timer.it_interval.tv_sec = spf;
		frame_timer.it_value.tv_usec = 0;
		frame_timer.it_value.tv_sec = spf;
	}
	else if (fps == 1) {
		frame_timer.it_interval.tv_usec = 0;
		frame_timer.it_interval.tv_sec = 1;
		frame_timer.it_value.tv_usec = 0;
		frame_timer.it_value.tv_sec = 1;
	}
	else {
		frame_timer.it_interval.tv_usec = 1000000 / fps;
		frame_timer.it_interval.tv_sec = 0;
		frame_timer.it_value.tv_usec = 1000000 / fps;
		frame_timer.it_value.tv_sec = 0;
	}
	notify_set_itimer_func(base_frame,nextframe,ITIMER_REAL,
		&frame_timer,ITIMER_NULL);
}

static void setrange(item,value,event)

Panel_item item;
unsigned int value;
Event *event;

{
	msg("");
	switch (value) {
	case 0:			/* fast */
		mode = 0;
	xv_set(speedslider,
			PANEL_LABEL_STRING,"Frames/sec",
			PANEL_VALUE,fps,
			NULL);
		break;
	case 1:			/* slow */
		mode = 1;
	xv_set(speedslider,
			PANEL_LABEL_STRING,"Seconds/frame",
			PANEL_VALUE,spf,
			NULL);
		break;
	default:
		hipserrprt = hipserrlev = HEL_ERROR;
		perr(HE_MSG,"Invalid range value, help!!?\n");
	}
	settimer();
}

static void newfile_proc(item,value,event)

Panel_item item;
unsigned int value;
Event *event;

{
	FILE *fp;
	Filename fname;

	fname = (Filename) xv_get(fileitem,PANEL_VALUE);
	if (strlen(fname) == 0) {
		msg("null file name");
		return;
	}
	if ((fp = fopen(fname,"r")) == NULL) {
		msg("can't open file");
		return;
	}
	fileinit(fp,fname,FALSE);
}

static Panel_setting text_proc(item,value,event)

Panel_item item;
unsigned int value;
Event *event;

{
	FILE *fp;
	Filename fname;

	fname = (Filename) xv_get(fileitem,PANEL_VALUE);
	if (strlen(fname) == 0) {
		msg("null file name");
		return(PANEL_NONE);
	}
	if ((fp = fopen(fname,"r")) == NULL) {
		msg("can't open file");
		return(PANEL_NONE);
	}
	fileinit(fp,fname,FALSE);
	return(PANEL_NONE);
}

msg(s)

char *s;

{
	xv_set(message,PANEL_LABEL_STRING,s,NULL);
}

static void done_proc(item,value,event)

Panel_item item;
unsigned int value;
Event *event;

{
	xv_set(header_frame,XV_SHOW,FALSE,NULL);
	textsw_reset(header_panel2,0,0);
}

static void header_proc(item,value,event)

Panel_item item;
unsigned int value;
Event *event;

{
	static char invhdtext[] = "Current header contents invalid.\n\
Reload this HIPS file and try again.";
	char *hstring,*formatheader();

	if (image_shown) {
		textsw_reset(header_panel2,0,0);
		if (validheader)
			hstring = formatheader(&hd);
		else
			hstring = invhdtext;
		textsw_replace_bytes(header_panel2,0,TEXTSW_INFINITY,
			hstring,strlen(hstring));
		textsw_normalize_view(header_panel2,0);
		xv_set(hdrfname,PANEL_LABEL_STRING,
			xv_get(currfile,PANEL_LABEL_STRING),NULL);
		xv_set(header_frame,XV_SHOW,TRUE,NULL);
	}
}

static Notify_value catchclose(frame,event,arg,type)

Frame frame;
Event *event;
Notify_arg arg;
Notify_event_type type;

{
	Notify_value value;

	value = notify_next_event_func(frame,event,arg,type);
	iconic = (int) xv_get(frame,FRAME_CLOSED);
	settimer();
	return(value);
}

static void quit_proc(item,value,event)

Panel_item item;
unsigned int value;
Event *event;

{
	xv_set(header_frame,XV_SHOW,FALSE,NULL);
	textsw_reset(header_panel2,0,0);	/* avoid prompt for saving
						   text edits! */
	xv_destroy_safe(base_frame);
}

gammalut()

{
	int i;
	double gr,gg,gb;
	
	gr = 1./gammar;
	gg = 1./gammag;
	gb = 1./gammab;
	gred[0] = ggreen[0] = gblue[0] = 0;
	if (wantcolor256) {
		for (i=1;i<256;i++) {
			gred[i] = 255.*pow((double) i/255.,gr) + .5;
			ggreen[i] = 255.*pow((double) i/255.,gg) + .5;
			gblue[i] = 255.*pow((double) i/255.,gb) + .5;
		}
	}
	else {
		sgred[0] = sggreen[0] = sgblue[0] = 0;
		for (i=1;i<64;i++) {
			sgred[i] = 255.*pow((double) i/63.,gr) + .5;
			sggreen[i] = 255.*pow((double) i/63.,gg) + .5;
			sgblue[i] = 255.*pow((double) i/63.,gb) + .5;
		}
	}
}

static void gammar_proc(item,value,event)

Panel_item item;
unsigned int value;
Event *event;

{
	msg("");
	gammar = ((double) value)/100.;
	if (ganged) {
		gammag = gammab = gammar;
		xv_set(greenslider,PANEL_VALUE,value,NULL);
		xv_set(blueslider,PANEL_VALUE,value,NULL);
	}
	gammalut();
	if (cmaptype == 1)
		setcmap(FALSE);
}

static void gammag_proc(item,value,event)

Panel_item item;
unsigned int value;
Event *event;

{
	msg("");
	gammag = ((double) value)/100.;
	if (ganged) {
		gammar = gammab = gammag;
		xv_set(redslider,PANEL_VALUE,value,NULL);
		xv_set(blueslider,PANEL_VALUE,value,NULL);
	}
	gammalut();
	if (cmaptype == 1)
		setcmap(FALSE);
}

static void gammab_proc(item,value,event)

Panel_item item;
unsigned int value;
Event *event;

{
	msg("");
	gammab = ((double) value)/100.;
	if (ganged) {
		gammar = gammag = gammab;
		xv_set(redslider,PANEL_VALUE,value,NULL);
		xv_set(greenslider,PANEL_VALUE,value,NULL);
	}
	gammalut();
	if (cmaptype == 1)
		setcmap(FALSE);
}

static void gang_proc(item,value,event)

Panel_item item;
unsigned int value;
Event *event;

{
	ganged = (value == 1);
	gammag = gammab = gammar;
	xv_set(greenslider,PANEL_VALUE,xv_get(redslider,PANEL_VALUE),NULL);
	xv_set(blueslider,PANEL_VALUE,xv_get(redslider,PANEL_VALUE),NULL);
	gammalut();
	if (cmaptype == 1)
		setcmap(FALSE);
}

static void setcmapchoice(item,value,event)

Panel_item item;
unsigned int value;
Event *event;

{
	msg("");
	cmaptype = value;
	if (obinary)
		msg("color map ignored for binary image");
	else {
		setcmap(FALSE);
		xv_set(gangeditem,XV_SHOW,cmaptype == 1,NULL);
	}
}

static void loadcmap_proc(item,value,event)

Panel_item item;
unsigned int value;
Event *event;

{
	Filename fname;

	msg("");
	fname = (Filename) xv_get(fileitem,PANEL_VALUE);
	if (strlen(fname) == 0) {
		msg("null file name");
		return;
	}
	if (readcmap(fname,256,&cnumcol,cred,cgreen,cblue) == HIPS_ERROR) {
		msg(hipserr);
		cmapvalid = FALSE;
	}
	else {
		cmapvalid = TRUE;
		if (color256)
			cmaptype = 2;
	}
	xv_set(gangeditem,XV_SHOW,cmaptype == 1,NULL);
	setcmapchoices();
	if (obinary)
		msg("color map ignored for binary image");
	else
		setcmap(FALSE);
}

setcmap(firsttime)

h_boolean firsttime;

{
	float dist,newdist,rdiff,gdiff,bdiff;
	int i,j,k,r1,g1,b1;
	h_boolean dosubst;
	unsigned long *substtable;
	Cms cms;

	if (!greydisplay) {
		if (image_shown)
			display1();
		return;
	}
	if (color2) {
		if (wantcolor256 && Cflag) {
			for (i=0;i<256;i++) {
				colors[i].pixel = blackpixel;
				colors[i].red = colors[i].green =
					colors[i].blue = i << 8;
				colors[i].flags = DoRed | DoGreen | DoBlue;
			}
		}
		else if (wantcolor256) {
			for (i=0;i<256;i++) {
				if (i == blackpixel) {
					colors[i].pixel = blackpixel;
					colors[i].red = colors[i].green =
						colors[i].blue = 0;
					colors[i].flags =
						DoRed | DoGreen | DoBlue;
				}
				else if (i == whitepixel) {
					colors[i].pixel = blackpixel;
					colors[i].red = colors[i].green =
						colors[i].blue = 255 << 8;
					colors[i].flags =
						DoRed | DoGreen | DoBlue;
				}
				else {
					colors[i].pixel = blackpixel;
					colors[i].red = colors[i].green =
						colors[i].blue = i << 8;
					colors[i].flags =
						DoRed | DoGreen | DoBlue;
				}
			}
		}
		else {
			for (i=0;i<256;i++) {
				colors[i].pixel = blackpixel;
				colors[i].red = colors[i].green =
					colors[i].blue = i << 10;
				colors[i].flags = DoRed | DoGreen | DoBlue;
			}
		}
	}
	else if (wantcolor256 && cmaptype == 0) {
		for (i=0;i<256;i++) {
			colors[i].pixel = i;
			colors[i].red = colors[i].green = colors[i].blue = i<<8;
			colors[i].flags = DoRed | DoGreen | DoBlue;
		}
	}
	else if (cmapvalid && cmaptype == 2) {
		for (i=0;i<cnumcol;i++) {
			colors[i].pixel = i;
			colors[i].red = cred[i]<<8;
			colors[i].green = cgreen[i]<<8;
			colors[i].blue = cblue[i]<<8;
			colors[i].flags = DoRed | DoGreen | DoBlue;
		}
		for (i=cnumcol;i<256;i++)
			colors[i].flags = 0;
	}
	else if (wantcolor256 && cmaptype == 1) {
		colors[0].flags = DoRed | DoGreen | DoBlue;
		for (i=0;i<256;i++) {
			colors[i].pixel = i;
			colors[i].red = gred[i]<<8;
			colors[i].green = ggreen[i]<<8;
			colors[i].blue = gblue[i]<<8;
			colors[i].flags = DoRed | DoGreen | DoBlue;
		}
	}
	else if (cmaptype == 1) {
		for (i=0;i<64;i++) {
			colors[i].pixel = i;
			colors[i].red = sgred[i]<<8;
			colors[i].green = sggreen[i]<<8;
			colors[i].blue = sgblue[i]<<8;
			colors[i].flags = DoRed | DoGreen | DoBlue;
		}
	}
	else if (color64 && cmaptype == 0) {
		for (i=0;i<64;i++) {
			colors[i].pixel = i;
			colors[i].red = colors[i].green = colors[i].blue =
				i<<10;
			colors[i].flags = DoRed | DoGreen | DoBlue;
		}
	}
	else {					/* header colormap */
		for (i=0;i<hnumcol;i++) {
			colors[i].pixel = i;
			colors[i].red = hred[i]<<8;
			colors[i].green = hgreen[i]<<8;
			colors[i].blue = hblue[i]<<8;
			colors[i].flags = DoRed | DoGreen | DoBlue;
		}
		for (i=hnumcol;i<256;i++)
			colors[i].flags = 0;
	}
	dosubst = FALSE;
	if (wantcolor256 && Cflag) {
		for (i=0;i<nconscolors;i++) {
			savecolors[i] = colors[conscolors[i].pixel];
			colors[conscolors[i].pixel] = conscolors[i];
			colors[conscolors[i].pixel].flags =
				DoRed | DoGreen | DoBlue;
		}
		if (firsttime) {
			for (i=0;i<256;i++)
				bytelut[i] = i;
			for (i=0;i<nconscolors;i++) {
				dist = -1;
				r1 = savecolors[i].red;
				g1 = savecolors[i].green;
				b1 = savecolors[i].blue;
				for (j=0;j<256;j++) {
					rdiff = ((int) colors[j].red) - r1;
					gdiff = ((int) colors[j].green) - g1;
					bdiff = ((int) colors[j].blue) - b1;
					newdist = rdiff*rdiff + gdiff*gdiff +
						bdiff*bdiff;
					if ((newdist < dist) || (dist == -1)) {
						k = j;
						dist = newdist;
					}
				}
				if (savecolors[i].pixel != k) {
					dosubst = TRUE;
					bytelut[savecolors[i].pixel] = k;
				}
			}
		}
	}
	cms = (Cms) xv_find(xvscreen,CMS,
		CMS_NAME,wantcolor256 ? cmapname256 : cmapname64,
		XV_AUTO_CREATE,FALSE,NULL);
	if (cms == NULL)
		cms = (Cms) xv_create(screen,CMS,
			CMS_NAME,wantcolor256 ? cmapname256 : cmapname64,
			CMS_TYPE,XV_DYNAMIC_CMS,
			CMS_SIZE,wantcolor256 ? 256 : 64,
			CMS_X_COLORS,colors,NULL);
	else
		xv_set(cms,CMS_X_COLORS,colors,NULL);
	if (firsttime && wantcolor256 && Cflag) {
		xv_set(base_frame,WIN_CMS,cms,NULL);
		xv_set(header_frame,WIN_CMS,cms,NULL);
		xv_set(image_frame,WIN_CMS,cms,NULL);
	}
	if (firsttime)
		xv_set(canvas,WIN_CMS,cms,NULL);
	if ((!wantcolor256) && firsttime) {
		dosubst = TRUE;
		substtable = (unsigned long *) xv_get(cms,CMS_INDEX_TABLE);
		for (i=0;i<64;i++)
			bytelut[i] = substtable[i];
	}
	if ((!color2) && dosubst) {
		for (i=0;i<nframes;i++) {
			chd->image = chd->firstpix = imagemem[i];
			h_applylut(chd,chd,256,bytelut);
		}
	}
	if (image_shown)
		display1();
}

setcmapchoices()

{
	if (!greydisplay)
		return;
	if (cmapvalid && hmapvalid && color256)
		xv_set(maptypeitem,
			PANEL_CHOICE_STRINGS,"Ramp","Gamma","File","Header",0,
			NULL);
	else if (cmapvalid && color256) {
		xv_set(maptypeitem,
			PANEL_CHOICE_STRINGS,"Ramp","Gamma","File",0,
			NULL);
		if (cmaptype > 2)
			cmaptype = 0;
	}
	else if (hmapvalid && color256) {
		xv_set(maptypeitem,
			PANEL_CHOICE_STRINGS,"Ramp","Gamma","Header",0,
			NULL);
		if (cmaptype > 2)
			cmaptype = 0;
	}
	else {
		xv_set(maptypeitem,
			PANEL_CHOICE_STRINGS,"Ramp","Gamma",0,
			NULL);
		if (cmaptype > 1)
			cmaptype = 0;
	}
	xv_set(maptypeitem,PANEL_VALUE,cmaptype,NULL);
}

static void resize_proc(window,event,arg)

Xv_window window;
Event *event;
Notify_arg arg;

{
	if (event_action(event) == WIN_RESIZE) {
		resizecount++;
	}
	if (resizecount > 1)
		resized = TRUE;
}

static void sizereset_proc(item,value,event)

Panel_item item;
unsigned int value;
Event *event;

{
	resizecount = 0;
	resized = FALSE;
	xv_set(canvas,
		CANVAS_WIDTH,ncols*pixelsize,
		CANVAS_HEIGHT,nrows*pixelsize,
		XV_WIDTH,pixelsize*ncols+xv_get(scrollv,XV_WIDTH)+2,
		XV_HEIGHT,pixelsize*nrows+xv_get(scrollh,XV_HEIGHT)+2,NULL);
	window_fit(image_frame);
	xv_set(scrollv,SCROLLBAR_VIEW_START,0,NULL);
	xv_set(scrollh,SCROLLBAR_VIEW_START,0,NULL);
	display1();
}
