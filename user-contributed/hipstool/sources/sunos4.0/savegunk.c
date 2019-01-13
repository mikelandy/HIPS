/* stuff we might use to translate from HIPStool command language to
 * Superplot.
 */

#include "hipstool.h"

int
save_log(output)
     FILE *output;
{
    Log current;
    int num_chars = 0;

    num_chars += fprintf(output, "s 0 0 %d %d\n", base.winfo.width, 
			 base.winfo.height);
    num_chars += fprintf(output, "F 0 0 %d %d 1\n", base.winfo.width, 
			 base.winfo.height);

    for(current = actions; current != NULLOG; current = current->next)
	num_chars += save_action(current, output, 0);
    return(num_chars);
}

/* ARGSUSED */
int
no_save(file, event, y)
     FILE *file;
     Log event;
     int y;
{
    return(0);
}

/* ARGSUSED */
void
no_load(infile, entry)
     FILE *infile;
     Log entry;
{
}

int
box_save(file, event, y)
     FILE *file;
     Log event;
     int y;
{
    int size = 0;

    Fprintf(file, "m %d %d%c", event->pairs[0][0], event->pairs[0][1],
	    x[y]);
    Fprintf(file, "L 4 %d %d %d %d %d %d %d %d%c", 
	    event->pairs[1][0], event->pairs[0][1],
	    event->pairs[1][0], event->pairs[1][1],
	    event->pairs[0][0], event->pairs[1][1],
	    event->pairs[0][0], event->pairs[0][1], x[y]);
    
    return(size);
}

void
box_load(infile, entry)
     FILE *infile;
     Log entry;
{
    int (*arr)[2] = entry->pairs;

    fscanf(infile, "m %d %d%*c", &arr[0][0], &arr[0][1]);
    fscanf(infile, "L 4 %*d %*d %d %d %*d %*d %*d %*d%*c", 
	   &arr[1][0], &arr[1][1]);
}

int
line_save(file, event, y)
     FILE *file;
     Log event;
     int y;
{
    int size = 0;

    Fprintf(file, "m %d %d%c", event->pairs[0][0], event->pairs[0][1],
	    x[y]);
    Fprintf(file, "n %d %d%c", event->pairs[1][0], event->pairs[1][1],
	    x[y]);
    
    return(size);
}

void
line_load(infile, entry)
     FILE *infile;
     Log entry;
{
    int (*arr)[2] = entry->pairs;

    fscanf(infile, "m %d %d%*c", &arr[0][0], &arr[0][1]);
    fscanf(infile, "n %d %d%*c", &arr[1][0], &arr[1][1]);
}

int
angle_save(file, event, y)
     FILE *file;
     Log event;
     int y;
{
    int size = 0;

    Fprintf(file, "m %d %d%c", event->pairs[0][0], event->pairs[0][1],
	    x[y]);
    Fprintf(file, "n %d %d%c", event->pairs[1][0], event->pairs[1][1],
	    x[y]);
    Fprintf(file, "m %d %d%c", event->pairs[2][0], event->pairs[2][1],
	    x[y]);
    Fprintf(file, "n %d %d%c", event->pairs[3][0], event->pairs[3][1],
	    x[y]);

    return(size);
}

void
angle_load(infile, entry)
     FILE *infile;
     Log entry;
{
    int (*arr)[2] = entry->pairs;

    fscanf(infile, "m %d %d%*c", &arr[0][0], &arr[0][1]);
    fscanf(infile, "n %d %d%*c", &arr[1][0], &arr[1][1]);
    fscanf(infile, "m %d %d%*c", &arr[2][0], &arr[2][1]);
    fscanf(infile, "n %d %d%*c", &arr[3][0], &arr[3][1]);
}

int
openspline_save(file, event, y)
     FILE *file;
     Log event;
     int y;
{
    int i, size = 0;
    
    Fprintf(file, "k 0 %d ", event->len);
    for(i = 0; i < event->len-1; i++)
	Fprintf(file, "%d %d ", event->pairs[i][0], event->pairs[i][1]);
    Fprintf(file, "%d %d%c", event->pairs[event->len-1][0],
	    event->pairs[event->len-1][1], x[y]);

    return(size);
}

void
openspline_load(infile, entry)
     FILE *infile;
     Log entry;
{
    int (*arr)[2], i;
    
    fscanf(infile, "k 0 %d ", &entry->len);
    entry->pairs = arr = (int (*)[2]) 
	calloc((unsigned)(2*entry->len), sizeof(int));
    for(i = 0; i < entry->len-1; i++)
	fscanf(infile, "%d %d ", &arr[i][0], &arr[i][1]);
    fscanf(infile, "%d %d%*c", &arr[entry->len-1][0],
	   &arr[entry->len-1][1]);
}

int
polygon_save(file, event, y)
     FILE *file;
     Log event;
     int y;
{
    int i, size = 0;
    
    Fprintf(file, "m %d %d%c", event->pairs[0][0], event->pairs[0][1], 
	    x[y]);
    Fprintf(file, "L %d ", event->len+1); /* chain */
    for(i = 0; i < event->len; i++)
	Fprintf(file, "%d %d ", event->pairs[i][0], event->pairs[i][1]);
    Fprintf(file, "%d %d%c", event->pairs[0][0], event->pairs[0][1], x[y]);

    return(size);
}

void
polygon_load(infile, entry)
     FILE *infile;
     Log entry;
{
    int (*arr)[2], i;

    fscanf(infile, "m %*d %*d%*c");
    fscanf(infile, "L %d ", &entry->len);
    entry->len--;
    entry->pairs = arr = (int (*)[2]) 
	calloc((unsigned)(2*entry->len), sizeof(int));
    for(i = 0; i < entry->len; i++)
	fscanf(infile, "%d %d ", &arr[i][0], &arr[i][1]);
    fscanf(infile, "%*d %*d%*c");
}

int
cookie_save(file, event, y)
     FILE *file;
     Log event;
     int y;
{
    int i, size = 0;

    Fprintf(file, "k 1 %d ", event->len);
    for(i = 0; i < event->len-1; i++)
	Fprintf(file, "%d %d ", event->pairs[i][0], event->pairs[i][1]);
    Fprintf(file, "%d %d%c", event->pairs[event->len-1][0],
	    event->pairs[event->len-1][1], x[y]);

    return(size);
}

void
cookie_load(infile, entry)
     FILE *infile;
     Log entry;
{
    int (*arr)[2], i;

    fscanf(infile, "k 1 %d ", &entry->len);
    entry->pairs = arr = (int (*)[2]) 
	calloc((unsigned)(2*entry->len), sizeof(int));
    for(i = 0; i < entry->len-1; i++)
	fscanf(infile, "%d %d ", &arr[i][0], &arr[i][1]);
    fscanf(infile, "%d %d%*c", &arr[entry->len-1][0],
	   &arr[entry->len-1][1]);
}

void
distance_bypass(string, entry)
     char *string;
     Log entry;
{
    char comment[100];

    if(string[0] == '[') {
	sscanf(string, "[(%d,%d)-(%d,%d)=%*d]:  %[^.].",
	       &entry->pairs[0][0], &entry->pairs[0][1],
	       &entry->pairs[1][0], &entry->pairs[1][1],
	       comment);
	entry->comment = Strdup(comment);
    } else {
	sscanf(string, "Distance from (%d,%d) to (%d,%d) is %*d.",
	       &entry->pairs[0][0], &entry->pairs[0][1],
	       &entry->pairs[1][0], &entry->pairs[1][1]);
    }
}

void
standout_bypass(string, entry)
     char *string;
     Log entry;
{
    char comment[100];

    if(string[0] == '[') {
	sscanf(string, "[r:%d g:%d b:%d: %[^.].",
	       &entry->pairs[0][1], &entry->pairs[1][1], 
	       &entry->pairs[2][1], comment);
	entry->comment = Strdup(comment);
    } else {
	sscanf(string, "Red: %d; Green: %d; Blue: %d.",
	       &entry->pairs[0][1], &entry->pairs[1][1], 
	       &entry->pairs[2][1]);
    }
    entry->pairs[0][0] = entry->pairs[1][0] = entry->pairs[2][0] = STANDOUT;
}
