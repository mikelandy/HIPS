/*******************************************************************
 * recursive, blob filling.
 *
 * barry shepherd (based on an algorithm by mike hughes)
 *
 *********************************************************************/

/* Scan through all of the pixels in a blob, either
 * deleting them or moving them to another image.
 * assume that the point (xs,ys) is contained within a blob
 * which is to have its grey values changed.
 * The blob is identified as all the connected non-zero
 * pixels which can be reached from (xs,ys).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "satts.h"
#define true	1
#define false	0

int area = 0,recur_border_copy();
void recur_delete_blob(),recur_move_blob(),recur_copy_blob(),recur_copy_blob2();
void recur_blobop(),inc_area();

void change_blob(sor,xd,yd,xs,ys,val)

register unsigned char *sor ;
int xd,yd,xs,ys,val;

{
	register int lx=xs,hx=xs+1,j;

	/* go left (changing) until hit boundary */
	while (pix(sor,lx,ys,xd) != 0) {
		pix(sor,lx--,ys,xd) = val ;
	}

	/* go right (changing) until hit boundary */
	while (pix(sor,hx,ys,xd) != 0) {
		pix(sor,hx++,ys,xd) = val ;
	}

	/* retrace the changed line looking above and below it */
	for (j = lx; j <=hx;j++) {
		if (pix(sor,j,ys+1,xd) != 0)
			change_blob(sor,xd,yd,j,ys+1,val);
		if (pix(sor,j,ys-1,xd) != 0)
			change_blob(sor,xd,yd,j,ys-1,val);
	}
}

int delete_blob(sor,xd,yd,xs,ys,debug)

register unsigned char *sor ;
int xd,yd,xs,ys,debug;

{
	area = 0;
	recur_delete_blob(sor,xd,yd,xs,ys,debug);
	return(area) ; 
}

void recur_delete_blob(sor,xd,yd,xs,ys,debug)

register unsigned char *sor ;
int xd,yd,xs,ys,debug;

{
	register int lx=xs,hx=xs+1,j ;
	int ysplus1,ysminus1 ;

	if ( (ys<0) || (ys>=yd) || (xs<0) || (xs>=xd) )
		return ;

	/* go left (changing) until hit boundary */
	while ( pix(sor,lx,ys,xd) && (lx>=0) ) {
		pix(sor,lx--,ys,xd) = 0 ;
		area++ ;
	}
	if (lx==-1) lx=0 ;

	/* go right (changing) until hit boundary */
	while (pix(sor,hx,ys,xd) && (hx < xd) ) {
		pix(sor,hx++,ys,xd) = 0 ;
		area++ ;
	}
	if (hx==xd) hx-- ;

	/* prevent testing pixels outside of the image */
	if ((ysplus1=ys+1) >= yd)
		ysplus1 = yd-1 ;
	if ((ysminus1=ys-1) < 0)
		ysminus1 = 0 ;

	/* retrace the changed line looking above and below it */
	for (j = lx; j <=hx;j++) {
		if (pix(sor,j,ysplus1,xd))
			recur_delete_blob(sor,xd,yd,j,ysplus1,debug);
		if (pix(sor,j,ysminus1,xd))
			recur_delete_blob(sor,xd,yd,j,ysminus1,debug);
	}
}

int move_blob(sor,dst,xd,yd,xs,ys,debug)

register unsigned char *sor,*dst ;
int xd,yd,xs,ys,debug;

{
	area = 0;
	recur_move_blob(sor,dst,xd,yd,xs,ys,debug);
	return(area) ; 
}

void recur_move_blob(sor,dst,xd,yd,xs,ys,debug)

register unsigned char *sor,*dst ;
int xd,yd,xs,ys,debug;

{
	register int lx=xs,hx=xs+1,j;
	unsigned char val ;

	if ( (ys<0) || (ys>=yd) || (xs<0) || (xs>=xd) )
		return ;

	/* go left (changing) until hit boundary */
	while ( (val = pix(sor,lx,ys,xd)) && (lx>=0) ) {
		pix(dst,lx,ys,xd) = val ;
		pix(sor,lx--,ys,xd) = 0 ;
		area++ ;
	}
	if (lx==-1) lx=0 ;


	/* go right (changing) until hit boundary */
	while ( (val = pix(sor,hx,ys,xd)) && (hx < xd) ) {
		pix(dst,hx,ys,xd) = val ;
		pix(sor,hx++,ys,xd) = 0 ;
		area++ ;
	}
	if (hx==xd) hx-- ;

	/* retrace the changed line looking above and below it */
	for (j = lx; j <=hx;j++) {
		if (pix(sor,j,ys+1,xd) != 0)
			recur_move_blob(sor,dst,xd,yd,j,ys+1,debug);
		if (pix(sor,j,ys-1,xd) != 0)
			recur_move_blob(sor,dst,xd,yd,j,ys-1,debug);
	}
}

/* copy_blob can now have a dst
 * picbuf a different size to the src picbuf
 * 6/6/88 for writing to fmem's etc. 
 * also dst can now be non-zero (7/6/88)
 */

int copy_blob_2fmem(sor,dst,sxd,syd,xs,ys,dxd,dyd,xoff,yoff,debug)

register unsigned char *sor,*dst ;
int sxd,syd,xs,ys,dxd,dyd,xoff,yoff,debug;

{
	unsigned char *flagpic,*getpicbuf() ;

	area = 0;	/* a global variable to save time*/

	/* NB in the old method it was assumed that the dst buf
	 * was zero'd before use and hence it could also act
	 * as a 'has-this-row-been-done' flag (ie hence the
	 * test of !pix(dst,x,y+/-1,xd) before recurring.
	 * If however the dst buf already contains an image
	 * then this cannot work, in this case much duplication of
	 * pixel copying must be done */

	/* if ( pix(dst,xoff+xs,yoff+ys,dxd) ) { fails when edge straddling*/
	if ( pix(dst,dxd/2,dyd/2,dxd) ) {	/* no good for binary dsts */
		/* a good sample-point ?? 
		 * this test may be a bit unreliable */
		/* flagpic = getpicbuf(sxd,syd) ; */
		flagpic = (unsigned char *)calloc(sxd*syd,sizeof(char)) ;
		recur_copy_blob2(sor,dst,sxd,syd,xs,ys,dxd,dyd,
					xoff,yoff,flagpic,debug);
		free(flagpic) ;
	}
	else {
		recur_copy_blob(sor,dst,sxd,syd,xs,ys,dxd,dyd,
					xoff,yoff,debug);
	}
	return(area) ; 
}

int copy_blob(sor,dst,xd,yd,xs,ys,debug)

register unsigned char *sor,*dst ;
int xd,yd,xs,ys,debug;

{
	unsigned char *flagpic,*getpicbuf() ;

	area = 0;	/* a global variable to save time*/

	if ( pix(dst,xs,ys,xd) ) {
		/* flagpic = getpicbuf(xd,yd) ; */
		flagpic = (unsigned char *)calloc(xd*yd,sizeof(char)) ;
		recur_copy_blob2(sor,dst,xd,yd,xs,ys,
					xd,yd,0,0,flagpic,debug);
		free(flagpic) ;
	}
	else
		recur_copy_blob(sor,dst,xd,yd,xs,ys,xd,yd,0,0,debug);

	return(area) ; 
}


/* NB assume dst is a zero'd picbuf (changed 7/6/88)
 * (sxd,syd) is src picsize and (dxd,dyd) is dst picsize
 * (xoff,yoff) is the offset to be added to all src (x,y)
 * pixel co-ords in order to refrence the dst picbuf
 */

void recur_copy_blob(sor,dst,sxd,syd,xs,ys,dxd,dyd,xoff,yoff,debug)

register unsigned char *sor,*dst ;
int sxd,syd,xs,ys,dxd,dyd,xoff,yoff,debug;

{
	register int lx,hx,j,xdst,ydst,indst;
	unsigned char val ;

	if ( (ys<0) || (ys>=syd) || (xs<0) || (xs>=sxd) )
		return ;
	lx=xs ;
	hx=xs+1 ;

	ydst = yoff + ys ;
	if ( ydst >= 0  &&  ydst < dyd ) indst = true ;
	else indst = false ;

	/* go left (changing) until hit boundary */
	while ( (val = pix(sor,lx,ys,sxd)) && (lx>=0) ) {
		xdst = xoff + (lx--) ;
		/* dont write outside of dst image */
		if (indst && (xdst >= 0) && (xdst < dxd)) {
			pix(dst,xdst,ydst,dxd) = val ;
			area++ ;
		}
	}
	if (lx==-1) lx=0 ;

	/* go right (changing) until hit boundary */
	while ( (val = pix(sor,hx,ys,sxd)) && (hx < sxd) ) {
		xdst = xoff + (hx++) ;
		/* dont write outside of dst image */
		if (indst && (xdst < dxd) && (xdst >= 0)) {
			pix(dst,xdst,ydst,dxd) = val ;
			area++ ;
		}
	}
	if (hx==sxd) hx-- ;

	/* retrace the changed line copying rows above
	 * and below it only if the row has not been done
	 * in the dst image */
	for (j = lx; j <=hx; j++) {
		if ( (pix(sor,j,ys+1,sxd)) &&
				!pix(dst,xoff+j,yoff+ys+1,dxd) )
			recur_copy_blob(sor,dst,sxd,syd,j,ys+1,
					dxd,dyd,xoff,yoff,debug);
		if ( (pix(sor,j,ys-1,sxd)) &&
				!pix(dst,xoff+j,yoff+ys-1,dxd) )
			recur_copy_blob(sor,dst,sxd,syd,j,ys-1,
					dxd,dyd,xoff,yoff,debug);
	}
}

void recur_copy_blob2(sor,dst,sxd,syd,xs,ys,dxd,dyd,xoff,yoff,flagpic,debug)

register unsigned char *sor,*dst,*flagpic;
int sxd,syd,xs,ys,dxd,dyd,xoff,yoff,debug;

{
	register int lx,hx,j,xdst,ydst,indst;
	unsigned char val ;

	if ( (ys<0) || (ys>=syd) || (xs<0) || (xs>=sxd) )
		return ;

	lx=xs ;
	hx=xs+1 ;

	ydst = yoff + ys ;

	if ( ydst >= 0  &&  ydst < dyd ) indst = true ;
	else indst = false ;

	/* go left (changing) until hit source image boundary */
	/* or fall out of the blob */
	while ( (val = pix(sor,lx,ys,sxd)) && (lx>=0) ) {
		pix(flagpic,lx,ys,sxd) = val ;
		xdst = xoff + (lx--) ;
		/* dont write outside of dst image */
		if (indst && (xdst >= 0) && (xdst < dxd)) {
			pix(dst,xdst,ydst,dxd) = val ;
			area++ ;
		}
	}
	if (lx==-1) lx=0 ;

	/* go right (changing) until hit boundary */
	/* or fall out of blob */
	while ( (val = pix(sor,hx,ys,sxd)) && (hx < sxd) ) {
		pix(flagpic,hx,ys,sxd) = val ;
		xdst = xoff + (hx++) ;
		/* dont write outside of dst image */
		if (indst && (xdst < dxd) && (xdst >= 0)) {
			pix(dst,xdst,ydst,dxd) = val ;
			area++ ;
		}
	}
	if (hx==sxd) hx-- ;

	/* retrace the changed line looking above and below it
	 * maintaining a separate flag-image */
	for (j = lx; j <=hx; j++) {
		if ( pix(sor,j,ys+1,sxd) && !pix(flagpic,j,ys+1,sxd) )
			recur_copy_blob2(sor,dst,sxd,syd,j,ys+1,
					dxd,dyd,xoff,yoff,
					flagpic,debug);
		if ( pix(sor,j,ys-1,sxd) && !pix(flagpic,j,ys-1,sxd) )
			recur_copy_blob2(sor,dst,sxd,syd,j,ys-1,
					dxd,dyd,xoff,yoff,
					flagpic,debug);
	}
}

int blobop(sor,xd,yd,xs,ys,blobpixfn,debug)

unsigned char *sor ;
void *blobpixfn() ;
int xd,yd,xs,ys,debug;

{
	unsigned char *dst ;

	/* check that there is a blob!! */
	if (!pix(sor,xs,ys,xd))
		return(false) ;
	dst = (unsigned char *)calloc(xd*yd,sizeof(char)) ;
	recur_blobop(sor,dst,xd,yd,xs,ys,blobpixfn) ;
	free(dst) ;
	return(true) ;
}

void recur_blobop(sor,dst,xd,yd,xs,ys,blobpixfn)

register unsigned char *sor,*dst ;
void *blobpixfn() ;
int xd,yd,xs,ys;

{
	register int lx=xs,hx=xs+1,j;

	if ( (ys<0) || (ys>=yd) || (xs<0) || (xs>=xd) )
		return ;

	/* go left (changing) until hit boundary */
	while ( pix(sor,lx,ys,xd) && (lx>=0) ) {
		(*blobpixfn)(lx,ys) ;
		/* label this pixel as being done */
		pix(dst,lx--,ys,xd) = 100 ;
	}
	if (lx==-1) lx=0 ;

	/* go right (changing) until hit boundary */
	while (pix(sor,hx,ys,xd) && (hx < xd) ) {
		(*blobpixfn)(hx,ys) ;
		/* label this pixel as being done */
		pix(dst,hx++,ys,xd) = 100 ;
	}
	if (hx==xd) hx-- ;

	/* retrace the changed line looking above and below it,
	 * only consider a pixel unseen if it is set in sor
	 * but not in dst.
	 */
	for (j = lx; j <=hx;j++) {
		if ( (pix(sor,j,ys+1,xd)) && !pix(dst,j,ys+1,xd) )
			recur_blobop(sor,dst,xd,yd,j,ys+1,blobpixfn);
		if ( (pix(sor,j,ys-1,xd)) && !pix(dst,j,ys-1,xd) )
			recur_blobop(sor,dst,xd,yd,j,ys-1,blobpixfn);
	}
}

/*********************************************************/
/* one particular application of blobop */

void inc_area()
{
	area++ ;
}

int blob_area(pic,xd,yd,bx,by,debug)

unsigned char *pic;
int xd,yd,bx,by,debug;

{
	area = 0 ;
	blobop(pic,xd,yd,bx,by,inc_area,debug) ;
	return(area) ;
}

/****************************************************************/
/* copy from one grey frame to another all pixles lying inside
   the connected boundary in border */

int border_copy(sor,dst,border,xd,yd,xs,ys,debug)

register unsigned char *sor,*dst,*border;
int xd,yd,xs,ys,debug;

{
	area = 0;	/* a global variable to save time*/

	recur_border_copy(sor,dst,border,xd,yd,xs,ys,xd,yd,0,0,debug,0,false);

	return(area) ; 
}

int border_fill(dst,border,xd,yd,xs,ys,debug,val)

register unsigned char *dst,*border;
int xd,yd,xs,ys,debug,val;

{
	area = 0;	/* a global variable to save time*/

	recur_border_copy((unsigned char *)NULL,dst,border,xd,yd,xs,ys,xd,yd,0,0,debug,val,false);
	return(area) ; 
}

int connected_border_fill(dst,border,xd,yd,xs,ys,debug,val)

register unsigned char *dst,*border;
int xd,yd,xs,ys,debug,val;

{
	area = 0;	/* a global variable to save time*/

	if (recur_border_copy((unsigned char *)NULL,dst,border,xd,yd,xs,ys,xd,yd,0,0,debug,val,true) == -1)
		return(-1) ; 
	return(area) ; 
}

int recur_border_copy(sor,dst,border,sxd,syd,xs,ys,dxd,dyd,xoff,yoff,debug,val,noedges)

register unsigned char *sor,*dst,*border ;
int sxd,syd,xs,ys,dxd,dyd,xoff,yoff,debug,val,noedges;

{
	register int lx,hx,j,xdst,ydst,indst;

	if ( (ys<0) || (ys>=syd) || (xs<0) || (xs>=sxd) ) {
		if (noedges)
			return -1 ;
		else
			return 0 ;
	}
	lx=xs ;
	hx=xs+1 ;

	ydst = yoff + ys ;
	if ( ydst >= 0  &&  ydst < dyd ) indst = true ;
	else indst = false ;

	/* go left (changing) until hit boundary */
	while ( (!pix(border,lx,ys,sxd)) && (lx>=0) ) {
		xdst = xoff + lx ;
		/* dont write outside of dst image */
		if (indst && (xdst >= 0) && (xdst < dxd)) {
			if (sor == (unsigned char *)NULL)
				pix(dst,xdst,ydst,dxd) = val ;
			else
				pix(dst,xdst,ydst,dxd) = pix(sor,lx,ys,sxd) ;
			area++ ;
		}
		lx-- ;
	}
	if (lx==-1) {
		if (noedges)
			return -1 ;
		else
			lx=0 ;
	}
	/* go right (changing) until hit boundary */
	while ( (!pix(border,hx,ys,sxd)) && (hx < sxd) ) {
		xdst = xoff + hx ;
		/* dont write outside of dst image */
		if (indst && (xdst < dxd) && (xdst >= 0)) {
			if (sor == (unsigned char *)NULL)
				pix(dst,xdst,ydst,dxd) = val ;
			else
				pix(dst,xdst,ydst,dxd) = pix(sor,hx,ys,sxd) ;
			area++ ;
		}
		hx++ ;
	}
	if (hx==sxd) { 
		if (noedges)
			return -1 ;
		else
			hx-- ;
	}
	/* retrace the changed line copying rows above
	 * and below it only if the row has not been done
	 * in the dst image */
	lx++ ; hx-- ;	/* avoid spilling over border */
	for (j = lx; j <= hx; j++) {
		if ( (!pix(border,j,ys+1,sxd)) && !pix(dst,xoff+j,yoff+ys+1,dxd) )
			if (recur_border_copy(sor,dst,border,sxd,syd,j,ys+1,
					dxd,dyd,xoff,yoff,debug,val,noedges) == -1)
					return -1;
		if ( (!pix(border,j,ys-1,sxd)) && !pix(dst,xoff+j,yoff+ys-1,dxd) )
			if (recur_border_copy(sor,dst,border,sxd,syd,j,ys-1,
					dxd,dyd,xoff,yoff,debug,val,noedges) == -1)
					return -1;
	}
	return 0;	/* MSL 12/28/18 - this is a guess */
}

