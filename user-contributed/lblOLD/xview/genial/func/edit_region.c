
/*
 *  edit_region.c            -Brian Tierney,   LBL
 */

#include "ui.h"
#include "common.h"
#include "llist.h"
#include "display.h"
#include "reg.h"
#include "log.h"
#include "sm.h"

#define OFF 0
#define EDIT 1
#define DELETE 2
#define COPY 3
#define FRAME_COPY 4
#define HIDE 5

static int curr_edit_func = OFF;
static int edit_log_id = 0;

void      edit_log_num_proc();
struct logent *log_by_id();

#define DEBUG

void
edit_region_proc(item, value, event)
    Panel_item item;
    int       value;
    Event    *event;
{
    fprintf(stderr, "in edit_region_proc: val = %d \n", value);

    curr_edit_func = value;

    /* make sure that there is at least 1 logged item */
    if (loghead == NULL || loghead->next == NULL) {
	xv_set(base_win->edit_mode, PANEL_VALUE, 0, NULL);
	value = 0;
    }
    clear_info();
    switch (value) {
    case 0:
#ifdef DEBUG
	fputs("genial: edit_region: off\n", stderr);
#endif
	xv_set(base_win->edit_log_num, PANEL_INACTIVE, TRUE, NULL);
	xv_set(base_win->edit_ok, PANEL_INACTIVE, TRUE, NULL);

	/* reset to previous state */
	if ((curfunc = log_by_id(get_current_lid())) == NULL)
	    return;

	set_plist_globals(curfunc->reg);

	fxn_select(curfunc->opcode);
	xv_set(base_win->func_sel, PANEL_VALUE, curfunc->opcode, NULL);
	xv_set(base_win->message9, PANEL_LABEL_STRING, "   ", NULL);
	fxn_init();
	new_state(IMG_LOADED);
	break;

    case 1:
	lab_info("move points with left mouse button", 1);
	lab_info("Hit <eval> when done moving points", 2);
	lab_info("reset edit-mode to Off to select next region", 3);

	edit_setup(loghead->id);
	break;

    case 2:			/* delete */
	lab_info("select object to delete by log number", 1);
	lab_info("Hit <OK> when done", 2);
	lab_info("reset edit-mode to Off to select next region", 3);

	edit_setup(loghead->id);/* wait till OK button is pushed before doing
				 * anything */
	break;

    case 3:
	lab_info("not yet implemented", 1);
	lab_info("reset edit-mode to Off to select next region", 3);

	fputs("genial: edit_region: copy (not yet implemented)\n", stderr);
	edit_setup(loghead->id);
	break;
    case 4:
	lab_info("not yet implemented", 1);
	lab_info("reset edit-mode to Off to select next region", 3);

	fputs("genial: edit_region: frame copy (not yet implemented)\n", stderr);
	edit_setup(loghead->id);
	break;
    case 5:
	lab_info("not yet implemented", 1);
	lab_info("reset edit-mode to Off to select next region", 3);

	fputs("genial: edit_region: hide (not yet implemented)\n", stderr);
	edit_setup(loghead->id);
	break;
    }
}

/***************************************************************/
edit_setup(id)
    int       id;
{
    fprintf(stderr, "in edit setup: id = %d \n", id);

    new_state(REG_EDIT);

    xv_set(base_win->edit_ok, PANEL_INACTIVE, FALSE, NULL);
    xv_set(base_win->edit_log_num, PANEL_INACTIVE, FALSE, NULL);

    /* set default log item */
    xv_set(base_win->edit_log_num, PANEL_VALUE, 0, NULL);
    edit_log_num_proc(NULL, 0, NULL);

    set_log_num_choices();
}

/***************************************************************/
/*
 * Notify callback function for `edit_log_num'.
  */
void
edit_log_num_proc(item, value, event)
    Panel_item item;
    int       value;
    Event    *event;
{
    struct logent *func;

#ifdef DEBUG
    fprintf(stderr, "edit setup: value %d \n", value);
#endif

    edit_log_id = atoi((char *) xv_get(base_win->edit_log_num,
				       PANEL_CHOICE_STRING, value, NULL));
#ifdef DEBUG
    fprintf(stderr, "editting log id # %d \n", edit_log_id);
#endif

    if ((func = log_by_id(edit_log_id)) == NULL) {
	fprintf(stderr, "Log Number not found \n");
	XBell(display, 0);
	return;
    }
    curfunc = func;
    set_plist_globals(curfunc->reg);
    fxn_select(curfunc->opcode);
    xv_set(base_win->func_sel, PANEL_VALUE, curfunc->opcode, NULL);
    xv_set(base_win->message9, PANEL_LABEL_STRING,
	   func_names[curfunc->opcode], NULL);
}

/***************************************************************/
set_log_num_choices()
{
    int       i = 0, log_cnt = 0;
    char    **log_id_buf, **alloc_2d_char_array();
    struct logent *func;

    /* first count the number of entries */
    func = loghead;
    while (func->next != NULL) {
	log_cnt++;
	func = func->next;
    }

    if (log_cnt <= 0)
	return;

    log_id_buf = alloc_2d_char_array(log_cnt, 4);
    func = loghead;
    while (func->next != NULL) {
	sprintf(log_id_buf[i], "%d", func->id);
	func = func->next;
	i++;
    }

#ifdef DEBUG
    printf(" setting number of log item choices: %d \n", log_cnt);
#endif

    xv_set(base_win->edit_log_num, PANEL_CHOICE_STRINGS, "1", NULL, NULL);

    for (i = 0; i < log_cnt; i++)
	xv_set(base_win->edit_log_num,
	       PANEL_CHOICE_STRING, i, log_id_buf[i], NULL);

    free(log_id_buf);
}

/***************************************************************/
void
do_edit_proc()
{				/* edit OK button pushed */

    if (curfunc == NULL)
	return;

    if (curr_edit_func == EDIT) {
	do_edit_eval();		/* same as eval button */
    }
    if (curr_edit_func == DELETE) {
	fxn_clear(curfunc);
	log_del(edit_log_id);
	set_log_num_choices();
	edit_region_proc(NULL, 2, NULL);	/* reset stuff */
    }
}

#ifdef NEED_LATER
/***************************************************************/
/* pfind_all() -- search all regions for an entry in a plist
 *  within 20 pixels' distance from the  given point
 */

struct plist
         *
pfind_all(dpt)
    XPoint    dpt;
{
    int       sm = 20;
    int       dist;
    struct plist *trp;
    struct logent *cfunc;
    struct plist *list_head;

    cfunc = logtail;
    while (cfunc) {
	list_head = cfunc->reg->r_plist[0];

	for (trp = list_head; trp != NULL; trp = trp->next) {
	    dist = (int) irint(distance(trp->pt, dpt));
#ifdef DEBUG
	    printf("distance to point:%d\n", dist);
#endif
	    if (dist <= sm) {
		curfunc = cfunc;/* set global curfunc pointer */
		setnpoints(curfunc->reg->r_plen + 1);
		cplhead = curfunc->reg->r_plist[0];
		setpvreg(curfunc->reg);
		return trp;
	    }
	}
	cfunc = cfunc->prev;
    }
    return NULL;
}


#endif

/**********************************/
char    **
alloc_2d_char_array(nx, ny)
    int       nx, ny;
{
    char    **array;
    register int i;

#define Calloc(x,y) (y *)calloc((unsigned)(x), sizeof(y))

    /* allocate 2-d array for input image data */
    /* allocate array of pointers */
    if ((array = Calloc(nx, char *)) == NULL)
	perror("calloc error: array ");

    /* allocate array for data */
    if ((array[0] = Calloc(nx * ny, char)) == NULL)
	perror("calloc error: array ");

    /* initialize pointer arrays */
    for (i = 1; i < nx; i++)
	array[i] = array[0] + (ny * i);

    return (array);
}
