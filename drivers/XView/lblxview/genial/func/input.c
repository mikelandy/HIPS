/*
 * input.c -- input routines for GENIAL.  Intimately tied to state_mach.c.
 *
 * This code module maintains dp_stack, a stack of functions which parse a
 * "stream" of input tokens.  Each input token is passed through each function
 * on the stack in turn.  Tokens reach this module through state_dispatch().
 * Functions are added or removed from the queue by dp_push() and
 * dp_del().
 *
 */

#include <stdio.h>
#include "display.h"
#include "ui.h"
#include "sm.h"
#include "log.h"
#include "reg.h"

#define MAXFUNCS 6

static int (*dp_stack[MAXFUNCS]) ();

static int dp_size = 0;

/* dp_level is used to maintain what level we are at so that dispatch_next()
   works appropriately.  */

static int dp_level;

dp_push(func)
    int       (*func) ();
{
    if (dp_size == MAXFUNCS) {
	fprintf(stderr, " dp_push: MAXFUNCS exceeded!! \n");
	return -1;
    } else {
	dp_stack[dp_size++] = func;
    }
    return 1;
}

dp_del(func)
    int       (*func) ();
{
    /*
     * for now, assume that the func argument refers to the top function on
     * the stack and just pop it off
     */
    if (dp_size > 0)
	dp_stack[--dp_size] = NULL;
    else
	fprintf(stderr, " dp_del: size < 0!! \n");
}

/* state_dispatch() is the single entry point for input */
int
state_dispatch(token, arg)
    int       token;		/* token as defined in sm.h */
    caddr_t  *arg;		/* pointer to an optional argument */
{
    /* set dp_level to the top function on the stack */
    dp_level = dp_size;

    return dispatch_next(token, arg);
}

/* dispatch_next():  call next function on the stack.  Keep track of where we
 are in the stack by use of dp_level */
int
dispatch_next(token, arg)
    int       token;		/* token as defined in sm.h */
    caddr_t  *arg;		/* pointer to an optional argument */
{
    if (dp_level < 0) {
	fprintf(stderr, " dispatch_next: level < 0!! \n");
	return -1;
    } else {
	dp_level--;
	return (*dp_stack[dp_level]) (token, arg);
    }
}
