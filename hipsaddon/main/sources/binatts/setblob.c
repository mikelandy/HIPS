/* Set the pixels contained in a specified blob. 
 * ----------------------------------------------
 * The blob is specified by the co-ords of any point inside of it.
 * The source frame can contain any number of blobs.
 * The destination frame will contain only the specified blob
 * and the pixels in the blob will be set to the value specified
 * as an argument.
 *  This function has two parts:
 *	Firstly, create a temporary pic containing only the border
 *	of the specified blob.
 *	Secondly, scan the pic-frame from left to right (ONLY for
 *	those rows which are contained in the blob) and set the blob
 *	pixels using the border frame as an inside/outside blob flag.
 *
 * delete_blob: This sets the dest frame = source frame and
 * 		labels all pixels in the blob as 0.
 * copy_blob:	This is set_blob but with the blob pixels set to 200.
 *
 * B.A.Shepherd, 1985.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define pix(a,x,y,xdim)	(*((unsigned char*) (a + (x+(y)*(xdim)))))
#define true	1
#define false	0

#define MOVEBLOB	0
#define MOVEHOLES	1
#define COPYBLOB	2
#define COPYGREYBLOB	3
#define COPYGREYENCLOSURE	4
#define DELETEBLOB	5
#define TEMPLATE	6
#define BORDER		7

#define OPN(opn,sor,dest,gsor,x,y,xdim) { switch (opn) { \
			case COPYBLOB: \
				pix(dest,x,y,xdim) = pix(sor,x,y,xdim) ; \
				break ; \
			case MOVEBLOB: \
				pix(dest,x,y,xdim) = pix(sor,x,y,xdim) ; \
				pix(sor,x,y,xdim) = 0; \
				break ; \
			case COPYGREYBLOB: \
			case COPYGREYENCLOSURE: \
				pix(dest,x,y,xdim) = pix(gsor,x,y,xdim) ; \
				break ; \
		} \
		area++ ; }
		/* if (debug)
			putpixel(DispFS,x,y,10); */

int set_blob(),innerborder();
void backtrace();

#ifdef USEOLDMETHODS

int old_blob_area(pic,xdim,ydim,bx,by,debug)
unsigned char *pic ;
{
	return( set_blob(pic,pic,0,xdim,ydim,bx,by,200,COPYBLOB,debug) );
}

/* superceded by changeblob.c */
int old_delete_blob(pic,xdim,ydim,bx,by,debug)
unsigned char *pic ;
{
	return( set_blob(pic,pic,0,xdim,ydim,bx,by,0,DELETEBLOB,debug) );
}

/* superceded by changeblob.c */
int old_copy_blob(sor,dest,xdim,ydim,bx,by,debug)
unsigned char *sor,*dest ;
{	
	set_frame(dest,xdim,ydim,0);
	return ( set_blob(sor,dest,0,xdim,ydim,bx,by,200,COPYBLOB,debug) );
}

/* superceded by changeblob.c */
int old_move_blob(sor,dest,xdim,ydim,bx,by,debug)
unsigned char *sor,*dest ;
{	
	return ( set_blob(sor,dest,0,xdim,ydim,bx,by,200,MOVEBLOB,debug) );
}

#endif


void move_holes(sor,dest,xdim,ydim,bx,by)

unsigned char *sor,*dest;
int xdim,ydim,bx,by;

{
	set_blob(sor,dest,0,xdim,ydim,bx,by,200,MOVEHOLES,TEMPLATE,false);
}

/* copy the contents of the src pic ( probably grey-level)
 * which lie within the blob template in the template pic
 * Can either include or exclude the holes within the template */
void copy_greyblob(template,dest,gsor,xd,yd,bx,by)

unsigned char *gsor,*dest,*template ;
int xd,yd,bx,by;

{
	set_blob(template,dest,gsor,xd,yd,bx,by,200,
			COPYGREYBLOB,false) ;
}

void copy_greyenclosure(template,dest,gsor,xd,yd,bx,by)

unsigned char *gsor,*dest,*template ;
int xd,yd,bx,by;

{
	set_blob(template,dest,gsor,xd,yd,bx,by,200,
			COPYGREYENCLOSURE,false) ;
}

/* possible operations so far (opn)
	MOVEBLOB,COPYBLOB,MOVEHOLES 
	COPYGREYBLOB, COPYGREYENCLOSURE
 */

int set_blob(sor,dest,gsor,xdim,ydim,bx,by,pixval,opn,debug)

unsigned char *sor,*dest,*gsor ;
int xdim,ydim,bx,by,pixval,opn,debug;

{
	register int x,y,blobline,in,justin;
	int	area=0  ;
	unsigned char *border;
	void *calloc() ;

	/* (bx,by) has to be a point in the blob */

	border = calloc(xdim*ydim,sizeof(char)) ; /* must be zeroed */
	innerborder(sor,border,xdim,ydim,bx,by,8,200,debug);

	y = by ;
	do {
		in = false ;
		justin = false ;
		blobline = false ;
		for (x=0;x<xdim;x++) {
			if (justin) {
				if (!pix(border,x,y,xdim)) {
					justin = false;
					in = true ;
				}
				if (pix(sor,x,y,xdim)) { 
					OPN(opn,sor,dest,gsor,x,y,xdim)
				}
				else { /*is this a hole or  vertical bar?*/
					/* NB a bar can be 2 pixels wide */
					backtrace(x,y,xdim,sor,dest,gsor,
					    border,&area,debug,pixval,opn);
					break ;
				}
			}
			else if (in) {
				if (pix(sor,x,y,xdim))
					OPN(opn,sor,dest,gsor,x,y,xdim)
				else if (pix(border,x-1,y,xdim))
					in = false; /*not a hole*/
				/* its a hole */
				else if (opn == MOVEHOLES)
					pix(dest,x,y,xdim) = pixval;
				else if (opn == COPYGREYENCLOSURE) {
					pix(dest,x,y,xdim)=
						pix(gsor,x,y,xdim) ;
					area++ ;
				}
			}
			else if ( pix(border,x,y,xdim) ) {
					justin = true ; /*just moved inside*/
					blobline = true ;
					OPN(opn,sor,dest,gsor,x,y,xdim)
			}
		}
		y++;
	} while (blobline && (y<ydim) ) ;

	y = by-1 ;
	do {
		in = false ;
		justin = false ;
		blobline = false ;
		for (x=0;x<xdim;x++) {
			if (justin) {
				if (!pix(border,x,y,xdim)) {
					justin = false;
					in = true ;
				}
				if (pix(sor,x,y,xdim))
					OPN(opn,sor,dest,gsor,x,y,xdim)
				else { /* is this a hole or a verical bar?*/
					backtrace(x,y,xdim,sor,dest,gsor,
					     border,&area,debug,pixval,opn);
					break ;
				}
			}
			else if (in) {
				if (pix(sor,x,y,xdim))
					OPN(opn,sor,dest,gsor,x,y,xdim)
				else if (pix(border,x-1,y,xdim))
					in = false; /*not a hole*/
				/* its a hole */
				else if (opn == MOVEHOLES)
					pix(dest,x,y,xdim) = pixval;
				else if (opn == COPYGREYENCLOSURE) {
					pix(dest,x,y,xdim)=
						pix(gsor,x,y,xdim) ;
					area++ ;
				}
			}
			else  if ( pix(border,x,y,xdim) ) {
					/* just moved inside */
					justin = true ; 
					blobline = true ;
					OPN(opn,sor,dest,gsor,x,y,xdim)
			}
		}
		y--;
	} while (blobline && (y>=0)) ;
	free(border);
	return (area) ;
}

/* This is a horrible routine but its only a bodge to
 * cover up a flaw in the overall algorithm.
 * Allowing this occasional flaw speeds thing up though!!
 */

void backtrace(xst,y,xdim,sor,dest,gsor,border,area,debug,pixval,opn)

unsigned char *sor,*dest,*gsor,*border;
int *area,xst,y,xdim,debug,pixval,opn;

{
	register int  x,in = false,justin = false ;

	for (x=xdim-1; x>=xst;x--) {
		if (justin) {
			if (!pix(border,x,y,xdim)) {
				justin = false;
				in = true ;
			}
			if (pix(sor,x,y,xdim)) 
				OPN(opn,sor,dest,gsor,x,y,xdim)

			/* is this a hole or a vertical bar?*/
			/* for safety assume its a vertical bar*/
			else if (debug) 
				fprintf(stderr,".");
		}
		else if (in) {
			if (pix(sor,x,y,xdim)) 
				OPN(opn,sor,dest,gsor,x,y,xdim)
			else if (pix(border,x+1,y,xdim))
				in = false; /*not a hole*/
			/* its a hole */
			else if (opn == MOVEHOLES)
				pix(dest,x,y,xdim) = pixval;
			else if (opn == COPYGREYENCLOSURE) {
				pix(dest,x,y,xdim)=
					pix(gsor,x,y,xdim) ;
				area++;
			}
		}
		else  if ( pix(border,x,y,xdim) ) {
			justin = true ; /* just moved inside */
			OPN(opn,sor,dest,gsor,x,y,xdim)
		}
	}
}
