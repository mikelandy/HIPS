/*
 * conf.c -- connection between analytic functions and rest of GENIAL
 */

#include <stdio.h>
#include "ui.h"
#include "conf.h"
#include "log.h"

static int cfunc = 0;		/* current function */

int 
nofunc()
{
    return 0;
}

extern int anot_init(), anot_eval(), anot_clear(), anot_change();

extern int zoom_init(), zoom_eval(), zoom_clear();

extern int ames_init(), ames_eval(), ames_clear(), 
	ames_reset(), ames_change();

extern int dist_init(), dist_eval(), dist_clear();

extern int trace_init(), trace_eval(), trace_clear(), trace_reset(),
          trace_change();

extern int histo_init(), histo_eval(), histo_clear(), 
	histo_reset(), histo_change();

extern int cmtfile_init(), cmtfile_clear(), cmtfile_reset();

struct fxnsw fxnsw[] =
{
    {trace_init, trace_eval, trace_clear, trace_reset, trace_change},
    {histo_init, histo_eval, histo_clear, histo_reset, histo_change},
    {zoom_init, zoom_eval, zoom_clear, NULL, zoom_eval},
    {dist_init, dist_eval, dist_clear, NULL, dist_eval},
    {ames_init, ames_eval, ames_clear, ames_reset, ames_eval},
    {anot_init, anot_eval, anot_clear, NULL, anot_change},
    {cmtfile_init, NULL, cmtfile_clear, cmtfile_reset, NULL},
};


fxn_init()
{
    if (fxnsw[cfunc].f_init != NULL) {
	if ((*fxnsw[cfunc].f_init) () == -1) {
	    clear_info();
	    lab_info("Error attempting to initialize function!", 1);
	}
    }
#ifdef DEBUG
    printf("fxn_init\n");
#endif
}

fxn_eval()
{
    if (fxnsw[cfunc].f_eval != NULL) {
	if ((*fxnsw[cfunc].f_eval) () == -1) {
	    clear_info();
	    lab_info("Error attempting to eval function!", 1);
	}
	draw_log();
	XFlush(display);
    }
#ifdef DEBUG
    printf("fxn_eval\n");
#endif
}

fxn_change(id)
int id;
{
    if (fxnsw[cfunc].f_change != NULL) {
	if ((*fxnsw[cfunc].f_change) (id) == -1) {
	    clear_info();
	    lab_info("Error attempting to eval function!", 1);
	}
	draw_log();
    }
#ifdef DEBUG
    printf("fxn_change\n");
#endif
}

/* note: fxn_clear routines should not attempt to clear the points in the
   region or the crosses.  leave that to to log_del() */
fxn_clear(log)
struct logent *log;
{
    if (fxnsw[log->opcode].f_clear != NULL) {
	if ((*fxnsw[log->opcode].f_clear) (log->id) == -1) {
	    clear_info();
	    lab_info("Error attempting to clear function!", 1);
	}
	draw_log();
	log->trace = NULL;
	log->hist = NULL;
	log->zoom = NULL;
    }
#ifdef DEBUG
    printf("fxn_clear\n");
#endif
}

fxn_reset()
{
    if (fxnsw[cfunc].f_reset != NULL) {
	if ((*fxnsw[cfunc].f_reset) () == -1) {
	    clear_info();
	    lab_info("Error attempting to initialize function!", 1);
	}
    }
#ifdef DEBUG
    printf("fxn_reset\n");
#endif
}

fxn_select(fid)
    int       fid;
{
    cfunc = fid;
    curfunc->opcode = fid;

#ifdef DEBUG
    printf("fxn_select:%d\n", fid);
#endif
}
