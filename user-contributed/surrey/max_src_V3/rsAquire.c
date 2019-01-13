/**************************************************************************
 *                                                                        * 
 * Function:  rsAquire()		       		                       	    *
 *                                                                        *
 * Usage:	    rsAquire  			                                	    *
 * Returns:  none							                        *
 * Defaults: none						                             *
 * Loads: cc -o -DPOLLED rsAquire rsAquire.c -ldgplib.a -lrsplib.a        *
 *              -lmvlib.a -Imaxvideo/include -Ilocaldir/include           *
 * Modified:                                                              *
 *                                                                        *
 * Description:Acquire an active 512x512 frame in LS of roistore          *
 *             Display active frame LS -> P7 -> DG     			    *
 *              FOR POLLED MODE ONLY        	     				    *
 *              Connections shoould be as follows	     			    *
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
    /* int RS_T50 = 0; */
    
    rsfd = rsOpen(RS_RBASE, RS_MBASE, RS_M2048, RS_WVECTOR, DQ_ON);
    dgfd = dgOpen(DG_BASE, DG_VECTOR, DQ_ON);
    
    /* Initialise boards */
    dgInit(dgfd, DG_UNSGD);
    rsInaqr(rsfd, DQ_M512, RS_T50);
    
    if (rsClose(rsfd) == NOOK)
	printf("ROISTORE, problems with freeing the memory\n");
    if (dgClose(dgfd) == NOOK)
	printf("DIGIMAX, problems with freeing the memory\n");
}
