
/*
 * regions/call.c -- entry points for region selection
 */

#include <stdio.h>
#include "ui.h"
#include "common.h"
#include "llist.h"
#include "sm.h"
#include "log.h"
#include "reg.h"
#include "display.h"

/* type of region for a particular function */
static int rtype = BOX;

/* #define TRACE_EX */

/***********************************************/
int
getrtype()
{
    return rtype;
}

/***********************************************/
setrtype(t)
    int       t;
{
    if (getnpoints() != 0) {
	XBell(display, 0);
	fprintf(stderr, "can not change region types once points have been entered.\n");
	return -1;
    }
    set_region_plim(t);
    return (0);
}

/***********************************************/
set_region_plim(t)
    int       t;
{
    rtype = t;
    switch (rtype) {
    case NOREG:
	setplim(0);
	new_state(REG_SEL);
	break;
    case LINE:
	if (line_mode != LSQ)
	    setplim(2);
	else
	    setplim(MAXPOINTS);
	break;
    case SPLINE:
	setplim(MAXPOINTS);
	break;
    case CLSPLINE:
	setplim(MAXPOINTS);
	break;
    case POLYGON:
	setplim(MAXPOINTS);
	break;
    case BOX:
	setplim(2);
	break;
    case DUBLIN:
	setplim(4);		/* 2 endpoints for 2 lines */
	break;
    case AN_TEXT:
	setplim(1);		/* just one point -- the start location */
	break;
    case AN_VEC:
	setplim(2);
	break;
    case ENTIRE_IMAGE:
	setplim(0);
	new_state(REG_SEL);
	break;
    default:
	printf("Unknown region type!\n");
	break;
    };
    reg_setup();
    return 0;
}

/*****************************************************************/
/* reg_setup() -- setup stuff for mode REG_SEL. */
reg_setup()
{
    pvreg_start();
}

/*****************************************************************/

/* initializes the current region */
/* the current region is used to "preview" the region before passing it the
 function with the eval button */
pvreg_start()
{
#ifdef TRACE_EX
    fprintf(stderr, "in pvreg_start \n");
#endif

    if (curfunc->pvreg != NULL) {
	free(curfunc->pvreg);
    }
    curfunc->pvreg = newreg(rtype);
}

pvreg_set(head)
    struct plist *head;
{
#ifdef TRACE_EX
    fprintf(stderr, "in pvreg_set \n");
#endif

    refresh_reg(curfunc->pvreg);/* un-draw any old point */

    /* initialize dlist and plist */
    while (curfunc->pvreg->r_dlist != NULL) {
	llist_free((llist **) & curfunc->pvreg->r_dlist);
    }
    if ((curfunc->pvreg->r_dlist =
	 (struct dlist *) malloc(sizeof(struct dlist))) == NULL) {
	perror("malloc");
	exit(1);
    }
    curfunc->pvreg->r_dlist->next = curfunc->pvreg->r_dlist->prev = NULL;
    curfunc->pvreg->r_dlist->len = curfunc->pvreg->r_plen = 0;
    curfunc->pvreg->r_plist = head;

    draw_cpl();			/* draw all cross-bars in the region */

#ifdef OLD
    if (getnpoints() < getplim())
    if (curfunc->pvreg->r_type != AN_TEXT && getnpoints() < 2) 
	return;
#endif
    if ((curfunc->pvreg->r_type == AN_TEXT && getnpoints() < 1) || 
        (curfunc->pvreg->r_type != AN_TEXT && getnpoints() < 2)) 
	return;

    if (line_mode == LSQ)
	curfunc->pvreg->r_flags = LSQ;

    /* if we have enough points, go ahead and draw the region */
    switch (curfunc->pvreg->r_type) {
    case LINE:
    case DUBLIN:
	draw_line(curfunc->pvreg);
	break;
    case SPLINE:
	draw_spline(curfunc->pvreg);
	break;
    case CLSPLINE:
	draw_clspline(curfunc->pvreg);
	break;
    case POLYGON:
	draw_polygon(curfunc->pvreg);
	break;
    case BOX:
	draw_box(curfunc->pvreg);
	break;
    case AN_TEXT:
	draw_text(curfunc->pvreg);
	break;
    case AN_VEC:
	draw_vec(curfunc->pvreg);
    }

    show_region_info();

}

/***************************************************************/
show_region_info()
{
    char     *mesg, *line_info(), *spline_info(), *clspline_info(), *polygon_info(), *box_info();

    switch (curfunc->pvreg->r_type) {
    case LINE:
    case DUBLIN:
	mesg = line_info(curfunc->pvreg);	/* line length */
	break;
    case SPLINE:
    case CLSPLINE:
	mesg = spline_info(curfunc->pvreg);	/* width and height */
	break;
    case POLYGON:
	mesg = polygon_info(curfunc->pvreg);	/* width and height (convex
						 * poly only) */
	break;
    case BOX:
	mesg = box_info(curfunc->pvreg);	/* width and heigth */
	break;
    case AN_TEXT:
	mesg = NULL;
	break;
    case AN_VEC:
	mesg = NULL;
	break;
    }

    xv_set(base_win->infomesg, PANEL_LABEL_STRING, mesg, NULL);
    panel_paint(base_win->infomesg, PANEL_CLEAR);
}


/***********************************************/
struct region *
interp_reg()
{
    struct region *reg;

#ifdef TRACE_EX
    fprintf(stderr, "in interp_reg \n");
#endif

    reg = newreg(rtype);
    if (get_plist(reg) < 0) {
	return NULL;
    }
    /* free the preview region */
    /*
     * set the r_plist member to NULL 'cos we just copied that to our current
     * region
     */
    if (curfunc->pvreg != NULL) {
	curfunc->pvreg->r_plist = NULL;
	curfunc->pvreg->r_plen = 0;
	free_reg(curfunc->pvreg);
	curfunc->pvreg = NULL;
    }
    if (line_mode == LSQ)
	reg->r_flags = LSQ;
    /* and finally, draw the region... */
    if (build_dreg(reg) < 0)
	return NULL;
    else
	return reg;
}

/***********************************************/
/* get the plist from the input */
get_plist(reg)
    struct region *reg;
{
#ifdef TRACE_EX
    fprintf(stderr, "in get_plist \n");
#endif

#ifdef OLD
    if (getnpoints() < getplim()) {
    if (curfunc->pvreg->r_type != AN_TEXT && getnpoints() < 2)  {
#else
    if ((curfunc->pvreg->r_type == AN_TEXT && getnpoints() < 1) || 
        (curfunc->pvreg->r_type != AN_TEXT && getnpoints() < 2)) { 
#endif
	/* not enough points to draw a region! */
	free(reg);
	return -1;
    }
    flush_cpl(reg);		/* get the current point list */

    return 1;
}

/***********************************************/
/* make dlists from plists */
build_dreg(reg)
    struct region *reg;
{
    int       rval = 0;

#ifdef TRACE_EX
    fprintf(stderr, "in build_dreg \n");
#endif

    switch (reg->r_type) {
    case LINE:
    case DUBLIN:
	draw_line(reg);
	break;
    case SPLINE:
	draw_spline(reg);
	break;
    case CLSPLINE:
	draw_clspline(reg);
	break;
    case POLYGON:
	draw_polygon(reg);
	break;
    case BOX:
	draw_box(reg);
	break;
    case AN_TEXT:
	draw_text(reg);
	break;
    case AN_VEC:
	draw_vec(reg);
	break;
    }
    return rval;
}

/***********************************************/
/* refresh_reg() takes a region, UN-draws it */
refresh_reg(reg)
    struct region *reg;
{
    int       wth, hgt, blank;
    char     *get_string(), *text;

#ifdef TRACE_EX
    fprintf(stderr, "in refresh_reg \n");
#endif

    if (reg != NULL) {
	if (reg->r_sbs != NULL) {
	    if (reg->r_sbs->string == NULL) {
		/* not yet copied to region structure, so  get from global in text.c */
		text = get_string(); 
		XTextExtents(xfs, text, strlen(text), &blank, &blank,
			     &blank, &reg->r_sbs->metr);
	    }
	    wth = reg->r_sbs->metr.width + 1;
	    hgt = reg->r_sbs->metr.ascent + reg->r_sbs->metr.descent + 4;
	    fprintf(stderr, "refreshing image: box size: (%d x %d) \n", wth, hgt);
	    XPutImage(display, img_win->d_xid, gc, orig_ximg,
		      reg->r_sbs->x, reg->r_sbs->y - 13,	/* source */
		      reg->r_sbs->x, reg->r_sbs->y - 13,	/* dest */
		      wth, hgt);
	}
	if (reg->r_plist != NULL)
	    ref_pl(reg->r_plist, img_win);
	if (reg->r_dlist != NULL)
	    ref_dlist(img_win->d_xid, reg->r_dlist);
    }
}

/***********************************************/
/*free_reg() frees a region and any associated storage */
free_reg(reg)
    struct region *reg;
{
#ifdef TRACE_EX
    fprintf(stderr, "in free_reg \n");
#endif

    if (reg != NULL) {
	if (reg->r_sbs != NULL) {
	    free(reg->r_sbs);
	    reg->r_sbs = NULL;
	}
	if (reg->r_plist != NULL)
	    llist_free((llist **) & reg->r_plist);
	while (reg->r_dlist != NULL) {
	    llist_free((llist **) & reg->r_dlist);
	}
	free(reg);
    }
}

/* newreg() allocates a new region and returns a pointer to it */
struct region
         *
newreg(type)
    int       type;
{
    struct region *reg;

#ifdef TRACE_EX
    fprintf(stderr, "in newreg \n");
#endif

    if ((reg = (struct region *) malloc(sizeof(struct region))) == NULL) {
	perror("malloc");
	exit(1);
    }
    if ((reg->r_dlist = (struct dlist *) malloc(sizeof(struct dlist))) == NULL) {
	perror("malloc");
	exit(1);
    }
    if ((reg->r_plist = (struct plist *)
	 malloc(sizeof(struct plist))) == NULL) {
	perror("malloc");
	exit(1);
    }
    reg->r_plist = NULL;
    reg->r_plen = 0;
    reg->r_sbs = NULL;
    reg->r_dlist->len = 0;
    reg->r_dlist->prev = reg->r_dlist->next = NULL;
    reg->r_dlist->flags = 0;
    reg->r_type = type;
    reg->r_flags = NULL;
    return reg;
}

/***********************************************/
/* reg_addpt() -- add a point to the plist of a region */
reg_addpt(reg, x, y)
    struct region *reg;
    int       x, y;
{
    struct plist *t;

#ifdef TRACE_EX
    fprintf(stderr, "in reg_addpt \n");
#endif

    t = (struct plist *) malloc(sizeof(struct plist));
    t->pt.x = x;
    t->pt.y = y;
    cbget(orig_ximg, &t->cb, t->pt);
    llist_add((llist *) t, (llist **) & reg->r_plist, (llist **) NULL);
}

/*********************************************************/
/* draw_reg() -- draw a region in the standout color on the display. */
draw_reg(reg)
    struct region *reg;
{
#ifdef TRACE_EX
    fprintf(stderr, "in draw_reg \n");
#endif

    if (reg != NULL) {
	if (reg->r_plist != NULL && !clean_mode)
	    draw_pl(reg->r_plist, img_win);
	if (reg->r_dlist != NULL) {
	    draw_dlist(img_win->d_xid, reg->r_dlist);
	}
	if (reg->r_sbs != NULL) {
	    draw_text_string(reg);
	}
    }
}
