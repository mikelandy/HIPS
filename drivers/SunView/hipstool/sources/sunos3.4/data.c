/* data.c
 * Max Rible
 * 
 * Manipulation of complex forms of data:  copying and destroying.
 * Much easier than doing it all by hand.
 */

#include "hipstool.h"

/* Syntax:  copy(TYPE_FOO, (foo *arg)) returns a copy of foo. */
caddr_t
copy(va_alist)
     va_dcl
{
#define Return(x) va_end(index); return((caddr_t)(x));
#define Dupstr(a,b) if((b) != NULL) { (a) = Strdup(b); }
    int type;
    va_list index;
    int i, j, size = VARIES;
    union {
	Command *command;
	Primit *primit;
	Point list;
	struct header *hipshead;
	struct logentry *log;
    } arg, new, tmp;

    va_start(index);
    type = va_arg(index, int);

    switch(type) {
    case TYPE_COMMAND:
	arg.command = va_arg(index, Command *);
	new.command = Calloc(1, Command);
	if(arg.command->func.len > 0)
	    new.command->func.primit = 
		(Primit *) copy(TYPE_PRIMITLIST, arg.command->func.primit, 
		     arg.command->func.len);
	new.command->func.len = arg.command->func.len;
	if(arg.command->anno.len > 0)
	    new.command->anno.primit = 
		(Primit *) copy(TYPE_PRIMITLIST, arg.command->anno.primit);
	new.command->anno.len = arg.command->anno.len;
	Dupstr(new.command->comment, arg.command->comment);
	new.command->breed = arg.command->breed;
	new.command->frame = arg.command->frame;
	Return(new.command);
    case TYPE_PRIMIT:
	arg.primit = va_arg(index, Primit *);
	new.primit = palloc(arg.primit->breed);
	if((size = priminfo[arg.primit->breed].loc_size) == VARIES)
	    new.primit->loc.list = (Point) copy(TYPE_POINTLIST, 
						arg.primit->loc.list);
	else
	    for(i = 0; i < size; i++) {
		new.primit->loc.coords[i][0] = arg.primit->loc.coords[i][0];
		new.primit->loc.coords[i][1] = arg.primit->loc.coords[i][1];
	    }
	new.primit->len = arg.primit->len;
	/* In the future we can add copying of auxiliary data. */
	Return(new.primit);
    case TYPE_POINTLIST:
	arg.list = va_arg(index, Point);
	new.list = Calloc(1, struct point);
	new.list->i.x = arg.list->i.x;
	new.list->i.y = arg.list->i.y;
	for(tmp.list = new.list, arg.list = arg.list->next;
	    arg.list != NULL;
	    tmp.list = tmp.list->next, arg.list = arg.list->next) {
	    tmp.list->next = Calloc(1, struct point);
	    tmp.list->next->i.x = arg.list->i.x;
	    tmp.list->next->i.y = arg.list->i.y;
	    tmp.list->next->prev = tmp.list;
	}
	new.list->prev = tmp.list;
	Return(new.list);
    case TYPE_PRIMITLIST:
	arg.primit = va_arg(index, Primit *);
	if(!arg.primit->listp) size = va_arg(index, int);
	if(size != VARIES) {
	    new.primit = palloc(arg.primit->breed, size);
	    for(i = 0; i < size; i++) {
		if(priminfo[arg.primit[i].breed].loc_size == VARIES) {
		    new.primit[i].loc.list =
			(Point) copy(TYPE_POINTLIST, arg.primit[i].loc.list);
		} else {
		    for(j = 0; j < arg.primit[i].len; j++) {
			new.primit[i].loc.coords[j][0] = 
			    arg.primit[i].loc.coords[j][0];
			new.primit[i].loc.coords[j][1] = 
			    arg.primit[i].loc.coords[j][1];
		    }
		}
		new.primit[i].len = arg.primit[i].len;
		/* Copy auxiliary data? */
	    }
	} else {		/* list of primitives */
	    new.primit = (Primit *) copy(TYPE_PRIMIT, arg.primit);
	    for(tmp.primit = new.primit, arg.primit = arg.primit->next;
		arg.primit != NULL;
		tmp.primit=tmp.primit->next,arg.primit=arg.primit->next,i++) {
		tmp.primit->next = (Primit *) copy(TYPE_PRIMIT, arg.primit);
		tmp.primit->listp = 1;
	    }
	}
	Return(new.primit);
    case TYPE_HIPSHEAD:
	arg.hipshead = va_arg(index, struct header *);
	new.hipshead = Calloc(1, struct header);
	memcpy((char *)new.hipshead, (char *)arg.hipshead, 
	       sizeof(struct header));
	Dupstr(new.hipshead->orig_name, arg.hipshead->orig_name);
	Dupstr(new.hipshead->seq_name, arg.hipshead->seq_name);
	Dupstr(new.hipshead->orig_date, arg.hipshead->orig_date);
	Dupstr(new.hipshead->seq_history, arg.hipshead->seq_history);
	Dupstr(new.hipshead->seq_desc, arg.hipshead->seq_desc);
	Return(new.hipshead);
    case TYPE_LOGENTRY:
	arg.log = va_arg(index, struct logentry *);
	new.log = Calloc(1, struct logentry);
	new.log->command = (Command *) copy(TYPE_COMMAND, arg.log->command);
	new.log->breed = arg.log->breed;
	new.log->active = arg.log->active;
	new.log->action_num = arg.log->action_num;
	Return(new.log);
    default:
	Return(NULL);
    }
#undef Dupstr
#undef Return
}

void
destroy(va_alist)
     va_dcl
{
#define Get(variable, type) variable = va_arg(index, type); if(*variable == \
							       NULL) break
    va_list index;
    int type;
    int i;
    union {
	Command **command;
	Primit **primit;
	Point *list;
	struct header **hipshead;
	struct logentry **log;
    } arg;
    Primit *primit, *oldprimit;
    Point pt, oldpt;

    va_start(index);
    type = va_arg(index, int);

    switch(type) {
    case TYPE_COMMAND:
	Get(arg.command, Command **);
	for(i = 0; i < (*arg.command)->func.len; i++)
	    pfree((*arg.command)->func.primit + i);
	destroy(TYPE_PRIMITLIST, &((*arg.command)->anno.primit));
	free((char *) (*arg.command)->histo);
	free((*arg.command)->comment);
	Cfree(*arg.command, 1, struct command);
	*arg.command = NULL;
	break;
    case TYPE_PRIMIT:
	Get(arg.primit, Primit **);
	pfree(*arg.primit);
	*arg.primit = NULL;
	break;
    case TYPE_POINTLIST:
	Get(arg.list, Point *);
	for(pt = *arg.list; pt != NULL;
	    oldpt = pt, pt = pt->next, Cfree(oldpt, 1, struct point)) ;
	*arg.list = NULL;
	break;
    case TYPE_PRIMITLIST:
	Get(arg.primit, Primit **);
	for(primit = *arg.primit; primit != NULL;
	    oldprimit = primit, primit = primit->next, 
	    destroy(TYPE_PRIMIT, &oldprimit)) ;
	*arg.primit = NULL;
	break;
    case TYPE_HIPSHEAD:
	Get(arg.hipshead, struct header **);
	free((*arg.hipshead)->orig_name);
	free((*arg.hipshead)->seq_name);
	free((*arg.hipshead)->orig_date);
	free((*arg.hipshead)->seq_history);
	free((*arg.hipshead)->seq_desc);
	Cfree(*arg.hipshead, 1, struct header);
	*arg.hipshead = NULL;
	break;
    case TYPE_LOGENTRY:
	Get(arg.log, struct logentry **);
	destroy(TYPE_COMMAND, &((*arg.log)->command));
	Cfree(*arg.log, 1, struct logentry);
	*arg.log = NULL;
	break;
    default:	
	break;
    }
    va_end(index);
}
