
#include <stdio.h>
#include "display.h"
#include "ui.h"
#include "sm.h"
#include "log.h"
#include "reg.h"

static int state;

init_sm()
{
    int       main_dispatch();

    state = IMG_UNLOADED;
    set_panel_state(state);
    dp_push(main_dispatch);
}

new_state(ns)
    int       ns;
{
    state = ns;
    set_panel_state(state);
}

int
getstate()
{
    return state;
}

/* main_dispatch() is the main dispatch loop for GENIAL */
int
main_dispatch(token, arg)
    int       token;		/* token as defined in sm.h */
    caddr_t  *arg;		/* pointer to an optional argument */
{
    XPoint    pt;		/* x and y *IMAGE* coordinates of the event */

#ifdef DEBUG
    printf(" main dispatch: state: %d, token: %d \n", state, token);
#endif

    set_watch_cursor();
    switch (state) {
	/*--------------------------------------------*/
    case IMG_UNLOADED:
	switch (token) {
	case LOAD:
	    if (load_image((char *) arg) == 1) {
		fxn_init();
		new_state(IMG_LOADED);
	    };
	    break;
	case QUIT:
	    shutdown();
	    break;
	default:
#ifdef DEBUG
	    printf("inappropriate action.\n");
#endif
	    break;
	}
	break;
	/*--------------------------------------------*/
    case IMG_LOADED:
	switch (token) {
	case LOAD:
	    clear_log();
	    fxn_reset();
	    if (load_image((char *) arg) == 1) {
		fxn_init();
		new_state(IMG_LOADED);
	    };
	    break;
	case CLEAR:
	    if (curfunc->prev != NULL) {
		log_del(curfunc->prev->id);
		fxn_clear(curfunc->prev);
	    }
	    break;
	case EVAL:
	    if (getrtype() == NOREG) {
		log_perm();
		fxn_eval();
		fxn_reset();
		fxn_init();
	    };
	    break;
	case FORW:
	    adv_frame();
	    break;
	case BACK:
	    rev_frame();
	    break;
	case FMENU:
	    fxn_reset();
	    fxn_select(*((int *) arg));
	    fxn_init();
	    break;
	case IMG_BUT:
	    if (event_is_down((Event *) arg)) {
		if ((event_id((Event *) arg) == BUT(1))) {
		    if (getrtype() != NOREG) {
			new_state(REG_SEL);
			pt.x = dtoi(event_x((Event *) arg));
			pt.y = dtoi(event_y((Event *) arg));
			if ((line_mode == DRAG) && (getrtype() == LINE))
			    start_line(pt.x, pt.y);
			else
			    add_point(pt.x, pt.y);
		    }
		}
	    }
	    break;
	case QUIT:
	    shutdown();
	    break;
	default:
#ifdef DEBUG
	    printf("Invalid action.\n");
#endif
	    break;
	}
	break;
	/*--------------------------------------------*/
    case REG_SEL:
	switch (token) {
	case LOAD:
	    clear_plist();
	    clear_log();
	    fxn_reset();
	    if (load_image((char *) arg) == 1) {
		fxn_init();
		new_state(IMG_LOADED);
	    };
	    new_state(IMG_LOADED);
	    break;
	case QUIT:
	    shutdown();
	case EVAL:
	    do_eval();
	    break;
	case CLEAR:
	    clear_plist();
	    new_state(IMG_LOADED);
	    break;
	case IMG_BUT:
	    if (event_is_down((Event *) arg)) {
		if (event_id((Event *) arg) == BUT(1)) {
		    pt.x = dtoi(event_x((Event *) arg));
		    pt.y = dtoi(event_y((Event *) arg));
		    add_point(pt.x, pt.y);	/* left button, add point */
		}
		if (event_id((Event *) arg) == BUT(2)) {
		    pt.x = dtoi(event_x((Event *) arg));
		    pt.y = dtoi(event_y((Event *) arg));
		    /* middle button, move point */
		    move_point((int) pt.x, (int) pt.y);
		}
		if (event_id((Event *) arg) == BUT(3)) {
		    pt.x = dtoi(event_x((Event *) arg));
		    pt.y = dtoi(event_y((Event *) arg));
		    del_point((int) pt.x, (int) pt.y);
		    if (getnpoints() == 0)	/* right button, remove point */
			new_state(IMG_LOADED);
		}
	    }
	    break;
	default:
	    break;
	}
	break;
	/*--------------------------------------------*/
    case REG_EDIT:
	switch (token) {
	case EVAL:
	    do_edit_eval();
	    break;
	case QUIT:
	    shutdown();
	    break;
	case IMG_BUT:
	    if (event_is_down((Event *) arg)) {
		if (event_id((Event *) arg) == BUT(1) ||
		    event_id((Event *) arg) == BUT(2)) {
		    pt.x = dtoi(event_x((Event *) arg));
		    pt.y = dtoi(event_y((Event *) arg));
		    /* left or middle button, move point */
		    move_point((int) pt.x, (int) pt.y);
		}
#ifdef NEED_LATER?
		if (event_id((Event *) arg) == BUT(3)) {
		    pt.x = dtoi(event_x((Event *) arg));
		    pt.y = dtoi(event_y((Event *) arg));
		    /* right button, remove point */
		    del_point((int) pt.x, (int) pt.y);
		    if (getnpoints() == 0)
			new_state(IMG_LOADED);
		}
#endif
	    }
	    break;
	default:
	    break;
	}
	break;
    default:
	fprintf(stderr, "OUCH!!  Invalid _STATE_!!!\n");
	exit(0);
	break;
    }
    unset_watch_cursor();
    return getstate();
}

/*******************************************************************/

do_eval()
{
    if (getrtype() != NOREG) {
	if (curfunc->reg == NULL) {
	    if (getrtype() == ENTIRE_IMAGE) {	/* create box region */
		setrtype(BOX);
		add_point(0, 0);
		add_point(orig_img->width, orig_img->height);
	    }
	    if ((curfunc->reg = interp_reg()) == NULL) {
		/* not enough points or some similar condition */
		return;
	    }
	} else {
	    /* make dlists from the plists */
	    if (build_dreg(curfunc->reg) < 0) {
		/* didnt work */
		printf("region draw didnt work\n");
		return;
	    }
	}
    }
    fxn_eval();
    log_perm();
    fxn_reset();
    fxn_init();
    new_state(IMG_LOADED);
}

/*******************************************************************/

do_edit_eval()
{
    if (curfunc->reg == NULL) {
	if ((curfunc->reg = interp_reg()) == NULL) {
	    /* not enough points or some similar condition */
	    return;
	}
    } else {
	/* make dlists from the plists */
	if (build_dreg(curfunc->reg) < 0) {
	    /* didnt work */
	    printf("region draw didnt work\n");
	    return;
	}
    }
    fxn_change(curfunc->id);
}

/*******************************************************************/
/* set_panel_state() sets the appropriate items to active or inactive based on
   the state */
set_panel_state(s)
    int       s;
{
    panel_deactivate();		/* turn everything off, then turn on what is
				 * needed */
    switch (s) {
    case IMG_UNLOADED:
	xv_set(base_win->file,
	       PANEL_INACTIVE, FALSE,
	       NULL);
	xv_set(base_win->quit,
	       PANEL_INACTIVE, FALSE,
	       NULL);
	break;
    case IMG_LOADED:
	xv_set(base_win->file,
	       PANEL_INACTIVE, FALSE,
	       NULL);
	xv_set(base_win->quit,
	       PANEL_INACTIVE, FALSE,
	       NULL);
	xv_set(base_win->clear,
	       PANEL_INACTIVE, FALSE,
	       NULL);
	xv_set(base_win->clear_all,
	       PANEL_INACTIVE, FALSE,
	       NULL);
	xv_set(base_win->func_sel,
	       PANEL_INACTIVE, FALSE,
	       NULL);
	xv_set(base_win->reg_type,
	       PANEL_INACTIVE, FALSE,
	       NULL);
	xv_set(base_win->refresh,
	       PANEL_INACTIVE, FALSE,
	       NULL);
	xv_set(base_win->edit_mode,
	       PANEL_INACTIVE, FALSE,
	       NULL);
	label_img(orig_img);
	break;
    case REG_SEL:
	xv_set(base_win->file,
	       PANEL_INACTIVE, FALSE,
	       NULL);
	xv_set(base_win->quit,
	       PANEL_INACTIVE, FALSE,
	       NULL);
	xv_set(base_win->clear,
	       PANEL_INACTIVE, FALSE,
	       NULL);
	xv_set(base_win->clear_all,
	       PANEL_INACTIVE, FALSE,
	       NULL);
	xv_set(base_win->eval,
	       PANEL_INACTIVE, FALSE,
	       NULL);
	xv_set(base_win->refresh,
	       PANEL_INACTIVE, FALSE,
	       NULL);
	xv_set(base_win->edit_mode,
	       PANEL_INACTIVE, FALSE,
	       NULL);
	break;
    case REG_EDIT:
	xv_set(base_win->quit,
	       PANEL_INACTIVE, FALSE,
	       NULL);
	xv_set(base_win->eval,
	       PANEL_INACTIVE, FALSE,
	       NULL);
	xv_set(base_win->edit_mode,
	       PANEL_INACTIVE, FALSE,
	       NULL);
	break;
    }
}

/************************************************************/
panel_deactivate()
{
    xv_set(base_win->file,
	   PANEL_INACTIVE, TRUE,
	   NULL);
    xv_set(base_win->quit,
	   PANEL_INACTIVE, TRUE,
	   NULL);
    xv_set(base_win->prev,
	   PANEL_INACTIVE, TRUE,
	   NULL);
    xv_set(base_win->next,
	   PANEL_INACTIVE, TRUE,
	   NULL);
    xv_set(base_win->func_sel,
	   PANEL_INACTIVE, TRUE,
	   NULL);
    xv_set(base_win->reg_type,
	   PANEL_INACTIVE, TRUE,
	   NULL);
    xv_set(base_win->eval,
	   PANEL_INACTIVE, TRUE,
	   NULL);
    xv_set(base_win->clear,
	   PANEL_INACTIVE, TRUE,
	   NULL);
    xv_set(base_win->clear_all,
	   PANEL_INACTIVE, TRUE,
	   NULL);
    xv_set(base_win->refresh,
	   PANEL_INACTIVE, TRUE,
	   NULL);
    xv_set(base_win->edit_mode,
	   PANEL_INACTIVE, TRUE,
	   NULL);
    xv_set(base_win->edit_log_num,
	   PANEL_INACTIVE, TRUE,
	   NULL);
    xv_set(base_win->edit_ok,
	   PANEL_INACTIVE, TRUE,
	   NULL);
}
