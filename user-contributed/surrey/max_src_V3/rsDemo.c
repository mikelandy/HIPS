/**************************************************************************
 *                                                                        * 
 * Function:  rsDemo()		       		 	                        *
 *                                                                        *
 * Usage:	    rsDemo  			          	                        *
 * Returns:  none							                        *
 * Defaults: none						                             *
 * Loads: cc -o -DPOLLED rsDemo rsDemo.c -ldgplib.a -lrsplib.a            *
 *              -lmvlib.a -Imaxvideo/include -Ilocaldir/include           *
 * Modified:                                                              *
 *                                                                        *
 * Description: Demonstration of the functions of the ROI and DIGI board  *
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
    /* int RS_T50 = 0; */	/* verbosity flag - set for printf's */
    
    /* Allocate memory for the boards' register structures */
    rsfd = rsOpen(RS_RBASE, RS_MBASE, RS_M2048, RS_WVECTOR, DQ_ON);
    dgfd = dgOpen(DG_BASE, DG_VECTOR, DQ_ON);
    
    /* Initialise Digimax board */
    dgInit(dgfd, DG_UNSGD);
    /* Demonstrate ROI transfers */
    rsIndm_512(rsfd, DQ_M512, RS_T50);delay(1);
    rsComplement(rsfd,RS_LS,RS_CMODE);delay(1);
    rsIndm_256(rsfd, DQ_M512, RS_T50);delay(3);
    rsIndm_128(rsfd, DQ_M512, RS_T50);delay(1);
    /* Demonstrate Graphics  */
    rsLineDraw(rsfd,RS_LS,100,300,328,300,0);
    rsLineDraw(rsfd,RS_LS,100,300,100,428,0);
    rsLineDraw(rsfd,RS_LS,100,428,328,428,0);
    rsLineDraw(rsfd,RS_LS,328,428,328,300,0);delay(1);
    rsCircle(rsfd,RS_LS,214,364,114,64,255);
    rsCircle(rsfd,RS_LS,214,364,64,64,128);
    rsCircle(rsfd,RS_LS,214,364,16,48,64);delay(3);
    /* Demonstrate other ROI features */
    rsIndm256(rsfd, DQ_M512, RS_T50);delay(2);
    rsIndm256_z1(rsfd, DQ_M512, RS_T50);delay(3);
    rsIndm256_z2(rsfd, DQ_M512, RS_T50);delay(3);
    rsIndm256_z3(rsfd, DQ_M512, RS_T50);delay(3);
    rsIndm256_z4(rsfd, DQ_M512, RS_T50);delay(3);
    rsIndm256(rsfd, DQ_M512, RS_T50);delay(2);
    /* Demonstrate digimax  */
    dgAnGain(dgfd,DG_M4DB);delay(1);
    dgAnGain(dgfd,DG_4DB);delay(1);
    dgAnGain(dgfd,DG_8DB);delay(1);
    dgAnGain(dgfd,DG_0DB);delay(1);
    dgAnOffs(dgfd,64);dgAnOffs(dgfd,96);dgAnOffs(dgfd,128);
    dgAnOffs(dgfd,150);dgAnOffs(dgfd,182);dgAnOffs(dgfd,200);
    dgAnOffs(dgfd,0);
    dgOBank(dgfd, DG_LUTA);delay(2);dgOBank(dgfd, DG_LUTD);delay(2);
    dgIBank(dgfd, DG_LUTB);delay(2);dgIBank(dgfd, DG_LUTF);delay(2);
    rsIndm_512(rsfd, DQ_M512, RS_T50);
    rsComplement(rsfd,RS_LS,RS_CMODE);
    rsIndm_256(rsfd, DQ_M512, RS_T50);


	
    if (rsClose(rsfd) == NOOK)
	printf("ROISTORE, problems with freeing the memory\n");
    if (dgClose(dgfd) == NOOK)
	printf("DIGIMAX, problems with freeing the memory\n");
}
