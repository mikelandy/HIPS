/*
 * include file for any source files that interact with the UI
 */

#include <sys/param.h>
#include <sys/types.h>
#include <xview/xview.h>
#include <xview/canvas.h>
#include <xview/panel.h>
#include <xview/frame.h>
#include <xview/textsw.h>
#include <xview/xv_xrect.h>
#include <xview/cms.h>
#include <xview/notice.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include "main_control_ui.h"
#include "file_ui.h"
#include "comment_ui.h"
#include "display_ui.h"

extern main_control_ctrlwin_objects *base_win;
#ifdef V1
extern main_control_imgwin_objects *genial_imgwin;
#endif
extern file_window1_objects *file_win;
extern display_ctrlwin_objects *disp_ctrl;
extern comment_comm_win_objects *comment_win;


/* display, winv are and gc are used globally in calls to X for any purpose */
extern Display *display;
extern Visual *winv;
extern GC gc;
extern int depth;

/* line_mode is an int represneting the present line drawing mode */
#define CLICK 0
#define DRAG 1
#define LSQ 2

/* function opcodes */
#define LINE_TRACE 0
#define HISTOGRAM 1
#define ZOOM 2
#define DISTANCE 3
#define ANGLE_MES 4
#define ANNOTATE 5
#define COMMENT 6

#define SCROLL_BAR_SIZE 21    /* width of scrollbar */

extern char *func_names[];
extern int line_mode, clean_mode;
