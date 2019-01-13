/*
 *  sunv: Suntools-based HIPS-format image displayer
 *
 *  Written by Pat Flynn, 1/88
 *  (flynn@cpsvax.cps.msu.edu)
 *  This software carries no warranty of any kind.
 *  modified by Mike Landy - 4/9/88 (added binary images, speedups)
 *  HIPS 2 - msl - 1/8/91
 *
 *  Permission to freely copy this software is hereby granted
 *  as long as this notice is kept intact.
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
 *    cc -O -o sunv sunv.c -lsuntool -lsunwindow -lpixrect -lhipsh -lhips -lm
 *
 *  Calling format:
 *    cat HIPSfile | sunv [-s n] [-f | -r | -c mapfile |
 *		-g gammar gammag gammab | -sg gammar gammag gammab]
 *
 *  n is the height/width of the screen rectangle corresponding to
 *    a single pixel in the input file (equivalent to the HIPS command
 *    "enlarge n".  n defaults to 1, meaning
 *    a MxM image file will appear in an MxM window on the screen
 *    (not counting the title bar, etc).
 *
 *  On one-bit displays, input images which are not bit-packed will be
 *  halftoned.
 *
 *  Sunv also accepts some of the suntools arguments (-W[pPliILtT]).
 */

#include <stdio.h>
#include <suntool/sunview.h>
#include <suntool/canvas.h>
#include <suntool/panel.h>
#include <suntool/seln.h>
#include <pixrect/pixrect_hs.h>
#include <hipl_format.h>
#include <math.h>

static short HIPSicon_image[]={
/* Format_version=1, Width=64, Height=64, Depth=1, Valid_bits_per_item=16
 */
	0xFFFF,0xFFFF,0xFFFF,0xFFFF,0x8000,0x0000,0x0000,0x0001,
	0x8000,0x0000,0x0000,0x0001,0x8000,0x0000,0x0000,0x0001,
	0x83FF,0x0000,0x0000,0x0001,0x83E0,0x0000,0x0000,0x0001,
	0x83E0,0x0000,0x0000,0x0001,0x83E0,0x0000,0x0000,0x0001,
	0x83E0,0x0000,0x0000,0x0001,0x8200,0x0000,0x0000,0x0001,
	0x8208,0x0000,0x0000,0x0001,0x8204,0x0000,0x0000,0x0001,
	0x8202,0x0000,0x0000,0x0001,0x8201,0x0000,0x0000,0x0001,
	0x8000,0x0000,0x0000,0x0001,0x8000,0x0000,0x0000,0x0001,
	0x8000,0x2000,0x0000,0x0001,0x8000,0x1000,0x0000,0x0001,
	0x8000,0x0800,0x0000,0x0001,0x8000,0x0400,0x0000,0x0001,
	0x8000,0x0000,0x0000,0x0001,0x8000,0x0000,0x0000,0x0001,
	0x8000,0x0080,0x0000,0x0001,0x8000,0x0040,0x0000,0x0001,
	0x8000,0x003F,0xFFFF,0xFFF9,0x8000,0x0020,0x0000,0x0009,
	0x8000,0x0020,0x0000,0x0009,0x8000,0x0020,0x0000,0x0009,
	0x8000,0x0020,0x0000,0x0009,0x8000,0x0020,0x0000,0x0009,
	0x8000,0x0020,0x0000,0x0009,0x8000,0x0020,0x0000,0x0009,
	0x8000,0x0020,0x0000,0x0009,0x8000,0x0020,0x0000,0x0009,
	0x8000,0x0020,0x0000,0x0009,0x8000,0x0020,0x0000,0x0009,
	0x8000,0x0020,0x0000,0x0009,0x8000,0x0020,0x0000,0x0009,
	0x8000,0x0021,0xC448,0x9109,0x8000,0x0022,0x244C,0x9109,
	0x8000,0x0022,0x044C,0x9109,0x8000,0x0021,0x044A,0x8A09,
	0x8000,0x0020,0xC44A,0x8A09,0x8000,0x0020,0x2449,0x8A09,
	0x8000,0x0022,0x2449,0x8409,0x8000,0x0022,0x2448,0x8409,
	0x8000,0x0021,0xC388,0x8409,0x8000,0x0020,0x0000,0x0009,
	0x8000,0x0020,0x0000,0x0009,0x8000,0x0020,0x0000,0x0009,
	0x8000,0x0020,0x0000,0x0009,0x8000,0x0020,0x0000,0x0009,
	0x8000,0x0020,0x0000,0x0009,0x8000,0x0020,0x0000,0x0009,
	0x8000,0x0020,0x0000,0x0009,0x8000,0x0020,0x0000,0x0009,
	0x8000,0x0020,0x0000,0x0009,0x8000,0x0020,0x0000,0x0009,
	0x8000,0x0020,0x0000,0x0009,0x8000,0x0020,0x0000,0x0009,
	0x8000,0x003F,0xFFFF,0xFFF9,0x8000,0x0000,0x0000,0x0001,
	0x8000,0x0000,0x0000,0x0001,0xFFFF,0xFFFF,0xFFFF,0xFFFF
};

DEFINE_ICON_FROM_IMAGE(icon,HIPSicon_image);

Frame base_frame;
Canvas canvas;
int pixelsize;
h_boolean ibinary = FALSE;
h_boolean fflag,rflag,cflag,gflag,sgflag;
static void displayit();
int nrows,ncols;
struct pixrect *mpr;
 
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


main(argc,argv)

int argc;
char **argv;

{
	Pixwin *canpixwin;
	byte red[256],green[256],blue[256],c,*pcmap;
	int method,numcol;
	h_boolean greydisplay;
	h_boolean obinary;
	register int i;
	struct header *chd,hd1,hd2,hd3,hd4,hd5;
	char *dummy,tmpname[100];
	Filename filename,mapfile;
	FILE *fp;
	double gammar,gammag,gammab;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&fflag,&rflag,&cflag,&mapfile,&gflag,
		&gammar,&gammag,&gammab,&sgflag,&gammar,&gammag,&gammab,
		&pixelsize,&dummy,&dummy,&dummy,&dummy,&dummy,&dummy,&dummy,
		&dummy,&dummy,FFONE,&filename);
	if (gammag < 0)
		gammag = gammar;
	if (gammab < 0)
		gammab = gammag;
	gammar = 1/gammar;
	gammag = 1/gammag;
	gammab = 1/gammab;
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd1,filename);
	if (cflag)
		readcmap(mapfile,256,&numcol,red,green,blue);
	if (hd1.pixel_format == PFMSBF || hd1.pixel_format == PFLSBF)
		ibinary = TRUE;
	nrows=hd1.orows; ncols=hd1.ocols;
	base_frame=window_create(NULL,FRAME,FRAME_LABEL,"sunv",
		WIN_WIDTH,9+ncols*pixelsize,
		WIN_HEIGHT,22+nrows*pixelsize,
		FRAME_ARGS,argc,argv,WIN_ERROR_MSG,
		"can't create window, sunv must be run under suntools",0);
	window_set(base_frame,FRAME_ICON,&icon,0);
	canvas=window_create(base_frame,CANVAS,
		CANVAS_WIDTH,ncols*pixelsize,
		CANVAS_HEIGHT,nrows*pixelsize,
		WIN_WIDTH,pixelsize*ncols,
		WIN_HEIGHT,pixelsize*nrows,
		CANVAS_REPAINT_PROC,displayit,0);
	canpixwin=canvas_pixwin(canvas);
	greydisplay =
		((canpixwin -> pw_pixrect -> pr_depth) < 8) ? FALSE : TRUE;
	obinary = (ibinary || (!greydisplay)) ? TRUE : FALSE;
	if (fflag || cflag || gflag || sgflag) {
	    if (!greydisplay) {
		perr(HE_IMSG,
			"color map for binary monitors is not meaningful");
	    }
	    else if (ibinary) {
		perr(HE_IMSG,
			"color map for binary images is not meaningful");
	    }
	}
	hd2.imdealloc = hd3.imdealloc = hd4.imdealloc = hd5.imdealloc = FALSE;
	clearroi(&hd1);
	if (hd1.pixel_format == ourpackedtype && pixelsize == 1) {
		fread_image(fp,&hd1,0,filename);
		chd = &hd1;
	}
	else {
		method = fset_conversion(&hd1,&hd2,types,filename);
		chd = &hd2;
		fread_imagec(fp,&hd1,&hd2,method,0,filename);
		if (!fflag && !obinary && !cflag && !gflag &&
		    (rflag || sgflag || findparam(&hd1,"cmap") == NULLPAR))
			h_shift_b(chd,chd,-2);
		if (pixelsize > 1) {
			dup_headern(chd,&hd3);
			if ((!obinary) && ((ncols * pixelsize) & 01)) {
				setsize(&hd3,(nrows * pixelsize),
					(ncols * pixelsize) + 1);
				setroi(&hd3,0,0,hd3.orows,hd3.ocols - 1);
			}
			else
				setsize(&hd3,(nrows * pixelsize),
					(ncols * pixelsize));
			alloc_image(&hd3);
			h_enlarge_b(chd,&hd3,pixelsize,pixelsize);
			chd = &hd3;
		}
		if (obinary) {
			if (!ibinary)
				h_halftone(chd,chd);
			dup_headern(chd,&hd4);
			setformat(&hd4,ourpackedtype);
			alloc_image(&hd4);
#ifdef MSBFVERSION
			h_btomp(chd,&hd4);
#else
			h_btolp(chd,&hd4);
#endif
			chd = &hd4;
		}
	}
	if (obinary) {
		if ((((chd->ocols)+7)/8) & 01) {
			dup_headern(chd,&hd5);
			setsize(&hd5,hd5.orows,hd5.ocols+8);
			setroi(&hd5,0,0,hd5.orows,hd5.ocols-8);
			alloc_image(&hd5);
#ifdef MSBFVERSION
			h_copy_mp(chd,&hd5);
#else
			h_copy_lp(chd,&hd5);
#endif
			chd = &hd5;
		}
	}
	else {
		if ((chd->ocols) & 01) {
			dup_headern(chd,&hd5);
			setsize(&hd5,hd5.orows,hd5.ocols+1);
			setroi(&hd5,0,0,hd5.orows,hd5.ocols-1);
			alloc_image(&hd5);
			h_copy_b(chd,&hd5);
			chd = &hd5;
		}
	}
	mpr = mem_point(ncols*pixelsize,nrows*pixelsize,obinary?1:8,
		(short *) chd->image);
	if (chd != &hd1)
		free_image(&hd1);
	if (chd != &hd2)
		free_image(&hd2);
	if (chd != &hd3)
		free_image(&hd3);
	if (chd != &hd4)
		free_image(&hd4);
	/* if (chd != &hd5)
		free_image(&hd5); Not necessary for sunv */
	window_set(canvas,CANVAS_DEPTH,obinary ? 1 : 8,0);
		
	if (obinary) {
		red[0]=green[0]=blue[0]=0;
		red[1]=green[1]=blue[1]=255;
		pw_setcmsname(canpixwin,"sunvb");
		pw_putcolormap(canpixwin,0,2,red,green,blue);
	}
	else if (fflag) {
		for(i=0;i<256;i++)
			red[i]=green[i]=blue[i]=i;
		pw_setcmsname(canpixwin,"sunvf");
		pw_putcolormap(canpixwin,0,256,red,green,blue);
	}
	else if (cflag) {
		pw_setcmsname(canpixwin,mapfile);
		pw_putcolormap(canpixwin,0,numcol,red,green,blue);
	}
	else if (gflag) {
		sprintf(tmpname,"gammar%fg%fb%f",gammar,gammag,gammab);
		pw_setcmsname(canpixwin,tmpname);
		red[0] = green[0] = blue[0] = 0;
		for (i=1;i<256;i++) {
			red[i] = 255.*pow((double) i/255.,gammar) + .5;
			green[i] = 255.*pow((double) i/255.,gammag) + .5;
			blue[i] = 255.*pow((double) i/255.,gammab) + .5;
		}
		pw_putcolormap(canpixwin,0,256,red,green,blue);
	}
	else if (sgflag) {
		sprintf(tmpname,"sgammar%fg%fb%f",gammar,gammag,gammab);
		pw_setcmsname(canpixwin,tmpname);
		red[0] = green[0] = blue[0] = 0;
		for (i=1;i<64;i++) {
			red[i] = 255.*pow((double) i/63.,gammar) + .5;
			green[i] = 255.*pow((double) i/63.,gammag) + .5;
			blue[i] = 255.*pow((double) i/63.,gammab) + .5;
		}
		pw_putcolormap(canpixwin,0,64,red,green,blue);
	}
	else if (rflag || findparam(&hd1,"cmap") == NULLPAR) {
		for(i=0;i<64;i++)
			red[i]=green[i]=blue[i]=i*4;
		pw_setcmsname(canpixwin,"sunv");
		pw_putcolormap(canpixwin,0,64,red,green,blue);
	}
	else {
		numcol = 768;
		getparam(&hd1,"cmap",PFBYTE,&numcol,&pcmap);
		if (numcol % 3)
			perr(HE_MSG,"colormap length not a multiple of 3");
		sprintf(tmpname,"sunv%d",getpid());
		pw_setcmsname(canpixwin,tmpname);
		pw_putcolormap(canpixwin,0,numcol/3,pcmap,
			pcmap+(numcol/3),pcmap+(2*numcol/3));
	}
	window_main_loop(base_frame);
	return(0);
}


static void displayit(canv,pw,repaint_area)

Canvas canv;
Pixwin *pw;
Rectlist *repaint_area;

{
	pw_rop(pw,0,0,ncols*pixelsize,nrows*pixelsize,PIX_SRC,mpr,0,0);
}
