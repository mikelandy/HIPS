
#include "satts.h"
#include <math.h>
#define false	0

void set_frame();
int expand(),borderlist(),AinB(),abs();

/*mmrad(pic,wo,wn,xdim,ydim,xc,yc,maxrad,minrad) old system*/

void mmrad(pic,wo,wn,xdim,ydim,xc,yc,maxr,minr)

unsigned char *pic,*wo,*wn;
int xdim,ydim;
float xc,yc;
int *maxr,*minr;

{
	unsigned char *temp ;
	register int change,type ;

	*minr = 0 ;
	type = 4 ;
	change = 4 ;
	set_frame(wo,xdim,ydim,0) ;
	pix(wo,(int)xc,(int)yc,xdim) = 100 ;

	while ( AinB(wo,pic,xdim,ydim) )
	{	expand(wo,wn,xdim,ydim,type) ;
		temp = wo ;
		wo = wn ;
		wn = temp ;
		type = type + change ;
		change = (-change) ;
		*minr += 1 ;
	}
	*maxr = *minr ;
	while ( !AinB(pic,wo,xdim,ydim) )
	{	expand(wo,wn,xdim,ydim,type) ;
		temp = wo ;
		wo = wn ;
		wn = temp ;
		type = type + change ;
		change = (-change) ;
		*maxr += 1 ;
	}
}

/* determine max & min radius and their directions
 * (relative to the centre of mass)
 */

void maxminrad(pic,xd,yd,xc,yc,maxrad,minrad,maxdir,mindir)

unsigned char *pic;
int xd,yd;
float xc,yc;
int *maxrad,*minrad;
int *maxdir,*mindir;

{
	int len,i ;
	int dist,maxdist = 0, mindist = 100000,mini=0,maxi=0 ;
	int blist[MAXBORDLEN][2],xgap,ygap ;

	len = borderlist(pic,blist,xd,yd,(int)xc,(int)yc,false);
	for (i=0;i<len;i++) {
		xgap = blist[i][0] - xc ;
		ygap = blist[i][1] - yc ;
		dist = xgap*xgap + ygap*ygap ;
		if (dist > maxdist) {
			maxdist = dist ;
			maxi = i ;
		}
		if (dist < mindist) {
			mindist = dist ;
			mini = i ;
		}
	}
	*maxrad = (int)sqrt((double)maxdist) ;
	*minrad = (int)sqrt((double)mindist) ;
	xgap = blist[maxi][0] - xc ;
	ygap = blist[maxi][1] - yc ;
	if (xgap)
		*maxdir = (int) ((atan2((double)ygap,(double)xgap) * 180)/PI) ;
	else
		*maxdir = 0 ;
	xgap = blist[mini][0] - xc ;
	ygap = blist[mini][1] - yc ;
	if (xgap)
		*mindir = (int) ((atan2((double)ygap,(double)xgap) * 180)/PI) ;
	else
		*maxdir = 0 ;
}

/* compute max and min perp distance of perimeter points to the 
 * a line draw thro the centroid at rads radians 
 */
void symmetrydists(pic,xd,yd,xc,yc,rads,maxdst,mindst)

unsigned char *pic ;
int xd,yd;
float xc,yc,rads ;
int *maxdst,*mindst ;

{
	int len,i ;
	int dist,abdist,signedmax,signedmin,maxdist = 0, mindist = 100000,mini,maxi ;
	int blist[MAXBORDLEN][2],xgap,ygap ;
	float a,c,root ;

	/* perp dist of pnt (x1,y1) to Ax + By + C = 0 
	 * is (Ax1 + By1 + C)/sqrt(A*A + B*B)
	 */
	a = tan((double)rads) ;
	/* B is -1 */
	c = yc - a * xc ;
	root = sqrt( (double)a*a + 1) ;

	len = borderlist(pic,blist,xd,yd,(int)xc,(int)yc,false);
	for (i=0;i<len;i++) {
		dist = (a*blist[i][0] - blist[i][1] + c)/root ;
		abdist = abs(dist) ;
		if (abdist > maxdist) {
			maxdist = abdist ;
			signedmax = dist ;
			maxi = i ;
		}
		if (abdist < mindist) {
			mindist = abdist ;
			signedmin = dist ;
			mini = i ;
		}
	}

	/* alter the sign of the distances so that +ve dist means
	 * to the right of the line specifiey by rads
	 * and -ve to the left */
	/*
	xgap = blist[maxi][0] - xc ;
	ygap = blist[maxi][1] - yc ;
	if (xgap)
		i = (int) atan2((double)ygap,(double)xgap) ;
	else
		i = 0 ;
	if (i < rads)	   to the left in a clockwise sense? 
	  
		maxdist = -maxdist ;
	xgap = blist[mini][0] - xc ;
	ygap = blist[mini][1] - yc ;
	i = (int) atan2((double)ygap,(double)xgap) ;
	if (i < rads)
		mindist = -mindist ;
	*/

	*maxdst = signedmax ;
	*mindst = signedmin ;
}

