/* sun_funcs.c
 * Max Rible
 * 
 * Support functions for suntools garbage.
 */

#include "hipstool.h"

#ifdef SUNTOOLS
struct interface io;

/* Maximum # chars == 6 */
#define Button_image(s) panel_button_image(io.panel, s, 6, io.font)

/* ARGSUSED */
void
window_environment_init(argc, argv)
     int argc;
     char *argv[];
{
    Pixwin *pw;

    io.font = pf_open(BOLD_FONT);

/* Set up the basic frame.
 */
    io.control = 
	window_create((Frame) 0, FRAME,
		      FRAME_LABEL, "HIPStool Control",
		      FRAME_EMBOLDEN_LABEL, TRUE,
		      FRAME_SHOW_LABEL, TRUE,
		      WIN_X, winx,
		      WIN_Y, winy,
		      WIN_COLUMNS, 64,
		      WIN_ROWS, 15,
		      WIN_SHOW, FALSE, /* no complaints */
		      0);

    io.panel = 
	window_create(io.control, PANEL,
		      PANEL_LABEL_BOLD, TRUE,
		      0);

    /*    ACTION_CUT, ACTION_COPY, ACTION_PASTE, */

/* Put on the same colormap as the display frame so you can see what
 * effect your actions have.
 */
    pw = (Pixwin *) window_get(io.panel, WIN_PIXWIN);
    pw_setcmsname(pw, Progname);
    pw_putcolormap(pw, 0, 256, red, green, blue);

/* Input file selection.
 */
    io.input = 
	panel_create_item(io.panel, PANEL_TEXT,
			  PANEL_NOTIFY_LEVEL, PANEL_ALL,
			  PANEL_LABEL_STRING, "Input file: ",
			  PANEL_VALUE_DISPLAY_LENGTH, 40,
			  PANEL_VALUE, infilename,
			  PANEL_ITEM_X, ATTR_COL(0),
			  PANEL_ITEM_Y, ATTR_ROW(0),
			  0);
    
    /* In the margin for input. */
    panel_create_item(io.panel, PANEL_BUTTON,
		      PANEL_LABEL_IMAGE, Button_image("load"),
		      PANEL_EVENT_PROC, load_proc,
		      PANEL_ITEM_X, ATTR_COL(54),
		      PANEL_ITEM_Y, ATTR_ROW(0),
		      0);

    /* Set up the menu for the load button. */
    io.load = 
	menu_create(MENU_STRINGS, 
		    load_menu_funcs[LOAD_HIPS_IMAGE].name,
		    load_menu_funcs[LOAD_LOG_FILE].name,
		    load_menu_funcs[LOAD_COMMENT_FILE].name,
		    0,
		    MENU_SHADOW, NULL,
		    0);

    update_load_menu();

/* Output file selection.
 */
    io.output = 
	panel_create_item(io.panel, PANEL_TEXT,
			  PANEL_NOTIFY_LEVEL, PANEL_ALL,
			  PANEL_LABEL_STRING, "Output file:",
			  PANEL_VALUE_DISPLAY_LENGTH, 40,
			  PANEL_VALUE, outfilename,
			  PANEL_ITEM_X, ATTR_COL(0),
			  PANEL_ITEM_Y, ATTR_ROW(1),
			  0);
    
    /* In the margin for output. */
    panel_create_item(io.panel, PANEL_BUTTON,
		      PANEL_LABEL_IMAGE, Button_image("save"),
		      PANEL_EVENT_PROC, save_proc,
		      PANEL_ITEM_X, ATTR_COL(54),
		      PANEL_ITEM_Y, ATTR_ROW(1),
		      0);

    /* Set up the menu for the save button. */
    io.save =
	menu_create(MENU_STRINGS, 
		    save_menu_funcs[SAVE_COMPLETE_IMAGE].name,
		    save_menu_funcs[SAVE_LOGGED_IMAGE].name,
		    save_menu_funcs[SAVE_BOX_SUBIMAGE].name,
		    save_menu_funcs[SAVE_TRACE_FILE].name,
		    save_menu_funcs[SAVE_HISTO_FILE].name,
		    save_menu_funcs[SAVE_LOG_FILE].name,
		    save_menu_funcs[SAVE_OVERLAID_IMAGE].name,
		    save_menu_funcs[SAVE_COMMENT_FILE].name,
		    0,
		    MENU_SHADOW, NULL,
		    0);

    update_save_menu();

/* A single command line.
 */
    io.command =
	panel_create_item(io.panel, PANEL_TEXT,
			  PANEL_NOTIFY_LEVEL, PANEL_ALL,
			  PANEL_LABEL_STRING, "Command:    ",
			  PANEL_VALUE_DISPLAY_LENGTH, 40,
			  PANEL_VALUE, "",
			  PANEL_ITEM_X, ATTR_COL(0),
			  PANEL_ITEM_Y, ATTR_ROW(2),
			  0);

    /* In the margin for command. */
    panel_create_item(io.panel, PANEL_BUTTON,
		      PANEL_LABEL_IMAGE, Button_image("parse"),
		      PANEL_NOTIFY_PROC, parse_proc,
		      PANEL_ITEM_X, ATTR_COL(54),
		      PANEL_ITEM_Y, ATTR_ROW(2),
		      0);

/* Comments that can be placed in the HIPS header and Superplot file.
 */
    io.comment =
	panel_create_item(io.panel, PANEL_TEXT,
			  PANEL_NOTIFY_LEVEL, PANEL_ALL,
			  PANEL_LABEL_STRING, "Comment:    ",
			  PANEL_VALUE_DISPLAY_LENGTH, 40,
			  PANEL_VALUE, "",
			  PANEL_ITEM_X, ATTR_COL(0),
			  PANEL_ITEM_Y, ATTR_ROW(3),
			  0);

    io.inputs[INPUT_LOG] = 
	panel_create_item(io.panel, PANEL_BUTTON,
			  PANEL_LABEL_IMAGE, Button_image("log"),
			  PANEL_NOTIFY_PROC, input_proc,
			  PANEL_ITEM_X, ATTR_COL(54),
			  PANEL_ITEM_Y, ATTR_ROW(3),
			  0);

/* A set of toggles for various attributes.
 */
    io.toggles =
	panel_create_item(io.panel, PANEL_TOGGLE,
			  PANEL_CHOICE_STRINGS,
/*			  "V Bar", "H Bar", "Resize", 0,*/
			  "Resize", "Auto log", 0,
			  PANEL_ITEM_X, ATTR_COL(0),
			  PANEL_ITEM_Y, ATTR_ROW(4),
/*			  PANEL_TOGGLE_VALUE, 0, FALSE,
			  PANEL_TOGGLE_VALUE, 1, FALSE,
			  PANEL_TOGGLE_VALUE, 2, resize,*/
			  PANEL_TOGGLE_VALUE, 0, resize,
			  PANEL_TOGGLE_VALUE, 1, auto_log,
			  PANEL_NOTIFY_PROC, toggle_proc,
			  0);

#ifdef MULTO_FRAMES
    io.page.back = 
	panel_create_item(io.panel, PANEL_BUTTON,
			  PANEL_LABEL_IMAGE, 
			  panel_button_image(io.panel, "<", 2, io.font),
			  PANEL_ITEM_X, ATTR_COL(28),
			  PANEL_ITEM_Y, ATTR_ROW(4),
			  PANEL_NOTIFY_PROC, page_proc,
			  0);
    io.page.forward = 
	panel_create_item(io.panel, PANEL_BUTTON,
			  PANEL_LABEL_IMAGE, 
			  panel_button_image(io.panel, ">", 2, io.font),
			  PANEL_ITEM_X, ATTR_COL(32),
			  PANEL_ITEM_Y, ATTR_ROW(4),
			  PANEL_NOTIFY_PROC, page_proc,
			  0);

    io.messages[MESSAGE_FRAME_NUMBER] = 
	panel_create_item(io.panel, PANEL_MESSAGE,
			  PANEL_LABEL_STRING, "1/1",
			  PANEL_ITEM_X, ATTR_COL(37),
			  PANEL_ITEM_Y, ATTR_ROW(4),
			  PANEL_VALUE_DISPLAY_LENGTH, 7,
			  0);
#endif		      

/* This would be a toggle, but I'm leaving provision for more
 * windows.
 */
    io.source =
	panel_create_item(io.panel, PANEL_CYCLE,
			  PANEL_LABEL_STRING, "Source: ",
			  PANEL_CHOICE_STRINGS, "Base", "Child", 0,
			  PANEL_NOTIFY_PROC, cycle_proc,
			  PANEL_ITEM_X, ATTR_COL(44),
			  PANEL_ITEM_Y, ATTR_ROW(4),
			  0);

/* The messages[] array are a bunch of places to put interesting
 * information.
 */
    io.messages[MESSAGE_CURSOR] =
	panel_create_item(io.panel, PANEL_MESSAGE,
			  PANEL_LABEL_IMAGE, cursortext,
			  PANEL_ITEM_X, ATTR_COL(0),
			  PANEL_ITEM_Y, ATTR_ROW(5),
			  0);
    
    /* In the margin for the cursor message */
    io.rescycle =
	panel_create_item(io.panel, PANEL_CYCLE,
			  PANEL_LABEL_STRING, "Grid: ",
			  PANEL_CHOICE_STRINGS, "1", "2", "3", "4",
			  "5", "6", "7", "8", "9", "10", 0,
			  PANEL_NOTIFY_PROC, cycle_proc,
			  PANEL_ITEM_X, ATTR_COL(42),
			  PANEL_ITEM_Y, ATTR_ROW(5),
			  0);

    io.messages[MESSAGE_SELECTION] = 
	panel_create_item(io.panel, PANEL_MESSAGE,
			  PANEL_LABEL_IMAGE, windowtext,
			  PANEL_ITEM_X, ATTR_COL(0),
			  PANEL_ITEM_Y, ATTR_ROW(7),
			  PANEL_VALUE_DISPLAY_LENGTH, 54,
			  0);

    io.messages[MESSAGE_INFO_1] =
	panel_create_item(io.panel, PANEL_MESSAGE,
			  PANEL_LABEL_IMAGE, infotext[0],
			  PANEL_ITEM_X, ATTR_COL(0),
			  PANEL_ITEM_Y, ATTR_ROW(8),
			  0);

    io.messages[MESSAGE_INFO_2] =
	panel_create_item(io.panel, PANEL_MESSAGE,
			  PANEL_LABEL_IMAGE, infotext[0],
			  PANEL_ITEM_X, ATTR_COL(0),
			  PANEL_ITEM_Y, ATTR_ROW(9),
			  0);

    panel_create_item(io.panel, PANEL_BUTTON,
		      PANEL_LABEL_IMAGE, Button_image("max"),
		      PANEL_NOTIFY_PROC, max_proc,
		      PANEL_ITEM_X, ATTR_COL(54),
		      PANEL_ITEM_Y, ATTR_ROW(5),
		      0);

    panel_create_item(io.panel, PANEL_BUTTON,
		      PANEL_LABEL_IMAGE, Button_image("quit"), 
		      PANEL_NOTIFY_PROC, quit_proc,
		      PANEL_ITEM_X, ATTR_COL(54),
		      PANEL_ITEM_Y, ATTR_ROW(7),
		      0);

/* The inputs array has a series of buttons and cycles all on line 5.
 */
    io.inputs[INPUT_START] = 
	panel_create_item(io.panel, PANEL_BUTTON,
			  PANEL_LABEL_IMAGE, Button_image("start"),
			  PANEL_NOTIFY_PROC, input_proc,
			  PANEL_ITEM_X, ATTR_COL(0),
			  PANEL_ITEM_Y, ATTR_ROW(6),
			  0);

    io.inputs[INPUT_EVAL] = 
	panel_create_item(io.panel, PANEL_BUTTON,
			  PANEL_LABEL_IMAGE, Button_image("eval"),
			  PANEL_NOTIFY_PROC, input_proc,
			  PANEL_ITEM_X, ATTR_COL(8),
			  PANEL_ITEM_Y, ATTR_ROW(6),
			  0);

    io.inputs[INPUT_CLEAR] = 
	panel_create_item(io.panel, PANEL_BUTTON,
			  PANEL_LABEL_IMAGE, Button_image("clear"), 
			  PANEL_NOTIFY_PROC, input_proc,
			  PANEL_ITEM_X, ATTR_COL(16),
			  PANEL_ITEM_Y, ATTR_ROW(6),
			  0);

    create_menus();

    io.inputs[INPUT_SELECT] =
	panel_create_item(io.panel, PANEL_TEXT,
			  PANEL_LABEL_STRING, "Function: ",
			  PANEL_VALUE, "Box",
			  PANEL_EVENT_PROC, input_proc,
			  PANEL_ITEM_X, ATTR_COL(25),
			  PANEL_ITEM_Y, ATTR_ROW(6),
			  0);
}

void
scrollset(winfo, vert, horiz)
     Windowinfo *winfo;
     Bool vert, horiz;
{
    int tmp;

    if((vert == 0) && (winfo->vbar != NULL)) {
	if(resize)
	    tmp = Win_width(winfo) + MARG_X;
	else 
	    tmp = (int) window_get(winfo->frame, WIN_WIDTH) - 
		(int) scrollbar_get(winfo->vbar, SCROLL_THICKNESS);
	window_set(winfo->frame,
		   WIN_WIDTH, tmp,
		   WIN_SHOW, TRUE,
		   WIN_LEFT_MARGIN, 0,
		   0);
	window_set(winfo->canvas,
		   WIN_VERTICAL_SCROLLBAR, NULL,
		   0);
	scrollbar_destroy(winfo->vbar);
	winfo->vbar = NULL;
    }

    if((horiz == 0) && (winfo->hbar != NULL)) {
	if(resize)
	    tmp = Win_height(winfo) + MARG_Y;
	else
	    tmp = (int) window_get(winfo->frame, WIN_HEIGHT) -
		(int) scrollbar_get(winfo->hbar, SCROLL_THICKNESS);
	scrollbar_destroy(winfo->hbar);
	window_set(winfo->canvas,
		   WIN_HORIZONTAL_SCROLLBAR, NULL,
		   0);
	window_set(winfo->frame,
		   WIN_HEIGHT, tmp,
		   WIN_SHOW, TRUE,
		   WIN_BOTTOM_MARGIN, 0,
		   0);
	winfo->hbar = NULL;
    }

    if((vert != 0) && (winfo->vbar == NULL)) {
	winfo->vbar = 
	    scrollbar_create(SCROLL_OBJECT, winfo->canvas,
			     SCROLL_DIRECTION, SCROLL_VERTICAL,
			     SCROLL_PLACEMENT, SCROLL_WEST,
			     0);
	window_set(winfo->canvas,
		   WIN_VERTICAL_SCROLLBAR, winfo->vbar,
		   0);
	if(resize)
	    tmp = Win_width(winfo) + MARG_X + 
		   (int) scrollbar_get(winfo->vbar, SCROLL_THICKNESS);
	else
	    tmp = (int) window_get(winfo->frame, WIN_WIDTH) +
		(int) scrollbar_get(winfo->vbar, SCROLL_THICKNESS);
	window_set(winfo->frame,
		   WIN_WIDTH, tmp,
		   WIN_LEFT_MARGIN, (int) scrollbar_get(winfo->vbar, 
							SCROLL_THICKNESS),
		   0);
    }

    if((horiz != 0) && (winfo->hbar == NULL)) {
	winfo->hbar = 
	    scrollbar_create(SCROLL_OBJECT, winfo->canvas,
			     SCROLL_DIRECTION, SCROLL_HORIZONTAL,
			     SCROLL_PLACEMENT, SCROLL_SOUTH,
			     0);
	window_set(winfo->canvas,
		   WIN_HORIZONTAL_SCROLLBAR, winfo->hbar,
		   0);
	if(resize)
	    tmp = Win_height(winfo) + MARG_Y + 
		(int) scrollbar_get(winfo->hbar, SCROLL_THICKNESS);
	else
	    tmp = (int) window_get(winfo->frame, WIN_HEIGHT) +
		(int) scrollbar_get(winfo->hbar, SCROLL_THICKNESS);
	window_set(winfo->frame,
		   WIN_HEIGHT, tmp,
		   WIN_BOTTOM_MARGIN, (int) scrollbar_get(winfo->hbar, 
							  SCROLL_THICKNESS),
		   0);
    }

    if(vert != 0 && winfo->vbar != NULL)
	scrollbar_set(winfo->vbar, SCROLL_VIEW_START, 0, 0);
    if(horiz != 0 && winfo->hbar != NULL)
	scrollbar_set(winfo->hbar, SCROLL_VIEW_START, 0, 0);
}

void
update_frame_label(which)	/* SUNTOOLS */
     FileInfo *which;
{
    Bool hbarp, vbarp;
    int cx, cy, ex, ey;
    char label[100];

    hbarp = (which->winfo.hbar != NULL);
    vbarp = (which->winfo.vbar != NULL);

    if(hbarp) {
	cx = (int) scrollbar_get(which->winfo.hbar, SCROLL_VIEW_START);
	ex = cx + (int) scrollbar_get(which->winfo.hbar, SCROLL_VIEW_LENGTH);
    } else {
	cx = 0; 
	ex = (int) window_get(which->winfo.frame, WIN_WIDTH) - MARG_X;
    }

    if(vbarp) {
	cy = (int) scrollbar_get(which->winfo.vbar, SCROLL_VIEW_START);
	ey = cy + (int) scrollbar_get(which->winfo.vbar, SCROLL_VIEW_LENGTH);
    } else {
	cy = 0; 
	ey = (int) window_get(which->winfo.frame, WIN_HEIGHT) - MARG_Y;
    }

    if(hbarp || vbarp)
	sprintf(label, 
		"HIPStool %s Image: %dx%d [%u,%u] [window (%d,%d)-(%d,%d)]",
		(which == &base ? "Base" : "Modified"),
		which->winfo.width, which->winfo.height, 
		which->extremes[0], which->extremes[1], 
		cx, cy, ex, ey);
    else
	sprintf(label,
		"HIPStool %s Image: %dx%d [%u,%u]",
		(which == &base ? "Base" : "Modified"),
		which->winfo.width, which->winfo.height,
		which->extremes[0], which->extremes[1]);

    window_set(which->winfo.frame, FRAME_LABEL, label, 0);
}

Pixrect *
chars_to_pixrect(ptr, image, width, height, user_lut)
     unsigned char *ptr;
     unsigned short *image;
     int width, height;
     unsigned char *user_lut;
{
    register int i, j, wid, index;
    register unsigned char *trptr, *charptr, *charbuf;

    charptr = ptr;
    wid = width;
    trptr = user_lut;

    if((charbuf = Calloc(width, unsigned char)) == NULL) {
	perror("line buffer allocation"); exit(-3); }

    index = (width >> 1) + (width & 1);

    for(i = 0; i < height; i++) {
	for(j = 0; j < wid; j++)
	    charbuf[j] = trptr[charptr[width*i + j]];
	Memcpy(image + i*index, charbuf, wid);
    }

    Cfree(charbuf, width, unsigned char);

    return(mem_point(width, height, 8, (short *) image));
}

Pixrect *
shorts_to_pixrect(ptr, image, width, height, user_lut)
     unsigned short *ptr;
     unsigned short *image;
     int width, height;
     unsigned char *user_lut;
{
    register int i, j, wid, index;
    register unsigned short *shortptr;
    register unsigned char *trptr, *charbuf;

    shortptr = ptr;
    wid = width;
    trptr = user_lut;

    if((charbuf = Calloc(width, unsigned char)) == NULL) {
	perror("line buffer allocation"); exit(-3); }

    index = (width >> 1) + (width & 1);

    for(i = 0; i < height; i++) {
	for(j = 0; j < wid; j++)
	    charbuf[j] = trptr[shortptr[width*i + j]];
	Memcpy(image + i*index, charbuf, wid);
    }

    Cfree(charbuf, width, unsigned char);

    return(mem_point(width, height, 8, (short *) image));
}

Pixrect *
longs_to_pixrect(ptr, image, width, height, user_lut)
     unsigned long *ptr;
     unsigned short *image;
     int width, height;
     unsigned char *user_lut;
{
    register int i, j, wid, index;
    register unsigned long *longptr;
    register unsigned char *trptr, *charbuf;

    longptr = ptr;
    wid = width;
    trptr = user_lut;

    if((charbuf = Calloc(width, unsigned char)) == NULL) {
	perror("line buffer allocation"); exit(-3); }

    index = (width >> 1) + (width & 1);

    for(i = 0; i < height; i++) {
	for(j = 0; j < wid; j++)
	    charbuf[j] = trptr[longptr[width*i + j]];
	Memcpy(image + i*index, charbuf, wid);
    }

    Cfree(charbuf, width, unsigned char);

    return(mem_point(width, height, 8, (short *) image));
}
#endif				/* SUNTOOLS */
