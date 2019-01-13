/* Initialise X subsystem */

#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xutil.h>
#include <X11/Shell.h>
#include <Xol/OpenLook.h>
#include <xdisp.h>

static String fallback_resources[] = {
	"*beep:						never",
	"*traversalOn:					False",
	"*inputFocusColor:				black",
	"*background:					gray80",
	"*font:						lucidasans-typewriter",
	"*scale:					10",
	NULL
	};


/*********************************
 * x_init()
 *********************************/

void x_init(argc,argv)
  int 		*argc;
  char		*argv[];
{
  int 		n;
  int		i,j;

    /*
     * Initialize Intrinsics
     */

    OlToolkitInitialize((XtPointer)NULL);

    app_toplevel = XtVaAppInitialize(&app,
			PROGRAM_NAME,
			(XrmOptionDescList)NULL,0,
                        argc,argv,
			fallback_resources,
			XtNtitle,PROGRAM_NAME,
			XtNiconName,PROGRAM_NAME,
			XtNx,0,
			XtNy,0,
			NULL);


    /*
     * Obtain display data
     */

    dpy = XtDisplay(app_toplevel);
    visual = OlVisualOfObject(app_toplevel);
    depth = DefaultDepthOfScreen(XtScreen(app_toplevel));
    rws = DefaultRootWindow(XtDisplay(app_toplevel));
    app_colormap = OlColormapOfObject(app_toplevel);
    mentries = visual->map_entries;

    if (mentries > MAXCOLOR) mentries = MAXCOLOR;

    black_pixel = BlackPixel(dpy,DefaultScreen(dpy));
    white_pixel = WhitePixel(dpy,DefaultScreen(dpy));


    /*
     * Create other top level shells
     */

    app_control = XtVaAppCreateShell(app,
			CONTROL_NAME,
			applicationShellWidgetClass,
			dpy,
			XtNtitle,CONTROL_NAME,
			XtNiconName,CONTROL_NAME,
			NULL);

    app_sample = XtVaAppCreateShell(app,
			SCONTROL_NAME,
			applicationShellWidgetClass,
			dpy,
			XtNtitle,SCONTROL_NAME,
			XtNiconName,SCONTROL_NAME,
			NULL);

    app_pcontrol = XtVaAppCreateShell(app,
			PCONTROL_NAME,
			applicationShellWidgetClass,
			dpy,
			XtNtitle,PCONTROL_NAME,
			XtNiconName,PCONTROL_NAME,
			NULL);
    app_scontrol = XtVaAppCreateShell(app,
			SCONTROL_NAME,
			applicationShellWidgetClass,
			dpy,
			XtNtitle,SCONTROL_NAME,
			XtNiconName,SCONTROL_NAME,
			NULL);
    app_gcontrol = XtVaAppCreateShell(app,
			GCONTROL_NAME,
			applicationShellWidgetClass,
			dpy,
			XtNtitle,GCONTROL_NAME,
			XtNiconName,GCONTROL_NAME,
			NULL);
    app_ov_gcontrol = XtVaAppCreateShell(app,
			VGCONTROL_NAME,
			applicationShellWidgetClass,
			dpy,
			XtNtitle,VGCONTROL_NAME,
			XtNiconName,VGCONTROL_NAME,
			NULL);
    app_icontrol = XtVaAppCreateShell(app,
			ICONTROL_NAME,
			applicationShellWidgetClass,
			dpy,
			XtNtitle,ICONTROL_NAME,
			XtNiconName,ICONTROL_NAME,
			NULL);
    app_ycontrol = XtVaAppCreateShell(app,
			YCONTROL_NAME,
			applicationShellWidgetClass,
			dpy,
			XtNtitle,YCONTROL_NAME,
			XtNiconName,YCONTROL_NAME,
			NULL);
    app_fcontrol = XtVaAppCreateShell(app,
			FCONTROL_NAME,
			applicationShellWidgetClass,
			dpy,
			XtNtitle,FCONTROL_NAME,
			XtNiconName,FCONTROL_NAME,
			NULL);
    app_vcontrol = XtVaAppCreateShell(app,
			VCONTROL_NAME,
			applicationShellWidgetClass,
			dpy,
			XtNtitle,VCONTROL_NAME,
			XtNiconName,VCONTROL_NAME,
			NULL);
}


/*********************************
 * graphics_init()
 *********************************/

void graphics_init()
{
  int i;

    /*
     * Set up colors
     */

    switch (visual->class) {
	case StaticGray:
	case StaticColor:
	case TrueColor:
		fprintf(stderr,"colormap is non-writeable\n");
		exit(1);
		break;
	case GrayScale:		
		fprintf(stderr,"visual is GrayScale\n");
		break;
	case PseudoColor:		
		fprintf(stderr,"visual is PseudoColor\n");
		break;
	case DirectColor:		
		fprintf(stderr,"visual is DirectColor\n");
		break;
	}

    colormap = XCreateColormap(dpy,rws,visual,AllocAll);

    for (i = 0; i < mentries; i++) {
	colors[i].pixel = i;
	colors[i].flags = DoRed | DoGreen | DoBlue;
	}

    if (ncolors <= 0) {
	unsigned long cells[2];
	if (XAllocColorCells(dpy,app_colormap,True,NULL,0,cells,2) == True) {
	    ncolors = mentries-cells[0];
	    XFreeColors(dpy,app_colormap,cells,2,0);
	    }
	else {
	    ncolors = mentries/2;
	    fprintf(stderr,
		"unable to allocate consistent colors: using %d\n",ncolors);
	    }
	}
    else if (ncolors > mentries-2)
	ncolors = mentries-2;

    fprintf(stderr,"colors: %d/%d\n",ncolors,mentries);

    XQueryColors(dpy,app_colormap,colors,mentries);
    XStoreColors(dpy,colormap,colors,mentries);

    /*
     * set colormaps for top level widgets
     */

    XtVaSetValues(app_toplevel,
		XtNcolormap,colormap,
		NULL);
    XtVaSetValues(app_control,
		XtNcolormap,colormap,
		NULL);
    XtVaSetValues(app_sample,
		XtNcolormap,colormap,
		NULL);


    /*
     * Create graphics contexts for the drawing, rubber banding and
     * drawing polygons
     */

    gc = XCreateGC(dpy,rws,NULL,NULL);

    lgcv.foreground = white_pixel;
    lgcv.background = black_pixel;
    lgcv.function = GXcopy;

    lgc = XCreateGC(dpy,
	 	    rws,
	 	    GCFunction | GCForeground,
		    &lgcv);

    llgcv.foreground = black_pixel;
    llgcv.line_style = LineOnOffDash;
    llgcv.function = GXxor;

    llgc = XCreateGC(dpy,
	 	    rws,
	 	    GCFunction | GCForeground | GCLineStyle,
		    &llgcv);

    dgcv.foreground = white_pixel;
    dgcv.line_style = LineOnOffDash;
    dgcv.function = GXcopy;

    dgc = XCreateGC(dpy,
	 	    rws,
	 	    GCFunction | GCForeground | GCLineStyle,
		    &dgcv);

    rgcv.foreground = 128;
    rgcv.function = GXxor;

    rgc = XCreateGC(dpy,
	 	    rws,
	 	    GCFunction | GCForeground,
		    &rgcv);

}



/***************************
 * xloop()
 ***************************/

void xloop()

/* Enter main X loop */

{
    XtRealizeWidget(app_toplevel);
    set_cursor(draw,current_cursor);
    XtAppMainLoop(app);
}
