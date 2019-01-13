/* defines.h
 * Max Rible
 *
 * Various and sundry definitions used throughout HIPStool.
 * This file is included in "hipstool.h".
 */

/* Sun screen colors */
#define STANDOUT 1		/* Colormap indices */
#define STANDOUT_MAP 0		/* Map level 1 to 0 */
#define SUN_WHITE 254
#define SUN_BLACK 255
#define ALMOST_BLACK 2
#define MAXCOLOR 253

enum truth {
    OFF, ON, TEST
    };

enum warp_option {
    WARP_ABLE, WARP_DELETE, WARP_SELECT, WARP_MODIFY
    };

enum do_spline_option {
    DO_SPLINE_OPEN, DO_SPLINE_CLOSED
    };

/* Useful definitions in designing borders--- I don't trust Sunfools */
#define TOP_BORDER 18
#define BORDER 5
#define MARG_X 2*BORDER		/* Suntools borders (left + right) */
#define MARG_Y TOP_BORDER + BORDER /* Suntools borders (top + bottom) */
#define MAX_WINX ((SCREEN_WIDTH - 2*MARG_X)/2)
#define MAX_WINY ((SCREEN_HEIGHT - 2*MARG_Y)/2)

enum which_frame {
    BASE_FRAME, CHILD_FRAME
    };

/* Graphics primitives */
enum primit {
    PRIMIT_NIL = -1,
    PRIMIT_LINE = 0, PRIMIT_BOX, PRIMIT_OPEN_SPLINE, PRIMIT_CLOSED_SPLINE,
    PRIMIT_POLYGON, PRIMIT_POINT, PRIMIT_TEXT, 
    NUM_PRIMITS
    };

/* Data storage in primitives */
enum primit_data_type {
    DATA_NONE = 0, DATA_TRACE, DATA_CROSS, DATA_TEXT, DATA_BOX
    };

/* General modes */
enum function_index {
    FUNC_BOX, FUNC_TRACE, FUNC_HISTO, FUNC_DISTANCE, FUNC_ANGLE,
    MAX_FUNCTIONS
};

#define VARIES -1		/* Indicates number of pairs variable */
#define NONPRESENT -1		/* Indicates not on the cycle */
#define XOR_COLOR -1		/* A noncolor */

/* Load modes */
enum load_mode {
    LOAD_HIPS_IMAGE, LOAD_LOG_FILE, LOAD_COMMENT_FILE, 
    NUM_LOAD_FUNCS
    };

/* Saving modes */
enum save_mode {
    SAVE_COMPLETE_IMAGE, SAVE_LOGGED_IMAGE, SAVE_BOX_SUBIMAGE,
    SAVE_TRACE_FILE, SAVE_HISTO_FILE, SAVE_LOG_FILE, SAVE_OVERLAID_IMAGE,
    SAVE_COMMENT_FILE, 
    NUM_SAVE_FUNCS
    };

/* Drawing modes */
enum draw_mode {
    DRAW_IN_PIXWIN, DRAW_AND_TRACE, DRAW_IN_MEMORY, 
    NUM_DRAWING_MODES
    };

/* Buttons */
enum input_index {
    INPUT_EVAL, INPUT_CLEAR, INPUT_SELECT, INPUT_START, INPUT_LOG, 
    MAX_INPUTS
    };

/* Messages */
enum message_index {
    MESSAGE_CURSOR, MESSAGE_SELECTION, MESSAGE_INFO_1, MESSAGE_INFO_2,
    MESSAGE_FRAME_NUMBER, 
    MAX_MESSAGES
    };

/* Text options for histo, trace functions */
#define TXTWID 43		/* should be 5*(width(character)) */
#define TXTHT 15		/* should be height(character) */

/* Different types for copy, destroy functions. */
enum copy_destroy_type {
    TYPE_COMMAND = 1, TYPE_PRIMIT, TYPE_POINTLIST,
    TYPE_PRIMITLIST, TYPE_HIPSHEAD, TYPE_LOGENTRY
    };

enum line_select_mode {
    CLICK_AND_CLICK = 1, CLICK_AND_DRAG, LEAST_SQUARES_FIT
    };

enum edit_mode {
    EDIT_LOG, EDIT_COLORMAP, EDIT_HEADERTEXT
    };

enum menu_return {
    RETURN_PRIMIT, RETURN_FUNCTION, RETURN_EDIT
    };
