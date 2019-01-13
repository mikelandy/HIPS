
/*
 *	@(#)xv_header.c	2.4	11/29/91
 *
 * XView preview for HIPS images V1.0
 *
 * Author: 	Cathy Waite
 * 		(c) The Turing Institute
 *
 * Fri Aug  9 17:06:10 BST 1991
 *
 *  xv_header.c : Contains defintions for command popups for displaying header
 *  		  files
 */
#include <xview/panel.h>
#include <xview/cms.h>

#include "xv_frame.h"
#include "xv_header.h"

/************************************************************************
 *
 *		Static XView Items .....
 *
 ************************************************************************/
Frame		header_command_frame;
Panel_item	header_name;
Textsw		header_text;
Window		header_xwin;

/***********************************************************************
 *
 *		NOTIFICATION PROCEDURES
 *
 ***********************************************************************/

void	hide_header_notify()
{
	xv_set(header_command_frame, FRAME_CMD_PUSHPIN_IN, FALSE, NULL);
}


/************************************************************************
 *
 *		Creation of Frame, Control Panel and Panel_items
 *
 ***********************************************************************/


/*
 * Create command frame popup for displaying header information
 *
 */
void	create_header_command_frame()
{
	Panel		panel;
	Panel_item	hide_button;
	int		header_panel_width, button_width;
	Cms		cms;

	cms = (Cms)xv_create(NULL, CMS,
		CMS_NAME,	"header",
		CMS_CONTROL_CMS, TRUE,
		CMS_SIZE,	CMS_CONTROL_COLORS,
		CMS_NAMED_COLORS, NULL,
		NULL);

	header_command_frame = xv_create(frame, FRAME_CMD,
		FRAME_LABEL,	"Image Header",
		NULL);


	header_xwin = (XID)xv_get(header_command_frame, XV_XID);
	panel = (Panel)xv_get(header_command_frame,
		FRAME_CMD_PANEL);


	header_name = (Panel_item) xv_create(panel, PANEL_MESSAGE,
		PANEL_LABEL_STRING,	filename,
		PANEL_LABEL_BOLD,	TRUE,
		NULL);

	hide_button = (Panel_item) xv_create(panel, PANEL_BUTTON,
		PANEL_LABEL_STRING,	"Hide",
		PANEL_NOTIFY_PROC,	hide_header_notify,
		NULL);

	window_fit_height(panel);

	header_text = (Textsw) xv_create(header_command_frame, TEXTSW,
		WIN_X,			0,
		WIN_ROWS,		20,
		WIN_COLUMNS,		60,
		TEXTSW_BROWSING,	TRUE,
		TEXTSW_LINE_BREAK_ACTION,	TEXTSW_WRAP_AT_CHAR,
		TEXTSW_DISABLE_CD,	TRUE,
		TEXTSW_DISABLE_LOAD,	TRUE,
		WIN_BELOW,		panel,
		WIN_CMS,		cms,
		WIN_BACKGROUND_COLOR,	CMS_CONTROL_COLORS-4,
		WIN_FOREGROUND_COLOR,	CMS_CONTROL_COLORS-2,
		NULL);

	button_width = (int)xv_get(hide_button, XV_WIDTH);

	header_panel_width = (int)xv_get(header_text, XV_WIDTH);
	xv_set(panel, XV_WIDTH, header_panel_width, NULL);
	xv_set(hide_button, XV_X, header_panel_width - button_width - 5, NULL);
	
	window_fit(header_command_frame);
}

void	update_hdr(image_header, filename)
struct header	image_header;
char	*filename;
{
	char	*formatheader();
	char	*hstring;

	textsw_reset(header_text, 0,0);
	hstring = formatheader(&image_header);
	
	textsw_replace_bytes(header_text,0,TEXTSW_INFINITY,hstring,
		strlen(hstring));
	textsw_normalize_view(header_text,0);
	xv_set(header_name, PANEL_LABEL_STRING,filename,NULL);
}

