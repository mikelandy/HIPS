/* X_funcs.c
 * Max Rible
 *
 * Support functions for X garbage.
 */

#include "hipstool.h"

#ifdef X_WINDOWS
Display *display;
int display_screen;

Window control;
XSizeHints control_hints;

void
window_environment_init(argc, argv)
     int argc;
     char *argv[];
{
    if((display = XOpenDisplay(display_name)) == NULL) {
	fprintf(stderr, "Can't open display %s!\n",
		XDisplayName(display_name));
	exit(-1);
    }

    display_screen = DefaultScreen(display);

    control =
	XCreateSimpleWindow(display, RootWindow(display, display_screen),
			    winx, winy, BORDER_WIDTH,
			    BlackPixel(display, display_screen),
			    WhitePixel(display, display_screen));

    control_hints.flags = PSize | PMinSize | PMaxSize;
    control_hints.width = 
	control_hints.min_width = 
	    control_hints.max_width = 500; /* Arbitrary */
    control_hints.height =
	control_hints.min_height =
	    control_hints.max_height = 500; /* Arbitrary */

    XSetStandardProperties(display, control, "HIPStool Control",
			   "HIPStool", NULL, NULL, 0, &control_hints);
    XSelectInput(display, control, ExposureMask | 
		 ButtonPressMask | ButtonReleaseMask |
		 KeyPressMask);

}
#endif				/* X_WINDOWS */
