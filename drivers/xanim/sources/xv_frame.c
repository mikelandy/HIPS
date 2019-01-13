/*
 *	@(#)xv_frame.c	2.11	11/29/91
 *
 * XView preview for HIPS-2 images V1.0
 *
 * Author: 	Cathy Waite
 * 		(c) The Turing Institute
 *
 * Fri Aug  9 17:06:10 BST 1991
 *
 * xv_frame.h : contains definition of and routines to create and handle base 
 * 				frame.
 */
#include "xv_frame.h"
#include <xview/icon.h>
#include <xview/notice.h>
#include <xview/xv_xrect.h>

#include "xv_file.h"
#include "xv_animation.h"
#include "xv_header.h"
#include "xv_adjust.h"
#include "xv_canvas.h"

#include "appl.h"
#include "colors.h"

/* Format flags for hips parseargs routine ... */
static Flag_Format	flag_fmt[] = { 

	{"64", {"256", LASTFLAG}, 0,
	{{PTBOOLEAN, "FALSE"}, LASTPARAMETER}},

	{"256", {"64", LASTFLAG}, 0,
	{{PTBOOLEAN, "TRUE"}, LASTPARAMETER}},

	{"r", {"i", "s", LASTFLAG}, 0,
	{{PTBOOLEAN, "TRUE"}, LASTPARAMETER}},

	{"s", {"i", "r", LASTFLAG}, 0,
	{{PTBOOLEAN, "FALSE"}, LASTPARAMETER}},

	{"i", {"s", "r", LASTFLAG}, 0,
	{{PTBOOLEAN, "FALSE"}, LASTPARAMETER}},

	{"a", {LASTFLAG}, 0,
	{{PTBOOLEAN, "FALSE"}, LASTPARAMETER}},

	{"c", {"w", LASTFLAG}, 0,
	{{PTBOOLEAN, "TRUE"}, LASTPARAMETER}},

	{"w", {"c", LASTFLAG}, 0,
	{{PTBOOLEAN, "FALSE"}, LASTPARAMETER}},

	{"p", {LASTFLAG}, 0,
	{{PTBOOLEAN, "FALSE"}, LASTPARAMETER}},

	{"f", {LASTFLAG}, 1, {{PTBOOLEAN, "FALSE"},
		{PTINT, "0", "from-frame"},
		{PTINT, "-999", "to-frame"},
		LASTPARAMETER}},

	{"d", {LASTFLAG}, 2, {{PTBOOLEAN, "FALSE"},
		{PTINT, "0", "width"},
		{PTINT, "0", "height"},
		LASTPARAMETER}},

	LASTFLAG };

/* The name of the file currently being displayed */
char		filename[TEXT_LENGTH];

char		info[TEXT_LENGTH];

h_boolean	valid_image = FALSE;
h_boolean	changes = FALSE;
/* RF: 9 */
h_boolean	with_control_panel = TRUE;

/************************************************************************
 *
 *		X11 Objects ...
 *
 ************************************************************************/
 int		screen;
 Visual		*visual;
 Display	*display;
 Window		frame_xwin, panel_xwin;
 GC		image_gc;

/************************************************************************
 *
 *		Static XView Items .....
 *
 ************************************************************************/
Frame		frame;
Panel_item	filename_item, view_button, edit_button, info_item;
Panel		control_panel;
Menu		file_menu, view_menu,edit_menu,panel_menu;
Menu_item	save_as_menu_item, 
		save_frame_item, 
		save_menu_item, 
		animation_menu_item, 
		control_menu_item,
		revert_menu_item;

int		frame_panel_height, panel_min_height;
int		frame_panel_width, panel_min_width;


/* To stop surious resizies of image when tool initialised */
int		ignore_resize_count = 3;

/* Used to avoid resizing window to the same size ...
   It seems two events are generated each time the window is 
   resized !?!
*/
int		window_height, window_width;


/************************************************************************
 *
 * 		Create the icon for the "closed" window
 *
 ************************************************************************/
short	icon_bits[] = {
#include "xview_hips.icon"
};

Icon	create_frame_icon()
{
	Server_image	icon_image;
	Icon		default_icon;

	icon_image = (Server_image)xv_create(NULL, SERVER_IMAGE,
		XV_WIDTH,		64,
		XV_HEIGHT,		64,
		SERVER_IMAGE_BITS,	icon_bits,
		NULL);

	default_icon =  (Icon)xv_create(frame, ICON,
			ICON_IMAGE,	icon_image,
			XV_X,		100,
			XV_Y,		100,
			NULL);

	xv_set(frame,FRAME_ICON, default_icon,NULL);
}

void	load_image_notify()
{
	int result;
	if (changes == TRUE)
	{
		result = notice_prompt(frame, NULL,
			NOTICE_MESSAGE_STRINGS, "The changes to the current\
image have not been saved.\n",
			"Are you sure you wish to load a new image?",
			NULL, 
			NOTICE_BUTTON_YES,	"Yes",
			NOTICE_BUTTON_NO,	"No",
			NULL);

		if (result == NOTICE_NO)
			return;
	}

	/* BF 5: Cannot load a file from stdin or stdout */
	if ((strcmp(filename,"stdin") == 0) || 
		(strcmp(filename,"stdout") == 0))
		xv_set(load_file_item, PANEL_VALUE, "", NULL);
	else
		xv_set(load_file_item, PANEL_VALUE, filename, NULL);
	xv_set(load_command_frame, XV_SHOW, TRUE, NULL);
}


void	save_image_notify()
{
	if ( (filename[0] == '\0') ||
		(strcmp(filename,"stdin") == 0) ||
		(strcmp(filename, "stdout") == 0))
	{
		if (saved_stdout == TRUE)
			/* BF 2. Cannot save to stdout more than once ... */
			xv_set(save_file_item, PANEL_VALUE, "", NULL);
		else
			xv_set(save_file_item, PANEL_VALUE, "stdout", NULL);
		xv_set(save_command_frame, XV_SHOW, TRUE,NULL);
	}
	else
	{
		if (changes == TRUE)
			save_file(filename);
	}
}


void	save_image_as_notify()
{
	if ((strcmp(filename, "stdin") == 0) ||
		(strcmp(filename,"stdout") == 0))
		if (saved_stdout == TRUE)
			/* BF 2. Cannot save to stdout more than once ... */
			xv_set(save_file_item, PANEL_VALUE, "", NULL);
		else
			xv_set(save_file_item, PANEL_VALUE, "stdout", NULL);
	else
		xv_set(save_file_item, PANEL_VALUE, filename, NULL);
	xv_set(save_command_frame, XV_SHOW, TRUE,NULL);
	save_frame = FALSE;
}

void	save_frame_notify()
{
	/* Check to see if currently animated ... if so do not save frame
	   as current frame will change 
	*/
	if (running == TRUE)
	{
		
		(void)  notice_prompt(frame, NULL,
		NOTICE_MESSAGE_STRINGS, "A single frame cannot be saved \
whilst animating a sequnce",
		"as the current frame changes.  Please stop the animation \
at the frame you wish saved", 
		"and select `Save Frame' again.",
		NULL,
		NOTICE_BUTTON, 			"Continue", 0,
		NULL);
		return;
	}

	if ((strcmp(filename, "stdin") == 0) ||
		(strcmp(filename,"stdout") == 0))
		if (saved_stdout == TRUE)
			/* BF 2. Cannot save to stdout more than once ... */
			xv_set(save_file_item, PANEL_VALUE, "", NULL);
		else
			xv_set(save_file_item, PANEL_VALUE, "stdout", NULL);
	else
		xv_set(save_file_item, PANEL_VALUE, filename, NULL);
	xv_set(save_command_frame, XV_SHOW, TRUE,NULL);
	save_frame = TRUE;
}



void	revert_notify()
{
	int result;

	result = notice_prompt(frame, NULL,
		NOTICE_MESSAGE_STRINGS, "Reverting will lose all changes \
made to the image. ",
		"Are you sure you wish to Revert?",
		NULL,
		NOTICE_BUTTON_YES,	"Yes",
		NOTICE_BUTTON_NO,	"No",
		NULL);

	if (result == NOTICE_YES)
		revert_image();
}

void	enable_revert()
{
	if (changes == FALSE)
	{
		xv_set(revert_menu_item, MENU_INACTIVE, FALSE, NULL);
		xv_set(save_menu_item, MENU_INACTIVE, FALSE, NULL);
		changes = TRUE;
	}
}

void	disable_revert()
{
	if (changes == TRUE)
	{
		xv_set(revert_menu_item, MENU_INACTIVE, TRUE, NULL);
		xv_set(save_menu_item, MENU_INACTIVE, TRUE, NULL);
		changes = FALSE;
	}
}


void	view_header_notify()
{
	xv_set(header_command_frame, XV_SHOW, TRUE, NULL);
}

void	animation_notify()
{
	xv_set(animation_command_frame, FRAME_CMD_PUSHPIN_IN, TRUE, NULL);
	xv_set(animation_command_frame, XV_SHOW, TRUE, NULL);
}

void	hide_control_panel()
{
	xv_set(control_panel, XV_SHOW, FALSE, NULL);
	with_control_panel = FALSE;
	xv_set(canvas,
		XV_X,	0,
		XV_Y,	0,
		NULL);
	
	frame_panel_width = frame_panel_height = 0;
	window_fit(frame);
	window_height = (int)xv_get(frame, XV_HEIGHT);
	window_width = (int)xv_get(frame, XV_WIDTH);
}

void	show_control_panel()
{
	xv_set(control_panel, XV_SHOW, TRUE, NULL);
	with_control_panel = TRUE;
	xv_set(canvas,
		XV_X,		0,
		XV_Y,		panel_min_height,
		NULL);
	frame_panel_width = panel_min_width;
	frame_panel_height = panel_min_height;
	update_frame(
		(int)xv_get(canvas, CANVAS_WIDTH),
		(int)xv_get(canvas, CANVAS_HEIGHT),
		TRUE
		);
	xv_set(canvas, XV_SHOW, TRUE, NULL);
}

void	adjust_notify()
{
	xv_set(adjust_command_frame, FRAME_CMD_PUSHPIN_IN, TRUE, NULL);
	xv_set(adjust_command_frame, XV_SHOW, TRUE, NULL);
}

void	extract_notify()
{
	interactive_extract();
}

void	report_error(info)
char		*info;
{
	(void)  notice_prompt(frame, NULL,
		NOTICE_MESSAGE_STRINGS, 	info, NULL,
		NOTICE_BUTTON, 			"Continue", 0,
		NULL);
}

void	print_message(message)
char	*message;
{
	xv_set(info_item, PANEL_LABEL_STRING, message, NULL);
}


#define	CANVAS_FRAME_OFFSET	2
void	update_frame(width, height, size_window) 
int	width, height;
h_boolean	size_window;
{

	print_message("");
	xv_set(canvas,  CANVAS_WIDTH, width,
			CANVAS_HEIGHT, height,
			XV_WIDTH, width+CANVAS_FRAME_OFFSET,
			XV_HEIGHT, height+CANVAS_FRAME_OFFSET,
			NULL);
	
	/* RF: 9 */
	if (with_control_panel == TRUE)
	{
		xv_set(control_panel,XV_WIDTH, 
			MAX(frame_panel_width, width+CANVAS_FRAME_OFFSET), 
			NULL);

		/* If image is not as wide as panel (and thus window)
		 * make the panel long enough so as to reach the bottom 
		 * of the window, and thus fill the gap to the right of it
		 */

		if (frame_panel_width > width+CANVAS_FRAME_OFFSET)
			xv_set(control_panel,
				XV_HEIGHT, frame_panel_height+height+CANVAS_FRAME_OFFSET,
				NULL);
		else
			xv_set(control_panel,XV_HEIGHT, 
				frame_panel_height,NULL);

		set_width_height(width, height);
	}
		
	if ((size_window == TRUE) || (frame_panel_width > width+CANVAS_FRAME_OFFSET))
				window_fit(frame);
	window_height = (int)xv_get(frame, XV_HEIGHT);
	window_width = (int)xv_get(frame, XV_WIDTH);
}

void	resize_internals(new_width, new_height)
int	new_width, new_height;
{
	int	canvas_width, canvas_height;

	canvas_width = MAX(1,new_width - CANVAS_FRAME_OFFSET);
	canvas_height = MAX(1,new_height  - CANVAS_FRAME_OFFSET - frame_panel_height);

	/* Will this ever be true, or will previus test catch it ?  */
	/* BF: 4 ... So window is resized properly if canvas is resized to
	 * the same size twice, when smaller than control panel.
	if ((canvas_width != image_width) || (canvas_height !=  image_height))
	*/
	update_frame(canvas_width, canvas_height, FALSE);
}

Notify_value frame_event_interposer(tframe, event, arg, type)
Frame			tframe;
Event			*event;
Notify_arg		arg;
Notify_event_type	type;
{
	h_boolean	old_state,new_state;
	int	new_width, new_height;
	Notify_value	value;

	if (event_action(event) == WIN_RESIZE)
		if (ignore_resize_count > 0)
			ignore_resize_count--;
		else
		{
			new_width = (int)xv_get(frame, XV_WIDTH);
			new_height = (int)xv_get(frame, XV_HEIGHT);

			if ((new_width == window_width) && 
			    (new_height == window_height))
				return(value);
			resize_internals(new_width, new_height);
		}
	else
	{
		old_state = (h_boolean)window_get(frame, FRAME_CLOSED);
		notify_next_event_func(tframe, event,arg, type);
		new_state = (h_boolean)window_get(frame, FRAME_CLOSED);
		if (new_state  != old_state)
			if (new_state == TRUE)
				/* Just been iconified */
				interrupt_timer();
			else
				/* Just been opened */
				restart_timer();
	}

	return(value);
}

Notify_value reset_all(client,status)
Frame		client;
Destroy_status	status;
{
	if (status == DESTROY_CHECKING)
	{
		int	result;

		if (changes == TRUE)
		{
			/*
			XSetWindowColormap(display, frame_xwin, DefaultColormap(display, screen));
			*/
			result = notice_prompt(frame, NULL,
			NOTICE_MESSAGE_STRINGS, "There are unsaved changes.", 
				"Are you sure you wish to Quit?",
			NULL,
			NOTICE_BUTTON_YES,	"Yes",
			NOTICE_BUTTON_NO,	"No",
			NULL);
			
			if (result == NOTICE_NO)
			{
				/* User decided not to quit after all...*/
				/*
				xv_set(frame, WIN_CMS, cms, NULL);
				*/
				notify_veto_destroy(client);	
			}
		}
	}
	else if (status == DESTROY_CLEANUP)
		return notify_next_destroy_func(client, status);
	
	return(NOTIFY_DONE);
}

void	quit_notify()
{
	xv_destroy_safe(frame);
}

void	panel_notify(panel, event)
Panel	panel;
Event	*event;
{
	if ((event_id(event) == MS_RIGHT) && event_is_down(event))
	{
		menu_show(panel_menu,panel, event, NULL);
	}
	else if ((event_id(event) == MS_MIDDLE) && event_is_down(event))
		cancel_extract();
}


/************************************************************************
 *
 *		Creation of Frame, Control Panel and Panel_items
 *
 ***********************************************************************/


/*
 * Routine to create the control panel, and its controls 
 */
void	create_control_panel()
{

	control_panel = (Panel)xv_create(frame, PANEL,
		PANEL_BACKGROUND_PROC,	panel_notify,
		WIN_ROW_GAP,		0,
		NULL);

	filename_item = (Panel_item)xv_create(control_panel, PANEL_MESSAGE,
		PANEL_LABEL_BOLD,	TRUE,
		PANEL_LABEL_STRING,	filename,
		XV_Y,			xv_row(control_panel,0),
		0);
	
	file_menu = (Menu)xv_create(NULL, MENU,
		MENU_GEN_PIN_WINDOW,	frame, "File",
		MENU_ITEM,
			MENU_STRING,		"Load ...",
			MENU_NOTIFY_PROC,	load_image_notify,
			NULL,
		NULL);
	
	save_menu_item = (Menu_item)xv_create(NULL, MENUITEM,
			MENU_STRING,		"Save",
			MENU_NOTIFY_PROC,	save_image_notify,
			MENU_INACTIVE,		TRUE,
			NULL,
		NULL);

	save_as_menu_item = (Menu_item)xv_create(NULL, MENUITEM,
			MENU_STRING,		"Save as ...",
			MENU_NOTIFY_PROC,	save_image_as_notify,
			MENU_INACTIVE,		TRUE,
			NULL);

	save_frame_item = (Menu_item)xv_create(NULL, MENUITEM,
			MENU_STRING,		"Save Frame ...",
			MENU_NOTIFY_PROC,	save_frame_notify,
			MENU_INACTIVE,		TRUE,
			NULL);

	revert_menu_item = (Menu_item)xv_create(NULL, MENUITEM,
			MENU_STRING,		"Revert",
			MENU_NOTIFY_PROC,	revert_notify,
			MENU_INACTIVE,		TRUE,
			NULL);

	xv_set(file_menu,
		MENU_APPEND_ITEM, save_menu_item, 
		MENU_APPEND_ITEM, save_as_menu_item,
		MENU_APPEND_ITEM, save_frame_item,
		MENU_APPEND_ITEM, revert_menu_item,
		NULL);

	(void)	xv_create(control_panel, PANEL_BUTTON,
		PANEL_LABEL_STRING,	"File",
		PANEL_ITEM_MENU,	file_menu,
		XV_Y,			xv_row(control_panel,1),
		XV_X,			0,
		NULL);
	
	view_menu = (Menu) xv_create(NULL, MENU,
		MENU_GEN_PIN_WINDOW,	frame, "View",
		MENU_ITEM,
			MENU_STRING,		"View Header...",
			MENU_NOTIFY_PROC,	view_header_notify,
			NULL,
		NULL);

	animation_menu_item = (Menu_item)xv_create(NULL, MENUITEM,
			MENU_STRING,		"Animate...",
			MENU_NOTIFY_PROC,	animation_notify,
			NULL);

	xv_set(view_menu, 
		MENU_APPEND_ITEM,	animation_menu_item,
		NULL);

	control_menu_item = (Menu_item)xv_create(NULL, MENUITEM,
			MENU_STRING,		"Hide Control Panel",
			MENU_NOTIFY_PROC,	hide_control_panel,
			NULL);

	xv_set(view_menu, 
		MENU_APPEND_ITEM,	control_menu_item,
		NULL);

	view_button = (Panel_item)xv_create(control_panel, PANEL_BUTTON,
		PANEL_LABEL_STRING,	"View",
		PANEL_INACTIVE,		TRUE,
		PANEL_ITEM_MENU,	view_menu,
		NULL);

	edit_menu = (Menu) xv_create(NULL, MENU,
		MENU_GEN_PIN_WINDOW,	frame, "Edit",
		MENU_ITEM,		
			MENU_STRING,		"Adjust...",
			MENU_NOTIFY_PROC,	adjust_notify,
			NULL,
		MENU_ITEM,
			MENU_STRING,		"Extract",
			MENU_NOTIFY_PROC,	extract_notify,
			NULL,
		NULL);

	edit_button = (Panel_item)xv_create(control_panel, PANEL_BUTTON,
		PANEL_LABEL_STRING,	"Edit",
		PANEL_INACTIVE,		TRUE,
		PANEL_ITEM_MENU,	edit_menu,
		NULL);

	(void)  xv_create(control_panel, PANEL_BUTTON,
		PANEL_LABEL_STRING,	"Quit",
		PANEL_NOTIFY_PROC,	quit_notify,
		NULL);

	panel_menu = (Menu)xv_create(NULL, MENU,
		MENU_TITLE_ITEM,	"xanim",
		MENU_ITEM,
			MENU_STRING,	"File",
			MENU_PULLRIGHT,		file_menu,
			NULL,
		MENU_ITEM,
			MENU_STRING,	"View",
			MENU_PULLRIGHT, 	view_menu,
			NULL,
		MENU_ITEM,
			MENU_STRING,	"Edit",
			MENU_PULLRIGHT, 	edit_menu,
			NULL,
		MENU_ITEM,
			MENU_STRING,	"Quit",
			MENU_NOTIFY_PROC, quit_notify,
			NULL,
		NULL);

	info_item = (Panel_item)xv_create(control_panel, PANEL_MESSAGE,
		PANEL_LABEL_BOLD,	TRUE,
		PANEL_LABEL_STRING,	"",
		XV_X,			xv_row(control_panel,0),
		XV_Y,			xv_row(control_panel,2)+5,
		0);
	
	window_fit(control_panel);
	panel_min_height = (int)xv_get(control_panel,XV_HEIGHT);
	panel_min_width = (int)xv_get(control_panel, XV_WIDTH);

	if (with_control_panel == TRUE)
	{
		frame_panel_height = panel_min_height;
		frame_panel_width = panel_min_width;
	}
	else
	{
		frame_panel_height = frame_panel_width = 0;
		xv_set(control_panel, XV_SHOW, FALSE, NULL);
	}

	panel_xwin = (XID)xv_get(control_panel, XV_XID);
	create_load_command_frame();
	create_save_command_frame();
	create_header_command_frame();
	create_animation_command_frame();
	create_adjust_command_frame();
}

void	match_visual()
{
	XVisualInfo	visual_template;
	XVisualInfo	*visual_list;
	int		visuals_matched;

	/* For now ... require PseudoColor ... */
	visual_template.screen = screen;
	visual_template.class = PseudoColor;
	visual_template.depth = 8;
	visual_list = XGetVisualInfo(display, VisualClassMask |
		VisualDepthMask, &visual_template, &visuals_matched);
	if (visuals_matched == 0)
	{
		/* HIPS error handling */
		perr(HE_MSG,"Error: xanim requires a display capable \
of PseudoColor, with a depth of 8.");
		exit(HIPS_ERROR);
	}
	visual = visual_list[0].visual;
	XFree((caddr_t)visual_list);
}

void	main(argc, argv)
int	argc;
char	*argv[];
{

	int		i;
	h_boolean		colors64, colors256, without_cp;
	h_boolean		start_animating;
	h_boolean		resize;
	h_boolean		interpolate;
	int		width, height;
	Filename	filenm;
	Progname = strsave(*argv);

	/*
	malloc_debug(2);
	*/

	/* Keep a copy of the input arguments for updating headers when
	   saving images.
	*/
 	global_argc = argc;
	global_argv= (char **)calloc(argc, sizeof(char *));
	for (i=0; i<argc; i++)
	{
		global_argv[i] = (char *)malloc(sizeof(char *)*strlen(argv[i]));
		strcpy(global_argv[i], argv[i]);
	}

	/* Extract XView arguments, and initialise display */
	xv_init(XV_INIT_ARGC_PTR_ARGV, &argc, argv, NULL);

	/* RF: 9 */
	/* Extract HIPS arguments */
	parseargs(argc, argv, flag_fmt, 
		&colors64, &colors256, 
		&retain_panel, &do_substitute, &ignore_panel, 
		&start_animating,
		&with_control_panel, &without_cp, 
		&interpolate,
		&select_frames, &first_frame, &last_frame,
		&resize, &width, &height,
		FFONE, &filenm);
	
	strcpy(filename, filenm);
	if (colors64 == TRUE)
	{
		no_colors = 64;
		fprintf(stderr,"WARNING: You cannot save images converted to 6 bits for 64 grey levels\n");
	}
	else
		no_colors = 256;

	if (interpolate == TRUE)
		slow_resize = TRUE;

	frame = (Frame)xv_create(NULL, FRAME,
		FRAME_LABEL,		"xanim V2.0 HIPS-2 Previewer",
		FRAME_INHERIT_COLORS,	TRUE,
		NULL);

	frame_xwin = (XID)xv_get(frame, XV_XID);
	display = (Display *)xv_get(frame, XV_DISPLAY);
	screen = DefaultScreen(display);
	image_gc = DefaultGC(display,screen);
	match_visual();

	create_control_panel();
	create_frame_icon();
	create_default_color_map();
	create_canvas();

	window_fit(frame);

	/* As this is an interactive tool, routines should never "exit"
	   when errors occur.
	*/
	hipserrprt = hipserrlev = HEL_SEVERE;

	/* BF: 5 Added extra startup parameter */
	load_file(filename, TRUE);


	if ((start_animating) && (no_frames > 1))
		run_notify();

	if (resize)
	{
		if (width < 1)
			width = image_width;
		if (height < 1)
			height = image_height;

		resize_image(width, height);
		update_frame(width, height, TRUE);
		disable_revert();

	}
	notify_interpose_event_func(frame, frame_event_interposer, NOTIFY_SAFE);
	notify_interpose_destroy_func(frame, reset_all);
	xv_main_loop(frame);
	exit(0);
}

void	enable_options(file_name)
char	*file_name;
{
	xv_set(filename_item, PANEL_LABEL_STRING, file_name, NULL);
	if (valid_image == FALSE)
	{
		valid_image = TRUE;
		if (no_colors != 64)
		{
			xv_set(save_as_menu_item, MENU_INACTIVE, FALSE, NULL);
			xv_set(save_frame_item, MENU_INACTIVE, FALSE, NULL);
		}
		if (changes == TRUE)
		{
			xv_set(revert_menu_item, MENU_INACTIVE, FALSE, NULL);
			if (no_colors == 64)
				xv_set(save_menu_item, MENU_INACTIVE, FALSE, NULL);
		}
		xv_set(view_button, PANEL_INACTIVE, FALSE, NULL);
		xv_set(edit_button, PANEL_INACTIVE, FALSE, NULL);
	}
}
	
void	disable_options(file_name)
char	*file_name;
{
	xv_set(filename_item, PANEL_LABEL_STRING, file_name, NULL); 
	if (valid_image == TRUE)
	{
		valid_image = FALSE;
		xv_set(save_as_menu_item, MENU_INACTIVE, TRUE, NULL);
		xv_set(save_frame_item, MENU_INACTIVE, TRUE, NULL);
		xv_set(save_menu_item, MENU_INACTIVE, TRUE, NULL);
		xv_set(revert_menu_item, MENU_INACTIVE, TRUE, NULL);
		xv_set(view_button, PANEL_INACTIVE, TRUE, NULL);
		xv_set(edit_button, PANEL_INACTIVE, TRUE, NULL);
	}
}
