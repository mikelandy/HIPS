/*
 * file_stubs.c - Notify and event callback function stubs.
 * This file was generated by `gxv' from `file.G'.
 */

#include "common.h"
#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/textsw.h>
#include <xview/xv_xrect.h>
#include "segal.h"

/*
 * Notify callback function for `but_close_image'.
 */
void
file_pop_load_image_but_close_image_notify_callback(item, event)
	Panel_item	item;
	Event		*event;
{
	file_pop_load_image_objects *ip = (file_pop_load_image_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */

	xv_set(File_pop_load_image->pop_load_image, XV_SHOW, FALSE, NULL);
	
	/* gxv_end_connections */

}

/*
 * Notify callback function for `but_load_mask'.
 */
void
load_mask(item, event)
	Panel_item	item;
	Event		*event;
{
	void load_mask_proc();

	file_pop_load_mask_objects *ip = (file_pop_load_mask_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	
	load_mask_proc();
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */

	/* gxv_end_connections */

}

/*
 * Notify callback function for `but_close_mask'.
 */
void
file_pop_load_mask_but_close_mask_notify_callback(item, event)
	Panel_item	item;
	Event		*event;
{
	file_pop_load_mask_objects *ip = (file_pop_load_mask_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */

	xv_set(File_pop_load_mask->pop_load_mask, XV_SHOW, FALSE, NULL);
	
	/* gxv_end_connections */

}

/*
 * Notify callback function for `but_create_new'.
 */
void
create_new(item, event)
	Panel_item	item;
	Event		*event;
{
	file_pop_new_mask_objects *ip = (file_pop_new_mask_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */
	/* gxv_end_connections */

}

/*
 * Notify callback function for `but_close_new'.
 */
void
file_pop_new_mask_but_close_new_notify_callback(item, event)
	Panel_item	item;
	Event		*event;
{
	file_pop_new_mask_objects *ip = (file_pop_new_mask_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */

	xv_set(File_pop_new_mask->pop_new_mask, XV_SHOW, FALSE, NULL);
	
	/* gxv_end_connections */

}

/*
 * Notify callback function for `but_create_new'.
 */
void
create_mask(item, event)
	Panel_item	item;
	Event		*event;
{
	file_pop_new_mask_objects *ip = (file_pop_new_mask_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */
	/* gxv_end_connections */

}

/*
 * Notify callback function for `but_create_new'.
 */
void
file_pop_new_mask_but_create_new_notify_callback(item, event)
	Panel_item	item;
	Event		*event;
{
	file_pop_new_mask_objects *ip = (file_pop_new_mask_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */

	{
		new_mask_proc();
	}
	
	/* gxv_end_connections */

}

/*
 * Notify callback function for `but_save_mask_as'.
 */
void
file_pop_save_as_but_save_mask_as_notify_callback(item, event)
	Panel_item	item;
	Event		*event;
{
	file_pop_save_as_objects *ip = (file_pop_save_as_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */

	{
		save_mask_as();
	}
	
	/* gxv_end_connections */

}

/*
 * Event callback function for `but_close_save'.
 */
void
file_pop_save_as_but_close_save_event_callback(item, event)
	Panel_item	item;
	Event		*event;
{
	file_pop_save_as_objects *ip = (file_pop_save_as_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */

	xv_set(File_pop_save_as->pop_save_as, XV_SHOW, FALSE, NULL);
	
	/* gxv_end_connections */

	panel_default_handle_event(item, event);
}

/*
 * Notify callback function for `but_i_save_as'.
 */
void
file_pop_save_image_but_i_save_as_notify_callback(item, event)
	Panel_item	item;
	Event		*event;
{
	file_pop_save_image_objects *ip = (file_pop_save_image_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */

	{
		save_image_as();
	}
	
	/* gxv_end_connections */

}

/*
 * Notify callback function for `but_i_close_save_as'.
 */
void
file_pop_save_image_but_i_close_save_as_notify_callback(item, event)
	Panel_item	item;
	Event		*event;
{
	file_pop_save_image_objects *ip = (file_pop_save_image_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */

	xv_set(File_pop_save_image->pop_save_image, XV_SHOW, FALSE, NULL);
	
	/* gxv_end_connections */

}

/*
 * Notify callback function for `but_load_header'.
 */
void
file_pop_load_image_but_load_header_notify_callback(item, event)
	Panel_item	item;
	Event		*event;
{
	file_pop_load_image_objects *ip = (file_pop_load_image_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */
	/* gxv_end_connections */

}

/*
 * Notify callback function for `row_from'.
 */
Panel_setting
file_pop_load_image_row_from_notify_callback(item, event)
	Panel_item	item;
	Event		*event;
{
	file_pop_load_image_objects *ip = (file_pop_load_image_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	int	value = (int) xv_get(item, PANEL_VALUE);
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */

	{
		segal.r1 = value;
		if(segal.r2 < value) {
			segal.r2 = value;
			xv_set(File_pop_load_image->row_to,
				PANEL_VALUE, value,
				NULL);
		}
		update_bytes_required();
	}
	
	/* gxv_end_connections */

	return panel_text_notify(item, event);
}

/*
 * Notify callback function for `row_to'.
 */
Panel_setting
file_pop_load_image_row_to_notify_callback(item, event)
	Panel_item	item;
	Event		*event;
{
	file_pop_load_image_objects *ip = (file_pop_load_image_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	int	value = (int) xv_get(item, PANEL_VALUE);
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */

	{
		segal.r2 = value;
		if(segal.r1 > value) {
			segal.r1 = value;
			xv_set(File_pop_load_image->row_from,
				PANEL_VALUE, value,
				NULL);
		}
		update_bytes_required();
	}
	
	/* gxv_end_connections */

	return panel_text_notify(item, event);
}

/*
 * Notify callback function for `col_from'.
 */
Panel_setting
file_pop_load_image_col_from_notify_callback(item, event)
	Panel_item	item;
	Event		*event;
{
	file_pop_load_image_objects *ip = (file_pop_load_image_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	int	value = (int) xv_get(item, PANEL_VALUE);
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */

	{
		segal.c1 = value;
		if(segal.c2 < value) {
			segal.c2 = value;
			xv_set(File_pop_load_image->col_to,
				PANEL_VALUE, value,
				NULL);
		}
		update_bytes_required();
	}
	
	/* gxv_end_connections */

	return panel_text_notify(item, event);
}

/*
 * Notify callback function for `col_to'.
 */
Panel_setting
file_pop_load_image_col_to_notify_callback(item, event)
	Panel_item	item;
	Event		*event;
{
	file_pop_load_image_objects *ip = (file_pop_load_image_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	int	value = (int) xv_get(item, PANEL_VALUE);
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */

	{
		segal.c2 = value;
		if(segal.c1 > value) {
			segal.c1 = value;
			xv_set(File_pop_load_image->col_from,
				PANEL_VALUE, value,
				NULL);
		}
		update_bytes_required();
	}
	
	/* gxv_end_connections */

	return panel_text_notify(item, event);
}

/*
 * Notify callback function for `frm_from'.
 */
Panel_setting
file_pop_load_image_frm_from_notify_callback(item, event)
	Panel_item	item;
	Event		*event;
{
	file_pop_load_image_objects *ip = (file_pop_load_image_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	int	value = (int) xv_get(item, PANEL_VALUE);
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */

	{
		segal.f1 = value;
		if(segal.f2 < value) {
			segal.f2 = value;
			xv_set(File_pop_load_image->frm_to,
				PANEL_VALUE, value,
				NULL);
		}
		update_bytes_required();
	}
	
	/* gxv_end_connections */

	return panel_text_notify(item, event);
}

/*
 * Notify callback function for `frm_to'.
 */
Panel_setting
file_pop_load_image_frm_to_notify_callback(item, event)
	Panel_item	item;
	Event		*event;
{
	file_pop_load_image_objects *ip = (file_pop_load_image_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	int	value = (int) xv_get(item, PANEL_VALUE);
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */

	{
		segal.f2 = value;
		if(segal.f1 > value) {
			segal.f1 = value;
			xv_set(File_pop_load_image->frm_from,
				PANEL_VALUE, value,
				NULL);
		}
		update_bytes_required();
	}
	
	/* gxv_end_connections */

	return panel_text_notify(item, event);
}

/*
 * Notify callback function for `set_color_format'.
 */
void
file_pop_load_image_set_color_format_notify_callback(item, value, event)
	Panel_item	item;
	int		value;
	Event		*event;
{
	file_pop_load_image_objects *ip = (file_pop_load_image_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */

	if (value == 0)
	{
		segal.color = TRUE;
		update_bytes_required();
	}
	
	if (value == 1)
	{
		segal.color = FALSE;
		update_bytes_required();
		img.hd.color_form = CFM_SGF;
	}
	
	/* gxv_end_connections */

}

/*
 * Notify callback function for `but_load_image'.
 */
void
file_pop_load_image_but_load_image_notify_callback(item, event)
	Panel_item	item;
	Event		*event;
{
	file_pop_load_image_objects *ip = (file_pop_load_image_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */

	{
		load_image();
		
	}
	
	/* gxv_end_connections */

}
