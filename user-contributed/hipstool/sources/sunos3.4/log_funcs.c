/* log_funcs.c
 * Max Rible
 *
 * Logging functions.
 */

#include "hipstool.h"

Log actions = NULLOG;
int action_number = 1;
static void display_action(), display_command(), reselect();
char *header_comment = NULL;

void
log_current_action()
{
    Log tmp;

    if(cur_func == NULL) return;

    save_menu_funcs[SAVE_LOGGED_IMAGE].active = 1;
    save_menu_funcs[SAVE_LOG_FILE].active = 1;
    save_menu_funcs[SAVE_OVERLAID_IMAGE].active = 1;
    update_save_menu();

    if(current_action_num != -1) {
	warp_action(current_action_num, WARP_MODIFY, ON);
	return;
    }

    tmp = Calloc(1, struct logentry);

    tmp->command = (Command *) copy(TYPE_COMMAND, cur_func);
    tmp->active = 1;
    tmp->action_num = action_number++;
    tmp->breed = cur_func->breed;
#ifdef MULTO_FRAMES
    tmp->frame = tmp->command->frame = base.frames.current;
#endif
    tmp->command->comment = 
	(*functions[cur_func->breed].comment)(getstring(io.comment));

    tmp->next = actions;
    actions = tmp;
}

/* Run all the operations to display everything (boxes, lines,
 * and so on.
 */
void

display_logged_actions(mode, value)
     int mode;
     int value;
{
    Log current;

    for(current = actions; current != NULLOG; current = current->next)
	if(current->frame == base.frames.current)
	    display_action(current, mode, value);
}

static void
display_action(event, t, v)
     Log event;
     int t;
     int v;
{
    Primit *primit;
    int x, y;
    char number[10];

    if(v == ON) {
	primit = event->command->func.primit;
	if(event->active)
	    sprintf(number, "%d", event->action_num);
	else
	    sprintf(number, "*%d", event->action_num);
	if(priminfo[primit->breed].loc_size == VARIES) {
	    x = primit->loc.list->i.x;
	    y = primit->loc.list->i.y;
	} else {
	    x = primit->loc.coords[0][0];
	    y = primit->loc.coords[0][1];
	}
	text(&base.winfo, x + 10, y + 20, number, STANDOUT);
    } else if(v == TEST)
	if(!event->active) return;

    display_command(event->command, t);
}

int
warp_action(num, quality, status)
     int num;
     int quality;
     int status;
{
    Log tmp, kill;
    char string[100];
    int active;

    if(actions == NULLOG) return(OFF);

    switch(quality) {
    case WARP_ABLE:
	for(tmp = actions; tmp != NULLOG; tmp = tmp->next) 
	    if(tmp->action_num == num) {
		active = tmp->active;
		switch(status) {
		case ON:    tmp->active = ON; break;
		case OFF:   tmp->active = OFF; break;
		case TEST:  break;
		default:    active = OFF; break;
		}
		return(active);
	    }
	break;
    case WARP_DELETE:
	if(actions == NULLOG) return(OFF);
	if(actions->action_num == num) {
	    tmp = actions;
	    actions = actions->next;
	    sprintf(string, "Action %d deleted.", tmp->action_num);
	    Update_info(string);
	    destroy(TYPE_LOGENTRY, &tmp);
	    return(ON);
	}
	for(tmp = actions; tmp != NULLOG; tmp = tmp->next)
	    if(tmp->next->action_num == num) {
		kill = tmp->next;
		tmp->next = tmp->next->next;
		destroy(TYPE_LOGENTRY, &kill);
		sprintf(string, "Action %d deleted.", num);
		Update_info(string);
		return(ON);
	    }
	sprintf(string, "Couldn't delete action %d!", num);
	Update_info(string);
	return(OFF);
	break;
    case WARP_SELECT:
	for(tmp = actions; tmp != NULLOG; tmp = tmp->next)
	    if(tmp->action_num == num) {
		display_action(tmp, DRAW_IN_PIXWIN, OFF);
		panel_set_value(io.inputs[INPUT_SELECT], 
				functions[tmp->breed].name);
		current_action_num = num;
		if(tmp->command->comment)
		    putstring(io.comment, tmp->command->comment);
		complete = 1;
		cur_func = (Command *) copy(TYPE_COMMAND, tmp->command);
		reselect(tmp->command->func.primit);
		Update_info(" ");
/* REDO */
/*		Prinfo1(functions[cur_func->breed].start.info[0]);
		Prinfo2(functions[cur_func->breed].start.info[1]);*/
		(*functions[cur_func->breed].setup)(cur_func, ON);
		return(ON);
	    }
	sprintf(string, "Couldn't select action %d!", num);
	Update_info(string);
	return(OFF);
	break;
    case WARP_MODIFY:
	for(tmp = actions; tmp != NULLOG; tmp = tmp->next)
	    if(tmp->action_num == num) {
		destroy(TYPE_COMMAND, &(tmp->command));
		tmp->command = (Command *) copy(TYPE_COMMAND, cur_func);
		if(tmp->command->comment == NULL) {
		    if(getstring(io.comment) != NULL)
			tmp->command->comment = Strdup(getstring(io.comment));
		} else {
		    if(getstring(io.comment) != NULL &&
		       strcmp(tmp->command->comment, getstring(io.comment)) != 0) {
			free(tmp->command->comment);
			tmp->command->comment = Strdup(getstring(io.comment));
		    }
		}
		return(ON);
	    }
	sprintf(string, "Couldn't modify action %d!", num);
	Update_info(string);
	return(OFF);
	break;
    }
    return(ON);
}

static void
reselect(primit)
     Primit *primit;
{
    Point tmp;

    if(priminfo[primit->breed].loc_size == VARIES)
	for(tmp = primit->loc.list; tmp != NULL; tmp = tmp->next)
	    do_cross(&(tmp->i), ON);
}

static void
display_command(command, t)
     Command *command;
     int t;
{
    int i;

    for(i = 0; i < command->func.len; i++)
	(*priminfo[command->func.primit[i].breed].display)
	    (command->func.primit + i, t);
}
