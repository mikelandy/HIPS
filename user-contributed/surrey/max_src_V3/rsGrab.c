/**************************************************************************
 *                                                                        * 
 * Function:  rsGrab()		       		 	                        *
 *                                                                        *
 * Usage:	    rsGrab  			          	                        *
 * Returns:  none							                        *
 * Defaults: none						                             *
 * Loads: cc -o -DPOLLED rsGrab rsGrab.c -ldgplib.a -lrsplib.a            *
 *              -lmvlib.a -Imaxvideo/include -Ilocaldir/include           *
 * Modified:5-IV-88                                                       *
 *                                                                        *
 * Description: Grab an full image into ROISTORE LS from the DIGIMAX      *
 *              FOR POLLED MODE ONLY        	     				    *
 *              Connections should be as follows	     			    *
 *              DIGI             ROI                                      * 
 *              P3       ->      P3                                       * 
 *    CAMERA -> P12                                                       * 
 *    TV     <- P13                                                       * 
 *              P7       <-      P7                                       * 
 *                       ->      P9                                       * 
 *              P10      ->      P10                                      * 
 **************************************************************************
 *                    Copyright (c) 1988                                  *
 *                    Captain Chaos 		V.3 MAX_WARE                 *
 **************************************************************************
 */

#include <dqHead.h>
#include <mvIntDefs.h>
#include <dgHead.h>
#include <rsHead.h>
#include <maxdefs.h>

main()
{
    
    RS_DESC *rsfd;	/* ROISTORE device descriptor */
    DG_DESC *dgfd;	/* DIGIMAX device descriptor */
    /* int RS_T50 = 0;	 */
    
    /* Allocate memory for the boards' register structures */
    rsfd = rsOpen(RS_RBASE, RS_MBASE, RS_M2048, RS_WVECTOR, DQ_ON);
    dgfd = dgOpen(DG_BASE, DG_VECTOR, DQ_ON);
    
    /* Initialise Digimax board */
    dgInit(dgfd, DG_UNSGD);
    /* Initialise ROISTORE board and grab a frame*/
    rsIn512(rsfd, DQ_M512, RS_T50);

    if (rsClose(rsfd) == NOOK)
	printf("ROISTORE, problems with freeing the memory\n");
    if (dgClose(dgfd) == NOOK)
	printf("DIGIMAX, problems with freeing the memory\n");
}
