/*
 * linedom.c -- routines for setting / geting the line drawing mode domains
 *
 */

#include <stdio.h>
#include "display.h"
#include "ui.h"
#include "reg.h"

static char *line_strtab[3] =
{ "Click and Click", "Click and Drag", "Least Squares Fit"};


line_setdom(reg)
    int       reg;
{
    switch (reg) {
    case LINE:
	set_line_types();
	break;
    default:
	set_other_types();
	break;
    }
    line_mode = CLICK;
}

set_line_types()
{
    xv_set(base_win->lmode,
	   PANEL_CHOICE_STRINGS,
	   line_strtab[CLICK],
#ifdef FIXED
	   line_strtab[DRAG],
	   line_strtab[LSQ],
#endif
	   NULL,
	   NULL);
}

set_other_types()
{
    xv_set(base_win->lmode,
	   PANEL_CHOICE_STRINGS,
	   line_strtab[CLICK],
	   NULL,
	   NULL);
}
