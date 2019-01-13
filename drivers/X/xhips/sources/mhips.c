/*
 * mhips - display a HIPS image or movie on a Mac under X11
 *
 * usage:	mhips [-d display] [-l lut-file] [-L framelabel]
 *			[-n] [-p] [-o] [-s framerate] [-b | -f]
 *
 * Mhips displays a single frame or movie under X11. It uses the X11
 *  parameters required for a Mac under OSX circa 2009 (i.e., a "TRUECOLOR"
 * X11 visual and a color byte order corresponding to HIPS color format ZRGB),
 * and thus may work on other machines for which TRUECOLOR is available and
 * this byte order works. The -d indicates the X11 server on which to
 * display. The -l flag provides a lookup table (for pseudocolor for
 * byte-formatted single-color images, and used separately on each
 * color channel for color images) in the same format as required
 * by addcmap. This lookup table will override a lookup table in the
 * image itself. If -l is not specified and the image has a lookup table,
 * it will be used. The -L flag provides a text label for the window;
 * otherwise the input filename will be used. If -n is specified, the movie
 * is not initially animated. -p indicates the movie should be shown
 * palindromically (first to last to first...). If -o is specified, the movie
 * will initially be animated just once. The -s switch specifies the frame
 * rate in frame/sec, which defaults to 15. The -b switch adds a sidebar to
 * the frame which will include (if it fits) a 256x10 colormap ramp and the
 * frame number. The -f switch puts the frame number on the image itself
 * instead.
 *
 * For movies (more than one frame), the window responds to the following
 * commands:
 *	>	move to next frame (if any)
 *	<	move to previous frame (if any)
 *	n	stop movie at the current frame
 *	p	switch to palindromic mode (and start movie if stopped)
 *	a	switch to forward-only (nonpalindromic) mode (and start movie
 *			if stopped)
 *	o	switch to play-once mode and start movie from frame 0
 *	r	switch to play-repeatedly mode (and start movie if stopped)
 *	q	quit
 *
 * to load:		cc -o mhips mhips.c -lX11 -lhipsh -lhips -lm
 *
 * Written based on xhips/xhipsc - msl - 1/12/09
 * perhaps add a switch for font size? ********
 */

/***********************************************************************
*  File:   xhipsc.c
*  Author: Patrick J. Flynn,from `xim.c' by Philip Thompson
*  $Date: $
*  $Revision: $
*  Purpose: To view a HIPS-formatted image.
*
*  I (PJF) consider this a derivative of `xim',and include the original
*  copyright notice.
*
*  Copyright (c) 1988  Philip R. Thompson
*                Computer Resource Laboratory (CRL)
*                Dept. of Architecture and Planning
*                M.I.T.,Rm 9-526
*                Cambridge,MA  02139
*   This  software and its documentation may be used,copied,modified,
*   and distributed for any purpose without fee,provided:
*       --  The above copyright notice appears in all copies.
*       --  This disclaimer appears in all source code copies.
*       --  The names of M.I.T. and the CRL are not used in advertising
*           or publicity pertaining to distribution of the software
*           without prior specific written permission from me or CRL.
*   I provide this software freely as a public service.  It is NOT a
*   commercial product,and therefore is not subject to an an implied
*   warranty of merchantability or fitness for a particular purpose.  I
*   provide it as is,without warranty. This software was not sponsored,
*   developed or connected with any grants,funds,salaries,etc.
*
*   This software is furnished  only on the basis that any party who
*   receives it indemnifies and holds harmless the parties who furnish
*   it against any claims,demands,or liabilities connected with using
*   it,furnishing it to others,or providing it to a third party.
*
*   Philip R. Thompson (phils@athena.mit.edu)
***********************************************************************/

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <hipl_format.h>
#include <math.h>
#include <sys/time.h>

#define BORDER	5	/* pixels around colormap and frame number */
#define	BARWIDTH 8	/* width of each color ramp */
#define	MAXUSLEEP 200000 /* maximum sleep time until check for events */

static Flag_Format flagfmt[] = {
	{"d",{LASTFLAG},1,{{PTBOOLEAN,"FALSE"},{PTSTRING,"","display"},
		LASTPARAMETER}},
	{"l",{LASTFLAG},1,{{PTBOOLEAN,"FALSE"},{PTFILENAME,"","lut-file"},
		LASTPARAMETER}},
	{"L",{LASTFLAG},1,{{PTBOOLEAN,"FALSE"},{PTSTRING,"","framelabel"},
		LASTPARAMETER}},
	{"n",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"p",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"o",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"s",{LASTFLAG},1,{{PTDOUBLE,"15","frame-rate"},LASTPARAMETER}},
	{"b",{"f",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"f",{"b",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,PFZRGB,LASTTYPE};		/* for MSBFirst servers */
int types2[] = {PFBYTE,PFBGRZ,LASTTYPE};	/* for LSBFirst servers */
int usleep();

int main(argc,argv)
int argc;
char **argv;

{
	struct header hd,hdp,hdg,hdb;
	int i,j,k,method,nc,nr,npix,count,count1,ovflow=0,screen,visualsMatched;
	int musecperf,f,fr,sizeoimg,ncw,numdig,maxcharwidth,maxascent;
	int maxdescent,maxcharrows,maxtextcols,firstbarcol,midbarcol,textx;
	int texty,len,sidebarcols,currf,needrefresh,incr,elapsed,timetogo;
	byte *img,**oimgs,r[256],g[256],b[256],*prlut,*pglut,*pblut,rl,gl,bl;
	byte *pr,*pg,*pb,*po,*ppr,*ppg,*ppb,pix,*sidebar;
	char *display_name,*framelabel,*clutFileName,keybuffer[10];;
	char digstring[10] = "";
	h_boolean dflag,Lflag,lflag,zrgbim_flag,bgrzim_flag,threecol,hascmap;
	h_boolean nflag,pflag,oflag,bflag,fflag,lsbfflag;
	double framerate;
	u_long blackpixel,whitepixel,gcmask;
	Display *dpy;
	Window root_win,image_win;
	Visual *visual;
	XEvent event;
	GC image_gc;
	XGCValues gc_val;
	XSetWindowAttributes xswa;
	XImage **images,*barimage;
	XSizeHints sizehints;
	XFontStruct *fontstruct;
	FILE *fp;
	Filename filename;
	XVisualInfo vTemplate,*visualList;
	char fontname[] = "8x13bold";
	struct timeval prevtime,currtime;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&dflag,&display_name,&lflag,&clutFileName,
		&Lflag,&framelabel,&nflag,&pflag,&oflag,&framerate,
		&bflag,&fflag,FFONE,&filename);
	if (!dflag)
		display_name = NULL;
	if (framerate <= 0)
		perr(HE_MSG,"frame rate must be positive");
	else {
		musecperf = (1000000/framerate);
		if (musecperf < 1)
			musecperf = 1;
	}

/* Open the display & set defaults */

	if ((dpy = XOpenDisplay(display_name)) == NULL) {
		fprintf(stderr,"%s: Can't open display '%s'\n",
			Progname,XDisplayName(display_name));
		exit(1);
	}
	lsbfflag = (ImageByteOrder(dpy) == LSBFirst);
	screen = XDefaultScreen(dpy);
	root_win = XDefaultRootWindow(dpy);

/* Read header and verify image file formats, allocate buffers */

	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd);
	hascmap = (findparam(&hd,"cmap") != NULLPAR);
	method = fset_conversion(&hd,&hdp,lsbfflag ? types2 : types,filename);
	nc = hd.ocols;
	nr = hd.orows;
	npix = nr*nc;
	img = hdp.image;
	if (hd.numcolor == 3)
		fr = hd.num_frame/3;
	else
		fr = hd.num_frame;
	zrgbim_flag = (hdp.pixel_format == PFZRGB);
	bgrzim_flag = (hdp.pixel_format == PFBGRZ);
	oimgs = (byte **) halloc(fr,sizeof(byte *));
	for (f=0;f<fr;f++)
		oimgs[f] = (byte *) halloc(npix,4*sizeof(byte));
	sizeoimg = npix*4;
	if (zrgbim_flag || bgrzim_flag) {
		if (method != METH_IDENT)
			free(hdp.image);
	}
	else {
		if (hd.numcolor == 3) {
			dup_headern(&hdp,&hdg);
			alloc_image(&hdg);
			dup_headern(&hdp,&hdb);
			alloc_image(&hdb);
		}
	}

/* get visual */

	vTemplate.screen = screen;
	vTemplate.class = TrueColor;
	vTemplate.depth = 24;
	visualList = XGetVisualInfo(dpy,VisualScreenMask | VisualClassMask |
		VisualDepthMask,&vTemplate,&visualsMatched);
	if (visualsMatched == 0) {
		fprintf(stderr,"%s: couldn't get a 24-bit TrueColor Visual\n",Progname);
		exit(1);
	}
	visual = visualList[0].visual;
	blackpixel = XBlackPixel(dpy,screen);
	whitepixel = XWhitePixel(dpy,screen);
	
/* get color lookup table */

	if (lflag) {
		readcmap(clutFileName,256,&count,r,g,b);
		prlut = r; pglut = g; pblut = b;
	}
	else if (hascmap) {
		count = 768;
		getparam(&hd,"cmap",PFBYTE,&count,&prlut);
		if (count % 3)
			perr(HE_MSG,"colormap length not a multiple of 3");
		count /= 3;
		pglut = prlut + count;
		pblut = pglut + count;
	}

/* read and process images */

	if (zrgbim_flag || bgrzim_flag) { /* ZRGB image: do LUT in place */
		for (f=0;f<fr;f++) {
			hdp.image = oimgs[f];
			if (method == METH_IDENT)
				hd.image = hdp.image;
			fread_imagec(fp,&hd,&hdp,method,0,filename);
		}
		if (lflag || hascmap) {
			count1 = count - 1;
			for (f=0;f<fr;f++) {
				ppr = oimgs[f];
				if (zrgbim_flag) {
					for (i=0;i<npix;i++) {
						ppr++;	/* skip Z byte */
						pix = *ppr;
						if (pix > count1) {
							*ppr++ = prlut[count1];
							ovflow++;
						}
						else {
							*ppr++ = prlut[pix];
						}
						pix = *ppr;
						if (pix > count1) {
							*ppr++ = pglut[count1];
							ovflow++;
						}
						else {
							*ppr++ = pglut[pix];
						}
						pix = *ppr;
						if (pix > count1) {
							*ppr++ = pblut[count1];
							ovflow++;
						}
						else {
							*ppr++ = pblut[pix];
						}
					}
				}
				else {
					for (i=0;i<npix;i++) {
						pix = *ppr;
						if (pix > count1) {
							*ppr++ = pblut[count1];
							ovflow++;
						}
						else {
							*ppr++ = pblut[pix];
						}
						pix = *ppr;
						if (pix > count1) {
							*ppr++ = pglut[count1];
							ovflow++;
						}
						else {
							*ppr++ = pglut[pix];
						}
						pix = *ppr;
						if (pix > count1) {
							*ppr++ = prlut[count1];
							ovflow++;
						}
						else {
							*ppr++ = prlut[pix];
						}
						ppr++;	/* skip Z byte */
					}
				}
			}
		}
	}
	else {		/* PFBYTE image, so copy to ZRGB or BGRZ output */
		if (hdp.numcolor == 3) {	/* 3 separate color planes */
			for (f=0;f<fr;f++) {
				po = oimgs[f];
				fread_imagec(fp,&hd,&hdp,method,0,filename);
				fread_imagec(fp,&hd,&hdg,method,0,filename);
				fread_imagec(fp,&hd,&hdb,method,0,filename);
				pr = hdp.image;
				pg = hdg.image;
				pb = hdb.image;
				if (lflag || hascmap) {
					h_applylut_B(pr,pr,nr,nc,nc,nc,count,
						prlut);
					ovflow += hips_hclip;
					h_applylut_B(pg,pg,nr,nc,nc,nc,count,
						pglut);
					ovflow += hips_hclip;
					h_applylut_B(pb,pb,nr,nc,nc,nc,count,
						pblut);
					ovflow += hips_hclip;
				}
				ppr = pr; ppg = pg; ppb = pb;
				if (lsbfflag) {
					for (i=0;i<npix;i++) {
						*po++ = *ppb++;
						*po++ = *ppg++;
						*po++ = *ppr++;
						*po++ = 0;
					}
				}
				else {
					for (i=0;i<npix;i++) {
						*po++ = 0;
						*po++ = *ppr++;
						*po++ = *ppg++;
						*po++ = *ppb++;
					}
				}
			}
		}
		else if (lflag || hascmap) {	/* PFBYTE pseudocolor */
			count1 = count - 1;
			for (f=0;f<fr;f++) {
				po = oimgs[f];
				fread_imagec(fp,&hd,&hdp,method,0,filename);
				ppr = img;
				if (lsbfflag) {
					for (i=0;i<npix;i++) {
						pix = *ppr++;
						if (pix > count1) {
							*po++ = pblut[count1];
							*po++ = pglut[count1];
							*po++ = prlut[count1];
							ovflow++;
						}
						else {
							*po++ = pblut[pix];
							*po++ = pglut[pix];
							*po++ = prlut[pix];
						}
						*po++ = 0;
					}
				}
				else {
					for (i=0;i<npix;i++) {
						pix = *ppr++;
						*po++ = 0;
						if (pix > count1) {
							*po++ = prlut[count1];
							*po++ = pglut[count1];
							*po++ = pblut[count1];
							ovflow++;
						}
						else {
							*po++ = prlut[pix];
							*po++ = pglut[pix];
							*po++ = pblut[pix];
						}
					}
				}
			}
		}
		else {					/* grayscale image */
			for (f=0;f<fr;f++) {
				po = oimgs[f];
				fread_imagec(fp,&hd,&hdp,method,0,filename);
				ppr = img;
				if (lsbfflag) {
					for (i=0;i<npix;i++) {
						*po++ = *ppr;
						*po++ = *ppr;
						*po++ = *ppr++;
						*po++ = 0;
					}
				}
				else {
					for (i=0;i<npix;i++) {
						*po++ = 0;
						*po++ = *ppr;
						*po++ = *ppr;
						*po++ = *ppr++;
					}
				}
			}
		}
	}
	if (ovflow)
		fprintf(stderr,"%s: %d lookup-table overflows\n",
			Progname,ovflow);
	images = (XImage **) hmalloc(fr*sizeof(XImage *));
	for (f=0;f<fr;f++)
		images[f] = XCreateImage(dpy,visual,(unsigned) 24,ZPixmap,0,
			(char *) (oimgs[f]),nc,nr,8,0);

/* Deal with sidebar and frame number */

	ncw = nc;
	if (bflag || fflag) {
		if ((fontstruct=XLoadQueryFont(dpy,fontname)) == NULL) {
			fprintf(stderr,"%s: can't load font %s, resetting -b/-f\n",
				Progname,fontname);
			fflag = bflag = 0;
		}
		else {
			numdig = ceil(log10(fr + 0.1));
			maxcharwidth = (fontstruct->max_bounds).width;
			maxascent = (fontstruct->max_bounds).ascent;
			maxdescent = (fontstruct->max_bounds).descent;
			maxcharrows = maxascent + maxdescent;
			maxtextcols = maxcharwidth*numdig;
		}
	}
	if (bflag) {
		if (nr < (256 + 3*BORDER + maxcharrows)) {
			fprintf(stderr,"%s: image too short to fit sidebar, resetting -b\n",
				Progname);
			bflag = 0;
		}
		else {
			if (maxtextcols > 4*BARWIDTH)
				sidebarcols = 2*BORDER + maxtextcols;
			else
				sidebarcols = 2*BORDER + 4*BARWIDTH;
			ncw = nc + sidebarcols;
			sidebar = (byte *) halloc(sidebarcols*nr,
				4*sizeof(byte));
			firstbarcol = (sidebarcols/2) - 2*BARWIDTH;
			for (i=0;i<256;i++) {
				po = sidebar + 4*(sidebarcols*(BORDER+i) + 
					firstbarcol);
				if (lflag || hascmap) {
					if (i < count) {
						rl = prlut[i];
						gl = pglut[i];
						bl = pblut[i];
					}
					else
						rl = gl = bl = 0;
				}
				else
					rl = gl = bl = i;
				if (lsbfflag) {
					for (j=0;j<BARWIDTH;j++) {
						*po++ = 0;
						*po++ = 0;
						*po++ = rl;
						*po++ = 0;
					}
					for (j=0;j<BARWIDTH;j++) {
						*po++ = 0;
						*po++ = gl;
						*po++ = 0;
						*po++ = 0;
					}
					for (j=0;j<BARWIDTH;j++) {
						*po++ = bl;
						*po++ = 0;
						*po++ = 0;
						*po++ = 0;
					}
					for (j=0;j<BARWIDTH;j++) {
						*po++ = bl;
						*po++ = gl;
						*po++ = rl;
						*po++ = 0;
					}
				}
				else {
					for (j=0;j<BARWIDTH;j++) {
						*po++ = 0;
						*po++ = rl;
						*po++ = 0;
						*po++ = 0;
					}
					for (j=0;j<BARWIDTH;j++) {
						*po++ = 0;
						*po++ = 0;
						*po++ = gl;
						*po++ = 0;
					}
					for (j=0;j<BARWIDTH;j++) {
						*po++ = 0;
						*po++ = 0;
						*po++ = 0;
						*po++ = bl;
					}
					for (j=0;j<BARWIDTH;j++) {
						*po++ = 0;
						*po++ = rl;
						*po++ = gl;
						*po++ = bl;
					}
				}
			}
			barimage = XCreateImage(dpy,visual,(unsigned) 24,
				ZPixmap,0,(char *) sidebar,sidebarcols,nr,8,0);
			midbarcol = nc + (sidebarcols/2);
			textx = midbarcol - (maxtextcols/2)
				+ (fontstruct->max_bounds).lbearing;
			texty = nr - (BORDER + maxdescent);
		}
	}
	if (fflag) {
		if (((maxcharrows + 2*BORDER) > nr) ||
		    ((maxtextcols + 2*BORDER) > nc)) {
			fprintf(stderr,"%s: image too small to fit frame #, resetting -f\n",
				Progname);
			fflag = 0;
		}
		else {
			textx = nc - (maxtextcols + BORDER)
				+ (fontstruct->max_bounds).lbearing;
			texty = nr - (BORDER + maxdescent);
		}
	}

/* Create window */

	xswa.event_mask = ExposureMask | ButtonPressMask;
	if (fr != 1)
		xswa.event_mask |= KeyPressMask;
	xswa.background_pixel = blackpixel;
	xswa.border_pixel = blackpixel;
	image_win = XCreateWindow(dpy,root_win,0,0,ncw,nr,5,24,
		InputOutput,visual,CWBackPixel | CWEventMask | CWBorderPixel,&xswa);
	xswa.border_pixel = 1;	/* MAC hardwired */
	XSetWindowBorder(dpy,image_win,xswa.border_pixel);

 /* set window manager hints */
	sizehints.flags = PSize | PMinSize | PMaxSize;
	sizehints.width = sizehints.min_width = sizehints.max_width = ncw;
	sizehints.height = sizehints.min_height = sizehints.max_height = nr;
	j = strlen(filename);
	k = 0;
	for (i=0;i<j;i++)
		if (filename[i]=='/')
			k = i+1;
	XSetStandardProperties(dpy,image_win,Lflag ? framelabel : (filename+k),
		NULL,None,argv,argc,&sizehints);
	gcmask = GCFunction | GCPlaneMask | GCForeground | GCBackground;
	gc_val.function = GXcopy;
	gc_val.plane_mask = AllPlanes;
	gc_val.foreground = whitepixel;
	gc_val.background = blackpixel;
	if (bflag || fflag) {
		gc_val.font = fontstruct->fid;
		gcmask |= GCFont;
	}
	image_gc = XCreateGC(dpy,image_win,gcmask,&gc_val);
	XMapWindow(dpy,image_win);	/* Map the image window. */

	if (fr == 1) {
		while (1) {	/* Set up a loop to maintain the image. */
			XNextEvent(dpy,&event);	/* Wait on input event. */
			switch ((int) event.type) {
			case Expose:
				XPutImage(dpy,image_win,image_gc,images[0],
					0,0,0,0,nc,nr);
				if (bflag) {
					XPutImage(dpy,image_win,image_gc,
						barimage,0,0,nc,0,
						sidebarcols,nr);
					sprintf(digstring,"%d",0);
					XDrawImageString(dpy,image_win,
						image_gc,textx,texty,digstring,
						strlen(digstring));
				}
				else if (fflag) {
					sprintf(digstring,"%d",0);
					XDrawImageString(dpy,image_win,
						image_gc,textx,texty,digstring,
						strlen(digstring));
				}
				break;
			case ButtonPress:
				switch ((int) event.xbutton.button) {
				case Button1:
					break;
				case Button2:
					break;
				case Button3:
					XDestroyWindow(dpy,image_win);
					XCloseDisplay(dpy);
					exit(0);
				}
				break;
			default:
				fprintf(stderr,"%s: bad X event\n",Progname);
				exit(-1);
			}
		}
	}

/* If arrive here, sequence has multiple frames */

	currf = 0;	/* next frame to be displayed */
	gettimeofday(&prevtime,NULL);	/* initialize timing */
	needrefresh = 1;
	incr = 1;

	while (1) {	/* main loop */
		while (XEventsQueued(dpy,QueuedAfterFlush)) {
			XNextEvent(dpy,&event);
			switch ((int) event.type) {
			case Expose:
				needrefresh = 1;
				break;
			case ButtonPress:
				switch ((int) event.xbutton.button) {
				case Button1:
					break;
				case Button2:
					break;
				case Button3:
					XDestroyWindow(dpy,image_win);
					XCloseDisplay(dpy);
					exit(0);
				}
				break;
			case MappingNotify:
				XRefreshKeyboardMapping((XMappingEvent*) &event);
				break;
			case KeyPress:
				len = XLookupString((XKeyEvent*) &event,
					keybuffer,sizeof(keybuffer),NULL,NULL);
				if (len != 1) break;
				switch (keybuffer[0]) {
				case '>':
					if (currf+1 < fr) {
						currf += 1;
						needrefresh = 1;
					}
					if (!nflag)
						gettimeofday(&prevtime,NULL);
					break;
				case '<':
					if (currf > 0) {
						currf -= 1;
						needrefresh = 1;
					}
					if (!nflag)
						gettimeofday(&prevtime,NULL);
					break;
				case 'n':
					nflag = 1;
					break;
				case 'p':
					pflag = 1;
					nflag = 0;
					incr = 1;
					currf = 0;
					gettimeofday(&prevtime,NULL);
					break;
				case 'a':
					pflag = nflag = 0;
					incr = 1;
					currf = 0;
					gettimeofday(&prevtime,NULL);
					break;
				case 'o':
					oflag = 1;
					nflag = 0;
					gettimeofday(&prevtime,NULL);
					currf = 0;
					incr = 1;
					needrefresh = 1;
					break;
				case 'r':
					oflag = 0;
					nflag = 0;
					gettimeofday(&prevtime,NULL);
					break;
				case 'q':
					XDestroyWindow(dpy,image_win);
					XCloseDisplay(dpy);
					exit(0);
				default:
					fprintf(stderr,"%s: unknown command '%s'\n",
						Progname,keybuffer);
				}
				break;
			default:
				fprintf(stderr,"%s: bad X event\n",Progname);
				exit(-1);
			}
		}
		if (needrefresh) {
			XPutImage(dpy,image_win,image_gc,images[currf],
				0,0,0,0,nc,nr);
			if (bflag) {
				XPutImage(dpy,image_win,image_gc,barimage,
					0,0,nc,0,sidebarcols,nr);
				sprintf(digstring,"%d",currf);
				XDrawImageString(dpy,image_win,image_gc,
					textx,texty,
					digstring,strlen(digstring));
			}
			else if (fflag) {
				sprintf(digstring,"%d",currf);
				XDrawImageString(dpy,image_win,
					image_gc,textx,texty,digstring,
					strlen(digstring));
			}
			XFlush(dpy);
			needrefresh = 0;
		}
		if (nflag)
			usleep(MAXUSLEEP);
		else {
			gettimeofday(&currtime,NULL);
			elapsed = (currtime.tv_sec - prevtime.tv_sec)*1000000
				+ (currtime.tv_usec - prevtime.tv_usec);
			timetogo = musecperf - elapsed;
			if (timetogo <= 0) {
				gettimeofday(&prevtime,NULL);
				if (incr == 1) {
					if (++currf < fr)
						needrefresh = 1;
					else if (pflag) {
						currf -= 2;
						incr = -1;
						needrefresh = 1;
					}
					else if (oflag) {
						--currf;
						nflag = 1;
					}
					else {
						currf = 0;
						needrefresh = 1;
					}
				}
				else {
					if (--currf >= 0)
						needrefresh = 1;
					else if (oflag) {
						nflag = 1;
						currf = 0;
					}
					else {
						currf = 1;
						incr = 1;
						needrefresh = 1;
					}
				}
			}
			else
				usleep(MAXUSLEEP > timetogo ? timetogo :
					MAXUSLEEP);
		}
	}
}
