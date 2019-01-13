/*
 * histo_stubs.c - Notify and event callback function stubs.
 * This file was generated by `gxv' from `histo.G'.
 */

#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/textsw.h>
#include <xview/xv_xrect.h>
#include "histo_ui.h"
#include <group.h>
#include "gen.h"
#include "genial.h"


/*
 * Global object definitions.
 */
histo_histo_ctrl_objects	*Histo_histo_ctrl;
histo_display_objects	*Histo_display;

#ifdef MAIN

/*
 * Instance XV_KEY_DATA key.  An instance is a set of related
 * user interface objects.  A pointer to an object's instance
 * is stored under this key in every object.  This must be a
 * global variable.
 */
Attr_attribute	INSTANCE;

main(argc, argv)
	int	argc;
	char	**argv;
{
	/*
	 * Initialize XView.
	 */
	xv_init(XV_INIT_ARGC_PTR_ARGV, &argc, argv, NULL);
	INSTANCE = xv_unique_key();
	
	/*
	 * Initialize user interface components.
	 * Do NOT edit the object initializations by hand.
	 */
	Histo_histo_ctrl = histo_histo_ctrl_objects_initialize(NULL, NULL);
	Histo_display = histo_display_objects_initialize(NULL, NULL);
	
	
	/*
	 * Turn control over to XView.
	 */
	xv_main_loop(Histo_display->display);
	exit(0);
}

#endif


/*
 * Notify callback function for `histno'.
 */
Panel_setting
histno_proc(item, event)
	Panel_item	item;
	Event		*event;
{
	histo_histo_ctrl_objects *ip = (histo_histo_ctrl_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	int	value = (int) xv_get(item, PANEL_VALUE);
	
	fprintf(stderr, "histo: histno_proc: value: %d\n", value);
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */

	/* gxv_end_connections */

	return panel_text_notify(item, event);
}

/*
 * Notify callback function for `clear_all'.
 */
void
clear_histo_color_proc(item, event)
	Panel_item	item;
	Event		*event;
{
	histo_histo_ctrl_objects *ip = (histo_histo_ctrl_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	
	fputs("histo: clear_histo_color_proc\n", stderr);
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */

	/* gxv_end_connections */

}

/*
 * Notify callback function for `min1'.
 */
Panel_setting
interval1_proc(item, event)
	Panel_item	item;
	Event		*event;
{
	histo_histo_ctrl_objects *ip = (histo_histo_ctrl_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	int	value = (int) xv_get(item, PANEL_VALUE);
	
	fprintf(stderr, "histo: interval1_proc: value: %d\n", value);
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */

	/* gxv_end_connections */

	return panel_text_notify(item, event);
}

/*
 * Notify callback function for `pnt1'.
 */
void
histo_color_paint_proc(item, value, event)
	Panel_item	item;
	int		value;
	Event		*event;
{
	histo_histo_ctrl_objects *ip = (histo_histo_ctrl_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	short	i;
	
	fputs("histo: histo_color_paint_proc\n", stderr);
	for (i = 0; i < 1; i++) {
		if (value & 01)
			fprintf(stderr, "\t%dth item selected\n", i);
		
		/* gxv_start_connections DO NOT EDIT THIS SECTION */

		/* gxv_end_connections */

		value >>= 1;
	}
}

/*
 * Notify callback function for `sel1'.
 */
void
histo_color_select_proc(item, value, event)
	Panel_item	item;
	int		value;
	Event		*event;
{
	histo_histo_ctrl_objects *ip = (histo_histo_ctrl_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	short	i;
	
	fputs("histo: histo_color_select_proc\n", stderr);
	for (i = 0; i < 1; i++) {
		if (value & 01)
			fprintf(stderr, "\t%dth item selected\n", i);
		
		/* gxv_start_connections DO NOT EDIT THIS SECTION */

		/* gxv_end_connections */

		value >>= 1;
	}
}

/*
 * Notify callback function for `min2'.
 */
Panel_setting
interval2_proc(item, event)
	Panel_item	item;
	Event		*event;
{
	histo_histo_ctrl_objects *ip = (histo_histo_ctrl_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	int	value = (int) xv_get(item, PANEL_VALUE);
	
	fprintf(stderr, "histo: interval2_proc: value: %d\n", value);
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */

	/* gxv_end_connections */

	return panel_text_notify(item, event);
}

/*
 * Notify callback function for `min3'.
 */
Panel_setting
interval3_proc(item, event)
	Panel_item	item;
	Event		*event;
{
	histo_histo_ctrl_objects *ip = (histo_histo_ctrl_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	int	value = (int) xv_get(item, PANEL_VALUE);
	
	fprintf(stderr, "histo: interval3_proc: value: %d\n", value);
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */

	/* gxv_end_connections */

	return panel_text_notify(item, event);
}

/*
 * Notify callback function for `min4'.
 */
Panel_setting
interval4_proc(item, event)
	Panel_item	item;
	Event		*event;
{
	histo_histo_ctrl_objects *ip = (histo_histo_ctrl_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	int	value = (int) xv_get(item, PANEL_VALUE);
	
	fprintf(stderr, "histo: interval4_proc: value: %d\n", value);
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */

	/* gxv_end_connections */

	return panel_text_notify(item, event);
}

/*
 * Notify callback function for `min5'.
 */
Panel_setting
interval5_proc(item, event)
	Panel_item	item;
	Event		*event;
{
	histo_histo_ctrl_objects *ip = (histo_histo_ctrl_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	int	value = (int) xv_get(item, PANEL_VALUE);
	
	fprintf(stderr, "histo: interval5_proc: value: %d\n", value);
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */

	/* gxv_end_connections */

	return panel_text_notify(item, event);
}

/*
 * Notify callback function for `min6'.
 */
Panel_setting
interval6_proc(item, event)
	Panel_item	item;
	Event		*event;
{
	histo_histo_ctrl_objects *ip = (histo_histo_ctrl_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	int	value = (int) xv_get(item, PANEL_VALUE);
	
	fprintf(stderr, "histo: interval6_proc: value: %d\n", value);
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */

	/* gxv_end_connections */

	return panel_text_notify(item, event);
}

/*
 * Notify callback function for `min7'.
 */
Panel_setting
interval7_proc(item, event)
	Panel_item	item;
	Event		*event;
{
	histo_histo_ctrl_objects *ip = (histo_histo_ctrl_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	int	value = (int) xv_get(item, PANEL_VALUE);
	
	fprintf(stderr, "histo: interval7_proc: value: %d\n", value);
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */

	/* gxv_end_connections */

	return panel_text_notify(item, event);
}

/*
 * Notify callback function for `min8'.
 */
Panel_setting
interval8_proc(item, event)
	Panel_item	item;
	Event		*event;
{
	histo_histo_ctrl_objects *ip = (histo_histo_ctrl_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	int	value = (int) xv_get(item, PANEL_VALUE);
	
	fprintf(stderr, "histo: interval8_proc: value: %d\n", value);
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */

	/* gxv_end_connections */

	return panel_text_notify(item, event);
}

/*
 * Repaint callback function for `palcanv'.
 */
void
palcanv_repaint_proc(canvas, paint_window, display, xid, rects)
	Canvas		canvas;
	Xv_window	paint_window;
	Display		*display;
	Window		xid;
	Xv_xrectlist	*rects;
{
	fputs("histo: palcanv_repaint_proc\n", stderr);
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */

	/* gxv_end_connections */

}

/*
 * Notify callback function for `color_histo'.
 */
void
histo_color_opts(item, event)
	Panel_item	item;
	Event		*event;
{
	histo_display_objects *ip = (histo_display_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	
	fputs("histo: histo_color_opts\n", stderr);
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */

	/* gxv_end_connections */

}

/*
 * Notify callback function for `refresh'.
 */
void
histo_refresh_proc(item, event)
	Panel_item	item;
	Event		*event;
{
	histo_display_objects *ip = (histo_display_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	
	fputs("histo: histo_refresh_proc\n", stderr);
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */

	/* gxv_end_connections */

}

/*
 * Notify callback function for `magnify'.
 */
void
mag_proc(item, event)
	Panel_item	item;
	Event		*event;
{
	histo_display_objects *ip = (histo_display_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	
	fputs("histo: mag_proc\n", stderr);
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */

	/* gxv_end_connections */

}

/*
 * Notify callback function for `unmagnify'.
 */
void
unmag_proc(item, event)
	Panel_item	item;
	Event		*event;
{
	histo_display_objects *ip = (histo_display_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	
	fputs("histo: unmag_proc\n", stderr);
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */

	/* gxv_end_connections */

}

/*
 * Notify callback function for `restore'.
 */
void
restore_proc(item, event)
	Panel_item	item;
	Event		*event;
{
	histo_display_objects *ip = (histo_display_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	
	fputs("histo: restore_proc\n", stderr);
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */

	/* gxv_end_connections */

}

/*
 * Event callback function for `histocanv'.
 */
Notify_value
histo_event_proc(win, event, arg, type)
	Xv_window	win;
	Event		*event;
	Notify_arg	arg;
	Notify_event_type type;
{
	histo_display_objects *ip = (histo_display_objects *) xv_get(xv_get(win, CANVAS_PAINT_CANVAS_WINDOW), XV_KEY_DATA, INSTANCE);
	
	fprintf(stderr, "histo: histo_event_proc: event %d\n", event_id(event));
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */

	/* gxv_end_connections */

	return notify_next_event_func(win, (Notify_event) event, arg, type);
}

/*
 * Repaint callback function for `histocanv'.
 */
void
histo_repaint_proc(canvas, paint_window, display, xid, rects)
	Canvas		canvas;
	Xv_window	paint_window;
	Display		*display;
	Window		xid;
	Xv_xrectlist	*rects;
{
	fputs("histo: histo_repaint_proc\n", stderr);
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */

	/* gxv_end_connections */

}
