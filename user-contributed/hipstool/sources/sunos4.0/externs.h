/* externs.h
 * Max Rible
 *
 * Extern declarations for anything that matters in HIPStool.
 * This file is included in "hipstool.h".
 */

/* hipstool.c */
extern int winx, winy;			/* Frame coordinates */
extern int auxiliary;			/* Which frame to use? */
extern int resize;			/* Resize on load */
extern char *infilename, *outfilename;	/* Pass arguments by globals, sure */

/* panel.c */
extern void quit_proc(), toggle_proc(), parse_proc(), cycle_proc(), 
    max_proc(), input_proc();
#ifdef MULTO_FRAMES
extern void page_proc();
#endif

/* list.c */
extern void add_point();
extern int move_point(), depth();

/* data.c */
extern void destroy();
extern caddr_t copy();

/* functions.c */
Command *cur_func;

extern double theta;

extern unsigned char red[256], green[256], blue[256], tr[256];
extern char cursortext[CURS_LEN], windowtext[INFO_LEN], infotext[2][100];
extern Bool complete;
extern int resolution, current_action_num, auto_log;
extern enum line_select_mode line_drawing_mode;

extern FileInfo base, proj;		/* Base and auxiliary */

extern Function functions[MAX_FUNCTIONS];

/* setups.c */
extern void no_setup(), box_setup(), edit_setup(), angle_setup(), 
    distance_setup();

/* selects.c */
extern void select_search(), line_select(), box_select(), list_select(),
    point_select(), text_select();

/* modifys.c */
extern void modify_search();
extern int line_modify(), box_modify(), list_modify(), point_modify(), 
    text_modify();

/* evals.c */
extern void box_eval(), angle_eval(), distance_eval();

/* edit.c */
extern void log_edit(), colormap_edit(), comment_edit();

/* log_funcs.c */
extern Log actions;
extern int action_number;
extern char *header_comment;
extern void log_current_action(), display_logged_actions();
extern enum truth warp_action();

/* log_load.c */
extern void load_log();

/* log_save.c */
extern int save_log();

/* log_comment.c */
char *no_comment(), *straight_comment(), *angle_comment(), 
    *distance_comment();

/* vector.c */
extern unsigned (*readarr[9])(); /* Read routines, index with data size */
extern void (*draw[3])();	/* Line drawing routines */
extern void doline2d(), fit_line(), undoline2d();

/* spline.c */
extern void do_spline();

/* metaprimits.c */
extern Primit *palloc();
extern void pfreee(), primit_display_line(), primit_display_box(), 
    primit_display_open_spline(), primit_display_closed_spline(), 
    primit_display_polygon(), primit_display_point(), primit_display_text(), 
    do_cross(), change_line_drawing_mode();

extern struct primit_info priminfo[NUM_PRIMITS];

/* zoom.c */
extern void do_zoom(), kill_zoom();

/* trace.c */
extern void trace_eval(), trace_setup();

/* histo.c */
extern void histo_eval(), histo_setup();

/* loadimage.c */
extern void load_proc(), load_image(), update_load_menu();
extern struct menu_entry load_menu_funcs[NUM_LOAD_FUNCS];

/* saveimage.c */
extern void save_proc(), save_image(), update_save_menu();
extern struct menu_entry save_menu_funcs[NUM_SAVE_FUNCS];

/* xheader.c */
extern void xheader_to_log(), log_to_xheader(), xheader_to_comment(),
    comment_to_xheader();

#ifdef SUNTOOLS
/* Suntools files:  primitives et al.
 */

/* sun_events.c */
extern void base_event_proc(), child_event_proc();

/* sun_primits.c */
#ifndef MACROIZED
void colormap256(), put_pix(), line(), text(), wipe(), putstring();
int get_pix();
char *getstring();
#endif				/* !MACROIZED */
void refresh(), batching(), newindow(), changewindow(), killwindow(), 
    init_cursor(), pixrefresh();

/* sun_funcs.c */
extern struct interface io;

extern void window_environment_init(), scrollset(), update_frame_label();
extern Pixrect *chars_to_pixrect(), *shorts_to_pixrect(), *longs_to_pixrect();

/* sun_menus.c */
extern struct function_menu_returned_information menu_info;
extern void create_menus();
#endif				/* SUNTOOLS */

#ifdef X_WINDOWS
/* X primitives et al. go here.
 */

/* X_primits.c */
extern void newindow();

/* X_funcs.c */
extern void window_environment_init();
#endif				/* X_WINDOWS */
