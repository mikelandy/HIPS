/**************************************************************************
 *                                                                        * 
 * Function: menu_demo()					                             *
 *                                                                        *
 * Usage:	   max_demo              		                             *
 * Returns:  none							                        *
 * Defaults: none						                             *
 * Loads: cc -c -DDG menu_demo menu_demo.o fsMenu.o dgMenu.o -ldglib.a    *
 *           -lfslib.a -lmxlib.a -Imaxvideo/include -Ilocaldir/include    *
 * Modified:Tony 5-VIII-87                                                *
 *                                                                        *
 * Description:This example menu program uses the Digimax and Framestore  *
 *             main application module  using fsMenu.c dgMenu.c functions *
 *             Connections should be as follows	     			    *
 *              DIGI             FRM                                      * 
 *              P3       ->      P3                                       * 
 *    CAMERA -> P12                                                       * 
 *    TV     <- P13                                                       * 
 *              P7       <-      P7                                       * 
 *              P10      ->      P10                                      * 
 **************************************************************************
 *                    Copyright (c) 1987                                  *
 *                    Captain Chaos                                       *
 **************************************************************************
 */
#include <fsHead.h>
#include <dgHead.h>
#include <menudefs.h>

main()
{
    
    FS_DESC *fsfd;	/* FRAMSTORE device descriptor */
    DG_DESC *dgfd;	/* DIGIMAX device descriptor */
    char opt;		/* user option */
    int verbose;	/* verbosity flag - set for printf's */
    
    /* initialize parameters */
    verbose = 0;

    printf("$State: Release2-0 $\n\n");
    /* Allocate memory for the boards' register structures */
    fsfd = fsOpen(FS_BASE, verbose);
    dgfd = dgOpen(DG_BASE, verbose);
    
    /* initialize all boards */
    fsInit(fsfd, FS_T50);
    dgInit(dgfd, DG_UNSGD);	/* unsigned lut's */
    
    /* clear the 3 framestores using the constant register set to 0 */
    fsFastClear(fsfd, FS_FS0, BLACK);
    fsFastClear(fsfd, FS_FS1, BLACK);
    fsFastClear(fsfd, FS_FS2, BLACK);

    printf("MaxVideo Application Software, type 'h' for help\n");

    /* perform operation specified by user input */
    do {
	printf(" > ");
	opt = getchar();
	switch (opt)
	{
	    case HELP:
		printf("h > help menu\n");
		printf("f > FRAMESTORE\n");
		printf("d > DIGIMAX\n");
		printf("q > quit\n");
		break;
	    case DIGI:
		dgApp(fsfd, dgfd);
		break;
	    case FRAME:
		fsApp(fsfd, dgfd);
		break;
	    case QUIT:
		exit(0);
	}
    } while (opt != QUIT);
    if (fsClose(fsfd) == NOOK)
	printf("FRAMESTORE, problems with freeing the memory\n");
    if (dgClose(dgfd) == NOOK)
	printf("DIGIMAX, problems with freeing the memory\n");
}

