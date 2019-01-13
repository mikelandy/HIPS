/*
 * definitions necessary for the state machine UI paradigm
 */

/* unique identifiers for UI events which may trigger a state transition */
#define LOAD 1    /* image loaded */
#define QUIT 2    /* quit button */
#define FORW 3    /* next frame */
#define BACK 4    /* previous frame */
#define IMG_BUT 5 /* button pushed in image window */
#define FMENU 6   /* new function selected */
#define CLEAR 7   /* clear button pushed */
#define EVAL 8    /* eval button pushed */

/* possible states */
#define IMG_UNLOADED 1
#define IMG_LOADED 2
#define REG_SEL 3   /* region selected = true */
#define REG_EDIT 4  /* edit mode */

extern int state;
