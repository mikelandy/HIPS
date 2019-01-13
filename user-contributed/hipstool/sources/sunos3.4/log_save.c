/* log_save.c
 * Max Rible
 *
 * Save log entries to a file stream pointer.  "end" is the character
 * with which to end an entry:  '\n' for individual logs, '\177' for
 * HIPS headers.
 */

#include "hipstool.h"

static int save_command(), save_primit_list(), save_primit();

#define Fprintf size += fprintf

int
save_log(output, end)
     FILE *output;
     char end;
{
    Log current;
    int num_chars = 0;

    for(current = actions; current != NULLOG; current = current->next)
	num_chars += save_command(current->command, output, end);
    return(num_chars);
}

/* Save a HIPStool language format command to output.  Returns # bytes written.
 */
static int
save_command(command, output, end)
     Command *command;
     FILE *output;
     char end;
{
    int size = 0;

    Fprintf(output, "((\"%s\" %d) ", functions[command->breed].name, 
	    command->breed);
    size += save_primit_list(command->func.primit, command->func.len, output);
    size += save_primit_list(command->anno.primit, command->anno.len, output);

    if(command->comment != NULL) 
	Fprintf(output, "\"%s\"", command->comment);
    else
	Fprintf(output, "()");

#ifdef MULTO_FRAMES
    Fprintf(output, " %d", command->frame);
#endif

    Fprintf(output, ")%c", end);

    return(size);
}

/* Save a list or array of Primits to output.  Returns # bytes written.
 */
static int
save_primit_list(primit, length, output)
     Primit *primit;
     int length;
     FILE *output;
{
    Primit *tmp;
    int size = 0, i;

    if(primit == NULL) {
	Fprintf(output, "(0 ) ");
	return(size);
    }

    Fprintf(output, "(%d ", length);
    if(primit->listp) {
	for(tmp = primit; tmp != NULL; tmp = tmp->next)
	    size += save_primit(tmp, output);
    } else {
	for(i = 0; i < length; i++)
	    size += save_primit(primit + i, output);
    }
    Fprintf(output, ") ");

    return(size);
}

/* Save a single Primit to output.  Returns # bytes written. 
 * Trace and cross information are not preserved.
 */
static int
save_primit(primit, output)
     Primit *primit;
     FILE *output;
{
    int size = 0, i;
    Point tmp;

    Fprintf(output, "((\"%s\" %d) ", priminfo[primit->breed].name,
	    primit->breed);

    if(priminfo[primit->breed].loc_size == VARIES) {
	Fprintf(output, "%d ", primit->len);
	for(tmp = primit->loc.list; tmp != NULL; tmp = tmp->next) 
	    Fprintf(output, "(%d,%d) ", tmp->i.x, tmp->i.y);
    } else
	for(i = 0; i < primit->len; i++)
	    Fprintf(output, "(%d,%d) ", primit->loc.coords[i][0],
		    primit->loc.coords[i][1]);

    if(priminfo[primit->breed].data_type == DATA_TEXT)
	Fprintf(output, "\"%s\"", primit->data.txt);

    Fprintf(output, ") ");

    return(size);
}
