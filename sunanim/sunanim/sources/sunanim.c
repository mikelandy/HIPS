/*
 *  sunanim: Suntools-based HIPS-format image displayer/animator
 *
 * Copyright (c) 1989 Michael Landy
 *
 *  Written by Mike Landy - 6/88
 *  (initial display code by Pat Flynn, 1/88)
 *  HIPS 2 - msl - 1/16/91
 *
 * By default, on an 8-or-more-bit console, this program uses 64 slots in
 * the colormap for gray-scale.  If multiple copies of the tool are running,
 * they all use the same 64 entries.  This lets the tool coexist more peacefully
 * with other color-piggish applications.  The low 2 bits in the 8-bit gray
 * value are ignored.  For most images, and given some poor monitor
 * performance, one can barely tell the difference.  This behavior may also
 * be specified using -r (in order to ignore a color map stored with the image
 * itself). If you want a full 256 color map, specify -f.  If the image
 * has its own colormap (in parameter `cmap'), and if -r, -f, -g, -sg and -c
 * are not specified, it will be used.  Finally, the user may provide a color
 * map with -c, -g or -sg.  For -c, the file is formatted as follows:
 *
 *		#-of-entries
 *		r(0) g(0) b(0)
 *		r(1) g(1) b(1)
 *		      .
 *		      .
 *		      .
 *		r(n-1) g(n-1) b(n-1)
 *
 * The image should not contain any pixel values greater than or equal to n.
 * -g generates an inverse gamma correction lookup table where gammar defaults
 * to 2, gammag defaults to gammar and gammab defaults to gammag.  -sg is like
 * -g except that only the 6 most-significant bits of each pixel are used,
 * and only 64 colormap entries are required as for -r.
 *
 *  Compile this with
 *    cc -O -o sunanim sunanim.c -lsuntool
 *		-lsunwindow -lpixrect -lhipsh -lhips -lm
 *
 *  Calling format:
 *    cat HIPSfile | sunanim [-s n] [-f | -r | -c mapfile |
 *		-g gammar gammag gammab | -sg gammar gammag gammab]
 *
 *  n is the height/width of the screen rectangle corresponding to
 *    a single pixel in the input file (equivalent to the HIPS command
 *    "enlarge n".  n defaults to 1, meaning
 *    a MxM image file will appear in an MxM window on the screen
 *    (not counting the title bar, etc.)  A value of n>1 is supported
 *    only for greyscale (8 bit per pixel) images.
 *
 *  On one-bit displays, input images which are not bit-packed will be
 *  halftoned.
 *
 *   Sunanim also accepts a subset of the suntool standard arguments
 *     (-W[iIlLpPtT]).
 */

#include <stdio.h>
#include <suntool/sunview.h>
#include <suntool/canvas.h>
#include <suntool/panel.h>
#include <suntool/seln.h>
#include <suntool/textsw.h>
#include <pixrect/pixrect_hs.h>
#include <sys/time.h>
#include <hipl_format.h>
#include <math.h>

static short HIPSicon_image[]={
/* Format_version=1, Width=64, Height=64, Depth=1, Valid_bits_per_item=16
 */
	0xFFFF,0xFFFF,0xFFFF,0xFFFF,0x8000,0x0000,0x0000,0x0001,
	0x8000,0x0000,0x0000,0x0001,0x83FF,0xFFFF,0xFF00,0x0001,
	0x8200,0x0000,0x0100,0x0001,0x8200,0x0000,0x0100,0x0001,
	0x8200,0x0000,0x0100,0x0001,0x823F,0xFFFF,0xFFF0,0x0001,
	0x8220,0x0000,0x0010,0x0001,0x8220,0x0000,0x0010,0x0001,
	0x8220,0x0000,0x0010,0x0001,0x8223,0xFFFF,0xFFFF,0x0001,
	0x8222,0x0000,0x0001,0x0001,0x8222,0x0000,0x0001,0x0001,
	0x8222,0x0000,0x0001,0x0001,0x8222,0x3FFF,0xFFFF,0xF001,
	0x8222,0x2000,0x0000,0x1001,0x8222,0x2000,0x0000,0x1001,
	0x8222,0x2000,0x0000,0x1001,0x8222,0x2000,0x0000,0x1001,
	0x8222,0x23FF,0xFFFF,0xFF01,0x8222,0x2200,0x0000,0x0101,
	0x8222,0x2200,0x0000,0x0101,0x8222,0x2200,0x0000,0x0101,
	0x8222,0x223F,0xFFFF,0xFFF9,0x8222,0x2220,0x0000,0x0009,
	0x8222,0x2220,0x0000,0x0009,0x8222,0x2220,0x0000,0x0009,
	0x8222,0x2220,0x0000,0x0009,0x8222,0x2220,0x0000,0x0009,
	0x8222,0x2220,0x0000,0x0009,0x8222,0x2220,0x0000,0x0009,
	0x8222,0x2220,0x1C44,0x8809,0x8222,0x2220,0x2244,0xC809,
	0x8222,0x2220,0x2044,0xC809,0x8222,0x2220,0x1044,0xA809,
	0x8222,0x2220,0x0C44,0xA809,0x8222,0x2220,0x0244,0x9809,
	0x8222,0x2220,0x2244,0x9809,0x8222,0x2220,0x2244,0x8809,
	0x83E2,0x2220,0x1C38,0x8809,0x8022,0x2220,0x0000,0x0009,
	0x8022,0x2220,0x0000,0x0009,0x8022,0x2220,0x0000,0x0009,
	0x803E,0x2220,0x0000,0x0009,0x8002,0x2220,0x844F,0xA109,
	0x8002,0x2220,0x8642,0x2109,0x8002,0x2221,0x4642,0x3309,
	0x8003,0xE221,0x4542,0x3309,0x8000,0x2221,0x4542,0x2D09,
	0x8000,0x2222,0x24C2,0x2D09,0x8000,0x2223,0xE4C2,0x2109,
	0x8000,0x3E22,0x2442,0x2109,0x8000,0x0222,0x244F,0xA109,
	0x8000,0x0220,0x0000,0x0009,0x8000,0x0220,0x0000,0x0009,
	0x8000,0x03E0,0x0000,0x0009,0x8000,0x0020,0x0000,0x0009,
	0x8000,0x0020,0x0000,0x0009,0x8000,0x0020,0x0000,0x0009,
	0x8000,0x003F,0xFFFF,0xFFF9,0x8000,0x0000,0x0000,0x0001,
	0x8000,0x0000,0x0000,0x0001,0xFFFF,0xFFFF,0xFFFF,0xFFFF
};

DEFINE_ICON_FROM_IMAGE(icon,HIPSicon_image);
#define ITIMER_NULL ((struct itimerval *) 0)

static void speed_proc(),frame_proc(),setrange(),newfile_proc();
static void nextframe(),displayit(),run_proc(),stop_proc(),step_proc();
static void rev_step_proc(),setdir(),runonce_proc(),done_proc();
static void header_proc(),quit_proc(),gammar_proc(),gammag_proc(),gammab_proc();
static void setpixrange(),setcmapchoice(),loadcmap_proc();
static Panel_setting text_proc();
static Notify_value resett(),catchclose();
struct itimerval frame_timer;
Frame base_frame,header_frame;
Canvas canvas;
Panel filepanel,controlpanel,header_panel1;
Panel_item speedslider,frameslider;
Panel_item currfile,message,fileitem,maptypeitem;
Panel_item hdrfname,redslider,greenslider,blueslider,numgls,glusage;
Textsw header_panel2;
Pixwin *canpixwin;

int fps = 15;
int spf = 1;
int mode = 0;		/* 0=fast, 1=slow */
int pixelsize;
h_boolean greydisplay,obinary;
h_boolean runonce = FALSE;
h_boolean image_shown = FALSE;
h_boolean cmap_loaded = FALSE;
h_boolean fullsw;		/* TRUE = 256 pixel values, FALSE = 64 pixel values
				and shift (applies to greyscale only) */
h_boolean wantfullsw;	/* TRUE = use 256 pixel values on next file load */
int currfr = 0;
int dir = 0;		/* 0=forward, 1=backward, 2=palindromic-forward,
				3=palindromic-backward */
int cmaptype = 0;	/* 0=hdr/ramp, 1=ramp, 2=gamma, 3=file */
h_boolean running = TRUE;
h_boolean iconic = FALSE;
struct header hd;
h_boolean fflag,rflag,cflag,gflag,sgflag;
double gammar,gammag,gammab;
h_boolean cmapvalid = FALSE;	/* TRUE if file colormap has been read */
h_boolean hmapvalid;		/* TRUE if header cmap has been found */
byte cred[256],cgreen[256],cblue[256],gred[256],ggreen[256],gblue[256];
byte sgred[64],sggreen[64],sgblue[64],rred[256],rgreen[256],rblue[256];
byte srred[64],srgreen[64],srblue[64],*hred,*hgreen,*hblue;
byte bred[2]={0,255},bgreen[2]={0,255},bblue[2]={0,255};
char hmapname[50],gmapname[100],sgmapname[100],*cmapname;
int cnumcol,hnumcol;
 
int ourpackedtype =
#ifdef MSBFVERSION
	PFMSBF;
#else
	PFLSBF;
#endif

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
    {"Wi",{LASTFLAG},0,{LASTPARAMETER}},
    {"WI",{LASTFLAG},1,{{PTSTRING,"",""},LASTPARAMETER}},
    {"Wl",{LASTFLAG},1,{{PTSTRING,"",""},LASTPARAMETER}},
    {"WL",{LASTFLAG},1,{{PTSTRING,"",""},LASTPARAMETER}},
    {"Wt",{LASTFLAG},1,{{PTSTRING,"",""},LASTPARAMETER}},
    {"WT",{LASTFLAG},1,{{PTSTRING,"",""},LASTPARAMETER}},
    {"Wp",{LASTFLAG},2,{{PTSTRING,"",""},{PTSTRING,"",""},LASTPARAMETER}},
    {"WP",{LASTFLAG},2,{{PTSTRING,"",""},{PTSTRING,"",""},LASTPARAMETER}},
    LASTFLAG};
int types[] = {PFBYTE,LASTTYPE};

char str[100];
int nrows,ncols,nframes,fpanelhgt,fpanelwdth,cpanelhgt;
h_boolean validheader;
struct pixrect **mpr;

main(argc,argv)

int argc;
char *argv[];
{
	int i;
	char c;
	char *dummy;
	Filename filename,mapfile;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&fflag,&rflag,&cflag,&mapfile,&gflag,
		&gammar,&gammag,&gammab,&sgflag,&gammar,&gammag,&gammab,
		&pixelsize,&dummy,&dummy,&dummy,&dummy,&dummy,&dummy,&dummy,
		&dummy,&dummy,FFONE,&filename);
	if (gammag < 0)
		gammag = gammar;
	if (gammab < 0)
		gammab = gammag;
	wantfullsw = fullsw = fflag || gflag || cflag;
	if (rflag || fflag)
		cmaptype = 1;
	else if (gflag || sgflag)
		cmaptype = 2;
	else if (cflag) {
		readcmap(mapfile,256,&cnumcol,cred,cgreen,cblue);
		cmapvalid = TRUE;
		cmaptype = 3;
		cmapname = strsave(mapfile);
	}
	else
		cmaptype = 0;
	sprintf(hmapname,"sunv%d",getpid());
	gammalut();
	for (i=0;i<256;i++)
		rred[i] = rgreen[i] = rblue[i] = i;
	for (i=0;i<64;i++)
		srred[i] = srgreen[i] = srblue[i] = i*4;
	fp = hfopenr(filename);
	init_windows(argc,argv);
	if ((fflag || cflag || gflag || sgflag) && !greydisplay)
		perr(HE_IMSG,
		    "full color map for binary monitors is not meaningful");
	fileinit(fp,filename,TRUE);
	notify_interpose_event_func(base_frame,catchclose,NOTIFY_SAFE);
	notify_interpose_destroy_func(base_frame,resett);
	settimer();
	hipserrprt = hipserrlev = HEL_SEVERE;
	window_main_loop(base_frame);
	return(0);
}

init_windows(argc,argv)

int argc;
char **argv;

{
	base_frame = window_create(NULL,FRAME,FRAME_LABEL,"sunanim",
		FRAME_ARGS,argc,argv,WIN_ERROR_MSG,
		"can't create window, sunanim must be run under suntools",0);
	window_set(base_frame,FRAME_ICON,&icon,0);
	filepanel = window_create(base_frame,PANEL,
		WIN_COLUMNS,68,
		0);
	canvas = window_create(base_frame,CANVAS,
		WIN_RIGHT_OF,filepanel,
		WIN_Y,0,
		CANVAS_REPAINT_PROC,displayit,0);
	canpixwin=canvas_pixwin(canvas);
	greydisplay = (canpixwin -> pw_pixrect -> pr_depth) >= 8 ? TRUE : FALSE;
	window_set(filepanel,WIN_ROWS,greydisplay ? 8 : 4,0);
	currfile = panel_create_item(filepanel,PANEL_MESSAGE,
		PANEL_ITEM_X,0,
		PANEL_ITEM_Y,6,
		PANEL_LABEL_STRING,"",
		0);
	panel_create_item(filepanel,PANEL_BUTTON,
		PANEL_ITEM_X,ATTR_COL(60),
		PANEL_ITEM_Y,6,
		PANEL_LABEL_IMAGE,panel_button_image(filepanel,"Header",0,0),
		PANEL_NOTIFY_PROC,header_proc,
		0);
	fileitem = panel_create_item(filepanel,PANEL_TEXT,
		PANEL_ITEM_X,0,
		PANEL_ITEM_Y,32,
		PANEL_LABEL_STRING,"Filename:",
		PANEL_VALUE,"",
		PANEL_VALUE_DISPLAY_LENGTH,30,
		PANEL_NOTIFY_LEVEL,PANEL_SPECIFIED,
		PANEL_NOTIFY_PROC,text_proc,
		0);
	panel_create_item(filepanel,PANEL_BUTTON,
		PANEL_ITEM_X,ATTR_COL(58),
		PANEL_ITEM_Y,32,
		PANEL_LABEL_IMAGE,panel_button_image(filepanel,"New File",0,0),
		PANEL_NOTIFY_PROC,newfile_proc,
		0);
	message = panel_create_item(filepanel,PANEL_MESSAGE,
		PANEL_ITEM_X,0,
		PANEL_ITEM_Y,58,
		PANEL_LABEL_STRING,"",
		0);
	panel_create_item(filepanel,PANEL_BUTTON,
		PANEL_ITEM_X,ATTR_COL(62),
		PANEL_ITEM_Y,58,
		PANEL_LABEL_IMAGE,panel_button_image(filepanel,"Quit",0,0),
		PANEL_NOTIFY_PROC,quit_proc,
		0);
	if (greydisplay) {
		redslider = panel_create_item(filepanel,PANEL_SLIDER,
			PANEL_ITEM_X,0,
			PANEL_ITEM_Y,88,
			PANEL_LABEL_STRING,"  Red Gamma*100",
			PANEL_VALUE,200,
			PANEL_MIN_VALUE,1,
			PANEL_MAX_VALUE,500,
			PANEL_NOTIFY_LEVEL,PANEL_ALL,
			PANEL_NOTIFY_PROC,gammar_proc,
			0);
		greenslider = panel_create_item(filepanel,PANEL_SLIDER,
			PANEL_ITEM_X,0,
			PANEL_ITEM_Y,118,
			PANEL_LABEL_STRING,"Green Gamma*100",
			PANEL_VALUE,200,
			PANEL_MIN_VALUE,1,
			PANEL_MAX_VALUE,500,
			PANEL_NOTIFY_LEVEL,PANEL_ALL,
			PANEL_NOTIFY_PROC,gammag_proc,
			0);
		blueslider = panel_create_item(filepanel,PANEL_SLIDER,
			PANEL_ITEM_X,0,
			PANEL_ITEM_Y,148,
			PANEL_LABEL_STRING," Blue Gamma*100",
			PANEL_VALUE,200,
			PANEL_MIN_VALUE,1,
			PANEL_MAX_VALUE,500,
			PANEL_NOTIFY_LEVEL,PANEL_ALL,
			PANEL_NOTIFY_PROC,gammab_proc,
			0);
		glusage = panel_create_item(filepanel,PANEL_CYCLE,
			PANEL_ITEM_X,ATTR_COL(46),
			PANEL_ITEM_Y,88,
			PANEL_LABEL_STRING,"Greylevel Usage:",
			PANEL_CHOICE_STRINGS,"64","256",0,
			PANEL_VALUE,fullsw ? 1 : 0,
			PANEL_NOTIFY_PROC,setpixrange,
			0);
		if (cmapvalid) {
		    maptypeitem = panel_create_item(filepanel,PANEL_CYCLE,
			PANEL_ITEM_X,ATTR_COL(46),
			PANEL_ITEM_Y,118,
			PANEL_LABEL_STRING,"Cmap Type:",
			PANEL_CHOICE_STRINGS,"Hdr/Ramp","Ramp","Gamma","File",0,
			PANEL_VALUE,3,
			PANEL_NOTIFY_PROC,setcmapchoice,
			0);
		}
		else {
		    maptypeitem = panel_create_item(filepanel,PANEL_CYCLE,
			PANEL_ITEM_X,ATTR_COL(46),
			PANEL_ITEM_Y,118,
			PANEL_LABEL_STRING,"Cmap Type:",
			PANEL_CHOICE_STRINGS,"Hdr/Ramp","Ramp","Gamma",0,
			PANEL_VALUE,(rflag || fflag) ? 1 : 
				((gflag || sgflag) ? 2 : 0),
			PANEL_NOTIFY_PROC,setcmapchoice,
			0);
		}
		numgls = panel_create_item(filepanel,PANEL_MESSAGE,
			PANEL_ITEM_X,ATTR_COL(46),
			PANEL_ITEM_Y,148,
			PANEL_LABEL_STRING,"",
			0);
		panel_create_item(filepanel,PANEL_BUTTON,
			PANEL_ITEM_X,ATTR_COL(57),
			PANEL_ITEM_Y,148,
			PANEL_LABEL_IMAGE,
			    panel_button_image(filepanel,"Load Cmap",0,0),
			PANEL_NOTIFY_PROC,loadcmap_proc,
			0);
	}
	controlpanel = window_create(base_frame,PANEL,
		WIN_BELOW,filepanel,
		WIN_X,0,
		WIN_COLUMNS,62,
		0);
	panel_create_item(controlpanel,PANEL_BUTTON,
		PANEL_LABEL_IMAGE,panel_button_image(controlpanel,"Run",0,0),
		PANEL_NOTIFY_PROC,run_proc,
		0);
	panel_create_item(controlpanel,PANEL_BUTTON,
		PANEL_LABEL_IMAGE,
			panel_button_image(controlpanel,"Run once",0,0),
		PANEL_NOTIFY_PROC,runonce_proc,
		0);
	panel_create_item(controlpanel,PANEL_BUTTON,
		PANEL_LABEL_IMAGE,panel_button_image(controlpanel,"Stop",0,0),
		PANEL_NOTIFY_PROC,stop_proc,
		0);
	panel_create_item(controlpanel,PANEL_BUTTON,
		PANEL_LABEL_IMAGE,panel_button_image(controlpanel,"Step",0,0),
		PANEL_NOTIFY_PROC,step_proc,
		0);
	frameslider = panel_create_item(controlpanel,PANEL_SLIDER,
		PANEL_ITEM_X,0,
		PANEL_ITEM_Y,42,
		PANEL_LABEL_STRING,"Frame number",
		PANEL_NOTIFY_LEVEL,PANEL_ALL,
		PANEL_NOTIFY_PROC,frame_proc,
		0);
	panel_create_item(controlpanel,PANEL_CYCLE,
		PANEL_ITEM_X,0,
		PANEL_ITEM_Y,62,
		PANEL_LABEL_STRING,"Direction:",
		PANEL_CHOICE_STRINGS,"forward","reverse","palindromic",0,
		PANEL_NOTIFY_PROC,setdir,
		0);
	speedslider = panel_create_item(controlpanel,PANEL_SLIDER,
		PANEL_ITEM_X,0,
		PANEL_ITEM_Y,92,
		PANEL_LABEL_STRING,"Frames/sec",
		PANEL_VALUE,15,
		PANEL_MIN_VALUE,1,
		PANEL_MAX_VALUE,60,
		PANEL_NOTIFY_LEVEL,PANEL_DONE,
		PANEL_NOTIFY_PROC,speed_proc,
		0);
	panel_create_item(controlpanel,PANEL_CYCLE,
		PANEL_ITEM_X,0,
		PANEL_ITEM_Y,112,
		PANEL_LABEL_STRING,"Range:",
		PANEL_CHOICE_STRINGS,"fast","slow",0,
		PANEL_NOTIFY_PROC,setrange,
		0);
	window_fit_height(controlpanel);
	fpanelhgt = (int) window_get(filepanel,WIN_HEIGHT);
	fpanelwdth = (int) window_get(filepanel,WIN_WIDTH);
	cpanelhgt = (int) window_get(controlpanel,WIN_HEIGHT);
	header_frame = window_create(base_frame,FRAME,
		WIN_SHOW,FALSE,
		0);
	header_panel1 = window_create(header_frame,PANEL,
		0);
	panel_create_item(header_panel1,PANEL_BUTTON,
		PANEL_LABEL_IMAGE,panel_button_image(header_panel1,"Done",0,0),
		PANEL_NOTIFY_PROC,done_proc,
		0);
	hdrfname = panel_create_item(header_panel1,PANEL_MESSAGE,
		PANEL_LABEL_STRING,"",
		0);
	window_fit_height(header_panel1);
	header_panel2 = window_create(header_frame,TEXTSW,
		WIN_BELOW,header_panel1,
		WIN_X,0,
		WIN_ROWS,20,
		WIN_COLUMNS,60,
		TEXTSW_BROWSING,TRUE,
		TEXTSW_LINE_BREAK_ACTION,TEXTSW_WRAP_AT_CHAR,
		TEXTSW_DISABLE_CD,TRUE,
		TEXTSW_DISABLE_LOAD,TRUE,
		0);
	window_fit(header_frame);
}

fileinit(fp,fname,firsttime)

FILE *fp;
h_boolean firsttime;
Filename fname;

{
	char *glmsg;
	h_boolean ibinary;
	int method,doshift,herror,k,l;
	register int i,j,fr;
	struct header hd2,hd3,hd4,hd5,*chd;
	struct hips_roi saveroi;

	running = TRUE;
	validheader = FALSE;
	msg("");
	if (fread_hdr_a(fp,&hd,fname) == HIPS_ERROR) {
		if (firsttime)
			perr(HE_MSG,hipserr);
		else {
			fclose(fp);
			free_image(&hd);
			msg(hipserr);
			return(0);
		}
	}
	if (hd.numcolor > 1 || type_is_col3(&hd)) {
		if (firsttime)
			perr(HE_MSG,"can't handle color images");
		else {
			fclose(fp);
			free_image(&hd);
			msg("can't handle color images");
			return(0);
		}
	}
	fullsw = wantfullsw;
	if (firsttime && findparam(&hd,"cmap") != NULLPAR &&
	    (!(fflag || gflag || cflag))) {
		wantfullsw = fullsw = TRUE;
		panel_set(glusage,PANEL_VALUE,1,0);
	}
	if (greydisplay) {
		if (fullsw && cmapvalid) {
		    panel_set(maptypeitem,
			PANEL_CHOICE_STRINGS,"Hdr/Ramp","Ramp","Gamma","File",0,
			0);
		}
		else {
		    panel_set(maptypeitem,
			PANEL_CHOICE_STRINGS,"Hdr/Ramp","Ramp","Gamma",0,
			0);
		    if (cmaptype == 3) {
			cmaptype = 0;
			panel_set(maptypeitem,PANEL_VALUE,0,0);
		    }
		}
	}
	if (image_shown) {
		for (i=0;i<nframes;i++)
			pr_close(mpr[i]);
		free(mpr);
		image_shown = FALSE;
		panel_set(currfile,
			PANEL_LABEL_STRING,"Current File: <none>",
			0);
	}
	ibinary = (hd.pixel_format == PFMSBF || hd.pixel_format == PFLSBF)
		? TRUE : FALSE;
	nrows=hd.orows; ncols=hd.ocols; nframes = hd.num_frame;
	obinary = (ibinary || (! greydisplay)) ? TRUE : FALSE;
	if ((fflag || cflag || gflag || sgflag) && ibinary && firsttime)
		perr(HE_IMSG,
			"full color map for binary images is not meaningful");
	doshift = (!(fullsw || obinary));
	hd2.imdealloc = hd3.imdealloc = hd4.imdealloc = hd5.imdealloc = FALSE;
	if ((mpr = (struct pixrect **)
		malloc(nframes*sizeof(struct pixrect *))) == 0) {
			if (firsttime)
				perr(HE_MSG,"can't allocate mpr");
			else {
				fclose(fp);
				free_image(&hd);
				msg("can't allocate mpr");
				return(0);
			}
	}
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
			if (doshift)
				if (h_shift_b(chd,chd,-2) == HIPS_ERROR)
					{herror++;break;}
			if (pixelsize > 1) {
			    if (fr == 0) {
			        dup_headern(chd,&hd3);
				if ((!obinary) && ((ncols * pixelsize) & 01)) {
				    if (setsize(&hd3,(nrows * pixelsize),
					(ncols * pixelsize) + 1)
					== HIPS_ERROR)
					    {herror++;break;}
				    if (setroi(&hd3,0,0,hd3.orows,hd3.ocols - 1)
					== HIPS_ERROR)
					    {herror++;break;}
				}
				else
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
			if (obinary) {
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
#ifdef MSBFVERSION
				if (h_btomp(chd,&hd4) == HIPS_ERROR)
					{herror++;break;}
#else
				if (h_btolp(chd,&hd4) == HIPS_ERROR)
					{herror++;break;}
#endif
				chd = &hd4;
			}
		}
		if (obinary) {
			if ((((chd->ocols)+7)/8) & 01) {
				if (fr == 0) {
				    dup_headern(chd,&hd5);
				    if (setsize(&hd5,hd5.orows,hd5.ocols+8)
					== HIPS_ERROR)
						{herror++;break;}
				    if (setroi(&hd5,0,0,hd5.orows,hd5.ocols-8)
					== HIPS_ERROR)
						{herror++;break;}
				}
				if (!hd5.imdealloc)
					if (alloc_image(&hd5) == HIPS_ERROR)
						{herror++;break;}
#ifdef MSBFVERSION
				if (h_copy_mp(chd,&hd5) == HIPS_ERROR)
					{herror++;break;}
#else
				if (h_copy_lp(chd,&hd5) == HIPS_ERROR)
					{herror++;break;}
#endif
				chd = &hd5;
			}
		}
		else {
			if ((chd->ocols) & 01) {
				if (fr == 0) {
				    dup_headern(chd,&hd5);
				    if (setsize(&hd5,hd5.orows,hd5.ocols+1)
					== HIPS_ERROR)
						{herror++;break;}
				    if (setroi(&hd5,0,0,hd5.orows,hd5.ocols-1)
					== HIPS_ERROR)
						{herror++;break;}
				}
				if (!hd5.imdealloc)
					if (alloc_image(&hd5) == HIPS_ERROR)
						{herror++;break;}
				if (h_copy_b(chd,&hd5) == HIPS_ERROR)
					{herror++;break;}
				chd = &hd5;
			}
		}
		mpr[fr] = mem_point(ncols*pixelsize,nrows*pixelsize,obinary?1:8,
			(short *) chd->image);
		chd->imdealloc = FALSE;
	}
	fclose(fp);
	free_image(&hd);
	free_image(&hd2);
	free_image(&hd3);
	free_image(&hd4);
	free_image(&hd5);
	if (herror) {
		msg(hipserr);
		for (i=0;i<fr;i++)
			pr_close(mpr[i]);
		free(mpr);
		return(0);
	}
	setroi2(&hd,&saveroi);
	currfr = 0;
	validheader = image_shown = TRUE;
	sprintf(str,"Current File: %s",fname);
	panel_set(currfile,PANEL_LABEL_STRING,str,0);
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
	setcmap();
	panel_set(frameslider,
		PANEL_VALUE,0,
		PANEL_MIN_VALUE,0,
		PANEL_MAX_VALUE,nframes-1,
		0);
	if (greydisplay) {
		if (obinary)
			glmsg = "GLs: 2";
		else if (fullsw)
			glmsg = "GLs: 256";
		else
			glmsg = "GLs: 64";
	}
	panel_set(numgls,PANEL_LABEL_STRING,glmsg,0);
	window_set(canvas,
		CANVAS_WIDTH,ncols*pixelsize,
		CANVAS_HEIGHT,nrows*pixelsize,
		WIN_WIDTH,pixelsize*ncols,
		WIN_HEIGHT,pixelsize*nrows,
		CANVAS_DEPTH,obinary ? 1 : 8,
		0);
	window_set(controlpanel,
		WIN_SHOW,(nframes>1) ? TRUE : FALSE,
		0);
	if (nframes > 1)
		i = fpanelhgt + cpanelhgt + 28;
	else
		i = fpanelhgt + 23;
	j = (int) window_get(canvas,WIN_HEIGHT) + 21;
	k = fpanelwdth;
	l = (int) window_get(canvas,WIN_WIDTH);
	window_set(filepanel,
		WIN_COLUMNS,68,
		WIN_ROWS,greydisplay ? 8 : 4,
		0);
	window_set(base_frame,
		WIN_HEIGHT,((i>j) ? i : j),
		WIN_WIDTH,k+l+15,
		0);
	if (nframes == 1) {
		running = FALSE;
		nextframe();
	}
	settimer();
}

static void displayit(canv,pw,repaint_area)

Canvas canv;
Pixwin *pw;
Rectlist *repaint_area;

{
	if (image_shown)
		pw_rop(pw,0,0,ncols*pixelsize,nrows*pixelsize,PIX_SRC,
			mpr[currfr],0,0);
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
		displayit(canvas,canpixwin,(Rectlist *) 0);
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
		displayit(canvas,canpixwin,(Rectlist *) 0); setfrm();
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
			}
		}
		else {
			currfr = currfr - 1;
			if (currfr < 0) {
				dir = 2;
				currfr = 1;
			}
		}
		displayit(canvas,canpixwin,(Rectlist *) 0); setfrm();
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
			}
		}
		else {
			currfr = currfr - 1;
			if (currfr < 0) {
				dir = 2;
				currfr = 1;
			}
		}
		displayit(canvas,canpixwin,(Rectlist *) 0); setfrm();
	}
	else
		settimer();	/* to clear the timer, just in case */
}

setfrm()

{
	panel_set_value(frameslider,currfr);
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
		panel_set(speedslider,
			PANEL_LABEL_STRING,"Frames/sec",
			PANEL_VALUE,fps,
			0);
		break;
	case 1:			/* slow */
		mode = 1;
		panel_set(speedslider,
			PANEL_LABEL_STRING,"Seconds/frame",
			PANEL_VALUE,spf,
			0);
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

	fname = (Filename) panel_get_value(fileitem);
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

	fname = (Filename) panel_get_value(fileitem);
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
	panel_set(message,PANEL_LABEL_STRING,s,0);
}

static void done_proc(item,value,event)

Panel_item item;
unsigned int value;
Event *event;

{
	window_set(header_frame,WIN_SHOW,FALSE,0);
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
		panel_set(hdrfname,PANEL_LABEL_STRING,
			panel_get(currfile,PANEL_LABEL_STRING),0);
		window_set(header_frame,WIN_SHOW,TRUE,0);
	}
}

static Notify_value resett(frame,status)

Frame frame;
Destroy_status status;

{
	window_set(frame,FRAME_NO_CONFIRM,FALSE,0);
	window_set(header_frame,WIN_SHOW,FALSE,0);
	textsw_reset(header_panel2,0,0);
	return(notify_next_destroy_func(frame,status));
}

static Notify_value catchclose(frame,event,arg,type)

Frame frame;
Event *event;
Notify_arg arg;
Notify_event_type type;

{
	Notify_value value;

	value = notify_next_event_func(frame,event,arg,type);
	iconic = (int) window_get(frame,FRAME_CLOSED);
	settimer();
	return(value);
}

static void quit_proc(item,value,event)

Panel_item item;
unsigned int value;
Event *event;

{
	window_destroy(base_frame);
}

gammalut()

{
	int i;
	double gr,gg,gb;
	
	gr = 1./gammar;
	gg = 1./gammag;
	gb = 1./gammab;
	gred[0] = ggreen[0] = gblue[0] = 0;
	for (i=1;i<256;i++) {
		gred[i] = 255.*pow((double) i/255.,gr) + .5;
		ggreen[i] = 255.*pow((double) i/255.,gg) + .5;
		gblue[i] = 255.*pow((double) i/255.,gb) + .5;
	}
	sgred[0] = sggreen[0] = sgblue[0] = 0;
	for (i=1;i<64;i++) {
		sgred[i] = 255.*pow((double) i/63.,gr) + .5;
		sggreen[i] = 255.*pow((double) i/63.,gg) + .5;
		sgblue[i] = 255.*pow((double) i/63.,gb) + .5;
	}
	sprintf(gmapname,"gammar%fg%fb%f",gammar,gammag,gammab);
	sprintf(sgmapname,"sgammar%fg%fb%f",gammar,gammag,gammab);
}

setcmap()

{
	if (obinary) {
		pw_setcmsname(canpixwin,"sunvb");
		pw_putcolormap(canpixwin,0,2,bred,bgreen,bblue);
	}
	else if (cmaptype == 1 || (cmaptype == 0 && !hmapvalid)) { /* ramp */
		if (fullsw) {
			pw_setcmsname(canpixwin,"sunvf");
			pw_putcolormap(canpixwin,0,256,rred,rgreen,rblue);
		}
		else {
			pw_setcmsname(canpixwin,"sunv");
			pw_putcolormap(canpixwin,0,64,srred,srgreen,srblue);
		}
	}
	else if (cmaptype == 0) {	/* header map */
		pw_setcmsname(canpixwin,hmapname);
		pw_putcolormap(canpixwin,0,hnumcol,hred,hgreen,hblue);
	}
	else if (cmaptype == 2) {	/* gamma map */
		if (fullsw) {
			pw_setcmsname(canpixwin,gmapname);
			pw_putcolormap(canpixwin,0,256,gred,ggreen,gblue);
		}
		else {
			pw_setcmsname(canpixwin,sgmapname);
			pw_putcolormap(canpixwin,0,64,sgred,sggreen,sgblue);
		}
	}
	else {	/* file map */
		pw_setcmsname(canpixwin,cmapname);
		pw_putcolormap(canpixwin,0,cnumcol,cred,cgreen,cblue);
	}
	if (image_shown)
		displayit(canvas,canpixwin,(Rectlist *) 0);
}

static void gammar_proc(item,value,event)

Panel_item item;
unsigned int value;
Event *event;

{
	msg("");
	gammar = ((double) value)/100.;
	gammalut();
	if (cmaptype == 2)
		setcmap();
}

static void gammag_proc(item,value,event)

Panel_item item;
unsigned int value;
Event *event;

{
	msg("");
	gammag = ((double) value)/100.;
	gammalut();
	if (cmaptype == 2)
		setcmap();
}

static void gammab_proc(item,value,event)

Panel_item item;
unsigned int value;
Event *event;

{
	msg("");
	gammab = ((double) value)/100.;
	gammalut();
	if (cmaptype == 2)
		setcmap();
}

static void setpixrange(item,value,event)

Panel_item item;
unsigned int value;
Event *event;

{
	wantfullsw = (value == 1);
	msg("change in greylevel usage takes effect at next file load");
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
	else
		setcmap();
}

static void loadcmap_proc(item,value,event)

Panel_item item;
unsigned int value;
Event *event;

{
	Filename fname;

	msg("");
	fname = (Filename) panel_get_value(fileitem);
	if (strlen(fname) == 0) {
		msg("null file name");
		return;
	}
	if (readcmap(fname,256,&cnumcol,cred,cgreen,cblue) == HIPS_ERROR) {
		msg(hipserr);
		cmapvalid = FALSE;
		panel_set(maptypeitem,
			PANEL_CHOICE_STRINGS,"Hdr/Ramp","Ramp","Gamma",0,0);
		if (cmaptype == 3) {
			cmaptype = 0;
			panel_set(maptypeitem,PANEL_VALUE,0,0);
		}
	}
	else {
		cmapvalid = TRUE;
		if (fullsw) {
		    panel_set(maptypeitem,
			PANEL_CHOICE_STRINGS,"Hdr/Ramp","Ramp","Gamma","File",0,
			PANEL_VALUE,3,
			0);
		    cmaptype = 3;
		}
	}
	cmapname = strsave(fname);
	if (obinary)
		msg("color map ignored for binary image");
	else
		setcmap();
}
