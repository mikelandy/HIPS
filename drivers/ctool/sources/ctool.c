#ifndef lint
static char ctoolSccsId[] = "@(#)ctool.c	1.3 10/21/91";
#endif
/*
	Copyright 1988 Alan Shaw and Eric Schwartz.
	No part of this software may be distributed or sold without the prior
	agreement of Prof. Eric Schwartz, Dept. of Psychiatry, NYU School of
	Medicine, 550 1st Ave., New York, New York, 10016.
ctool.c
        Revision:
	hek Fri Dec 21 12:44:25 EST 1990
	    Modified all references to hd.ocols to be filtered through
	    to32(), a macro that raises its integer argument to the
	    nearest multiple of 32, as per the Pixrect programmers guide.
	    This action occurs in all instances save those where the
	    actual data file is read, this must, of course, be read in
	    with the original dimensions.
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>
#include <errno.h>
#include <math.h>

#include <suntool/sunview.h>
#include <suntool/canvas.h>
#include <suntool/panel.h>
#include <pixrect/pixrect_hs.h>

#include <hipl_format.h>

#define	TOOL_BORDERWIDTH	(5)

#define	Brushradius	8
#define	BRUSHCURSOROP	PIX_SRC ^ PIX_DST

#define	BRUSH		'b'
#define	CHANGEBRUSH	'c'
#define	RESTORE_LUT	'R'
#define	READIN		'i'
#define	READINOP	'o'
#define	WRITE		'w'
#define	WRITEBOX	'W'
#define	FORMAT		'g'
#define	PAINTBOX	'p'
#define	INITBOX		'x'
#define	SIZEBOX		'X'
#define	QUIT		'q'
#define	FATBITS		'f'
#define	LOCATOR		'l'
#define	KILLFATBITS	'k'
#define	RESTORE		'r'
#define	FILTER1		'F'
#define	FILTER2		'|'
#define	LUTSWEEP	's'
#define	LUTSWEEP_DOWN	'd'
#define	RANLUTS		'L'
#define	RANLUTS_OFF	'O'
#define	ZOOMIN		'z'
#define	ZOOMOUT		'Z'
#define	BLINK		'B'
#define	UNDO		'u'
#define	CHANGEMENU	'm'
#define	STRETCHLUT	'S'
#define	LUTFRAME	'U'
#define	POWERPOINT	'!'
#define	CURSOROP	'*'
#define	SETBKGD		'&'
#define	NEXTFRAME	'n'

/*	The following strings contain SPACES and not TABS.
*	A tab comes out as a reverse-video space in a menu.
*/

#define	RESTORE_LUTSTRING	"restore LUT   (R)"
#define	READINSTRING		"read in       (i)"
#define	WRITESTRING		"write         (w)"
#define	WRITEBOXSTRING		"write box     (W)"
#define	PAINTBOXSTRING		"paint box     (p)"
#define	INITBOXSTRING		"init  box     (x)"
#define	SIZEBOXSTRING		"set box size  (X)"
#define	QUITSTRING		"quit          (q)"
#define	FATBITSSTRING		"fat bits      (f)"
#define	KILLFATBITSSTRING	"kill fat bits (k)"
#define	RESTORESTRING		"restore       (r)"
#define	FILTERSTRING		"filter box  (F |)"
#define	UNDOSTRING		"undo          (u)"
#define	SHORTMENUSTRING		"short menu    (m)"
#define	LONGMENUSTRING		"long menu     (m)"
#define	LUTSWEEPSTRING		"LUT sweep     (s)"
#define	LUTSWEEPDOWNSTRING	"LUT sweep down(d)"
#define	STRETCHLUTSTRING	"LUT stretch   (S)"
#define	RANLUTSSTRING		"ranluts       (L)"
#define	RANLUTSOFFSTRING	"ranluts off   (O)"
#define	ZOOMINSTRING		"zoom in       (z)"
#define	ZOOMOUTSTRING		"zoom out      (Z)"
#define	LUTFRAMESTRING		"LUT frame     (U)"
#define	NEXTFRAMESTRING		"next frame    (n)"

#define	BITPIXHT	(bitwinrows/Bitsize)
#define	BITPIXWD	(bitwincols/Bitsize)

#define to32(a)         (((int)(a) + 3) & ~3)

/* values for Outformat: */
#define	INFORMAT	0
#define	FORCEGRLE	1

Pixwin		*imgpw, *bitpw, *lutpw;
struct header	hd;		/* hips header */
int		BackRed, BackGreen, BackBlue;
int		Magfactor = 1;	/* magnify image on screen? */
void		image_selected(), bits_selected(), lut_selected();

static Pixrect	*save1, *save2, *brushsave, *readin_image;
static int	readin_op = PIX_SRC;
static int	Outformat = INFORMAT;
static int	save_x, save_y, save_dx, save_dy;

static int	cursorx, cursory;
				/* from event position in image_selected() */

static Frame		tool, filename_frame, command_frame, size_frame,
			message_frame, lut_frame, bkgd_frame;
static Panel		filename_panel, command_panel, size_panel,
			message_panel, bkgd_panel;
static Panel_item	filename_item, command_item, size_item, bkgd_item;
static Canvas		imgsw, bitsw, lutsw;
static Pixrect		*image_in_window, *backup_image;
static Menu		longmenu, shortmenu, bitmenu, lutmenu;
static int		imgwinrows;	/* number of rows in image subwindow */
static int		imgwincols;	/* number of cols in image subwindow */
static int		bitwinrows, bitwincols;
static char		coordstring[40];
				/* pixel coordinates displayed in image menu */
static char		locatorstring[40];
static char		boxposstring[40];	/* box position */
static char		boxsizstring[40];	/* box size */
static char		brushstring[40];
static char		changebrushstring[40];
static char		readinopstring[40];
static char		formatstring[40];
static char		blinkstring[40];
static char		colorstring[40] = "                  ";
				/* color index displayed in fat bit menu:
				initialised to this string of spaces to make
				menu_create give a wide enough menu item */
static char		lutstring[40];	/* location string in bitmenu */

static int		Bitsize = 16;	/* side of a fat bit in image pixels */
/*
static short cursor_data[16] = {
	0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0xfffe,
	0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100
};
*/

static short square_brush_data[16] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

static short diamond_brush_data[16] = {
	0x0180, 0x03c0, 0x07e0, 0x0ff0, 0x1ff8, 0x3ffc, 0x7ffe, 0xffff,
	0xffff, 0x7ffe, 0x3ffc, 0x1ff8, 0x0ff0, 0x07e0, 0x03c0, 0x0180
};

static short point_brush_data[16] = {
	0, 0x80, 0, 0, 0, 0, 0, 0, 0x4081, 0, 0, 0, 0, 0, 0, 0x80
};

/*
mpr_static(main_cursor_pr, 16, 16, 1, cursor_data);
*/

mpr_static(square_brush_pixrect, 16, 16, 1, square_brush_data);
mpr_static(diamond_brush_pixrect, 16, 16, 1, diamond_brush_data);
mpr_static(point_brush_pixrect, 16, 16, 1, point_brush_data);

static Cursor		main_cursor, brush_cursor;
static Pixrect		*stencil_pixrect;

static struct rasterfile rh;		/* sun image header */
static int		ac;		/* to hold argc */
static char		**av;		/* to hold argv */
static float		*floatbuf;
static char		labelname[150];	/* label for the frame */
static char		*coordposition;
				/* where in label to put current coords */

/* static u_char		colorbuf[768]; */
static u_char		*colorbuf;
static int		colorentries;

static int		(*fileproc)();

static int	box_xposition, box_yposition, box_xlength, box_ylength;
static h_boolean	blinking	= FALSE;
static h_boolean	locator		= FALSE;
static h_boolean	box_is_on	= TRUE;
static h_boolean	bitwin_on	= FALSE;
static h_boolean	lutframe_is_on	= FALSE;
static int	my_client_object;
static int	*me = &my_client_object;
static int	FBdepth;	/* depth of frame buffer in bits per pixel */
static int	Framenumber;	/* sequence number of frame
							currently displayed */
static char	*framenumberposition;	/* where to display the frame number */

/* //////////////////////////////////////////////////////////////////////// */

main(argc, argv)
  int	argc;
  char	*argv[];
{
  struct pixfont	*font;
  int		i;
  void		redraw();
  void		redraw_bitwin();
  h_boolean	LABELSPEC	= FALSE;
  char		*labelspec;
  h_boolean	CREATE		= FALSE;
  int		xlength, ylength;
  h_boolean	POSITION_SET	= FALSE;
  int		tool_xposition, tool_yposition;
  Panel_setting	filename_proc(), command_proc(), size_proc(), bkgd_proc();
  u_char		*imagebuf;
  
  Progname = strsave(*argv);
  
  ac = argc + 2;
  av = (char **)(calloc(ac, sizeof(char *)));
  for (i = 0; i < argc; i++)
    av[i] = argv[i];
  av[argc] = "(@(#)ctool.c	1.110 7/20/89)";
  av[argc + 1] = "";
  
  BackRed		= 255;
  BackGreen	= 0;
  BackBlue	= 0;
  
  for (i = 1; i < argc; i++) {
      if (argv[i][0] != '-')
	break;
      switch (argv[i][1]) {
	case 'b':
	  if (argv[i][2])
	    BackRed = atoi(argv[i] + 2);
	  else
	    BackRed = atoi(argv[++i]);
	  BackGreen = atoi(argv[++i]);
	  BackBlue = atoi(argv[++i]);
	  break;
	case 'l':
	  LABELSPEC = TRUE;
	  if (argv[i][2])
	    labelspec = argv[i] + 2;
	  else
	    labelspec = argv[++i];
	  break;
	case 'c':	/* create */
	  CREATE = TRUE;
	  if (argv[i][2])
	    ylength = atoi(argv[i] + 2);
	  else
	    ylength = atoi(argv[++i]);
	  xlength = atoi(argv[++i]);
	  break;
	case 'm':
	  if (argv[i][2])
	    Magfactor = atoi(argv[i] + 2);
	  else
	    Magfactor = atoi(argv[++i]);
	  break;
	case 'W':	/* generic tool arguments */
	  switch (argv[i][2]) {
	    case 'p':
	      POSITION_SET = TRUE;
	      if (argv[i][3])
		tool_xposition = atoi(argv[i] + 3);
	      else
		tool_xposition = atoi(argv[++i]);
	      tool_yposition = atoi(argv[++i]);
	      break;
	    default:
	      fprintf(stderr, "%s: unknown flag %c%c\n",
		      av[0], av[i][1], av[i][2]);
	      exit(1);
	      break;
	    }
	  break;
	default:
	  fprintf(stderr,
		  "%s: unknown flag %c\n", av[0], av[i][1]);
	  exit(1);
	}
    }
  if (i < argc) {
      if (!CREATE) {
	  if (freopen(argv[i], "r", stdin) == NULL) {
	      perr(HE_OPEN, argv[i]);
	      exit(1);
	    }
	  if (!LABELSPEC)
	    labelspec = argv[i];
	  i++;
	}
    }
  else
    if (!LABELSPEC)
      labelspec = argv[0];
  
  if (CREATE) {
      int		ch;
      char		date_buf[128], *bp;
      FILE		*fp, *popen();
      char		*username, *getenv();
      
      if (fp = popen("date", "r")) {
	  bp = date_buf;
	  while ((ch = getc(fp)) != EOF) {
	      *bp++ = ch;
	    }
	  pclose(fp);
	}
      username = getenv("USER");
      init_header(&hd, username, "ctool image", 1, date_buf,
		  ylength, xlength, PFBYTE, 1, ""); 
    }
  else
    read_header(&hd);

  /* ensure that the sizepix field is correctly set */
  sethdrsize(&hd);
  
  rh.ras_width	= Magfactor * hd.ocols;
  rh.ras_height	= Magfactor * hd.orows;
  bitwincols	= imgwincols	= rh.ras_width;
  bitwinrows	= imgwinrows	= rh.ras_height;
  
  /* Make the tool */
  
  font = pw_pfsysopen();	/* get font for window size reckoning */
  
  main_cursor = cursor_create(CURSOR_IMAGE, &square_brush_pixrect,
			      CURSOR_SHOW_CURSOR,		FALSE,
			      CURSOR_SHOW_CROSSHAIRS,		TRUE,
			      CURSOR_CROSSHAIR_LENGTH,	8,
			      CURSOR_CROSSHAIR_GAP,		2,
			      CURSOR_OP,			PIX_SRC^PIX_DST,
			      CURSOR_VERT_HAIR_OP,		PIX_SRC^PIX_DST,
			      CURSOR_HORIZ_HAIR_OP, 		PIX_SRC^PIX_DST,
			      0, 0);
  
  brush_cursor = cursor_create(CURSOR_IMAGE, &square_brush_pixrect,
			       CURSOR_XHOT,			8,
			       CURSOR_YHOT,			8,
			       CURSOR_OP,			BRUSHCURSOROP,
			       0, 0);
  
  stencil_pixrect = &square_brush_pixrect;
  
  longmenu = menu_create(
			 MENU_STRING_ITEM, coordstring,		(caddr_t)'N',
			 MENU_STRING_ITEM, boxposstring,		(caddr_t)'N',
			 MENU_STRING_ITEM, boxsizstring,		(caddr_t)'N',
			 MENU_STRING_ITEM, FATBITSSTRING,	(caddr_t)FATBITS,
			 MENU_STRING_ITEM, locatorstring,	(caddr_t)LOCATOR,
			 MENU_STRING_ITEM, KILLFATBITSSTRING,	(caddr_t)KILLFATBITS,
			 MENU_STRING_ITEM, brushstring,		(caddr_t)BRUSH,
			 MENU_STRING_ITEM, changebrushstring,	(caddr_t)CHANGEBRUSH,
			 MENU_STRING_ITEM, LUTSWEEPSTRING,	(caddr_t)LUTSWEEP,
			 MENU_STRING_ITEM, LUTSWEEPDOWNSTRING,	(caddr_t)LUTSWEEP_DOWN,
			 MENU_STRING_ITEM, LUTFRAMESTRING,	(caddr_t)LUTFRAME,
			 MENU_STRING_ITEM, RESTORESTRING,	(caddr_t)RESTORE,
			 MENU_STRING_ITEM, RANLUTSSTRING,	(caddr_t)RANLUTS,
			 MENU_STRING_ITEM, RANLUTSOFFSTRING,	(caddr_t)RANLUTS_OFF,
			 MENU_STRING_ITEM, RESTORE_LUTSTRING,	(caddr_t)RESTORE_LUT,
			 MENU_STRING_ITEM, FILTERSTRING,		(caddr_t)FILTER1,
			 MENU_STRING_ITEM, UNDOSTRING,		(caddr_t)UNDO,
			 MENU_STRING_ITEM, SHORTMENUSTRING,	(caddr_t)CHANGEMENU,
			 MENU_STRING_ITEM, READINSTRING,		(caddr_t)READIN,
			 MENU_STRING_ITEM, readinopstring,	(caddr_t)READINOP,
			 MENU_STRING_ITEM, WRITESTRING,		(caddr_t)WRITE,
			 MENU_STRING_ITEM, WRITEBOXSTRING,	(caddr_t)WRITEBOX,
			 MENU_STRING_ITEM, formatstring,		(caddr_t)FORMAT,
			 MENU_STRING_ITEM, PAINTBOXSTRING,	(caddr_t)PAINTBOX,
			 MENU_STRING_ITEM, INITBOXSTRING,	(caddr_t)INITBOX,
			 MENU_STRING_ITEM, SIZEBOXSTRING,	(caddr_t)SIZEBOX,
			 MENU_STRING_ITEM, ZOOMINSTRING,		(caddr_t)ZOOMIN,
			 MENU_STRING_ITEM, ZOOMOUTSTRING,	(caddr_t)ZOOMOUT,
			 MENU_STRING_ITEM, NEXTFRAMESTRING,	(caddr_t)NEXTFRAME,
			 MENU_STRING_ITEM, blinkstring,		(caddr_t)BLINK,
			 MENU_STRING_ITEM, QUITSTRING,		(caddr_t)QUIT,
			 0, 0);
  
  shortmenu = menu_create(
			  MENU_STRING_ITEM, coordstring,		(caddr_t)'N',
			  MENU_STRING_ITEM, LONGMENUSTRING,	(caddr_t)CHANGEMENU,
			  0, 0);
  
  bitmenu = menu_create(
			MENU_STRING_ITEM, colorstring,	(caddr_t)'N',
			0, 0);
  
  lutmenu = menu_create(
			MENU_STRING_ITEM, lutstring,		(caddr_t)'N',
			MENU_STRING_ITEM, STRETCHLUTSTRING,	(caddr_t)STRETCHLUT,
			0, 0);
  
  sprintf(labelname, labelspec);
  switch(hd.pixel_format)
    {
#ifdef	GRLE
    case PFGRLE:
      strcat(labelname, " [grle]");
      break;
      
    case PFSRLE:
      strcat(labelname, " [srle]");
      break;
#endif
    case PFINT:
      strcat(labelname, " [int]");
      break;
      
    case PFFLOAT:
      strcat(labelname, " [float]");
      break;
      
    case PFBYTE:
      strcat(labelname, " [byte]");
      break;
      
    case PFMSBF:
    case PFLSBF:
      strcat(labelname, " [bpacked]");
      break;
      
    default:
      strcat(labelname, " [unknown format]");
      break;
    }
  
  if (hd.num_frame > 1) {
      strcat(labelname, " [frame ");
      framenumberposition = labelname + strlen(labelname);
      strcat(labelname, "1]");
    }
  
  coordposition = labelname + strlen(labelname);
  
  tool = window_create(0,	FRAME,
		       FRAME_LABEL,	labelname,
		       WIN_TOP_MARGIN,		0,
		       WIN_LEFT_MARGIN,	0,
		       WIN_WIDTH,	imgwincols + 2 * TOOL_BORDERWIDTH,
		       WIN_HEIGHT,	imgwinrows + (font->pf_defaultsize.y+2)
		       + TOOL_BORDERWIDTH,
		       WIN_ERROR_MSG, "can't create tool",
		       0);
  
  if (POSITION_SET)
    window_set(tool,
	       WIN_X,	tool_xposition,
	       WIN_Y,	tool_yposition,
	       0,	0);
  
  /* Set up the hips image subwindow. */
  
  imgsw = window_create(tool, CANVAS,
			WIN_WIDTH,	imgwincols,
			WIN_HEIGHT,	imgwinrows,
			CANVAS_WIDTH,	imgwincols,
			CANVAS_HEIGHT,	imgwinrows,
			CANVAS_REPAINT_PROC,	redraw,
			WIN_EVENT_PROC, image_selected,
			WIN_ERROR_MSG,	"can't create image canvas",
			0);
  
  bitsw = window_create(tool, CANVAS,
			WIN_WIDTH,	bitwincols,
			WIN_HEIGHT,	bitwinrows,
			CANVAS_WIDTH,	bitwincols,
			CANVAS_HEIGHT,	bitwinrows,
			CANVAS_AUTO_EXPAND,	TRUE,
			CANVAS_AUTO_SHRINK,	TRUE,
			CANVAS_REPAINT_PROC,	redraw_bitwin,
			WIN_EVENT_PROC, bits_selected,
			WIN_ERROR_MSG,	"can't create bits canvas",
			0);
  
  filename_frame = window_create(tool, FRAME,
				 WIN_ERROR_MSG, "can't create filename frame",
				 0);
  
  command_frame = window_create(tool, FRAME,
				WIN_ERROR_MSG, "can't create command frame",
				0);
  
  size_frame = window_create(tool, FRAME,
			     WIN_ERROR_MSG, "can't create size frame",
			     0);
  
  bkgd_frame = window_create(tool, FRAME,
			     WIN_ERROR_MSG, "can't create bkgd frame",
			     0);
  
  message_frame = window_create(tool, FRAME,
				WIN_WIDTH,	100,
				WIN_HEIGHT,	10,
				WIN_ERROR_MSG, "can't create message frame",
				0);
  
  lut_frame = window_create(tool,	FRAME,
			    WIN_WIDTH,	256 + 2 * TOOL_BORDERWIDTH,
			    WIN_HEIGHT,	256 + 2 * TOOL_BORDERWIDTH,
			    WIN_ERROR_MSG,	"can't create lut frame",
			    0);
  
  lutsw = window_create(lut_frame, CANVAS,
			WIN_WIDTH,	256,
			WIN_HEIGHT,	256,
			WIN_EVENT_PROC, lut_selected,
			WIN_ERROR_MSG,	"can't create lut canvas",
			0);
  
  filename_panel	= window_create(filename_frame, PANEL, 0);
  command_panel	= window_create(command_frame, PANEL, 0);
  size_panel	= window_create(size_frame, PANEL, 0);
  bkgd_panel	= window_create(bkgd_frame, PANEL, 0);
  message_panel	= window_create(message_frame, PANEL, 0);
  
  filename_item	= panel_create_item(filename_panel, PANEL_TEXT,
				    PANEL_VALUE_DISPLAY_LENGTH,	22,
				    PANEL_VALUE_STORED_LENGTH,	255,
				    PANEL_LABEL_STRING,	"File:   ",
				    PANEL_NOTIFY_PROC,	filename_proc,
				    0);
  
  command_item	= panel_create_item(command_panel, PANEL_TEXT,
				    PANEL_VALUE_DISPLAY_LENGTH,	22,
				    PANEL_VALUE_STORED_LENGTH,	255,
				    PANEL_LABEL_STRING,	"Command:   ",
				    PANEL_NOTIFY_PROC,	command_proc,
				    0);
  
  size_item	= panel_create_item(size_panel, PANEL_TEXT,
				    PANEL_VALUE_DISPLAY_LENGTH,	22,
				    PANEL_VALUE_STORED_LENGTH,	255,
				    PANEL_LABEL_STRING,	"Box size (x y):   ",
				    PANEL_NOTIFY_PROC,	size_proc,
				    0);
  
  bkgd_item	= panel_create_item(bkgd_panel, PANEL_TEXT,
				    PANEL_VALUE_DISPLAY_LENGTH,	22,
				    PANEL_VALUE_STORED_LENGTH,	255,
				    PANEL_LABEL_STRING,	"Background colors (r g b):   ",
				    PANEL_NOTIFY_PROC,	bkgd_proc,
				    0);
  
  panel_create_item(message_panel,	PANEL_MESSAGE,
		    PANEL_LABEL_STRING, "Move the image within the",
		    0);
  panel_create_item(message_panel,	PANEL_MESSAGE,
		    PANEL_LABEL_STRING, "frame with the left mouse",
		    0);
  panel_create_item(message_panel,	PANEL_MESSAGE,
		    PANEL_LABEL_STRING, "button.  Confirm position",
		    0);
  panel_create_item(message_panel,	PANEL_MESSAGE,
		    PANEL_LABEL_STRING, "with the middle button.",
		    0);
  
  window_set(filename_panel, PANEL_CARET_ITEM, filename_item, 0, 0);
  window_set(command_panel, PANEL_CARET_ITEM, command_item, 0, 0);
  window_set(size_panel, PANEL_CARET_ITEM, size_item, 0, 0);
  window_set(bkgd_panel, PANEL_CARET_ITEM, bkgd_item, 0, 0);
  
  window_fit(filename_panel);
  window_fit(filename_frame);
  
  window_fit(command_panel);
  window_fit(command_frame);
  
  window_fit(size_panel);
  window_fit(size_frame);
  
  window_fit(bkgd_panel);
  window_fit(bkgd_frame);
  
  window_fit(message_panel);
  window_fit(message_frame);
  
  imgpw = canvas_pixwin(imgsw);
  bitpw = canvas_pixwin(bitsw);
  lutpw = canvas_pixwin(lutsw);
  
  /* get the frame buffer depth: */
  
  FBdepth = imgpw->pw_pixrect->pr_depth;
  if (FBdepth > 1)
    FBdepth = 8;
  rh.ras_depth = FBdepth;
  if (CREATE && (FBdepth == 1)) {
#ifdef MSBFVERSION
      hd.pixel_format = PFMSBF;
#else
      hd.pixel_format = PFLSBF;
#endif MSBFVERSION
    }
  
  /* set up the events mask */
  
  window_set(imgsw, WIN_CONSUME_KBD_EVENTS,
	     WIN_UP_EVENTS, WIN_ASCII_EVENTS, 0, 0);
  window_set(bitsw, WIN_CONSUME_KBD_EVENTS,
	     WIN_UP_EVENTS, WIN_ASCII_EVENTS, 0, 0);
  window_set(lutsw, WIN_CONSUME_KBD_EVENTS,
	     WIN_ASCII_EVENTS, 0, 0);
  window_set(lutsw, WIN_IGNORE_PICK_EVENTS,
	     WIN_UP_EVENTS, 0, 0);
  window_set(imgsw, WIN_CONSUME_PICK_EVENT, LOC_DRAG, 0, 0);
  window_set(bitsw, WIN_CONSUME_PICK_EVENT, LOC_DRAG, 0, 0);
  window_set(lutsw, WIN_CONSUME_PICK_EVENT, LOC_DRAG, 0, 0);
  window_set(imgsw, WIN_CONSUME_PICK_EVENT, LOC_STILL, 0, 0);
  window_set(bitsw, WIN_CONSUME_PICK_EVENT, LOC_STILL, 0, 0);
  window_set(imgsw, WIN_CONSUME_PICK_EVENTS, WIN_IN_TRANSIT_EVENTS, 0, 0);
  window_set(bitsw, WIN_CONSUME_PICK_EVENTS, WIN_IN_TRANSIT_EVENTS, 0, 0);
  
  /* Read the hips image into memory */
  
  image_in_window= mem_create(to32(rh.ras_width), rh.ras_height, rh.ras_depth);
  backup_image	= mem_create(rh.ras_width, rh.ras_height, rh.ras_depth);
  save1		= mem_create(rh.ras_width, rh.ras_height, rh.ras_depth);
  save2		= mem_create(rh.ras_width, rh.ras_height, rh.ras_depth);
  brushsave	= mem_create(rh.ras_width, rh.ras_height, rh.ras_depth);
  
  init_box(hd.ocols, hd.orows);
  
  if (findparam(&hd, "cmap"))
    {
      colorentries = 2;
      getparam(&hd, "cmap", PFBYTE, &colorentries, &colorbuf);
    }
  else
    colorentries = 0;
  
  if ((imagebuf = (u_char *) calloc(hd.orows * hd.ocols, sizeof(u_char)))==NULL)
    error("can't allocate core");
  
  if (!CREATE) {
      if (hd.pixel_format == PFFLOAT) {
	  floatbuf = (float *) malloc(hd.orows * hd.ocols * sizeof(float));
	  if (floatbuf == NULL)
	    error("can't allocate core");
	  if (load(stdin, hd.orows, hd.ocols, hd.pixel_format,
		   hd.sizepix, floatbuf, PFFLOAT, 32) < 0)
	    error("load failure");
	  scale(floatbuf, imagebuf);
	}
      else {
	  if (load(stdin, hd.orows, hd.ocols, hd.pixel_format,
		   hd.sizepix, imagebuf, PFBYTE, FBdepth) < 0)
	    error("load failure");
	}
      Framenumber = 1;
    }
  
{
  int informat = hd.pixel_format;
  
  hd.pixel_format = PFBYTE;
  if (to_sun(&hd, imagebuf, &rh, (colormap_t *)0,
	     mpr_d(image_in_window)->md_image, Magfactor) < 0)
    error("problem converting hips image");
  hd.pixel_format = informat;
}
free(imagebuf);

if (colorentries)
  setupcolormap(colorbuf);
else
  setupfullgraycolormap();

draw_lutgraph();

pr_rop(backup_image, 0, 0, rh.ras_width, rh.ras_height,
       PIX_SRC, image_in_window, 0, 0);

/* write the image into the image window */

pw_write(imgpw, 0, 0, rh.ras_width, rh.ras_height,
	 PIX_SRC | PIX_DST, image_in_window, 0, 0);

window_set(imgsw, WIN_CURSOR, main_cursor, 0, 0);
window_set(bitsw, WIN_CURSOR, main_cursor, 0, 0);

/*
  window_set(imgsw, CANVAS_RESIZE_PROC, resize, 0, 0);
  */

/* Now the input select loop */

window_main_loop(tool);

exit(0);
}


/* //////////////////////////////////////////////////////////////////////// */

sethdrsize(hdr)
  struct header *hdr;
{
  switch(hdr->pixel_format)	/* below we need pixel size in bits, rather
				   than bytes.  We store this in hd.sizepix,
				   which is ok because the HIPS library won't
				   be using hd.sizepix again in this tool -
				   we don't use setsize() or alloc_image() */
    {
    case PFCOMPLEX:
      hdr->sizepix = 64;
      break;

    case PFFLOAT:
    case PFINT:
      hdr->sizepix = 32;
      break;

    case PFSHORT:
      hdr->sizepix = 16;
      break;

    case PFGRLE:
    case PFBYTE:
      hdr->sizepix = 8;
      break;
      
    case PFSRLE:
      hdr->sizepix = 1;
      break;
    }
}

/* //////////////////////////////////////////////////////////////////////// */

blink_on()
{
struct itimerval	blink_timer;
Notify_value		my_blinker();
#define	ITIMER_NULL	((struct itimerval *)0)

	blink_timer.it_interval.tv_usec = 0;
	blink_timer.it_interval.tv_sec = 1;
	blink_timer.it_value.tv_usec = 0;
	blink_timer.it_value.tv_sec = 1;
	(void) notify_set_itimer_func(me, my_blinker, ITIMER_REAL,
						&blink_timer, ITIMER_NULL);
	blinking = TRUE;
}

/* //////////////////////////////////////////////////////////////////////// */

blink_off()
{
Notify_value		my_blinker();

	(void) notify_set_itimer_func(me, my_blinker, ITIMER_REAL,
						ITIMER_NULL, ITIMER_NULL);
	blinking = FALSE;
}

/* //////////////////////////////////////////////////////////////////////// */

/*ARGSUSED*/
Notify_value my_blinker(me, which)
Notify_client	me;
int		which;
{
	if (box_is_on) {
		erase_box();
		if (bitwin_on && locator)
			erase_locator();
	}
	else {
		draw_box();
		if (bitwin_on && locator)
			draw_locator();
	}
	box_is_on = !box_is_on;
	return(NOTIFY_DONE);
}

/* //////////////////////////////////////////////////////////////////////// */

/*ARGSUSED*/
void redraw(canvas, pw, repaint_area)
Canvas		canvas;
Pixwin		*pw;
Rectlist	*repaint_area;
{
	pw_write(pw, 0, 0, rh.ras_width, rh.ras_height,
				PIX_SRC, image_in_window, 0, 0);
	if (box_is_on && !blinking)
		draw_box();
	if (bitwin_on && locator)
		draw_locator();
}

/* //////////////////////////////////////////////////////////////////////// */

static int	bitwin_centerx, bitwin_centery;
static int	bitwin_centerBcol, bitwin_centerBrow;
		/* lowest real x and y coordinates in bitwin of the fat bit
		representing (bitwin_centerx, bitwin_centery) */

double		exponent = 1.;		/* current LUT function exponent */
int		lthresh = 0;
int		uthresh = 255;	/* current LUT sweep thresholds */

static int	image_mode;

/* values for image_mode: */
#define	NULLMODE	0
#define	BRUSHMODE	1
#define	UNBRUSHMODE	2
#define	MOVEMODE	3		/* move box */
#define	STRETCHMODE	4		/* stretch box */
#define	SHIFTMODE	5		/* move read-in picture */

static int	cursorstate;

/* values for cursorstate: */
#define	NORMAL	0
#define	BLACK	1
#define	WHITE	2

/*ARGSUSED*/
void image_selected(window, event, arg)
  Window	window;
  Event	*event;
  caddr_t	arg;
{
  caddr_t		menu_show();
  FILE		*fp = stdout;           /* set output to stdout */
/*  int		fd = 1;			set output to stdout */
  static int	paintcolor = 0;
  static Menu	displaymenu;
  static h_boolean firsttime = TRUE;
  short		code;
  int		x, y, color, toolheight;
  int		writeout(), writeout_box(), readin();
  int		i;
  static h_boolean inshift = FALSE;	/* were we in shiftmode when we
					   went into brushmode? If so, go back
					   there when we leave brushmode */
  static h_boolean Moved = FALSE;		/* have we just moved or resized the
					   box? If so, allow adjust_box to
					   position it properly */
  static h_boolean Shifted = FALSE;	/* have we just shifted the read-in
					   image? If so, allow fillin_box to
					   paint it properly */
  h_boolean	replace_cursor = FALSE;
  
  
  if (firsttime) {
      displaymenu = shortmenu;
      firsttime = FALSE;
    }
  
  x = event_x(event);
  y = event_y(event);
  
  /* pixel coordinatess in hips orientation: */
#define	hipsx(x)	((x) / Magfactor)
#define	hipsy(y)	((imgwinrows - (y) - 1) / Magfactor)
  
  /* screen pixel at starting corner of image pixel: */
#define big(x)		(((x)/Magfactor) * Magfactor)
  
  color = pr_get(image_in_window, x, y);
  if (hd.pixel_format == PFFLOAT)
    sprintf(coordstring, "x %d y %d (%f)", hipsx(x), hipsy(y),
	    *(floatbuf + hd.ocols * hipsy(y) + hipsx(x)));
  else
    sprintf(coordstring, "x %d y %d (%d)", hipsx(x), hipsy(y),
	    color);
  
  if (displaymenu == longmenu) {
      sprintf(boxposstring, "box x %d y %d", hipsx(box_xposition + 1),
	      hipsy(box_yposition + box_ylength - 1));
      sprintf(boxsizstring, "    w %d h %d",
	      (box_xlength - 1)/Magfactor, (box_ylength - 1)/Magfactor);
      sprintf(brushstring, "brush %d ", paintcolor);
      if ((image_mode == BRUSHMODE)
	  ||  (image_mode == UNBRUSHMODE))
	sprintf(brushstring + strlen(brushstring), "off");
      else
	sprintf(brushstring + strlen(brushstring), "on");
      for (i = strlen(brushstring); i < 14; i++)
	sprintf(brushstring + strlen(brushstring), " ");
      sprintf(brushstring + strlen(brushstring), "(%c)", BRUSH);
      
      if (stencil_pixrect == &square_brush_pixrect)
	sprintf(changebrushstring, "diamond brush (%c)",
		CHANGEBRUSH);
      else if (stencil_pixrect == &diamond_brush_pixrect)
	sprintf(changebrushstring, "point brush   (%c)",
		CHANGEBRUSH);
      else
	sprintf(changebrushstring, "square brush  (%c)",
		CHANGEBRUSH);
      
      if (readin_op == PIX_SRC)
	sprintf(readinopstring, "readop src^dst(%c)",
		READINOP);
      else
	sprintf(readinopstring, "readop src    (%c)",
		READINOP);
      
      if (blinking)
	sprintf(blinkstring, "box blink off (%c)", BLINK);
      else
	sprintf(blinkstring, "box blink on  (%c)", BLINK);
      
      if (locator)
	sprintf(locatorstring, "locator off   (%c)", LOCATOR);
      else
	sprintf(locatorstring, "locator on    (%c)", LOCATOR);
      if (Outformat == FORCEGRLE)
	sprintf(formatstring, "write byte    (%c)", FORMAT);
      else
	sprintf(formatstring, "write grle    (%c)", FORMAT);
    }
  
  if ((code = event_id(event)) == MS_RIGHT
      &&  event_is_down(event)) {
      code = (short) (menu_show(displaymenu, imgsw, event, 0));
      replace_cursor = TRUE;
    }
  switch (code) {

#define	COMMON_CHOICES	\
		case NEXTFRAME:						\
	{								\
	u_char	*imagebuf;						\
	if (++Framenumber > hd.num_frame) {				\
		fprintf(stderr, "%s: on last frame\n", Progname);	\
		break;							\
	}								\
	sprintf(framenumberposition, "%d]", Framenumber);		\
	window_set(tool, FRAME_LABEL, labelname, 0, 0);			\
        if ((imagebuf = (u_char *) calloc(hd.orows * hd.ocols,            \
					  sizeof(u_char)))  == NULL) \
		error("can't allocate core");				\
		if (hd.pixel_format == PFFLOAT) {			\
			if (load(stdin, hd.orows, hd.ocols, hd.pixel_format, \
							hd.sizepix, \
					floatbuf, PFFLOAT, 32) < 0)	\
				error("load failure");			\
			scale(floatbuf, imagebuf);			\
		}							\
		else {							\
			if (load(stdin, hd.orows, hd.ocols, hd.pixel_format, \
							hd.sizepix, \
					imagebuf, PFBYTE, FBdepth) < 0)	\
				error("load failure");			\
		}							\
	{								\
	int informat = hd.pixel_format;					\
									\
	hd.pixel_format = PFBYTE;					\
	if (to_sun(&hd, imagebuf, &rh, (colormap_t *)0,			\
			mpr_d(image_in_window)->md_image, Magfactor) < 0) \
		error("problem converting hips image");			\
	hd.pixel_format = informat;					\
	}								\
	free(imagebuf);							\
	}								\
	pr_rop(backup_image, 0, 0, rh.ras_width, rh.ras_height,		\
					PIX_SRC, image_in_window, 0, 0); \
	pw_write(imgpw, 0, 0, rh.ras_width, rh.ras_height,		\
				PIX_SRC, image_in_window, 0, 0); \
	if (bitwin_on)							\
		killfatbits();						\
	break;								\
		case 'N':						\
			paintcolor = color;				\
			break;						\
		case 'P':						\
			paintcolor++;					\
			if (paintcolor > 255)				\
				paintcolor -= 256;			\
			break;						\
		case 'M':						\
			paintcolor--;					\
			if (paintcolor < 0)				\
				paintcolor += 256;			\
			break;						\
		case PAINTBOX:						\
			paintbox(paintcolor);				\
			break;						\
		case INITBOX:						\
			if (box_is_on)					\
				erase_box();				\
			init_box(hd.ocols, hd.orows);                     \
			break;						\
		case SIZEBOX:						\
			{						     \
			int size_framewidth = (int) window_get(size_frame,   \
								WIN_WIDTH);  \
			int toolwidth = (int) window_get(tool, WIN_WIDTH);   \
									     \
			window_set(size_frame,				     \
					WIN_X,	toolwidth - size_framewidth, \
					WIN_Y,	0,	     \
					0,	0);			     \
									     \
			window_set(size_frame, WIN_SHOW, TRUE, 0, 0);	     \
			}						     \
			break;						\
		case RESTORE:						\
			pr_rop(image_in_window, 0, 0,			\
					rh.ras_width, rh.ras_height,	\
					PIX_SRC, backup_image, 0, 0);	\
			pw_write(imgpw, 0, 0,				\
					rh.ras_width, rh.ras_height,	\
					PIX_SRC, image_in_window,	\
					0, 0);				\
			if (bitwin_on) {				\
				paint_bitwin();				\
				if (locator)				\
					draw_locator();			\
			}						\
			if (box_is_on && !blinking)			\
				draw_box();				\
			break;						\
		case LUTSWEEP:						\
			if (lthresh < uthresh)				\
				sweepfunc(++lthresh);			\
			break;						\
		case LUTSWEEP_DOWN:					\
			if (lthresh < uthresh)				\
				sweepfunc_down(--uthresh);		\
			break;						\
		case RESTORE_LUT:	/* to original state */		\
			if (ranluts_are_on())				\
				break;					\
			exponent = 1.;					\
			lthresh = 0;					\
			uthresh = 255;					\
			if (colorentries)				\
				setupcolormap(colorbuf);		\
			else						\
				restore_lut();				\
			break;						\
		case RANLUTS:						\
			ranluts();					\
			break;						\
		case RANLUTS_OFF:					\
			ranluts_off();					\
			break;						\
		case LOC_RGNENTER:					\
			putluts();					\
			break;						\
		case WRITE:						\
	/* write current image to stdout or to specified file		\
	 with current powerfunc */ 					\
			fileproc = writeout;				\
			if (isatty(fileno(fp))) {			\
				toolheight = (int) window_get(tool,	\
							WIN_HEIGHT);	\
									\
				window_set(filename_frame,		\
						WIN_X,	0,		\
						WIN_Y,	toolheight,	\
						0,	0);		\
									\
				window_set(filename_frame,		\
						WIN_SHOW, TRUE,		\
						0, 0);			\
			}						\
			else						\
				if (writeout(fp) == -1)			\
					fprintf(stderr,			\
					"%s: writeout failure\n",	\
								av[0]);	\
			break;						\
		case WRITEBOX:						\
	/* write current box to stdout or to specified file		\
	 with current powerfunc */ 					\
			fileproc = writeout_box;			\
			if (isatty(fileno(fp))) {			\
				toolheight = (int) window_get(tool,	\
							WIN_HEIGHT);	\
									\
				window_set(filename_frame,		\
						WIN_X,	0,		\
						WIN_Y,	toolheight,	\
						0,	0);		\
									\
				window_set(filename_frame,		\
						WIN_SHOW, TRUE,		\
						0, 0);			\
			}						\
			else						\
				if (writeout_box(fp) == -1)		\
					fprintf(stderr,			\
					"%s: writeout failure\n",	\
								av[0]);	\
			break;						\
		case FORMAT:						\
			if (Outformat == INFORMAT)			\
				Outformat = FORCEGRLE;			\
			else						\
				Outformat = INFORMAT;			\
			break;						\
		case FILTER1:						     \
		case FILTER2:						     \
			{						     \
			int cmd_framewidth = (int) window_get(command_frame, \
								WIN_WIDTH);  \
			int toolwidth = (int) window_get(tool, WIN_WIDTH);   \
									     \
			toolheight = (int) window_get(tool, WIN_HEIGHT);     \
									     \
			window_set(command_frame,			     \
					WIN_X,	toolwidth - cmd_framewidth,  \
					WIN_Y,	toolheight,		     \
					0,	0);			     \
									     \
			window_set(command_frame, WIN_SHOW, TRUE, 0, 0);     \
			}						     \
			break;						     \
		case UNDO:						\
			undo();						\
			break;						\
		case '=':						\
		case '+':						\
			if (ranluts_are_on())				\
				break;					\
			exponent += 0.05;				\
			powerfunc();					\
			break;						\
		case '-':						\
			if (ranluts_are_on())				\
				break;					\
			exponent -= 0.05;				\
			powerfunc();					\
			break;						\
		case LOCATOR:						\
			if (locator) {					\
				erase_locator();			\
				locator = FALSE;			\
			}						\
			else {						\
				if (bitwin_on)				\
					draw_locator();			\
				locator = TRUE;				\
			}						\
			break;						\
		case KILLFATBITS:					\
			if (bitwin_on) {				\
				if (locator)				\
					erase_locator();		\
				killfatbits();				\
			}						\
			break;						\
		case ZOOMIN:						\
			if (bitwin_on) {				\
				if (locator)				\
					erase_locator();		\
				Bitsize *= 2;				\
				paint_bitwin();				\
				if (locator)				\
					draw_locator();			\
			}						\
			break;						\
		case ZOOMOUT:						\
			if ((bitwin_on) && (Bitsize > 1)) {		\
				if (locator)				\
					erase_locator();		\
				Bitsize /= 2;				\
				paint_bitwin();				\
				if (locator)				\
					draw_locator();			\
			}						\
			break;						\
		case BLINK:						\
			if (blinking) {					\
				blink_off();				\
				box_is_on = TRUE;			\
				draw_box();				\
				if (bitwin_on && locator)		\
					draw_locator();			\
			}						\
			else						\
				blink_on();				\
			break;						\
		case QUIT:	/* quit the tool */			\
			exit(0);					\
			break;						\
		case READINOP:						\
			if (readin_op == PIX_SRC)			\
				readin_op = PIX_SRC ^ PIX_DST;		\
			else						\
				readin_op = PIX_SRC;			\
			break;						\
		case LUTFRAME:						\
			if (lutframe_is_on)				\
				window_set(lut_frame,			\
					WIN_SHOW, FALSE,		\
					0, 0);				\
			else						\
				window_set(lut_frame,			\
					WIN_SHOW, TRUE,			\
					0, 0);				\
			lutframe_is_on = !lutframe_is_on;		\
			break;						\
		case CURSOROP:						\
			switch (cursorstate) {				\
			case NORMAL:					\
				cursor_set(main_cursor,			\
				CURSOR_VERT_HAIR_OP,	PIX_SRC|PIX_COLOR(255),\
				CURSOR_HORIZ_HAIR_OP, 	PIX_SRC|PIX_COLOR(255),\
					0);				\
				cursorstate = WHITE;			\
				break;					\
			case WHITE:					\
				cursor_set(main_cursor,			\
				CURSOR_VERT_HAIR_OP,	PIX_SRC&PIX_COLOR(0), \
				CURSOR_HORIZ_HAIR_OP, 	PIX_SRC&PIX_COLOR(0), \
					0);				\
				cursorstate = BLACK;			\
				break;					\
			case BLACK:					\
				cursor_set(main_cursor,			\
				CURSOR_VERT_HAIR_OP,	PIX_SRC^PIX_DST, \
				CURSOR_HORIZ_HAIR_OP, 	PIX_SRC^PIX_DST, \
					0);				\
				cursorstate = NORMAL;			\
				break;					\
			}						\
			window_set(imgsw, WIN_CURSOR, main_cursor, 0, 0); \
			window_set(bitsw, WIN_CURSOR, main_cursor, 0, 0); \
			break;
COMMON_CHOICES
 case SETBKGD:						
  {						     
    int bkgd_framewidth = (int) window_get(bkgd_frame,   
					   WIN_WIDTH);  
    int toolwidth = (int) window_get(tool, WIN_WIDTH);   
    
    window_set(bkgd_frame,				     
	       WIN_X,	toolwidth - bkgd_framewidth, 
	       WIN_Y,	0,	     
	       0,	0);			     
    
    window_set(bkgd_frame, WIN_SHOW, TRUE, 0, 0);	     
  }						     
		  break;						
		case BRUSH:
		  if ((image_mode == BRUSHMODE)
		      ||  (image_mode == UNBRUSHMODE)) {
		      if (inshift)
			image_mode = SHIFTMODE;
		      else
			image_mode = NULLMODE;
		      window_set(imgsw,
				 WIN_CURSOR, main_cursor,
				 0, 0);
		      window_set(imgsw,
				 WIN_IGNORE_PICK_EVENT, LOC_TRAJECTORY,
				 0, 0);
		    }
		  else {
		      if (image_mode == SHIFTMODE)
			inshift = TRUE;
		      else
			inshift = FALSE;
		      image_mode = BRUSHMODE;
		      window_set(imgsw,
				 WIN_CURSOR, brush_cursor,
				 0, 0);
		      window_set(imgsw,
				 WIN_CONSUME_PICK_EVENT, LOC_TRAJECTORY,
				 0, 0);
		      /* unbrush init: */
		      pr_rop(brushsave, 0, 0,
			     rh.ras_width, rh.ras_height, PIX_SRC,
			     image_in_window, 0, 0);
		    }
		  break;
		case CHANGEBRUSH:
		  if (stencil_pixrect == &square_brush_pixrect)
		    stencil_pixrect = &diamond_brush_pixrect;
		  else if (stencil_pixrect == &diamond_brush_pixrect)
		    stencil_pixrect = &point_brush_pixrect;
		  else
		    stencil_pixrect = &square_brush_pixrect;
		  cursor_set(brush_cursor,
			     CURSOR_IMAGE, stencil_pixrect, 0);
		  if ((image_mode == BRUSHMODE)
		      ||  (image_mode == UNBRUSHMODE))
		    window_set(imgsw,
			       WIN_CURSOR, brush_cursor,
			       0, 0);
		  break;
		case MS_LEFT:
		  switch (image_mode) {
		    case BRUSHMODE:
		    case UNBRUSHMODE:
		      image_mode = BRUSHMODE;
		      brush(big(x), big(y), paintcolor);
		      break;
		    case SHIFTMODE:
		      break;
		    default:
		      image_mode = MOVEMODE;
		      break;
		    }
		  break;
		case MS_MIDDLE:
		  switch (image_mode) {
		    case BRUSHMODE:
		    case UNBRUSHMODE:
		      image_mode = UNBRUSHMODE;
		      unbrush(big(x), big(y));
		      break;
		    case SHIFTMODE:
		      image_mode = NULLMODE;
		      window_set(message_frame,
				 WIN_SHOW, FALSE,
				 0, 0);
		      break;
		    default:
		      image_mode = STRETCHMODE;
		      break;
		    }
		  break;
		case LOC_DRAG:
		  switch (image_mode) {
		    case BRUSHMODE:
		      brush(big(x), big(y), paintcolor);
		      break;
		    case UNBRUSHMODE:
		      unbrush(big(x), big(y));
		      break;
		    case MOVEMODE:
		      Moved = TRUE;
		      move_box(x, y);
		      break;
		    case STRETCHMODE:
		      Moved = TRUE;
		      stretch_box(big(x) - 1,
				  big(y) + Magfactor - 1);
		      break;
		    case SHIFTMODE:
		      Shifted = TRUE;
		      shift(big(x), big(y));
		      break;
		    default:
		      break;
		    }
		  break;
		case LOC_STILL:
		  break;
		case READIN:
		  readin_start(x, y);
		  break;
		case CHANGEMENU:
		  if (displaymenu == longmenu)
		    displaymenu = shortmenu;
		  else
		    displaymenu = longmenu;
		  break;
		case FATBITS:
		  fatbits(x, y);
		  if (locator)
		    draw_locator();
		  break;
		default:
		  break;
		}
  
  if (Moved			/* box has been moved or resized */
      &&  !(int)window_get(imgsw, WIN_EVENT_STATE, MS_LEFT)
      /* left button is not down */
      &&  !(int)window_get(imgsw, WIN_EVENT_STATE, MS_MIDDLE))
    /* middle button is not down */ {
	Moved = FALSE;
	adjust_box();
      }
  
  if (Shifted
      &&  !(int)window_get(imgsw, WIN_EVENT_STATE, MS_LEFT)
      /* left button is not down */
      &&  !(int)window_get(imgsw, WIN_EVENT_STATE, MS_MIDDLE))
    /* middle button is not down */ {
	Shifted = FALSE;
	adjust_box();
	fillin_box();
      }
  
  if (replace_cursor)
    window_set(imgsw, WIN_MOUSE_XY, x, y, 0);
}

/* //////////////////////////////////////////////////////////////////////// */

paintbox(color)
int	color;
{
	undo_init();

	pr_rop(image_in_window, box_xposition + 1, box_yposition + 1,
		box_xlength - 1, box_ylength - 1,
		PIX_SRC | PIX_COLOR(color),
		(Pixrect *)0, 0, 0);

	pw_write(imgpw, box_xposition + 1, box_yposition + 1,
		box_xlength - 1, box_ylength - 1,
		PIX_SRC,
		image_in_window, box_xposition + 1, box_yposition + 1);

	if (bitwin_on) {
		int i, j;

		pw_batch_on(bitpw);
		for (i = box_xposition + 1; i < box_xposition + box_xlength;
									i++)
			if ((i >= bitwin_centerx - 1 - BITPIXWD/2)
			&&  (i <= bitwin_centerx + 1 + BITPIXWD/2))
				for (j = box_yposition + 1;
						j < box_yposition + box_ylength;
									j++)
					if ((j >= bitwin_centery
							- 1 - BITPIXHT/2)
					&&  (j <= bitwin_centery
							+ 1 + BITPIXHT/2))
						paintfatbit(
			bitwin_centerBcol + (i - bitwin_centerx) * Bitsize,
			bitwin_centerBrow + (j - bitwin_centery) * Bitsize,
			pw_get(imgpw, i, j));

		pw_batch_off(bitpw);
		bitwin_marks();
	}

	if (box_is_on && !blinking)
		draw_box();
}

/* //////////////////////////////////////////////////////////////////////// */

static int	rbrow, rbcol;
static int	rrow, rcol;

brush(x, y, color)
int	x, y, color;
{
	if ((stencil_pixrect == &point_brush_pixrect)
	||  (Magfactor > 1)) {
		pr_rop(image_in_window, x, y,
			Magfactor, Magfactor,
			PIX_SRC | PIX_COLOR(color),
			(Pixrect *)0, 0, 0);

		pw_write(imgpw, x, y,
			Magfactor, Magfactor,
			PIX_SRC,
			image_in_window, x, y);

		if (bitwin_on) {
			paintbigfatbit(
				rbcol + (x - rcol) * Bitsize,
				rbrow + (y - rrow) * Bitsize,
				color);

			bitwin_marks();
		}

		return;
	}

	pr_stencil(image_in_window, x - Brushradius, y - Brushradius,
		2 * Brushradius, 2 * Brushradius,
		PIX_SRC | PIX_COLOR(color),
		stencil_pixrect, 0, 0,
		(Pixrect *)0, 0, 0);

	pw_write(imgpw, x - Brushradius, y - Brushradius,
		2 * Brushradius, 2 * Brushradius,
		PIX_SRC,
		image_in_window, x - Brushradius, y - Brushradius);

	if (bitwin_on
	&&  x - Brushradius <= bitwin_centerx + 1 + BITPIXWD/2
	&&  y - Brushradius <= bitwin_centery + 1 + BITPIXHT/2
	&&  x + Brushradius > bitwin_centerx - 1 - BITPIXWD/2
	&&  y + Brushradius > bitwin_centery - 1 - BITPIXHT/2) {
		register int	i, j;
		register h_boolean ibottom = FALSE, itop = TRUE;
		register h_boolean jbottom, jtop;
		register int	bx = bitwin_centerBcol
						- bitwin_centerx * Bitsize,
				by = bitwin_centerBrow
						- bitwin_centery * Bitsize;

		pw_batch_on(bitpw);
		for (i = x - Brushradius; i < x + Brushradius; i++) {
			if (!ibottom
			&&  i >= bitwin_centerx - 1 - BITPIXWD/2)
				ibottom = TRUE;

			if (itop
			&&  i > bitwin_centerx + 1 + BITPIXWD/2)
				itop = FALSE;

			if (ibottom && itop) {
				jbottom = FALSE, jtop = TRUE;
				for (j = y - Brushradius; j < y + Brushradius;
									j++) {
					if (!jbottom
					&&  j >= bitwin_centery
							- 1 - BITPIXHT/2)
						jbottom = TRUE;

					if (jtop
					&&  j > bitwin_centery
							+ 1 + BITPIXHT/2)
						jtop = FALSE;

					if (jbottom && jtop)
						paintfatbit(
							bx + i * Bitsize,
							by + j * Bitsize,
							pw_get(imgpw, i, j));
				}
			}
		}
		pw_batch_off(bitpw);

		bitwin_marks();
	}

	if (box_is_on && !blinking)
		draw_box();
}

/* //////////////////////////////////////////////////////////////////////// */

unbrush(x, y)
int	x, y;
{
	if ((stencil_pixrect == &point_brush_pixrect)
	||  (Magfactor > 1)) {
		pr_rop(image_in_window, x, y,
			Magfactor, Magfactor,
			PIX_SRC,
			brushsave, x, y);

		pw_write(imgpw, x, y,
			Magfactor, Magfactor,
			PIX_SRC,
			image_in_window, x, y);

		if (bitwin_on) {
			paintbigfatbit(
				rbcol + (x - rcol) * Bitsize,
				rbrow + (y - rrow) * Bitsize,
				pw_get(imgpw, x, y));

			bitwin_marks();
		}

		return;
	}

	pr_stencil(image_in_window, x - Brushradius, y - Brushradius,
		2 * Brushradius, 2 * Brushradius,
		PIX_SRC,
		stencil_pixrect, 0, 0,
		brushsave,
		x - Brushradius, y - Brushradius);

	pw_write(imgpw,
		x - Brushradius, y - Brushradius,
		2 * Brushradius, 2 * Brushradius,
		PIX_SRC,
		image_in_window,
		x - Brushradius, y - Brushradius);

	if (bitwin_on) {
		int i, j;

		pw_batch_on(bitpw);
		for (i = x - Brushradius; i < x + Brushradius; i++)
			if ((i >= bitwin_centerx - 1 - BITPIXWD/2)
			&&  (i <= bitwin_centerx + 1 + BITPIXWD/2))
				for (j = y - Brushradius; j < y + Brushradius;
									j++)
					if ((j >= bitwin_centery
							- 1 - BITPIXHT/2)
					&&  (j <= bitwin_centery
							+ 1 + BITPIXHT/2))
						paintfatbit(
			bitwin_centerBcol + (i - bitwin_centerx) * Bitsize,
			bitwin_centerBrow + (j - bitwin_centery) * Bitsize,
			pw_get(imgpw, i, j));
		pw_batch_off(bitpw);

		bitwin_marks();
	}

	if (box_is_on && !blinking)
		draw_box();
}

/* //////////////////////////////////////////////////////////////////////// */

fatbits(x, y)
  int	x, y;
{
  window_set(tool, WIN_WIDTH, 2 * imgwincols + 3 * TOOL_BORDERWIDTH, 0, 0);
  if (bitwin_on && locator)
    erase_locator();
  bitwin_centerx = x, bitwin_centery = y;
  paint_bitwin();
  draw_box();
  window_set(bitsw, WIN_SHOW, TRUE, 0, 0);
  window_fit(imgsw);
  window_fit(bitsw);
  window_fit(tool);
  bitwin_on = TRUE;
  if (locator)
    draw_locator();
}

/* //////////////////////////////////////////////////////////////////////// */

killfatbits()
{
/*
*	Don't do this; it screws up the image pixwin:
*
*	window_set(tool,
*			WIN_WIDTH,	imgwincols + 2 * TOOL_BORDERWIDTH,
*			0, 0);
*/
	window_set(bitsw, WIN_SHOW, FALSE, 0, 0);
	window_fit(imgsw);
	window_fit(tool);
	bitwin_on = FALSE;
}

/* //////////////////////////////////////////////////////////////////////// */

Panel_setting filename_proc(item, event)
  Panel_item	item;
  Event		*event;
{
  static char	filename[255];
  FILE *fp;
  int		/*fd, */ readin();
  
  strcpy(filename, (char *)panel_get_value(filename_item));
  
  if (!strcmp(filename, "")) {
      window_set(filename_frame, WIN_SHOW, FALSE, 0, 0);
      if (fileproc == readin) {
	  image_mode = NULLMODE;
	  window_set(message_frame, WIN_SHOW, FALSE, 0, 0);
	}
      return(panel_text_notify(item, event));
    }
  
  if (filename[0] == '~') {
      char *home, *getenv(), *strchr();
      char temp[255];
      
      if (filename[1] == '/')
	home = getenv("HOME");
      else {
#include <pwd.h>
	  strcpy(temp, filename + 1);
	  *(strchr(temp, '/')) = '\0';
	  home = (getpwnam(temp))->pw_dir;
	}
      strcpy(temp, strchr(filename, '/'));
      strcpy(filename, home);
      strcat(filename, temp);
    }
  
  if (fileproc == readin) {
      if (!(fp = fopen(filename, "r")))
	perr(HE_OPEN, filename);
      else if (fileproc(fp, cursorx, cursory) == -1)
	fclose(fp);
      else {
	  fclose(fp);
	  window_set(filename_frame, WIN_SHOW, FALSE, 0, 0);
	}
    }
  
  else {
      if (!(fp = fopen(filename, "w")))
/*  if ((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1)*/
	perr(HE_OPEN, filename);
      else if (fileproc(fp) == -1)
	fclose(fp);
      else {
	  fclose(fp);
	  window_set(filename_frame, WIN_SHOW, FALSE, 0, 0);
	}
    }
  
  return(panel_text_notify(item, event));
}


/* //////////////////////////////////////////////////////////////////////// */

Panel_setting	command_proc(item, event)
Panel_item	item;
Event		*event;
{
char	commandline[1024];

	strcpy(commandline, (char *)panel_get_value(command_item));
	if (strcmp(commandline, ""))
		filter(commandline, &rh);
	window_set(command_frame, WIN_SHOW, FALSE, 0, 0);
	return(panel_text_notify(item, event));
}

/* //////////////////////////////////////////////////////////////////////// */

Panel_setting	size_proc(item, event)
Panel_item	item;
Event		*event;
{
char	dimensions[1024];
int	x, y;

	strcpy(dimensions, (char *)panel_get_value(size_item));
	sscanf(dimensions, "%d %d", &x, &y);
	setboxsize(x * Magfactor, y * Magfactor);
	window_set(size_frame, WIN_SHOW, FALSE, 0, 0);
	return(panel_text_notify(item, event));
}

/* //////////////////////////////////////////////////////////////////////// */

Panel_setting	bkgd_proc(item, event)
  Panel_item	item;
  Event		*event;
{
  char	colors[1024];
  int	r, g, b;
  
  strcpy(colors, (char *)panel_get_value(bkgd_item));
  sscanf(colors, "%d %d %d", &r, &g, &b);
  set_background(r, g, b);
  window_set(bkgd_frame, WIN_SHOW, FALSE, 0, 0);
  return(panel_text_notify(item, event));
}


/* //////////////////////////////////////////////////////////////////////// */

readin(fp, x, y)
  FILE *fp;
  int	/*fd, */ x, y;
{
  struct header		myhd;
  u_char		*readbuf;
  struct rasterfile	temprh;
  int delta_x, delta_y;
  static h_boolean	firsttime = TRUE;
  
  if (firsttime)
    firsttime = FALSE;
  else
    pr_destroy(readin_image);
  
  fread_header(fp, &myhd, "inputfile");
  sethdrsize(&myhd);     /* ensure that sizepix field is correctly set */
  
  if ((readbuf = (u_char *) calloc(myhd.orows * myhd.ocols, 
				   sizeof(u_char))) == NULL)
    error("can't allocate core");
  if (load(fp, myhd.orows, myhd.ocols, myhd.pixel_format,
	   myhd.sizepix, readbuf, PFBYTE, hd.sizepix) < 0) {
      fprintf(stderr, "%s: load failure\n", av[0]);
      free(readbuf);
      return(-1);
    }
  
  readin_image = mem_create(to32(myhd.ocols * Magfactor), 
			    myhd.orows * Magfactor,
			    myhd.sizepix);
  if (to_sun(&myhd, readbuf, &temprh, (colormap_t *)0,
	     mpr_d(readin_image)->md_image, Magfactor) < 0) {
      fprintf(stderr, "%s: to_sun failure\n", av[0]);
      free(readbuf);
      return(-1);
    }
  
  box_is_on = TRUE;
  
  erase_box();
/*  box_xposition = big(x + (Magfactor + 1)/2) - 1;
  box_yposition = big(y + (Magfactor + 1)/2) - Magfactor * myhd.orows - 1;
  box_xposition = delta_x - 1;
  box_yposition = delta_y - 1; */

/*  box_xlength = Magfactor * myhd.ocols + 1;
  box_ylength = Magfactor * myhd.orows + 1; */
  draw_box();
  
  undo_init();
  
/*  pw_write(imgpw, big(x + (Magfactor + 1)/2),
	   big(y + (Magfactor + 1)/2) - Magfactor * myhd.orows, */
  pw_write(imgpw, box_xposition, box_yposition,
	   /* bottom left corner of image
	      read in at cursor position */
/*	   temprh.ras_width, temprh.ras_height, */
	   box_xlength, box_ylength,
	   readin_op, readin_image, 0, 0);
/*  pr_rop(image_in_window, big(x + (Magfactor + 1)/2),
	 big(y + (Magfactor + 1)/2) - Magfactor * myhd.orows, */
  pr_rop(image_in_window, box_xposition, box_yposition,
/*	 temprh.ras_width, temprh.ras_height, */
	 box_xlength, box_ylength,
	 readin_op, readin_image, 0, 0);
  free(readbuf);
  
  if ((bitwin_on)
      &&  (save_y + save_dy >= bitwin_centery - 1 - BITPIXHT/2)
      &&  (save_y <= bitwin_centery + 1 + BITPIXHT/2)
      &&  (save_x + save_dx >= bitwin_centerx - 1 - BITPIXWD/2)
      &&  (save_x <= bitwin_centerx + 1 + BITPIXWD/2))
    paint_bitwin();
  
  return(0);
}

/* //////////////////////////////////////////////////////////////////////// */

writeout(fp)
  FILE *fp;
/*  int	fd; */
{
  u_char *image_out;
  char	string[30];
  char	*savehist, *history = hd.seq_history;
  int	size = hd.orows * hd.ocols * sizeof(u_char);
  int	pformat = hd.pixel_format;
  int	numframes = hd.num_frame;
  
  if (((image_out = (u_char *) malloc(size)) == NULL)
      ||  ((savehist = (char *) malloc((strlen(history) + 1) * sizeof(char)))
	   == NULL)) {
      fprintf(stderr, "%s: can't allocate core\n", av[0]);
      return(-1);
    }
  strcpy(savehist, history);
  
  if (box_is_on)
    erase_box();
  
  pw_read(image_in_window, 0, 0,
	  rh.ras_width, rh.ras_height, PIX_SRC,
	  imgpw, 0, 0);
  
  if (box_is_on && !blinking)
    draw_box();
  
  to_hips(&rh, (colormap_t *)0, mpr_d(image_in_window)->md_image, &hd,
	  image_out, Magfactor);
  sprintf(string, "(exponent %.2lf", exponent);
  if (hd.num_frame > 1)
    sprintf(string + strlen(string), "; frame %d", Framenumber);
  sprintf(string + strlen(string), ")");
  av[ac - 1] = string;
  
  update_header(&hd, ac, av);
  
  hd.pixel_format		= PFBYTE;
  
#ifdef	GRLE
  
  if (Outformat == FORCEGRLE)
    hd.pixel_format = PFGRLE;
  
#endif
  
  hd.num_frame		= 1;
  if (FBdepth == 1)
#ifdef MSBFVERSION
    hd.pixel_format = PFMSBF;
#else
  hd.pixel_format = PFLSBF;
#endif MSBFVERSION
  
  if (colorentries == 0)
    output_lutfunc(image_out);
  
  fwrite_header(fp, &hd, "outputfile");
  if (store(fp, hd.orows, hd.ocols, hd.pixel_format, 
	    hd.sizepix, image_out, PFBYTE, FBdepth) < 0) {
      fprintf(stderr, "%s: store failure\n", av[0]);
      hd.pixel_format = pformat;
      return(-1);
    }
  
  hd.seq_history = savehist;
  hd.pixel_format = pformat;
  hd.num_frame = numframes;
  
  return(0);
}

/* //////////////////////////////////////////////////////////////////////// */

/*ARGSUSED*/
void redraw_bitwin(canvas, pw, repaint_area)
Canvas		canvas;
Pixwin		*pw;
Rectlist	*repaint_area;
{
	bitwincols = (int) window_get(canvas, CANVAS_WIDTH);
	bitwinrows = (int) window_get(canvas, CANVAS_HEIGHT);
	paint_bitwin();
}

/* //////////////////////////////////////////////////////////////////////// */

paint_bitwin()
{
  register int	row, col, Bcol, Brow, fromx, fromy;
  int		color, Bcol_start, colstart;
  
  /* center bitwin at bitwin_centerx, bitwin_centery */
  
  colstart = - 1 - BITPIXWD/2;
  fromx = bitwin_centerx + colstart;
  while (fromx % Magfactor)
    fromx--, colstart--;
  Bcol_start = (bitwincols - Bitsize)/2 + colstart * Bitsize;
  row = - 1 - BITPIXHT/2;
  fromy = bitwin_centery + row;
  while (fromy % Magfactor)
    fromy--, row--;
  Brow = (bitwinrows - Bitsize)/2 + row * Bitsize;
  pw_batch_on(bitpw);
  for (;
       row <= 1 + BITPIXHT/2;
       row += Magfactor, Brow += Magfactor * Bitsize) {
      fromy = bitwin_centery + row;
      for (col = colstart, Bcol = Bcol_start;
	   col <= 1 + BITPIXWD/2;
	   col += Magfactor, Bcol += Magfactor * Bitsize) {
	  fromx = bitwin_centerx + col;
	  color = pr_get(image_in_window, fromx, fromy);
	  paintbigfatbit(Bcol, Brow, color);
	  if (row <= 0
	      &&  row > -Magfactor
	      &&  col <= 0
	      &&  col > -Magfactor) {
	      bitwin_centerBrow = Brow - row * Magfactor;
	      bitwin_centerBcol = Bcol - col * Magfactor;
	      
	      rbrow = Brow, rbcol = Bcol;
	      rrow = fromy, rcol = fromx;
	    }
	}
    }
  pw_batch_off(bitpw);
  
  bitwin_marks();
  if (box_is_on)
    draw_box_in_bitwin();
}

/* //////////////////////////////////////////////////////////////////////// */

bitwin_marks()
{
int	rbitsize = Magfactor * Bitsize;	/* side of a fat bit
							in real screen pixels */

	/* put little cursor marks at the center of each edge of the
	bitwin pointing at the center (bitwin_centerx, bitwin_centery): */

	pw_batch_on(bitpw);

	pw_vector(bitpw, bitwincols/2, 0,
			bitwincols/2,	4,
			PIX_NOT(PIX_SRC) ^ PIX_DST,
			0);
	pw_vector(bitpw, bitwincols/2, bitwinrows - 5,
			bitwincols/2,	bitwinrows - 1,
			PIX_NOT(PIX_SRC) ^ PIX_DST,
			0);
	pw_vector(bitpw, 0, bitwinrows/2,
			4,	bitwinrows/2,
			PIX_NOT(PIX_SRC) ^ PIX_DST,
			0);
	pw_vector(bitpw, bitwincols - 5, bitwinrows/2,
			bitwincols - 1,	bitwinrows/2,
			PIX_NOT(PIX_SRC) ^ PIX_DST,
			0);

	/* outline the center fatbit in bitwin: */

	pw_vector(bitpw, rbcol, rbrow,
			rbcol + rbitsize - 1, rbrow,
			PIX_SRC, 255);
	pw_vector(bitpw, rbcol, rbrow,
			rbcol, rbrow + rbitsize - 1,
			PIX_SRC, 255);
	pw_vector(bitpw, rbcol, rbrow + rbitsize - 1,
			rbcol + rbitsize - 1, rbrow + rbitsize - 1,
			PIX_SRC, 255);
	pw_vector(bitpw, rbcol + rbitsize - 1, rbrow,
			rbcol + rbitsize - 1, rbrow + rbitsize - 1,
			PIX_SRC, 255);


	pw_batch_off(bitpw);

	sprintf(coordposition, " %d %d",
				hipsx(bitwin_centerx), hipsy(bitwin_centery));
	window_set(tool, FRAME_LABEL, labelname, 0, 0);
}

/* //////////////////////////////////////////////////////////////////////// */

paintfatbit(Bcol, Brow, color)
int	Bcol, Brow, color;
{
	if (hd.pixel_format == PFSRLE)
		pw_writebackground(bitpw, Bcol, Brow,
				Bitsize, Bitsize,
				color ? PIX_SET : PIX_CLR);
	else
		pw_write(bitpw, Bcol, Brow,
			Bitsize, Bitsize,
			PIX_SRC | PIX_COLOR(color),
			(Pixrect *)0, 0, 0);
}

/* //////////////////////////////////////////////////////////////////////// */

paintbigfatbit(Bcol, Brow, color)
  int	Bcol, Brow, color;
{
  if (hd.pixel_format == PFSRLE)
    pw_writebackground(bitpw, Bcol, Brow,
		       Magfactor * Bitsize, Magfactor * Bitsize,
		       color ? PIX_SET : PIX_CLR);
  else
    pw_write(bitpw, Bcol, Brow,
	     Magfactor * Bitsize, Magfactor * Bitsize,
	     PIX_SRC | PIX_COLOR(color),
	     (Pixrect *)0, 0, 0);
}

/* //////////////////////////////////////////////////////////////////////// */

/*ARGSUSED*/
void bits_selected(window, event, arg)
  Window	window;
  Event	*event;
  caddr_t	arg;
{
  FILE          *fp = stdout;
  int		x, y, Brow, Bcol, col, row, fromx, fromy;
  int		color;
  static int	paintcolor = 0;
  short		code;
  caddr_t       menu_show();
  static h_boolean paintmode = FALSE;
  int		toolheight;
  h_boolean	replace_cursor = FALSE;
  
  x = event_x(event);
  y = event_y(event);
  
  row = - 1 - BITPIXHT/2;
  fromy = bitwin_centery + row;
  while (fromy % Magfactor)
    fromy--, row--;
  Brow = (bitwinrows - Bitsize)/2 + row * Bitsize;
  col = - 1 - BITPIXWD/2;
  fromx = bitwin_centerx + col;
  while (fromx % Magfactor)
    fromx--, col--;
  Bcol = (bitwincols - Bitsize)/2 + col * Bitsize;
  while (Brow <= y - Magfactor * Bitsize)
    Brow += Magfactor * Bitsize;
  while (Bcol <= x - Magfactor * Bitsize)
    Bcol += Magfactor * Bitsize;
  
  col = rcol + (Bcol - rbcol)/Bitsize;
  row = rrow + (Brow - rbrow)/Bitsize;
  
  /* now (Bcol, Brow) is the lower left corner of the bigfat bit we're in;
     and (col, row) is the pixel in the real image corresponding to
     the fat bit. */
  
  color = pw_get(bitpw, x, y);
  if (hd.pixel_format == PFFLOAT)
    sprintf(colorstring, "x %d y %d (%f)", hipsx(col), hipsy(row),
	    *(floatbuf + hd.ocols * hipsy(row) + hipsx(col)));
  else
    sprintf(colorstring, "x %d y %d (%d)", hipsx(col), hipsy(row),
	    color);
  
  if ((code = event_id(event)) == MS_RIGHT
      &&  event_is_down(event)) {
      code = (short) (menu_show(bitmenu, bitsw, event, 0));
      paintmode = FALSE;
      replace_cursor = TRUE;
    }
  switch (code) {
      COMMON_CHOICES
      case MS_MIDDLE:
	paintmode = FALSE;
      break;
    case MS_LEFT:
      paintmode = TRUE;
    case LOC_DRAG:
      if (paintmode) {
	  pr_rop(image_in_window,
		 col, row, Magfactor, Magfactor,
		 PIX_SRC | PIX_COLOR(paintcolor),
		 (Pixrect *)0, 0, 0);
	  pw_write(imgpw,
		   col, row, Magfactor, Magfactor,
		   PIX_SRC,
		   image_in_window, col, row);
	  paintbigfatbit(Bcol, Brow, paintcolor);
	}
      break;
    case FATBITS:
      fatbits(col + Magfactor/2, row + Magfactor/2);
      if (locator)
	draw_locator();
      break;
    case READIN:
      readin_start(col, row);
      break;
    default:
      break;
    }
  
  if (replace_cursor)
    window_set(bitsw, WIN_MOUSE_XY, x, y, 0);
}


/* //////////////////////////////////////////////////////////////////////// */

#define	NONE	0
#define	LOWER	1
#define	UPPER	2

/*ARGSUSED*/
void lut_selected(window, event, arg)
  Window	window;
  Event	*event;
  caddr_t	arg;
{
  short		code;
  int		x, y;
  static int	state = NONE;
  caddr_t		menu_show();
  
  x	= event_x(event);
  y	= 255 - event_y(event);
  
  sprintf(lutstring, "x %d y %d", x, y);
  
  if ((code = event_id(event)) == MS_RIGHT
      &&  event_is_down(event)) {
      code = (short) (menu_show(lutmenu, lutsw, event, 0));
    }
  
  switch (code) {
    case '=':
    case '+':
      if (ranluts_are_on())
	break;
      exponent += 0.05;
      powerfunc();
      break;
    case '-':
      if (ranluts_are_on())
	break;
      exponent -= 0.05;
      powerfunc();
      break;
    case RESTORE_LUT:	/* to original state */	
      if (ranluts_are_on())
	break;
      exponent = 1.;
      lthresh = 0;
      uthresh = 255;
      if (colorentries)
	setupcolormap(colorbuf);
      else
	restore_lut();
      break;
    case MS_LEFT:
      state = LOWER;
      if (x < lthresh)
	while (lthresh > x)
	  unsweepfunc(lthresh--);
      else
	while (lthresh < x)
	  sweepfunc(++lthresh);
      break;
    case MS_MIDDLE:
      state = UPPER;
      if (x > uthresh)
	while (uthresh < x)
	  unsweepfunc_down(uthresh++);
      else
	while (uthresh > x)
	  sweepfunc_down(--uthresh);
      break;
    case LOC_DRAG:
      switch (state) {
	case LOWER:
	  if (x < lthresh)
	    while (lthresh > x)
	      unsweepfunc(lthresh--);
	  else
	    while (lthresh < x)
	      sweepfunc(++lthresh);
	  break;
	case UPPER:
	  if (x > uthresh)
	    while (uthresh < x)
	      unsweepfunc_down(uthresh++);
	  else
	    while (uthresh > x)
	      sweepfunc_down(--uthresh);
	  break;
	default:
	  break;
	}
      break;
    case POWERPOINT:
      powerpoint(x, y);
      break;
    case STRETCHLUT:
      stretchfunc();
      break;
    case LOC_RGNENTER:
      putluts();
      break;
    default:
      break;
    }
}

/* //////////////////////////////////////////////////////////////////////// */

readin_start(x, y)
  int	x, y;
{
  int	toolheight;
  
  image_mode = SHIFTMODE;
  window_set(message_frame,
	     WIN_SHOW, TRUE,
	     0, 0);
  fileproc = readin;
  cursorx = x, cursory = y;
  toolheight = (int) window_get(tool, WIN_HEIGHT);
  
  window_set(filename_frame,
	     WIN_X,	0,
	     WIN_Y,	toolheight,
	     0,	0);
  
  window_set(filename_frame,
	     WIN_SHOW, TRUE,
	     0, 0);
}

/* //////////////////////////////////////////////////////////////////////// */

execute_command(hdr, insize, arrayin, outsize, arrayout, commandline)
  struct header	*hdr;
  int		insize, outsize;
  char		*arrayin, *arrayout;
  char		*commandline;
{
  int		pfdout[2], pfdin[2];
  int		bytesback = 0;
  static h_boolean firsttime = TRUE;
  static int	(*entrypipe)();
  h_boolean	wrong = FALSE;
  FILE *fp;
  
  pipe(pfdin);
  pipe(pfdout);
  switch (fork()) {
    case -1:
      perror("fork");
      close(pfdout[0]);
      close(pfdout[1]);
      close(pfdin[0]);
      close(pfdin[1]);
      return(0);
      break;
    case 0:
      close(0);
      dup(pfdout[0]);
      close(1);
      dup(pfdin[1]);
      close(pfdout[0]);
      close(pfdout[1]);
      close(pfdin[0]);
      close(pfdin[1]);
      execlp("/bin/sh", "sh", "-c", commandline, NULL);
      
      printf("/bin/sh not found\n");
      execlp("cat", "cat", NULL);
    }
  close(pfdout[0]);
  close(pfdin[1]);
  if (firsttime) {
      /* hek 19Dec90: what was the original intent here ????
	 signal is defined as: 
	 
	 void (*signal(sig, func))()
	 void (*func)();
	 
	 so why then the following:
	 
	 entrypipe = signal(SIGPIPE, SIG_IGN); */
      
      signal(SIGPIPE, SIG_IGN);
      firsttime = FALSE;
    }
  else
    signal(SIGPIPE, SIG_IGN);
  fp = fdopen(pfdout[1],"w");
  fwrite_header(fp, hdr, "<pipe>");
  if (fwrite(arrayin, insize,1,fp) != 1) {
      if (errno == EPIPE)
	wrong = TRUE;
      else
	fprintf(stderr, "%s: error during write to filter\n",
		av[0]);
    }
  fclose(fp);
  if (!wrong) {
      fp = fdopen(pfdin[0],"r");
      fread_header(fp, hdr, "<pipe>");
      bytesback = fread(arrayout,1,outsize,fp);
    }
  fclose(fp);
  
  /* hek 19Dec:
     is it possible that this was the intent of the above misunderstanding? */
  signal(SIGPIPE, signal(SIGPIPE, SIG_IGN));
  return(bytesback);
}

/* //////////////////////////////////////////////////////////////////////// */

init_box(cols, rows)
  int		cols, rows;
{
  box_xposition = box_yposition = -1;
  box_xlength = Magfactor * cols + 1;
  box_ylength = Magfactor * rows + 1;
}

/* //////////////////////////////////////////////////////////////////////// */

/*
* h_boolean boxpixel(x, y)
* {
* 	if ((x == box_xposition || x == box_xposition + box_xlength)
* 	&&  (y >= box_yposition && y <= box_yposition + box_ylength))
* 		return(TRUE);
* 	if ((y == box_yposition || y == box_yposition + box_ylength)
* 	&&  (x >= box_xposition && x <= box_xposition + box_xlength))
* 		return(TRUE);
* 	return(FALSE);
* }
*/

/* //////////////////////////////////////////////////////////////////////// */

draw_box()
{
  pw_vector(imgpw, box_xposition, box_yposition,
	    box_xposition + box_xlength, box_yposition,
	    PIX_SRC, 255);
  pw_vector(imgpw, box_xposition, box_yposition + box_ylength,
	    box_xposition + box_xlength, box_yposition + box_ylength,
	    PIX_SRC, 255);
  pw_vector(imgpw, box_xposition, box_yposition,
	    box_xposition, box_yposition + box_ylength,
	    PIX_SRC, 255);
  pw_vector(imgpw, box_xposition + box_xlength, box_yposition,
	    box_xposition + box_xlength, box_yposition + box_ylength,
	    PIX_SRC, 255);
  
  if (bitwin_on) {
      draw_box_in_bitwin();
      bitwin_marks();
    }
}

/* //////////////////////////////////////////////////////////////////////// */

draw_locator()
{
int	xstart	= bitwin_centerx - 1 - BITPIXWD/2,
	xend	= bitwin_centerx + 1 + BITPIXWD/2,
	ystart	= bitwin_centery - 1 - BITPIXHT/2,
	yend	= bitwin_centery + 1 + BITPIXHT/2;

	pw_vector(imgpw,
		xstart, ystart,
		xend, ystart,
		PIX_SRC, 255);
	pw_vector(imgpw,
		xstart, yend,
		xend, yend,
		PIX_SRC, 255);
	pw_vector(imgpw,
		xstart, ystart,
		xstart, yend,
		PIX_SRC, 255);
	pw_vector(imgpw,
		xend, ystart,
		xend, yend,
		PIX_SRC, 255);
}

/* //////////////////////////////////////////////////////////////////////// */

erase_locator()
{
int	xstart	= bitwin_centerx - 1 - BITPIXWD/2,
	xend	= bitwin_centerx + 1 + BITPIXWD/2,
	ystart	= bitwin_centery - 1 - BITPIXHT/2,
	yend	= bitwin_centery + 1 + BITPIXHT/2;

	pw_write(imgpw,
		xstart, ystart,
		BITPIXWD + 3, 1, PIX_SRC, image_in_window,
		xstart, ystart);
	pw_write(imgpw,
		xstart, yend,
		BITPIXWD + 3, 1, PIX_SRC, image_in_window,
		xstart, yend);
	pw_write(imgpw,
		xstart, ystart,
		1, BITPIXHT + 3, PIX_SRC, image_in_window,
		xstart, ystart);
	pw_write(imgpw,
		xend, ystart,
		1, BITPIXHT + 3, PIX_SRC, image_in_window,
		xend, ystart);
}

/* //////////////////////////////////////////////////////////////////////// */

draw_box_in_bitwin()
{
register	i;
register	bottomy	= bitwin_centery - 1 - BITPIXHT/2,
		topy	= bitwin_centery + 1 + BITPIXHT/2,
		bottomx	= bitwin_centerx - 1 - BITPIXWD/2,
		topx	= bitwin_centerx + 1 + BITPIXWD/2;

	pw_batch_on(bitpw);
	if ((box_yposition >= bottomy)
	&&  (box_yposition <= topy))
		for (i = box_xposition; i < box_xposition + box_xlength + 1;
									i++) {
			if (i > topx)
				break;
			if (i < bottomx)
				continue;
			paintfatbit(rbcol + (i - rcol) * Bitsize,
				    rbrow + (box_yposition - rrow) * Bitsize,
				    255);
		}

	if ((box_yposition + box_ylength >= bottomy)
	&&  (box_yposition + box_ylength <= topy))
		for (i = box_xposition; i < box_xposition + box_xlength + 1;
									i++) {
			if (i > topx)
				break;
			if (i < bottomx)
				continue;
			paintfatbit(rbcol + (i - rcol) * Bitsize,
				    rbrow
					+ (box_yposition + box_ylength - rrow)
								* Bitsize,
				    255);
		}

	if ((box_xposition >= bottomx)
	&&  (box_xposition <= topx))
		for (i = box_yposition; i < box_yposition + box_ylength + 1;
									i++) {
			if (i > topy)
				break;
			if (i < bottomy)
				continue;
			paintfatbit(rbcol + (box_xposition - rcol) * Bitsize,
				    rbrow + (i - rrow) * Bitsize,
				    255);
		}

	if ((box_xposition + box_xlength >= bottomx)
	&&  (box_xposition + box_xlength <= topx))
		for (i = box_yposition; i < box_yposition + box_ylength + 1;
									i++) {
			if (i > topy)
				break;
			if (i < bottomy)
				continue;
			paintfatbit(rbcol
					+ (box_xposition + box_xlength - rcol)
								* Bitsize,
				    rbrow + (i - rrow) * Bitsize,
				    255);
		}
	pw_batch_off(bitpw);
}

/* //////////////////////////////////////////////////////////////////////// */

erase_box()
{
register	i;
register	bottomy	= bitwin_centery - 1 - BITPIXHT/2,
		topy	= bitwin_centery + 1 + BITPIXHT/2,
		bottomx	= bitwin_centerx - 1 - BITPIXWD/2,
		topx	= bitwin_centerx + 1 + BITPIXWD/2;

	pw_write(imgpw, box_xposition, box_yposition,
		box_xlength + 1, 1,
		PIX_SRC, image_in_window, box_xposition, box_yposition);
	pw_write(imgpw, box_xposition, box_yposition + box_ylength,
		box_xlength + 1, 1,
		PIX_SRC, image_in_window, box_xposition,
		box_yposition + box_ylength);
	pw_write(imgpw, box_xposition, box_yposition,
		1, box_ylength + 1,
		PIX_SRC, image_in_window, box_xposition, box_yposition);
	pw_write(imgpw, box_xposition + box_xlength, box_yposition,
		1, box_ylength + 1,
		PIX_SRC, image_in_window, box_xposition + box_xlength,
		box_yposition);

	pw_batch_on(bitpw);
	if ((box_yposition >= bottomy)
	&&  (box_yposition <= topy))
		for (i = box_xposition; i < box_xposition + box_xlength + 1;
									i++) {
			if  (i > topx)
				break;
			if (i < bottomx)
				continue;
			paintfatbit(rbcol + (i - rcol) * Bitsize,
				    rbrow + (box_yposition - rrow) * Bitsize,
				    pw_get(imgpw, i, box_yposition));
		}

	if ((box_yposition + box_ylength >= bottomy)
	&&  (box_yposition + box_ylength <= topy))
		for (i = box_xposition; i < box_xposition + box_xlength + 1;
									i++) {
			if  (i > topx)
				break;
			if (i < bottomx)
				continue;
			paintfatbit(rbcol + (i - rcol) * Bitsize,
				    rbrow
					+ (box_yposition + box_ylength - rrow)
								* Bitsize,
				    pw_get(imgpw, i, box_yposition
								+ box_ylength));
		}

	if ((box_xposition >= bottomx)
	&&  (box_xposition <= topx))
		for (i = box_yposition; i < box_yposition + box_ylength + 1;
									i++) {
			if  (i > topy)
				break;
			if (i < bottomy)
				continue;
			paintfatbit(rbcol + (box_xposition - rcol) * Bitsize,
				    rbrow + (i - rrow) * Bitsize,
				    pw_get(imgpw, box_xposition, i));
		}

	if ((box_xposition + box_xlength >= bottomx)
	&&  (box_xposition + box_xlength <= topx))
		for (i = box_yposition; i < box_yposition + box_ylength + 1;
									i++) {
			if  (i > topy)
				break;
			if (i < bottomy)
				continue;
			paintfatbit(rbcol + (box_xposition + box_xlength - rcol)
								* Bitsize,
				    rbrow + (i - rrow) * Bitsize,
				    pw_get(imgpw, box_xposition + box_xlength,
									i));
		}
	pw_batch_off(bitpw);
}

/* //////////////////////////////////////////////////////////////////////// */

move_box(x, y)
int	x, y;
{
int	left, right, new_box_xposition, new_box_yposition, third;

	new_box_xposition = box_xposition;
	new_box_yposition = box_yposition;

	third	= box_xlength/3;
	if (x < box_xposition + third
	||  x > box_xposition + box_xlength - third) {
		left	= abs(box_xposition - x);
		right	= abs(box_xposition + box_xlength - x);
		if (left < right)
			new_box_xposition = x - 1;
		else
			new_box_xposition = x - box_xlength;
	}

	third	= box_ylength/3;
	if (y < box_yposition + third
	||  y > box_yposition + box_ylength - third) {
		left	= abs(box_yposition - y);
		right	= abs(box_yposition + box_ylength - y);
		if (left < right)
			new_box_yposition = y - 1;
		else
			new_box_yposition = y - box_ylength;
	}

	if (new_box_xposition != box_xposition
	||  new_box_yposition != box_yposition) {
		erase_box();
		box_xposition = new_box_xposition;
		box_yposition = new_box_yposition;
		draw_box();
	}
}

/* //////////////////////////////////////////////////////////////////////// */

adjust_box()
{
	erase_box();
	box_xposition = big(box_xposition + (Magfactor + 1)/2) - 1;
	box_yposition = big(box_yposition + (Magfactor + 1)/2) - 1;
	draw_box();
}

/* //////////////////////////////////////////////////////////////////////// */

stretch_box(x, y)
int	x, y;
{
int	left, right, third;

	erase_box();

	third	= box_xlength/3;
	if (x < box_xposition + third
	||  x > box_xposition + box_xlength - third) {
		left	= x - box_xposition;
		right	= box_xposition + box_xlength - x;
		if (abs(left) < abs(right)) {
			box_xposition = x;
			box_xlength -= left;
		}
		else
			box_xlength -= right - 1;
	}

	third	= box_ylength/3;
	if (y < box_yposition + third
	||  y > box_yposition + box_ylength - third) {
		left	= y - box_yposition;
		right	= box_yposition + box_ylength - y;
		if (abs(left) < abs(right)) {
			box_yposition = y;
			box_ylength -= left;
		}
		else
			box_ylength -= right - 1;
	}

	if (box_xlength < 1)
		box_xlength = 1;
	if (box_ylength < 1)
		box_ylength = 1;

	draw_box();
}

/* //////////////////////////////////////////////////////////////////////// */

setboxsize(x, y)
int	x, y;
{
	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;
	erase_box();
	box_yposition += box_ylength - y - 1;
	box_xlength = x + 1;
	box_ylength = y + 1;
	draw_box();
}

/* //////////////////////////////////////////////////////////////////////// */

filter(commandline)
char			commandline[1000];
{
Pixrect			*temp_image;
char			*inbuf, *outbuf, *buffer;
struct rasterfile	temprh;
struct header		temphd;
int			imagebytes, maxgrlebytes, bytesback;

	undo_init();

	imagebytes	= ((box_xlength - 1)/Magfactor)
	                      * ((box_ylength - 1)/Magfactor);
	maxgrlebytes	= 3.5 * imagebytes;

        if ((inbuf = (char *)calloc(imagebytes, sizeof(u_char))) == NULL) {
                perr(HE_ALLOC, "inbuf");
                exit(1);
        }
        if ((outbuf = (char *)calloc(maxgrlebytes, sizeof(u_char))) == NULL) {
		perr(HE_ALLOC, "outbuf");
                exit(1);
        }
     
	temprh.ras_magic	= rh.ras_magic;
	temprh.ras_width	= box_xlength - 1;
	temprh.ras_height	= box_ylength - 1;
	temprh.ras_depth	= rh.ras_depth; 
	temprh.ras_length	= temprh.ras_height
			* mpr_linebytes(temprh.ras_width, temprh.ras_depth);
	temprh.ras_type		= rh.ras_type;
	temprh.ras_maptype	= rh.ras_maptype;
	temprh.ras_maplength	= rh.ras_maplength;

	temp_image = mem_create(temprh.ras_width, temprh.ras_height, 
							temprh.ras_depth);
	pr_rop(temp_image, 0, 0,
		box_xlength - 1, box_ylength - 1, PIX_SRC,
		image_in_window, box_xposition + 1, box_yposition + 1);

	init_header(&temphd, "", "", 1, "",
		(box_ylength - 1)/Magfactor, (box_xlength - 1)/Magfactor,
		PFBYTE, 1, "");
        to_hips(&temprh, (colormap_t *)0, mpr_d(temp_image)->md_image, &temphd,
							inbuf, Magfactor);

	bytesback = execute_command(&temphd, imagebytes, inbuf,
					maxgrlebytes, outbuf, commandline);
	buffer = outbuf;
	if (temphd.pixel_format != PFBYTE
#ifdef	GRLE
	&&  temphd.pixel_format != PFGRLE
#endif
					) {
		fprintf(stderr,
"%s: command '%s' produces incompatible format; try '%s | byte'\n",
					Progname, commandline, commandline);
		bytesback = 0;
	}

#ifdef	GRLE
	if (temphd.pixel_format == PFGRLE) {
		while (execute_command(&temphd, bytesback, outbuf,
				temprh.ras_width * temprh.ras_height, inbuf,
				"/usr/local/hipl/byte") == 0)
			;
		buffer = inbuf;
        }
#endif

	if (bytesback != 0) {
		if (to_sun(&temphd, buffer, &temprh, (colormap_t *)0,
				mpr_d(temp_image)->md_image, Magfactor) < 0)
			error("problem converting hips image");

		pw_write(imgpw, box_xposition + 1, box_yposition + 1,
				temprh.ras_width, temprh.ras_height,
                                PIX_SRC, temp_image, 0, 0);
		pr_rop(image_in_window, box_xposition + 1, box_yposition + 1, 
				box_xlength - 1, box_ylength - 1, PIX_SRC,
				temp_image, 0, 0);
	}

	pr_destroy(temp_image);
        free(inbuf);
        free(outbuf);
}

static h_boolean		undo_start;
static h_boolean		undone;

/* //////////////////////////////////////////////////////////////////////// */

undo_init()
{
	save_x	= box_xposition + 1;
	save_y	= box_yposition + 1;
	save_dx	= box_xlength - 1;
	save_dy	= box_ylength - 1;

	if (box_is_on)
		erase_box();

	pw_read(save1, save_x, save_y,
		save_dx, save_dy, PIX_SRC,
		imgpw, save_x, save_y);

	if (box_is_on && !blinking)
		draw_box();

	undo_start = TRUE, undone = FALSE;
}

/* //////////////////////////////////////////////////////////////////////// */

undo()
{
static Pixrect	*one, *two;
static h_boolean	firsttime = TRUE;

	if (firsttime)
		one = save1, two = save2, firsttime = FALSE;
	if (undo_start)
		one = save1, two = save2, undo_start = FALSE;

	if (box_is_on)
		erase_box();

	pw_read(two, save_x, save_y,
		save_dx, save_dy, PIX_SRC,
		imgpw, save_x, save_y);
	pw_write(imgpw, save_x, save_y,
		save_dx, save_dy, PIX_SRC,
		one, save_x, save_y);
	pr_rop(image_in_window, save_x, save_y,
		save_dx, save_dy, PIX_SRC,
		one, save_x, save_y);

	if ((bitwin_on)
	&&  (save_y + save_dy >= bitwin_centery - 1 - BITPIXHT/2)
	&&  (save_y <= bitwin_centery + 1 + BITPIXHT/2)
	&&  (save_x + save_dx >= bitwin_centerx - 1 - BITPIXWD/2)
	&&  (save_x <= bitwin_centerx + 1 + BITPIXWD/2))
		paint_bitwin();

	if (one == save1)
		one = save2, two = save1;
	else
		one = save1, two = save2;

	if (box_is_on && !blinking)
		draw_box();

	undone = !undone;
}

/* //////////////////////////////////////////////////////////////////////// */

shift(x, y)		/* shift the read-in image box */
int	x, y;
{
	if (x == box_xposition
	&&  y == box_yposition)
		return;

	if (!undone)
		undo();

	move_box(x, y);
}

/* //////////////////////////////////////////////////////////////////////// */

fillin_box()	/* fill in the shifted readin-box with the readin-image */
{
	undo_init();

	pw_write(imgpw, box_xposition + 1, box_yposition + 1,
		save_dx, save_dy, readin_op,
		readin_image, 0, 0);
	pr_rop(image_in_window, box_xposition + 1, box_yposition + 1,
		save_dx, save_dy, readin_op,
		readin_image, 0, 0);

	if ((bitwin_on)
	&&  (save_y + save_dy >= bitwin_centery - 1 - BITPIXHT/2)
	&&  (save_y <= bitwin_centery + 1 + BITPIXHT/2)
	&&  (save_x + save_dx >= bitwin_centerx - 1 - BITPIXWD/2)
	&&  (save_x <= bitwin_centerx + 1 + BITPIXWD/2))
		paint_bitwin();

}

/* //////////////////////////////////////////////////////////////////////// */

writeout_box(fp)
  FILE *fp;
  /*int	fd; */
{
int			imagebytes;
char			*buffer;
struct rasterfile	temprh;
Pixrect			*temp_image;
char			*savehist, *history = hd.seq_history;
char			string[40];
int			rows, cols, pformat;
int			numframes = hd.num_frame;

	imagebytes = box_xlength * box_ylength / (Magfactor * Magfactor);
	if (hd.pixel_format == PFSRLE)
		imagebytes = (imagebytes + 7)/8;
	buffer = (char *) calloc(imagebytes, sizeof(char));
	if (buffer == NULL) {
		perr(HE_ALLOC, "writeout_box buffer");
		exit(1);
	}

	temprh.ras_magic	= rh.ras_magic;
	temprh.ras_width	= box_xlength - 1;
	temprh.ras_height	= box_ylength - 1;
	temprh.ras_depth	= rh.ras_depth; 
	temprh.ras_length	= temprh.ras_height
			* mpr_linebytes(temprh.ras_width, temprh.ras_depth);
	temprh.ras_type	= rh.ras_type;
	temprh.ras_maptype	= rh.ras_maptype;
	temprh.ras_maplength	= rh.ras_maplength;

	temp_image = mem_create(to32(temprh.ras_width), temprh.ras_height, 
							temprh.ras_depth);
	pr_rop(temp_image, 0, 0,
		box_xlength - 1, box_ylength - 1, PIX_SRC,
		image_in_window, box_xposition + 1, box_yposition + 1);

	savehist = (char *) malloc((strlen(history) + 1) * sizeof(char));
	if (savehist == NULL) {
		perr(HE_ALLOC, "writeout_box savehist");
		exit(1);
	}

	pformat = hd.pixel_format;
	rows = hd.orows, cols = hd.ocols;
	strcpy(savehist, history);

	to_hips(&temprh, (colormap_t *)0, mpr_d(temp_image)->md_image, &hd,
							buffer, Magfactor);
	sprintf(string, "(exponent %.2lf; extract %d %d %d %d)", exponent,
			(box_ylength - 1)/Magfactor,
						(box_xlength - 1)/Magfactor,
			hipsy(box_yposition + box_ylength) + 1,
			hipsx(box_xposition) + 1);

	av[ac - 1] = string;

	update_header(&hd, ac, av);

	hd.pixel_format		= PFBYTE;

#ifdef	GRLE

	if (Outformat == FORCEGRLE)
		hd.pixel_format = PFGRLE;

#endif

	hd.num_frame		= 1;
	if (FBdepth == 1)
#ifdef MSBFVERSION
		hd.pixel_format = PFMSBF; 
#else
		hd.pixel_format = PFLSBF; 
#endif MSBFVERSION


	if (colorentries == 0)
		output_lutfunc(buffer);

	fwrite_header(fp, &hd, "outputfile");
	if (store(fp, (box_ylength - 1)/Magfactor, 
		  (box_xlength - 1)/Magfactor,
		  hd.pixel_format, hd.sizepix,
		  buffer, PFBYTE, FBdepth) < 0) {
		fprintf(stderr, "%s: store failure\n", av[0]);
		hd.seq_history = savehist;
		hd.rows = hd.orows = rows, hd.cols = hd.ocols = cols;
		hd.pixel_format = pformat;
		return(-1);
	}

	hd.seq_history = savehist;
	hd.orows = hd.rows = rows, hd.ocols = hd.cols = cols;
	hd.pixel_format = pformat;
	hd.num_frame = numframes;
	return(0);
}

/* //////////////////////////////////////////////////////////////////////// */

scale(real, byte)
float	*real;
u_char	*byte;
{
register float	*realp;
register u_char	*bytep;
register int	i;
float		maximum, minimum, b, c;

	i = hd.orows * hd.ocols - 1;
	maximum = minimum = *real;
	realp = real;
	while (i--) {
		if (*++realp > maximum)
			maximum = *realp;
		if (*realp < minimum)
			minimum = *realp;
	}
	if (maximum == minimum) {
		bytep = byte;
		i = hd.orows * hd.ocols;
		while (i--)
			*bytep++ = 128;
		return;
	}
	b = 255/(maximum - minimum);
	c = -minimum * b;
	bytep = byte, realp = real;
	i = hd.orows * hd.ocols;
	while (i--)
	  *bytep++ = *realp++ * b + c;
}

/* //////////////////////////////////////////////////////////////////////// */

error(message)
  char	*message;
{
  fprintf(stderr, "%s: %s\n", av[0], message);
  exit(1);
}

