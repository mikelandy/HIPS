/*
 * image_reg_stubs.c - Notify and event callback function stubs.
 * This file was generated by `gxv' from `image_reg.G'.
 */

#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/textsw.h>
#include <xview/xv_xrect.h>
#include "segal.h"


/*
 * Notify callback function for `but_close_image_reg'.
 */
void
image_reg_pop_image_reg_but_close_image_reg_notify_callback(item, event)
	Panel_item	item;
	Event		*event;
{
	image_reg_pop_image_reg_objects *ip = (image_reg_pop_image_reg_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */

	xv_set(Image_reg_pop_image_reg->pop_image_reg, FRAME_CMD_PUSHPIN_IN, FALSE, NULL);
	xv_set(Image_reg_pop_image_reg->pop_image_reg, XV_SHOW, FALSE, NULL);
	
	xv_set(Image_reg_pop_ref_frame->pop_ref_frame, FRAME_CMD_PUSHPIN_IN, FALSE, NULL);
	xv_set(Image_reg_pop_ref_frame->pop_ref_frame, XV_SHOW, FALSE, NULL);
	
	/* gxv_end_connections */

}
