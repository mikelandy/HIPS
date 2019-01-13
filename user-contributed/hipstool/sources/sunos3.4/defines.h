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

#define	OFF	0
#define	ON	1
#define	TEST	2

#define	WARP_ABLE	0
#define	WARP_DELETE	1
#define	WARP_SELECT	2
#define	WARP_MODIFY	3

#define	DO_SPLINE_OPEN	0
#define	DO_SPLINE_CLOSED 1

/* Useful definitions in designing borders--- I don't trust Sunfools */
#define TOP_BORDER 18
#define BORDER 5
#define MARG_X 2*BORDER		/* Suntools borders (left + right) */
#define MARG_Y TOP_BORDER + BORDER /* Suntools borders (top + bottom) */
#define MAX_WINX ((SCREEN_WIDTH - 2*MARG_X)/2)
#define MAX_WINY ((SCREEN_HEIGHT - 2*MARG_Y)/2)

#define	BASE_FRAME	0
#define	CHILD_FRAME	1

/* Graphics primitives */
#define	PRIMIT_NIL	-1
#define	PRIMIT_LINE	0
#define	PRIMIT_BOX	1
#define	PRIMIT_OPEN_SPLINE 2
#define	PRIMIT_CLOSED_SPLINE 3
#define	PRIMIT_POLYGON	4
#define	PRIMIT_POINT	5
#define	PRIMIT_TEXT	6 
#define	NUM_PRIMITS	7

/* Data storage in primitives */
#define	DATA_NONE	0
#define	DATA_TRACE	1
#define	DATA_CROSS	2
#define	DATA_TEXT	3
#define	DATA_BOX	4

/* General modes */
#define	FUNC_BOX	0
#define	FUNC_TRACE	1
#define	FUNC_HISTO	2
#define	FUNC_DISTANCE	3
#define	FUNC_ANGLE	4
#define	MAX_FUNCTIONS	5

#define VARIES -1		/* Indicates number of pairs variable */
#define NONPRESENT -1		/* Indicates not on the cycle */
#define XOR_COLOR -1		/* A noncolor */

/* Load modes */
#define	LOAD_HIPS_IMAGE	0
#define	LOAD_LOG_FILE	1
#define	LOAD_COMMENT_FILE 2
#define	NUM_LOAD_FUNCS	3

/* Saving modes */
#define	SAVE_COMPLETE_IMAGE	0
#define	SAVE_LOGGED_IMAGE	1
#define	SAVE_BOX_SUBIMAGE	2
#define	SAVE_TRACE_FILE		3
#define	SAVE_HISTO_FILE		4
#define	SAVE_LOG_FILE		5
#define	SAVE_OVERLAID_IMAGE	6
#define	SAVE_COMMENT_FILE	7 
#define	NUM_SAVE_FUNCS		8

/* Drawing modes */
#define	DRAW_IN_PIXWIN	0
#define	DRAW_AND_TRACE	1
#define	DRAW_IN_MEMORY	2
#define	NUM_DRAWING_MODES 3

/* Buttons */
#define	INPUT_EVAL	0
#define	INPUT_CLEAR	1
#define	INPUT_SELECT	2
#define	INPUT_START	3
#define	INPUT_LOG	4 
#define	MAX_INPUTS	5

/* Messages */
#define	MESSAGE_CURSOR	0
#define	MESSAGE_SELECTION 1
#define	MESSAGE_INFO_1	2
#define	MESSAGE_INFO_2	3
#define	MESSAGE_FRAME_NUMBER 4 
#define MAX_MESSAGES	5

/* Text options for histo, trace functions */
#define TXTWID 43		/* should be 5*(width(character)) */
#define TXTHT 15		/* should be height(character) */

/* Different types for copy, destroy functions. */
#define	TYPE_COMMAND	1
#define	TYPE_PRIMIT	2
#define	TYPE_POINTLIST	3
#define	TYPE_PRIMITLIST	4
#define	TYPE_HIPSHEAD	5
#define	TYPE_LOGENTRY	6

#define	CLICK_AND_CLICK	1
#define	CLICK_AND_DRAG	2
#define	LEAST_SQUARES_FIT 3

#define	EDIT_LOG	0
#define	EDIT_COLORMAP	1
#define	EDIT_HEADERTEXT	2

#define	RETURN_PRIMIT	0
#define	RETURN_FUNCTION	1
#define	RETURN_EDIT	2
