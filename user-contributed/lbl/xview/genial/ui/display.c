/*
 * display.c
 */

#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/textsw.h>
#include <xview/xv_xrect.h>
/* #include <gdd.h> */
#include "display_ui.h"

/*
 * Notify callback function for `gamma'.
 */
void
gamma_proc(item, value, event)
    Panel_item item;
    int       value;
    Event    *event;
{
    float     v = (float) value * 4 / 100 + 1;
    printf("setting gamma to:%f\n", v);
    set_gam(v);
#ifdef DEBUG
    fprintf(stderr, "display: gamma_proc: value: %d\n", value);
#endif
}

/*
 * Notify callback function for `shrink_fac'.
 */
void
shrink_proc(item, value, event)
    Panel_item item;
    int       value;
    Event    *event;
{
    set_shrink_fac(value);
#ifdef DEBUG
    fprintf(stderr, "display: shrink_proc: value: %u\n", value);
#endif
}

/*
 * Notify callback function for `cmap_min'.
 */
Panel_setting
cm_min_proc(item, event)
    Panel_item item;
    Event    *event;
{
    int       value = (int) xv_get(item, PANEL_VALUE);

    set_cmin(value);
#ifdef DEBUG
    fprintf(stderr, "display: cm_min_proc: value: %d\n", value);
#endif
    return panel_text_notify(item, event);
}

/*
 * Notify callback function for `cmap_max'.
 */
Panel_setting
cm_max_proc(item, event)
    Panel_item item;
    Event    *event;
{
    int       value = (int) xv_get(item, PANEL_VALUE);

    set_cmax(value);
#ifdef DEBUG
    fprintf(stderr, "display: cm_max_proc: value: %d\n", value);
#endif
    return panel_text_notify(item, event);
}
