/**************************************************************************
*
*  Copyright (c) 1990, 1991, 1992, 1993 Jens Michael Carstensen
*
*  File:   xshow.c
*  Author: J. Michael Carstensen, 
*	   Institute of Mathematical Statistics 
*	   and Operations Research (IMSOR),
*	   Technical University of Denmark.
*
*  Disclaimer:  No guarantees of performance accompany this software,
*  nor is any responsibility assumed on the part of the author or IMSOR.
*  All the software has been tested extensively and every effort has 
*  been made to insure its reliability.
* 
*  To compile:
*	cc -o xshow xshow.c -lX11 -lhips -lm
*
***************************************************************************
*  
*  Acknowledgements:
*
*  The Author was inspired by many other programs and programmers. 
*  I wish to acknowledge my inspiration from:
*
*  histobe.c	by Allan Aasbjerg Nielsen, IMSOR
*  xhips.c 	by Patrick J. Flynn
*  xim.c 	by Philip R. Thompson, M.I.T.
*  xshowdraw.c	by Niels Flensted-Jensen, IMSOR
*  xv.c 	by John Bradley, University of Pennsylvania
*
**************************************************************************/

#define Version		"3.2"

/******* These defines should be set for the specific implementation *****/

#define Maxclass	10
#define Lutdir		"/ussnk1/images/lut/"

/************** End of implementation specific defines *******************/

/* Image formats */ 

#define	HIPS		0
#define	GIF		1
#define	TIFF		2
#define	SUN		3
#define	RLE		4
#define	XBM		5
#define	GOP		6
#define	PBM    		7
#define	TGA		8
#define	PS		9
#define	JFIF 		10	
#define	BMP 		11	

/* Preset colormaps */ 

#define	Hot		1
#define	Rainbow		2
#define	Graylevel	3
#define RGB		4

/* Histogram match functions */

#define Equalize	1
#define Convex		2
#define Concave		3

/* Button 1 modes */

#define N1modes		2
#define Mark 	 	0
#define Threshold	1
char	*B1text[]={"MARK     ","THRESHOLD"};

/* Button 2 modes */

#define N2modes		2
#define Drawclass 	0
#define Profile 	1
char	*B2text[]={"DRAWCLASS","PROFILE  "};

/* Button 3 modes */

#define N3modes		4
#define Stretch 	0
#define Extract		1
#define Enlarge		2
#define Histo		3
char	*B3text[]={"STRETCH  ","EXTRACT  ","ENLARGE  ","HISTO    "};

#define Short		short int
#define Byte		unsigned char

#include <fcntl.h>
#include <hipl_format.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/Xutil.h>

char		*pform[]={
			"byte","short","int","float","complex","ASCII",
			"double","double complex","","","",
			"","","","","",
			"","","","","",
			"","","","","",
			"","","","","",
			"","signed byte","unsigned short","unsigned int","rgb",
			"rgbz","","","",""
		};
int		classcolors[][3]={
			{  0,  0,  0},
			{255,  0,  0},
			{  0,255,  0},
			{  0,  0,255},
			{255,255,  0},
			{  0,255,255},
			{255,  0,255},
			{255,127,  0},
			{  0,255,127},
			{127,  0,255},
			{127,255,  0}
		};
u_long  	blackpixel, whitepixel, drawpixel, classpixel[Maxclass+1];
u_long		backgroundpixel;
Bool  		debug_flag = False, pipefunction=False, original=True;
Display  	*dpy;
int  		screen,depth;
int		nrows,ncols;
Window  	root_win,image_win;
GC		image_gc;
Visual  	*visual = NULL,*rootvisual;
char		lutfname[256];
Byte		*buffer;
int		map[256];
Bool		rgb=False;
FILE		*inpipe;

void syntax()
{
        fprintf(stderr,"Usage: (xshow %s)\n",Version); 
        fprintf(stderr,"xshow ");
	fprintf(stderr,"[-anim] ");
	fprintf(stderr,"[-debug] ");
	fprintf(stderr,"[-display disp] "); 
	fprintf(stderr,"[-hot] ");
	fprintf(stderr,"\n      ");
	fprintf(stderr,"[-interval nmusec] ");
	fprintf(stderr,"[-load] ");
	fprintf(stderr,"[-lut lutfile] ");
	fprintf(stderr,"[-name ttl] ");
	fprintf(stderr,"[-newcmap] ");
	fprintf(stderr,"\n      ");
	fprintf(stderr,"[-pipe] ");
	fprintf(stderr,"[-rainbow] ");
	fprintf(stderr,"[-title ttl] ");
	fprintf(stderr,"[-unscaled] ");
	fprintf(stderr,"\n      ");
	fprintf(stderr,"[-Usage] ");
	fprintf(stderr,"[-wrap] ");
	fprintf(stderr,"[-16] ");
	fprintf(stderr,"[-256] ");
	fprintf(stderr,"\n      ");
	fprintf(stderr,"[< iseq | <input file>] ");
	fprintf(stderr,"\n");
}

int main(argc, argv)
int argc;
char **argv;
{
    register  int 	i, j, k;
    int			f,r,c,nr,nc;
    FILE		*fp,*tfp,*infp=stdin;
    char		*infilename;
    Byte		magic[20];
    int  		iconcols, iconrows, iconpix, iconfact;
    int  		npix, sizepix, nf, pixfmt, ncolors=64; 
    int			xp, yp, len; 
    int			lutselect=Graylevel,mapentries;
    int			button1_mode=0,button2_mode=0,button3_mode=0;
    double		*dbuffer,dmin,dmax,*dptr,intdif,colorscale;
    float		*fbuffer,fmin,fmax,*fptr;
    int			*ibuffer,imin,imax,*iptr;
    Short		*sbuffer,smin,smax,*sptr;
    Byte		*bbuffer,bmin,bmax,*bptr,*trainptr = NULL;
    Byte	  	*icon_buf;
    Byte		*bufptr,*subptr,*cptr,*origptr;
    char  		*wname, *display_name = NULL;
    char		keybuffer[10],hips_name[10];
    char		str1[80];
    Bool  		newmap_flag = False, conmap_flag = False, noread=False;
    Bool		noscale = False, oldframe;
    Bool		animate = False,loadall=False,wrap=False,reverse=False;
    Bool		sidebar=True,pipe_image=False,slicing=False;
    Bool		Pressed1 = False, Pressed2 = False, Pressed3 = False;
    Bool		compress=False,popened=False;
    int			oldX, oldY, newX, newY;
    struct header 	hd,nhd,thd;
    int                 iclass = 0;
    int			sidebarwidth,colbarwidth,colwidth=110;
    int                 l,m,count,thrmin,thrmax;
    int			image_format=HIPS;
    clock_t		goal,nmusec=40000,clk;
    u_long 		color2pix();
    void		DrawRect(),fill(),fillrow(),fillrow1(),dline(),makecolormap();
    void		histobe();
    int			hips_cmd();


    Colormap  		colormap, GetColormap(),blackcmap;
    Cursor		def_cursor,wait_cursor;
    Font		font;
    XFontStruct		*fontstruct;
    unsigned long	gcmask;
    GC  		icon_gc;
    Pixmap		icon_pixmap;
    Window  		icon_win;
    XColor  		colors[256], fore_color, back_color, qcolor,blackcolor;
    XEvent  		event;
    XExposeEvent  	*expose;
    XCrossingEvent  	*xcrossing;
    XGCValues  		gc_val, gc_new, gc_draw;
    XImage  		*image = NULL, *icon_image = NULL;
    XSetWindowAttributes  xswa;
    XStandardColormap	best_rgb;
    XSizeHints  	sizehints;
    XVisualInfo		xvisinfo;
    XWMHints  		wmhints;

    Progname = strsave(*argv);
    wname = "xshow";
    infilename = "stdin";

/* Parse command line */

    for (i=1;i<argc;i++) {
        if (strncmp(argv[i], "-anim", 5) == 0) {
            animate = True;
            continue;
        }
        if (strncmp(argv[i], "-de", 3) == 0) {
            debug_flag = True;
            continue;
        }
        if (strncmp(argv[i], "-dis", 4) == 0) {
            display_name = argv[++i];
            continue;
        }
        if (strncmp(argv[i], "-hot", 4) == 0) {
            lutselect = Hot;
            continue;
        }
        if (strncmp(argv[i], "-inter", 6) == 0) {
            nmusec = atoi(argv[++i]);
            continue;
        }
        if (strncmp(argv[i], "-load", 5) == 0) {
            loadall = True;
            continue;
        }
        if (strncmp(argv[i], "-lut", 4) == 0) {
            lutselect = 0;
	    strcpy(lutfname,Lutdir);
	    strcat(lutfname,argv[++i]);
	    strcat(lutfname,".lut");
            continue;
        }
        if (strncmp(argv[i], "-name", 5) == 0 || 
	    strncmp(argv[i], "-title", 6) == 0) {
            wname = argv[++i];
            continue;
        }
        if (strncmp(argv[i], "-newcmap", 8) == 0) {
            newmap_flag = True;
            continue;
        }
        if (strncmp(argv[i], "-p", 2) == 0) {
            pipe_image = True;
            continue;
        }
        if (strncmp(argv[i], "-rai", 4) == 0) {
            lutselect = Rainbow;
            continue;
        }
        if (strncmp(argv[i], "-u", 2) == 0) {
            noscale = True;
            continue;
        }
        if (strncmp(argv[i], "-U", 2) == 0) {
            syntax();;
	    exit(0);
        }
        if (strncmp(argv[i], "-wrap", 5) == 0) {
            loadall = True;
	    wrap = True;
            continue;
        }
        if (strncmp(argv[i],"-16",3) == 0) {
            ncolors=16;
            continue;
        }
        if (strncmp(argv[i],"-256",4) == 0) {
            ncolors=256;
            continue;
        }
        if (strncmp(argv[i],"-",1) == 0) {
	    fprintf(stderr,"xshow: unknown option '%s'\n",argv[i]);
	    syntax();
	    exit(1);
	}
	if (infp!=stdin) {
	    fprintf(stderr,"xshow: input file already opened\n");
	    fprintf(stderr,"       file '%s' not opened\n",argv[i]);
	    continue;
	}
	infilename=argv[i];
	if (infilename[strlen(infilename)-2]=='.' &&
		infilename[strlen(infilename)-1]=='Z') {
		sprintf(str1,"uncompress %s",infilename);
		system(str1);
		infilename[strlen(infilename)-2]=0;
		compress=True;
	}
	if ((infp=fopen(argv[i],"r")) == NULL) {
	    fprintf(stderr,"xshow: unable to read '%s'\n",argv[i]);
	    syntax();
	    exit(1);
	} else {
	    if (fread(magic,1,20,infp) != 20)
		perr(HE_MSG,"Error during read of magic");
	    if (strncmp(magic,"HIPS",4) == 0) {
		image_format=HIPS;
	    }
	    else if (strncmp(magic,"#define",7) == 0) {
		image_format=XBM;
		fprintf(stderr,"X bitmap\n");
	    }
	    else if (strncmp(magic,"GIF",3) == 0) {
		image_format=GIF;
		fprintf(stderr,"GIF image\n");
	    }
	    else if ((strncmp(magic,"MM",2) == 0) ||
		     (strncmp(magic,"II",2) == 0)) {
		image_format=TIFF;
		fprintf(stderr,"TIFF image\n");
	    }
	    else if (strncmp(magic,"BM",2) == 0) {
		image_format=BMP;
		fprintf(stderr,"Windows bitmap\n");
	    }
	    else if ((magic[0] == 0xe9) && (magic[1] == 0xed)) {
		image_format=GOP;
		fprintf(stderr,"GOP image\n");
	    }
	    else if ((magic[0] == 0x52) && (magic[1] == 0xcc)) {
		image_format=RLE;
		fprintf(stderr,"RLE image\n");
	    }
	    else if ((magic[0] == 0x59) && (magic[1] == 0x15)) {
		image_format=SUN;
		fprintf(stderr,"SUN raster image\n");
	    }
	    else if ((magic[0] == 0xff) && (magic[1] == 0xd8) &&
		     (magic[2] == 0xff)) {
		image_format=JFIF;
		fprintf(stderr,"JPEG image\n");
	    }
	    else if ((magic[0] == '%') && (magic[1] == '!')) {
		image_format=PS;
		fprintf(stderr,"PostScript file\n");
	    }
	    else if ((magic[0]=='P') && (magic[1]>'0') && (magic[1]<'7')) {
		image_format=PBM;
		fprintf(stderr,"PBM image\n");
	    }
	    else if ((magic[1]<2) && (magic[2]<12)) {
		image_format=TGA;
		fprintf(stderr,"Targa image\n");
	    }
	    if (image_format != HIPS) fclose(infp); else rewind(infp);
	}
    }

/*  Open the display & set default screen and root window */

    if ((dpy = XOpenDisplay(display_name)) == NULL) {
        fprintf(stderr,"Can't open display '%s'\n", XDisplayName(display_name));
	exit(1);
    }
    screen 	= XDefaultScreen(dpy);
    root_win 	= XDefaultRootWindow(dpy);
    if (XDisplayPlanes(dpy, screen) == 1) {
    	fprintf(stderr,"Can't display grayscale on monochrome screen\n");
	exit(1);
    }

/* Read header */

    switch (image_format) {
    case HIPS: 	fread_header(infp,&hd,infilename);
		break;
    case GOP:   
		sprintf(str1,"imf2hips <%s",infilename);
		infp=popen(str1,"r");
		fread_header(infp,&hd,infilename);
		break;
    case TGA:   
		sprintf(str1,"tga2hips <%s",infilename);
		infp=popen(str1,"r");
		fread_header(infp,&hd,infilename);
		break;
    default:    fprintf(stderr,"This format is not supported\n");
		exit(0);
    }
    update_header(&hd,argc,argv);
    if (pipe_image) write_header(&hd);
    ncols=hd.ocols;
    nrows=hd.orows;
    npix = ncols * nrows;
    nf=hd.num_frame;
    pixfmt=hd.pixel_format;
    switch (pixfmt) {
    case PFBYTE   : sizepix=1; break;
    case PFSHORT  : sizepix=2; break;
    case PFINT    : sizepix=4; break;
    case PFFLOAT  : sizepix=4; break;
    case PFDOUBLE : sizepix=8; break;
    case PFRGB    : sizepix=4; rgb=True; break;
    case PFRGBZ   : sizepix=4; rgb=True; break;
    default       : fprintf(stderr,"bad pixel format\n"); exit(1);
    }
    fprintf(stderr,"%d x %d %s image displayed in\n",nrows,ncols,pform[pixfmt]);

    nhd=hd;
    nhd.num_frame=1;
    thd=hd;
    thd.num_frame=1;
    thd.pixel_format = PFBYTE;

/* Select visual */

    if (rgb) {
	if (XMatchVisualInfo(dpy,screen,24,DirectColor,&xvisinfo)) {
		visual=xvisinfo.visual;
		depth=xvisinfo.depth;
		ncolors=visual->map_entries;
		fprintf(stderr,
			"24-bit DirectColor, %d cmap entries, %d used.\n",
			visual->map_entries,ncolors);
	}
	else {
		fprintf(stderr,"24-bit DirectColor not found\n");
		exit(1);
	}
	lutselect=RGB;
	sidebar=False;
    }
    else {
	if (ncolors==256 && 
	   XMatchVisualInfo(dpy,screen,8,PseudoColor,&xvisinfo)) {
		visual=xvisinfo.visual;
		depth=xvisinfo.depth;
		fprintf(stderr,
			"8-bit PseudoColor, %d cmap entries, %d used.\n",
			visual->map_entries,ncolors);
	}
	else {
    		depth 	= XDefaultDepth(dpy,screen);
    		visual 	= XDefaultVisual(dpy, screen);
		fprintf(stderr,"%d-bit ",depth);
		switch (visual->class) {
		case PseudoColor: fprintf(stderr,"PseudoColor"); break;
		case DirectColor: fprintf(stderr,"DirectColor"); break;
		case GrayScale: fprintf(stderr,"GrayScale"); break;
		case StaticColor: fprintf(stderr,"StaticColor"); break;
		case TrueColor: fprintf(stderr,"TrueColor"); break;
		case StaticGray: fprintf(stderr,"StaticGray"); break;
		}
		fprintf(stderr,", %d cmap entries, %d used.\n",
			visual->map_entries,ncolors);
	}
    }
    rootvisual=XDefaultVisual(dpy, screen);
    mapentries	= visual->map_entries;
    blackpixel 	= XBlackPixel(dpy, screen);
    whitepixel 	= XWhitePixel(dpy, screen);
    def_cursor=XCreateFontCursor(dpy, XC_crosshair);
    wait_cursor=XCreateFontCursor(dpy, XC_watch);

/* Allocate buffers */

    if (rgb) {
	buffer=(Byte *) halloc(npix*4,1);		
	bufptr=buffer;
    }
    else {
	buffer=(Byte *) halloc(npix,1);
    	bufptr=(Byte *) halloc(npix*((loadall) ? nf : 1),sizepix);
    }
    dbuffer=(double *) bufptr;
    fbuffer=(float  *) bufptr;
    ibuffer=(int    *) bufptr;
    sbuffer=(Short  *) bufptr;
    bbuffer=(Byte *) bufptr;
    hd.image=nhd.image=subptr=origptr=bufptr;

/*  Allocate the icon with max. dimension of 50 */

    iconfact = (nrows/50) > (ncols/50) ? (nrows/50) : (ncols/50);
    if (iconfact == 0) iconfact = 1;
    if ((iconcols = ncols / iconfact +1) % 2) iconcols -= 1;
    iconrows = nrows / iconfact;
    iconpix = iconrows*iconcols;
    if (debug_flag)
        fprintf(stderr,"icon width %d  height %d  factor %d\n",
            iconcols, iconrows, iconfact);
    icon_buf = (Byte *)halloc(iconpix,(rgb) ? 4 : 1);

/* Make and get the colormap */

    makecolormap(colors,ncolors,lutselect);
    if (ncolors > mapentries-6)          /* Don't bother trying to fit */
      newmap_flag = True;     /* into default map, faster too */
    colormap = GetColormap(colors, ncolors, &newmap_flag,&conmap_flag);

/* Create image and icon  */

    icon_image = XCreateImage(dpy, visual, depth, ZPixmap,
		0, (char *)icon_buf, iconcols, iconrows, 8, 0);
    image = XCreateImage(dpy, visual, depth, ZPixmap,
    		0, (char *)buffer, ncols, nrows, 8, 0);

/* Set window attributes */

    xswa.event_mask = ExposureMask | ButtonPressMask | ColormapChangeMask |
        LeaveWindowMask | EnterWindowMask;
    xswa.background_pixel = backgroundpixel;
    xswa.border_pixel = whitepixel;
    xswa.colormap = colormap;
    xswa.cursor = def_cursor;
    colbarwidth=sidebarwidth=BETWEEN(ncols/20,10,20);
    if (rgb) sidebarwidth=0;
    image_win = XCreateWindow(dpy, root_win, 0, 0,
        ncols+sidebarwidth+colwidth, nrows, 5, depth,
        InputOutput, visual, CWBackPixel | CWEventMask | CWCursor |
        CWBorderPixel | CWColormap, &xswa);
    xswa.event_mask = ExposureMask, 
    icon_win = XCreateWindow(dpy, root_win, 0, 0,
        iconcols, iconrows, 1, depth,
        InputOutput, visual, CWBackPixel | CWBorderPixel | CWColormap, &xswa);

/* Set window manager hints */

    sizehints.flags = PPosition | PSize | PMinSize | PMaxSize | PResizeInc;
    sizehints.min_width = ncols;
    sizehints.max_width = ncols+sidebarwidth+colwidth;
    sizehints.min_height = MAX(64,nrows);
    sizehints.max_height = MAX(100,nrows);
    sizehints.width = sizehints.max_width;	/* obsolete as of R4 */
    sizehints.height = sizehints.min_height;	/* obsolete as of R4 */
    sizehints.x = 0;	/* obsolete as of R4 */
    sizehints.y = 0;	/* obsolete as of R4 */
    sizehints.width_inc=1;
    sizehints.height_inc=0;
    XSetStandardProperties(dpy, image_win, wname, Progname,
            None, argv, argc, &sizehints);
/*
    icon_pixmap = XCreatePixmapFromBitmapData(dpy,screen,xlogo32_bits,
		xlogo32_width,xlogo32_height,blackpixel,whitepixel,depth);
    wmhints.icon_pixmap = icon_pixmap;
*/
    wmhints.flags = IconWindowHint | IconPositionHint;
    wmhints.icon_window = icon_win;
    wmhints.icon_x = XDisplayWidth(dpy,screen) - 200;
    wmhints.icon_y = 2;
    XSetWMHints(dpy, image_win, &wmhints);

    gc_val.function = GXcopy;
    gc_val.plane_mask = AllPlanes;
    gc_val.foreground = blackpixel;
    gc_val.background = whitepixel;
    if ((fontstruct=XLoadQueryFont(dpy,"8x13bold")) != NULL) {
    	gc_val.font = font = fontstruct->fid;
	gcmask=GCFunction | GCPlaneMask | GCForeground | GCBackground | GCFont;
    }
    else
	gcmask=GCFunction | GCPlaneMask | GCForeground | GCBackground;
    gc_draw.function = GXcopy;
    gc_draw.plane_mask = AllPlanes;
    gc_draw.foreground = color2pix(colormap, ncolors, 255, 0, 0);
    gc_draw.background = whitepixel;
    gc_new.function = GXinvert;
    gc_new.foreground = XBlackPixel (dpy, screen);
    gc_new.background = XWhitePixel (dpy, screen);
    image_gc = XCreateGC(dpy,image_win, gcmask, &gc_val);
    icon_gc = XCreateGC(dpy, icon_win, GCFunction | GCPlaneMask |
        GCForeground | GCBackground, &gc_val);

/* Map image window. */

    XMapWindow(dpy, image_win);
    if (newmap_flag) {
        XInstallColormap(dpy, colormap);
        if ((ncolors > 254) && (!rgb)) {
            fore_color.red   = colors[255].red;     /* force the last */
            fore_color.green = colors[255].green;   /* two colors and */
            fore_color.blue  = colors[255].blue;   /* sacrifice cursor */
            back_color.red   = colors[254].red;
            back_color.green = colors[254].green;
            back_color.blue  = colors[254].blue;
            XRecolorCursor(dpy, xswa.cursor, &fore_color, &back_color);
        }
    }

/*
    if (rgb) {
	XAllocColorPlanes(dpy,colormap,True,pixels,ncolors,8,8,8,
		xvisinfo.red_mask,xvisinfo.green_mask,xvisinfo.blue_mask);
    	XGetStandardColormap(dpy,root_win,&best_rgb,XA_RGB_BEST_MAP);
	xswa.colormap=best_rgb.colormap;
    	XChangeWindowAttributes(dpy,image_win,CWColormap,&xswa);
    }
    */

/* Select events to listen for  */

    XSelectInput(dpy, image_win, (ButtonPressMask | ButtonReleaseMask | 
	PointerMotionMask | KeyPressMask |
	ColormapChangeMask | ExposureMask | LeaveWindowMask | EnterWindowMask));
    XSelectInput(dpy, icon_win, ExposureMask | LeaveWindowMask |
	ColormapChangeMask | EnterWindowMask);

/* Load all frames if load option set */

    if (loadall) {
	if (fread(bufptr,sizepix,nf*npix,infp)!=nf*npix) {
		fprintf(stderr,"Error during read\n");
		exit(1);
	}
    	wait(0);wait(0);wait(0);wait(0);wait(0);wait(0);
	if (pipe_image) 
	if (fwrite(bufptr,sizepix,nf*npix,stdout)!=nf*npix) {
		fprintf(stderr,"Error during write\n");
		exit(1);
	}
    }

    expose = (XExposeEvent *)&event;
    xcrossing = (XCrossingEvent *)&event;
    goal=clock();
    if (debug_flag) fprintf(stderr,"Starting frame loop\n");
    xp=ncols+1;
    yp=0;

/***************************** For each frame ******************************/
    for (f=0;f<nf && f>=0;f+=(reverse) ? -1 : 1) {

    /* Time alignment */
    if (animate) {
	while ((clk=clock())<goal);
	goal=clk+nmusec;
    }

    /* Display frame number */
    XSetForeground(dpy,image_gc,whitepixel);
    XSetBackground(dpy,image_gc,backgroundpixel);
    sprintf(str1,"Frame : %3d",f);
    XDrawImageString(dpy,image_win,image_gc,
    	ncols+sidebarwidth+5,20,str1,11);

    /* Get next frame and display current pixel value */
    if (!noread) {
	if (loadall) {
		bufptr=origptr+f*npix*sizepix;
    		dbuffer=(double *) bufptr;
    		fbuffer=(float  *) bufptr;
    		ibuffer=(int    *) bufptr;
    		sbuffer=(Short  *) bufptr;
    		bbuffer=(Byte *) bufptr;
	}
	else {
	    if (fread(bufptr,hd.sizepix,npix,infp)!=npix) {
		fprintf(stderr,"Error during read of frame %d\n",f);
		exit(1);
	    }
	    if (pipe_image)
	    if (fwrite(bufptr,hd.sizepix,npix,stdout)!=npix) {
		fprintf(stderr,"Error during write of frame %d\n",f);
		exit(1);
	    }
	}
 	if (xp<ncols) 
		switch (pixfmt) {
		case PFDOUBLE:
			sprintf(str1,"  %f    ",dbuffer[ncols*yp+xp]); break;
		case PFFLOAT:
			sprintf(str1,"  %f    ",fbuffer[ncols*yp+xp]); break;
		case PFINT:
			sprintf(str1,"  %9d",ibuffer[ncols*yp+xp]); break;
		case PFSHORT:
			sprintf(str1,"  %9d",sbuffer[ncols*yp+xp]); break;
		case PFBYTE:
			sprintf(str1,"  %9u",bbuffer[ncols*yp+xp]); break;
		case PFRGB:
			sprintf(str1,"%3d,%3d,%3d",
				buffer[(ncols*yp+xp)*4+1],
				buffer[(ncols*yp+xp)*4+2],
				buffer[(ncols*yp+xp)*4+3]); break;
		case PFRGBZ:
			sprintf(str1,"%3d,%3d,%3d,%3d",
				buffer[(ncols*yp+xp)*4+1],
				buffer[(ncols*yp+xp)*4+2],
				buffer[(ncols*yp+xp)*4+3],
				buffer[(ncols*yp+xp)*4]); break;
		}
	else sprintf(str1,"              ");
    	XDrawImageString(dpy,image_win,image_gc,
    		ncols+sidebarwidth+5,50,str1,strlen(str1));
        if (f==nf-1) {wait(0);wait(0);wait(0);wait(0);wait(0);wait(0);}
    }

    if (!pipefunction) {
    original=True;
    switch (pixfmt) {
    case PFDOUBLE:
		dmin=dbuffer[0];
		dmax=dbuffer[0];
		for (i=1;i<npix;i++) {
			if (dbuffer[i]>dmax) dmax=dbuffer[i];
			if (dbuffer[i]<dmin) dmin=dbuffer[i];
		}
		intdif=1.000001*(dmax-dmin);
		if (intdif == 0.0) intdif=1.0;
		colorscale=(double)ncolors/intdif;
		for (i=0;i<npix;i++) 
		    buffer[i]=(Byte)(colorscale*(dbuffer[i]-dmin));
		break;
    case PFFLOAT:
		fmin=fbuffer[0];
		fmax=fbuffer[0];
		for (i=1;i<npix;i++) {
			if (fbuffer[i]>fmax) fmax=fbuffer[i];
			if (fbuffer[i]<fmin) fmin=fbuffer[i];
		}
		intdif=1.000001*(fmax-fmin);
		if (intdif == 0.0) intdif=1.0;
		colorscale=(double)ncolors/intdif;
		for (i=0;i<npix;i++) 
		    buffer[i]=(Byte)(colorscale*(double)(fbuffer[i]-fmin));
		break;
    case PFINT:
		imin=ibuffer[0];
		imax=ibuffer[0];
		for (i=1;i<npix;i++) {
			if (ibuffer[i]>imax) imax=ibuffer[i];
			if (ibuffer[i]<imin) imin=ibuffer[i];
		}
		if (imin==imax) {
			fprintf(stderr,"Frame %d: unicolor %d\n",f,imin);
			intdif=1.0;
		}
		else intdif=1.000001*(double)(imax-imin);
		colorscale=(double)ncolors/intdif;
		for (i=0;i<npix;i++) 
		    buffer[i]=(Byte)(colorscale*(double)(ibuffer[i]-imin));
		break;
    case PFSHORT:
		smin=sbuffer[0];
		smax=sbuffer[0];
		for (i=1;i<npix;i++) {
			if (sbuffer[i]>smax) smax=sbuffer[i];
			if (sbuffer[i]<smin) smin=sbuffer[i];
		}
		if (smin==smax) {
			fprintf(stderr,"Frame %d: unicolor %d\n",f,smin);
			intdif=1.0;
		}
		else intdif=1.000001*(double)(smax-smin);
		colorscale=(double)ncolors/intdif;
		for (i=0;i<npix;i++) 
		    buffer[i]=(Byte)(colorscale*(double)(sbuffer[i]-smin));
		break;
    case PFBYTE:
		if (noscale) {
		  if (ncolors==256) memcpy(buffer,bbuffer,npix);
		  if (ncolors==64) 
		  	for (i=0;i<npix;i++) buffer[i]=(bbuffer[i]>>2);
		  if (ncolors==16) 
			for (i=0;i<npix;i++) buffer[i]=(bbuffer[i]>>4);
		}
		else {
		  bmin=bbuffer[0];
		  bmax=bbuffer[0];
		  for (i=1;i<npix;i++) {
			if (bbuffer[i]>bmax) bmax=bbuffer[i];
			if (bbuffer[i]<bmin) bmin=bbuffer[i];
		  }
		  if (bmin==bmax) {
			fprintf(stderr,"Frame %d: unicolor %d\n",f,bmin);
			intdif=1.0;
		  }
		  else intdif=1.000001*(double)(bmax-bmin);
		  colorscale=(double)ncolors/intdif;
		  for (i=0;i<npix;i++) 
		      buffer[i]=(Byte)(colorscale*(double)(bbuffer[i]-bmin));
		  }
		break;
    case PFRGB: 
		j=3*npix-1;
		for (i=4*npix-1;i>=3;) {
			buffer[i--]=buffer[j--];
			buffer[i--]=buffer[j--];
			buffer[i--]=buffer[j--];
			buffer[i--]=0;
		}
		break;
    case PFRGBZ:
		for (i=0;i<npix;i++) {
			j=4*i;
			k=buffer[j+3];
			buffer[j+3]=buffer[j+2];
			buffer[j+2]=buffer[j+1];
			buffer[j+1]=buffer[j];
			buffer[j]=k;
		}
		break;
    } /* end case */
    if (conmap_flag) 
	for (i=0;i<npix;i++) buffer[i] = (Byte)colors[buffer[i]].pixel;
    } /* end if (!pipefunction) ... */
    pipefunction=False;

    /* Update icon (except during animation) */
    if (!animate || f==0) {
    if (rgb)
    for (i=0;i<iconrows;i++)  for (j=0;j<iconcols;j++) {
	icon_buf[i*iconcols+j]=buffer[(i*ncols+j)*4*iconfact];
	icon_buf[i*iconcols+j+1]=buffer[(i*ncols+j)*4*iconfact+1];
	icon_buf[i*iconcols+j+2]=buffer[(i*ncols+j)*4*iconfact+2];
	icon_buf[i*iconcols+j+3]=buffer[(i*ncols+j)*4*iconfact+3];
    }
    else for (i=0;i<iconrows;i++)  for (j=0;j<iconcols;j++) {
	icon_buf[i*iconcols+j]=buffer[(i*ncols+j)*iconfact];
    }
    } /* if !animate */

    /* Update image */
    if (f>0 || noread || wrap) {
        XPutImage(dpy, image_win, image_gc, image, 0, 0,
        	0, 0, ncols, nrows);
    }   

    /* Event loop for image interactions */
    noread=False;
    oldframe=True;
    while (oldframe) {

	if (XEventsQueued(dpy,QueuedAfterFlush)==0) {
        	if (f>0 && f<nf-1 && animate) {
			break;
		}
		if (wrap && animate) {
			break;
		}
	}

        XNextEvent(dpy, &event); 
	xp=event.xbutton.x;
	yp=event.xbutton.y;
        switch((int)event.type) {
        int modulo;     /* Temporary var. for expose->x % 4 */
        case Expose:
            if (expose->window == icon_win) {
                XPutImage(dpy, icon_win, icon_gc, icon_image, 0, 0,
                    0, 0, iconcols, iconrows);
                break;
            }
            if (debug_flag)
                fprintf(stderr,
                "expose event x= %d y= %d width= %d height= %d\n",
                expose->x, expose->y, expose->width, expose->height);
            modulo = expose->x % 4;
            if (modulo != 0) {
                expose->x -= modulo;
                expose->width += modulo;
            }

    	    if (sidebar) {
	    XSetFunction(dpy,image_gc,GXcopy);
	    if (0) {
		for (i=0;i<nrows;i++) {
		  XSetForeground(dpy,image_gc,
			((u_long)((i*ncolors)/nrows)<<16) |
			(color2pix(colormap,ncolors,0,0,0) & (u_long)0x00ffff));
		  XDrawLine(dpy,image_win,image_gc,
			ncols,nrows-i-1,ncols+colbarwidth-1,nrows-i-1);
		  XSetForeground(dpy,image_gc,
			((u_long)((i*ncolors)/nrows)<<8) |
			(color2pix(colormap,ncolors,0,0,0) & (u_long)0xff00ff));
		  XDrawLine(dpy,image_win,image_gc,
			ncols+colbarwidth,nrows-i-1,
			ncols+2*colbarwidth-1,nrows-i-1);
		  XSetForeground(dpy,image_gc,
			(u_long)((i*ncolors)/nrows) |
			(color2pix(colormap,ncolors,0,0,0) & (u_long)0xffff00));
		  XDrawLine(dpy,image_win,image_gc,
			ncols+2*colbarwidth,nrows-i-1,
			ncols+3*colbarwidth-1,nrows-i-1);
		}
	    }
	    else {
		for (i=0;i<sizehints.max_height;i++) {
		  if (newmap_flag)
		      XSetForeground(dpy,image_gc,
			(u_long)((i*ncolors)/sizehints.max_height));
		  else
		      XSetForeground(dpy,image_gc,
			colors[(i*ncolors)/sizehints.max_height].pixel);
		  XDrawLine(dpy,image_win,image_gc,
			ncols,sizehints.max_height-i-1,
			ncols+sidebarwidth-1,sizehints.max_height-i-1);
		}
		XSetForeground(dpy,image_gc,blackpixel);
		XDrawLine(dpy,image_win,image_gc,
			ncols,0,ncols,sizehints.max_height);
    	    }
	    }

	    /* Text in right column */

    	    XSetForeground(dpy,image_gc,whitepixel);
            XSetBackground(dpy,image_gc,backgroundpixel);
	    XDrawLine(dpy,image_win,image_gc,
		ncols+sidebarwidth,0,ncols+sidebarwidth,sizehints.max_height);

            sprintf(str1,"Frame : %3d",f);
            XDrawImageString(dpy,image_win,image_gc,
    		ncols+sidebarwidth+5,20,str1,11);

    	    sprintf(str1,"(%4d,%4d)",yp,xp);
    	    XDrawImageString(dpy,image_win,image_gc,
    		ncols+sidebarwidth+5,35,str1,11);
	    
	    sprintf(str1,"1) %s    ",B1text[button1_mode]);
    	    XDrawImageString(dpy,image_win,image_gc,
    		ncols+sidebarwidth+5,65,str1,strlen(str1));
	    sprintf(str1,"2) %s    ",B2text[button2_mode]);
    	    XDrawImageString(dpy,image_win,image_gc,
    		ncols+sidebarwidth+5,80,str1,strlen(str1));
	    sprintf(str1,"3) %s    ",B3text[button3_mode]);
    	    XDrawImageString(dpy,image_win,image_gc,
    		ncols+sidebarwidth+5,95,str1,strlen(str1));

	    /* Refresh image */

            if (expose->width % 4 != 0)
                expose->width += 4 - (expose->width % 4);
            XPutImage(dpy, image_win, image_gc, image,
                expose->x, expose->y, expose->x, expose->y,
                expose->width, expose->height);
            if (debug_flag)
                fprintf(stderr, "Actual expose: %d  %d  %d  %d\n",
                expose->x, expose->y, expose->width, expose->height);

	    if (animate) oldframe=False;
            break;
        case ButtonPress:
            switch((int)event.xbutton.button) {
            case Button1: 
                Pressed1 = True;
		XSetInputFocus(dpy,image_win,RevertToParent,event.xbutton.time);
              	oldX = xp;
              	oldY = yp;
		if (xp<ncols && yp<nrows && button1_mode==Mark) {
			XSetForeground(dpy,image_gc,drawpixel);
	       	 	XDrawPoint(dpy,image_win,image_gc,xp,yp); 
	       	 	XDrawPoint(dpy,image_win,image_gc,xp+1,yp); 
	        	XDrawPoint(dpy,image_win,image_gc,xp,yp+1); 
	        	XDrawPoint(dpy,image_win,image_gc,xp-1,yp); 
	        	XDrawPoint(dpy,image_win,image_gc,xp,yp-1); 
		}
		if (xp>ncols+sidebarwidth) {
		if (yp>=53 && yp<67) {
		button1_mode=(button1_mode+1)%2;
	    	sprintf(str1,"1) %s ",B1text[button1_mode]);
    	    	XDrawImageString(dpy,image_win,image_gc,
    			ncols+sidebarwidth+5,65,str1,strlen(str1));
		}
		if (yp>=68 && yp<82) {
		button2_mode=(button2_mode+1)%2;
	    	sprintf(str1,"2) %s ",B2text[button2_mode]);
    	    	XDrawImageString(dpy,image_win,image_gc,
    			ncols+sidebarwidth+5,80,str1,strlen(str1));
		}
		if (yp>=83 && yp<97) {
		button3_mode=(button3_mode+1)%N3modes;
	    	sprintf(str1,"3) %s ",B3text[button3_mode]);
    	    	XDrawImageString(dpy,image_win,image_gc,
    			ncols+sidebarwidth+5,95,str1,strlen(str1));
		}
		}

		break;
            case Button2: 
                Pressed2 = True;
		if (iclass>0) {
		  oldX = xp;
		  oldY = yp;
		}
               	break;
            case Button3:
              	Pressed3 = True;
      		XChangeGC(dpy, image_gc, 
                     GCForeground | GCBackground | GCFunction , &gc_new);
              	oldX = xp;
              	oldY = yp;
              	newX = oldX;
              	newY = oldY;
               	break;
            }
	    break;
        case MotionNotify:
    	    xp=BETWEEN(xp,0,sizehints.max_width);
    	    yp=BETWEEN(yp,0,sizehints.max_height);
    	    sprintf(str1,"(%4d,%4d)",yp,xp);
    	    XSetForeground(dpy,image_gc,whitepixel);
    	    XSetBackground(dpy,image_gc,backgroundpixel);
    	    XDrawImageString(dpy,image_win,image_gc,
    		ncols+sidebarwidth+5,35,str1,11);
	    
 	    if (xp<ncols && yp<nrows) {
		switch (pixfmt) {
		case PFDOUBLE:
			sprintf(str1,"  %f    ",dbuffer[ncols*yp+xp]); break;
		case PFFLOAT:
			sprintf(str1,"  %f    ",fbuffer[ncols*yp+xp]); break;
		case PFINT:
			sprintf(str1,"  %9d",ibuffer[ncols*yp+xp]); break;
		case PFSHORT:
			sprintf(str1,"  %9d",sbuffer[ncols*yp+xp]); break;
		case PFBYTE:
			sprintf(str1,"  %9u",bbuffer[ncols*yp+xp]); break;
		case PFRGB:
			sprintf(str1,"%3d,%3d,%3d",
				buffer[(ncols*yp+xp)*4+1],
				buffer[(ncols*yp+xp)*4+2],
				buffer[(ncols*yp+xp)*4+3]); break;
		case PFRGBZ:
			sprintf(str1,"%3d,%3d,%3d,%3d",
				buffer[(ncols*yp+xp)*4+1],
				buffer[(ncols*yp+xp)*4+2],
				buffer[(ncols*yp+xp)*4+3],
				buffer[(ncols*yp+xp)*4]); break;
		}
	    }
	    else sprintf(str1,"              ");
    	    XDrawImageString(dpy,image_win,image_gc,
    		ncols+sidebarwidth+5,50,str1,strlen(str1));

            if (Pressed1) {
	        yp=BETWEEN(yp,0,nrows-1);
		/*
		XSetForeground(dpy,image_gc,drawpixel);
		if (thrupper && yp<oldY) for (i=yp;i<oldY;i++) 
		    XDrawLine(dpy,image_win,image_gc,
			ncols,i,ncols+sidebarwidth-1,i);
		if (thrlower && yp>oldY) for (i=yp;i>oldY;i--) 
		    XDrawLine(dpy,image_win,image_gc,
			ncols,i,ncols+sidebarwidth-1,i);
		if (thrupper && yp>oldY) for (i=yp;i>oldY;i--) {
		    XDrawLine(dpy,image_win,image_gc,
			ncols,i,ncols+sidebarwidth-1,i);
		    if (newmap_flag)
		      XSetForeground(dpy,image_gc,
			(u_long)(((nrows-1-i)*ncolors)/nrows));
		    else
		      XSetForeground(dpy,image_gc,
			colors[((nrows-1-i)*ncolors)/nrows].pixel);
		}
		if (thrlower && yp<oldY) for (i=yp;i<oldY;i++) {
		    if (newmap_flag)
		      XSetForeground(dpy,image_gc,
			(u_long)(((nrows-1-i)*ncolors)/nrows));
		    else
		      XSetForeground(dpy,image_gc,
			colors[((nrows-1-i)*ncolors)/nrows].pixel);
		    XDrawLine(dpy,image_win,image_gc,
			ncols,i,ncols+sidebarwidth-1,i);
		}
		*/
		oldY=yp;
	    }
            if (Pressed2) {
	        xp=BETWEEN(xp,0,ncols-1);
	        yp=BETWEEN(yp,0,nrows-1);
		if (iclass>0) {
			dline(trainptr,oldX,oldY,xp,yp,iclass);
			XSetForeground(dpy,image_gc,classpixel[iclass]);
			XSetFunction(dpy,image_gc,GXcopy);
			XDrawLine(dpy,image_win,image_gc,oldX,oldY,xp,yp);
               	 	XFlush(dpy);
		}
	        oldX = xp;
	        oldY = yp;
            }
            if (Pressed3) {
                DrawRect (oldX, oldY, newX, newY);
                newX = xp;
                newY = yp;
                DrawRect (oldX, oldY, newX, newY);
                XFlush(dpy);
            }
            break;
        case ButtonRelease:
	    switch (event.xbutton.button) {
	    case Button1 :
	      Pressed1 = False;
	      break;
            case Button2 :
              Pressed2 = False;
      	      XChangeGC(dpy, image_gc, 
                         GCForeground | GCBackground | GCFunction , &gc_val);
	      break;
	    case Button3 :
              Pressed3 = False;
      	      XChangeGC(dpy, image_gc, 
                     GCForeground | GCBackground | GCFunction , &gc_val);
	      oldX=BETWEEN(oldX,0,ncols-1);
	      oldY=BETWEEN(oldY,0,nrows-1);
	      newX=BETWEEN(newX,0,ncols-1);
	      newY=BETWEEN(newY,0,nrows-1);
	      fprintf(stderr,"Rectangle: (%d,%d)	(%d,%d)\n",
		oldY,oldX,newY,newX);
	      if (button3_mode==Stretch) {
		nr=ABS(oldY-newY);
		nc=ABS(oldX-newX);
		r=MIN(oldY,newY);
		c=MIN(oldX,newX);
		histobe(bufptr,ncolors,r,c,nr,nc,Convex);
		for (i=0;i<256;i++) map[i]=(Byte)colors[map[i]].pixel;
		for (i=r;i<r+nr;i++) for (j=c;j<c+nc;j++) 
			buffer[i*ncols+j]=map[bufptr[i*ncols+j]];
		noread=True;
		f--;
		oldframe=False;
		pipefunction=True;
		original=False;
		
	      }
	      else if (button3_mode) {
	        nhd.orows=nhd.rows=ABS(newY-oldY)+1;
	        nhd.ocols=nhd.cols=ABS(newX-oldX)+1;
	        nhd.frow=nhd.fcol=0;
	        nhd.num_frame=1;
	        if (rgb) nhd.pixel_format=PFRGB;
		switch (button3_mode) {
		case Extract: 
	        if ((fp=fopen("xshow.hips","w")) == NULL) {
	  	  fprintf(stderr,"unable to create xshow.hips\n");
		  popened=True;
		  exit(1);
	        }
	        fwrite_header(fp,&nhd,"xshow.hips");
		break;
		case Enlarge:
		if (rgb) sprintf(str1,"htob | enlarge -s 3 | htorgb | xshow");
		else sprintf(str1,"enlarge -s 3 | xshow");
		if ((fp=popen(str1,"w")) == NULL) {
		 	fprintf(stderr,"Can't open pipe to enlarge\n"); 
			popened=True;
			exit(1);
		}
	        fwrite_header(fp,&nhd,"pipe");
		break;
		case Histo:
		if ((fp=popen("histo | disphist | xshow -title Histogram","w"))
				== NULL) {
		 	fprintf(stderr,"Can't open pipe to histo\n"); 
			popened=True;
			exit(1);
		}
	        fwrite_header(fp,&nhd,"pipe");
		break;
		} /* End switch */
	        subptr=cptr=bufptr+
		  (MIN(oldY,newY)*ncols+MIN(oldX,newX))*sizepix;
	        if (rgb) {
	        cptr++;
	        for (i=0;i<nhd.orows;i++) { 
	      	  for (j=0;j<nhd.ocols;j++) {
			if (fwrite(cptr+j*sizepix,1,3,fp) != 3) {
				fprintf(stderr,"error during write\n");
				exit(1);
			}
	      	  }
		  cptr+=ncols*sizepix;
	        }
	        } /* End rgb */
		/* Non rgb */
	        else { 
	        for (i=0;i<nhd.orows;i++) {
		  if (fwrite(cptr,nhd.sizepix,nhd.ocols,fp) != nhd.ocols) {
			fprintf(stderr,"error during write\n");
			exit(1);
		  }
		  cptr+=ncols*sizepix;
	        }
	        } /* End non rgb */
		if (button3_mode==Extract) fclose(fp); else fflush(fp);
	      } /* end Extract and enlarge */
              break;
	    }
	    break;
        case MappingNotify:
            XRefreshKeyboardMapping ((XMappingEvent*) &event);
	case KeyPress:
	    len=XLookupString((XKeyEvent*)&event,keybuffer,sizeof(keybuffer),NULL,NULL);
	    if (len != 1) break;
	    switch (keybuffer[0]) {
	    case 'a': case '>':
		if (f<nf-1 || wrap) {
			animate=True;
			oldframe=False;
			reverse=False;
		} 
		else {
			XBell(dpy,30);
			fprintf(stderr,"Last frame\n");
		}
		break;
	    case '<':
		if (!loadall) {
			XBell(dpy,30);
			fprintf(stderr,"Reverse only works with loadall\n");
		}
		else if (f>0 || wrap) {
			animate=True;
			oldframe=False;
			reverse=True;
		} 
		else {
			XBell(dpy,30);
			fprintf(stderr,"Last frame\n");
		}
		break;
	    case 'b': case 'B': case 'e': 
		if (pixfmt != PFBYTE) {
			XBell(dpy,30);
			fprintf(stderr,"Pixel format must be byte\n");
			break;
		}
		XDefineCursor(dpy,image_win,wait_cursor);
		XFlush(dpy);
		if (keybuffer[0]=='b')
			histobe(bufptr,ncolors,0,0,nrows,ncols,Convex);
		if (keybuffer[0]=='B')
			histobe(bufptr,ncolors,0,0,nrows,ncols,Concave);
		if (keybuffer[0]=='e')
			histobe(bufptr,ncolors,0,0,nrows,ncols,Equalize);
		for (i=0;i<256;i++) map[i]=(Byte)colors[map[i]].pixel;
		for (i=0;i<npix;i++) buffer[i]=map[bufptr[i]];
		noread=True;
		f--;
		oldframe=False;
		pipefunction=True;
		original=False;
		XDefineCursor(dpy,image_win,def_cursor);
	        break;
	    case 'c': case 'C':
		if (iclass==0) {
                  thd.image=trainptr=(Byte *) halloc(npix,sizeof(Byte));
		}
		if (trainptr!=NULL) {
		  if (iclass==Maxclass) {
			XBell(dpy,30);
			fprintf(stderr,"Max number of classes: %d\n",Maxclass);
		  }
		  else {
	            iclass++;
		    fprintf( stderr, "Press button 2 and drag to mark set %d\n", 			iclass );
		  }
		}
	        break;
	    case 'f': 
		XDefineCursor(dpy,image_win,wait_cursor);
		XFlush(dpy);
		XSetForeground(dpy,image_gc,classpixel[iclass]);
		XSetFunction(dpy,image_gc,GXcopy);
		if (trainptr!=NULL) {
			/*
			trainptr[yp*ncols+xp]=iclass;
			XDrawPoint(dpy,image_win,image_gc,xp,yp); 
			fill(trainptr,xp,yp);
			*/
			fillrow(trainptr,iclass,xp,xp,yp);
		}
		XDefineCursor(dpy,image_win,def_cursor);
	        break;
	    case 'F':
		XDefineCursor(dpy,image_win,wait_cursor);
		XFlush(dpy);
		XSetForeground(dpy,image_gc,classpixel[iclass]);
		XSetFunction(dpy,image_gc,GXcopy);
			/*
			trainptr[yp*ncols+xp]=iclass;
			XDrawPoint(dpy,image_win,image_gc,xp,yp); 
			fill(trainptr,xp,yp);
			*/
			fillrow1(bbuffer,trainptr,iclass,xp,xp,yp);
		XDefineCursor(dpy,image_win,def_cursor);
	        break;
	    case 'g': case 'G':
		XDefineCursor(dpy,image_win,wait_cursor);
		XFlush(dpy);
		hips_cmd("gopcopy",&hd,0);
		XDefineCursor(dpy,image_win,def_cursor);
	        break;
	    case 'h': 
		fprintf(stderr,"\n***  Header information  ***\n");
	    /*
		fprintf(stderr,"Original name     : %s",hd.orig_name);
		fprintf(stderr,"Sequence name     : %s",hd.seq_name);
		fprintf(stderr,"Number of frames  : %d\n",nf);
		fprintf(stderr,"Original date     : %s",hd.orig_date);
		fprintf(stderr,"Number of rows    : %d\n",nrows);
		fprintf(stderr,"Number of columns : %d\n",ncols);
		fprintf(stderr,"Pixel format      : %s\n",pform[pixfmt]);
		fprintf(stderr,"\nSequence history  : \n%s\n",hd.seq_history);
	    */
		hips_cmd("seeheader",&hd,1);
	        break;
	    case 'H': 
		hips_cmd("histo | disphist | xshow -title Histogram",&nhd,0);
	        break;
	    case 'm': case 'M':
		/*
        	blackcmap = XCreateColormap(dpy,root_win,rootvisual,AllocAll);
		blackcolor.red=blackcolor.green=blackcolor.blue=0;
		for (i=0;i<rootvisual->map_entries;i++) {
			blackcolor.pixel=(u_long)i;
        		XStoreColor(dpy, blackcmap, &blackcolor);
		}
		fprintf(stderr,"Black colormap\n");
                XInstallColormap(dpy, blackcmap);
		XSetWindowColormap(dpy,root_win,blackcmap);
		*/
	        break;
	    case '1': 
		button1_mode=(button1_mode+1)%2;
	    	sprintf(str1,"1) %s ",B1text[button1_mode]);
    	    	XDrawImageString(dpy,image_win,image_gc,
    			ncols+sidebarwidth+5,65,str1,strlen(str1));
	        break;
	    case '2': 
		button2_mode=(button2_mode+1)%2;
	    	sprintf(str1,"2) %s ",B2text[button2_mode]);
    	    	XDrawImageString(dpy,image_win,image_gc,
    			ncols+sidebarwidth+5,80,str1,strlen(str1));
	        break;
	    case '3': 
		button3_mode=(button3_mode+1)%N3modes;
	    	sprintf(str1,"3) %s ",B3text[button3_mode]);
    	    	XDrawImageString(dpy,image_win,image_gc,
    			ncols+sidebarwidth+5,95,str1,strlen(str1));
	        break;
	    case 'n': case ')': case ' ':
		if (f<nf-1 || wrap) {
			oldframe=False;
			reverse=False;
		} 
		else {
			XBell(dpy,30);
			fprintf(stderr,"Last frame\n");
		}
		break;
	    case '(':
		if (!loadall) {
			XBell(dpy,30);
			fprintf(stderr,"Reverse only works with loadall\n");
		}
		else if (f>0 || wrap) {
			oldframe=False;
			reverse=True;
		} 
		else {
			XBell(dpy,30);
			fprintf(stderr,"Last frame\n");
		}
		break;
	    case 'q': case 'Q':
                if (newmap_flag)
                    XInstallColormap(dpy, XDefaultColormap(dpy,screen));
                XDestroyWindow(dpy, image_win);
                XDestroyWindow(dpy, icon_win);
                XCloseDisplay(dpy);

		if (image_format != HIPS) pclose(inpipe); else fclose(inpipe);
		if (popened) pclose(fp);
		if (compress) {
			sprintf(str1,"compress %s",infilename);
			system(str1);
		}

		/*  
		 *  save training image 
		 */
                if (iclass>0 && trainptr!=NULL) {
	      		if ((tfp=fopen("trainingsets.hips","w")) == NULL) {
	  		  fprintf(stderr,"unable to create trainingsets.hips");
			  exit(1);
			}
	      		fwrite_header(tfp,&thd,"trainingsets.hips");
                  	if (fwrite(trainptr,sizeof(Byte),npix,tfp)<npix) {
			  fprintf(stderr,"error during write\n");
			  exit(1);
			}
			fclose(tfp);
                }

                exit(0);
	    case 'r': case 'R':
                if (iclass>0 && trainptr!=NULL) {
			free(trainptr);
			iclass=0;
                }
		if (original)
        		XPutImage(dpy, image_win, image_gc, image, 0, 0,
        			0, 0, ncols, nrows);
		else {
			oldframe=False;
			f--;
			noread=True;
		}
		nhd=hd;
		subptr=bufptr;
	        break;
	    case 's': 
		hips_cmd("framevar",&nhd,0);
	        break;
	    case 'S':
		animate=False;
		break;
	    case 't': 
		slicing=True;
		thrmin=128;
		thrmax=255;
	        break;
	    case 'T': 
		slicing=False;
	        break;
	    case 'x': case 'y':
		XDefineCursor(dpy,image_win,wait_cursor);
      		XChangeGC(dpy, image_gc, 
                     GCForeground | GCBackground | GCFunction , &gc_new);
		if (keybuffer[0]=='y') {
			XDrawLine(dpy,image_win,image_gc,xp,0,xp,nrows-1);
			sprintf(str1,"slice -v -p %d | xshow -title \"Vertical Slice\"",xp);
		}
		else {
			XDrawLine(dpy,image_win,image_gc,0,yp,ncols-1,yp);
			sprintf(str1,"slice -h -p %d | xshow -title \"Horizontal Slice\"",yp);
		}
		XFlush(dpy);
      	    	XChangeGC(dpy, image_gc, 
                     GCForeground | GCBackground | GCFunction , &gc_val);
		hips_cmd(str1,&nhd,0);
		XDefineCursor(dpy,image_win,def_cursor);
	        break;
	    case 'w': case 'W':
		XDefineCursor(dpy,image_win,wait_cursor);
		XFlush(dpy);
		fprintf(stderr,"waiting ..  ");
		wait(0);
		fprintf(stderr,"done\n");
		XDefineCursor(dpy,image_win,def_cursor);
	        break;
	    default:
		/*
		XDefineCursor(dpy,image_win,wait_cursor);
		XFlush(dpy);
		strcpy(hips_name,"xshow_");
		strncat(hips_name,keybuffer,1);
		hips_cmd(hips_name,&nhd,0);
		XDefineCursor(dpy,image_win,def_cursor);
	        break;
		*/
		fprintf(stderr,"Bad key event: %s\n",keybuffer);
	    }
            break;
        case LeaveNotify:
            if (newmap_flag && (xcrossing->mode != NotifyGrab))
                XInstallColormap(dpy, XDefaultColormap(dpy,screen));
            break;
        case EnterNotify:
            if (newmap_flag && (xcrossing->mode != NotifyUngrab))
                XInstallColormap(dpy, colormap);
            break;
        case ColormapNotify:
                /* Don't do anything for now */
            break;
        default:
             fprintf(stderr,"Bad X event.\n");
        }
    } /* End while  */
    if (wrap && f==nf-1 && !reverse) f=(-1);
    if (wrap && f==0 && reverse) f=nf;
    } /* Next frame */
/**************************** Next frame ***********************************/

    return(0);
}  /* end main */

int hips_cmd(cmd,hd,flag)
char	*cmd;
struct header *hd;
int	flag;
{
	int	npix;

	if ((inpipe=popen(cmd,"w")) == NULL) {
             	fprintf(stderr,"Can't open pipe to command: %s\n",cmd);
	}
	else {
    		npix=hd->rows*hd->cols;
    		fwrite_header(inpipe,hd,"pipe");
		if (!(flag & 0x0001)) {
    		    if (fwrite(hd->image,hd->sizepix,npix,inpipe) != npix) {
			fprintf(stderr,"error during pipe write\n");
			exit(1);
		    }
		}
	}
	fflush(inpipe);
}

Colormap GetColormap(colors, ncolors, newmap_flag, conmap_flag)
XColor  colors[];
int  ncolors;
Bool  *newmap_flag, *conmap_flag;
{
    int i,j;
    Colormap cmap, cmap2;
    XColor qcolor;
    u_long color2pix();
    
    if ((ncolors > XDisplayCells(dpy,screen)) && !rgb)    /* an X nonfeature */
        ncolors = XDisplayCells(dpy,screen);

    if (*newmap_flag) {
        cmap = XCreateColormap(dpy, root_win, visual, AllocAll);
        XStoreColors(dpy, cmap, colors, ncolors);
    } 
    else {
	*conmap_flag = True;
        cmap = XDefaultColormap(dpy, screen);
        for (i=0; i < ncolors; i++) {
            if (XAllocColor(dpy, cmap, &colors[i]) == 0) {
                fprintf(stderr,"Too many colors %d - new map made\n",i);
                cmap2 = XCopyColormapAndFree(dpy, cmap);
                *newmap_flag = True;
                for ( ; i < ncolors; i++)
                    XAllocColor(dpy, cmap2, &colors[i]);
                cmap = cmap2;
                break;
            }
        }
    }
    if (debug_flag)
      for (i=0; i < ncolors; i++) {
	 if (rgb) qcolor.pixel = (u_long)((i<<16)+(i<<8)+i);
         else qcolor.pixel = (u_long)i;
         XQueryColor(dpy, cmap, &qcolor);
         fprintf(stderr,"color[%3d]: pix %6x r= %5u g= %5u b= %5u\n",i,
            qcolor.pixel, qcolor.red, qcolor.green, qcolor.blue, qcolor.flags);
      }
    if (*newmap_flag) {
        whitepixel = color2pix(cmap, ncolors, 255, 255, 255);
        blackpixel = color2pix(cmap, ncolors, 0, 0, 0);
    }
    backgroundpixel = color2pix(cmap, ncolors, 0, 0, 255);
    drawpixel  = color2pix(cmap, ncolors, 255, 68, 68);
    for (i=0;i<=Maxclass;i++) classpixel[i]=color2pix(cmap,ncolors,
	classcolors[i][0],classcolors[i][1],classcolors[i][2]);
    return(cmap);
}


/* Find the pixel value in the colormap that is closest to a given color */

u_long color2pix(cmap, ncolors, red, green, blue)
Colormap cmap;
int ncolors, red, green, blue;
{
    int 	i, red2, blue2, green2;
    XColor 	qcolor;
    u_long 	value;
    long 	dist,least,minr,ming,minb,newr,newg,newb;
    
    if (rgb) {
    minr=ming=minb=256*256;
    for (i=0;i<ncolors;i++) {
        qcolor.pixel = (u_long)((i<<16)+(i<<8)+i);
        XQueryColor(dpy, cmap, &qcolor);
	if (ABS((int)qcolor.red-256*red)<minr) {
		minr=ABS((int)qcolor.red/256-red);
		newr=i;
	}
	if (ABS((int)qcolor.green-256*green)<ming) {
		ming=ABS((int)qcolor.green-256*green);
		newg=i;
	}
	if (ABS((int)qcolor.blue-256*blue)<minb) {
		minb=ABS((int)qcolor.blue-256*blue);
		newb=i;
	}
    }
    value = (u_long)((newr<<16)+(newg<<8)+newb);
    }
    else {
    least=1e5;
    for (i=0;i<ncolors;i++) {
        qcolor.pixel = (u_long)i;
        XQueryColor(dpy, cmap, &qcolor);
        red2 = (int)qcolor.red / 257;   
        green2 = (int)qcolor.green / 257;   
        blue2 = (int)qcolor.blue / 257; 
        dist = ((red2 - red) * (red2 - red)) +
               ((green2 - green) * (green2 - green)) +
               ((blue2 - blue) * (blue2 - blue));
        if (dist == 0)
            return(qcolor.pixel);
        else if (dist < least) {
            least = dist;
            value = qcolor.pixel;
        }
    }
    }
    return(value);
}

/* Draw a rectangle */

void DrawRect ( x1, y1, x2, y2)
int 		x1, y1, x2, y2;
{
   XDrawLine (dpy, image_win, image_gc, x1, y1, x2, y1);
   XDrawLine (dpy, image_win, image_gc, x2, y1, x2, y2);
   XDrawLine (dpy, image_win, image_gc, x1, y2, x2, y2);
   XDrawLine (dpy, image_win, image_gc, x1, y1, x1, y2);
}

void fillrow(pic,pval,xf1,xf2,yf)
Byte	*pic;
int	pval;
int	xf1,xf2,yf;
{
	Byte 	*pp;
	int	i,ist;
	Bool	fill_on;

	if (*(pic+yf*ncols+xf1) != pval)
		while (xf1>0 && *(pic+yf*ncols+xf1-1) != pval) xf1--;
	if (*(pic+yf*ncols+xf2) != pval)
		while (xf2<ncols-1 && *(pic+yf*ncols+xf2+1) != pval) xf2++;
	
	pp=pic+yf*ncols+xf1;
	fill_on = (*pp != pval);
	for (i=ist=xf1;i<=xf2+1;i++,pp++) {
		if (fill_on) {
		  if (i==xf2+1 || *pp == pval) {
			fill_on=False;
			if (yf>0) 
				fillrow(pic,pval,ist,i-1,yf-1);
			if (yf<nrows-1) 
				fillrow(pic,pval,ist,i-1,yf+1);
		  }
		}
		else {
		  if (*pp != pval) {
			fill_on=True;
			ist=i;
		  }
		}
		if (i<ncols) *pp= pval;
	}
	XDrawLine(dpy,image_win,image_gc,xf1,yf,xf2,yf);
}

void fillrow1(pic,trainptr,pval,xf1,xf2,yf)
Byte	*pic,*trainptr;
int	pval;
int	xf1,xf2,yf;
{
	Byte 	*pp;
	int	i,ist;
	Bool	fill_on;

	if (*(pic+yf*ncols+xf1) == 0)
		while (xf1>0 && *(pic+yf*ncols+xf1-1) == 0) xf1--;
	if (*(pic+yf*ncols+xf2) == 0)
		while (xf2<ncols-1 && *(pic+yf*ncols+xf2+1) == 0) xf2++;
	
	pp=pic+yf*ncols+xf1;
	fill_on = (*pp == 0);
	for (i=ist=xf1;i<=xf2+1;i++,pp++) {
		if (fill_on) {
		  if (i==xf2+1 || *pp != 0) {
			fill_on=False;
			if (yf>0) 
				fillrow1(pic,trainptr,pval,ist,i-1,yf-1);
			if (yf<nrows-1) 
				fillrow1(pic,trainptr,pval,ist,i-1,yf+1);
		  }
		}
		else {
		  if (*pp == 0) {
			fill_on=True;
			ist=i;
		  }
		}
		trainptr[yf*ncols+i] = pval;
		pic[yf*ncols+i] = 255;
	}
	XDrawLine(dpy,image_win,image_gc,xf1,yf,xf2,yf);
}

void fill(pic,xf,yf)
Byte	*pic;
int	xf,yf;
{
	Byte 	*p;

	XDrawPoint(dpy,image_win,image_gc,xf,yf); 
	p=pic+yf*ncols+xf;
	if (xf>0 && (*(p-1) != *p)) {
		*(p-1)= *p;
		fill(pic,xf-1,yf);
	}
	if (yf>0 && (*(p-ncols) != *p)) {
		*(p-ncols)= *p;
		fill(pic,xf,yf-1);
	}
	if (xf<ncols-1 && (*(p+1) != *p)) {
		*(p+1)= *p;
		fill(pic,xf+1,yf);
	}
	if (yf<nrows-1 && (*(p+ncols) != *p)) {
		*(p+ncols)= *p;
		fill(pic,xf,yf+1);
	}
}

void dline(ifr,x1,y1,x2,y2,n)
Byte	*ifr;
int	x1,y1,x2,y2,n;
{
	int	d,dx,dy;
	int	aincr,bincr,yincr;
	int	xx,yy;
	void	swap();
	int	flip=0;

	if (abs(x2-x1)<abs(y2-y1)) {
		swap(&x1,&y1);
		swap(&x2,&y2);
		flip++;
	}
		
	if (x1>x2) {
		swap(&x1,&x2);
		swap(&y1,&y2);
	}
	if (y2>y1) yincr =  1;
	else 	   yincr = -1;
	dx=x2-x1;
	dy=abs(y2-y1);
	d=2*dy-dx;
	aincr=2*(dy-dx);
	bincr=2*dy;
	xx=x1;
	yy=y1;
	if (flip) ifr[xx*ncols+yy]=n;
	else ifr[yy*ncols+xx]=n;
	for (xx=x1+1;xx<=x2;xx++) {
		if (d>=0) {
			yy+=yincr;
			d+=aincr;
		}
		else d+=bincr;
		if (flip) ifr[xx*ncols+yy]=n;
		else ifr[yy*ncols+xx]=n;
	}
	
}

void swap(pa,pb)
int 	*pa,*pb;
{
	int	t;

	t = *pa;
	*pa = *pb;
	*pb = t;
}


/* Get or make the colormap */
void makecolormap(colors,ncolors,lutselect)
XColor	colors[256];
int	ncolors,lutselect;
{
    int		i,j,mult;
    int		lutr,lutg,lutb;
    FILE	*lutfile;

    mult=256*(256/ncolors);
    for (i=0;i<ncolors;i++) {
    		colors[i].pixel = (u_long)i;
    		colors[i].flags = DoRed | DoGreen | DoBlue;
    }
    switch (lutselect) {
    case 0:
	if ((lutfile = fopen(lutfname,"r")) == NULL) {
		fprintf(stderr,"Can't open lutfile: %s\n",lutfname);
		exit(1);
	}
    	fprintf(stderr,"Scanning lutfile %s\n",lutfname);
    	for (i=0; i < ncolors; i++) {
    		for (j=0; j < 256/ncolors; j++) {
			fscanf(lutfile,"%d %d %d",&lutr,&lutg,&lutb);
			if (j == 0) {
	if (debug_flag)
    		fprintf(stderr,"%d  %d  %d  %d  %d\n",i,j,lutr,lutg,lutb);
    			colors[i].red=(u_short)(256*lutr);
			colors[i].green=(u_short)(256*lutg);
			colors[i].blue=(u_short)(256*lutb);
			}
		}
    	}
	fclose(lutfile);
	break;
    case Graylevel:
    	for (i=0; i < ncolors; i++) {
    		colors[i].red=colors[i].green=colors[i].blue=(u_short)mult*i+1;
    	}
	break;
    case Hot:
    	for (i=0; i < ncolors/4; i++) {
    		colors[i].red=(u_short)i*mult*4;
		colors[i].green=(u_short)i*mult*2;
		colors[i].blue=(u_short)0;
    	}
    	for (; i < ncolors/2; i++) {
    		colors[i].red=(u_short)65535;
		colors[i].green=(u_short)i*mult*2;
		colors[i].blue=(u_short)0;
    	}
    	for (; i < ncolors; i++) {
    		colors[i].red=(u_short)65535;
		colors[i].green=(u_short)65535;
		colors[i].blue=(u_short)(i-ncolors/2)*mult*2;
    	}
	break;
    case Rainbow:
    	for (i=0; i < (ncolors+6)/7; i++) {
    		colors[i].red=(u_short)(7*i*mult);
		colors[i].green=(u_short)0;
		colors[i].blue=(u_short)0;
    	}
    	for (; i < (2*ncolors+6)/7; i++) {
    		colors[i].red=(u_short)65535;
		colors[i].green=(u_short)((7*i-ncolors)*mult);
		colors[i].blue=(u_short)0;
    	}
    	for (; i < (3*ncolors+6)/7; i++) {
    		colors[i].red=(u_short)((3*ncolors-7*i)*mult);
		colors[i].green=(u_short)65535;
		colors[i].blue=(u_short)0;
    	}
    	for (; i < (4*ncolors+6)/7; i++) {
    		colors[i].red=(u_short)0;
		colors[i].green=(u_short)65535;
		colors[i].blue=(u_short)((7*i-3*ncolors)*mult);
    	}
    	for (; i < (5*ncolors+6)/7; i++) {
    		colors[i].red=(u_short)0;
		colors[i].green=(u_short)((5*ncolors-7*i)*mult);
		colors[i].blue=(u_short)65535;
    	}
    	for (; i < (6*ncolors+6)/7; i++) {
    		colors[i].red=(u_short)((7*i-5*ncolors)*mult);
		colors[i].green=(u_short)0;
		colors[i].blue=(u_short)65535;
    	}
    	for (; i < ncolors; i++) {
    		colors[i].red=(u_short)65535;
		colors[i].green=(u_short)((7*i-6*ncolors)*mult);
		colors[i].blue=(u_short)65535;
    	}
	break;
    case RGB:
        for (i=0; i < ncolors; i++) {
		colors[i].pixel = (u_long)((i<<16)+(i<<8)+i);
		colors[i].red=(u_short)(i<<8);
		colors[i].green=(u_short)(i<<8);
		colors[i].blue=(u_short)(i<<8);
		colors[i].flags = DoRed | DoGreen | DoBlue;	
	}
	break;
    default:
	fprintf(stderr,"Bad lut\n"); exit(1);
    }
}


void histobe(bufp,ncolors,r,c,nr,nc,func)
Byte	*bufp;
int	r,c,nr,nc,func;
{
	int	i,j,hist[256],nhist,mapval;
	double	suminp,cuminp[256],sumref,diff,difmin,temp,alpha,beta;
	Byte	*p;
	double 	cumref[256];

	if (func) {
	switch (func) {
	case Equalize: alpha=beta=1.0;
		break;
	case Convex  : alpha=beta=4.0;
		break;
	case Concave : alpha=beta=0.5;
		break;
	}

	sumref=0.0;
	for (i=0;i<256;i++) {
		temp=((double)i+0.5)/256.0;
		sumref+=pow(temp,alpha-1.0)*pow(1.0-temp,beta-1.0);
		cumref[i]=sumref;
	}
	for (i=0;i<256;i++) cumref[i]/=sumref;
	if (debug_flag) for (i=0;i<256;i++) 
		fprintf(stderr,"Ref %d : %f\n",i,cumref[i]);
	} /* if func */

	nhist=nr*nc;
	for (i=0;i<256;i++) hist[i]=0;
	p=bufp+r*ncols+c;
	for (i=0;i<nr;i++,p+=ncols) for (j=0;j<nc;j++) hist[*(p+j)]++;

	suminp=0.0;
	for (i=0;i<256;i++) {
		suminp+=(double)hist[i]/(double)nhist;
		cuminp[i]=suminp;
	}
	if (debug_flag) for (i=0;i<256;i++) 
		fprintf(stderr,"Inp %d : %f\n",i,cuminp[i]);

	for (i=0;i<256;i++) {
		difmin=1.0;
		for (j=0;j<256;j++) {
			diff=ABS(cuminp[i]-cumref[j]);
			if (diff<=difmin) {
				difmin=diff;
				mapval=j;
			}
		}
		map[i]=(mapval*ncolors)/256;
		if (debug_flag) fprintf(stderr,"Map %d : %d\n",i,mapval);
	}
	
}
