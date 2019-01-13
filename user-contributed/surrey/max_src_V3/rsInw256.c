/*************************************************************************
*                              rsIn256.c
**************************************************************************
* ARGUMENTS:                                                              *
*       RS_DESC *rsfd;  pointer to device descriptor                      *
*                                                                         *
* RETURNS:                                                                *
*      none                                                               *
*                                                                         *
* DESCRIPTION:  POLLED MODE ONLY                                          *
*      To initialize ROISTORE , these values assume that			    *
*      a P3 master providing rs-170 or CCIR timing is available on P3.    *
*      acquire image P10 -> MS  P9 -> LS                                  *
*      ROI transfer from MS to LS of 256x256 thro' MUX                    *
*      Display the window within a 512x512 frame LS -> P7                 *
***************************************************************************
*
* Revision 1.2  14/03/88  tony
* March 1988 Release 3.0
**************************************************************************
*     			 Copyright (c) 1987                                  *
*     		        Captain Choas							   *
**************************************************************************/

#include <rsHead.h>

rsIn256(rsfd)
RS_DESC *rsfd;	/* Pointer to Roistore to device descriptor structure */
{

/* Zero out all registers first */
    rsRegSet(rsfd);
    
/* Set up Write Roi to aquire an image from the P9 and P10 input
 * ports. P9 is connected to the LS and P10 to MS of the frame
 * memory.  The Write roi is locked to P3 timing.  Image is input
 * in Interlace, Start on Even, and input a pair of fields.  The
 * field increment is set to 0x200.  The horizontal modulus is
 * set to 0x400.  The Pixel 0 address to set to 0x0000. When transfer
 * is complete stop
 */
	printf("ROISTORE, set up write into LS & MS\n");

/* Timing a ROI sequence   */
    rsWHST(rsfd, 0x6F);			/* Set Write Roi Horizontal Start  */
    rsWHET(rsfd, 0x7);			/* Set Write Roi Horizontal End    */
    rsWVST(rsfd, 0x14);			/* Set Write Roi Vertical Start    */
    rsWVET(rsfd, 0x105);			/* Set Write Roi Vertical End      */
/* Control parameters for memory   */
    rsWPix0(rsfd, 0x0);			/* Set Write Roi Pixel 0 Address   */
    rsWHLen(rsfd, 0x400);		/* Set Write Roi Horizontal Length */
    rsWFldInc(rsfd, 0x200);		/* Set Write Roi Field Increment   */
/* Control parameters for interleaving   */
    rsWHZoom(rsfd,RS_HZMX1);		/* Set Write Horiz. Zoom to unity  */
    rsWVZoom(rsfd,RS_VZMX1); 	 	/* Set Write Vert. Zoom to unity   */
/* Framing a ROI sequence   */
    rsWIntlMd(rsfd, RS_INTLACE);	/* Set Write Roi input interlace   */
    rsWXmitFld(rsfd,RS_PAIR);		/* Set Write Roi input Pair fields */
    rsWStField(rsfd, RS_EVEN);	/* Set Write Roi Start on Even     */
/* Enabling a ROI sequence   */
    rsWSVMd(rsfd, DQ_ENABLE);		/* Select Vertical, continuous     */
    rsWHSAE(rsfd, DQ_DISABLE);	/* Set Write Hor. Continuous count */
    rsWVSAE(rsfd, DQ_ENABLE);		/* Select vertical stop at end     */
/* Setting memory mode   */
    rsVWPMd(rsfd, 0x0);			/* Enable video writes, acquire    */
    rsMemMd(rsfd, RS_WORD);		/* Set Memory to word mode         */
/* Selecting a timing bus   */
    rsWRoiTS(rsfd, DQ_ENABLE);	/* Slave Write Roi to P3 timing    */
    rsWRoiBus(rsfd, 0);			/* Select Bus 0 on P4, anyways     */
/* Selecting I/O MUX   */
    rsP9InpMd(rsfd, DQ_PRIMARY);	/* Set P9  to request Primary      */
    rsP10InpMd(rsfd, DQ_PRIMARY);	/* Set P10 to request Primary      */
    rsMSofLS(rsfd, RS_BLNK | RS_ACTV, RS_P9MS);  /* Connect LS -> P9  */
    rsLSofLS(rsfd, RS_BLNK | RS_ACTV, RS_P9LS);  /* Connect LS -> P9  */
    rsMSofMS(rsfd, RS_BLNK | RS_ACTV, RS_P10MS); /* Connect MS -> P10 */
    rsLSofMS(rsfd, RS_BLNK | RS_ACTV, RS_P10LS); /* Connect MS -> P10 */
    rsMXfer(rsfd);				/* Update all mux connections	    */
/* Kludge for POLLED MODE   */
    rsClWIntr(rsfd);			/* Clear the write interupt bit */
/* Starting the ROI transfer   */
    rsWFire(rsfd);				/* Fire the write interupt	    */
/* Wait for transfer to complete   */
    /*rsWWait(rsfd, 1);			 
    rsWWNBusy(rsfd);			 Wait until write ROI is not active */

	printf("ROISTORE, set up write OK\n");
/* Set up new ROI transfer to send a non-interlaced 256x256, 
 * window from MS to LS thro' MUX.
 */
	printf("ROISTORE, set up read & write from LS into MS\n");

/* Changing the ROI read parameters   */
    rsRVSyncMd(rsfd, DQ_ON);		/* Set read inhibit Vsync pulse */
/* Timing a ROI sequence   */
    rsRHST(rsfd, 0x1a);			/* Set Read  Roi Horizontal Start  */
    rsRHET(rsfd, 0x11a);			/* Set Read  Roi Horizontal End    */
    rsRVST(rsfd, 0x0);			/* Set Read  Roi Vertical Start    */
    rsRVET(rsfd, 0x100);			/* Set Read  Roi Vertical End      */
/* Control parameters for memory   */
    rsRPix0(rsfd, 0x10080);		/* Set Read  Roi Pixel 0 Address   */
    rsRFldInc(rsfd, 0x0);		/* Set Read  Roi Field Increment   */
    rsRHLen(rsfd, 0x200);		/* Set Read  Roi Horizontal Length */
/* Control parameters for interleaving   */
    rsRHZoom(rsfd,RS_HZMX1);  	/* Set Read  Horiz. Zoom to unity  */
    rsRVZoom(rsfd,RS_VZMX1);		/* Set Read  Vert. Zoom to unity   */
/* Framing a ROI sequence   */
    rsRIntlMd(rsfd, RS_CONT);		/* Set Read Roi output continuous  */
    rsRXmitFld(rsfd, RS_SINGL);   /* Set Read Roi output Single field */
    rsRStField(rsfd, RS_EVEN);	/* Set Read  Roi Start on Even     */
/* Enabling a ROI sequence   */
    rsRSVMd(rsfd, DQ_ENABLE);		/* Select Vertical, continuous     */
    rsRHSAE(rsfd, DQ_DISABLE);	/* Set Read  Hor. Continuous count */
    rsRVSAE(rsfd, DQ_ENABLE);		/* Select vertical stop at end     */
/* Selecting a timing master and timing bus   */
    rsRMasMd(rsfd, RS_SLAVE);		/* Read Roi is set to be a Master  */
    rsRRoiTS(rsfd, DQ_ENABLE);	/* Slave Read  Roi to P3 timing    */
    rsRRoiBus(rsfd, 0);			/* Select Bus 0 on P4, anyways     */
/* Change of ROI read parameters complete  */
    rsRVSyncMd(rsfd, DQ_OFF);		/* Set read Vsync pulses on*/
	printf("ROISTORE, set up read OK\n");
    
/* Changing the ROI write parameters   */
    rsWVSyncMd(rsfd, DQ_ON);		/* Set write Vsync pulses off*/
/* Timing a ROI sequence   */
    rsWHST(rsfd, 0x35);			/* Set Write Roi Horizontal Start  */
    rsWHET(rsfd, 0x135);			/* Set Write Roi Horizontal End    */
    rsWVST(rsfd, 0x0);			/* Set Write Roi Vertical Start    */
    rsWVET(rsfd, 0x100);			/* Set Write Roi Vertical End      */
/* Control parameters for memory   */
    rsWPix0(rsfd, 0x10080);		/* Set Write Roi Pixel 0 Address   */
    rsWFldInc(rsfd, 0x0);		/* Set Write Roi Field Increment   */
    rsWHLen(rsfd, 0x200);		/* Set Write Roi Horizontal Length */
/* Control parameters for interleaving   */
    rsWHZoom(rsfd,RS_HZMX1);  	/* Set Write Horiz. Zoom to unity  */
    rsWVZoom(rsfd,RS_VZMX1); 		/* Set Write Vert. Zoom to unity   */
/* Framing a ROI sequence   */
    rsWIntlMd(rsfd, RS_CONT); 	/* Set Write Roi input interlace   */
    rsWXmitFld(rsfd,RS_SINGL);	/* Set Write Roi input Pair fields */
    rsWStField(rsfd, RS_EVEN);	/* Set Write Roi Start on Even     */
/* Enabling a ROI sequence   */
    rsWSVMd(rsfd, DQ_ENABLE);		/* Select Vertical, continuous     */
    rsWVSAE(rsfd, DQ_ENABLE);		/* Select vertical stop at end     */
    rsWHSAE(rsfd, DQ_DISABLE);	/* Set Write Hor. Continuous count */
/* Selecting memory mode   */
    rsVWPMd(rsfd, 0xc);			/* Enable video write protect      */
    rsMemMd(rsfd, RS_WORD);		/* Set Memory to word mode         */
    rsLatchMd(rsfd, RS_SINGL);	/* Set Latch to active video       */	
/* Selecting a timing bus   */
    rsWRoiTS(rsfd, DQ_ENABLE);	/* Slave Write Roi to P3 timing    */
    rsWRoiBus(rsfd, 0);			/* Select Bus 0 on P4, anyways     */
/* Change of ROI write parameters complete  */
    rsWVSyncMd(rsfd, DQ_OFF);		/* Set write Vsync pulses on*/

/* Selecting I/O MUX   */
    rsP56OD(rsfd, DQ_ON);		/* Disable data out the P56 port   */
    rsP56OZ(rsfd, DQ_OFF);		/* Disable P56 zoomed data output  */
    rsP56OB(rsfd, DQ_OFF);		/* Disable P56 blanking            */
    rsP56IDMd(rsfd, RS_NORML);	/* Disable P56 output interleve    */
    rsIIMode(rsfd, RS_NORML);		/* Disable Input Interleve         */
    rsMSofLS(rsfd, RS_BLNK | RS_ACTV, RS_MMBMS);    /*Connect LS -> MS*/
    rsLSofLS(rsfd, RS_BLNK | RS_ACTV, RS_MMBLS);    /*Connect LS -> MS*/
    rsMXfer(rsfd);				/* Update all mux connections	    */
	printf("ROISTORE, set up write OK\n");
/* Kludge for POLLED MODE   */
    rsClRIntr(rsfd);			/* Clear the read interupt bit */
    rsClWIntr(rsfd);			/* Clear the write interupt bit */
/* Starting the ROI transfer   */
    rsRFire(rsfd);				/* Fire the write interupt	    */
    rsWFire(rsfd);				/* Fire the read interupt	    */
	printf("ROISTORE, set up FIRE OK\n");
/* Wait for ROI transfer to finish   */
    /*rsWWait(rsfd, 1);			  
    rsWWNBusy(rsfd);			 Wait until write ROI transfer not busy */

/* Set up a ROI transfer from the LS to digimax, a 256x256 in a
 * 512x512 frame from the P7 port. MS -> P8
 */
	printf("ROISTORE, set  FIRE  OK\n");
/* Change of ROI read parameters  */
    rsRVSyncMd(rsfd, DQ_ON);		/* Set write Vsync pulses off*/
/* Timing a ROI sequence   */
    rsRHST(rsfd, 0x58);			/* Set Read  Roi Horizontal Start  */
    rsRHET(rsfd, 0x258);			/* Set Read  Roi Horizontal End    */
    rsRVST(rsfd, 0x14);			/* Set Read  Roi Vertical Start    */
    rsRVET(rsfd, 0x105);			/* Set Read  Roi Vertical End      */
/* Control parameters for memory   */
    rsRPix0(rsfd, 0x0);			/* Set Read  Roi Pixel 0 Address   */
    rsRHLen(rsfd, 0x400);		/* Set Read  Roi Horizontal Length */
    rsRFldInc(rsfd, 0x200);		/* Set Read  Roi Field Increment   */
/* Control parameters for interleaving   */
    rsRHZoom(rsfd,RS_HZMX1);  	/* Set Read  Horiz. Zoom to unity  */
    rsRVZoom(rsfd,RS_VZMX1); 		/* Set Read  Vert. Zoom to unity   */
/* Framing a ROI sequence   */
    rsRIntlMd(rsfd, RS_INTLACE); 	/* Set Read Roi output interlace   */
    rsRXmitFld(rsfd, RS_PAIR); 	/* Set Read Roi output Pair fields */
    rsRStField(rsfd, RS_EVEN);	/* Set Read  Roi Start on Even     */
/* Enabling a ROI sequence   */
    rsRSVMd(rsfd, RS_CONT);		/* Select Vertical, continuous     */
    rsRHSAE(rsfd, DQ_DISABLE); 	/* Set Read  Hor. Continuous count */
    rsRVSAE(rsfd, DQ_ENABLE);		/* Select vertical stop at end     */
/* Selecting memory mode   */
    rsMemMd(rsfd, RS_WORD);		/* Set Memory to word mode         */
/* Selecting a timing bus and master   */
    rsRMasMd(rsfd, RS_SLAVE);		/* Read Roi is set to be a Master  */
    rsRRoiTS(rsfd, DQ_ENABLE);	/* Slave Read  Roi to P3 timing    */
    rsRRoiBus(rsfd, 0);			/* Select Bus 0 on P4, anyways     */
/* Change of ROI read parameters complete  */
    rsRVSyncMd(rsfd, DQ_OFF);		/* Set write Vsync pulses on*/
	printf("ROISTORE, set read up from LS to DIGI  OK\n");

/* Set up misc controls. */
    rsP56OD(rsfd, DQ_ON);		/* Disable data out the P56 port   */
    rsP56OZ(rsfd, DQ_OFF);		/* Disable P56 zoomed data output  */
    rsP56OB(rsfd, DQ_OFF);		/* Disable P56 blanking            */
    rsP56IDMd(rsfd, RS_NORML);	/* Disable P56 output interleve    */
    rsIIMode(rsfd, RS_NORML);		/* Disable Input Interleve         */
    rsP7InpMd(rsfd, DQ_PRIMARY);	/* Set P7  to request Primary      */
    rsP8InpMd(rsfd, DQ_PRIMARY);	/* Set P8  to request Primary      */
	printf("ROISTORE, set up from misc controls OK\n");
        
/* Set up mux transfer control, direction and connections  */
    rsLatchMd(rsfd, RS_SINGL);	/* Single Buffer, Instantaneous    */
    rsP7Dir(rsfd, DQ_OUT);		/* Turn on P7 output port         */
    rsP8Dir(rsfd, DQ_OUT);		/* Turn on  P8 output port         */
    rsMSofP7(rsfd, RS_BLNK | RS_ACTV, RS_MLBMS);    /* Connect P7 -> LS */
    rsLSofP7(rsfd, RS_BLNK | RS_ACTV, RS_MLBLS);    /* Connect P7 -> LS */
    rsMSofP8(rsfd, RS_BLNK | RS_ACTV, RS_MMBMS);    /* Connect P8 -> MS */
    rsLSofP8(rsfd, RS_BLNK | RS_ACTV, RS_MMBLS);    /* Connect P8 -> MS */
    rsMXfer(rsfd);		/* Update all mux connections	    */
	printf("ROISTORE, set up from MUX controls OK\n");
/* Kludge for POLLED MODE   */
    rsClRIntr(rsfd);			/* Clear the read interupt bit */
/* Starting the ROI transfer  */
    rsRFire(rsfd);
	printf("ROISTORE, set read Fire OK\n");
}

rsClRIntr(rsfd)
RS_DESC *rsfd;
{
	rxsClear(rsfd, rsInt0, RS_IRINT);
}
rsClWIntr(rsfd)
RS_DESC *rsfd;
{
	rxsClear(rsfd, rsInt0, RS_IWINT);
}
