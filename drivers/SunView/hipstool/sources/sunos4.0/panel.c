/* panel.c
 * Max Rible
 * Panel event procedures for hipstool.
 */

#include "hipstool.h"

static void clear();

void
quit_proc()			/* SUNTOOLS */
{
    window_set(io.control, FRAME_NO_CONFIRM, TRUE, 0);
    window_destroy(io.control);
    if(proj.winfo.pr) pr_destroy(proj.winfo.pr);
    if(base.winfo.pr) pr_destroy(base.winfo.pr);
    return;
}

void
toggle_proc()			/* SUNTOOLS */
{
    resize = (int) panel_get(io.toggles, PANEL_TOGGLE_VALUE, 0);
    auto_log = (int) panel_get(io.toggles, PANEL_TOGGLE_VALUE, 1);
}

#define CMDBUFSIZE 200
#define CMDARGS 20
#define FUDGESIZE 2048

void
parse_proc()
{
#ifdef PARSE_WORKING
    char cmdbuf[CMDBUFSIZE], *args[CMDARGS];
    int i, numargs, to[2], from[2];

    strcpy(cmdbuf, getstring(io.command));

    args[0] = cmdbuf;

    /* Assumption:  no spaces in front of command. */
    for(numargs = 1, i = 1; (i < CMDBUFSIZE) && (cmdbuf[i] != '\0'); i++) {
	if(cmdbuf[i] == ' ') { cmdbuf[i] = '\0'; continue; }
	else if(cmdbuf[i-1] == '\0') { 
	    args[numargs++] = cmdbuf + i; 
	    continue; 
	}
    }
    args[numargs] = NULL;

    pipe(from);			/* Open from process to program */
    pipe(to);			/* Open from program to process */

    /* Child process #1:  Save image down pipe to send to
     * command routine.
     */
    if(fork() == 0) {		/* Is child process #1 */
	close(from[0]); close(to[1]); close(to[0]);
	if(cur_func->func.primit != NULL)
	    save_image((Bool)(cur_func->func.primit->breed == PRIMIT_BOX), 
		       fdopen(from[1], "w"), ON);
	else save_image(0, fdopen(from[1], "w"), ON);
	_exit(1);
    }

    /* Child process #2:  Process and pass to parent */
    if(fork() == 0) {		/* Is child process #2 */
	close(from[1]); close(to[0]); /* Those are the parent's...*/
	dup2(from[0], 0);	/* Get input from parent */
	dup2(to[1], 1);		/* Send output to parent */
	close(from[0]); close(to[1]); /* Clean up those file descriptors */
	execvp(args[0], args);
	_exit(1);
    }

    /* Parent:  Load in output. */

    close(from[0]); close(to[1]); close(from[1]); /* Kid stuff */
    
    load_image(&proj, CHILD_FRAME, fdopen(to[0],"r")); /* Output from child */

    close(to[0]);
#endif
}

/* ARGSUSED */
void
cycle_proc(item, value, event)	/* SUNTOOLS */
     Panel_item item;
     int value;
     Event *event;
{
    if(item == io.rescycle)
	resolution = value + 1;
    if(item == io.source)
	auxiliary = value;
}

void
max_proc()			/* SUNTOOLS */
{
    FileInfo *which;
    int x, y;
    Bool vbar, hbar;

    if(auxiliary == 0)
	which = &base;
    else 
	which = &proj;
		   
    x = which->winfo.width + MARG_X;
    y = which->winfo.height + MARG_Y;

    if(y > SCREEN_HEIGHT) vbar = 1; else vbar = 0;
    if(x > SCREEN_WIDTH) hbar = 1; else hbar = 0;

    if(vbar) x += (int) scrollbar_get((Scrollbar) SCROLLBAR, SCROLL_THICKNESS);
    if(hbar) y += (int) scrollbar_get((Scrollbar) SCROLLBAR, SCROLL_THICKNESS);

    scrollset(&(which->winfo), vbar, hbar);

    window_set(which->winfo.frame,
	       WIN_WIDTH, MIN(SCREEN_WIDTH, x),
	       WIN_HEIGHT, MIN(SCREEN_HEIGHT, y),
	       0);
}

/* ARGSUSED */
void
input_proc(item, event) /* SUNTOOLS */
     Panel_item item;
     Event *event;
{
    static enum function_index current_function = FUNC_BOX;

    if(item == io.inputs[INPUT_START]) {
	clear();
	complete = 0;
	cur_func = Calloc(1, Command);
	cur_func->breed = current_function;
	if(functions[current_function].primit.breed == PRIMIT_NIL) {
	    cur_func->func.primit =
		palloc(menu_info.breed, menu_info.num);
	    cur_func->func.len = menu_info.num;
	} else {
	    cur_func->func.primit =
		palloc(functions[current_function].primit.breed,
		       functions[current_function].primit.num);
	    cur_func->func.len = functions[current_function].primit.num;
	}
/*	Update_info(functions[current_function].start.instruction);
	Prinfo1(functions[current_function].start.info[0]);
	Prinfo2(functions[current_function].start.info[1]);*/
	(*functions[current_function].setup)(cur_func, ON);
    }

    if(item == io.inputs[INPUT_SELECT]) {
	caddr_t retval;
	clear();
	
	if(event_action(event) == MS_RIGHT && event_is_down(event)) {
	    retval = menu_show(io.rootmenu, io.control, event, 0);
	    if((int) retval == 0) return;
	    switch((enum menu_return) retval) {
	    case RETURN_PRIMIT:
		break;
	    case RETURN_FUNCTION:
		current_function = menu_info.function;
		panel_set_value(io.inputs[INPUT_SELECT], 
				functions[current_function].name);
		break;
	    case RETURN_EDIT:
		switch(menu_info.edit) {
		case EDIT_LOG:
		    log_edit();
		    break;
		case EDIT_COLORMAP:
		    colormap_edit();
		    break;
		case EDIT_HEADERTEXT:
		    comment_edit();
		    break;
		}
		break;
	    }
	}
    }

    if(item == io.inputs[INPUT_CLEAR]) {
	if(cur_func != NULL)
	    (*functions[cur_func->breed].setup)(cur_func, OFF);
	clear();
	if(!base.virgin)
	    refresh(&base.winfo, NULL);
	Update_info(" "); Prinfo1(" "); Prinfo2(" ");
	complete = 1;
    }

    if(cur_func == NULL) return;

    if(item == io.inputs[INPUT_EVAL]) { 
	(*functions[cur_func->breed].eval)(cur_func);
	if(auto_log)
	    log_current_action();
    }

    if(item == io.inputs[INPUT_LOG]) {
	log_current_action();
    }
}

static void
clear()
{
    Update_info(" ");
    Prinfo1(" ");
    Prinfo2(" ");

    current_action_num = -1;

    destroy(TYPE_COMMAND, &cur_func);
}

#ifdef MULTO_FRAMES
void
page_proc(item)
     Panel_item item;
{
    char buf[100];

    if(item == io.page.forward) {
	if(base.frames.current < base.frames.num) {
	    base.frames.current++;
	    switch(base.datasize) {
	    case 1:
		base.buf.chars += (base.winfo.width * base.winfo.height);
		break;
	    case 2:
		base.buf.shorts += (base.winfo.width * base.winfo.height);
		break;
	    case 4:
		base.buf.longs += (base.winfo.width * base.winfo.height);
		break;
	    }
	    pixrefresh(&base);
	}
    }
    if(item == io.page.back) {
	if(base.frames.current > 1) {
	    base.frames.current--;
	    switch(base.datasize) {
	    case 1:
		base.buf.chars -= (base.winfo.width * base.winfo.height);
		break;
	    case 2:
		base.buf.shorts -= (base.winfo.width * base.winfo.height);
		break;
	    case 4:
		base.buf.longs -= (base.winfo.width * base.winfo.height);
		break;
	    }
	    pixrefresh(&base);
	}
    }

    sprintf(buf, "%d/%d", base.frames.current, base.frames.num);
    panel_set(io.messages[MESSAGE_FRAME_NUMBER], PANEL_LABEL_STRING,
	      buf, 0);
}
#endif
