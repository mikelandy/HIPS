/*
 * anot.c -- routines to allow for textual annotation of an image
 *
 */

#include "display.h"
#include "ui.h"
#include "log.h"
#include "reg.h"

anot_init()
{
    reg_setdom(ANOT);

    clear_info();
    lab_info("For text, click on a position and begin typing.", 1);
    lab_info("Otherwise, select an annotation region.", 2);
    lab_info("Hit return (for text) or click <eval> when finished", 3);
    return 0;
}

anot_eval()
{
    text_reset(curfunc->reg);

    clear_info();
    return 0;
}

anot_clear()
{
    clear_info();

    return 0;
}

anot_change()
{
    return 0;
}
