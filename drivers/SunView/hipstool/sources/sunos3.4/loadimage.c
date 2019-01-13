/* loadimage.c
 * Max Rible
 * Load routines for hipstool.
 */

#include "hipstool.h"

static Windowinfo *zaptmp; /* For zap() only */
static void zap_proc();
static void display_init(), load_chars(), load_shorts(), load_ints(),
    load_hips_image(), load_log_file(), load_comment_file();
static int load_mode = LOAD_HIPS_IMAGE;

struct menu_entry load_menu_funcs[NUM_LOAD_FUNCS] = {
/*  { name,		   active, action }		/* defined index */
    { "HIPS image",		1, load_hips_image },	/* LOAD_HIPS_IMAGE */
    { "HIPStool log file",	0, load_log_file },	/* LOAD_LOG_FILE */
    { "HIPStool comment file",	0, load_comment_file }	/* LOAD_COMMENT_FILE */
};

/* ARGSUSED */
void
load_proc(item, event)
     Panel_item item;
     Event *event;
{
    char label[100];
    int new_mode;

    if(event_is_up(event)) return;

    switch(event_action(event)) {
    case MS_LEFT:
	(*load_menu_funcs[load_mode].action)();
	break;
    case MS_RIGHT:
	new_mode = (int) menu_show(io.load, io.control, event, 0);
	if(new_mode == 0) return;
	load_mode = new_mode - 1;
	sprintf(label, "Load mode:  %s.", load_menu_funcs[load_mode].name);
	Update_info(label);
	break;
    default:
	break;
    }

    return;
}

static void
load_hips_image()
{
    if(auxiliary)
	load_image(&proj, CHILD_FRAME,(FILE *) -1);
    else {
	actions = NULLOG;

	load_menu_funcs[LOAD_LOG_FILE].active = 1;
	load_menu_funcs[LOAD_COMMENT_FILE].active = 1;
	update_load_menu();

	save_menu_funcs[SAVE_LOGGED_IMAGE].active = 0;
	save_menu_funcs[SAVE_LOG_FILE].active = 0;
	save_menu_funcs[SAVE_OVERLAID_IMAGE].active = 0;
	update_save_menu();

	load_image(&base, BASE_FRAME,(FILE *) -1);
    }
}


static void
load_log_file()
{
    FILE *input;

    if((input = fopen(getstring(io.input), "r")) == NULL) {
	perror("Input file"); return; }
    load_log(input);
    fclose(input);
}

static void
load_comment_file()
{
    FILE *input;
    struct stat statbuf;

    if((input = fopen(getstring(io.input), "r")) == NULL) {
	perror("Input file"); return; }
    fstat(fileno(input), &statbuf);
    header_comment = Calloc(statbuf.st_size, char);
    Fread(header_comment, char, statbuf.st_size, input);
    save_menu_funcs[SAVE_LOGGED_IMAGE].active = 1;
    save_menu_funcs[SAVE_COMMENT_FILE].active = 1;
    update_save_menu();
    fclose(input);
}

void
update_load_menu()		/* SUNTOOLS */
{
    int i;

    for(i = 0; i < NUM_LOAD_FUNCS; i++)
	menu_set(menu_get(io.load, MENU_NTH_ITEM, i+1),
		 MENU_INACTIVE, !load_menu_funcs[i].active,
		 0);
    if(!load_menu_funcs[load_mode].active) {
	load_mode = LOAD_HIPS_IMAGE;
	Update_info("Load mode:  Complete image.");
    }
}

/* frame specifies which frame to load to.
 * If fd is legitimate, then load_image reads from it and dumps it
 * in the appropriate frame; otherwise, it reads from a file.
 */

void
load_image(which, frame, fp)
     FileInfo *which;
     int frame;
     FILE *fp;
{
    FILE *input;
    struct itimerval itimevalue;
    char foo[10];

/* Figure out what we're loading from where
 */
    if (((int) fp) < 0) {
	if(infilename == NULL) {
	    if((infilename = Strdup(getstring(io.input))) == NULL)
		input = stdin;
	} else {
	    if(strcmp(infilename, getstring(io.input)) != 0) {
		free(infilename);
		infilename = Strdup(getstring(io.input));
	    }
	}
	if((input = fopen(infilename, "r")) == NULL) {
	    perror("input file"); return;
	}
    } else
	input = fp;

/* Clean up after ourselves.
 */
    if(!which->virgin) {
#ifdef USE_MMAP
	if(munmap(which->map.addr, which->map.len) == -1)
	    perror("munmap failure");
#else
	switch(which->datasize) {
	case 1:
	    Cfree(which->buf.chars, which->winfo.width*which->winfo.height, 
		  unsigned char); 
	    break;
	case 2:
	    Cfree(which->buf.shorts, which->winfo.width*which->winfo.height, 
		  unsigned short); 
	    break;
	case 4:
	    Cfree(which->buf.longs, which->winfo.width*which->winfo.height, 
		  unsigned long); 
	    break;
	default:
	    break;
	}
#endif
	Cfree(which->image, ((which->winfo.width >> 1) + 
			     (which->winfo.width & 1))*which->winfo.height,
	      unsigned short);
	Cfree(which->user_lut, which->extremes[1] + 1, unsigned char);
    }

/* Find out what we're getting into.
 */
    fread_header(input, &which->hips,"");
    which->winfo.width = which->hips.ocols;
    which->winfo.height = which->hips.orows;
#ifdef MULTO_FRAMES
    which->frames.num = which->hips.num_frame;
#endif
    switch(which->hips.pixel_format) {
    case PFINT:		which->datasize = 4; break;
    case PFSHORT:	which->datasize = 2; break;
    case PFBYTE:	which->datasize = 1; break;
    default:
	fprintf(stderr, "Bad input data type.\n");
	return;
    }
    xheader_to_log(&which->hips);
    xheader_to_comment(&which->hips);

    which->image = 
	Calloc(((which->winfo.width >> 1) + (which->winfo.width & 1)) * 
	       which->winfo.height, unsigned short);

/* Get ready for the image.
 */
    display_init(which, frame);

/* Read in the file.
 */
    switch(which->datasize) {
    case 1:	load_chars(which, input); break;
    case 2:	load_shorts(which, input); break;
    case 4:	load_ints(which, input); break;
    }

#ifdef MULTO_FRAMES
    switch(which->datasize) {
    case 1:	which->frames.base.chars = which->buf.chars; break;
    case 2:	which->frames.base.shorts = which->buf.shorts; break;
    case 4:	which->frames.base.longs = which->buf.longs; break;
    }
    which->frames.current = 1;
#endif

    update_frame_label(which);

    if(which->virgin) which->virgin = 0;
    if(which == &base) {
	sprintf(foo, "1/%d", base.frames.num);
	panel_set(io.messages[MESSAGE_FRAME_NUMBER], PANEL_LABEL_STRING,
		  foo, 0);
    }

#ifdef SUNTOOLS
/* Make suntools wait before loading in the image.
 * This is apparently magic.
 */
    itimevalue.it_value.tv_sec = 0;
    itimevalue.it_interval.tv_sec = 0;
    itimevalue.it_interval.tv_usec = 0;
    itimevalue.it_value.tv_usec = 100000;
    zaptmp = &which->winfo;
    notify_set_itimer_func(io.control, (Notify_func) zap_proc, ITIMER_REAL,
			   &itimevalue, ((struct itimerval *) 0)); 
#else
    refresh(which, NULL);
#endif

    return;
}

#ifdef SUNTOOLS
static void
zap_proc()
{
    refresh(zaptmp, NULL);
}
#endif

static void
display_init(which, frame)	/* SUNTOOLS */
     FileInfo *which;
     int frame;
{
    char label[100];
    char *name;
    int tmpx, tmpy;
    void (*event_proc)();

/* Clean up any old scrollbars along with the canvas. 
 */
    if(which->winfo.canvas != NULL) {
	scrollset(&which->winfo, 0, 0);
	window_destroy(which->winfo.canvas);
	which->winfo.canvas = NULL;
    }
    
/* Determine window dimensions.
 */
    if(resize || which->winfo.frame == NULL) {
	tmpx = Win_width(&which->winfo);
	tmpy = Win_height(&which->winfo);
    } else {
	tmpx = (int) window_get(which->winfo.frame, WIN_WIDTH) - MARG_X;
	tmpy = (int) window_get(which->winfo.frame, WIN_HEIGHT) - MARG_Y;
    }

    if(frame == BASE_FRAME) name = "Base"; else name = "Modified";

/* Create or adjust window appropriately.
 */
    sprintf(label, "HIPStool %s Image: %dx%d",
	    name, which->winfo.width, which->winfo.height);

    if(which->winfo.frame == NULL)
	newindow(&which->winfo,
		 which->winfo.width, which->winfo.height,
		 (frame == BASE_FRAME ? 10 :Win_width(&base.winfo)+10+MARG_X), 
		 250,
		 label);
    else
	changewindow(&which->winfo, tmpx, tmpy, label);

/* Set up appropriate event procedures.
 */
    if(frame == BASE_FRAME)
	event_proc = base_event_proc;
    else
	event_proc = child_event_proc;

    window_set(which->winfo.canvas,
	       WIN_EVENT_PROC, event_proc,
	       WIN_CONSUME_PICK_EVENTS, WIN_NO_EVENTS,
	       LOC_MOVE, WIN_MOUSE_BUTTONS, LOC_DRAG, 
	       WIN_RESIZE, SCROLL_REQUEST, 0,
	       0);

/* Make the cursor visible --- use the XOR operation.
 */
    init_cursor(&which->winfo);
    
/* Set scrollbars if too large, otherwise turn off.
 */
    if(frame == BASE_FRAME) {
	panel_set(io.toggles,
		  PANEL_TOGGLE_VALUE, 0, which->winfo.height > MAX_WINY,
		  PANEL_TOGGLE_VALUE, 1, which->winfo.width > MAX_WINX,
		  0);
    }

    scrollset(&which->winfo, (Bool)(which->winfo.height > MAX_WINY), 
	      (Bool)(which->winfo.width > MAX_WINX));
}

static void
load_chars(which, input)
     FileInfo *which;
     FILE *input;
{
    int area, image_size, i, header_size;
    double maximum;
    unsigned long tmp, minv = 0xFF, maxv = 0;

    area = which->winfo.width * which->winfo.height;
#ifdef MULTO_FRAMES
    area *= which->frames.num;
#endif
#ifdef USE_MMAP
    image_size = area * sizeof(unsigned char);

    header_size = (int) ftell(input);

    if((which->map.addr =
	mmap((caddr_t) NULL, image_size + header_size, PROT_READ, MAP_SHARED, 
	     fileno(input), (off_t) 0)) == (caddr_t) -1)
	perror("mmap failure");
    which->map.len = image_size + header_size;
    which->buf.chars = (unsigned char *) which->map.addr + header_size;
#else
    if((which->buf.chars = Calloc(area, unsigned char)) == NULL) {
	perror("image allocation"); exit(-3); }

    if(Fread(which->buf.chars, unsigned char, area, input) != area) {
	perror("image read"); return; }
#endif

/* Find minima and maxima.
 */
    for(i = 0; i < area; i++) {
	if((tmp = (u_long) which->buf.chars[i]) > maxv) maxv = tmp;
	if(tmp < minv) minv = tmp;
    }

    which->extremes[0] = (int) minv;
    which->extremes[1] = (int) maxv;

/* Create scaled lookup table. 
 */
    maximum = (double) (maxv - minv);

    which->user_lut = Calloc(maxv+1, unsigned char);
    for(i = 0; i < minv; i++)
	which->user_lut[i] = tr[0];
    for(i = minv; i <= maxv; i++)
	which->user_lut[i] = tr[SCALE((double)(i - minv)/maximum)];

/* Get the image.
 */
    which->winfo.pr = 
	chars_to_pixrect(which->buf.chars, which->image,
			 which->winfo.width, which->winfo.height,
			 which->user_lut);
}

static void
load_shorts(which, input)
     FileInfo *which;
     FILE *input;
{
    int area, image_size, header_size, i;
    double maximum;
    unsigned long tmp, minv = 0xFFFF, maxv = 0;

    area = which->winfo.width * which->winfo.height;
#ifdef MULTO_FRAMES
    area *= which->frames.num;
#endif
#ifdef USE_MMAP
    image_size = area * sizeof(unsigned short);

    header_size = (int) ftell(input);

    if((which->map.addr =
	mmap((caddr_t) NULL, image_size + header_size, PROT_READ, MAP_SHARED, 
	     fileno(input), (off_t) 0)) == (caddr_t) -1)
	perror("mmap failure");
    which->map.len = image_size + header_size;
    which->buf.shorts = (unsigned short *) (which->map.addr + header_size);
#else
    if((which->buf.shorts = Calloc(area, unsigned short)) == NULL) {
	perror("image allocation"); exit(-3); }

    if(Fread(which->buf.shorts, unsigned short, area, input) != area) {
	perror("image read"); return; }
#endif

/* Find minima and maxima.
 */
    for(i = 0; i < area; i++) {
	if((tmp = (u_long) which->buf.shorts[i]) > maxv) maxv = tmp;
	if(tmp < minv) minv = tmp;
    }

    which->extremes[0] = (int) minv;
    which->extremes[1] = (int) maxv;

/* Create scaled lookup table. 
 */
    maximum = (double) (maxv - minv);

    which->user_lut = Calloc(maxv+1, unsigned char);
    for(i = 0; i < minv; i++)
	which->user_lut[i] = tr[0];
    for(i = minv; i <= maxv; i++)
	which->user_lut[i] = tr[SCALE((double)(i - minv)/maximum)];

/* Get the image.
 */
    which->winfo.pr = 
	shorts_to_pixrect(which->buf.shorts, which->image,
			  which->winfo.width, which->winfo.height,
			  which->user_lut);
}

static void
load_ints(which, input)
     FileInfo *which;
     FILE *input;
{
    int area, image_size, header_size, i;
    double maximum;
    unsigned long tmp, minv = 0xFFFFFFFF, maxv = 0;

    area = which->winfo.width * which->winfo.height;
#ifdef MULTO_FRAMES
    area *= which->frames.num;
#endif

#ifdef USE_MMAP
    image_size = area * sizeof(unsigned long);

    header_size = (int) ftell(input);

    if((which->map.addr =
	mmap((caddr_t) NULL, image_size + header_size, PROT_READ, MAP_SHARED, 
	     fileno(input), (off_t) 0)) == (caddr_t) -1)
	perror("mmap failure");
    which->map.len = image_size + header_size;
    which->buf.longs = (unsigned long *) (which->map.addr + header_size);
#else
    if((which->buf.longs = Calloc(area, unsigned long)) == NULL) {
	perror("image allocation"); exit(-3); }

    if(Fread(which->buf.longs, unsigned long, area, input) != area) {
	perror("image read"); return; }
#endif

/* Find minima and maxima.
 */
    for(i = 0; i < area; i++) {
	tmp = (u_long) which->buf.longs[i];
	if(tmp > maxv) maxv = tmp;
	if(tmp < minv) minv = tmp;
    }

    which->extremes[0] = (int) minv;
    which->extremes[1] = (int) maxv;

/* Create scaled lookup table. 
 */
    maximum = (double) (maxv - minv);

    which->user_lut = Calloc(maxv+1, unsigned char);
    for(i = 0; i < minv; i++)
	which->user_lut[i] = tr[0];
    for(i = minv; i <= maxv; i++)
 	which->user_lut[i] = tr[SCALE((double)(i - minv)/maximum)];

/* Get the image.
 */
    which->winfo.pr = 
	longs_to_pixrect(which->buf.longs, which->image,
			 which->winfo.width, which->winfo.height,
			 which->user_lut);
}
