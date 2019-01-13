/*
 *  xvhips: XView-based HIPS-format image displayer
 *
 *  based on sunv, originally written by Pat Flynn, 1/88
 *  (flynn@cpsvax.cps.msu.edu)
 *  modified by Mike Landy - 4/9/88 (added binary images, speedups)
 *  HIPS 2 - msl - 1/8/91
 *  converted to XView - msl - 3/28/92
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
 * and only 64 colormap entries are required as for -r.  For the full
 * 256-color colormaps (-f, -g, -c, or a colormap stored in a header when -r
 * isn't specified), the default behavior is to use the full colormap
 * unaltered.  This results in the scrollbars and window frame having the
 * appearance governed by that colormap even when the cursor is over the
 * scrollbars (often causing them to disappear).  If the user specifies -C,
 * then the control colors (needed for the scrollbars and window frame) are
 * altered to be identical to those required for the scrollbars.  Any pixels
 * in the displayed image with those values are replaced with values which
 * result in a displayed color which is as close as possible to the desired
 * color.
 *      The window defaults to be just large enough to contain the entire
 * image.  The image may be resized by the user in the usual way.  The user
 * may also specify that the window be of a different size with the -S flag.
 * This specifies the amount of the image that will fit in the initially
 * displayed canvas (and number of columns defaults to be the same as the
 * number of rows).
 *	By default the frame label contains the name of the file being
 * displayed.  The label may also be specified with the -l switch.
 *
 *  Compile this with:
 *
 * cc -DULORIG -O -I/usr/include -I/usr/openwin/include -o xvhips \
 *	xvhips.c -L/usr/local/lib -L/usr/openwin/lib -lhipsh -lhips -lm \
 *	-lxview -lolgx -lX11
 *
 *  Calling format:
 *    cat HIPSfile | xvhips [-s n] [-f | -r | -c mapfile |
 *		-g gammar gammag gammab | -sg gammar gammag gammab]
 *		[-C] [-S viewrows [viewcols]] [-l framelabel]
 *
 *  n is the height/width of the screen rectangle corresponding to
 *    a single pixel in the input file (equivalent to the HIPS command
 *    "enlarge n".  n defaults to 1, meaning
 *    a MxM image file will appear in an MxM window on the screen
 *    (not counting the title bar, etc).
 *
 *  On one-bit displays, input images which are not bit-packed will be
 *  halftoned.
 */

#include <stdio.h>
#include <xview/xview.h>
#include <xview/canvas.h>
#include <xview/panel.h>
#include <xview/seln.h>
#include <xview/cms.h>
#include <xview/svrimage.h>
#include <xview/screen.h>
#include <xview/icon.h>
#include <xview/scrollbar.h>
#include <X11/Xutil.h>
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
	0x0000,0x0000,0x0000,0x0000, 0x0000,0x0000,0x0000,0x0000,
	0x0000,0x8000,0x0000,0x0000, 0x0000,0x4000,0x0000,0x0000,
	0x0000,0x2000,0x0000,0x0000, 0x0000,0x1000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000, 0x0000,0x0000,0x0000,0x0000,
	0x0000,0x03FF,0xFFFF,0xFFF0, 0x0000,0x0200,0x0000,0x0010,
	0x0000,0x0200,0x0000,0x0010, 0x0000,0x0200,0x0000,0x0010,
	0x0000,0x0200,0x0000,0x0010, 0x0000,0x0200,0x0000,0x0010,
	0x0000,0x0200,0x0000,0x0010, 0x0000,0x0200,0x0000,0x0010,
	0x0000,0x0200,0x0000,0x0010, 0x0000,0x0200,0x0000,0x0010,
	0x0000,0x0200,0x0000,0x0010, 0x0000,0x0200,0x0000,0x0010,
	0x0000,0x0200,0x0000,0x0010, 0x0000,0x0200,0x0000,0x0010,
	0x0000,0x0200,0x0000,0x0010, 0x0000,0x0200,0x0000,0x0010,
	0x0000,0x0289,0x1224,0xE1D0, 0x0000,0x0289,0x1224,0x9210,
	0x0000,0x0251,0x1224,0x9210, 0x0000,0x0220,0xA3E4,0xE190,
	0x0000,0x0250,0xA224,0x8050, 0x0000,0x0288,0x4224,0x8050,
	0x0000,0x0288,0x4224,0x8390, 0x0000,0x0200,0x0000,0x0010,
	0x0000,0x0200,0x0000,0x0010, 0x0000,0x0200,0x0000,0x0010,
	0x0000,0x0200,0x0000,0x0010, 0x0000,0x0200,0x0000,0x0010,
	0x0000,0x0200,0x0000,0x0010, 0x0000,0x0200,0x0000,0x0010,
	0x0000,0x0200,0x0000,0x0010, 0x0000,0x0200,0x0000,0x0010,
	0x0000,0x0200,0x0000,0x0010, 0x0000,0x0200,0x0000,0x0010,
	0x0000,0x0200,0x0000,0x0010, 0x0000,0x0200,0x0000,0x0010,
	0x0000,0x0200,0x0000,0x0010, 0x0000,0x03FF,0xFFFF,0xFFF0,
	0x0000,0x0000,0x0000,0x0000, 0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000, 0x0000,0x0000,0x0000,0x0000
};

Icon icon;
Server_image icon_image;
Display *dpy;
Visual *visual;
Frame base_frame;
Canvas canvas;
Scrollbar scrollh,scrollv;
Xv_window canvpaintwin;
Xv_Screen xvscreen;
Window canvasxwin;
GC imagegc;
XColor colors[256];
int pixelsize,blackpixel,whitepixel;
h_boolean ibinary;
h_boolean fflag,rflag,cflag,gflag,sgflag,Cflag,lflag;
static void displayit();
int nrows,ncols,screen;
int ourpackedtype;
struct header *chd,hd1,hd2,hd3,hd4;
h_boolean color2=FALSE,color64=TRUE,color256=FALSE;
unsigned long *substtable;
byte *bytelut;

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
    {"l",{LASTFLAG},1,{{PTBOOLEAN,"FALSE"},{PTSTRING,"","framelabel"},
	LASTPARAMETER}},
    LASTFLAG};
int types[] = {PFBYTE,LASTTYPE};


main(argc,argv)

int argc;
char **argv;

{
	byte red[256],green[256],blue[256],c,*pcmap;
	char frlabel[256];
	int method,numcol,nc,*savepval,nsubst,r1,g1,b1,k,viewrows,viewcols;
	float dist,newdist,rdiff,gdiff,bdiff;
	h_boolean greydisplay;
	register int i,j;
	char tmpname[100],*framelabel;
	Filename filename,mapfile;
	FILE *fp;
	double gammar,gammag,gammab;
	XVisualInfo vistemp,*vislist;
	Cms cms;
	Colormap defcolormap;
	XColor cdef,*savexc;
	Xv_cmsdata *cmsdata;
	int vismatch;

	Progname = strsave(*argv);
	xv_init(XV_INIT_ARGC_PTR_ARGV,&argc,argv,NULL);
	parseargs(argc,argv,flagfmt,&fflag,&rflag,&cflag,&mapfile,&gflag,
		&gammar,&gammag,&gammab,&sgflag,&gammar,&gammag,&gammab,
		&pixelsize,&Cflag,&viewrows,&viewcols,&lflag,&framelabel,
		FFONE,&filename);
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
	gammar = 1/gammar;
	gammag = 1/gammag;
	gammab = 1/gammab;
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd1,filename);
	if (cflag)
		readcmap(mapfile,256,&numcol,red,green,blue);
	ibinary = (hd1.pixel_format == PFMSBF || hd1.pixel_format == PFLSBF);
	nrows=hd1.orows; ncols=hd1.ocols;
	j = strlen(filename);
	k = 0;
	for (i=0;i<j;i++)
		if (filename[i] == '/')
			k = i+1;
	strcpy(frlabel,"xvhips: ");
	strcat(frlabel,filename+k);
	base_frame=xv_create(NULL,FRAME,XV_LABEL,lflag ? framelabel : frlabel,
		NULL);
	dpy = (Display *) xv_get(base_frame,XV_DISPLAY);
	xvscreen = (Xv_Screen) xv_get(base_frame,XV_SCREEN);
	screen = DefaultScreen(dpy);
	ourpackedtype = PFLSBF;		/* Using bitmaps which assume LSBF */
	imagegc = DefaultGC(dpy,screen);
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
	color2 = ibinary || !greydisplay;
	color256 = (fflag || gflag || cflag ||
		(findparam(&hd1,"cmap") != NULLPAR && !rflag)) && !color2;
	color64 = !(color2 || color256);
	if (Cflag && !color256) {
		perr(HE_IMSG,
			"-C is only relevant for 256-cell colormaps, ignored");
		Cflag = FALSE;
	}
	XFree((caddr_t)vislist);
	icon_image = (Server_image) xv_create(NULL,SERVER_IMAGE,
		XV_WIDTH,64,
		XV_HEIGHT,64,
		SERVER_IMAGE_BITS,HIPSicon_image,NULL);
	icon = (Icon) xv_get(base_frame,FRAME_ICON);
	xv_set(icon,ICON_IMAGE,icon_image,NULL);
	xv_set(base_frame,FRAME_ICON,icon,NULL);
	canvas=xv_create(base_frame,CANVAS,
		WIN_DYNAMIC_VISUAL,color256,
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
	if (viewrows <= 0) {
		viewcols = pixelsize*ncols;
		viewrows = pixelsize*nrows;
	}
	xv_set(canvas,
		CANVAS_WIDTH,ncols*pixelsize,
		CANVAS_HEIGHT,nrows*pixelsize,
		XV_WIDTH,viewcols+xv_get(scrollv,XV_WIDTH)+2,
		XV_HEIGHT,viewrows+xv_get(scrollh,XV_HEIGHT)+2,NULL);
	window_fit(base_frame);
	canvpaintwin = canvas_paint_window(canvas);
	canvasxwin = (XID) xv_get(canvpaintwin,XV_XID);
	if (fflag || cflag || gflag || sgflag) {
	    if (!greydisplay) {
		perr(HE_IMSG,
			"pseudocolor for this monitor is not available");
	    }
	    else if (ibinary) {
		perr(HE_IMSG,
			"color map for binary images is not meaningful");
	    }
	}
	hd2.imdealloc = hd3.imdealloc = hd4.imdealloc = FALSE;
	clearroi(&hd1);
	if (hd1.pixel_format == ourpackedtype && pixelsize == 1) {
		fread_image(fp,&hd1,0,filename);
		chd = &hd1;
	}
	else {
		method = fset_conversion(&hd1,&hd2,types,filename);
		chd = &hd2;
		fread_imagec(fp,&hd1,&hd2,method,0,filename);
		if (color64)
			h_shift_b(chd,chd,-2);
		if (pixelsize > 1) {
			dup_headern(chd,&hd3);
			setsize(&hd3,nrows*pixelsize,ncols*pixelsize);
			alloc_image(&hd3);
			h_enlarge_b(chd,&hd3,pixelsize,pixelsize);
			chd = &hd3;
		}
		if (color2) {
			if (!ibinary)
				h_halftone(chd,chd);
			dup_headern(chd,&hd4);
			setformat(&hd4,ourpackedtype);
			alloc_image(&hd4);
			if (ourpackedtype == PFMSBF)
				h_btomp(chd,&hd4);
			else
				h_btolp(chd,&hd4);
			chd = &hd4;
		}
	}
#ifdef LLORIG
	h_invert(chd,chd);
#endif
	if (chd != &hd1)
		free_image(&hd1);
	if (chd != &hd2)
		free_image(&hd2);
	if (chd != &hd3)
		free_image(&hd3);
	/* if (chd != &hd4)
		free_image(&hd4); Not necessary */
		
	if (color2) {
		blackpixel = BlackPixel(dpy,screen);
		whitepixel = WhitePixel(dpy,screen);
	}
	else if (fflag) {
		for (i=0;i<256;i++) {
			colors[i].pixel = i;
			colors[i].red = colors[i].green = colors[i].blue = i<<8;
			colors[i].flags = DoRed | DoGreen | DoBlue;
		}
		strcpy(tmpname,"xvhipsf");
		numcol = 256;
	}
	else if (cflag) {
		for (i=0;i<numcol;i++) {
			colors[i].pixel = i;
			colors[i].red = red[i]<<8;
			colors[i].green = green[i]<<8;
			colors[i].blue = blue[i]<<8;
			colors[i].flags = DoRed | DoGreen | DoBlue;
		}
		strcpy(tmpname,mapfile);
		numcol = 256;
	}
	else if (gflag) {
		sprintf(tmpname,"gammar%fg%fb%f",gammar,gammag,gammab);
		colors[0].pixel = 0;
		colors[0].red = colors[0].green = colors[0].blue = 0;
		colors[0].flags = DoRed | DoGreen | DoBlue;
		for (i=1;i<256;i++) {
			colors[i].pixel = i;
			colors[i].red =
				((int) (255.*pow((double) i/255.,gammar) + .5))
				<< 8;
			colors[i].green =
				((int) (255.*pow((double) i/255.,gammag) + .5))
				<< 8;
			colors[i].blue =
				((int) (255.*pow((double) i/255.,gammab) + .5))
				<< 8;
			colors[i].flags = DoRed | DoGreen | DoBlue;
		}
		numcol = 256;
	}
	else if (sgflag) {
		sprintf(tmpname,"sgammar%fg%fb%f",gammar,gammag,gammab);
		colors[0].pixel = 0;
		colors[0].red = colors[0].green = colors[0].blue = 0;
		colors[0].flags = DoRed | DoGreen | DoBlue;
		for (i=1;i<64;i++) {
			colors[i].pixel = i;
			colors[i].red =
				((int) (255.*pow((double) i/63.,gammar) + .5))
				<< 8;
			colors[i].green =
				((int) (255.*pow((double) i/63.,gammag) + .5))
				<< 8;
			colors[i].blue =
				((int) (255.*pow((double) i/63.,gammab) + .5))
				<< 8;
			colors[i].flags = DoRed | DoGreen | DoBlue;
		}
		numcol = 64;
	}
	else if (rflag || findparam(&hd1,"cmap") == NULLPAR) {
		for (i=0;i<64;i++) {
			colors[i].pixel = i;
			colors[i].red = colors[i].green = colors[i].blue =
				i<<10;
			colors[i].flags = DoRed | DoGreen | DoBlue;
		}
		strcpy(tmpname,"xvhips");
		numcol = 64;
	}
	else {
		numcol = 768;
		getparam(&hd1,"cmap",PFBYTE,&numcol,&pcmap);
		if (numcol % 3)
			perr(HE_MSG,"colormap length not a multiple of 3");
		sprintf(tmpname,"xvhips%d",getpid());
		numcol /= 3;
		for (i=0;i<numcol;i++) {
			colors[i].pixel = i;
			colors[i].red = (*(pcmap+i))<<8;
			colors[i].green = (*(pcmap+numcol+i))<<8;
			colors[i].blue = (*(pcmap+2*numcol+i))<<8;
			colors[i].flags = DoRed | DoGreen | DoBlue;
		}
		numcol = 256;
	}
	if (color64 || (color256 && !Cflag)) {
		cms = (Cms) xv_find(xvscreen,CMS,
			CMS_NAME,tmpname,
			XV_AUTO_CREATE,FALSE,NULL);
		if (cms == NULL) {
			cms = (Cms) xv_create(screen,CMS,
				CMS_NAME,tmpname,
				CMS_TYPE,color64 ? XV_STATIC_CMS :
					XV_DYNAMIC_CMS,
				CMS_SIZE,numcol,
				CMS_X_COLORS,colors,NULL);
		}
		xv_set(canvas,WIN_CMS,cms,NULL);
	}
	else if (color256 && Cflag) {
		strcat(tmpname,"_C");
		defcolormap = DefaultColormap(dpy,screen);
		cmsdata = (Xv_cmsdata *) xv_get(scrollv,WIN_CMS_DATA); 
		nc = cmsdata->size;
		savexc = (XColor *) halloc(nc,sizeof(XColor));
		savepval = (int *) halloc(nc,sizeof(int));
		for (i=0;i<nc;i++) { 
			cdef.red = cmsdata->red[i]<<8; 
			cdef.green = cmsdata->green[i]<<8;
			cdef.blue = cmsdata->blue[i]<<8; 
			XAllocColor(dpy,defcolormap,&cdef);
			savexc[i] = colors[cdef.pixel];
			savepval[i] = cdef.pixel;
			colors[cdef.pixel] = cdef;
			colors[cdef.pixel].flags = DoRed | DoGreen | DoBlue;
		}
		bytelut = (byte *) halloc(numcol,sizeof(byte));
		for (i=0;i<numcol;i++)
			bytelut[i] = i;
		nsubst = 0;
		for (i=0;i<nc;i++) {
			dist = -1;
			r1 = savexc[i].red;
			g1 = savexc[i].green;
			b1 = savexc[i].blue;
			for (j=0;j<numcol;j++) {
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
			if (savepval[i] != k) {
				nsubst++;
				bytelut[savepval[i]] = k;
			}
		}
		if (nsubst)
			h_applylut(chd,chd,numcol,bytelut);
		cms = (Cms) xv_find(xvscreen,CMS,
			CMS_NAME,tmpname,
			XV_AUTO_CREATE,FALSE,NULL);
		if (cms == NULL) {
			cms = (Cms) xv_create(screen,CMS,
				CMS_NAME,tmpname,
				CMS_TYPE,XV_DYNAMIC_CMS,
				CMS_SIZE,numcol,
				CMS_X_COLORS,colors,NULL);
		}
		xv_set(canvas,WIN_CMS,cms,NULL); /*  (on frame???) */
	}
	if (color64) {
		substtable = (unsigned long *) xv_get(cms,CMS_INDEX_TABLE);
		bytelut = (byte *) halloc(64,sizeof(byte));
		for (i=0;i<64;i++)
			bytelut[i] = substtable[i];
		h_applylut(chd,chd,64,bytelut);
	}
	window_main_loop(base_frame);
	return(0);
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

	if (color2) {
		XSetForeground(dpy,imagegc,whitepixel);
		XSetBackground(dpy,imagegc,blackpixel);
		pxmap = XCreatePixmapFromBitmapData(dpy,win,
			(char *) chd->image,chd->ocols,chd->orows,1,0,1);
		XCopyPlane(dpy,pxmap,win,imagegc,0,0,chd->ocols,chd->orows,
			0,0,1);
		XFreePixmap(dpy,pxmap);
	}
	else {
		image = XCreateImage(dpy,visual,8,ZPixmap,0, 
			(char *) chd->image,chd->ocols,chd->orows,8,0);
		XPutImage(dpy,win,imagegc,image,0,0,0,0,chd->ocols,chd->orows);
		image->data = (char *) 0;
		XDestroyImage(image);
	}
}
