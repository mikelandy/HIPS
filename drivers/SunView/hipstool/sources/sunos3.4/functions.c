/* functions.c
 * Max Rible
 *
 * Setting up all the various functions available in hipstool.
 */

#include "hipstool.h"

Command *cur_func;		/* Current function */

double theta;			/* Angle */

unsigned char red[256], green[256], blue[256], tr[256];
char cursortext[CURS_LEN], windowtext[INFO_LEN], infotext[2][100];
Bool complete;
int resolution;			/* Grid resolution */
int current_action_num = -1;	/* Index into log of edited action */
int auto_log = 0;
int line_drawing_mode = CLICK_AND_CLICK;

FileInfo base, proj;

Function functions[MAX_FUNCTIONS] = {
/*    { name, breed,
	  eval, setup, comment,
	  { primitive, number },
	  right_eval }, */
    { "Box", FUNC_BOX, 
	  box_eval, box_setup, straight_comment,
	  { PRIMIT_BOX, 1 },
	  1, },
    { "Trace", FUNC_TRACE, 
	  trace_eval, trace_setup, straight_comment,
	  { PRIMIT_NIL, 0 }, 
	  0, },
    { "Histogram", FUNC_HISTO, 
	  histo_eval, histo_setup, straight_comment,
	  { PRIMIT_NIL, 0 }, 
	  0, },
    { "Distance", FUNC_DISTANCE, 
	  distance_eval, distance_setup, distance_comment,
	  { PRIMIT_POINT, 2 }, 
	  1, },
    { "Angle measure", FUNC_ANGLE, 
	  angle_eval, angle_setup, angle_comment,
	  { PRIMIT_LINE, 2 }, 
	  1, }
};

