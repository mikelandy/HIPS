/* Event handlers */

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xol/DrawArea.h>
#include <xdisp.h>

void set_pointer_data();
void clear_pointer_data();
void touch_polys();
void delete_poly();


/********************************************
 * pointer_event_handler()
 ********************************************/

void pointer_event_handler(w,client_data,event,continue_to_dispatch)
  Widget 	w;
  XtPointer 	client_data;
  XEvent 	*event;
  Boolean 	*continue_to_dispatch;

  /* Called whenever the mouse moves within the widget w.
   * Display the coordinates and pixel value in the widget
   * described by client_data
   */
{
  int r,c;
  Widget t = (Widget)client_data;
  char s[64];

    xytorc(event->xbutton.x,event->xbutton.y,r,c);

    if (r < nrows && c < ncols && r >= 0 && c >= 0) {
	get_data_by_name(frame,r,c,s);
        set_pointer_data(t,r,c,s,images[frame][r*ncols+c]);
	}
    else
        clear_pointer_data(t);
 
}


/********************************************
 * button_press_event()
 ********************************************/

void button_press_event(w,client_data,event,continue_to_dispatch)
  Widget 	w;
  XtPointer 	client_data;
  XEvent 	*event;
  Boolean 	*continue_to_dispatch;

{
  int c,r;
  int button = event->xbutton.button;
  Dimension width,height;
  Position x,y;

    xytorc(event->xbutton.x,event->xbutton.y,r,c);

    if (button == Button1) {
	if (w == draw) {
	    if (defining_poly) add_poly_vertex(r,c);
	    else if (naming_polys) name_poly(r,c);
	    else if (editing_polys) edit_poly_start(r,c);
	    else if (deleting_polys) delete_poly(r,c);
	    }
	}
    else if (button == Button3) {
	if (w == draw)
	    if (XtIsRealized(app_control))
		XtUnrealizeWidget(app_control);
	    else {
	    	XtVaGetValues(app_toplevel,
			XtNx,&x,
			XtNy,&y,
			XtNwidth,&width,
			XtNheight,&height,
			NULL);
	   	 XtVaSetValues(app_control,
			XtNx,x+width+5,
			XtNy,y-25+1,
			NULL);
		XtRealizeWidget(app_control);
		}
	}
}
    


/********************************************
 * button_motion_event()
 ********************************************/

void button_motion_event(w,client_data,event,continue_to_dispatch)
  Widget 	w;
  XtPointer 	client_data;
  XEvent 	*event;
  Boolean 	*continue_to_dispatch;

{
  int r = event->xbutton.y;
  int c = event->xbutton.x;
  int button = event->xbutton.button;
  int state = event->xbutton.state;

    if (button == Button1 && state & Button1Mask) {  /* workaround!! */
	if (w == sample) 
	    track_pan_movement(r,c,False);
	else if (w == draw && in_poly_edit) {
    	    xytorc(event->xbutton.x,event->xbutton.y,r,c);
	    edit_poly_track(r,c);
	    }
	}
}
    


/********************************************
 * button_release_event()
 ********************************************/

void button_release_event(w,client_data,event,continue_to_dispatch)
  Widget 	w;
  XtPointer 	client_data;
  XEvent 	*event;
  Boolean 	*continue_to_dispatch;

{
  int r = event->xbutton.y;
  int c = event->xbutton.x;
  int button = event->xbutton.button;
  int state = event->xbutton.state;

    if (button == Button1 && state & Button1Mask) {  /* workaround!! */
	if (w == sample) 
	    track_pan_movement(r,c,True);
	else if (w == draw && in_poly_edit) {
    	    xytorc(event->xbutton.x,event->xbutton.y,r,c);
	    edit_poly_end(r,c);
	    }
	}
}
    


/*********************************
 * redisplay()
 *********************************/

void redisplay(w,client_data,call_data)
  Widget	w;
  XtPointer	client_data;
  XtPointer	call_data;

/* Redisplay contents of widget w after an expose event */

{
  OlDrawAreaCallbackStruct *cb = (OlDrawAreaCallbackStruct *)call_data;
  pix_data_t *idp = (pix_data_t *)client_data;

    /*
     * Extract exposed area from the event and copy from
     * pixmap to window
     */

    XCopyArea(dpy,idp->pix,XtWindow(w),idp->gc,0,0,
 		idp->width,idp->height,0,0);

    /* if any extra drawing required, do it here */

    if (w == draw) {
	touch_polys();
    	if (XtIsRealized(sample))
	    XClearArea(dpy,XtWindow(sample),0,0,0,0,True);
	}
    else if (w == sample) {
	pan_box_data.old_w = 0;
    	pan_box_data.old_h = 0;
	draw_pan_box();
	}
    else if (w == hist[0])
    	update_limit_lines(0);
    else if (w == hist[1])
    	update_limit_lines(1);
    else if (w == hist[2])
    	update_limit_lines(2);
    else if (w == hist[3])
    	update_limit_lines(3);
}


/*********************************
 * resize()
 *********************************/

void resize(w,client_data,call_data)
  Widget	w;
  XtPointer	client_data;
  XtPointer	call_data;
{
    Dimension width,height;
    pix_data_t *idp = (pix_data_t *)client_data;
    int mag = idp->mag;
    int nc = idp->max_width;
    int nr = idp->max_height;
    int redraw;

    XtVaGetValues(w,			/* get new dimensions */
		XtNwidth,&width,
		XtNheight,&height,
		NULL);

    if (XtIsRealized(w))
	XClearArea(dpy,XtWindow(w),0,0,0,0,True);

    /* if pixmap is big enough, don't redraw */

    if (idp->width == 0 || idp->height == 0) redraw = True;
    else if (width < idp->width && height < idp->height) redraw = False;
    else if (idp->width >= nc*mag && idp->height >= nr*mag) redraw = False;
    else redraw = True;

    update_pan_details(image_pix_data.origin.r,image_pix_data.origin.c);

    if (redraw == False) return;

    idp->width = (width > mag*nc) ? mag*nc : width+mag;
    idp->height = (height > mag*nr) ? mag*nr : height+mag;

    if (idp->pix) 
	XFreePixmap(dpy,idp->pix);

    idp->pix = XCreatePixmap(dpy,rws,idp->width,idp->height,idp->depth);
    draw_image(idp);
}



/*****************************************
 * set_pointer_data
 *****************************************/

void set_pointer_data(w,r,c,v,pv)
  Widget	w;
  int		r,c;
  char		*v;
  int		pv;
{
    /* 
     * w is the text widget
     * r,c are the pointer coordinates
     * v is the value at r,c
     * pv is the pixel value at r,c
     */

    char s[64];

    sprintf(s,"[%d] (%d,%d) = %s",frame,r,c,v);

    XtVaSetValues(w,
		XtNstring,s,
		NULL);
}


/*****************************************
 * clear_pointer_data
 *****************************************/

void clear_pointer_data(w)
  Widget	w;
{
    char s[32];

    sprintf(s,"[%d]",frame);

    XtVaSetValues(w,
		XtNstring,s,
		NULL);
}


