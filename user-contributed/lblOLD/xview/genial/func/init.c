/*
 * main initialization routines for GENIAL
 *
 */

#include <stdio.h>
#include "display.h"
#include "ui.h"
#include "sm.h"


char     *in_image = NULL;

main(argc, argv)
    int       argc;
    char     *argv[];
{
    void      parse_args();

    Progname = strsave(*argv);
    hipserrlev = HEL_SEVERE;	/* only exit if severe errors */
    hipserrprt = HEL_ERROR;	/* print messages for hips errors */

    parse_args(argc, argv);

    /*
     * Initialize XView.
     */
    xv_init(XV_INIT_ARGS, argc, argv, NULL);

    /* initialize user interface */
    init_ui();

    /* initialize the colormap */
    cmap_init();

    /* initialize state machine */
    init_sm();

    /* initialize log */
    init_log();

    /* lets start... */
    rdom_linear();

    /* load file if given as command line arguement */
    if (in_image != NULL) {
	if (load_image(in_image) == 1) {
	    fxn_init();
	    new_state(IMG_LOADED);
	}
    }
    xv_main_loop(base_win->ctrlwin);
    exit(0);
}

/***********************************************************/
shutdown()
{

#ifdef DEBUG
    printf("shutting down\n");
#endif

    exit(0);
}

/***********************************************************/
void
parse_args(argc, argv)
    int       argc;
    char     *argv[];
{
    void      usageterm();

    int       verbose = 0;

    /* Interpret options  */
    while (--argc > 0 && (*++argv)[0] == '-') {
	char     *s;
	for (s = argv[0] + 1; *s; s++)
	    switch (*s) {
	    case 'i':
		if (argc < 2)
		    usageterm();
		in_image = *++argv;
		fprintf(stderr, " using image file: %s\n", in_image);
		argc--;
		break;
	    case 'v':
		verbose++;
		break;
	    case 'h':
		usageterm();
		break;
	    default:
		usageterm();
		break;
	    }
    }
}

/******************************************************/
void
usageterm()
{
    fprintf(stderr, "Usage: genial [-i image][-v][-h][-help] \n");
    fprintf(stderr, "        [-i HIPS file] load specified image file \n");
    fprintf(stderr, "        [-v] verbose mode (not yet implemented) \n");
    fprintf(stderr, "        [-h] this list \n");
    fprintf(stderr, "        [-help] list of window attribute help \n\n");
    exit(0);
}
