/*
 * log.c -- routines for maintaining log entries
 */

#include <stdio.h>
#include "common.h"
#include "llist.h"
#include "display.h"
#include "ui.h"
#include "reg.h"
#include "log.h"

struct logent *curfunc = NULL;	/* the current log entry (not yet evaled) */
struct logent *lastfunc = NULL;	/* the last 'evaled' log entry */
struct logent *loghead, *logtail;	/* head and tail of log list */

static int lid = 1;
struct logent *newfunc(), *log_by_id();

XFontStruct *xfs;		/* structure for holding font information.
				 * initialized in init_log() */

/****************************************************************/
init_log()
{
    lid = 1;

    curfunc = newfunc();
    loghead = logtail = curfunc;

    /*
     * get font for log label numbers load an XFonstStruct off the server and
     * make that the current font
     */

    if (!(xfs = XLoadQueryFont(display, "8x13bold"))) {
	fprintf(stderr, "Font 8x13bold not found, trying \'fixed\' \n");
	if (!(xfs = XLoadQueryFont(display, "fixed"))) {
	    fprintf(stderr, "error geting fonts, exitting...\n");
	    exit(-1);
	}
    }
    XSetFont(display, gc, xfs->fid);

    return;
}

/*************************************************************/
struct logent *
newfunc()
{
    struct logent *f;

    f = (struct logent *) malloc(sizeof(struct logent));
    f->opcode = 0;
    f->id = lid;

#ifdef DEBUG
    printf("New function: ID# = %d \n", lid);
#endif

    f->fnum = -1;		/* will assign appropriate frame number in
				 * log_perm() */
    f->reg = f->pvreg = NULL;
    f->sbs = NULL;
    f->auxdata = NULL;
    f->auxdsize = 0;
    f->trace = NULL;
    f->hist = NULL;
    f->zoom = NULL;
    f->next = f->prev = NULL;

    lid++;			/* set next lid # */

    return f;
}

/***************************************************************/
/* log_del() -- delete function out of log */
log_del(id)
    int       id;
{
    XPoint    pt;
    struct logent *func;

    if ((func = log_by_id(id)) == NULL)
	return;

    printf("deleting object # %d from log\n", func->id);
#ifdef DEBUG
    printf("before delete:\n");
    show_log_list();		/* for debugging */
#endif

    refresh_reg(func->reg);
    free_reg(func->reg);
    free_reg(func->pvreg);
    if (func->sbs != NULL) {
	pt.x = itod(func->sbs->x);
	pt.y = itod(func->sbs->y);
	/* redraw image under log # */
	XPutImage(display, img_win->d_xid, gc, disp_ximg,
		  pt.x, pt.y - 13, pt.x, pt.y - 13,
		  func->sbs->metr.width,
		  func->sbs->metr.ascent + func->sbs->metr.descent + 4);
	free(func->sbs);
	func->sbs = NULL;
    }
    llist_del((llist *) func, (llist **) & loghead, (llist **) & logtail);

    curfunc = logtail;
    curfunc->id--;
    lid--;

#ifdef DEBUG
    printf("after delete:\n");
    show_log_list();		/* for debugging */
#endif
}

/*********************************************************************/
renumber_logged_objects()
{
    struct logent *func;
    char      title[80];
    int       id = 1;

    func = loghead;
    while (func != NULL) {
	func->id = id;
	log_label(func);
	switch (func->opcode) {
	case LINE_TRACE:
	    if (func->trace != NULL) {
		sprintf(title, "Trace: %d", func->id);
		xv_set(func->trace->win_info->trwin, XV_LABEL, title, NULL);
	    }
	    break;
	case HISTOGRAM:
	    if (func->hist != NULL) {
		sprintf(title, "Histogram: %d", func->id);
		xv_set(func->hist->histo_display->display, XV_LABEL, title, NULL);
	    }
	    break;
	case ZOOM:
	    if (func->zoom != NULL) {
		sprintf(title, "Zoom: %d", func->id);
		xv_set(func->zoom->display->zmwin, XV_LABEL, title, NULL);
	    }
	    break;
	case DISTANCE:
	    break;
	case ANGLE_MES:
	    break;
	case ANNOTATE:
	    break;
	case COMMENT:
	    break;

	}
	id++;
	func = func->next;
    }


}

/*********************************************************************/
show_log_list()
{				/* for debugging */
    struct logent *func;

    if (loghead == NULL)
	return;

    printf("head: id= %d; ", loghead->id);
    func = loghead->next;
    while (func != NULL) {
	printf("next: id= %d; ", func->id);
	func = func->next;
    }

    printf("tail: id= %d; ", logtail->id);
    printf("curfunc: id= %d \n", curfunc->id);
}

/*********************************************************************/
/* clear_log() -- clears entire log */
clear_log()
{
    struct logent *func;

#ifdef DEBUG
    printf("clear_log\n");
#endif

    func = logtail->prev;	/* clear logged objects, starting from the
				 * end if the list */
    while (func != NULL && func->id > 0) {
	fxn_clear(func);
	log_del(func->id);
	func = logtail->prev;	/* new logtail now ! */
    }
}

/**********************************************************************/
/* log_by_id() -- get a log entry by its id # */
struct logent
         *
log_by_id(id)
    int       id;
{
    struct logent *trav;

    for (trav = loghead; trav != NULL; trav = trav->next) {
	if (trav->id == id)
	    return trav;
    }
    return NULL;
}

/**********************************************************************/
/* log_perm -- move curfunc into the log and initialize another one
 *             this is called everytime 'eval' is hit
 */
log_perm()
{
    struct logent *next;

#ifdef TRACE_EX
    fprintf(stderr, " in log_perm \n");
#endif

    if (curfunc->fnum < 0)
	curfunc->fnum = curframe();

    log_label(curfunc);
    orig_img->file_saved = 0;	/* logged object will need save */

    next = newfunc();
    next->opcode = curfunc->opcode;	/* copy opcode from previous entry */

    /* add new log structure to the end of the list */
    llist_add((llist *) next, (llist **) & loghead, (llist **) & logtail);

    curfunc = next;
}

/**********************************************************************/
/* check_frame() checks a log id and verifies whether it is in the current
   frame.
 */
int
check_frame(id)
    int       id;
{
    struct logent *ent;

    ent = log_by_id(id);
    if (ent == NULL)
	return 0;
    if (ent->fnum == curframe())
	return 1;
    else
	return 0;
}

/*****************************************************************/
/* draw_log() -- refresh all points and crosses in all logged objects */
draw_log()
{
    struct logent *trav;

#ifdef TRACE_EX
    fprintf(stderr, "in draw_log \n");
#endif

    for (trav = loghead; trav != NULL; trav = trav->next) {
	if (trav->fnum == curframe()) {
	    if (trav->reg != NULL) {
		draw_reg(trav->reg);
	    }
	    log_label(trav);
	}
    }
    draw_cpl();
}

/**********************************************************************/
/* log_chreg() -- change the region associated with a particular log
   entry. */

log_chreg(id, newreg)
    int       id;		/* id of log entry */
    struct region *newreg;	/* new region */
{
    struct logent *ent;

    ent = log_by_id(id);
    refresh_reg(ent->reg);
    free_reg(ent->reg);
    ent->reg = newreg;
    /*
     * the new region may have moved sufficiently to justify adding/changing
     * the sbs label
     */
    log_label(ent);
    draw_log();
};

/**********************************************************************/
/* add an sbs label to a log */
log_label(ent)
    struct logent *ent;
{
    unsigned  blank;
    XPoint    pt;
    char      msg[12];

#ifdef TRACE_EX
    fprintf(stderr, " in log label \n");
#endif

    if (ent->fnum != curframe())
	return;

    sprintf(msg, "%d", ent->id);
    xv_set(base_win->log_num, PANEL_LABEL_STRING, msg, NULL);

    if (ent->sbs != NULL) {
	pt.x = itod(ent->sbs->x);
	pt.y = itod(ent->sbs->y) - 13;
	XPutImage(display, img_win->d_xid, gc, disp_ximg, pt.x,
		  pt.y, pt.x, pt.y, ent->sbs->metr.width, 15);
	free(ent->sbs);
	ent->sbs = NULL;
    }
    /* in clean_mode, dont number anotation objects */
    if (clean_mode && ent->opcode == ANNOTATE)
	return;

    if (ent->reg == NULL || ent->reg->r_plist == NULL)
	return;
    ent->sbs = (struct strbs *) malloc(sizeof(struct strbs));
    ent->sbs->string = (char *) malloc(10);
    ent->sbs->x = ent->reg->r_plist->cb.left.x;
    ent->sbs->y = ent->reg->r_plist->cb.top.y + 13;
    pt.x = itod(ent->sbs->x);
    pt.y = itod(ent->sbs->y);
    itoa(ent->id, ent->sbs->string);
    XTextExtents(xfs, ent->sbs->string, strlen(ent->sbs->string),
		 &blank, &blank, &blank, &ent->sbs->metr);
    /* write the character and get the backing store */
    XSetForeground(display, gc, standout);
    XDrawString(display, img_win->d_xid, gc, pt.x, pt.y,
		ent->sbs->string, strlen(ent->sbs->string));
}

/****************************************************************/
/*
   routine to add the log to a header for save operations
*/

save_log(head)
    struct header *head;
{
    if (loghead != NULL) {
	add_log(head, loghead);
    }
}

/**********************************************************************/
/* a couple quick routiens for converting small integers to strings.
 * These are stolen outright from K & R, p. 59-60
 */

reverse(s)			/* reverse string s in place */
    char     *s;
{
    int       c, i, j;

    for (i = 0, j = strlen(s) - 1; i < j; i++, j--) {
	c = s[i];
	s[i] = s[j];
	s[j] = c;
    }
}

/********************************************/
itoa(n, s)			/* convert n to characters in s */
    int       n;
    char     *s;
{
    int       i, sign;

    if ((sign = n) < 0)
	n = -n;
    i = 0;
    do {
	s[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);
    if (sign < 0)
	s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}

/*******************************************/
int
get_current_lid()
{
    return (lid - 1);
}
