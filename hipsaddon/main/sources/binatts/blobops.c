/****************************************************************************** 
 * ROUTINES TAKING BLOBS AS ARGUMENTS
 * At present blobs only exist for the current-window so we can assume
 * that all drawing is to be done into the current window
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "satts.h"

#define true 1
#define false 0

int blobcentroid(),blobpraxis();
void move_holes(),free_blobs();

void blob_centroid(bp)  
struct	blobstruct *bp ;
{
	if (bp->xc == NOT_DONE) {	/* MSL 12/28/18 not sure if 
					isolate_blob should be run or not,
					it was oddly commented out */
		/*isolate_blob(bp);	 in getattval.c */
		blobcentroid(bp->pic,bp->xd,bp->yd,bp->xst,bp->yst,
							&bp->xc,&bp->yc,false);
	}
}

void blob_praxis(bp)  
struct blobstruct *bp ;
{
	if (bp->majdir == NOT_DONE) {
		blob_centroid(bp); 
		blobpraxis(bp->pic,bp->xd,bp->yd,bp->xst,bp->yst,
				&bp->xc,&bp->yc,&bp->majdir,
				&bp->majvar,&bp->minvar,false) ;
	}
}

/* copy all holes found in a blob to a separate picbuf for later processing*/
void move_blobholes(bp)
struct blobstruct *bp ;
{
	unsigned char *getpicbuf() ;

	/* if bp->holepic exists then to holes have been copied already!! */
	if (!bp->holepic) {
		/* bp->holepic = getpicbuf(bp->xd,bp->yd) ; */
		bp->holepic = (unsigned char *)calloc(bp->xd*bp->yd,sizeof(char)) ;
		move_holes(bp->pic,bp->holepic,
					bp->xd,bp->yd,bp->xst,bp->yst);
		if (bp->holelist) {
			free_blobs(&bp->holelist) ;
			/* numblobs = 0 ; I don't think this is needed */
		}
	}
}

