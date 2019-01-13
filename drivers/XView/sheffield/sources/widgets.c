#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xol/OpenLook.h>
#include <Xol/StaticText.h>
#include <Xol/RubberTile.h>
#include <Xol/ControlAre.h>
#include <Xol/DrawArea.h>
#include <Xol/Exclusives.h>
#include <Xol/Nonexclusi.h>
#include <Xol/Caption.h>
#include <Xol/RectButton.h>
#include <Xol/OblongButt.h>
#include <Xol/Slider.h>
#include <Xol/Notice.h>
#include <Xol/TextField.h>
#include <Xol/PopupWindo.h>
#include <Xol/Caption.h>
#include <xdisp.h>

static Widget rbs[128];
static char *frame_names[MAXFRAME];
static char *zoom_names[] = {"1","2","4","8"};
static char *window_names_1[] = {"Pan","Gamma","Properties"};
static char *window_names_2[] = {"Info","Polygons","File"};
static char *precision_names[] = {"0","1","2","3","4","5","6"};
static char *cdtype_names[] = {"rectangular","polar"};
static char *extreme_names[] = {"l/h","l/l","h/h","h/l"};
static char *cmy_names[] = {"RGB","CMY"};
static char *cache_names[] = {"off","on"};
static char *h_names[] = {"histogram","cdf"};
static char *poly_names[] = {"Define","Name","Edit","Delete","Hide","Clear",
				"Stats"};
static char *file_names[] = {"Load Polys","Save Polys",
				"Load Header","Save Header",
				"Print Image"};
static char *ov_actions[] = {"Draw"};
static char *diag_names[] = {"Diag"};

int n = 0;

static Widget control,pcontrol,gcontrol,ov_gcontrol,ycontrol,fcontrol,vcontrol;
static Widget a_upper,a_lower;
static Widget diag;

extern char *lut_names[];
extern char *cursor_names[];

void pointer_event_handler();
void button_press_event();
void button_motion_event();
void button_release_event();
void redisplay();
void resize();
void frame_callback();
void zoom_callback();
void lut_callback();
void cursor_callback();
void window_callback_1();
void window_callback_2();
void precision_callback();
void cdtype_callback();
void extreme_callback();
void cmy_callback();
void cache_callback();
void h_callback();
void slider_low_callback();
void slider_high_callback();
void poly_callback();
void file_callback();
void create_ex_nonex();
void create_obs();
void ask_yes_callback();
void ask_no_callback();
void ask_verification_callback();
void ov_red_frame_callback();
void ov_green_frame_callback();
void ov_blue_frame_callback();
void ov_callback();
void diag_callback();


/***************************
 * create_widgets()
 ***************************/
void create_widgets()
{
    int fr;
    int i;

    /*
     * Main window widgets
     */

    rt = XtVaCreateManagedWidget("rt",
				rubberTileWidgetClass,
				app_toplevel,
				XtNorientation,OL_VERTICAL,
				XtNbackground,black_pixel,
				NULL);

    text = XtVaCreateManagedWidget("text",
				staticTextWidgetClass,
				rt,
				XtNbackground,black_pixel,
				XtNfontColor,white_pixel,
				XtNweight,0,
				XtNgravity,NorthWestGravity,
				XtNrecomputeSize,False,
				NULL);

    draw = XtVaCreateManagedWidget("draw",
			drawAreaWidgetClass,
			rt,
			XtNwidth,(ncols > maxcols) ? maxcols : ncols,
			XtNheight,(nrows > maxrows) ? maxrows : nrows,
			XtNbackground,black_pixel,
			NULL);

    /*
     * Control window widgets 
     */

    control = XtVaCreateManagedWidget("control",
				controlAreaWidgetClass,
				app_control,
				XtNalignCaptions,TRUE,
				XtNlayoutType,OL_FIXEDCOLS,
				XtNmeasure,1,
				NULL);

    for (fr = 0; fr < nframes; fr++) {
	frame_names[fr] = (char *)malloc(4);
	sprintf(frame_names[fr],"%d",fr);
	}
    frame_names[nframes] = "*";

    create_ex_nonex(control,frame_names,exclusivesWidgetClass,
			nframes+(nframes > 1),"Frame:",frame_callback);
    create_ex_nonex(control,zoom_names,exclusivesWidgetClass,
			XtNumber(zoom_names),"Zoom:",zoom_callback);
    create_ex_nonex(control,lut_names+(1-hips_lut_exists),exclusivesWidgetClass,
			nluts+hips_lut_exists,"Lut:",lut_callback);
    create_ex_nonex(control,cursor_names,exclusivesWidgetClass,
			ncursors,"Cursor:",cursor_callback);
    create_ex_nonex(control,window_names_1,nonexclusivesWidgetClass,
			XtNumber(window_names_1),"Windows:",window_callback_1);
    create_ex_nonex(control,window_names_2,nonexclusivesWidgetClass,
			XtNumber(window_names_2)," ",window_callback_2);
/*    create_obs(control,diag_names,XtNumber(diag_names),diag_callback);
*/

    /*
     * properties
     */

    pcontrol = XtVaCreateManagedWidget("pcontrol",
				controlAreaWidgetClass,
				app_pcontrol,
				XtNalignCaptions,TRUE,
				XtNlayoutType,OL_FIXEDCOLS,
				XtNmeasure,1,
				NULL);
	
    create_ex_nonex(pcontrol,precision_names,exclusivesWidgetClass,
	    XtNumber(precision_names),"float precision:",precision_callback);
    create_ex_nonex(pcontrol,cdtype_names,exclusivesWidgetClass,
	    XtNumber(cdtype_names),"complex numbers:",cdtype_callback);
    create_ex_nonex(pcontrol,h_names,exclusivesWidgetClass,
	    XtNumber(h_names),"distributions:",h_callback);
    create_ex_nonex(pcontrol,extreme_names,exclusivesWidgetClass,
	    XtNumber(extreme_names),"extreme values:",extreme_callback);
    create_ex_nonex(pcontrol,cmy_names,exclusivesWidgetClass,
	    XtNumber(cmy_names),"color scheme:",cmy_callback);
    create_ex_nonex(pcontrol,cache_names,exclusivesWidgetClass,
	    XtNumber(cache_names),"pixmap cache:",cache_callback);

    /*
     * image sample 
     */

    sample = XtVaCreateManagedWidget("sample",
				drawAreaWidgetClass,
				app_scontrol,
				XtNwidth,scols,
				XtNheight,srows,
				XtNbackground,black_pixel,
				NULL);

    /*
     * gamma control 
     */

    gcontrol = XtVaCreateManagedWidget("gcontrol",
				controlAreaWidgetClass,
				app_gcontrol,
				XtNlayoutType,OL_FIXEDCOLS,
				XtNmeasure,1,
				NULL);
	
    hist[0] = XtVaCreateManagedWidget("hist",
      				drawAreaWidgetClass,
				gcontrol,
				XtNwidth,HIST_WIDTH,
				XtNheight,HIST_HEIGHT,
				XtNborderWidth,1,
				NULL);

    colorbar[0] = XtVaCreateManagedWidget("colorbar",
      				drawAreaWidgetClass,
				gcontrol,
				XtNwidth,HIST_WIDTH,
				XtNheight,COLORBAR_HEIGHT,
				XtNborderWidth,1,
				NULL);

    slider_low[0] =  XtVaCreateManagedWidget("slider_low",
				sliderWidgetClass,
				gcontrol,
				XtNorientation,OL_HORIZONTAL,
				XtNgranularity,1,
				XtNticks,16,
				XtNtickUnit,OL_SLIDERVALUE,
				XtNwidth,256,
				XtNsliderMax,255,
				XtNsliderValue,0,
				NULL);

    slider_high[0] =  XtVaCreateManagedWidget("slider_high",
				sliderWidgetClass,
				gcontrol,
				XtNorientation,OL_HORIZONTAL,
				XtNgranularity,1,
				XtNticks,16,
				XtNtickUnit,OL_SLIDERVALUE,
				XtNwidth,256,
				XtNsliderMax,255,
				XtNsliderValue,255,
				NULL);

    s_text[0] = XtVaCreateManagedWidget("s_text",
				staticTextWidgetClass,
				gcontrol,
				XtNgravity,NorthWestGravity,
				XtNstring,"0<>255",
				NULL);

    /*
     * overlay gamma control 
     */

    ov_gcontrol = XtVaCreateManagedWidget("ov_gcontrol",
				controlAreaWidgetClass,
				app_ov_gcontrol,
				XtNlayoutType,OL_FIXEDCOLS,
				XtNmeasure,1,
				NULL);

    for (i = 1; i < 4; i++) {
	
    	hist[i] = XtVaCreateManagedWidget("",
      				drawAreaWidgetClass,
				ov_gcontrol,
				XtNwidth,HIST_WIDTH,
				XtNheight,HIST_HEIGHT,
				XtNborderWidth,1,
				NULL);

    	colorbar[i] = XtVaCreateManagedWidget("",
      				drawAreaWidgetClass,
				ov_gcontrol,
				XtNwidth,HIST_WIDTH,
				XtNheight,COLORBAR_HEIGHT,
				XtNborderWidth,1,
				NULL);

    	slider_low[i] =  XtVaCreateManagedWidget("",
				sliderWidgetClass,
				ov_gcontrol,
				XtNorientation,OL_HORIZONTAL,
				XtNgranularity,1,
				XtNticks,16,
				XtNtickUnit,OL_SLIDERVALUE,
				XtNwidth,256,
				XtNsliderMax,255,
				XtNsliderValue,0,
				NULL);

    	slider_high[i] =  XtVaCreateManagedWidget("",
				sliderWidgetClass,
				ov_gcontrol,
				XtNorientation,OL_HORIZONTAL,
				XtNgranularity,1,
				XtNticks,16,
				XtNtickUnit,OL_SLIDERVALUE,
				XtNwidth,256,
				XtNsliderMax,255,
				XtNsliderValue,255,
				NULL);

    	s_text[i] = XtVaCreateManagedWidget("",
				staticTextWidgetClass,
				ov_gcontrol,
				XtNgravity,NorthWestGravity,
				XtNstring,"0<>255",
				NULL);
	}

    /*
     *  info 
     */

    info = XtVaCreateManagedWidget("info",
				staticTextWidgetClass,
				app_icontrol,
				NULL);

    /*
     * polygons
     */

    ycontrol = XtVaCreateManagedWidget("ycontrol",
				controlAreaWidgetClass,
				app_ycontrol,
				XtNalignCaptions,TRUE,
				XtNlayoutType,OL_FIXEDCOLS,
				XtNmeasure,1,
				NULL);
	
    create_ex_nonex(ycontrol,poly_names,nonexclusivesWidgetClass,
	    XtNumber(poly_names)," ",poly_callback);

    /*
     * file control
     */

    fcontrol = XtVaCreateManagedWidget("fcontrol",
				controlAreaWidgetClass,
				app_fcontrol,
				XtNlayoutType,OL_FIXEDCOLS,
				XtNmeasure,1,
				NULL);
	
    create_obs(fcontrol,file_names,XtNumber(file_names),file_callback);


    /*
     * frame chooser for overlays
     */

    vcontrol = XtVaCreateManagedWidget("vcontrol",
				controlAreaWidgetClass,
				app_vcontrol,
				XtNalignCaptions,TRUE,
				XtNlayoutType,OL_FIXEDCOLS,
				XtNmeasure,1,
				NULL);
	
    create_ex_nonex(vcontrol,frame_names,exclusivesWidgetClass,
			nframes,"Red:",ov_red_frame_callback);
    create_ex_nonex(vcontrol,frame_names,exclusivesWidgetClass,
			nframes,"Green:",ov_green_frame_callback);
    create_ex_nonex(vcontrol,frame_names,exclusivesWidgetClass,
			nframes,"Blue:",ov_blue_frame_callback);

    create_obs(vcontrol,ov_actions,XtNumber(ov_actions),ov_callback);

    /*
     * ask widgets
     */

    a_popup = XtVaCreatePopupShell("a_popup",
				popupWindowShellWidgetClass,
				app_toplevel,
				XtNx,0,
				XtNy,0,
				NULL);

    XtVaGetValues(a_popup,
		XtNupperControlArea,&a_upper,
		XtNlowerControlArea,&a_lower,
		NULL);
		
    a_caption = XtVaCreateManagedWidget("a_caption",
				captionWidgetClass,
				a_upper,
				XtNspace,8,
				NULL);

    a_text = XtVaCreateManagedWidget("a_text",
				textFieldWidgetClass,
				a_caption,
				XtNwidth,400,
				NULL);

    a_accept = XtVaCreateManagedWidget("a_accept",
				oblongButtonWidgetClass,
				a_lower,
				XtNlabel,"Confirm",
				NULL);

    a_cancel = XtVaCreateManagedWidget("a_cancel",
				oblongButtonWidgetClass,
				a_lower,
				XtNlabel,"Cancel",
				NULL);

    XtAddCallback(a_accept,XtNselect,ask_yes_callback,NULL);
    XtAddCallback(a_cancel,XtNselect,ask_no_callback,NULL);
    XtAddCallback(a_text,XtNverification,ask_verification_callback,NULL);

}



/***************************
 * create_ex_nonex()
 ***************************/
void create_ex_nonex(parent,names,class,number,label,callback)
  Widget 	parent;
  char		*names[];
  WidgetClass	class;
  int		number;
  String	label;
  void		(*callback)();
{

    /* Create a set of exclusive widgets */

    Widget caption,ex;
    int i;

    caption = XtVaCreateManagedWidget("",
				captionWidgetClass,
				parent,
				XtNlabel,label,
				NULL);

    ex = XtVaCreateManagedWidget("",
				class,
				caption,
				NULL);

    for (i = 0; i < number; i ++) {
	rbs[n] = XtVaCreateManagedWidget(names[i],
				rectButtonWidgetClass,
				ex,
				NULL);
	if (callback != NULL) {
	    XtAddCallback(rbs[n],XtNselect,callback,i);
	    if (class == nonexclusivesWidgetClass)
		XtAddCallback(rbs[n],XtNunselect,callback,i);
	    }
	n++;
	}
}



/***************************
 * create_obs()
 ***************************/
void create_obs(parent,names,number,callback)
  Widget 	parent;
  char		*names[];
  int		number;
  void		(*callback)();
{

    /* Create a set of oblong button widgets */

    int i;

    for (i = 0; i < number; i ++) {
	rbs[n] = XtVaCreateManagedWidget(names[i],
				oblongButtonWidgetClass,
				parent,
				NULL);
	if (callback != NULL)
	    XtAddCallback(rbs[n],XtNselect,callback,i);
	n++;
	}
}



/***************************
 * add_events()
 ***************************/
void add_events()
{
  int i;

    XtVaSetValues(app_toplevel,
		XtNcolormap,colormap,
		NULL);
    XtVaSetValues(app_control,
		XtNcolormap,colormap,
		NULL);
    XtVaSetValues(app_sample,
		XtNcolormap,colormap,
		NULL);
    XtVaSetValues(app_pcontrol,
		XtNcolormap,colormap,
		NULL);
    XtVaSetValues(app_scontrol,
		XtNcolormap,colormap,
		NULL);
    XtVaSetValues(app_gcontrol,
		XtNcolormap,colormap,
		NULL);
    XtVaSetValues(app_ov_gcontrol,
		XtNcolormap,colormap,
		NULL);
    XtVaSetValues(app_icontrol,
		XtNcolormap,colormap,
		NULL);
    XtVaSetValues(app_ycontrol,
		XtNcolormap,colormap,
		NULL);
    XtVaSetValues(app_fcontrol,
		XtNcolormap,colormap,
		NULL);
    XtVaSetValues(app_vcontrol,
		XtNcolormap,colormap,
		NULL);
    XtVaSetValues(draw,
		XtNcolormap,colormap,
		NULL);
    XtVaSetValues(sample,
		XtNcolormap,colormap,
		NULL);
    for (i = 0; i < 4; i++)
        XtVaSetValues(colorbar[i],
		XtNcolormap,colormap,
		NULL);
    for (i = 0; i < 4; i++)
        XtVaSetValues(hist[i],
		XtNcolormap,colormap,
		NULL);

    XtAddEventHandler(draw,PointerMotionMask,FALSE,
                      	pointer_event_handler,text);
    XtAddEventHandler(draw,ButtonPressMask,FALSE,
                        button_press_event,NULL);
    XtAddEventHandler(draw,Button1MotionMask,FALSE,
                      button_motion_event,NULL);
    XtAddEventHandler(draw,ButtonReleaseMask,FALSE,
                      button_release_event,NULL);
    XtAddEventHandler(sample,Button1MotionMask,FALSE,
                      button_motion_event,NULL);
    XtAddEventHandler(sample,ButtonReleaseMask,FALSE,
                      button_release_event,NULL);

    XtAddCallback(draw,XtNexposeCallback,redisplay,&image_pix_data);
    XtAddCallback(draw,XtNresizeCallback,resize,&image_pix_data);

    XtAddCallback(sample,XtNexposeCallback,redisplay,&sample_pix_data);
    XtAddCallback(sample,XtNresizeCallback,resize,&sample_pix_data);

    for (i = 0; i < 4; i++) {
        XtAddCallback(colorbar[i],XtNexposeCallback,
		redisplay,&colorbar_pix_data[i]);
        XtAddCallback(colorbar[i],XtNresizeCallback,
		resize,&colorbar_pix_data[i]);
    	XtAddCallback(hist[i],XtNexposeCallback,redisplay,&hist_pix_data[i]);
    	XtAddCallback(hist[i],XtNresizeCallback,resize,&hist_pix_data[i]);
    	XtAddCallback(slider_low[i],XtNsliderMoved,slider_low_callback,i);
    	XtAddCallback(slider_high[i],XtNsliderMoved,slider_high_callback,i);
	}
}

