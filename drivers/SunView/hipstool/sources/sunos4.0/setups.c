/* setups.c
 * Max Rible
 *
 * Setup procedures for HIPStool functions.
 */

#include "hipstool.h"

static void line_1_done(), line_2_done();

/* ARGSUSED */
void
no_setup(command, value)
     Command *command;
     enum truth value;
{
}

/* ARGSUSED */
void
box_setup(command, value)
     Command *command;
     enum truth value;
{
    if(value == OFF) {
	save_menu_funcs[SAVE_BOX_SUBIMAGE].active = 0;
	update_save_menu();
    }
}

/* ARGSUSED */
void
edit_setup(command, value)
     Command *command;
     enum truth value;
{
    if(value == ON)
	display_logged_actions(DRAW_IN_PIXWIN, ON);
}

/* ARGSUSED */
void
angle_setup(command, value)
     Command *command;
     enum truth value;
{
    static Panel_item finis[2];

    if(line_drawing_mode == LEAST_SQUARES_FIT) {
	if(value == ON) {
	    finis[0] = 
		panel_create_item(io.panel, PANEL_BUTTON,
				  PANEL_LABEL_IMAGE, 
				  panel_button_image(io.panel, "1",
						     2, io.font),
				  PANEL_NOTIFY_PROC, line_1_done,
				  PANEL_ITEM_X, ATTR_COL(45),
				  PANEL_ITEM_Y, ATTR_ROW(10),
				  0);
			      
	    finis[1] = 
		panel_create_item(io.panel, PANEL_BUTTON,
				  PANEL_LABEL_IMAGE, 
				  panel_button_image(io.panel, "2",
						     2, io.font),
				  PANEL_NOTIFY_PROC, line_2_done,
				  PANEL_ITEM_X, ATTR_COL(50),
				  PANEL_ITEM_Y, ATTR_ROW(10),
				  0);
	    panel_paint(finis[0], PANEL_CLEAR);
	    panel_paint(finis[1], PANEL_CLEAR);
	} else {		/* value == OFF */
	    panel_destroy_item(finis[0]);
	    panel_destroy_item(finis[1]);
	    panel_paint(io.panel, PANEL_CLEAR);
	}
    }
}

static void
line_1_done()
{
    if(cur_func->func.primit[1].loc.list == NULL) {
	cur_func->func.primit[1].loc.list = cur_func->func.primit[0].loc.list;
	cur_func->func.primit[0].loc.list = NULL;
    }

    if(cur_func->func.primit[1].loc.coords[0][0] != -1)
	undoline2d(&(cur_func->func.primit[1].data.trace));

    fit_line(cur_func->func.primit[1].loc.list,
	     &(cur_func->func.primit[1].data.trace),
	     0, ON, cur_func->func.primit[1].loc.coords);
}

static void
line_2_done()
{
    if(cur_func->func.primit[0].loc.coords[0][0] != -1)
	undoline2d(&(cur_func->func.primit[0].data.trace));

    fit_line(cur_func->func.primit[0].loc.list,
	     &(cur_func->func.primit[0].data.trace),
	     0, ON, cur_func->func.primit[0].loc.coords);
}

void
distance_setup(command, value)
     Command *command;
     enum truth value;
{
    Primit *primit = command->func.primit;

    if(complete == 0 || value != ON) return;

    do_cross(&(primit[0].data.cross), OFF);
    do_cross(&(primit[1].data.cross), OFF);
}
