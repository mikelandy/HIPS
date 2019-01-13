/* log_load.c
 * Max Rible
 *
 * Load log entries from a file stream pointer.
 */

#include "hipstool.h"

static Command *load_command();
static Primit *load_primit_list(), *load_primit();

void
load_log(input)
     FILE *input;
{
    Log newentry;

    save_menu_funcs[SAVE_LOG_FILE].active = 1;
    save_menu_funcs[SAVE_LOGGED_IMAGE].active = 1;
    save_menu_funcs[SAVE_OVERLAID_IMAGE].active = 1;
    update_save_menu();

    while(!feof(input)) {
	newentry = Calloc(1, struct logentry);
	newentry->command = load_command(input);
	if(newentry->command == NULL) { 
	    Cfree(newentry, 1, struct logentry); 
	    return;
	}
	newentry->breed = newentry->command->breed;
	newentry->action_num = action_number++;
#ifdef MULTO_FRAMES
	newentry->frame = newentry->command->frame;
#endif
	newentry->active = 1;
	newentry->next = actions;
	actions = newentry;
    }
}

static Command *
load_command(input)
     FILE *input;
{
    int x;
    char comment[100];
    Command *new;

    if(fscanf(input, "((\"%*[^\"]\" %d) ", &x) != 1)
	return(NULL);

    new = Calloc(1, Command);

    new->breed = x;
    new->func.primit = load_primit_list(input, &(new->func.len), 0);
    new->anno.primit = load_primit_list(input, &(new->anno.len), 1);

    if(fgetc(input) == '\"') {
	fscanf(input, "%[^\"]\"", comment);
	new->comment = Strdup(comment);
    } else
	fscanf(input, ")");

#ifdef MULTO_FRAMES
    fscanf(input, " %d", &(new->frame));
#endif

    fscanf(input, ")%*c");

    return(new);
}

static Primit *
load_primit_list(input, length, listp)
     FILE *input;
     int *length, listp;
{
    Primit *new, *tmp;
    int len, i;

    fscanf(input, "(%d ", &len);

    if(len == 0) {
	fscanf(input, ") ");
	return(NULL);
    }

    if(listp) {
	new = load_primit(input, NULL);
	for(tmp = new, i = 1; i < len; i++, tmp = tmp->next)
	    tmp->next = load_primit(input, NULL);
    } else {
	new = Calloc(len, Primit);
	for(i = 0; i < len; i++)
	    (void) load_primit(input, new + i);
    }
    fscanf(input, ") ");

    *length = len;

    return(new);
}

static Primit *
load_primit(input, prim)
     FILE *input;
     Primit *prim;
{
    Primit *new;
    int i;

    if(prim == NULL)
	new = Calloc(1, Primit);
    else
	new = prim;

    fscanf(input, "((\"%*[^\"]\" %d) ", &(new->breed));

    if(priminfo[new->breed].loc_size == VARIES) {
	fscanf(input, "%d ", &(new->len));
	new->loc.list = Calloc(new->len, struct point);
	for(i = 0; i < new->len; i++) {
	    fscanf(input, "(%d,%d) ", &(new->loc.list[i].i.x),
		   &(new->loc.list[i].i.y));
	    new->loc.list[i].next = new->loc.list + i + 1;
	    new->loc.list[i].prev = new->loc.list + i - 1;
	}
	new->loc.list[0].prev = new->loc.list + new->len - 1;
	new->loc.list[new->len - 1].next = NULL;
    } else {
	new->len = priminfo[new->breed].loc_size;
	new->loc.coords = (int (*)[2]) calloc((unsigned)(2*new->len), 
					      sizeof(int));
	for(i = 0; i < new->len; i++)
	    fscanf(input, "(%d,%d) ", &(new->loc.coords[i][0]),
		   &(new->loc.coords[i][1]));
    }

    /* Load coordinates in */

    fscanf(input, ") ");
    return(new);
}
