/* Callback routines */

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xol/OpenLook.h>
#include <Xol/DrawArea.h>
#include <Xol/Slider.h>
#include <xdisp.h>

extern lut_t *lut_list[];
extern Cursor cursor_list[];

static int mags[] = {1,2,4,8};
static lut_t *nov_lut;

void place_window();
void update_pan_details();
void add_final_vertex();
void load_polys();
void save_polys();
void load_header();
void save_header();
void print_image();
void slider_low_callback();
void slider_high_callback();


/***************************
 * frame_callback()
 ***************************/

void frame_callback(w,client_data,call_data)
  Widget	w;
  XtPointer	client_data;
  XtPointer	call_data;

/* Callback to handle change of frame */

{
    int i;
    int fr = (int)client_data;

    frame = fr;

    if (frame >= nframes) {
	if (!XtIsRealized(app_vcontrol)) {
	    place_window(app_vcontrol);
	    XtRealizeWidget(app_vcontrol);
	    nov_lut = current_lut;
	    }
	if (images[frame] == NULL) return;
	set_lut(lut_list[nluts]);
    	if (XtIsRealized(app_gcontrol)) {
	    XtUnrealizeWidget(app_gcontrol);
	    place_window(app_ov_gcontrol);
	    XtRealizeWidget(app_ov_gcontrol);
	    }
	}
    else {
    	hist_pix_data[0].pix = (cdf_mode) ? cdf_pixmaps[frame] :
		 hist_pixmaps[frame];

    	if (XtIsRealized(app_vcontrol)) {
	    XtUnrealizeWidget(app_vcontrol);
	    set_lut(nov_lut);
	    }
    	if (XtIsRealized(app_ov_gcontrol)) {
	    XtUnrealizeWidget(app_ov_gcontrol);
	    place_window(app_gcontrol);
	    XtRealizeWidget(app_gcontrol);
	    }
	}

    sample_pix_data.width = sample_pix_data.height = 0;
    sample_pix_data.im = samples[frame];
    image_pix_data.width = image_pix_data.height = 0;
    image_pix_data.im = images[frame];
    resize(draw,&image_pix_data,NULL);

    if (XtIsRealized(app_scontrol))
	resize(sample,&sample_pix_data,NULL);

    if (XtIsRealized(app_gcontrol)) {
	XClearArea(dpy,XtWindow(hist[0]),0,0,0,0,True);
	resize(colorbar[frame],colorbar_pix_data+frame,NULL);
	}
}


void place_window(w)
  Widget	w;
{
    Dimension width,height;
    Position x,y;

    XtVaGetValues(app_control,
		XtNx,&x,
		XtNy,&y,
		XtNwidth,&width,
		XtNheight,&height,
		NULL);
    XtVaSetValues(w,
		XtNx,x-4,
		XtNy,y+height+5,
		NULL);
}


/***************************
 * zoom_callback()
 ***************************/

void zoom_callback(w,client_data,call_data)
  Widget	w;
  XtPointer	client_data;
  XtPointer	call_data;

/* Callback to handle change of magnification */

{
    int m = (int)client_data;
    image_pix_data.mag = mags[m];
    image_pix_data.width = image_pix_data.height = 0;

    update_pan_details(image_pix_data.origin.r,image_pix_data.origin.c);

    resize(draw,&image_pix_data,NULL);

    if (XtIsRealized(app_scontrol))
	resize(sample,&sample_pix_data,NULL);
}


/***************************
 * lut_callback()
 ***************************/

void lut_callback(w,client_data,call_data)
  Widget	w;
  XtPointer	client_data;
  XtPointer	call_data;

/* Callback to handle change of lut */

{
    int l = (int)client_data;
    if (frame < nframes) {
	changing_lut = 1;
	set_lut(lut_list[l]);
	} 
}


/***************************
 * cursor_callback()
 ***************************/

void cursor_callback(w,client_data,call_data)
  Widget	w;
  XtPointer	client_data;
  XtPointer	call_data;

/* Callback to handle change of cursor */

{
    int c = (int)client_data;
    set_cursor(draw,cursor_list[c]); 
}


/***************************
 * window_callback_1()
 ***************************/

void window_callback_1(w,client_data,call_data)
  Widget	w;
  XtPointer	client_data;
  XtPointer	call_data;

/* Callback to handle change of window */

{
    int win = (int)client_data;
    Dimension width,height;
    Position x,y;

    switch(win) {
	case 0:	if (XtIsRealized(app_scontrol))
		    XtUnrealizeWidget(app_scontrol);
		else {
		    resize(sample,&sample_pix_data,NULL);
		    place_window(app_scontrol);
    		    pan_box_data.old_w = 0;
    		    pan_box_data.old_h = 0;
		    XtRealizeWidget(app_scontrol);
		    }
		break;
	case 1:	if (XtIsRealized(app_gcontrol))
		    XtUnrealizeWidget(app_gcontrol);
		else if (XtIsRealized(app_ov_gcontrol))
		    XtUnrealizeWidget(app_ov_gcontrol);
		else if (frame < nframes) {
		    resize(colorbar[0],colorbar_pix_data+0,NULL);
		    place_window(app_gcontrol);
		    XtRealizeWidget(app_gcontrol);
		    }
		else {
		    resize(colorbar[1],colorbar_pix_data+1,NULL);
		    resize(colorbar[2],colorbar_pix_data+2,NULL);
		    resize(colorbar[3],colorbar_pix_data+3,NULL);
		    place_window(app_ov_gcontrol);
		    XtRealizeWidget(app_ov_gcontrol);
		    }
		break;
	case 2:	if (XtIsRealized(app_pcontrol))
		    XtUnrealizeWidget(app_pcontrol);
		else {
		    place_window(app_pcontrol);
		    XtRealizeWidget(app_pcontrol);
		    }
		break;
	}

}


/***************************
 * window_callback_2()
 ***************************/

void window_callback_2(w,client_data,call_data)
  Widget	w;
  XtPointer	client_data;
  XtPointer	call_data;

/* Callback to handle change of window */

{
    int win = (int)client_data;
    Dimension width,height;
    Position x,y;

    switch(win) {
	case 0:	if (XtIsRealized(app_icontrol))
		    XtUnrealizeWidget(app_icontrol);
		else {
	    	    XtVaSetValues(info,
			XtNstring,formatheaderc(&hdr,False),
			NULL);
		    place_window(app_icontrol);
		    XtRealizeWidget(app_icontrol);
		    }
		break;
	case 1:	if (XtIsRealized(app_ycontrol))
		    XtUnrealizeWidget(app_ycontrol);
		else {
		    place_window(app_ycontrol);
		    XtRealizeWidget(app_ycontrol);
		    }
		break;
	case 2:	if (XtIsRealized(app_fcontrol))
		    XtUnrealizeWidget(app_fcontrol);
		else {
		    place_window(app_fcontrol);
		    XtRealizeWidget(app_fcontrol);
		    }
		break;
	}
}



/***************************
 * precision_callback()
 ***************************/

void precision_callback(w,client_data,call_data)
  Widget	w;
  XtPointer	client_data;
  XtPointer	call_data;

/* Callback to handle change of precision */

{
    precision = (int)client_data;
}


/***************************
 * cdtype_callback()
 ***************************/

void cdtype_callback(w,client_data,call_data)
  Widget	w;
  XtPointer	client_data;
  XtPointer	call_data;

/* Callback to handle change of cdtype */

{
    cdconv = (int)client_data;
}


/***************************
 * extreme_callback()
 ***************************/

void extreme_callback(w,client_data,call_data)
  Widget	w;
  XtPointer	client_data;
  XtPointer	call_data;

/* Callback to handle change of extreme lut values */

{
  int i;

    extreme_type = (int)client_data;

    for (i = 0; i < 4; i++) {
	slider_low_callback(slider_low[i],i,&gamma_limits[i].ll);
	slider_high_callback(slider_high[i],i,&gamma_limits[i].ul);
	}
}


/***************************
 * cmy_callback()
 ***************************/

void cmy_callback(w,client_data,call_data)
  Widget	w;
  XtPointer	client_data;
  XtPointer	call_data;

/* Callback to handle change of RGB/CMY color scheme */

{
  int i;

    cmy_state = (int)client_data;

    for (i = 1; i < 4; i++) {
	slider_low_callback(slider_low[i],i,&gamma_limits[i].ll);
	slider_high_callback(slider_high[i],i,&gamma_limits[i].ul);
	}
}


/***************************
 * cache_callback()
 ***************************/

void cache_callback(w,client_data,call_data)
  Widget	w;
  XtPointer	client_data;
  XtPointer	call_data;

/* Callback to handle pixmap caching state */

{
    pix_cache_on = (int)client_data;
}


/***************************
 * h_callback()
 ***************************/

void h_callback(w,client_data,call_data)
  Widget	w;
  XtPointer	client_data;
  XtPointer	call_data;

/* Callback to handle change of histogram/cdf choice */

{
    cdf_mode = (int)client_data;
    hist_pix_data[0].pix = (cdf_mode) ? cdf_pixmaps[frame] : 
				hist_pixmaps[frame];
    hist_pix_data[1].pix = (cdf_mode) ? cdf_pixmaps[red_frame] : 
				hist_pixmaps[red_frame];
    hist_pix_data[2].pix = (cdf_mode) ? cdf_pixmaps[green_frame] : 
				hist_pixmaps[green_frame];
    hist_pix_data[3].pix = (cdf_mode) ? cdf_pixmaps[blue_frame] : 
				hist_pixmaps[blue_frame];

    if (XtIsRealized(app_gcontrol))
	XClearArea(dpy,XtWindow(hist[0]),0,0,0,0,True);
    else if (XtIsRealized(app_ov_gcontrol)) {
	XClearArea(dpy,XtWindow(hist[1]),0,0,0,0,True);
	XClearArea(dpy,XtWindow(hist[2]),0,0,0,0,True);
	XClearArea(dpy,XtWindow(hist[3]),0,0,0,0,True);
	}
}


/***************************
 * slider_low_callback()
 ***************************/

void slider_low_callback(w,client_data,call_data)
  Widget      w;
  XtPointer   client_data,call_data;
{
  int tmp = *(int *)call_data;
  int i = (int)client_data;
  gamma_limits_t *limits = gamma_limits+i;

    update_limit_lines(i);

    if (tmp > limits->ul) {
        XtVaSetValues(slider_high[i],XtNsliderValue,tmp,NULL);
	limits->ul = tmp;
        }

    XtVaSetValues(slider_low[i],XtNsliderValue,tmp,NULL);

    limits->ll = tmp;
    set_xmap(i,limits->ll,limits->ul);
    set_lut(current_lut);
    update_limit_lines(i);
    update_limit_text(i);
}


/***************************
 * slider_high_callback()
 ***************************/

void slider_high_callback(w,client_data,call_data)
  Widget      w;
  XtPointer   client_data,call_data;
{
  int tmp = *(int *)call_data;
  int i = (int)client_data;
  gamma_limits_t *limits = gamma_limits+i;

    update_limit_lines(i);

    if (tmp < limits->ll) {
        XtVaSetValues(slider_low[i],XtNsliderValue,tmp,NULL);
	limits->ll = tmp;
        }

    XtVaSetValues(slider_high[i],XtNsliderValue,tmp,NULL);

    limits->ul = tmp;
    set_xmap(i,limits->ll,limits->ul);
    set_lut(current_lut);
    update_limit_lines(i);
    update_limit_text(i);
}



/***************************
 * poly_callback()
 ***************************/

void poly_callback(w,client_data,call_data)
  Widget      w;
  XtPointer   client_data,call_data;
{
  int action = (int)client_data;

    switch (action) {
	case 0:	if (defining_poly) {
		    defining_poly = 0;
    		    if (current_polygon == NULL) return;
		    add_final_vertex();
		    }
		else {
		    defining_poly = 1;
		    }
		break;
	case 1:	naming_polys = 1-naming_polys;
		break;
	case 2:	editing_polys = 1-editing_polys;
		break;
	case 3:	deleting_polys = 1-deleting_polys;
		break;
	case 4: showing_polys = 1-showing_polys;
		XClearArea(dpy,XtWindow(draw),0,0,0,0,True);
		break;
	case 5:	clear_polys();
		XtVaSetValues(w,XtNset,FALSE,NULL);
		break;
	case 6:	do_stats();
		XtVaSetValues(w,XtNset,FALSE,NULL);
		break;
	}
}


/***************************
 * file_callback()
 ***************************/

void file_callback(w,client_data,call_data)
  Widget      w;
  XtPointer   client_data,call_data;
{
  int action = (int)client_data;

    switch(action) {
	case 0:	ask("Load Polys","File name:",def_file_name,load_polys);
		break;
	case 1:	ask("Save Polys","File name:",def_file_name,save_polys);
		break;
	case 2:	ask("Load Header","File name:",def_file_name,load_header);
		break;
	case 3:	ask("Save Header","File name:",def_file_name,save_header);
		break;
	case 4:	ask("Print Image","File name:",def_file_name,print_image);
		break;
	}
}


/***************************
 * ov_callback()
 ***************************/

void ov_callback(w,client_data,call_data)
  Widget      w;
  XtPointer   client_data,call_data;
{
  int action = (int)client_data;
  int i;

    switch(action) {
	case 0:	ov_create_frame();
		set_lut(lut_list[nluts]);
    		sample_pix_data.width = sample_pix_data.height = 0;
    		sample_pix_data.im = samples[frame];
    		image_pix_data.width = image_pix_data.height = 0;
    		image_pix_data.im = images[frame];
    		resize(draw,&image_pix_data,NULL);
    for (i = 1; i < 4; i++) {
	colorbar_pix_data[i].width = colorbar_pix_data[i].height = 0;
	resize(colorbar[i],&colorbar_pix_data[i],NULL);
	}
		if (XtIsRealized(app_gcontrol)) {
		    XtUnrealizeWidget(app_gcontrol);
		    place_window(app_ov_gcontrol);
		    XtRealizeWidget(app_ov_gcontrol);
		    }
	        if (XtIsRealized(app_scontrol))
		    resize(sample,&sample_pix_data,NULL);
		break;
	}
}


/***************************
 * ov frame callbacks
 ***************************/

void ov_red_frame_callback(w,client_data,call_data)
  Widget      w;
  XtPointer   client_data,call_data;
{
  int fr = (int)client_data;

    red_frame = fr;
}


void ov_green_frame_callback(w,client_data,call_data)
  Widget      w;
  XtPointer   client_data,call_data;
{
  int fr = (int)client_data;

    green_frame = fr;
}


void ov_blue_frame_callback(w,client_data,call_data)
  Widget      w;
  XtPointer   client_data,call_data;
{
  int fr = (int)client_data;

    blue_frame = fr;
}


/***************************
 * diag callbacks
 ***************************/

void diag_callback(w,client_data,call_data)
  Widget      w;
  XtPointer   client_data,call_data;
{
  int i;

    for (i = 0; i < 256; i++) {
	if (i%16 == 0) printf("\n");
	printf("%d ",hists[0][i]);
	}
    printf("\n");
}


