/*   The Scry system is copyright (C) 1988, 1989, Regents  of  the
*University  of  California.   Anyone may reproduce ``Scry'',
*the software in this distribution, in whole or in part, pro-
*vided that:
*
*(1)  Any copy  or  redistribution  of  Scry  must  show  the
*     Regents  of  the  University of California, through its
*     Lawrence Berkeley Laboratory, as the source,  and  must
*     include this notice;
*
*(2)  Any use of this software must reference this  distribu-
*     tion,  state that the software copyright is held by the
*     Regents of the University of California, and  that  the
*     software is used by their permission.
*
*     It is acknowledged that the U.S. Government has  rights
*in  Scry  under  Contract DE-AC03-765F00098 between the U.S.
*Department of Energy and the University of California.
*
*     Scry is provided as a professional  academic  contribu-
*tion  for  joint exchange.  Thus it is experimental, is pro-
*vided ``as is'', with no warranties of any kind  whatsoever,
*no  support,  promise  of updates, or printed documentation.
*The Regents of the University of California  shall  have  no
*liability  with respect to the infringement of copyrights by
*Scry, or any part thereof.
*
*For information contact:
*	Bill Johnston
*	Information and Computing Sciences Division
*	Lawrence Berkeley Laboratory
*	Berkeley, CA 94720
*	wejohnston@lbl.gov
*/

/* hipstool.c
 * Max Rible           2/8/1989
 * Read in HIPS files and stick them on Sun screens.
 * Don't use funky HIPS programs.
 */

#include "hipstool.h"

int winx, winy;			/* Frame coordinates */
int auxiliary;			/* source: 0: primary image 1: auxiliary */
int resize;			/* 0: don't resize image on load 1: resize */
char *infilename, *outfilename;

static int parse_args();
static void init_vars();

static FILE *input;
static int noread;

main(argc, argv)
     int argc;
     char *argv[];
{
    Progname = strsave(*argv);
    init_vars();

    parse_args(argc, argv);

    window_environment_init(argc, argv);

    if((infilename != NULL) || !noread) {
	(load_menu_funcs[LOAD_LOG_FILE]).active = 1;
	load_menu_funcs[LOAD_COMMENT_FILE].active = 1;
	update_load_menu();

	load_image(&base, BASE_FRAME, (FILE *) -1);
    }

#ifdef SUNTOOLS
    window_main_loop(io.control);
#endif
}

static int
parse_args(argc, argv)
     int argc;
     char *argv[];
{
    int i;

    for(i = 1; i < argc; i++) {
	if(argv[i][0] == '-') switch(argv[i][1]) {
	case 'i':
	    infilename = argv[++i];
	    break;
	case 'o':
	    outfilename = argv[++i];
	    break;
#ifdef X_WINDOWS
	case 'd':
	    display_name = argv[++i];
	    break;
#endif
	case 'N':		/* Compatibility with old */
	case 's':
	    break;
	case 'h':		/* Help! */
	default:
puts("HIPStool help:");
puts("-i infile      Read from infile.");
puts("-o outfile     Write to outfile.");
#ifdef X_WINDOWS
puts("-d display     Use X display.");
#endif
	    exit(0);
	}
    }
}


static void
init_vars()
{
    int i;

    for(i = 0; i <= MAXCOLOR; i++)	/* Initialize color ramps */
	red[i] = green[i] = blue[i] = tr[i] = i;
    red[0] = green[0] = blue[0] = SUN_BLACK;
    red[STANDOUT] = 0; green[STANDOUT] = 255; blue[STANDOUT] = 0;
    red[SUN_WHITE] = green[SUN_WHITE] = blue[SUN_WHITE] = 0xFF;
    red[SUN_BLACK] = green[SUN_BLACK] = blue[SUN_BLACK] = 0;
    tr[0] = SUN_BLACK;
    tr[SUN_BLACK] = 0;
    tr[STANDOUT] = tr[STANDOUT_MAP];

#ifdef SUNTOOLS
    base.winfo.pw = NULL;
    proj.winfo.canvas = base.winfo.canvas = NULL;
    base.winfo.vbar = base.winfo.hbar = NULL;
    proj.winfo.vbar = proj.winfo.hbar = NULL;
#endif

#ifdef X_WINDOWS
    display_name = NULL;
#endif

    winx = winy = 100;
    infilename = outfilename = NULL;
    proj.virgin = base.virgin = 1;
    input = NULL;
    auxiliary = 0;
    resolution = 1;
    resize = 1;
    noread = 1;

    return;
}
