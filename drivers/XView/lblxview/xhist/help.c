
#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>

#include <X11/X.h>
#include <X11/Xlib.h>

#include <xview/xview.h>
#include <xview/frame.h>
#include <xview/panel.h>
#include <xview/canvas.h>
#include <xview/font.h>
#include <xview/xv_xrect.h>

#define color_White  WhitePixel(dpy0, DefaultScreen(dpy0))
#define color_Black  BlackPixel(dpy0, DefaultScreen(dpy0))

extern u_long color_Red;
extern u_long color_Blue;
extern u_long color_Green;
extern u_long color_Orange;

#define TXTHT2 19

Frame     help_frame;

Xv_Font   def_font;

help_proc()
{
    Canvas    canvas;
    Panel     panel;
    XGCValues gcvalues;
    void      help_repaint_proc();
    void      help_quit();

/*
 xv_init(XV_INIT_ARGC_PTR_ARGV, &argc, argv, NULL);
*/

    help_frame = (Frame) xv_create(NULL, FRAME,
				   FRAME_LABEL, " xhist help manual ",
				   XV_WIDTH, 570,
				   XV_HEIGHT, 720,
				   NULL);

    canvas = (Canvas) xv_create(help_frame, CANVAS,
				XV_X, 0,
				XV_Y, 0,
				XV_WIDTH, WIN_EXTEND_TO_EDGE,
				XV_HEIGHT, 680,
				CANVAS_X_PAINT_WINDOW, TRUE,
				CANVAS_REPAINT_PROC, help_repaint_proc,
				NULL);

/* window_fit(help_frame); */

    panel = (Panel) xv_create(help_frame, PANEL,
			      XV_X, 0,
			      XV_Y, 680,
			      XV_WIDTH, WIN_EXTEND_TO_EDGE,
			      XV_HEIGHT, WIN_EXTEND_TO_EDGE,
			      NULL);

    (void) xv_create(panel, PANEL_BUTTON,
		     XV_X, 200,
		     XV_Y, 12,
		     PANEL_LABEL_STRING, "Quit  help  manual",
		     PANEL_NOTIFY_PROC, help_quit,
		     NULL);

    def_font = (Xv_Font) xv_find(help_frame, FONT,
				 FONT_STYLE, FONT_STYLE_BOLD, NULL);

    xv_main_loop(help_frame);
}

void
help_repaint_proc(canvas, pw, dpy0, xwin, xrects)
    Canvas    canvas;
    Xv_window pw;
    Display  *dpy0;
    Window    xwin;
    Xv_xrectlist *xrects;
{
    int       i;
    char     *t[50];
    GC        gc2;
    Font      fid;
    int       ypos;

    for (i = 0; i < 50; i++)
	t[i] = NULL;
    t[0] = "xhist - display the histogram of an image";
    t[1] = " ";

    t[2] = "SYNOPSIS -----------------------------------------------------------";

    t[3] = "    % xhist";
    t[4] = "    % xhist image_file";
    t[5] = "    % xhist < image_file";
    t[6] = "    % ...... | xhist               ( pipe )";

    t[10] = "DESCRIPTION --------------------------------------------------------";
    t[11] = "  Type of image file may be byte, short, int, or float.";
    t[12] = "  All red numbers are pixel values.";
    t[13] = "  All blue numbers are the counts of pixel values";
    t[14] = "                                  (or counts of a range of pixel values).";
    t[15] = "  Each vertical line refers to a pixel value ( or a range of pixel values )";
    t[16] = "                                          and its counts.";

    t[17] = "  The red vertical line is used to check a certain pixel value.";
    t[18] = "  Orange vertical lines represent the range of interesting pixel values.";
    t[19] = "  Green horizontal lines represent the range of interesting counts.";

    t[25] = "CONTORL  BUTTONS --------------------------------------------------";
    t[26] = "  Check pixel value :  check the pixel value specified by \"pixel value : __\" field";
    t[28] = "  <--  :  move red vertical bar left by 1 unit";
    t[29] = "  -->  :  move red vertical bar right by 1 unit";
    t[30] = "  Select |  :   select vertical margin (orange vertical line) for next region";
    t[31] = "  Eval :  Evaluate a region within vertical ( and horizontal ) margins";
    t[32] = "  Restore :  restore histogram for this image";
    t[33] = "  Set count margin : set the count margin specified by \"count margin : __\" field";

    t[34] = "  Load :  load image file";
    t[35] = "  QUIT :  quit xhist";
    t[36] = "  Help :  show this help manual";

    t[40] = "MOUSE  EVENTS  on the graph ------------------------------------------";
    t[41] = "  LEFT BUTTON :  move red vertical bar to check pixel value and its counts";
    t[42] = "  MIDDLE BUTTON :  select vertical margin (orange vertical line) for next region";
    t[43] = "  RIGHT BUTTON :  select horizontal margin (green horizonal line) for next region";

    gc2 = DefaultGC(dpy0, DefaultScreen(dpy0));

    XSetForeground(dpy0, gc2, color_Black);
    XSetBackground(dpy0, gc2, color_White);
    fid = (Font) xv_get(def_font, XV_XID);
    XSetFont(dpy0, gc2, fid);

    XDrawString(dpy0, xwin, gc2, 20, 24, t[0], strlen(t[0]));
    ypos = 52;
    for (i = 2; i < 50; i++) {
	switch (i) {
	case 37:
	case 7:
	    ypos = ypos + 14;
	    break;		/* blank line	  */
	case 12:
	    XSetForeground(dpy0, gc2, color_Red);
	    break;
	case 13:
	    XSetForeground(dpy0, gc2, color_Blue);
	    break;
	case 15:
	    XSetForeground(dpy0, gc2, color_Black);
	    break;
	case 17:
	    XSetForeground(dpy0, gc2, color_Red);
	    break;
	case 18:
	    XSetForeground(dpy0, gc2, color_Orange);
	    break;
	case 19:
	    XSetForeground(dpy0, gc2, color_Green);
	    break;
	case 20:
	    XSetForeground(dpy0, gc2, color_Black);
	    ypos = ypos + 14;
	    break;

	case 28:
	    XSetForeground(dpy0, gc2, color_Red);
	    break;
	case 30:
	    XSetForeground(dpy0, gc2, color_Orange);
	    break;
	case 31:
	    XSetForeground(dpy0, gc2, color_Black);
	    break;
	case 33:
	    XSetForeground(dpy0, gc2, color_Green);
	    break;
	case 34:
	    XSetForeground(dpy0, gc2, color_Black);
	    ypos = ypos + 14;
	    break;

	case 41:
	    XSetForeground(dpy0, gc2, color_Red);
	    break;
	case 42:
	    XSetForeground(dpy0, gc2, color_Orange);
	    break;
	case 43:
	    XSetForeground(dpy0, gc2, color_Green);
	    break;
	}
	if (t[i] == NULL)
	    continue;
	XDrawString(dpy0, xwin, gc2, 10, ypos, t[i], strlen(t[i]));
	ypos = ypos + TXTHT2;
    }

}

void
help_quit()
{
    xv_destroy_safe(help_frame);
}
