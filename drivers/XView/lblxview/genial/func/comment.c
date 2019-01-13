/*
 * comment.c -- functions handle comment window         -Brian Tierney, LBL
 */

#include <stdio.h>
#include "ui.h"
#include "display.h"
#include "common.h"
#include "log.h"
#include "reg.h"
#include "comment_ui.h"

/****************************************************************/
cmtfile_init()
{
    int       cmtfile_save();

    setrtype(NOREG);
    reg_setdom(NONE);

    lab_info("Enter comments in the comment window", 1);
    lab_info("Hit <eval> when finished", 2);

    cmtfile_load();

    xv_set(comment_win->comm_win, FRAME_CMD_PUSHPIN_IN, TRUE, NULL);
    xv_set(comment_win->comm_win, XV_SHOW, TRUE, FRAME_CLOSED, FALSE, NULL);

    return 0;
}

/****************************************************************/
cmtfile_reset()
{
#ifdef DEBUG
    printf("comment reset \n");
#endif

    cmtfile_save();

    /* close comment window */
    xv_set(comment_win->comm_win, FRAME_CMD_PUSHPIN_IN, FALSE, NULL);
    xv_set(comment_win->comm_win, XV_SHOW, FALSE, NULL);
}

/****************************************************************/
cmtfile_clear()
{
#ifdef DEBUG
    printf("comment clear \n");
#endif

    textsw_erase(comment_win->textpane1, 0, TEXTSW_INFINITY);
}

/****************************************************************/

cmtfile_load()
{
    textsw_delete(comment_win->textpane1, 0, TEXTSW_INFINITY);

    xv_set(comment_win->textpane1, TEXTSW_INSERTION_POINT, 0, NULL);
    textsw_insert(comment_win->textpane1, orig_img->comment,
		  strlen(orig_img->comment));
}

/****************************************************************/
cmtfile_save()
{
    char     *comment_buf;
    int       buf_size;

    buf_size = (int) xv_get(comment_win->textpane1, TEXTSW_LENGTH, NULL);
#ifdef DEBUG
    printf(" in cmtfile_save. \n");
    printf(" textsw buffer size will be: %d \n", buf_size);
#endif

    comment_buf = calloc(buf_size + 1, 1);

    xv_get(comment_win->textpane1, TEXTSW_CONTENTS, 0,
	   comment_buf, buf_size, NULL);
    comment_buf[buf_size] = '\0';

    orig_img->comment = comment_buf;

    return 0;
}
