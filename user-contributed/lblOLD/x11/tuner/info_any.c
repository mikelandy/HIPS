/*	info_any . c
#
%	Any list changed here, must be change tuner.h as well!
*/
char	*dpy_root_mode = "Display in Root", *clr_root_mode = "Clear Root",
	*ctrlist[numctrl] = {"INFO", "annotate", "append", "copy", "crop",
		"cut", "delete", "draw", "mean sub-area", "paint", "paste",
		"snap window", "NOP"},
	*paralist[numpara] = {"FITS Type", "Change ETA Scale",
		"Interpolating LEVEL", "R.G.B. Weight", "Background",
		"Window Display Mode", "NOP"},
	*filelist[numcomd] = {"Empty Frame", "Change DIR", "Loading Frame",
			"map 1 to 3", "Output TYPE", "Quit", "NOP"},
	*fontlist[numfont] = {"white", "blcak", "pick color in image",
			"6x10", "8x16", "9x15", "10x20", "12x24",
			"rot-s16", "vr-40", "vrl-40", "NOP"},
	*Help_message_array1="\
button:      LEFT            MIDDLE           RIGHT\n\
alone :  value+coord      toggle-zoom         crop\n\
+SHIFT:    Magnify        zoom-shift          Shrink\n\
+CTRL :   EDIT-MENU       SCALE-MENU         FILE_MENU\n\
CTRL + Any Button => show Control-Panel; Press 'c' key to close it.\n\
\nfor multiple frame image from standard input, use '+' key \
to go forward, and '-' key to go backward\n\
press 'q' key in an image window to quit that image\n\
press 'q' key on panel to quit system\n\
more : see man tuner",

*display_name, *fname="std_in", *PasteMesg =
{"Move Cursor to an Image and Click Button, Move cursor to a Point to Paste and Release Button"},

*PaintMesg = {"Click on an image to raise it\nPress LEFT button to edit\n\
MIDDLE button to change feature\nand RIGHT button to quit\n\
CTRL-Y to end typing"};

#ifndef	C_TUNER
arg_fmt_list_string	arg_list[] =	{
	{"-#", "%d", NCOLOR, 1, 1, "gray levels. Default is %.f"},
	{"-cc", "%+", 0, 1, 0,
		"change colormap for new cmap with lazy window manage"},
	{"-cq", "%d", NCOLOR, 1, 0,
		"query color map table. none or pos - sys, neg - user"},
	{"-d[ebug]", "%d", 1, 1, 0, "debug level : max level = 3"},
	{"-di[splay]", "%s", 0, 1, 1,
		"hostname :	specify a point to display"},
	{"-e", "%* %d", 6, 2, 1, "elastic range [200 - 120000] default=1200"},
	{"-i", "%d", 12, 1, 1, "maximum images can display at same time. [%.f]"},
	{"-n", "%-", 0, 1, 0, "new colormap -- use separated colormap table"},
	{"-p", "%i", 384, 1, 0, "color precision. [%.f]"},
	{"-s", "%i", 1, 1, 0,
		"start speed level {0 - 3}.	[default = %.f]\n\
		It's better for both panel color and starting speed"},
	{"-v", "%i", 1, 1, 0, "verbose message : maximum level = 3"},
	{"-R[x][y]", "%E %i", 2, 2, 0, "interpolation region [2-8] default 2x2"},
	{" I/O:	[<] input", No, 0, 0, 0, "end of usage"},
		NULL	};
#endif
