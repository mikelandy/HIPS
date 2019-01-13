/**************************************************************************
 *                                                                        * 
 * Function: max_clear()			       		                        *
 *                                                                        *
 * Usage:	   max_clear  filename		                                  *
 * Returns:  none							                        *
 * Defaults: none						                             *
 * Loads: cc -o -DDG max_clear max_clear.c -ldglib.a -lfslib.a            *
 *              -lmxlib.a -Imaxvideo/include -Ilocaldir/include           *
 * Modified:                                                              *
 *                                                                        *
 * Description:clear  Datacube Framestore                                 *
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
#include <maxdefs.h>

main()
{
    
    FS_DESC *fsfd;	/* FRAMSTORE device descriptor */
    DG_DESC *dgfd;	/* DIGIMAX device descriptor */
    char opt;		/* user option */
    int verbose;	/* verbosity flag - set for printf's */
    
    /* initialize parameters */
    verbose = 0;

    /* Allocate memory for the boards' register structures */
    fsfd = fsOpen(FS_BASE, verbose);
    dgfd = dgOpen(DG_BASE, verbose);
    
    /* clear the 3 framestores using the constant register set to 0 */
    fsFastClear(fsfd, FS_FS0, BLACK);
    fsFastClear(fsfd, FS_FS1, BLACK);
    fsFastClear(fsfd, FS_FS2, BLACK);

    if (fsClose(fsfd) == NOOK)
	printf("FRAMESTORE, problems with freeing the memory\n");
    if (dgClose(dgfd) == NOOK)
	printf("DIGIMAX, problems with freeing the memory\n");
}
