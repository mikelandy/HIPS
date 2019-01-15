/*
 * text.c -- routine for dealing with annotation text
 *
 */

#include <stdio.h>
#include "ui.h"
#include "common.h"
#include "llist.h"
#include "sm.h"
#include "log.h"
#include "reg.h"
#include "display.h"

static char cbuf[256];
static int nchar = 0;

struct plist *getcpl();

/*********************************************/
draw_text(reg)
    struct region *reg;
{
    unsigned  blank;

    printf(" in draw_text \n");

    reg->r_sbs = (struct strbs *) malloc(sizeof(struct strbs));
    reg->r_sbs->string = NULL;

    if (nchar > 0) {
	reg->r_sbs->string = (char *) malloc(nchar + 1);
	strcpy(reg->r_sbs->string, cbuf);
    }
    reg->r_sbs->x = reg->r_plist->pt.x;
    reg->r_sbs->y = reg->r_plist->pt.y;

    if (nchar > 0) {
	XTextExtents(xfs, reg->r_sbs->string, strlen(reg->r_sbs->string),
		     &blank, &blank, &blank, &reg->r_sbs->metr);

	draw_text_string(reg);
    }
}

/*********************************************/
draw_text_string(reg)
    struct region *reg;
{
    if (reg->r_sbs == NULL || reg->r_plist == NULL)
	return;

    printf("drawing string: %s  at (%d,%d) \n",
	   reg->r_sbs->string, reg->r_sbs->x, reg->r_sbs->y);
#ifdef DEBUG
#endif

    XSetForeground(display, gc, standout);
    XDrawString(display, img_win->d_xid, gc, reg->r_sbs->x, reg->r_sbs->y,
		reg->r_sbs->string, strlen(reg->r_sbs->string));
}

/*****************************************************/

text_reset(reg)
    struct region *reg;
{
    bzero(cbuf, nchar);
    nchar = 0;
    flush_cpl(reg);
}

/*********************************************/
add_char(c)
    char      c;
{
    struct plist *pl;

    if (getrtype() != AN_TEXT)
	return;
    if (getnpoints() < 1 && pl == NULL)
	return;

    if ((pl = getcpl()) == NULL)
	return;

    if (nchar > 255) {
	fprintf(stderr, "character buffer overflow!\n");
	nchar = 0;
	return;
    }
    if (c == '\n' || c == '\r') {
	/* implict eval */
	state_dispatch(EVAL, NULL);
	return;
    }
    if (c == '\b' || c == 0x7f) {
	if (nchar > 0) {
	    XPutImage(display, img_win->d_xid, gc, orig_ximg, pl->cb.top.x,
		      pl->cb.top.y, pl->cb.top.x, pl->cb.top.y,
		      (strlen(cbuf) + 1) * 9, 30);
	    cbuf[--nchar] = 0;
	}
    } else
	cbuf[nchar++] = c;

    XSetForeground(display, gc, standout);
    XDrawString(display, img_win->d_xid, gc, pl->pt.x, pl->pt.y,
		cbuf, strlen(cbuf));
}

/*********************************************/
/* used for loading in text from the log. Works just like a bunch of
   succesive add_char() operations, except that: there is no del or
   backspace processing, newline does not cause a state transition,
   and this is more efficient */
add_string(s)
    char     *s;
{

#ifdef DEBUG
    printf("String:%s\n", s);
#endif
    if (getrtype() != AN_TEXT)
	return;
    if (getnpoints() < 1)
	return;

    strcpy(cbuf, s);
    nchar = strlen(cbuf) + 1;
}

/*********************************************/
set_string(s)
    char     *s;
{
    strcpy(cbuf, s);
    nchar = strlen(s) + 1;

    fprintf(stderr, "setting nchar to %d \n", nchar);
}

/*****************************************************/
char     *
get_string()
{
    return (cbuf);
}
