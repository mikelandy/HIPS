/*************************************************************************
*                              rsIn512.c
**************************************************************************
* ARGUMENTS:                                                              *
*       RS_DESC *rsfd;  pointer to device descriptor                      *
*       int res;        resolution mode, a 384(-7) or 512(-10) roistore   *
*       int hzmode;     Field display rate, 50 or 60 fields per second    *
*                                                                         *
* RETURNS:                                                                *
*      none                                                               *
*                                                                         *
* DESCRIPTION:                                                            *
*      To initialize ROISTORE , these values assume that			    *
*      a P3 master providing rs-170 or CCIR timing is available on P3.    *
*      Grab a full image P10 -> MS  P9 -> LS                              *
*      Display  a full frozen  frame LS -> P7    			              *
*                                                                         *
***************************************************************************
*
* Revision 1.2  14/03/88  tony
* March 1988 Release 3.0
**************************************************************************
*     			 Copyright (c) 1987                                  *
*     		        Captain Chaos							   *
**************************************************************************/

#include <rsHead.h>

rsIn512(rsfd, res, hzmode)
RS_DESC *rsfd;	/* Pointer to Roistore to device descriptor structure */
int res;        /* Resolution mode, a 384(-7) or 512(-10) roistore    */
int hzmode;     /* Field display rate, 50 or 60 fields per second     */
{

/* Zero out all registers first */
    rsRegSet(rsfd);
    
/* Set up standard parameters of ROI transfer */
/* Standard write parameters  */
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
    rsMemMd(rsfd, RS_BYTE);		/* Set Memory to word mode         */
/* Selecting a timing bus   */
    rsWRoiTS(rsfd, DQ_ENABLE);	/* Slave Write Roi to P3 timing    */
    rsWRoiBus(rsfd, 0);			/* Select Bus 0 on P4, anyways     */
/* Standard read parameters  */
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
    rsMemMd(rsfd, RS_BYTE);		/* Set Memory to byte mode         */
/* Selecting a timing bus and master   */
    rsRMasMd(rsfd, RS_SLAVE);		/* Read Roi is set to be a Master  */
    rsRRoiTS(rsfd, DQ_ENABLE);	/* Slave Read  Roi to P3 timing    */
    rsRRoiBus(rsfd, 0);			/* Select Bus 0 on P4, anyways     */
/* Set up misc controls. */
    rsP56OD(rsfd, DQ_ON);		/* Disable data out the P56 port   */
    rsP56OZ(rsfd, DQ_OFF);		/* Disable P56 zoomed data output  */
    rsP56OB(rsfd, DQ_OFF);		/* Disable P56 blanking            */
    rsP56IDMd(rsfd, RS_NORML);	/* Disable P56 output interleve    */
    rsIIMode(rsfd, RS_NORML);		/* Disable Input Interleve         */

/* Set up a ROI transfer from the  digimax
 * 512x512 (just a bit bigger) frame from the P10 port. P10 -> LS
 */
/* Timing a write ROI sequence  Vlen =(frame req. / 2) for interlace mode  */
    rsWHST(rsfd, 0x5f);			/* Set Write Roi Horizontal Start   */
    rsWHET(rsfd, 0x244);			/* Set Write Roi Horizontal End   */
    rsWVST(rsfd, 0xa);			/* Set Write Roi Vertical Start  */
    rsWVET(rsfd, 0x118);			/* Set Write Roi Vertical End   */
/* Control parameters for memory Hlen 1024 Finc 512 for interlace mode  */
    rsWPix0(rsfd, 0x0);			/* Set Write Roi Pixel 0 Address  */
    rsWHLen(rsfd, 0x400);		/* Set Write Roi Horizontal Length */
    rsWFldInc(rsfd,0x200);		/* Set Write Roi Field Increment   */

/* Set up a ROI transfer from the LS to digimax,  a
 * 512x512 (just a bit bigger) frame from the P7 port. LS -> P7
 */
/* Timing a ROI sequence   */
    rsRHST(rsfd, 0x58);			/* Set Read  Roi Horizontal Start 88 */
    rsRHET(rsfd, 0x258);			/* Set Read  Roi Horizontal End 600  */
    rsRVST(rsfd, 0x14);			/* Set Read  Roi Vertical Start 20   */
    rsRVET(rsfd, 0x122);			/* Set Read  Roi Vertical End    261  */
/* Control parameters for memory Hlen 1024 Finc 512 for interlace mode  */
    rsRPix0(rsfd, 0x0);			/* Set Read  Roi Pixel 0 Address   */
    rsRHLen(rsfd, 0x400);		/* Set Read  Roi Horizontal Length */
    rsRFldInc(rsfd,0x200);		/* Set Read  Roi Field Increment   */
/* Set up misc controls. */
    rsP7InpMd(rsfd, DQ_PRIMARY);	/* Set P7  to request Primary      */
    rsP10InpMd(rsfd, DQ_PRIMARY);	/* Set P10 to request Primary      */
/* Set up mux transfer control, direction and connections  */
    rsLatchMd(rsfd, RS_SINGL);	/* Single Buffer, Instantaneous    */
    rsP7Dir(rsfd, DQ_OUT);		/* Turn on P7 output port         */
    rsMSofP7(rsfd, RS_BLNK | RS_ACTV, RS_MLBMS);    /* Connect P7 -> LS */
    rsLSofP7(rsfd, RS_BLNK | RS_ACTV, RS_MLBLS);    /* Connect P7 -> LS */
    rsMSofLS(rsfd, RS_BLNK | RS_ACTV, RS_P10MS);  /* Connect LS -> P10  */
    rsLSofLS(rsfd, RS_BLNK | RS_ACTV, RS_P10LS);  /* Connect LS -> P10  */
    rsMXfer(rsfd);		/* Update all mux connections	    */
/* Kludge for POLLED MODE   */
    rsClWIntr(rsfd);			/* Clear the write interupt bit */
    rsClRIntr(rsfd);			/* Clear the read interupt bit */
/* Starting the ROI transfer  */
    rsWFire(rsfd);				/* Fire the write interupt	    */
    rsRFire(rsfd);				/* Fire the read interupt	    */
}


