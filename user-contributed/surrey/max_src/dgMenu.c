/**************************************************************************
 *                                                                        * 
 * Function: dgMenu()			     		                        *
 *                                                                        *
 * Usage:	function called from menu_demo()                                *
 * Returns:  none							                        *
 * Defaults: none						                             *
 * Loads:                                                                 *
 * Modified:Tony 5-VIII-87                                                *
 *                                                                        *
 * Description:DIGI application note, display           
	              *
 *             FRAMESTORE init sets up FS0 as the MUX output (P7)         *
 *             DIGIMAX init sets up P7 as D/A source			         *
 *             Set up FS P7 - DG P7 and DG P10 - FS P10 as A/D            *
 **************************************************************************
 *                    Copyright (c) 1987                                  *
 *                    Captain Chaos                                       *
 **************************************************************************
 */

#include <fsHead.h>
#include <dgHead.h>
#include <menudefs.h>

dgApp(fsfd, dgfd)
FS_DESC *fsfd;		/* FRAMESTORE device descriptor */
DG_DESC *dgfd;		/* DIGIMAX device descriptor */
{
    int retcode;
    dgInit(dgfd, DG_UNSGD);	    /* initialize DG, ramp luts */
    fsInit(fsfd, FS_T50);		    /* initialize FS */
    retcode = dgSelDtoASrc(dgfd, DG_INTDISP); /* no matter the connection,
			       we should see the internal A/D out of P7 */
    dgWaitFld(dgfd, TICKS);
    dgInit(dgfd, DG_UNSGD);	    /* reinitialize */
    if (retcode != OK)
	printf("problem in initializing \n");
}
