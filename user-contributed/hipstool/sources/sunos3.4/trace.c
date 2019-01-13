/* trace.c
 * Max Rible
 *
 * Everything for HIPStool traces I can gather together.
 */

#include "hipstool.h"

static char string[100];

static void graph(), trace_win(), trace_event_proc();

#define	TRACE_SET_LEFT	0
#define	TRACE_SET_RIGHT	1
#define	TRACE_DISPLAY	2

#define	GRAPH_VERTICAL	0
#define	GRAPH_HORIZONTAL 1

#define	BUTTON_UP	0
#define	BUTTON_LEFT	1
#define	BUTTON_RIGHT	2

typedef struct trace_information {
    unsigned *trace[3], length;
    Windowinfo winfo;
    int leftx, rightx;
    int orientation;
    int maxv;
    struct cross left, right;
    int current;
} TraceInfo;

void
trace_eval(command)
     Command *command;
{
    Primit *primit = command->func.primit;
    int dx, dy, (*loc)[2];

    if(primit->data.traces->info[0] != NULL)
	Cfree(primit->data.traces->info[0], 3*primit->data.traces->length,
	      int);
    primit->data.traces->info[0] = primit->data.traces->info[1] = 
	primit->data.traces->info[2] = NULL;
    primit->data.traces->length = 0;

    if(primit->breed == PRIMIT_LINE) {
	loc = primit->loc.coords;
	if(line_drawing_mode == LEAST_SQUARES_FIT) {
	    fit_line(primit->loc.list, &(primit->data.trace),
		     base.datasize, ON, loc);
	} else {
	    doline2d(loc[0][0], loc[0][1], loc[1][0], loc[1][1], 
		     primit->data.traces, base.datasize, ON);
	}
	
	dx = abs(loc[1][0] - loc[0][0]); dy = abs(loc[1][1] - loc[0][1]);
	
	if(dx > dy) {		/* Horizontal graph */
	    graph(&(primit->data.trace), -100, 500, GRAPH_HORIZONTAL, 
		  base.extremes[1]);
	} else {
	    graph(&(primit->data.trace), -100, 500, GRAPH_VERTICAL, 
		  base.extremes[1]);
	}
    } else {
	do_spline(primit->loc.list, DO_SPLINE_OPEN, DRAW_AND_TRACE);
	graph(primit->data.traces, -100, 500, GRAPH_HORIZONTAL, 
	      base.extremes[1]);
    }

    sprintf(string, "Trace complete, length %d.", primit->data.traces->length);
    save_menu_funcs[SAVE_TRACE_FILE].active = 1;
    update_save_menu();
    Update_info(string);
}

/* ARGSUSED */
void
trace_setup(command, value)
     Command *command;
     int value;
{
    if(value == OFF) {
	save_menu_funcs[SAVE_TRACE_FILE].active = 0;
	update_save_menu();
    }
}

/* Graph width values from 0 to 255.
 * (x,y):  upper left corner   align:  GRAPH_VERTICAL or GRAPH_HORIZONTAL
 */
static void
graph(vals, x, y, align, maxv)
     Trace *vals;
     int x, y;
     int align;
     unsigned maxv;
{
    int width;
    TraceInfo *newtraceinfo;

    width = vals->length;

    newtraceinfo = Calloc(1, TraceInfo);

    newindow(&newtraceinfo->winfo, 
	     (int)(align == GRAPH_VERTICAL ? 256 + BAR_SIZE : width + TXTWID),
	     (int)(align == GRAPH_VERTICAL ? width + TXTHT : 256 + BAR_SIZE),
	     x, y,
	     "HIPStool trace");

    newtraceinfo->trace[0] = Calloc(width, unsigned);
    newtraceinfo->trace[1] = Calloc(width, unsigned);
    newtraceinfo->trace[2] = Calloc(width, unsigned);
    Memcpy(newtraceinfo->trace[0], vals->info[0], sizeof(unsigned)*width);
    Memcpy(newtraceinfo->trace[1], vals->info[1], sizeof(unsigned)*width);
    Memcpy(newtraceinfo->trace[2], vals->info[2], sizeof(unsigned)*width);
    newtraceinfo->length = (unsigned) width;
    newtraceinfo->leftx = -1;
    newtraceinfo->rightx = -1;
    newtraceinfo->orientation = align;
    newtraceinfo->maxv = maxv;

#ifdef SUNTOOLS
    window_set(newtraceinfo->winfo.canvas,
	       WIN_CLIENT_DATA, newtraceinfo,
	       WIN_EVENT_PROC, trace_event_proc,
	       WIN_CONSUME_PICK_EVENTS,
	       WIN_NO_EVENTS, WIN_MOUSE_BUTTONS, LOC_DRAG, 0,
	       0);
#endif

    trace_win(newtraceinfo, TRACE_DISPLAY);

    Update_info("To display crosses on the line and bars on the trace:");
    Prinfo1("Use left and right mouse buttons on the trace window.");
    Prinfo2("The middle mouse button clears the marks.");
}

static void
trace_win(va_alist)
     va_dcl
{
    TraceInfo *info;
    int argument;
    va_list index;
    int i, *val, x, width;
    Cross *cross;
    char label[20];

#define WARP(x) (info->maxv ? SCALE((double)(x)/(double)info->maxv) : (int)(x))
#define XORP(x, y) put_pix(&info->winfo, x, y, 0xFF ^ \
			   get_pix(&info->winfo, x, y))

    va_start(index);
    info = va_arg(index, TraceInfo *);
    argument = va_arg(index, int);

    switch(argument) {
    case TRACE_SET_LEFT:
	val = &info->leftx; cross = &info->left;
	goto display;
    case TRACE_SET_RIGHT:
	val = &info->rightx; cross = &info->right;
    display:			/* Fall through */
	x = va_arg(index, int);
	do_cross(cross, OFF);
	if(*val >= 0)
	    if(info->orientation == GRAPH_VERTICAL) {
		line(&info->winfo,
		     BAR_SIZE, *val, 
		     BAR_SIZE + WARP(info->trace[0][*val]), *val,
		     ALMOST_BLACK);
		line(&info->winfo,
		     BAR_SIZE + WARP(info->trace[0][*val]) + 1, *val,
		     BAR_SIZE + 255, *val,
		     253);
		XORP(BAR_SIZE + 64, *val); 
		XORP(BAR_SIZE + 128, *val);
		XORP(BAR_SIZE + 192, *val);
	    } else if(info->orientation == GRAPH_HORIZONTAL) {
		line(&info->winfo,
		     *val, 255, *val, 255 - WARP(info->trace[0][*val]),
		     ALMOST_BLACK);
		line(&info->winfo,
		     *val, 255 - WARP(info->trace[0][*val]) - 1, *val, 0, 253);
		XORP(*val, 64); XORP(*val, 128); XORP(*val, 192);
	    }
	*val = x;
	cross->x = info->trace[1][x];
	cross->y = info->trace[2][x];
	if(x < 0) break;
	do_cross(cross, ON);
	if(info->orientation == GRAPH_VERTICAL)
	    line(&info->winfo, BAR_SIZE, x, BAR_SIZE + 255, x, STANDOUT);
	else if(info->orientation == GRAPH_HORIZONTAL)
	    line(&info->winfo, x, 0, x, 255, STANDOUT);
	break;
    case TRACE_DISPLAY:
	width = info->length;

	batching(&info->winfo, ON);
	
	wipe(&info->winfo, 253);
	
	if(info->orientation == GRAPH_VERTICAL) {
	    for(i = 0; i < width; i++) {
		line(&info->winfo, BAR_SIZE, i, BAR_SIZE + 
		     WARP(info->trace[0][i]), i, ALMOST_BLACK);
		line(&info->winfo, 0, i, BAR_SIZE-1, i, 
		     (int) base.user_lut[info->trace[0][i]]);
	    }
	    /* Graph labeling at 0.0, 0.25, 0.5, 0.75, 1.0. */
	    text(&info->winfo, BAR_SIZE, width + TXTHT-2, "0", 
		 ALMOST_BLACK);
	    
	    line(&info->winfo, BAR_SIZE + 64, 0, BAR_SIZE + 64, width - 1,
		 XOR_COLOR);
	    sprintf(label, "%-5d", (info->maxv ? info->maxv/4 : 64));
	    text(&info->winfo, 64 + BAR_SIZE - TXTWID, width + TXTHT - 2,
		 label, ALMOST_BLACK);
	    
	    line(&info->winfo, BAR_SIZE + 128, 0, BAR_SIZE + 128, width - 1,
		 XOR_COLOR);
	    sprintf(label, "%-5d", (info->maxv ? info->maxv/2 : 128));
	    text(&info->winfo, 128 + BAR_SIZE - TXTWID, width + TXTHT - 2,
		 label, ALMOST_BLACK);
	    
	    line(&info->winfo, BAR_SIZE + 192, 0, BAR_SIZE + 192, width - 1,
		 XOR_COLOR);
	    sprintf(label, "%-5d", (info->maxv ? (3*info->maxv)/4 : 192));
	    text(&info->winfo, 192 + BAR_SIZE - TXTWID, width + TXTHT - 2,
		 label, ALMOST_BLACK);
	    
	    sprintf(label, "%-5d", (info->maxv ? info->maxv : 255));
	    text(&info->winfo, 256 + BAR_SIZE - TXTWID, width + TXTHT - 2,
		 label, ALMOST_BLACK);
	} else {		/* info->orientation == GRAPH_HORIZONTAL */
	    for(i = 0; i < width; i++) {
		line(&info->winfo, i, 255, i, 255 - WARP(info->trace[0][i]),
		     ALMOST_BLACK);
		line(&info->winfo, i, 256, i, 255 + BAR_SIZE,
		     (int) base.user_lut[info->trace[0][i]]);
	    }
	    
	    text(&info->winfo, width+2, 255, "0", ALMOST_BLACK);
	    
	    line(&info->winfo, 0, 192, width-1, 192, XOR_COLOR);
	    sprintf(label, "%d", (info->maxv ? info->maxv/4 : 64));
	    text(&info->winfo, width+2, 192, label, ALMOST_BLACK);
	    
	    line(&info->winfo, 0, 128, width-1, 128, XOR_COLOR);
	    sprintf(label, "%d", (info->maxv ? info->maxv/2 : 128));
	    text(&info->winfo, width+2, 128, label, ALMOST_BLACK);
	    
	    line(&info->winfo, 0, 64, width-1, 64, XOR_COLOR);
	    sprintf(label, "%d", (info->maxv ? (3*info->maxv)/4 : 192));
	    text(&info->winfo, width+2, 64, label, ALMOST_BLACK);
	    
	    sprintf(label, "%d", (info->maxv ? info->maxv : 255));
	    text(&info->winfo, width+2, TXTHT, label, ALMOST_BLACK);
	}
	batching(&info->winfo, OFF);
	break;
    }
#undef WARP
#undef XORP
}

/* ARGSUSED */
static void
trace_event_proc(window, event, arg)
     Window window;
     Event *event;
     caddr_t arg;
{
    int x, y, up, idx, action;
    TraceInfo *info;
    double a[2], b[2];

    x = event_x(event); y = event_y(event); up = event_is_up(event);
    info = (TraceInfo *) window_get(window, WIN_CLIENT_DATA);
    if(info->orientation == GRAPH_VERTICAL) idx = y; else idx = x;

    if(up) {
	info->current = BUTTON_UP;
	return;
    }
    if(idx > info->length || x < 0 || y < 0) return;
	
    if((action = event_action(event)) == LOC_DRAG)
	if(info->current != BUTTON_UP) {
	    if(info->current == BUTTON_LEFT) { 
		action = MS_LEFT;
	    } else if(info->current == BUTTON_RIGHT) {
		action = MS_RIGHT;
	    }
	}

    switch(action) {
    case MS_LEFT:
	info->current = BUTTON_LEFT; trace_win(info, TRACE_SET_LEFT, idx);
	break;
    case MS_RIGHT:
	info->current = BUTTON_RIGHT; trace_win(info, TRACE_SET_RIGHT, idx);
	break;
    case MS_MIDDLE:
	trace_win(info, TRACE_SET_LEFT, -1);
	trace_win(info, TRACE_SET_RIGHT, -1);
	info->current = BUTTON_UP;
	break;
    }

    Update_gray(info->leftx, 
		(info->leftx >= 0 ? info->trace[0][info->leftx] : -1),
		info->rightx,
		(info->rightx >= 0 ? info->trace[0][info->rightx] : -1));

    if(info->leftx >= 0 && info->rightx >= 0) {
	a[0] = (double) info->trace[1][info->leftx];
	a[1] = (double) info->trace[2][info->leftx];
	b[0] = (double) info->trace[1][info->rightx];
	b[1] = (double) info->trace[2][info->rightx];
	sprintf(string, "Distance %d.",
		(int) sqrt(pow(a[0]-b[0],2.0) + pow(a[1]-b[1],2.0)));
	Update_info(string);
    }
}
