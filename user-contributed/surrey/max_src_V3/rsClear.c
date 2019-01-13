/**************************************************************************
 *                                                                        * 
 * Function:  rsclear()		       		 	                        *
 *                                                                        *
 * Usage:	    rsclear  			          	                        *
 * Returns:  none							                        *
 * Defaults: none						                             *
 * Loads: cc -o -DPOLLED rsclear rsclear.c -ldgplib.a -lrsplib.a              *
 *              -lmvlib.a -Imaxvideo/include -Ilocaldir/include           *
 * Modified:                                                              *
 *                                                                        *
 * Description:clear the roistore with a set value                        *
 *              FOR POLLED MODE ONLY        	     				    *
 *              Connections NOT NESSECARY	           			    *
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

    /* Allocate memory for the boards' register structures */
    rsfd = rsOpen(RS_RBASE, RS_MBASE, RS_M2048, RS_WVECTOR, DQ_ON);
    dgfd = dgOpen(DG_BASE, DG_VECTOR, DQ_ON);
 
    /* clear LS & MS using the constant register set to 0 */
    rsClrDisp(rsfd, RS_LS, 0);
    rsClrDisp(rsfd, RS_MS, 0);
     
    if (rsClose(rsfd) == NOOK)
	printf("ROISTORE, problems with freeing the memory\n");
    if (dgClose(dgfd) == NOOK)
	printf("DIGIMAX, problems with freeing the memory\n");
}
