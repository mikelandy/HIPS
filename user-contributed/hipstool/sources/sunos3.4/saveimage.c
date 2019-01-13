/* saveimage.c
 * Max Rible
 * Save procedure for hipstool.
 */

#include "hipstool.h"

static int save_mode = SAVE_COMPLETE_IMAGE;
static void save_complete_image(), save_logged_image(), save_box_subimage(),
    save_trace_file(), save_histo_file(), save_log_file(), 
    save_overlaid_image(), save_comment_file();

struct menu_entry save_menu_funcs[NUM_SAVE_FUNCS] = {
/*  { name,		   	   active, action } */
    { "HIPS image sans log",		1, save_complete_image },
    { "HIPS image with special header",	0, save_logged_image },
    { "Rectangular HIPS subimage", 	0, save_box_subimage },
    { "ASCII trace vector",		0, save_trace_file },
    { "ASCII histogram vector",		0, save_histo_file },
    { "HIPStool log file",		0, save_log_file },
    { "HIPS image with log overlaid",	0, save_overlaid_image },
    { "HIPStool comment file",		0, save_comment_file }
};

/* ARGSUSED */
void
save_proc(item, event)
     Panel_item item;
     Event *event;
{
    char label[100], *savefile_name;
    int new_mode;
    FILE *output;

    if(event_is_up(event)) return;

    switch(event_action(event)) {
    case MS_LEFT:
	if(strlen(savefile_name = getstring(io.output)) == 0) {
	    Update_info("You need a filename!");
	    return;
	} else {
	    if(outfilename != NULL) free(outfilename);
	    outfilename = Strdup(savefile_name);
	    if(outfilename == NULL) {
		Update_info("You need a filename!"); return;
	    } else
		if(outfilename[0] == '>') output = stdout;
		else if((output = fopen(outfilename, "w")) == NULL) {
		    perror("output file"); return;
		}
	}
	(*save_menu_funcs[save_mode].action)(output);
	if(output != stdout) fclose(output);
	else fflush(output);
	break;
    case MS_RIGHT:
	new_mode = (int) menu_show(io.save, io.control, event, 0);
	if(new_mode == 0) return;
	save_mode = new_mode - 1;
	sprintf(label, "Save mode:  %s.", save_menu_funcs[save_mode].name);
	Update_info(label);
	break;
    default:
	break;
    }

    return;
}

static void
save_complete_image(output)
     FILE *output;
{
    save_image(0, output, OFF);
    Update_info("Saved complete image.");
}

static void
save_logged_image(output)
     FILE *output;
{
    save_image(0, output, ON);
    Update_info("Saved logged image.");
}

static void
save_box_subimage(output)
     FILE *output;
{
    int (*coords)[2];
    char label[100];

    save_image(1, output, OFF);
    coords = cur_func->func.primit->loc.coords;
    sprintf(label, "Saved %dx%d partial image.", 
	    coords[1][0] - coords[0][0] + 1, coords[1][1] - coords[0][1] + 1);
    Update_info(label);
}

static void
save_trace_file(output)
     FILE *output;
{
    int i;
    char label[100];

    fprintf(output, "%d\n", cur_func->func.primit->data.trace.length);
    for(i = 0; i < cur_func->func.primit->data.traces->length; i++)
	fprintf(output, "%d %d %d\n",
		cur_func->func.primit->data.trace.info[0][i], 
		cur_func->func.primit->data.trace.info[1][i], 
		cur_func->func.primit->data.trace.info[2][i]);
    sprintf(label, "Saved gray vector of length %d.",
	    cur_func->func.primit->data.trace.length);
    Update_info(label);
}

static void
save_histo_file(output)
     FILE *output;
{
    int i;
    char label[100];

    for(i = 0; i <= (base.extremes[1] - base.extremes[0]); i++)
	fprintf(output, "%u %u\n", base.extremes[0] + i, 
		cur_func->histo[i]);
    sprintf(label, "Saved histogram of [%u,%u].", 
	    base.extremes[0], base.extremes[1]);
    Update_info(label);
}

static void
save_log_file(output)
     FILE *output;
{
    int size;
    char label[100];

    size = save_log(output, '\n');
    sprintf(label, "Wrote log of %d bytes.", size);
    Update_info(label);
}

static void
save_overlaid_image(output)
     FILE *output;
{
    union {
	unsigned char *chars;
	unsigned short *shorts;
	unsigned long *longs;
    } ptr;

    switch(base.datasize) {
    case 1: 
	ptr.chars = Calloc(base.winfo.width*base.winfo.height, unsigned char); 
	break;
    case 2:
	ptr.shorts =Calloc(base.winfo.width*base.winfo.height,unsigned short); 
	break;
    case 4:
	ptr.longs = Calloc(base.winfo.width*base.winfo.height, unsigned long); 
	break;
    default:
	return;
    }
    Memcpy(ptr.chars, base.buf.chars,
	   base.winfo.width*base.winfo.height*base.datasize);
    display_logged_actions(DRAW_IN_MEMORY, OFF);
    save_image(0, output, ON);
    Memcpy(base.buf.chars, ptr.chars, 
	   base.winfo.width*base.winfo.height*base.datasize);
    cfree((char *)ptr.chars,
	  (unsigned)(base.winfo.width*base.winfo.height), 
	  (unsigned)base.datasize);
    Update_info("Saved logged, overwritten image.");
}

static void
save_comment_file(output)
     FILE *output;
{
    Fwrite(header_comment, char, strlen(header_comment), output);
    Update_info("Saved comment file.");
}

void
update_save_menu()		/* SUNTOOLS */
{
    int i;

    for(i = 0; i < NUM_SAVE_FUNCS; i++)
	menu_set(menu_get(io.save, MENU_NTH_ITEM, i+1),
		 MENU_INACTIVE, !save_menu_funcs[i].active,
		 0);
    if(!save_menu_funcs[save_mode].active) {
	save_mode = SAVE_COMPLETE_IMAGE;
	Update_info("Save mode:  Complete image.");
    }
}

void
save_image(partial, outfile, logp)
     Bool partial;
     FILE *outfile;
     int logp;
{
    int i, idx, len;
    int (*coords)[2];
    FileInfo *which;
    struct header *hipshead;
    char *arghv[5], string[100];

    if(auxiliary) which = &proj; else which = &base;

    /* Doing a partial save from (x1,y1)-(x2,y2) is the same
     * as running "extract (y2-y1+1) (x2-x1+1) y1 x1".
     */
    if(partial) {
	coords = cur_func->func.primit->loc.coords;

	hipshead = (struct header *) copy(TYPE_HIPSHEAD, &(which->hips));
	setsize(hipshead,coords[1][1] - coords[0][1] + 1,
		coords[1][0] - coords[0][0] + 1);
	hipshead->num_frame = 1;
	sprintf(string, "extract %d %d %d %d", 
		hipshead->rows, hipshead->cols, coords[0][1], coords[0][0]);
	arghv[0] = string;
	len = strlen(string);
	for(i = 1, idx = 1; i < len; i++) {
	    if(string[i] == ' ') string[i] = '\0';
	    if(string[i-1] == '\0') arghv[idx++] = string+i;
	}
	update_header(hipshead, 5, arghv);
	fwrite_header(outfile, hipshead);
	destroy(TYPE_HIPSHEAD, &hipshead);
    } else {
	if(logp) {
	    log_to_xheader(&which->hips);
	    comment_to_xheader(&which->hips);
	}
	fwrite_header(outfile, &which->hips);
    }

    if(partial && !auxiliary) {
	for(i = coords[0][1]; i <= coords[1][1]; i++) {
	    switch(which->datasize) {
	    case 1:
		Fwrite(which->buf.chars + which->winfo.width*i + 
		       coords[0][0], unsigned char,
		       coords[1][0] - coords[0][0] + 1,
		       outfile);
		break;
	    case 2:
		Fwrite(which->buf.shorts + which->winfo.width*i +
		       coords[0][0], unsigned short,
		       coords[1][0]- coords[0][0] + 1,
		       outfile);
		break;
	    case 4:
		Fwrite(which->buf.longs + which->winfo.width*i +
		       coords[0][0], unsigned long, 
		       coords[1][0]- coords[0][0] + 1,
		       outfile);
		break;
	    }
	}
    } else {
	switch(which->datasize) {
	case 1:	
	    Fwrite(which->buf.chars, unsigned char, 
		   which->winfo.width*which->winfo.height, outfile); 
	    break;
	case 2:	
	    Fwrite(which->buf.shorts, unsigned short, 
		   which->winfo.width*which->winfo.height, outfile); 
	    break;
	case 4:	
	    Fwrite(which->buf.longs, unsigned long, 
		   which->winfo.width*which->winfo.height, outfile); 
	    break;
	}
    }

    return;
}
