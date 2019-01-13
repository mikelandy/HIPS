/* structs.h
 * Max Rible
 *
 * Structure definitions and typedefs for HIPStool.
 * This file is included in "hipstool.h".
 */

typedef unsigned Bool;

/* The interface structure is a handy place to keep all the flotsam
 * and jetsam of managing the main window input/output interface.
 * It includes all the various structures for buttons and panels
 * and toggles and cycles..
 */
struct interface {
#ifdef SUNTOOLS
    Frame control;		/* Base frame for tool */
    Panel panel;		/* Panel on base frame */
    Pixfont *font;
    /* Information interaction */
    Panel_item 
	input,			/* Input filename */
	output,			/* Output filename */
	command,		/* Commands */
	comment;		/* Comments */
    Panel_item messages[MAX_MESSAGES];
    /* User interaction */
    Panel_item toggles, inputs[MAX_INPUTS];
    Panel_item 
	rescycle,		/* Grid snap resolution */
	source;			/* Which window to read from */
#ifdef MULTO_FRAMES
    struct { 
	Panel_item forward, back;
    } page;
#endif
    Menu load, save;		/* Load and save buttons */
    Menu rootmenu;		/* Control */
#endif
#ifdef X_WINDOWS
    struct {
	Display display;
	char *name;
	int screen;
    } display;
    Window control;
#endif
};

/* This is just a basic window information structure that can
 * be legitimately passed around so programs can do operations
 * in them.
 */
typedef struct windowinfo {
#ifdef SUNTOOLS
    Frame frame;
    Canvas canvas;
    Pixwin *pw;
    Pixrect *pr;
    Scrollbar vbar, hbar;
#endif    
#ifdef X_WINDOWS
    Window window;
    XSizeHints size_hints;
    GC gc;
    XGCValues gcvalues;
#endif
    int width, height;
} Windowinfo;

/* This is a descriptor of the actual window on the data.
 */
typedef struct sundisplay {
    Windowinfo winfo;
    unsigned short *image;
    union {
	unsigned char *chars;
	unsigned short *shorts;
	unsigned long *longs;
    } buf;
    unsigned char *user_lut;	/* 0 to extremes[1] */
#ifdef USE_MMAP
    struct {
	caddr_t addr;
	int len;
    } map;
#endif
#ifdef MULTO_FRAMES
    struct {
	union {
	    unsigned char *chars;
	    unsigned short *shorts;
	    unsigned long *longs;
	} base;
	int num, current;
    } frames;
#endif
    struct header hips;
    unsigned extremes[2];	/* Extreme values */
    unsigned datasize:3;	/* 0-4 */
    Bool virgin:1;
} FileInfo;

/* Structure of a basic function like box histogram, line trace,
 * find angle, &c.  Pointers to functions galore.
 */
typedef struct function {
    char *name;
    enum function_index breed;
    void (*eval)();
    void (*setup)();
    char *(*comment)();
    struct {
	enum primit breed;
	int num;
    } primit;
    Bool right_eval:1;
} Function;

#ifdef OLD
typedef struct function {
    char *name;			/* Function name */
    int cycle_number;		/* Index on cycle of selections */
    int num_pairs;		/* # pairs of numbers attached */
    struct {			/* See input_proc() */
	char *instruction;
	char *info[2];
    } start;
    struct {
	enum primit breed;
	int num;
    } primit;
    void (*eval)();		/* evals.c: (void) */
    void (*clear)();		/* clears.c: (void) */
    char *(*comment)();
    void (*setup)();
    /* Flags */
    Bool loggable:1;		/* Can put in log */
    Bool right_eval:1;		/* Eval on right-click */
} Function;
#endif

/* Basic structure holding menus together.
 */
struct menu_entry {
    char *name;			/* Name of menu function */
    Bool active:1;		/* Whether it can be called or not*/
    void (*action)();		/* Function it calls */
};

/* Primitive structure for point display.
 */
typedef struct cross {
    int x;
    int y;
    unsigned char values[4*CROSS_RADIUS + 1];
} Cross;

/* Primitive structure for creation of lists of points.  Each point
 * has an associated cross.
 * The prev pointer of the head of the list points to the tail of
 * the list.  This helps in playing connect-the-dots.
 */
typedef struct point *Point;
#define NULLPOINT ((Point) 0)
struct point {
    Cross i;
    Point next, prev;
};

typedef struct tracedata {
    unsigned *info[3];
    unsigned length;
} Trace;

typedef struct primitive Primit;
typedef struct primitive {
    enum primit breed;
    struct {
	int (*coords)[2];	/* Reconstruction information */
	Point list;
    } loc;
    int len;			/* Amount of data present in loc */
    union {
	Trace trace;		/* PRIMIT_LINE */
	Trace traces[4];	/* PRIMIT_BOX */
	Cross cross;		/* PRIMIT_POINT */
	char *txt;		/* PRIMIT_TEXT */
    } data;
    Primit *next;		/* (listp == 1) <==> (next != NULL) */
    Bool listp:1;		/* If primitive is in a list */
    Bool auxiliary:1;		/* auxiliary data present in union "data" */
};

typedef struct command {
    struct {			/* We can add other stuff later */
	Primit *primit;
	int len;
    } func;
    struct {
	Primit *primit;
	int len;
    } anno;
    unsigned *histo;
    char *comment;
    int frame;
    enum function_index breed;
} Command;

struct primit_info {
    char *name;
    int loc_size;		/* Number if fixed */
    enum primit_data_type data_type; /* traces/crosses/text */
    Bool drag_create;		/* Drag on selection */
    void (*select)();		/* Select function */
    int (*modify)();		/* Modify function */
    void (*display)();		/* Display function */
};

/* Basic structure for a log entry.
 */
typedef struct logentry *Log;
#define NULLOG ((Log) 0)

struct logentry {
    Command *command;		/* Complete command copy */
    enum function_index breed;
    int action_num;		/* Editing index */
    int frame;			/* Frame # */
    Bool active:1;		/* Display on presentation or not */
    Log next;			/* Next log entry */
};

struct function_menu_returned_information {
    enum primit breed;
    int num;
    enum function_index function;
    enum edit_mode edit;
};
