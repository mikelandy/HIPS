/**************************************************************************
 *                                                                        * 
 * Function: max_grab()			       		                        *
 *                                                                        *
 * Usage:	   max_grab  filename		                                  *
 * Returns:  none							                        *
 * Defaults: none						                             *
 * Loads: cc -o -DDG max_grab max_grab.c -ldglib.a -lfslib.a              *
 *              -lmxlib.a -Imaxvideo/include -Ilocaldir/include           *
 * Modified:                                                              *
 *                                                                        *
 * Description:grab a  frame onto the Datacube Framestore FS0             *
 *             Connections should be as follows	     			    *
 *              DIGI             FRM                                      * 
 *              P3       ->      P3                                       * 
 *    CAMERA -> P12                                                       * 
 *    TV     <- P13                                                       * 
 *              P7       <-      P7                                       * 
 *              P10      ->      P10                                      * 
 *                                                           		    *
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
    
    /* initialize all boards */
    fsInit(fsfd, FS_T50);
    dgInit(dgfd, DG_UNSGD);	/* unsigned lut's */
    
    /* clear the 3 framestores using the constant register set to 0 */
    fsFastClear(fsfd, FS_FS0, BLACK);
    fsFastClear(fsfd, FS_FS1, BLACK);
    fsFastClear(fsfd, FS_FS2, BLACK);

    /* set DG gain to 4 */
    /* dgSelAnGain(dgfd, DG_4DB);*/
	    
    /* aquire an image in FS0 */
    fs0Acquire(fsfd);
    fs0WaitFrm(fsfd);
    fs0Freeze(fsfd);

    if (fsClose(fsfd) == NOOK)
	printf("FRAMESTORE, problems with freeing the memory\n");
    if (dgClose(dgfd) == NOOK)
	printf("DIGIMAX, problems with freeing the memory\n");
}
