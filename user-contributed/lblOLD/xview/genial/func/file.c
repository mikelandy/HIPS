
/*
 * file.c -- routines for loading and saving GENIAL images
 *
 */

#include <stdio.h>
#include <fcntl.h>
#include <strings.h>
#include <X11/Xlib.h>

#include "common.h"
#include "ui.h"
#include "display.h"
#include "reg.h"
#include "log.h"
#include "file_ui.h"

#define READ_BLOCK_SIZE  524288	/* half a Meg... */

/*
 * the following defines help us avoid including rasterfile.h which is
 * occasionally kept in nonstandard places...
 */

struct rasterfile {
	int	ras_magic;		/* magic number */
	int	ras_width;		/* width (pixels) of image */
	int	ras_height;		/* height (pixels) of image */
	int	ras_depth;		/* depth (1, 8, or 24 bits) of pixel */
	int	ras_length;		/* length (bytes) of image */
	int	ras_type;		/* type of file; see RT_* below */
	int	ras_maptype;		/* type of colormap; see RMT_* below */
	int	ras_maplength;		/* length (bytes) of following map */
	/* color map follows for ras_maplength bytes, followed by image */
};
#define	RAS_MAGIC	0x59a66a95
#define RT_STANDARD	1	/* Raw pixrect image in 68000 byte order */
#define RMT_EQUAL_RGB	1	/* red[ras_maplength/3],green[],blue[] */

static int lmode = 0, smode = 0;/* load mode */

static int boxnum = 0;
static file_rect_choice_win_objects *rect_win = NULL;	/* just a popup */
FILE     *file_open(), *open_output_file();

set_lmode(m)
    int       m;
{
    lmode = m;
}

int
load_image(fname)
    char     *fname;
{
    FILE     *fp;
    struct header hd;
    int       i, remain;
    unsigned  data_len;
    int       name_loc, bytes_read;
    char     *buffer;
    char      msg[256], short_name[50];
    XImage   *mk_x_img();

    if ((fp = file_open(fname)) == NULL)
	return (-1);

    /* remove path name from file name if necessary */
    name_loc = (int) rindex(fname, '/');
    if (name_loc != NULL)
	sprintf(short_name, "%s", name_loc + 1);
    else
	sprintf(short_name, "%s", fname);

    xv_set(base_win->image_fname, PANEL_LABEL_STRING, short_name, NULL);
    panel_paint(base_win->image_fname, PANEL_CLEAR);

    if (lmode == 1) {
	load_log_file(fname, fp);
	return 1;
    }
    /* initialize the display structure */
    if (orig_img == NULL) {
	orig_img = (struct img_data *) malloc(sizeof(struct img_data));
	bzero((char *) orig_img, sizeof(struct img_data));
    } else {
	/* free the data space from previous image */
	free(orig_img->data);
	free(orig_img->lut);
    }

    /* read in HIPS header */
    if (fread_header(fp, &hd, (Filename) fname) == HIPS_ERROR) {
	XBell(display, 0);
	fclose(fp);
	return (-1);
    }
    orig_img->head = hd;

    switch (orig_img->head.pixel_format) {
    case PFUINT:
	orig_img->dsize = 4;
	break;
    case PFUSHORT:
	orig_img->dsize = 2;
	break;
    case PFBYTE:
	orig_img->dsize = 1;
	break;
    default:
	XBell(display, 0);
	fprintf(stderr, "Data type not currently understood.\n");
	return (-1);
    }

    orig_img->width = orig_img->head.ocols;
    orig_img->height = orig_img->head.orows;
    orig_img->nframes = orig_img->head.num_frame;
    orig_img->comment = orig_img->head.seq_desc;
    orig_img->cframe = 1;
    orig_img->file_saved = 0;
    data_len = (unsigned) (orig_img->width * orig_img->height *
			   orig_img->nframes);
    if ((buffer = (char *) calloc(data_len, orig_img->dsize)) == NULL) {
	sprintf(msg, "Error allocating storage:");
	(void) strcat(msg, sys_errlist[errno]);
	lab_info(msg, 1);
	fprintf(stderr, "%s\n", msg);
	return (-1);
    }
#define SIMPLER
#ifdef SIMPLER
    fprintf(stderr, "genial: Loading image...... \n");
    if (fread(buffer, orig_img->dsize, data_len, fp) != data_len) {
	XBell(display, 0);
	perror("\n error reading data\n");
	return (-1);
    }
    orig_img->data = buffer;
#else
    /*
     * NOTE!! There is a bug here that causes a segv on certain sized
     * images!! (ex: 480x512)
     */
    orig_img->data = buffer;
    /* read in the image a fraction at a time... */
    for (i = 0; (unsigned) (i * READ_BLOCK_SIZE) < data_len; i++) {
	bytes_read = fread(buffer, orig_img->dsize, READ_BLOCK_SIZE, fp);
	if ((bytes_read < 0) ||
	    (data_len * orig_img->dsize < READ_BLOCK_SIZE &&
	     bytes_read != data_len * orig_img->dsize)) {
	    XBell(display, 0);
	    sprintf(msg, "Error reading image from file:");
	    (void) strcat(msg, sys_errlist[errno]);
	    lab_info(msg, 1);
	    fprintf(stderr, "%s\n", msg);
	    return (-1);
	}
	sprintf(msg, "Loading image: %d%% loaded",
		(int) (i * READ_BLOCK_SIZE) * 100 / data_len);
	lab_info(msg, 1);
	printf("%s\n", msg);

	buffer += (READ_BLOCK_SIZE * orig_img->dsize);
    }
    remain = data_len - (--i * READ_BLOCK_SIZE);
    if (fread(buffer, orig_img->dsize, remain, fp) < 0) {
	sprintf(msg, "Error reading image from file:");
	XBell(display, 0);
	(void) strcat(msg, sys_errlist[errno]);
	lab_info(msg, 1);
	fprintf(stderr, "%s\n", msg);
	return (-1);
    }
#endif
    (void) xv_set(img_win->d_win, FRAME_MAX_SIZE,
		      orig_img->width + SCROLL_BAR_SIZE,
					orig_img->height + SCROLL_BAR_SIZE, NULL);


    sprintf(msg, "Loading image: %d%% loaded", 100);
    lab_info(msg, 1);
    printf("%s\n", msg);

    /* create lut buffer with size of 1 frame */
    if (depth == 24)
	buffer = (char *) calloc(orig_img->width * orig_img->height * 4,
				 sizeof(byte));
    else
	buffer = (char *) calloc(orig_img->width * orig_img->height,
				 sizeof(byte));
    if (buffer == NULL) {
	sprintf(msg, "Error allocating storage:");
	(void) strcat(msg, sys_errlist[errno]);
	lab_info(msg, 1);
	fprintf(stderr, "%s\n", msg);
	return (-1);
    }
    orig_img->lut = buffer;

    printf("Finding extrema.  Linear scan 1 of 3:");
    fflush(stdout);
    /* find extrema of image */
    extrema(orig_img);
    putchar('\n');

    /* initialize the display */
    init_display(orig_img);

    printf("Building colormap.... ");
    fflush(stdout);
    build_cmap(orig_img);	/* sets orig_img->lut */

    printf("Building original XImage.  Linear scan 2 of 3:");
    fflush(stdout);
    /* make an XImage */
    orig_ximg = mk_x_img(orig_img, orig_ximg, 0);
    putchar('\n');

    printf("Building display XImage.  Linear scan 3 of 3:");
    fflush(stdout);
    disp_ximg = mk_x_img(orig_img, disp_ximg, 1);
    putchar('\n');

    printf("Displaying image.\n");
    disp_img();

    printf("Loading log.\n");
    read_log(&orig_img->head);
    printf("Loading complete\n");
    return 1;
}

/***************************************************************/
FILE     *
file_open(fname)
    char     *fname;
{
    FILE     *fp;
    char      msg[256];

    if ((fp = fopen(fname, "r")) == NULL) {
	sprintf(msg, "Error opening:%s: ", fname);
	(void) strcat(msg, sys_errlist[errno]);
	XBell(display, 0);
	lab_info(msg, 1);
	fprintf(stderr, "%s\n", msg);
    }
    return (fp);
}

/***************************************************************/

load_log_file(fname, fp)
    char     *fname;
    FILE     *fp;
{
    struct header hd;

    /* read in HIPS header */
    if (fread_header(fp, &hd, (Filename) fname) == HIPS_ERROR) {
	XBell(display, 0);
	fclose(fp);
	return (-1);
    }
    printf("Loading log.\n");
    read_log(&hd);
    printf("Loading complete\n");

    return 1;
}

/********************************************************/
#define getbytes(c,n) ( *((unsigned *) (c)) >> (8*(4-n)))

extrema(ras)
    struct img_data *ras;
{
    unsigned  minv, maxv;
    register unsigned j;
    register int i;

    /* find minima and maxima */
    minv = (unsigned long) getbytes(ras->data, ras->dsize);
    maxv = 0;
    if (ras->dsize == 1) {
	for (i = 0; i < ras->height * ras->width; i++) {
	    j = *((unsigned char *) ras->data + i);
	    if (j > maxv)
		maxv = j;
	    else if (j < minv)
		minv = j;
	}
    } else if (ras->dsize == 2) {
	for (i = 0; i < (ras->height * ras->width); i++) {
	    j = *((unsigned short *) ras->data + i);
	    if (j > maxv)
		maxv = j;
	    else if (j < minv)
		minv = j;
	}
    } else if (ras->dsize == 4) {
	for (i = 0; i < ras->height * ras->width; i += 4) {
	    j = *((unsigned int *) ras->data + i);
	    if (j > maxv)
		maxv = j;
	    else if (j < minv)
		minv = j;
	}
    }
    ras->maxv = maxv;
    ras->minv = minv;
}

int
set_smode(m)
    int       m;
{
    smode = m;
}

save_image(fname, item)
    char     *fname;
    Panel_item item;
{
    FILE     *fp;
    char     *data_ptr;

    fp = open_output_file(fname);

    if (smode == 4) {
	save_rasterfile(fp);
	return 1;
    }
    if (smode == 5) {
	save_trace_vector(fp);
	return 1;
    }
    if (smode == 6) {
	save_histogram_vector(fp);
	return 1;
    }
    if (smode == 0 || smode == 3) {
	save_log(&orig_img->head);
    }
    if (smode == 2) {
	if (rect_win == NULL) {
	    rect_win = file_rect_choice_win_objects_initialize(NULL,
							 file_win->window1);
	}
	set_rect_panel_state();
	xv_set(rect_win->rect_choice_win,
	       XV_SHOW, TRUE,
	       FRAME_CLOSED, FALSE, NULL);
	return 1;
    }
    orig_img->head.seq_desc = orig_img->comment;
    orig_img->head.sizedesc = strlen(orig_img->comment);
    fwrite_header(fp, &orig_img->head, fname);
    orig_img->file_saved = 1;

    if (smode == 3) {		/* mode 3 is header only */
	fclose(fp);
	return 0;
    }
    /* set data_ptr to beginning of first frame */
    if (orig_img->cframe != 1)
	data_ptr = (char *) (orig_img->data - ((orig_img->cframe - 1) *
		   (orig_img->width * orig_img->height * orig_img->dsize)));
    else
	data_ptr = (char *) orig_img->data;

    if (fwrite(data_ptr, orig_img->dsize,
	orig_img->width * orig_img->height * orig_img->nframes, fp) == -1) {
	XBell(display, 0);
	perror("write:");
	fclose(fp);
	return -1;
    }
    fclose(fp);
    return 0;
}

/* the following routines are for use with saving rectangular subimages */

/* set_rect_panel_state() sets the appropriate items as valid or invalid */
set_rect_panel_state()
{
    struct logent *ent, *log_by_id();

    ent = log_by_id(boxnum);
    if (ent == NULL) {
	xv_set(rect_win->box_save,
	       PANEL_INACTIVE, TRUE,
	       NULL);
	return;
    }
    if (ent->reg) {
	if (ent->reg->r_type != BOX) {
	    xv_set(rect_win->box_save,
		   PANEL_INACTIVE, TRUE,
		   NULL);
	    return;
	}
    } else {
	xv_set(rect_win->box_save,
	       PANEL_INACTIVE, TRUE,
	       NULL);
	return;
    }
    /* it really is a valid box, so set the save button */
    xv_set(rect_win->box_save,
	   PANEL_INACTIVE, FALSE,
	   NULL);
}

/*
 * Notify callback function for `box_lid'.
 */
Panel_setting
boxid_proc(item, event)
    Panel_item item;
    Event    *event;
{
    int       value = (int) xv_get(item, PANEL_VALUE);

    boxnum = value;
    set_rect_panel_state();
    return panel_text_notify(item, event);
}

/*
 * Notify callback function for `box_cancel'.
 */
void
box_cancel_proc(item, event)
    Panel_item item;
    Event    *event;
{
    xv_set(rect_win->rect_choice_win, XV_SHOW, FALSE, NULL);
}


/*
 * Notify callback function for `box_save'.
 * this does the actual saving
 */
void
box_save_proc(item, event)
    Panel_item item;
    Event    *event;
{
    char     *fname = (char *) xv_get(file_win->s_fname, PANEL_VALUE);
    struct header newhead;
    struct logent *ent, *log_by_id();
    char      msg[80];
    XPoint    p1, p2;
    char     *data;
    FILE     *fp;
    int       rows, cols, x, y;

    ent = log_by_id(boxnum);
    if (ent == NULL) {
	(void) notice_prompt(rect_win->controls2, NULL,
			     NOTICE_FOCUS_XY, event_x(event), event_y(event),
			     NOTICE_MESSAGE_STRINGS,
			"Selected log entry is not a valid rectangle", NULL,
			     NOTICE_BUTTON_YES, "OK",
			     NULL);
	return;
    }
    if (ent->reg) {
	if (ent->reg->r_type != BOX) {
	    (void) notice_prompt(rect_win->controls2, NULL,
			    NOTICE_FOCUS_XY, event_x(event), event_y(event),
				 NOTICE_MESSAGE_STRINGS,
			"Selected log entry is not a valid rectangle", NULL,
				 NOTICE_BUTTON_YES, "OK",
				 NULL);
	    return;
	}
    } else {
	(void) notice_prompt(rect_win->controls2, NULL,
			     NOTICE_FOCUS_XY, event_x(event), event_y(event),
			     NOTICE_MESSAGE_STRINGS,
			"Selected log entry is not a valid rectangle", NULL,
			     NOTICE_BUTTON_YES, "OK",
			     NULL);
	return;
    }

    /* its a valid box number, so lets do it */

    fp = open_output_file(fname);

    bcopy((char *) &orig_img->head, (char *) &newhead, sizeof(struct header));

    p1.x = ent->reg->r_plist->pt.x;
    p1.y = ent->reg->r_plist->pt.y;
    p2.x = ent->reg->r_plist->next->pt.x;
    p2.y = ent->reg->r_plist->next->pt.y;

    cols = p2.x - p1.x;
    rows = p2.y - p1.y;
    newhead.orows = newhead.rows = rows;
    newhead.ocols = newhead.cols = cols;
    newhead.num_frame = 1;

    data = malloc(rows * cols * orig_img->dsize);
    switch (orig_img->dsize) {
    case 1:
	for (y = p1.y; y < p2.y; y++)
	    for (x = p1.x; x < p2.x; x++)
		*(data + ((y - p1.y) * cols * orig_img->dsize) + (x - p1.x)) =
		    (char) dval(x, y, orig_img, 0);
	break;
    case 2:
	for (y = p1.y; y < p2.y; y++)
	    for (x = p1.x; x < p2.x; x++)
		*((u_short *) data + ((y - p1.y) * cols + (x - p1.x))) =
		    (u_short) dval(x, y, orig_img, 0);
	break;
    case 4:
	for (y = p1.y; y < p2.y; y++)
	    for (x = p1.x; x < p2.x; x++)
		*((u_int *) data + ((y - p1.y) * cols + (x - p1.x))) =
		    (u_int) dval(x, y, orig_img, 0);
	break;
    }

    fwrite_header(fp, &newhead, fname);
    if (fwrite(data, rows * cols, orig_img->dsize, fp) < 1) {
	XBell(display, 0);
	sprintf(msg, "Error writing:%s: ", fname);
	(void) strcat(msg, sys_errlist[errno]);
	lab_info(msg, 1);
	fprintf(stderr, "%s\n", msg);
	return;
    }
    fclose(fp);
}

/***********************************************************************/

save_trace_vector(fp)
    FILE     *fp;
{
    char     *buffer, tstr[80];
    struct logent *tmp;
    struct trcontext *trace;
    int       log_size = 1000;
    struct header hd;
    int i, cnt=0;

    buffer = (char *) calloc(log_size, 1);

    for (tmp = loghead; tmp != NULL && tmp->reg != NULL; tmp = tmp->next) {
	trace = tmp->trace;
	if (trace != NULL) {
	    sprintf(tstr, "Trace # %d \n", cnt++);
	    for (i = 0; i < trace->npts; i++) {
		sprintf(tstr, "%d %d %d \n", trace->pbuf[i].pt.x,
			trace->pbuf[i].pt.y, trace->pbuf[i].oval);
		strcat(buffer, tstr);
		if (strlen(buffer) > log_size - 50) {
		    log_size += 1000;
		    buffer = (char *) realloc(buffer, log_size);
		}
	    }
	}
    }

    init_header(&hd, "genial-trace", "", 0, "", 0, 0, PFASCII, 1, "");
    setparam(&hd, "Genial-Trace", PFASCII, strlen(buffer) + 1, buffer);
    fwrite_header(fp, &hd, "genial trace log");
    fclose(fp);
}

/***********************************************************************/

save_histogram_vector(fp)
    FILE     *fp;
{
    char     *buffer, tstr[80];
    struct logent *tmp;
    struct hcontext *histo;
    int       log_size = 1000;
    struct header hd;
    int i, cnt=0;

  /* maybe should be using HIPS histogram format (PFHIST) */

    buffer = (char *) calloc(log_size, 1);

    for (tmp = loghead; tmp != NULL && tmp->reg != NULL; tmp = tmp->next) {
	histo = tmp->hist;
	if (histo != NULL) {
	    sprintf(tstr, "Histogram # %d \n", cnt++);
	    for (i = 0; i < NUM_BUCKETS; i++) {
		sprintf(tstr, "%d ", histo->countvec[i]);
		strcat(buffer, tstr);
		if (strlen(buffer) > log_size - 30) {
		    log_size += 1000;
		    buffer = (char *) realloc(buffer, log_size);
		}
	    }
	    strcat(buffer, "\n");
	}
    }

    init_header(&hd, "genial-histogram", "", 0, "", 0, 0, PFASCII, 1, "");
    setparam(&hd, "Genial-Histogram", PFASCII, strlen(buffer) + 1, buffer);
    fwrite_header(fp, &hd, "genial histogram");
    fclose(fp);
}

/***********************************************************************/

save_rasterfile(fp)
    FILE     *fp;
{
    struct rasterfile sunheader;
    int       i, y, nc, linesize;
    char     *line;
    XImage   *ximage;
    Window    win;
    Pixmap    pixmap;
    XColor    color_table[256];
    byte      rmap[256], gmap[256], bmap[256];

    win = RootWindow(display, DefaultScreen(display));
    pixmap = XCreatePixmap(display, win, orig_img->width, orig_img->height, 8);
    XCopyArea(display, img_win->d_xid, pixmap, gc, 0, 0,
	      orig_img->width, orig_img->height, 0, 0);

    if (!(ximage = XGetImage(display, pixmap, 0, 0, orig_img->width,
			     orig_img->height, (~0), ZPixmap))) {
	fprintf(stderr, "Error: XGetImage failed \n");
	return (-1);
    }
    /*
     * we could get fancy here and find only the colormap entries which are
     * used, and redo the ximage->data entries to reflect the compressed
     * colormap, but this works OK for now...  -BT
     */

    nc = 256;
    for (i = 0; i < 256; i++) {
	color_table[i].pixel = (unsigned long) i;
    }

    XQueryColors(display, XDefaultColormap(display, DefaultScreen(display)),
		 color_table, nc);	/* fills in color_table entries */

    for (i = 0; i < 256; i++) {
	rmap[i] = (byte) (color_table[i].red);
	gmap[i] = (byte) (color_table[i].green);
	bmap[i] = (byte) (color_table[i].blue);
#ifdef DEBUG
	fprintf(stderr, "color map entry (%d): r: %d, g: %d, b: %d \n",
		i, (int) rmap[i], (int) gmap[i], (int) bmap[i]);
#endif
    }
    linesize = orig_img->width;
    if (linesize % 2)
	linesize++;
    line = (char *) malloc(linesize);

    /* fill in Sun rasterfile header */
    sunheader.ras_magic = RAS_MAGIC;
    sunheader.ras_width = orig_img->width;
    sunheader.ras_height = orig_img->height;
    sunheader.ras_depth = 8;
    sunheader.ras_length = linesize * orig_img->height;
    sunheader.ras_type = RT_STANDARD;
    sunheader.ras_maptype = RMT_EQUAL_RGB;
    sunheader.ras_maplength = 3 * nc;
    fwrite(sunheader, sizeof(struct rasterfile), 1, fp);

    fwrite(rmap, sizeof(byte), nc, fp);
    fwrite(gmap, sizeof(byte), nc, fp);
    fwrite(bmap, sizeof(byte), nc, fp);

    /* write the image */
    line[linesize - 1] = 0;
    for (y = 0; y < orig_img->height; y++) {

	memcpy(line, ximage->data + y * ximage->bytes_per_line, orig_img->width);

	if (fwrite(line, sizeof(char), linesize, fp) != linesize) {
	    free(line);
	    return (-1);
	}
    }
    free(line);

    fclose(fp);
    return (0);
}

/*************************************************************/

FILE     *
open_output_file(fname)
    char     *fname;
{
    FILE     *fp;
    char      msg[80];
    int       result;
    Panel     panel = (Panel) xv_get(file_win->save, PANEL_PARENT_PANEL, NULL);

    if ((fp = fopen(fname, "r")) != NULL) {
	fclose(fp);
	result = notice_prompt(panel, NULL,
			       NOTICE_MESSAGE_STRINGS,
			       "Overwrite existing file?", NULL,
			       NOTICE_BUTTON_YES, "Yes",
			       NOTICE_BUTTON_NO, "No",
			       NULL);

	if (result == NOTICE_NO)
	    return (NULL);
    }
    if ((fp = fopen(fname, "w")) == NULL) {
	sprintf(msg, "Error opening:%s: ", fname);
	XBell(display, 0);
	(void) strcat(msg, sys_errlist[errno]);
	lab_info(msg, 1);
	fprintf(stderr, "%s\n", msg);
	return (NULL);
    }
    return (fp);
}
