/* histo.c
 * Max Rible
 *
 * Everything I can gather together about HIPStool histograms.
 */

#include "hipstool.h"

static void histo(), flood_fill(), histo_win(), histo_event_proc();

#define	HISTO_SET_LEFT	0
#define	HISTO_SET_RIGHT	1
#define	HISTO_DISPLAY	2

#define	BUTTON_UP	0
#define	BUTTON_LEFT	1
#define	BUTTON_RIGHT	2

typedef struct histo_information {
    int leftx, rightx;		/* Left and right coordinates */
    unsigned extremes[2];	/* Extreme values of data */
    unsigned range[2];		/* Current displayed range */
    unsigned *data, display[256];
    unsigned maxnum;
    int current;
    Windowinfo winfo;
} HistoInfo;

#define COLOR_BLACK 2
#define COLOR_GREEN 1
#define COLOR_RED 3
#define COLOR_WHITE 0

void
histo_eval(command)
     Command *command;
{
    Primit *primit = command->func.primit;
    Point tmp;
    int x = 0, y = 0, num = 0;
    unsigned (*getval)() = readarr[base.datasize], *arr;

    command->histo = Calloc(base.extremes[1] - base.extremes[0] + 1, unsigned);

    if(primit->breed == PRIMIT_BOX) {
	arr = command->histo;
	for(x = primit->loc.coords[0][0]; x < primit->loc.coords[1][0]; x++)
	    for(y = primit->loc.coords[0][1]; y < primit->loc.coords[1][1];y++)
		arr[((*getval)(x,y,0)) - base.extremes[0]]++;
    } else {
	for(tmp = primit->loc.list; tmp->next != NULL; tmp = tmp->next)
	    do_cross(&tmp->i, OFF);
	
	refresh(&base.winfo, NULL);
	
	(*priminfo[primit->breed].display)(primit, DRAW_IN_PIXWIN);
	
	for(tmp = primit->loc.list; tmp != NULL; tmp = tmp->next) {
	    x += tmp->i.x; y += tmp->i.y; num++;
	}
	flood_fill(x / num, y / num, getval, command->histo);
    }
    histo(command->histo, 500, 50, base.extremes[0], base.extremes[1]);
    save_menu_funcs[SAVE_HISTO_FILE].active = 1;
    update_save_menu();
    Update_info("Histogram complete.");

    refresh(&base.winfo, cur_func);
}

/* ARGSUSED */
void
histo_setup(command, value)
     Command *command;
     int value;
{
    if(value == OFF) {
	if(cur_func->func.primit->breed == PRIMIT_BOX)
	    save_menu_funcs[SAVE_BOX_SUBIMAGE].active = 0;
	save_menu_funcs[SAVE_HISTO_FILE].active = 0;
	update_save_menu();
    }
}

static void
histo(data, x, y, minv, maxv)
     unsigned *data, minv, maxv;
     int x, y;
{
    unsigned char communist[4], ecologist[4], royalty[4];
    HistoInfo *newhistoinfo;

    communist[COLOR_BLACK] = 0;
    ecologist[COLOR_BLACK] = 0;
    royalty[COLOR_BLACK] = 0;
    communist[COLOR_RED] = 255;
    ecologist[COLOR_RED] = 0;
    royalty[COLOR_RED] = 0;
    communist[COLOR_GREEN] = 0;
    ecologist[COLOR_GREEN] = 255;
    royalty[COLOR_GREEN] = 0;
    communist[COLOR_WHITE] = 255;
    ecologist[COLOR_WHITE] = 255;
    royalty[COLOR_WHITE] = 255;

    newhistoinfo = Calloc(1, HistoInfo);

    newindow(&newhistoinfo->winfo,
	     256 + TXTWID, 256 + TXTHT,
	     x, y,
	     "HIPStool histogram");

    pw_setcmsname(newhistoinfo->winfo.pw, "HIPShisto");
    pw_putcolormap(newhistoinfo->winfo.pw, 0, 4, communist,ecologist,royalty);

    newhistoinfo->data = data;
    newhistoinfo->range[0] = newhistoinfo->extremes[0] = minv;
    newhistoinfo->range[1] = newhistoinfo->extremes[1] = maxv;
    newhistoinfo->leftx = newhistoinfo->rightx = -1;

#ifdef SUNTOOLS
    window_set(newhistoinfo->winfo.canvas,
	       WIN_CLIENT_DATA, newhistoinfo,
	       WIN_EVENT_PROC, histo_event_proc,
	       WIN_CONSUME_PICK_EVENTS,
	       WIN_NO_EVENTS, WIN_MOUSE_BUTTONS, LOC_DRAG, 0,
	       0);
#endif

    histo_win(newhistoinfo, HISTO_DISPLAY);
}

static void
flood_fill(x, y, read_proc, arr)
     int x, y;
     unsigned (*read_proc)(), *arr;
{
#define Recurse(dx, dy) if(get_pix(&base.winfo, x+dx, y+dy) != STANDOUT) flood_fill(x+dx, y+dy, read_proc, arr)

    arr[(*read_proc)(x, y, 0) - base.extremes[0]]++;
    put_pix(&base.winfo, x, y, STANDOUT);
    Recurse(-1, 0); Recurse(1, 0); Recurse(0, -1); Recurse(0, 1);
#undef Recurse
}

static void
histo_win(va_alist)
     va_dcl
{
    HistoInfo *info;
    int argument;
    char label[80];
    int i, x, *val;
    unsigned delta, maxnum, start, end;
    va_list index;

    va_start(index);
    info = va_arg(index, HistoInfo *);
    argument = va_arg(index, int);

    switch(argument) {
    case HISTO_SET_LEFT:
	val = &info->leftx;
	goto display;
    case HISTO_SET_RIGHT:
	val = &info->rightx;
    display:			/* Fall through */
	x = va_arg(index, int);

	if(*val >= 0) {
	    line(&info->winfo,
		 *val, 255, 
		 *val, (int)(255-SCALE((double)info->display[info->leftx]/
				       (double)info->maxnum)),
		 COLOR_BLACK);
	    line(&info->winfo,
		 *val, (int)(255-SCALE((double)info->display[info->leftx]/
				       (double)info->maxnum)),
		 *val, 0,
		 COLOR_WHITE);
	}
	*val = x;
	line(&info->winfo, x, 0, x, 255, COLOR_GREEN);
	break;
    case HISTO_DISPLAY:
	delta = info->range[1] - info->range[0] + 1;
	sprintf(label, "New range [%d,%d].", info->range[0], info->range[1]);
	Update_info(label);
	
	for(i = 0; i < 256; i++) info->display[i] = 0;
	
	if(info->range[1] < 256) {
	    for(i = info->range[0]; i <= info->range[1]; i++)
		info->display[i] = info->data[i - info->extremes[0]];
	    start = 0; end = 255;
	    info->range[0] = 0; info->range[1] = 255;
	} else {
	    for(i = info->range[0]; i <= info->range[1]; i++)
		info->display[SCALE((double)(i-info->range[0])/(double)delta)]
		    += info->data[i - info->extremes[0]];
	    start = info->range[0]; end = info->range[1];
	}
	
	maxnum = 0;
	for(i = 0; i < 256; i++)
	    maxnum = MAX(maxnum, info->display[i]);

	Update_info("Calculated new histogram.");
	
	batching(&info->winfo, ON);
	
	wipe(&info->winfo, COLOR_WHITE);
	
	/* Horizontal labeling */
	sprintf(label, "%d", start);
	text(&info->winfo, 0, 255 + TXTHT, label, COLOR_BLACK);
	
	line(&info->winfo, 64, 0, 64, 255, COLOR_RED);
	sprintf(label, "%d", start + (end - start + 1)/4);
	text(&info->winfo, 64, 255 + TXTHT, label, COLOR_BLACK);
	
	line(&info->winfo, 128, 0, 128, 255, COLOR_RED);
	sprintf(label, "%d", start + (end - start + 1)/2);
	text(&info->winfo, 128, 255 + TXTHT, label, COLOR_BLACK);
	
	line(&info->winfo, 192, 0, 192, 255, COLOR_RED);
	sprintf(label, "%d", start + (3*(end - start + 1))/4);
	text(&info->winfo, 192, 255 + TXTHT, label, COLOR_BLACK);
	
	sprintf(label, "%d", end);
	text(&info->winfo, 255, 255 + TXTHT, label, COLOR_BLACK);
	
	for(i = 0; i < 256; i++)
	    line(&info->winfo,
		 i, 255,
		 i, (int)(255-SCALE((double)(info->display[i]) /
				    (double)maxnum)),
		 COLOR_BLACK);

	/* Vertical labeling */
	sprintf(label, "%d", maxnum);
	text(&info->winfo, 257, TXTHT, label, COLOR_BLACK);
	
	line(&info->winfo, 0, 64, 255, 64, XOR_COLOR);
	sprintf(label, "%d", (3*maxnum)/4);
	text(&info->winfo, 257, 64, label, COLOR_BLACK);
	
	line(&info->winfo, 0, 128, 255, 128, XOR_COLOR);
	sprintf(label, "%d", maxnum/2);
	text(&info->winfo, 257, 128, label, COLOR_BLACK);
	
	line(&info->winfo, 0, 192, 255, 192, XOR_COLOR);
	sprintf(label, "%d", maxnum/4);
	text(&info->winfo, 257, 192, label, COLOR_BLACK);
	
	text(&info->winfo, 257, 255, "0", COLOR_BLACK);
	
	info->leftx = info->rightx = -1;
	
	info->maxnum = maxnum;

	batching(&info->winfo, OFF);
	break;
    }
    va_end(index);
}

/* ARGSUSED */
static void
histo_event_proc(window, event, arg)
     Window window;
     Event *event;
     caddr_t arg;
{
    int x, up, action;
    HistoInfo *info;
    unsigned delta;

    x = event_x(event); up = event_is_up(event); action = event_action(event);
    info = (HistoInfo *) window_get(window, WIN_CLIENT_DATA);

    if(up) { info->current = BUTTON_UP; return; }
    if(action == LOC_DRAG) {
	if(info->current == BUTTON_UP) { 
	    return;
	} else if(info->current == BUTTON_LEFT) {
	    action = MS_LEFT;
	} else if(info->current == BUTTON_RIGHT) {
	    action = MS_RIGHT;
	}
    }
    if(x < 0 || x > 255) return;

    switch(action) {
    case MS_LEFT:
	info->current = BUTTON_LEFT;
	histo_win(info, HISTO_SET_LEFT, x);
	break;
    case MS_RIGHT:
	info->current = BUTTON_RIGHT;
	histo_win(info, HISTO_SET_RIGHT, x);
	break;
    case MS_MIDDLE:
	delta = info->range[1] - info->range[0] + 1;
	if(info->leftx < 0 || info->rightx < 0) {
	    info->range[0] = info->extremes[0];
	    info->range[1] = info->extremes[1];
	    info->leftx = -1; info->rightx = -1;
	} else {
	    info->range[1] = info->range[0] +
		(int)(((double)info->rightx/255.0)*((double)delta));
	    info->range[0] = info->range[0] +
		(int)(((double)info->leftx/255.0)*((double)delta));
	}

	histo_win(info, HISTO_DISPLAY);
	break;
    }
}
