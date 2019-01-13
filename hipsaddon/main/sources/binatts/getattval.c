
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "satts.h"

#define true 	1
#define false	0
#define POSPIC	0
#define OUTSIDE	1
#define INSIDE	0



#define hsh(A,B,C)	( ( A -0140) + 40*( B -0140) + 1600*( C -0140) )
#define AREA		hsh('r','a','r')
#define MAXPERIM	hsh('m','a','p')
#define MINPERIM	hsh('m','i','p')
#define PERIMDIFF	hsh('d','f','p')
#define AVPERIM		hsh('a','v','p')
#define MAXLEN		hsh('m','a','l')
#define MAXWIDTH	hsh('m','a','w')
#define XCENT		hsh('x','c','e')
#define YCENT		hsh('y','c','e')
#define PRAXIS		hsh('p','r','a')
#define PALEN		hsh('p','a','l')
#define PAWIDTH		hsh('p','a','w')
#define SYMAXLEN	hsh('s','m','a')
#define SYMINLEN	hsh('s','m','i')
#define HEXTENT		hsh('h','e','x')
#define VEXTENT		hsh('v','e','x')
#define MAXRADIUS	hsh('m','a','r')
#define MINRADIUS	hsh('m','i','r')
#define MAXDIR		hsh('m','a','d')
#define MINDIR		hsh('m','i','d')
#define RADIUSDIFF	hsh('d','f','r')
#define AVRADIUS	hsh('a','v','r')
#define RADRATIO	hsh('r','r','a')
#define MAJVARIANCE	hsh('m','a','v')
#define SMAJVARIANCE	hsh('s','M','v')
#define MINVARIANCE	hsh('m','i','v')
#define SMINVARIANCE	hsh('s','m','v')
#define CGINERTIA	hsh('c','g','i')
#define RADIUSGYRATION	hsh('r','a','g')
#define SRADGYRATION	hsh('s','r','g')
#define MAJSKEW		hsh('m','a','s')
#define MINSKEW		hsh('m','i','s')
#define MAJKURT		hsh('m','a','k')
#define MINKURT		hsh('m','i','k')
#define SHRINKWIDTH	hsh('s','h','w')
#define PROROUND	hsh('p','r','o')
#define AVDISTOPERIM	hsh('a','v','d')
#define CIRCULARITY	hsh('c','i','r')
#define DISTCIRCULARITY	hsh('d','c','i')
#define MAXLENASPRAT	hsh('m','l','a')
#define PAASPRAT	hsh('p','a','a')
#define PASCATRAT	hsh('s','c','a')
#define ELONG		hsh('e','l','o')
#define MAXLENRECT	hsh('m','l','r')
#define PARECT		hsh('p','a','r')
#define HOLLOWSAREA	hsh('h','o','l')
#define HOLECNT		hsh('h','c','n')

#define MJSK	2
#define MISK	3
#define MJKT	4
#define MIKT	5

#define CON8	8
#define CON4	4

#define PIBY2	1.57079
#define Q400PI	1256.636
#define Q100BYPI	31.8310	
#define Q100PI	314.59
#define Q25BYPI	7.9577

float length(),maxlen();
unsigned char *wkpic1 = NULL,*wkpic2 = NULL;
/*extern int debug_pic;*/
void getrawarea(),getperims(),getpraxis(),getshrwidth(),getmmrad();
void getsymdist(),getpalen(),getmaxlen(),getavdist(),getholecnt();
void isolate_blob(),copy_frame(),maxminrad(),getsymdist(),symmetrydists();
void set_frame(),move_blobholes();
float getlength();
int blobsymetries(),blob_area(),blobcentroid(),blobpraxis(),fill();
int blob_8perim(),blob_4perim(),copy_blob(),shrink(),blobcnt();

int getattval(attnam,bp)
char *attnam ;
struct blobstruct *bp ;
{
	switch(hsh(attnam[0],attnam[1],attnam[2])) 
	{
		case AREA:
			getrawarea(bp) ;
			return ((int) bp->rawarea) ;
			break ;
		case MAXPERIM:
			getperims(bp) ;
			return ((int) bp->maxper) ;
			break ;
		case MINPERIM:
			getperims(bp) ;
			return ((int) bp->minper) ;
			break ;
		case PERIMDIFF:
			getperims(bp) ;
			return ((int) (bp->maxper - bp->minper)) ;
			break ;
		case AVPERIM:
			getperims(bp) ;
			return ((int) (bp->avper)) ;
			break ;
		case MAJVARIANCE:
			getpraxis(bp) ;
			return ((int) bp->majvar) ;
			break ;
		case SMAJVARIANCE:
			getpraxis(bp) ;
			getrawarea(bp) ;
			if (!bp->rawarea)
				return(-1) ;
			return ((int) ((bp->majvar*1000)/bp->rawarea)) ;
			break ;
		case MINVARIANCE:
			getpraxis(bp) ;
			return ((int) bp->minvar);
			break ;
		case SMINVARIANCE:
			getpraxis(bp) ;
			getrawarea(bp) ;
			if (!bp->rawarea)
				return(-1) ;
			return ((int) ((bp->minvar*1000)/bp->rawarea));
			break ;
		case CGINERTIA:
		case RADIUSGYRATION:
			/* radius of gyration k is given by:- */
			/* M.(k*k) = m1(r1*r1) + m2(r2*r2)+.. */
			/* this is same as CGinertia / area  */
			getpraxis(bp) ;
			return ((int) (bp->majvar + bp->minvar)) ;
			break ;
		case SRADGYRATION:
			getpraxis(bp);
			getrawarea(bp) ;
			if (!bp->rawarea)
				return(-1) ;
			return ((int) (((bp->majvar + bp->minvar)*100)/
					bp->rawarea)) ;
			break ;
		case MAJSKEW:
			getpraxis(bp);
			return( blobsymetries(bp->pic,bp->xd,bp->yd,
					(int)bp->xc,(int)bp->yc,
					bp->majdir,MJSK,false));
			break ;
		case MINSKEW:
			getpraxis(bp);
			return( blobsymetries(bp->pic,bp->xd,bp->yd,
					(int)bp->xc,(int)bp->yc,
					bp->majdir,MISK,false));
			break ;
		case MAJKURT:
			getpraxis(bp);
			return( blobsymetries(bp->pic,bp->xd,bp->yd,
					(int)bp->xc,(int)bp->yc,
					bp->majdir,MJKT,false));
			break ;
		case MINKURT:
			getpraxis(bp);
			return( blobsymetries(bp->pic,bp->xd,bp->yd,
					(int)bp->xc,(int)bp->yc,
					bp->majdir,MIKT,false));
			break ;
		case SHRINKWIDTH:
			getshrwidth(bp);
			return(bp->shrwid) ;
			break ;
		case ELONG:
			getshrwidth(bp);
			getrawarea(bp) ;
			if (!bp->shrwid)
				return(-1) ;
			return ((int) ((bp->rawarea*100)/
					(bp->shrwid*bp->shrwid*4))) ;
			break ;
		case MAXRADIUS:
			getmmrad(bp);
			return( bp->maxrad) ;
			break ;
		case MINRADIUS:
			getmmrad(bp);
			return( bp->minrad) ;
			break ;
		case MAXDIR:
			getmmrad(bp);
			return( bp->maxdir) ;
			break ;
		case MINDIR:
			getmmrad(bp);
			return( bp->mindir) ;
			break ;
		case RADIUSDIFF:
			getmmrad(bp);
			return( bp->maxrad - bp->minrad) ;
			break ;
		case AVRADIUS:
			getmmrad(bp);
			return ((int) (bp->avrad));
			break ;
		case RADRATIO:
			getmmrad(bp);
			if (!(bp->maxrad + bp->minrad))
				return(-1) ;
			return ((int) ((200*(bp->maxrad - bp->minrad))/
					(bp->maxrad + bp->minrad)));
			break ;
		case HEXTENT:
			return((int)getlength(bp,(float)0));
			break ;
		case VEXTENT:
			return((int)getlength(bp,(float)1.5707986));
			break ;
		case SYMAXLEN:
			getsymdist(bp) ;
			return((int)bp->symaxlen);
			break ;
		case SYMINLEN:
			getsymdist(bp) ;
			return((int)bp->syminlen);
			break ;
		case XCENT:
			getpraxis(bp);
			return ((int) bp->xc) ;
			break ;
		case YCENT:
			getpraxis(bp);
			return ((int) bp->yc) ;
			break ;
		case PRAXIS:
			getpraxis(bp);
			return ((int)((180 * bp->majdir)/PI)) ;
			break ;
		case PALEN:
			getpalen(bp);
			return ((int) bp->palen) ;
			break ;
		case PAWIDTH:
			getpalen(bp);
			return ((int) bp->pawid) ;
			break ;
		case MAXLEN:
			getmaxlen(bp);
			return ((int) bp->maxlen) ;
			break ;
		case MAXLENASPRAT:
			getmaxlen(bp);
			if (!bp->perplen)
				return(-1) ;
			return ((int) ((bp->maxlen*100)/bp->perplen));
			break ;
		case MAXLENRECT:
			getmaxlen(bp);
			getrawarea(bp) ;
			if ((!bp->perplen*bp->maxlen))
				return(-1) ;
			return ((int) ((bp->rawarea*100)/
					(bp->perplen*bp->maxlen)));
			break ;
		case PAASPRAT:
			getpalen(bp);
			if (!bp->pawid)
				return(-1) ;
			return ((int) ((bp->palen*100)/bp->pawid)) ;
			break ;
		case PASCATRAT:
			getpraxis(bp);
			if (!bp->minvar)
				return(-1) ;
			return ((int) ((bp->majvar*100)/bp->minvar)) ;
			break ;
		case PARECT:
			getpalen(bp);
			getrawarea(bp) ;
			if (!(bp->palen*bp->pawid))
				return(-1) ;
			return ((int) ((bp->rawarea*100)/
				(bp->palen*bp->pawid)));
			break ;
		case PROROUND:
			getperims(bp) ;
			getrawarea(bp) ;
			if (!bp->rawarea)
				return(-1) ;
			return ((int) ((bp->avper*bp->avper*Q25BYPI)/
					(bp->rawarea)));
			break ;
		case CIRCULARITY:
			getmmrad(bp);
			getrawarea(bp) ;
			if (!bp->rawarea)
				return(-1) ;
			return ((int) ((bp->avrad*bp->avrad*Q100PI)/
					bp->rawarea));
			break ;
		case AVDISTOPERIM:
			getavdist(bp);
			return ((int) (bp->avdist)) ;
			break ;
		case DISTCIRCULARITY:
			getavdist(bp);
			getrawarea(bp) ;
			if (!bp->avdist)
				return(-1) ;
			return ((int) (bp->rawarea/
						(bp->avdist*bp->avdist)));
			break ;
		case HOLLOWSAREA:
			getmaxlen(bp);
			getrawarea(bp) ;
			if (!(bp->rawarea+bp->bparea))
				return(-1) ;
			return ((int) ((bp->bparea*100)/
					(bp->bparea + bp->rawarea)));
			break ;
		case HOLECNT:
			getholecnt(bp) ;
			return(bp->holecnt) ;
			break ;
		default:
			/* printf("unknown attribute!\n"); */
			return(-1) ;
	}
}

void getrawarea(bp)
struct blobstruct *bp ;
{
	if (bp->rawarea == NOT_DONE)
		bp->rawarea = blob_area(bp->pic,bp->xd,bp->yd,
					bp->xst,bp->yst,false);
}

void getpraxis(bp)
struct blobstruct *bp ;
{
	if (bp->majdir == NOT_DONE) {
		if (bp->xc == NOT_DONE)
			blobcentroid(bp->pic,bp->xd,bp->yd,bp->xst,bp->yst,
					&bp->xc,&bp->yc,false);
		blobpraxis(bp->pic,bp->xd,bp->yd,bp->xst,bp->yst,
				&bp->xc,&bp->yc,&bp->majdir,&bp->majvar,
				&bp->minvar,false); 
	}
}

float getlength(bp,dir)
struct blobstruct *bp ;
float dir ;
{
	isolate_blob(bp) ;
	return(length(bp->bo_pic,wkpic2,
			bp->xd,bp->yd,bp->xst,bp->yst,dir));
}

void getmaxlen(bp)
struct blobstruct *bp ;
{	float dir ;

	/* pic=orig pic; wkpic1=bounding poly; wkpic2=work pic */

	if (bp->maxlen == NOT_DONE) {
		/* length needs a singleblob frame..
		 * it can possibly be changed to trace the blobs border though..
		 */
		isolate_blob(bp) ;
		/* on exit wkpic1 will contain all on the tangent calipers
		 * generated during the length calc */
		bp->maxlen= maxlen(bp->bo_pic,wkpic1,wkpic2,bp->xd,bp->yd,
					bp->xst,bp->yst,&dir);
		bp->perplen= length(bp->bo_pic,wkpic2,bp->xd,bp->yd,
					bp->xst,bp->yst,dir+PIBY2);

		/* compute area of bounding polygon */
		copy_frame(bp->bo_pic,wkpic2,bp->xd,bp->yd) ;
		bp->bparea = fill(wkpic1,wkpic2,bp->xd,bp->yd);
		/*debug_dispic(wkpic2,bp->xd,bp->yd,true,"bdg.poly") ;*/
	}
}

void getpalen(bp)
struct blobstruct *bp ;
{
	if (bp->palen == NOT_DONE) {
		getpraxis(bp);
		/* length needs a singleblob frame..
		 * it can possibly be changed to trace the blobs border though..
		 */
		isolate_blob(bp) ;
		bp->palen=length(bp->bo_pic,wkpic1,bp->xd,bp->yd,
					bp->xst,bp->yst,bp->majdir) ;
		bp->pawid=length(bp->bo_pic,wkpic1,bp->xd,bp->yd,
					bp->xst,bp->yst,bp->majdir+PIBY2) ;
	}
}

void getperims(bp)
struct blobstruct *bp ;
{
	if (bp->maxper == NOT_DONE) {

		/* old method - this gives perim including holes perims */
		/*
		copy_blob(bp->pic,wkpic1,bp->xd,bp->yd,bp->xst,bp->yst,false);
		bp->maxper=shrink(wkpic1,wkpic2,bp->xd,bp->yd,CON8) ;
		bp->minper=shrink(wkpic1,wkpic2,bp->xd,bp->yd,CON4) ;
		bp->avper =((bp->maxper)+(bp->minper))/2 ;
		fprintf(stderr,"old maxp=%d, old mip=%d\n",
					bp->maxper,bp->minper); */

		/* new method */
		/*debug_dispic(bp->pic,bp->xd,bp->yd,true,"about to trace 8per");*/
		bp->minper= blob_8perim(bp->pic,bp->xd,bp->yd,
						bp->xst,bp->yst,false);	
		/*debug_dispic(bp->pic,bp->xd,bp->yd,true,"about to trace 4per");*/
		bp->maxper= blob_4perim(bp->pic,bp->xd,bp->yd,
						bp->xst,bp->yst,false);	
		bp->avper =((bp->maxper)+(bp->minper))/2 ;
	}
}

void getmmrad(bp)
struct blobstruct *bp ;
{
	getpraxis(bp);
	if (bp->maxrad == NOT_DONE) {
		/* old method using expands and shrinks******
		mmrad(bp->pic,wkpic1,wkpic2,bp->xd,bp->yd,bp->xc,bp->yc,
					&bp->maxrad,&bp->minrad);
		**********************************/
		maxminrad(bp->pic,bp->xd,bp->yd,bp->xc,bp->yc,
				&bp->maxrad,&bp->minrad,
				&bp->maxdir,&bp->mindir) ;
		bp->avrad = (bp->maxrad + bp->minrad)/2 ;
	}
}

void getsymdist(bp)
struct blobstruct *bp ;
{
	getpraxis(bp);
	if (bp->symaxlen == NOT_DONE) {
		symmetrydists(bp->pic,bp->xd,bp->yd,
				bp->xc,bp->yc,bp->majdir,
				&bp->symaxlen,&bp->syminlen);
	}
}

void getavdist(bp)
struct blobstruct *bp ;
{
	int j,cnt ;
	float dist ;
	
	if (bp->avdist != NOT_DONE)
		return ;
	dist = 0 ;
	j = 1 ;
	set_frame(wkpic1,bp->xd,bp->yd,0) ;
	bp->rawarea= copy_blob(bp->pic,wkpic1,bp->xd,bp->yd,
						bp->xst,bp->yst,false);
	do
	{	if (j%2)
			cnt=shrink(wkpic1,wkpic2,bp->xd,bp->yd,8);
		else
			cnt=shrink(wkpic2,wkpic1,bp->xd,bp->yd,4);
		dist += cnt*j ;
		j++ ;
	}
	while ( cnt ) ;
	bp->avdist = dist/(bp->rawarea);
}

void getshrwidth(bp)
struct blobstruct *bp ;
{
	int shrwid,cnt ;

	if (bp->shrwid != NOT_DONE)
		return ;
	set_frame(wkpic1,bp->xd,bp->yd,0) ;
	bp->rawarea= copy_blob(bp->pic,wkpic1,bp->xd,bp->yd,
						bp->xst,bp->yst,false);
	shrwid = 1 ;
	do
	{	if ( (shrwid++)%2 )
			cnt=shrink(wkpic1,wkpic2,bp->xd,bp->yd,8);
		else
			cnt=shrink(wkpic2,wkpic1,bp->xd,bp->yd,4);
	}
	while ( cnt ) ;
	bp->shrwid = --shrwid ;
}

void getholecnt(bp)
struct blobstruct *bp ;
{
	if (bp->holecnt != NOT_DONE)
		return ;
	move_blobholes(bp),
	/* set a minumum hole area of 3 pixels to eliminate noise*/
	bp->holecnt = blobcnt(bp->holepic,bp->xd,bp->yd,4,false);
}

/* when binatts is next compiled it will need to change
   all occurences of pre_getattval() to
   pre_getattval(xdim,ydim).
   Note also that no cleaning is now performed in pre_getattval */

void pre_getattval(xdim,ydim)

int xdim,ydim;

{	
	/* bineye calls this every time a new pic is captured */
	/* the new pic may or may not be same size as old pic */
	if ( wkpic1 != NULL)
		free(wkpic1);
	if ( wkpic2 != NULL)
		free(wkpic2);
	if ( ((wkpic1 = (unsigned char *)malloc(xdim*ydim))==NULL) ||
	     ((wkpic2 = (unsigned char *)malloc(xdim*ydim))==NULL) ) {
	      fprintf(stderr,"malloc of work-pics failed!\n");;
	      exit(-1) ;
	}
}

void isolate_blob(bp)
struct blobstruct *bp ;
{
	if (bp->bo_pic != NULL)
		free(bp->bo_pic) ;
	bp->bo_pic = (unsigned char *)calloc(bp->xd*bp->yd,sizeof(char)) ;
	bp->rawarea = copy_blob(bp->pic,bp->bo_pic,bp->xd,bp->yd,
					bp->xst,bp->yst,false);
}

void isolate_blobs(bp)
struct blobstruct *bp ;
{
	if (bp->bo_pic == NULL)
		bp->bo_pic = (unsigned char *)calloc(bp->xd*bp->yd,sizeof(char)) ;
	bp->rawarea = copy_blob(bp->pic,bp->bo_pic,bp->xd,bp->yd,
					bp->xst,bp->yst,false);
}
