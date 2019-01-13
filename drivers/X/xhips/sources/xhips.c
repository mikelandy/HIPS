/***********************************************************************
*  File:   xhips.c
*  Author: Patrick J. Flynn, from `xim.c' by Philip Thompson
*  $Date: $
*  $Revision: $
*  Purpose: To view a HIPS-formatted image.
*
*  I (PJF) consider this a derivative of `xim', and include the original
*  copyright notice.
*
*  Copyright (c) 1988  Philip R. Thompson
*                Computer Resource Laboratory (CRL)
*                Dept. of Architecture and Planning
*                M.I.T., Rm 9-526
*                Cambridge, MA  02139
*   This  software and its documentation may be used, copied, modified,
*   and distributed for any purpose without fee, provided:
*       --  The above copyright notice appears in all copies.
*       --  This disclaimer appears in all source code copies.
*       --  The names of M.I.T. and the CRL are not used in advertising
*           or publicity pertaining to distribution of the software
*           without prior specific written permission from me or CRL.
*   I provide this software freely as a public service.  It is NOT a
*   commercial product, and therefore is not subject to an an implied
*   warranty of merchantability or fitness for a particular purpose.  I
*   provide it as is, without warranty. This software was not sponsored,
*   developed or connected with any grants, funds, salaries, etc.
*
*   This software is furnished  only on the basis that any party who
*   receives it indemnifies and holds harmless the parties who furnish
*   it against any claims, demands, or liabilities connected with using
*   it, furnishing it to others, or providing it to a third party.
*
*   Philip R. Thompson (phils@athena.mit.edu)
***********************************************************************/

/* converted to HIPS2:  Felix Huang, LBL
 */

#ifndef LINT
static char xim_rcs_id[] =
    "$ $";
#endif

#include <hipl_format.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <stdio.h>
#include <stdlib.h>

h_boolean debug_flag;
Display *dpy;
int screen;
Window root_win;
Visual *visual = NULL;
u_long blackpixel,whitepixel;

static Flag_Format flagfmt[] = {
	{"d",{LASTFLAG},1,{{PTBOOLEAN,"FALSE"},{PTSTRING,"","display"},
		LASTPARAMETER}},
	{"c",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"b",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"g",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"L",{LASTFLAG},1,{{PTBOOLEAN,"FALSE"},{PTSTRING,"","framelabel"},
		LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,LASTTYPE};
void error();

int main(argc,argv)

int argc;
char **argv;

{
    register unsigned i,j,k;
    register byte *buffer,*red_buf,*grn_buf,*blu_buf,*icon_buf;
    unsigned buf_size;
    int icon_width,icon_height,iconfact,method;
    int buf_width,buf_height,ncolors=64;
    char *win_name=NULL,*display_name=NULL,*framelabel;
    char *str_index;
	void *calloc(),*malloc();
    h_boolean newmap_flag,dflag,gflag,lflag;
    struct header hd,hdp;
    XColor colors[256],fore_color,back_color;
    Window image_win,icon_win;
    Colormap colormap,GetColormap();
    XEvent event;
    XExposeEvent *expose;
    XCrossingEvent *xcrossing;
    GC image_gc,icon_gc;
    XGCValues gc_val;
    XSetWindowAttributes xswa;
    XImage *image=NULL,*icon_image=NULL;
    XSizeHints sizehints;
    XWMHints wmhints;
    FILE *fp;
    Filename filename;

    Progname = strsave(*argv);
    parseargs(argc,argv,flagfmt,&dflag,&display_name,&newmap_flag,&debug_flag,
	&gflag,&lflag,&framelabel,FFONE,&filename);
    fp = hfopenr(filename);
    if (!dflag)
	display_name = NULL;
    if (gflag)
          ncolors=256;

    /*  Open the display & set defaults */
    if ((dpy = XOpenDisplay(display_name)) == NULL)
        error("Can't open display '%s'", XDisplayName(display_name));
    screen = XDefaultScreen(dpy);
    root_win = XDefaultRootWindow(dpy);
    visual = XDefaultVisual(dpy, screen);
    blackpixel = XBlackPixel(dpy, screen);
    whitepixel = XWhitePixel(dpy, screen);
    if (XDisplayPlanes(dpy, screen) == 1)
      error("Can't display grayscale on monochrome screen","\0");

    /* Read header and verify image file formats */

    fread_hdr_a(fp,&hd);
    method = fset_conversion(&hd,&hdp,types,filename);
    buf_width=hd.ocols;
    buf_height=hd.orows;
    fprintf(stderr,"size %d x %d\n",buf_height,buf_width);
    buf_size = buf_width * buf_height;
    /* Get or make the color table.
    */
    for (i=0; i < ncolors; i++) {
            colors[i].pixel = (u_long)i;
            j=256*((ncolors==64)?i*4:i);
            colors[i].red=colors[i].green=colors[i].blue=(u_short)j;
            colors[i].flags = DoRed | DoGreen | DoBlue;
        }
    /* malloc() and read the data buffer(s)
    */
    buffer = hdp.image;
    fread_imagec(fp,&hd,&hdp,method,0,filename);
    /* if 64-color map, reduce pixel values as well */
    if (ncolors==64)
      for(i=0;i<buf_width*buf_height;i++)
        buffer[i] /= 4;

    /*  Allocate the icon with max. dimension of 50 */
    iconfact = (buf_height/50) > (buf_width/50) ? (buf_height/50)
        :(buf_width/50);
    if (iconfact == 0)
        iconfact = 1;
    if ((icon_width = buf_width / iconfact +1) % 2)
        icon_width -= 1;
    icon_height = buf_height / iconfact;
    if (debug_flag)
        fprintf(stderr,"icon width %d  height %d  factor %d\n",
            icon_width, icon_height, iconfact);
    icon_buf = (byte *)malloc((unsigned)icon_width*icon_height);
    if (icon_buf == NULL)
        error("Can't malloc() icon buffer", "\0");

    /* process and store the image and icon.
    */
    if (ncolors > 250)          /* Don't bother trying to fit */
      newmap_flag = True;     /* into default map, faster too */
    colormap = GetColormap(colors, ncolors, &newmap_flag, buffer,
                           buf_size);
    icon_image = XCreateImage(dpy, visual, 8, ZPixmap, 0,
                              (char *)icon_buf, icon_width, icon_height, 8, 0);
    for (i=0; i < icon_height; i++)
      for (j=0; j < icon_width; j++)
          XPutPixel(icon_image, j, i, buffer[(i*buf_width+j)*iconfact]);
    image = XCreateImage(dpy, visual, 8, ZPixmap, 0, (char *)buffer,
                         buf_width, buf_height, 8, 0);
    if (debug_flag)
        fprintf(stderr,"processed.\n");

    /* Get window attributes */
    xswa.event_mask = ExposureMask |ButtonPressMask |ColormapChangeMask|
        LeaveWindowMask | EnterWindowMask;
    xswa.background_pixel = blackpixel;
    xswa.border_pixel = whitepixel;
    xswa.colormap = colormap;
    xswa.cursor = XCreateFontCursor(dpy, XC_gumby);
    image_win = XCreateWindow(dpy, root_win, 0, 0,
        buf_width, buf_height, 5, XDefaultDepth(dpy,screen),
        InputOutput, visual, CWBackPixel |CWEventMask |CWCursor |
        CWBorderPixel |CWColormap, &xswa);
    xswa.event_mask = ExposureMask;
    icon_win = XCreateWindow(dpy, root_win, 0, 0,
        icon_width, icon_height, 1, XDefaultDepth(dpy,screen),
        InputOutput, visual, CWBackPixel | CWBorderPixel, &xswa);

    /* set window manager hints */
    sizehints.flags = PPosition | PSize | PMinSize | PMaxSize;
    sizehints.width = sizehints.min_width = buf_width;
    sizehints.max_width = buf_width;
    sizehints.height = sizehints.min_height = buf_height;
    sizehints.max_height = buf_height;
    sizehints.x = 0;
    sizehints.y = 0;
    j = strlen(filename);
    k = 0;
    for (i=0;i<j;i++)
	if (filename[i] == '/')
		k = i+1;
    XSetStandardProperties(dpy, image_win, lflag ? framelabel : (filename+k),
	    win_name, None, argv, argc, &sizehints);
    wmhints.flags = IconWindowHint | IconPositionHint;
    wmhints.icon_window = icon_win;
    wmhints.icon_x = XDisplayWidth(dpy,screen) - 200;
    wmhints.icon_y = 2;
    XSetWMHints(dpy, image_win, &wmhints);

    gc_val.function = GXcopy;
    gc_val.plane_mask = AllPlanes;
    gc_val.foreground = blackpixel;
    gc_val.background = whitepixel;
    image_gc = XCreateGC(dpy,image_win, GCFunction | GCPlaneMask |
        GCForeground | GCBackground, &gc_val);
    icon_gc = XCreateGC(dpy, icon_win, GCFunction | GCPlaneMask |
        GCForeground | GCBackground, &gc_val);

    XMapWindow(dpy, image_win);             /* Map the image window. */
    if (newmap_flag) {
        XInstallColormap(dpy, colormap);
        if (ncolors > 254) {
            fore_color.red = colors[255].red;       /* force the last */
            fore_color.green = colors[255].green;   /* two colors and */
            fore_color.blue = colors[255].blue;   /* sacrifice cursor */
            back_color.red = colors[254].red;
            back_color.green = colors[254].green;
            back_color.blue = colors[254].blue;
            XRecolorCursor(dpy, xswa.cursor, &fore_color, &back_color);
        }
    }

    /* Select events to listen for  */
    XSelectInput(dpy, image_win, (ButtonPressMask | ColormapChangeMask |
        ExposureMask | LeaveWindowMask | EnterWindowMask));
    XSelectInput(dpy, icon_win, ExposureMask);

    if (debug_flag)
       fprintf(stderr,"While loop.\n");
    expose = (XExposeEvent *)&event;
    xcrossing = (XCrossingEvent *)&event;
    while (True) {          /* Set up a loop to maintain the image. */
        XNextEvent(dpy, &event);           /* Wait on input event. */
        switch((int)event.type) {
        int modulo;     /* Temporary var. for expose->x % 4 */
        case Expose:
            if (expose->window == icon_win) {
                XPutImage(dpy, icon_win, icon_gc, icon_image, 0, 0,
                    0, 0, icon_width, icon_height);
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
            if (expose->width % 4 != 0)
                expose->width += 4 - (expose->width % 4);
            XPutImage(dpy, image_win, image_gc, image,
                expose->x, expose->y, expose->x, expose->y,
                expose->width, expose->height);
            if (debug_flag)
                fprintf(stderr, "Actual expose: %d  %d  %d  %d\n",
                expose->x, expose->y, expose->width, expose->height);
            break;
        case ButtonPress:
            switch((int)event.xbutton.button) {
            case Button1: break;
            case Button2: break;
            case Button3:
                if (newmap_flag)
                    XInstallColormap(dpy, XDefaultColormap(dpy,screen));
                XDestroyWindow(dpy, image_win);
                XDestroyWindow(dpy, icon_win);
                XCloseDisplay(dpy);
                exit(0);
            }
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
    }
}  /* end main */


Colormap GetColormap(colors, ncolors, newmap_flag, buf, bufsize)

XColor  colors[];
int  ncolors;
Bool  *newmap_flag;
register byte  *buf;
unsigned  bufsize;

{
    register int i;
    Colormap cmap, cmap2;
    XColor qcolor;
    u_long GetColorValue();

    if (ncolors > XDisplayCells(dpy,screen))    /* an X nonfeature */
        ncolors = XDisplayCells(dpy,screen);
    if (debug_flag)
        fprintf(stderr,"Colormap size %d\n", ncolors);

    if (*newmap_flag) {
        cmap = XCreateColormap(dpy, root_win, visual, AllocAll);
        XStoreColors(dpy, cmap, colors, ncolors);
    } else {
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
        for (i=0; i < bufsize; i++)
            buf[i] = (byte)colors[buf[i]].pixel;
    }
    if (*newmap_flag) {
        whitepixel = GetColorValue(cmap, ncolors, 255, 255, 255);
        blackpixel = GetColorValue(cmap, ncolors, 0, 0, 0);
    }
    if (debug_flag)
      for (i=0; i < ncolors; i++) {
         qcolor.pixel = (u_long)i;
         XQueryColor(dpy, cmap, &qcolor);
         fprintf(stderr,"color[%3d]: pix %3lu r= %5u g= %5u b= %5u\n",i,
            qcolor.pixel, qcolor.red, qcolor.green, qcolor.blue);
      }
    return(cmap);
}


/* Find the the closest color in the colormap.
*/
u_long GetColorValue(cmap, ncolors, red, green, blue)

Colormap cmap;
int ncolors, red, green, blue;

{
    register int i, red2, blue2, green2;
    XColor qcolor;
    u_long value;
    long dist, least = 1e5;

    for (i=0; i < ncolors; i++) {
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
    return(value);
}

void error(s1, s2)

char *s1, *s2;   /* Error description string. */

{
    fprintf(stderr,"%c%s: Error =>\n%c", 7, Progname, 7);
    fprintf(stderr, s1, s2);
    fprintf(stderr, "\n");
    exit(1);
}
