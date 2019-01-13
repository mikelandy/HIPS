/*	getx . c
#
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

This software is copyright (C) by the Lawrence Berkeley Laboratory.
Permission is granted to reproduce this software for non-commercial
purposes provided that this notice is left intact.

It is acknowledged that the U.S. Government has rights to this software
under Contract DE-AC03-765F00098 between the U.S.  Department of Energy
and the University of California.

This software is provided as a professional and academic contribution
for joint exchange. Thus, it is experimental, and is provided ``as is'',
with no warranties of any kind whatsoever, no support, no promise of
updates, or printed documentation. By using this software, you
acknowledge that the Lawrence Berkeley Laboratory and Regents of the
University of California shall have no liability with respect to the
infringement of other copyrights by any part of this software.

For further information about this notice, contact William Johnston,
Bld. 50B, Rm. 2239, Lawrence Berkeley Laboratory, Berkeley, CA, 94720.
(wejohnston@lbl.gov)

For further information about this software, contact:
	Jin Guojun
	Bld. 50B, Rm. 2275, Lawrence Berkeley Laboratory, Berkeley, CA, 94720.
	g_jin@lbl.gov

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%	Elastic Control Panel (Editing), Multi-Thread and X Extension
%
% Author:	Jin Guojun - LBL	Mon. Apr 1, 1991
*/

#include "tuner.h"
#ifdef	_DEBUG_
#include "uthread.h"
extern	int	u_thr_mode;
#endif

#ifndef	ExTITLE
#define	ExTITLE	"socket"
#endif
#ifndef	DEF_TITLE
#define	DEF_TITLE	"Standard Input"
#endif
#ifndef	KEEP_WIN_STILL
#define	KEEP_WIN_STILL	8
#endif
#ifndef	XLOGO
#define	XLOGO	"./getxe.logo"
#endif

/* must be able to fit 3 * log2 (MAXIMUM_LEVELS) bits in a 32 bit word */

#ifndef	DEFAULT_LEVELS
#define DEFAULT_LEVELS	240
#endif
#define MAXIMUM_LEVELS	1024
#define	isame	!strcmp

void	update_pic();

/* Make SysV macros map to BSD macros */
#ifndef _tolower
#define _tolower tolower
#define _toupper toupper
#endif

/*	Global variables	*/

static	char 	*Help_message_getx =	"\
If start with -T, press CTRL + any Button to show PANEL\n\
multiple frame HIPS or FITS image will not be forked\n\n\
button:      LEFT            MIDDLE           RIGHT\n\n\
alone :  value+coord      toggle-zoom\n\
+SHIFT:    Magnify        zoom-shift          Shrink\n\n\
+CTRL :	moving objects\n\
The # key (1-9) for magnifing scale\n\
for multiple frame image from standard input, use '+' key\
to go forward, and '-' key to go backward one frame\n\
press 'q' key in an image window to quit that image\n\
For more information: see man tuner",
	**infnames, *display_name, *window_geometry, *vis_type,
	*title_str = DEF_TITLE, *svport;

static	bool	STS, fforward, nofork, regular_rle, still_win,
		vflag, aflag, bin_flag, mono_flag, iflag, clear_root;

extern	int	num_images, row_dpy_mode, shared_cmap, stingy_flag;

static struct {
	CONST_DECL char	*string;
	int		type;
}  visual_type_table[] =	{
	{ "staticgray",	StaticGray	},
	{ "grayscale",	GrayScale	},
	{ "staticgrey",	StaticGray	},
	{ "greyscale",	GrayScale	},
	{ "staticcolor", StaticColor	},
	{ "pseudocolor", PseudoColor	},
	{ "truecolor",	TrueColor	},
	{ "directcolor", DirectColor	}
	};

#define	basic_alist1	\
	&window_geometry, &aflag,	\
	&bin_flag, &mono_flag,		\
	&clear_root, &display_gamma,	\
	&row_dpy_mode,	\
	&movie, &movie, &movie_frams_sec,	\
	&specified_levels, &OsameI
#ifdef	_DEBUG_
#define	basic_alist2	\
	&shared_cmap, &regular_rle, &vroot_dpy,	\
	&stingy_flag, &STS,	\
	&title_str, &verbose, &DEBUGANY, &u_thr_mode
#else
#define	basic_alist2	\
	&shared_cmap, &regular_rle, &vroot_dpy,	\
	&stingy_flag, &STS,	\
	&title_str, &verbose, &DEBUGANY
#endif

#ifdef	FREE_PORT_CHIOCE
#define	basic_alist	basic_alist1, &svport, basic_alist2
#else
#define	basic_alist	basic_alist1, basic_alist2
#endif

arg_fmt_list_string	arg_list[] =	{
	{"-d", "%s", No, 1, 1, "display name <host[:x.y]>"},
	{"-f", "%b", True, 1, 0, "run on foreground"},
	{"-i", "%1 %D", 1.0, 2, 1, "gamma of image. (default %.1f)"},
	{"-I", "%2 %D", 1.0, 2, 1,
		"gamma: specify gamma of display image was computed for"},
	{"-N", "%d", MAX_IMAGE_PTRS, 1, 1, "Number of images (%.f)"},
	{"-T", "%b", True, 1, 0, "use Tuner panel"},
	{"-v", "%1 %s", No, 2, 1,
		"vis_type: specify Visual type to be used\n\
			(string or int, 0 to 5)"},

	{"-=", "%s", (int) NULL, 1, 1, "window_geometry:\n\
			Specify window geometry (but min size set by file)."},
	{"-a", "%b", True, 1, 0, "Do not dither"},
	{"-b", "%b", True, 1, 0, "binary (black and white)"},
	{"-B", "%b", True, 1, 0, "monochrome"},
	{"-cls", "%d", 1, 1, 0, "clear ROOT window"},
	{"-g", "%D", 1.5, 1, 1,	"gamma for display. (default %.1f)"},
	{"-j", "%b", True, 1, 0, "display images line by line"},
	{"-M", "%d", 64, 1, 0, "Max movie frames reside in mem (default %.f)"},
	{"-m", "%1 %d", 30, 2, 0, "frames/sec:	run movie [set speed (%.f)]"},
	{"-n", "%d", DEFAULT_LEVELS, 1, 1,
		"number of colors to dither colormaps (%.f)"},
	{"-O", "%b", False, 1, 0,
		"Output 3-channels RLE. Otherwise, output same as input"},
#ifdef	FREE_PORT_CHIOCE
	{"-P", "%s", No, 1, 1, "Port # for Ext X server"},
#endif
	{"-p", "%b", False, 1, 0, "perform no-shared colormap"},
	{"-r", "%b", True, 1, 0, "regular RLE coordinate"},
	{"-root", "%d", 1, 1, 0, "display in ROOT window [# > 1 = stay perm."},
	{"-S", "%b", True, 1, 0, "Slow mode for remote display"},
	{"-s", "%b", True, 1, 0,
		"slower start, but get fixed color for panel"},
	{"-t", "%s", (int)NULL, 1, 0, "title on image windows"},
	{"-V", "%b", True, 1, 0, "Verbose some information; High level debug"},
	{"-D", "%d", 1, 1, 0, "Debug level: turn on kernel debug"},
#ifdef	_DEBUG_
	{"-BT", "%d", U_BOUND_THREAD, 1, 0, "bound thread [or other] to a lwp"},
#endif
	{"input:	Any Image file. Use stdin if none specified", "0",
		0, 0, 0, "\nEnd of options"},	NULL	},
	*b_arg_list = arg_list + 7;
/*
%	getx will also read picture comments from the input file to
%		determine the image gamma.  These are
%
%	image_gamma=	gamma of image (equivalent to -i)
%	display_gamma=	gamma of display image was computed for.
%
%	Any command line arguments override the values in the file.
%
% Outputs:	Puts image(s) on screen.
% Assumptions:	Input file is converted to RLE format.
*/

void
init_img_flag(img)
Image*	img;
{
	img->binary_img = bin_flag;
	img->mono_img = mono_flag | bin_flag;
	if (img->mono_img)	{
		img->color_dpy = False;
		img->dpy_channels = 1;
	}
	img->dither_img = !aflag;
	if (!img->lvls)
		img->lvls = specified_levels;
}



main(argc, argv)
int	argc;
char*	*argv;
{
int	nf, n_malloced, using_stdin,
	movie=0, movie_frams_sec=0,
	visual_type= -1;
double	image_gamma=0.0;	/* default gamma for image */
extern	void	DestroyPanel();

#	include	<signal.h>
#ifdef	IGNORE_SIGPOLL
	signal(SIGPOLL, SIG_IGN);
#endif
	signal(SIGINT, DestroyPanel);

Progname = *argv;	OsameI = shared_cmap = True;

    if ((nf=parse_argus(&infnames, argc, argv, arg_list,
		&display_name, &nofork,
		&iflag, &image_gamma,
		&iflag, &image_gamma,
		&n_malloced, &tuner_flag,
		&vflag, &vis_type,
		basic_alist)) < 0)
#ifdef	IGNORE_PARSE_ERROR
	if (nf < 0)	nf = 0;
#else
	exit(1);
#endif
	if (!shared_cmap)
		precision = 32;	/* better for CFM_SCF	*/
	if (vroot_dpy > 1)	/* tricky?	*/
		n_malloced = vroot_dpy;
	if (n_malloced < KEEP_WIN_STILL)
		still_win = n_malloced;

	if (!clear_root && (using_stdin = !nf))	{	/* no input	*/
	    io_test(fileno(stdin),
		if (!freopen(XLOGO, "rb", stdin)) prgmerr('f', XLOGO);
		/* if no one is set, then set to use non-shared colormap and
			convert input to RGB output as possible	for XES	*/
		if (OsameI & shared_cmap)	OsameI = shared_cmap = 0);
		nf++;
	}
	/*	open the display	*/

    if (display_name == NULL || *display_name == '\0')
	display_name = getenv("DISPLAY");
    if ((Dpy=XOpenDisplay(display_name)) == NULL)
	prgmerr(1, "Cant open display %s\n", display_name);

    /* Work around bug in X11/NeWS server colormap allocation. */
    if (isame("X11/NeWS - Sun Microsystems Inc.", ServerVendor(Dpy)) &&
		VendorRelease(Dpy) == 2000)
	no_color_ref_counts = True;

#if	defined	HIPS2_HF | defined HIPS_IMAGE
	hipserrprt = hipserrlev = HEL_SEVERE;
#endif

	if (specified_levels < 1)
		specified_levels = DEFAULT_LEVELS; /* default starting point */
	if (specified_levels > MAXIMUM_LEVELS)
		specified_levels = MAXIMUM_LEVELS;
	pic = (Image**)ZALLOC(n_malloced, sizeof(*pic), "img_info");

    if (vflag) {
	if (isdigit(vis_type[0]))
		visual_type = atoi(vis_type);
	else {
	register char	*ptr = vis_type;
	register int	i;
	visual_type = 9999;

	    do	if (isupper(*ptr)) *ptr = _tolower(*ptr);
	    while (*ptr);

	    for (i=0; i < COUNT_OF(visual_type_table); i++)
		if (isame(visual_type_table[i].string, vis_type)) {
		    visual_type = visual_type_table[i].type;
		    break;
		}
	}

	if (visual_type < 0 || visual_type > 5) {
		message("Bad visual type %s, ignoring request\n", vis_type);
	    visual_type = -1;
	}
    }

	cmn_hd.visual_class = visual_type;
	init_img_info(&cmn_hd, Dpy, RLE, -1);
	find_appropriate_visual(&cmn_hd);	/* for global set up */
	cmn_hd.pixmap_failed = stingy_flag && !movie;
	if (vroot_dpy | clear_root)	get_vroot_win(&cmn_hd);
	if (clear_root)	{
		KillRetainedProperty(&cmn_hd);
		ResetWindowBackground(cmn_hd.win, clear_root);
		goto	close_xd;
	}
	if (iflag==2 && image_gamma)	cmn_hd.gamma = 1. / image_gamma;
	else if (iflag==1)	cmn_hd.gamma = image_gamma;
	if (STS)	start_fast = False;
	CreateColorTuner(Dpy, Monitor[1].dpy, &cmn_hd, tuner_flag, vroot_dpy, STS);
	if (movie)	{
		VCTEntry >>= 1,
		display_gamma += 1;
		INIT_PERFORM_TIMING();
	}
	regular_rle |= movie;

    {	register int	i;
    for (i=0; i < nf; i++)	{
	if (!using_stdin && strcmp(infnames[i], "-")) {
		cmn_hd.name = infnames[i];
		cmn_hd.IN_FP = zreopen(cmn_hd.name, No, NULL);	/* No clean for /tmp/ztmp */
		if (!cmn_hd.IN_FP)	{
			prgmerr(nf==1, cmn_hd.name);
			continue;
		}
	} else	cmn_hd.name = title_str;
	if (tuner_flag && i)	{
		if (LoadImage(cmn_hd.IN_FP, pic+num_images, cmn_hd.name)>0)
			num_images++;
	} else	nofork |= LoadGXImage(&num_images, &cmn_hd, &pic, movie,
				window_geometry, still_win) && using_stdin;
    }
    }
	if (!num_images)
		prgmerr(1, "no image can be displayed");
	if (num_images == 1)	{	/* no movie anyway	*/
		regular_rle &= ~movie;
		movie = 0;
	} else	{
	float	tt = pic[0]->frames / TOTAL_CONSUMED("\nload movie in");
		message("Rate = %.2f f/s (%.2f MBps)\n",
			tt, tt * pic[0]->width * pic[0]->height * 9.5367e-7);
	}
    if (!tuner_flag)	{
	InfoWin = CreatePanel(10, 10, 512, 256, "Information", Monitor, NULL);
	MGray = GetGray(Dpy, Monitor[0].cmap, ncolors, 64);
	XSetWindowBackground(Dpy, InfoWin->win, MGray);
    }

#ifndef	X_Extender
	if (!vroot_dpy || tuner_flag)
#endif
#ifdef unix
	if (!nofork) {
	    if (fork()==0) {
		setpgrp( /* Set process group to avoid signals under sh. */
#ifndef SYS_V_SETPGRP
			0, getpid()
#endif
					);
	/*	close(2);	we need stderr for info.	*/
#ifndef	NO_POPEN
		setsid();
		if (!tuner_flag)
#endif
		close(1);
	/*	close(0);	freopen stdin may cause problem	*/
		update_pic(movie, movie_frams_sec,
				MAX(num_images, n_malloced));
	    }	else	exit(0);	/* never shutdown here	*/
	} else
#endif
	update_pic(movie, movie_frams_sec, MAX(num_images,n_malloced));
close_xd:
	if (vroot_dpy > 1)	RetainWindowProperty(&cmn_hd);
	DestroyPanel(NULL);
}



/* define what to do on mouse buttons */
static int button_action[3][3] = {
	ACTION_MAGNIFY,		ACTION_PIXEL_INFO,		ACTION_OBJECT,
	ACTION_PAN,		ACTION_SWITCH_MAG_MODE,		ACTION_TUNER,
	ACTION_UNMAGNIFY,	ACTION_SUB_WINDOW,		ACTION_TUNER
	};

/* equivalent to movie mouse buttons on flip_action[3][2] =	{
	ACTION_MOVIE_FORWARD,	ACTION_CYCLE,
	ACTION_MOVIE_STEP,	ACTION_MOVIE_SPEED,
	ACTION_MOVIE_BACKWARD,	ACTION_CYCLE_TO_AND_FRO
	};
*/

static void	/* use global *pic[]	*/
update_pic(movie, movie_frams_sec, maxRLE_images)
x_bool	movie;
{
XEvent event;
int	i, action, cur_img=0, found_event,
	sb=0, Tuner_act=-1, ExSERVERS=1;
register image_information*	img=NULL;

#define	n	num_images
#define	flip_frame	cur_img

	/* variables to use for movie mode */
x_bool	flip_forward = True;
Window	dead_window;
int	movie_udelay=0, bounce_mode;

	if (tuner_flag)
		histinfo.histp = pic[0]->hist,	DrawPanel();
#ifdef	X_Extender
	{
#include <sys/socket.h>
	char	pn[32];
	int	ret_port = x_extender_init(svport, False, 0, SOCK_STREAM);

	sprintf(pn, "tcp port # = %d", ret_port);
	img = pic[0];
	XDrawString(img->dpy,img->window,img->gc,
			img->w-144, img->h-8, pn, strlen(pn));
	if (ret_port < 0)	goto	quit_server;
	img = NULL;
#ifdef	GETXE_RTP
	if (x_extender_init(0, True, 28000, SOCK_DGRAM) > 0)
		ExSERVERS++;
#endif
	}
#endif

    while (n) {
	image_information **	rpic=pic;

#ifdef	X_Extender
	while (!XEventsQueued(rpic[0]->dpy, QueuedAfterFlush))
	for (i=0; i < ExSERVERS; i++)	{
	register FILE*	sfp = x_extender(i);
	    if (sfp)	{
# ifdef	SOCKET_DEBUG
		while (!feof(sfp))
			putc(getc(sfp), stderr);
# else
		title_str = ExTITLE;
		if (i = get_arg_list(&infnames))
			parse_argus(NULL, i, infnames, b_arg_list, basic_alist);
		if (clear_root)	{
			CopyToRootWindow(&cmn_hd, NULL, Yes);
			KillRetainedProperty(&cmn_hd);
			ResetWindowBackground(cmn_hd.win, clear_root);
			if (clear_root & ~3)	return;
			clear_root = vroot_dpy = 0;
		}
		cmn_hd.name = title_str;
		cmn_hd.IN_FP = sfp;
		if (tuner_flag)	{
		    if ((i=LoadImage(sfp, rpic+n, cmn_hd.name)) == FileLoad) {
			if (img)	img->active = False;
			img = rpic[n++];
			img->active++;
		    }
#	ifdef	GETXE_TEST_UDP
		    else if (!i)	goto	cleanudp;
#	endif
		} else if (LoadGXImage(&num_images, &cmn_hd, &pic,
				movie, window_geometry, still_win) >= 0)
			rpic = pic;
#	ifdef	GETXE_TEST_UDP
		else	{
cleanudp:	char	sbuf[256];	/* for testing udp only	*/
			while (read(fileno(sfp), sbuf, sizeof(sbuf)) > 0);
		}
#	endif
# endif
	    }
	}
#endif
	XNextEvent(rpic[0]->dpy, &event);

	if (movie)	{	/* all windows are the same! */
		movie_udelay =
		    (movie_frams_sec) ? 1000000 / movie_frams_sec : 0;
		img = rpic[flip_frame];
	} else {
		cur_img = WhichImage(event.xany.window, rpic, n);
		if (cur_img < 0)
			img = NULL;
		else	{
			for (i=n; i--;)
			    if (rpic[i]->active)
				rpic[i]->active = 0;
			img = rpic[cur_img];
			img->event = &event;
			img->active++;
		}
	}
	if (!img && event.type!=Expose && event.type!=EnterNotify)
		continue;

	switch (event.type) {

	case ButtonPress:
	    i = event.xbutton.button - Button1;
	    if (i < 0 || i > COUNT_OF(button_action))
		action = ACTION_DEFAULT;
	    else{
		register int j=event.xbutton.state & (ShiftMask | ControlMask);
		if (j > 2)	j = 2;
		else		j ^= tuner_flag | !regular_rle;
		action = button_action[i][j];
		}

	    switch (action)	{

	    case ACTION_PIXEL_INFO:/*	case ACTION_CYCLE:	*/
		/* cycle in the current (flip_forward) direction */
		if (movie)	goto	cycle_movie;
		if (!(img->parts && (sb=OnScrollBar(img->parts,
			&event.xbutton))) && img->scan_data)
			ColorImageInfo(img, event.xbutton.y
#ifdef	SCROLLBAR_on_CANVAS
		- img->y
#endif
				> img->resize_h>>1, fforward, regular_rle);
		continue;
	    case ACTION_SUB_WINDOW:/*	case ACTION_CHDIR:	*/
		if (movie)	{
			flip_forward = !flip_forward;
			goto	afs;
		}
		if (!tuner_flag)	continue;
		set_left_ptr_cursor(img->window);
		img->tmp_offset = fontHeight + IIMargin;
		TrackSubWin(img, &histinfo, event.xbutton.x, event.xbutton.y,
			DrawsRect, CropButton,
#ifdef	SCROLLBAR_on_CANVAS
		img->y);
#else
		0);
#endif
		continue;

	    case ACTION_MAGNIFY:/*	case ACTION_MOVIE_FORWARD:	*/
		if (movie) {
		    cur_img = action_flip_forward (cur_img, rpic, movie_udelay,
					 n, ButtonPressMask|KeyPressMask,
					 &event, &found_event);
		    flip_frame = cur_img;
		    flip_forward = True;
		}
		else mag_pan(img, action, event.xbutton.x, event.xbutton.y,
			img->mag_fact + 1, stingy_flag);
		continue;
	    case ACTION_UNMAGNIFY:/*	case ACTION_MOVIE_BACKWARD:	*/
		if (movie) {
		    cur_img = action_flip_backward(cur_img, rpic,
				movie_udelay, n,
				ButtonPressMask | KeyPressMask,
				&event, &found_event);
		    flip_frame = cur_img;
		    flip_forward = False;
		}
		else	mag_pan(img, action, event.xbutton.x, event.xbutton.y,
			img->mag_fact - 1, stingy_flag);
		continue;
	    case ACTION_SWITCH_MAG_MODE:/*	case ACTION_MOVIE_SPEED:	*/
	    /* X calls by John Bradley, U of Penn, hacked up by mrf. */
		if (movie)	{
		x_bool	first = 1;
		int	height = DisplayHeight(Dpy, Screen) >> 2,/* use DH/4 */
			ly = 0, s = movie_frams_sec;
		    MapPixWindow(img=rpic[0], True, MGray);

		    Loop	{	/* loop until button released */
			Window foo,poo;
			int rx, ry, x, y, inc;
			unsigned int mask;

			if (XQueryPointer(img->dpy, img->window, &foo, &poo,
					&rx, &ry, &x, &y, &mask)) {
			    if (!(mask&(Button1Mask|Button2Mask|Button3Mask)))
				break;	/* released */

			    if (first)
				ly = y;
			    inc = (ly - y) * 100 / height;

			    /* wait for new pixel */
			    if ((first || movie_frams_sec + inc != s)) {
				s = movie_frams_sec + inc;
				s = (s < 0) ? 0 : s & 0x7F;
				DrawSpeedWindow(img, s);
				first = 0;
			    }
			}
			else	break;
		    }
		    movie_frams_sec = s;
		    UnmapPixWindow(img);
		}
		else mag_pan(img, action, event.xbutton.x, event.xbutton.y,
			img->mag_fact, stingy_flag);
		continue;

	    case ACTION_PAN:/*	case ACTION_MOVIE_STEP:	*/
		/* step movie in current direction. */
		if (movie) {
afs:		    if (flip_forward) {
			if (flip_frame == n-1)
				img = rpic[flip_frame=0];
			else	img = rpic[++flip_frame];
		    }
		    else {
			if (flip_frame < 1)
				img = rpic[flip_frame = n-1];
			else	img = rpic[--flip_frame];
		    }
		    handle_exposure(img, Draws, 0, 0, img->w, img->h, img->h);
		    XStoreName(img->dpy, img->frame, img->title);
		}
		else	mag_pan(img, action, event.xbutton.x, event.xbutton.y,
			img->mag_fact, stingy_flag);
		continue;

	    case ACTION_OBJECT:
		if (i=on_superimpose_elem(img, event.xbutton.x, event.xbutton.y))
			superimpose_handle(img, i,
				event.xbutton.x, event.xbutton.y);
		continue;

	    case ACTION_TUNER:
		if (tuner_flag && Tuner_act < 0)
tuner_action:		Tuner(rpic, maxRLE_images, &Tuner_act);
		else	XBell(img->dpy, 0);
	    default:
		continue;
	    }
	    break;	/* not reached */
	case EnterNotify:	{	/* an ETA section */
	static	int	last_active = -1;
		if (!tuner_flag)	continue;	/* not ETA */
		if (cur_img==last_active || cur_img<0)
			if (img)	continue; /* enter same window again */
			else	goto	tuner_action;
		else	{
			last_active = cur_img;
			if (img->linearup < 24) {
				if (img->img_num < 3)	img->RGB = ButtonSync;
				ButtonState(FButton) = ButtonSync;
				ResetORange(img);
				img->curve = ETALinear;
			}
			Panel_Image(img, histinfo.lkt);
			Fresh_ImageScreen(img, NULL /* no interpolation */, &i);
		}
	}	continue;
	case Expose:
	if (img && event.xexpose.window == img->window)
	    handle_exposure(img, Draws, event.xexpose.x, event.xexpose.y,
			    event.xexpose.width, event.xexpose.height,
			    img->h, tuner_flag);
	else if (tuner_flag)
		Exposure_handler(&event, NULL, No);
	continue;

	case ButtonRelease:
	if (sb)	{
#ifndef	SCROLLBAR_on_CANVAS
		exposure_r(img, Draws, 0, 0, img->resize_w, img->resize_h, No);
#	endif
		DrawScrollBars(img->parts, img->parts->scrollbar.type),
		sb = 0;
	}
	continue;

	case ConfigureNotify:
	case ResizeRequest:
		ResizeWindow(img, &event);	continue;
	case MotionNotify:
	if (sb)
		SetScrollBar(img->parts,event.xbutton.x,event.xbutton.y,sb-1);
	case LeaveNotify:
	case NoExpose:	continue;

	case KeyPress:	{
	char	string[256], *symstr, *XKeysymToString();
	KeySym	keysym;
	XComposeStatus	stat;
	x_bool	shifted_key;
	int	handled_key = keysym,
		length = XLookupString(&event, string, 256, &keysym, &stat);

	    string[length] = 0;
	    symstr = XKeysymToString(keysym);
	    shifted_key = event.xkey.state & ShiftMask;

	    if (length == 1)	{
		i = string[0];
		if ((toupper(i) == 'Q' || i == 3)) /* Qs or ^C */
			break;
		if (isdigit(i))	{
			i = atoi(string);
			if (handled_key == XK_Alt_L || handled_key == XK_Alt_R)
				i = -i;
			mag_pan(img, ACTION_MAGNIFY, event.xkey.x,
				event.xkey.y, i, stingy_flag);
		} else	switch (i)	{
		case '+':if (img->img_num>1 && !movie) {
		byte*	tp = img->data;
			length = img->w * img->h * img->dpy_channels;
			img->data += length;
			if (!tuner_flag)
				img->scan_data = img->data;
			if (fforward++) {	/* both kludge plus 1 */
				if (img->RGB >= img->img_num)
					goto	fwd_resume;
				memcpy(tp, img->data, length);
			(img->std_swif)(FI_RLOAD_BUF, img, img->data, OsameI);
			}
			img->RGB++;
			DumpScan_to_dpy(img);

fwd_resume:		img->data = tp;
			if (!tuner_flag)
				img->scan_data = img->data;
		}	break;
		case '-':
			if (fforward) {
				fforward = 0;
				img->RGB--;
				DumpScan_to_dpy(img);
			}
			break;
		case 'b':	/* back and forth mode */
		    if (!movie)	break;
			bounce_mode = 1;	goto	flip_movie;
		case 'c':	case 'C':
		    if (movie) {
cycle_movie:		bounce_mode = 0;
			flip_forward = (string[0] == 'c');
flip_movie:		flip_frame = action_movie_cycle
				(flip_frame, rpic, n, flip_forward,
					movie_udelay, bounce_mode);
			img = rpic[flip_frame];
		    }
		    break;
		case ' ':
		    if (movie)	goto	afs;
		    break;
		case 'h':	case 'H':
			Toggle_Info(Help_message_getx);
			break;
		case 'i':	case 'I':
	/*	Install/deinstall colormap. Should only do this if no window
	manager running, but that's hard to tell.  Let user deal with it... */
		    if (img->colormap)
			if (string[0] == 'i')
				XInstallColormap(img->dpy, img->colormap);
			else	XUninstallColormap(img->dpy, img->colormap);
		    break;
		case 'r':	if (vroot_dpy)	{
				vroot_dpy = 0;
				ResetWindowBackground(img->window, 0);
			} else if (cmn_hd.win)	{
				vroot_dpy++;
				XSetWindowBackgroundPixmap(img->dpy,img->window,
					img->pixmap);
			}
		default:
			handled_key = False;
		}
	    }	else	handled_key = False;

	    DPRINTF(stderr, "%s %x, %s String '%s' - %d\n", symstr, keysym,
		(shifted_key) ? "shifted" : "unshifted", string, length);

	    if (handled_key > 0) {
		/* GACK! the F28-34 keysyms are for the Suns!	*/
		/* on the DECs they are Left Right Up and Down w/ShiftMask */
		/* on the ardent they are KP_4 KP_6 KP_8 KP_2 w/ShiftMask */
		/* insert your favorite shifted arrow keysyms here	*/

		if (isame(symstr, "Left") || isame(symstr, "F30"))
			i = shifted_key ? 0 : img->w>>2,
			length = img->h >> 1;
		else if (isame(symstr, "Up") || isame(symstr, "F28"))
			i = img->w >> 1,
			length = shifted_key ? 0 : img->h>>2;
		else if (isame(symstr, "Right") || isame(symstr, "F32"))
			i = shifted_key ? img->w-1 : img->w * 3 >> 2,
			length = img->h>>1;
		else if (isame(symstr, "Down") || isame(symstr, "F34"))
			i = img->w >> 1,
			length = shifted_key ? img->h-1 : img->h * 3 >> 2;
		else	continue;
		mag_pan(img, ACTION_PAN, i, length, img->mag_fact, stingy_flag);
	    }
	    continue;
	}
	case MapNotify:
		if (event.xunmap.window == img->frame && img->icn_window)
			XUnmapWindow(img->dpy, img->icn_window);
		else if (event.xunmap.window == img->icn_window)
			XUnmapWindow(img->dpy, img->frame);
		continue;
	case UnmapNotify:
		if (event.xunmap.window == img->frame && img->icn_window)	{
			LoadIcon(img);
			XMapWindow(img->dpy, img->icn_window);
		}
		else if (event.xunmap.window == img->icn_window)
			XMapWindow(img->dpy, img->frame);
		continue;
	case MappingNotify:
		XRefreshKeyboardMapping(&event.xmapping);
		continue;

	default:	prgmerr(0, "Event type %0X?", event.type);
	continue;
	}

	dead_window = DestroyColorImage(img);	/* exit this window */
	firstmap = 0;
	if (--n) {	/* pack imgs in there good */
	    for (;cur_img < n; cur_img++)
		rpic[cur_img] = rpic[cur_img+1];
	    rpic[cur_img] = NULL;
	}
	else	if (!tuner_flag)	{
#ifdef	X_Extender
quit_server:	XBell(Dpy, 0);
		Toggle_Info("\n                  Quit Extended X Server");
		sleep(2);
		XBell(Dpy, 0);
#endif
		break;
	}
	if (rpic[0]->window == dead_window)
		break;	/* only for safety of movies. Not really reached */
    }
}
