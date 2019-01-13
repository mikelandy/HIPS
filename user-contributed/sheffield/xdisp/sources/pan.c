/* routines for controlling panning round sampled image */

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <xdisp.h>

void update_pan_details();
void draw_pan_box();


/*********************************
 * track_pan_movement()
 *********************************/

void track_pan_movement(r,c,end)
  int r;
  int c;
  int end;
{
    r = r*sample_rate;
    c = c*sample_rate;

    if (c < 0) c = 0;
    if (r < 0) r = 0;

    update_pan_details(r,c);

    if (end) {
	pan_box_data.old_w = pan_box_data.old_h = 0;
	XClearArea(dpy,XtWindow(sample),0,0,0,0,True);
	}
    else if (XtIsRealized(sample))
	draw_pan_box();

    /* force a redraw of full image if necessary */

    if (end && XtIsRealized(draw)) {
	image_pix_data.width = image_pix_data.height = 0;
    	resize(draw,&image_pix_data,NULL);
	}
}



/*****************************************
 * draw_pan_box()
 *****************************************/

void draw_pan_box()

/* draw a box on the sub image window corresponding to the viewed area.
   Drawing is done in Xor mode, so the old box has to be deleted first. */

{
    /*
     * delete old box if necessary
     */

    if (pan_box_data.old_w) 
    	XDrawRectangle(XtDisplay(sample),
			XtWindow(sample),
			rgc,
			pan_box_data.old_c,
			pan_box_data.old_r,
			pan_box_data.old_w,
			pan_box_data.old_h);

    /*
     * draw new box
     */

    XDrawRectangle(XtDisplay(sample),
		XtWindow(sample),
		rgc,
		pan_box_data.c,
		pan_box_data.r,
		pan_box_data.w,
		pan_box_data.h);

    /*
     * update old box details
     */

    pan_box_data.old_c = pan_box_data.c;
    pan_box_data.old_r = pan_box_data.r;
    pan_box_data.old_w = pan_box_data.w;
    pan_box_data.old_h = pan_box_data.h;
}



/*****************************************
 * update_pan_details()
 *****************************************/

void update_pan_details(or,oc)
  int	or;
  int	oc;

{
    Dimension w,h;
    int mag = image_pix_data.mag;

    XtVaGetValues(draw,			/* get view area dimensions */
		XtNwidth,&w,
		XtNheight,&h,
		NULL);

    w/= mag;
    h/= mag;

    if (h > image_pix_data.max_height) h = image_pix_data.max_height;
    if (w > image_pix_data.max_width) w = image_pix_data.max_width;

    if (or+h > image_pix_data.max_height) or = image_pix_data.max_height-h;
    if (oc+w > image_pix_data.max_width) oc = image_pix_data.max_width-w;

    image_pix_data.origin.r = or;
    image_pix_data.origin.c = oc;

    pan_box_data.r = or/sample_rate;
    pan_box_data.c = oc/sample_rate;
    pan_box_data.w = w/sample_rate;
    pan_box_data.h = h/sample_rate;

}



