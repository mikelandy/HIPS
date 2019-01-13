/**************************************************************************
 *                                                                        * 
 * Function: fsMenu()			     		                        *
 *                                                                        *
 * Usage:	function called from menu_demo()                                *
 * Returns:  none							                        *
 * Defaults: none						                             *
 * Loads:                                                                 *
 * Modified:Tony 5-VIII-87                                                *
 *                                                                        *
 * Description:FRAME application note, put a ramp in FS0	              *
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

fsApp(fsfd, dgfd)
FS_DESC *fsfd;		/* FRAMESTORE device descriptor */
DG_DESC *dgfd;		/* DIGIMAX device descriptor */
{
    dgInit(dgfd, DG_UNSGD);	    /* initialize DG, ramp luts */
    fsInit(fsfd, FS_T50);		    /* initialize FS */
    fsFastClear(fsfd, FS_FS0, BLACK);  /* clear fs0 to 0 */
    fsHramp(fsfd, FS_FS0);	    /* put a ramp in FS0 */
    dgWaitFld(dgfd, TICKS);
    fsFastClear(fsfd, FS_FS0, BLACK);  /* clear fs0 to 0 */
    fsVramp(fsfd, FS_FS0);	    /* put a ramp in FS0 */
    dgWaitFld(dgfd, TICKS);
    fsFastClear(fsfd, FS_FS0, BLACK);  /* clear fs0 to 0 */
    fsInit(fsfd, FS_T50);		    /* reinitialize */
}
