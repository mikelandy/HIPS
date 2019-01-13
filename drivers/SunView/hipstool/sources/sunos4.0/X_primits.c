/* X_primits.c
 * Max Rible
 *
 * Graphics primitives that work under X.
 */

#include "hipstool.h"

#ifdef X_WINDOWS
void
newindow(winfo, width, height, posx, posy, label)
     Windowinfo *winfo;
     int width, height, posx, posy;
     char *label;
{
    winfo->window =
	XCreateSimpleWindow(display, control, posx, posy,
			    width, height, BORDER_WIDTH, 
			    BlackPixel(display, display_screen),
			    WhitePixel(display, display_screen));

    winfo->size_hints.flags = PSize | PMinSize | PMaxSize;
    winfo->size_hints.width = 
	winfo->size_hints.min_width = 
	    winfo->size_hints.max_width =
		width;
    winfo->size_hints.height = 
	winfo->size_hints.min_height = 
	    winfo->size_hints.max_height = height;

    /* Find an appropriate icon pixmap instead of that NULL */
    XSetStandardProperties(display, winfo->window, label,
			   "HIPStool", NULL, NULL, 0, &winfo->size_hints);
    XSelectInput(display, winfo->window, ExposureMask);

    winfo->gc = XCreateGC(display, winfo->window, 0, winfo->gcvalues);
    XSetForeground(display, gc, BlackPixel(display, display_screen));
    XSetLineAttributes(display, gc, 1, LineSolid, CapButt, JoinBevel);

    XMapWindow(display, winfo->window);
}
#endif				/* X_WINDOWS */
