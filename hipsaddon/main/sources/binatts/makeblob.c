#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "satts.h"

struct blobstruct *make_blob(picp,xd,yd,bloblist)

unsigned char *picp ;
int xd,yd;
struct	blobstruct **bloblist ;

{
	struct blobstruct *blob;
	void*calloc() ;

	blob = ( struct blobstruct * )calloc( 1,sizeof( struct blobstruct ));

	/* keep track of where the pic data can be found */
	blob->pic = picp;	/* this pic can contain lots of blobs*/		
	blob->bo_pic = NULL;	/* this pic contains only this blob */
	blob->holepic = NULL;	/* will hold holes if needed */
	blob->workpic = NULL;	/* work pic if needed */
	blob->xd = xd ;
	blob->yd = yd ;
	blob->xc = NOT_DONE;
	blob->yc = NOT_DONE ;

	blob->holecnt = NOT_DONE;
	blob->area = NOT_DONE;
	blob->rawarea = NOT_DONE;
	blob->majdir = NOT_DONE ;
	blob->majvar = blob->minvar = NOT_DONE;
	blob->maxrad = blob->minrad = blob->shrwid = NOT_DONE ;
	blob->pa = blob->avdist = blob->avper = NOT_DONE ;
	blob->maxper = blob->minper = NOT_DONE;
	blob->avrad = blob->bparea = NOT_DONE;
	blob->palen = blob->pawid = NOT_DONE;
	blob->maxlen = blob->perplen = NOT_DONE;
	blob->symaxlen = blob->syminlen = NOT_DONE ;
	blob->next = *bloblist ;
	*bloblist = blob ;
	blob->holelist = NULL ;
	return(blob);
}

void free_blobs(bloblist)
struct blobstruct **bloblist ;
{
	struct blobstruct *bp,*nextbp ;

	bp = *bloblist ;
	while (bp != (struct blobstruct *)NULL) {
		if (bp->bo_pic != NULL)
			free(bp->bo_pic);
		if (bp->holepic != NULL)
			free(bp->holepic);
		if (bp->workpic != NULL)
			free(bp->workpic);
		/* holes can have holes which have holes.. etc .. */
		if (bp->holelist)
			free_blobs(&bp->holelist);

		nextbp = bp->next ;
		free(bp);
		bp = nextbp;
	}
	*bloblist = NULL ;
}
