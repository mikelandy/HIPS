
/*
 * trace_opts.c
 */

#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include "ui.h"
#include "display.h"
#include "trace_ui.h"
#include "scale.h"
#include "log.h"
#include "reg.h"

struct trcontext *trace_by_lid();

extern trace_tcntrl_objects *trace_tcntrl;
extern trace_trwin_objects *trace_twin;

static int save_scale_type, save_min, save_max;

/*****************************************************************/
trace_opt_proc()
{
    if (trace_tcntrl == NULL)
	return;

    if (((int) xv_get(trace_tcntrl->tcntrl, XV_SHOW, NULL)) == FALSE) {
	xv_set(trace_tcntrl->tcntrl,
	       FRAME_CMD_PUSHPIN_IN, TRUE, NULL);
	xv_set(trace_tcntrl->tcntrl,
	       XV_SHOW, TRUE, FRAME_CLOSED, FALSE, NULL);
    } else {
	xv_set(trace_tcntrl->tcntrl,
	       FRAME_CMD_PUSHPIN_IN, FALSE, NULL);
	xv_set(trace_tcntrl->tcntrl,
	       XV_SHOW, FALSE, NULL);
    }
    return;
}

/*****************************************************************/
/* routine to update the control panel to reflect the status of the current
   trace */

trace_panel(curtrace)
    struct trcontext *curtrace;
{
    xv_set(trace_tcntrl->radius,
	   PANEL_VALUE, curtrace->t_attr.avgrad,
	   NULL);
    xv_set(trace_tcntrl->scale,
	   PANEL_VALUE, curtrace->t_attr.scale_type,
	   NULL);
    xv_set(trace_tcntrl->min,
	   PANEL_VALUE, curtrace->t_attr.axmin,
	   PANEL_MIN_VALUE, 0,
	   PANEL_MAX_VALUE, orig_img->maxv,
	   NULL);
    xv_set(trace_tcntrl->max,
	   PANEL_VALUE, curtrace->t_attr.axmax,
	   PANEL_MIN_VALUE, 0,
	   PANEL_MAX_VALUE, orig_img->maxv,
	   NULL);

    save_scale_type = curtrace->t_attr.scale_type;
    save_min = curtrace->t_attr.axmin;
    save_max = curtrace->t_attr.axmax;
}

/*****************************************************************/
void
trace_opt_reset(item, event)
    Panel_item item;
    Event    *event;
{
#ifdef DEBUG
#endif
    fputs("trace: trace_opt_reset (not yet done!) \n", stderr);

    xv_set(trace_tcntrl->scale,
	   PANEL_VALUE, save_scale_type,
	   NULL);
    xv_set(trace_tcntrl->min,
	   PANEL_VALUE, save_min,
	   NULL);
    xv_set(trace_tcntrl->max,
	   PANEL_VALUE, save_max,
	   NULL);
}

/*****************************************************************/
Panel_setting
traceno_proc(item, event)
    Panel_item item;
    Event    *event;
{
    int       value = (int) xv_get(item, PANEL_VALUE);
    struct trcontext *context;

    if (value < 1) {
	xv_set(item, PANEL_VALUE, 1, NULL);
	return panel_text_notify(item, event);
    }
    if (value > get_current_lid()) {
	xv_set(item, PANEL_VALUE, get_current_lid(), NULL);
	return panel_text_notify(item, event);
    }

    context = trace_by_lid(value);

    if (context == NULL) {
	panel_text_notify(item, event);
	return panel_text_notify(item, event);
    }
    trace_panel(context);
#ifdef DEBUG
    fprintf(stderr, "trace: traceno_proc: value: %d\n", value);
#endif
    panel_text_notify(item, event);
    return panel_text_notify(item, event);
}


/*****************************************************************/
/*
 * Notify callback function for `scale'.
 */
void
scale_proc(item, value, event)	/* change scale type */
    Panel_item item;
    int       value;
    Event    *event;
{
    struct trcontext *context;

    context = trace_by_lid(xv_get(trace_tcntrl->traceno, PANEL_VALUE, NULL));
    if (context == NULL) {
	panel_text_notify(item, event);
	return;
    }
    context->t_attr.scale_type = value;

    scale_trace(context);
    trace_panel(context);
    /* repaint the window */
    trcanv_repaint_proc(context->win_info->trcanv, context->trpntw, display,
			context->trxid, (Xv_xrectlist *) NULL);
#ifdef DEBUG
    fprintf(stderr, "trace: scale_proc: value: %u\n", value);
#endif
}

/*****************************************************************/
/*
 * Notify callback function for `radius'.
 */
Panel_setting
rad_proc(item, event)
    Panel_item item;
    Event    *event;
{
    int       value = (int) xv_get(item, PANEL_VALUE);
    struct trcontext *context;

#ifdef DEBUG
    fprintf(stderr, "trace: rad_proc: value: %d\n", value);
#endif
    context = trace_by_lid(xv_get(trace_tcntrl->traceno, PANEL_VALUE, NULL));
    if (context == NULL) {
	return panel_text_notify(item, event);
    }
    context->t_attr.avgrad = value;

    /* repaint the window */
    trcanv_repaint_proc(context->win_info->trcanv, context->trpntw, display,
			context->trxid, (Xv_xrectlist *) NULL);
    return panel_text_notify(item, event);
}

/*****************************************************************/
/*
 * Notify callback function for `min'.
 */
Panel_setting
min_proc(item, event)
    Panel_item item;
    Event    *event;
{
    int       value = (int) xv_get(item, PANEL_VALUE);
    struct trcontext *context;

#ifdef DEBUG
    fprintf(stderr, "trace: min_proc: value: %d\n", value);
#endif
    context = trace_by_lid(xv_get(trace_tcntrl->traceno, PANEL_VALUE, NULL));
    if (context == NULL) {
	return panel_text_notify(item, event);
    }
    context->t_attr.scale_type = 2;
    scale_trace(context);
    trace_panel(context);

    /* repaint the window */
    trcanv_repaint_proc(context->win_info->trcanv, context->trpntw, display,
			context->trxid, (Xv_xrectlist *) NULL);
    return panel_text_notify(item, event);
}

/*****************************************************************/
/*
 * Notify callback function for `max'.
 */
Panel_setting
max_proc(item, event)
    Panel_item item;
    Event    *event;
{
    int       value = (int) xv_get(item, PANEL_VALUE);
    struct trcontext *context;

#ifdef DEBUG
    fprintf(stderr, "trace: max_proc: value: %d\n", value);
#endif
    context = trace_by_lid(xv_get(trace_tcntrl->traceno, PANEL_VALUE, NULL));
    if (context == NULL) {
	return panel_text_notify(item, event);
    }
    context->t_attr.scale_type = 2;
    scale_trace(context);
    trace_panel(context);

    /* repaint the window */
    trcanv_repaint_proc(context->win_info->trcanv, context->trpntw, display,
			context->trxid, (Xv_xrectlist *) NULL);
    return panel_text_notify(item, event);
}
