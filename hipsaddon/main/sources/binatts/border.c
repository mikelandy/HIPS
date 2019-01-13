/* Find the border of a specified blob.
 * ----------------------------------------
 * These functions operate on single blobs only
 * as opposed to the whole frame, and use a border following
 * algorithm independent of the size of the picture.
 * The source frame can contain any number of blobs.
 * The blob to be operated on is specified by giving the co-ods
 * of a point inside it, typically the centriod (xc,yc).
 *
 * innerborder returns a frame containing the 8 or 4 connected perimeter
 * of the blob, where the perimeter is part of the blob,
 * outer8border does the same but the border is in the background.
 * In both cases the value of the border pixels is specified
 * as an argument (newval).
 *
 * blob_8shrink shrinks the specified blob by removing its 8
 * connected innner border.
 * blob_8expand expands the specified blob by adding its 8 connected
 * outer border to it.
 *
 * B.A.Shepherd, 1984.
 */

#include <stdio.h>
#define pix(a,x,y,xdim)	(*((unsigned char *) (a + (x+(y)*(xdim)))))
#define POSPIC 0
#define NEGPIC 1
#define ROOT2	1.41421

/* the neighbours of a pixel are labelled:
 *	0 1 2
 *	7   3
 *	6 5 4
 *
 * the start position for the clockwise search for a border pixel
 * given the relation the latest one is to the one before is (next_start):
 *
 * (	6   0    0
 *	6  Prev  2
 *	4   4    2 )
 *
 *   
 *	5   7    7
 *	5  Prev  1
 *	3   3    1
 */

int ncord[8][2] = {
	-1 , -1 ,
	 0 , -1 ,
	 1 , -1 ,
	 1 ,  0 ,
	 1 ,  1 ,
	 0 ,  1 ,
	-1 ,  1 ,
	-1 ,  0 } ;

int next_8start[8] = {6,0,0,2,2,4,4,6};
int next_4start[8] = {5,7,7,1,1,3,3,5};

int innerborder(),outer8border();

/* NB src is NOT corrupted in blob_4perim or blob_8perim (see newval bits)*/

int blob_8perim(src,xdim,ydim,xc,yc,debug)	/* (xc,yc) is any point in the blob*/

unsigned char *src ;
int xdim,ydim,xc,yc,debug;

{
	return (innerborder(src,src,xdim,ydim,xc,yc,8,-1,debug));
}	

int blob_4perim(src,xdim,ydim,xc,yc,debug)	/* (xc,yc) is any point in the blob*/

unsigned char *src ;
int xdim,ydim,xc,yc,debug;

{
	return (innerborder(src,src,xdim,ydim,xc,yc,4,-1,debug));
}	

/* what if the blob splits into 2 or more smaller blobs?
 * will innerborder() ever terminate? */

int blob_8shrink(src,xdim,ydim,xc,yc,debug)	/* (xc,yc) is any point in the blob*/

unsigned char *src ;
int xdim,ydim,xc,yc,debug;

{
	return (innerborder(src,src,xdim,ydim,xc,yc,8,0,debug));
}	

int blob_4shrink(src,xdim,ydim,xc,yc,debug)	/* (xc,yc) is any point in the blob*/

unsigned char *src;
int xdim,ydim,xc,yc,debug;

{
	return (innerborder(src,src,xdim,ydim,xc,yc,4,0,debug));
}	

int blob_8expand(src,xdim,ydim,xc,yc,debug)	/* (xc,yc) is any point in the blob*/

unsigned char *src ;
int xdim,ydim,xc,yc,debug;

{
	return (outer8border(src,src,xdim,ydim,xc,yc,190,debug));
}	

/* in this one src contains only a closed border (ie no filling) 
 * the aim is only to compute the area enclosed by this border
 */
int inside_area(src,xd,yd,xc,yc,debug)	/* (xc,yc) is pnt on the border*/

unsigned char *src;
int xd,yd,xc,yc,debug;

{
	return(innerborder(src,src,xd,yd,xc,yc,8,-3,debug)) ;
}

/* this is the odd one out, blist isn't a picy but is a 2D array of integers.
   The co-ords of each pixel are to be loaded into blist so that:
	(blist[i][0],blist[i][1]) are the (x,y) co-ords of the ith border pixel
   NB. An  8-connected border only is computed.
*/
int borderlist(src,blist,xd,yd,bx,by,debug)

unsigned char *src,*blist;
int xd,yd,bx,by,debug;

{
	return( innerborder(src,blist,xd,yd,bx,by,8,-2,debug)) ;
}

/* type can be 8 or 4 (connected) */
int innerborder(src,dest,xdim,ydim,sx,sy,type,newval,debug)

register unsigned char *src,*dest;
int xdim,ydim,sx,sy,type,newval,debug;

{
	register int bx,by,x,y,pixel,bordcnt,outside,innerarea ;
	register int *blist;

	blist = (int *)dest;

	/*fprintf(stderr,"the val of bord pixs are %d %d %d\n",
		pix(src,sx-1,sy,xdim),
		pix(src,sx,sy,xdim),
		pix(src,sx+1,sy,xdim)); */

	while ( pix(src,sx,sy,xdim) && (sx >= 0) ) sx-- ; 
	sx++ ;

	bordcnt = 0 ;
	innerarea = 0 ;

	if (type == 8)
		outside = 0 ;		/* we know outside=7 (ie to the left)*/
	else
		outside = 1 ;	        /* is not in the blob
					 * so first try should be:
						outside=0  for 8 connected
						outside=1  for 4 connected */
	x = sx ;
	y = sy ;

	do {
		/* label current border pixel */
		if (newval >= 0)
			pix(dest,x,y,xdim) = newval;
		else if (newval == -1)		/* set equal to src pic vals */
			pix(dest,x,y,xdim) = pix(src,x,y,xdim);
		else if (newval == -2) {
			/*-----------------------------------------*/
			/* generate info. for border segmentation
			   dest is to be considered here not a a picture
			   pointer but as a 2d integer array which 
			   will hold the border co-ords */
			blist[bordcnt*2] = x;
			blist[bordcnt*2 + 1] = y;
			/*-----------------------------------------*/
		}
		/* else if newval < -2 nothing is done */
		/*
		if (debug)
			putpixel(x,y,240);
		*/
		bordcnt += 1 ;
		/* search neighbourhood until next blob pixel found */
		do {
			bx = x + ncord[outside%8][0] ;
			by = y + ncord[outside%8][1] ;
			/* ignore pixels outside frame*/
			if ((bx!=-1)&&(by!=-1)&&(bx!=xdim)&&(by!=ydim)) {
				if (pix(src,bx,by,xdim)) 
					break;	/* found next border pixel*/
			}
			if (type == 4)
				outside += 2;	/* ignore diagonal neighbours
						   ie outside can only be:-
						    1,3,5,7,9,11,13 */
			else
				outside++ ;
		}
		while ( outside < 14 ) ;	
		if (outside >= 14)	/* must be a single pixel blob! */
			break ;
		/* jump to this next border pixel*/
		x = bx ;
		y = by ;

		/* compute the area inside (& including) the border.
		 * (outside % 8) will be one of:-
		 *  0 1 2
		 *  7   3
		 *  6 5 4
		 * when we move rightwards add on the ypos
		 * when we move leftwards subtract the ypos */

		switch(outside % 8) {
			case 0:
			case 6:
			case 7:
				innerarea += by + 1 ;
				break ;
			case 2:
			case 3:
			case 4:
				innerarea -= by ;
				break ;
		};

		/* find new outside pixel in relation to new border pixel */
		if (type == 4)
			outside = next_4start[(outside) % 8] ;
		else
			outside = next_8start[(outside) % 8] ;
	}
	while ((x != sx) || (y != sy)) ;
	if (newval < -2)
		return(innerarea);
	if (newval < -1)
		blist[bordcnt*2] = -1 ;	/* list delimeter */
	return(bordcnt) ;
}

int outer8border(src,dest,xdim,ydim,sx,sy,newval,debug)

unsigned char *src,*dest;
int xdim,ydim,sx,sy,newval,debug;

{
	register int bx,by,x,y,pixel,bordcnt,outside ;

	while ( pix(src,sx,sy,xdim) ) sx-- ; 
	sx++ ;

	bordcnt = 0 ;
	outside = 7 ;			/* to the left of start pixel*/
	x = sx ;
	y = sy ;

	do {
		/* find next border pixel */
		do
		{	
			bx = x + ncord[outside%8][0] ;
			by = y + ncord[outside%8][1] ;
			outside++ ;
			if ((bx==-1)||(by==-1)||(bx==xdim)||(by==ydim))
				continue ;
			if (pix(src,bx,by,xdim)) 
				break ;			/* found it! */
			pix(dest,bx,by,xdim) = newval ;
		}
		while ( outside < 14 ) ;	

		x = bx ;
		y = by ;
		outside = next_8start[outside % 8] ;
		bordcnt += 1 ;
	}
	while ((x != sx) || (y != sy)) ;
	return(bordcnt) ;
}

/* type can be 8 or 4 (connected) */
float borderlength(src,xdim,ydim,stx,sty,finx,finy,type,debug)

register unsigned char *src;
int xdim,ydim,stx,sty,finx,finy,type,debug;

{
	register int bx,by,x,y,pixel,outside ;
	float bordlen = 0 ;

	x = stx ;
	y = sty ;

	/* find the background! any background pixel will do */
	for (outside=0;outside<8;outside++) {
		bx = x + ncord[outside%8][0] ;
		by = y + ncord[outside%8][1] ;
		/* ignore pixels outside frame*/
		if ((bx!=-1)&&(by!=-1)&&(bx!=xdim)&&(by!=ydim)) {
			if (!pix(src,bx,by,xdim)) 
					break;	/* found a background pixel*/
		}
	}

	do {
		/* search neighbourhood until next blob pixel found */
		do {
			bx = x + ncord[outside%8][0] ;
			by = y + ncord[outside%8][1] ;
			/* ignore pixels outside frame*/
			if ((bx!=-1)&&(by!=-1)&&(bx!=xdim)&&(by!=ydim)) {
				if (pix(src,bx,by,xdim)) 
					break;	/* found next border pixel*/
			}
			if (type == 4)
				outside += 2;	/* ignore diagonal neighbours
						   ie outside can only be:-
						    1,3,5,7,9,11,13 */
			else
				outside++ ;
		}
		while ( outside < 14 ) ;	

		if (outside >= 14)	/* must be a single pixel blob! */
			break ;
		if (outside%2)		/* even nos mean a diagonal jump */
			bordlen += 1 ;
		else
			bordlen += ROOT2 ;

		/* jump to this next border pixel*/
		x = bx ;
		y = by ;

		/* find new outside pixel in relation to new border pixel */
		if (type == 4)
			outside = next_4start[(outside) % 8] ;
		else
			outside = next_8start[(outside) % 8] ;

	}
	while ((x != finx) || (y != finy)) ;
	return(bordlen) ;
}
