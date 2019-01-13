/*
 * regdom.c -- routines for setting / geting the region domains
 *
 */

#include <stdio.h>
#include "display.h"
#include "ui.h"
#include "reg.h"

static char *reg_strtab[MAXREG] =
{
 "None", "line", "Spline", "C-Spline", "Polygon", "Box", "Dbl Line",
    "All Image", "Text", "Arrow"};

/*
 * reg_map[] is a map of the region types presently in the menu, for use by
 * the menu proc.  So, for example, if the menu notify procedure for the
 * region type is called with a value of 2, reg_map[value] will give the
 * corresponding region type.
 *
 */

static int reg_map[MAXREG];

reg_setdom(grp)
    int       grp;
{
    int       value;

    switch (grp) {
    case LINEAR:
	rdom_linear();
	break;
    case AREA:
	rdom_area();
	break;
    case ANOT:
	rdom_anot();
	break;
    case TRACE:
	rdom_trace();
	break;
    case NONE:
	if (getrtype() == NOREG) {
	    rdom_none();
	    return;
	} else {
	    xv_set(base_win->reg_type,
		   PANEL_CHOICE_STRINGS,
		   reg_strtab[getrtype()],
		   NULL,
		   NULL);
	    reg_map[0] = getrtype();
	}
	break;
    }
    value = (int) xv_get(base_win->reg_type, PANEL_VALUE, NULL);
    setrtype(reg_map[value]);
}

/*
 * Notify callback function for `reg_type'.
 */
void
reg_type_proc(item, value, event)
    Panel_item item;
    int       value;
    Event    *event;
{
#ifdef DEBUG
    fprintf(stderr, "genial: reg_type_proc: value: %u\n", value);
#endif
    setrtype(reg_map[value]);

    line_setdom(reg_map[value]);
}

rdom_linear()
{
    reg_map[0] = LINE;
    reg_map[1] = SPLINE;
    xv_set(base_win->reg_type,
	   PANEL_CHOICE_STRINGS,
	   reg_strtab[LINE],
	   reg_strtab[SPLINE],
	   NULL,
	   NULL);
}

rdom_area()
{
    reg_map[0] = BOX;
    reg_map[1] = ENTIRE_IMAGE;
    reg_map[2] = POLYGON;
    reg_map[3] = CLSPLINE;
    xv_set(base_win->reg_type,
	   PANEL_CHOICE_STRINGS,
	   reg_strtab[BOX],
	   reg_strtab[ENTIRE_IMAGE],
	   reg_strtab[POLYGON],
	   reg_strtab[CLSPLINE],
	   NULL,
	   NULL);
}

rdom_anot()
{
    reg_map[0] = AN_TEXT;
    reg_map[1] = AN_VEC;
    reg_map[2] = LINE;
    reg_map[3] = SPLINE;
    reg_map[4] = BOX;
    xv_set(base_win->reg_type,
	   PANEL_CHOICE_STRINGS,
	   reg_strtab[AN_TEXT],
	   reg_strtab[AN_VEC],
	   reg_strtab[LINE],
	   reg_strtab[SPLINE],
	   reg_strtab[BOX],
	   NULL,
	   NULL);
}

rdom_trace()
{
    reg_map[0] = LINE;
    reg_map[1] = SPLINE;
    xv_set(base_win->reg_type,
	   PANEL_CHOICE_STRINGS,
	   reg_strtab[LINE],
	   reg_strtab[SPLINE],
	   NULL,
	   NULL);
}

rdom_none()
{
    xv_set(base_win->reg_type,
	   PANEL_CHOICE_STRINGS,
	   reg_strtab[NOREG], NULL,
	   NULL);
}
