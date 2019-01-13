/* hipstool.h
 * Max Rible
 * Header file for hipstool.
 */

/* Conditional compilation flags.
 */
#define USE_MMAP		/* Just mmap into memory, don't fread */
#define MULTO_FRAMES		/* Allow multiple frames */
#define SUNTOOLS		/* Compile to run under suntools */
/*#define X_WINDOWS		/* Compile to run under X windows */
/*#define XVIEW			/* Compile to run under Xview */
/*#define MACROIZED		/* Use macroized primitives when possible */
#define SUNVIEW_4_0		/* Running SunView Release 4.0 */

/* General header files to include.
 */
#include <stdio.h>
#include <math.h>		/* pow(), log(), sqrt(), etc. */
#include <sys/types.h>		/* typedefs */
#include <sys/stat.h>		/* For fstat */
#include <varargs.h>		/* For varargs */
#ifdef USE_MMAP
#include <sys/mman.h>		/* mmap */
#endif

/* HIPS header file */
#include <hipl_format.h>	/* HIPS header declaration */

/* Window-system specific header files */
#ifdef SUNTOOLS
#include <suntool/sunview.h>	/* General */
#include <suntool/canvas.h>
#include <suntool/panel.h>
#include <suntool/scrollbar.h>
#include <suntool/seln.h>
/* Header files that aren't specified but one includes them to make lint
 * shut up.
 */
#include <suntool/tool_struct.h>
#include <sunwindow/win_lock.h>
#include <pixrect/memreg.h>
/* #include <pixrect/cg1reg.h> */
/* #include <pixrect/cg2reg.h> */
/* #include <pixrect/bw1reg.h> */
#include <sys/uio.h>
#endif

#ifdef X_WINDOWS
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xresource.h>
#include <X11/Xos.h>
#endif

/* Definitions for mucking with on installation.
 */
#define BOLD_FONT "/usr/lib/fonts/fixedwidthfonts/screen.b.12"
#define SNAPDIST 5		/* Snap to line endpoint */
#define SCREEN_WIDTH 1152	/* Dimensions for standard Sun monitor */
#define SCREEN_HEIGHT 900
#define CROSS_RADIUS 13		/* Size of one arm */
#define BAR_SIZE 20		/* Bar of colors for trace graph */

#define HEADER_LOG_DATA_NAME "HIPStool log"
#define HEADER_COMMENT_DATA_NAME "HIPStool comments"

#include "defines.h"

/* Note on "macros.h":  the strdup function is apparently not part of
 * standard UNIX.  However, all calls to it are given by the
 * Strdup macro, which can be changed to a call to any function that,
 * given a string, will return a pointer to a new copy of that string.
 */
#include "macros.h"

#include "structs.h"

#include "externs.h"
