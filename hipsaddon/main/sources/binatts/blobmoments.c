/* Compute moments of a blob
   converted from moments.c (ie compute moments
   of a complete frame march,86 (B.Shepherd) */

#include <stdio.h>
#include <math.h>

/* for grey level pics make PIXVAL = (*pixptr) */
#define PIXVAL	1
#define MY_PI	3.14159
#define true 	1
#define false	0
#define M20	0
#define M02	1
#define M30	2
#define M03	3
#define M40	4
#define M04	5
#define	MAXMOM	6

extern int x,y ;	/* these are in blobop.c */

/* globals so that both blobpraxis & blobsymetries() have access */
int momt[MAXMOM],highermoms_done ;
double cm20,cm02 ;
void xy_sums(),xy_sumsqs(),high_ssqs();

/* globals so that blobpixfn() has access to them! */
int xcnt,ycnt,blobop();
extern int area;	
double xinrt,yinrt,xyinrt ;
float cosmj,sinmj ;
double cm30,cm03,cm40,cm04;
void Qroots(),get_inc(),find_end(),free();
unsigned char *getpicbuf();
int abs();

int blobcentroid(pic,xd,yd,bx,by,xc,yc,debug)

unsigned char *pic ;
int xd,yd,bx,by,debug;
float *xc,*yc ;

{
	/* init globals for blobop() */
	xcnt = ycnt = area = 0 ;

	blobop(pic,xd,yd,bx,by,xy_sums,debug);
	*xc = (float)xcnt/area ; 
	xcnt = *xc ;	/* for central moments to use */
	*yc = (float)ycnt/area ; 
	ycnt = *yc ;
	return(area);
}


/*--------------------------------------------*/
/* compute 2nd order central moments.
 * blob is at (bx,by) & also at (xc,yc) if centroid
 * has previously been found (ie (xc,yc) != (0,0)) */

int blobpraxis(pic,xd,yd,bx,by,xc,yc,majdir,majvar,minvar,debug)

unsigned char *pic;
int xd,yd,bx,by,debug;
float *xc,*yc;
double *majdir,*majvar,*minvar;

{
	double diff ;
	float x1,y1,x2,y2,x3,y3,incx,incy,len1,len2 ;
	unsigned char *tanpic ;

	*majdir = *majvar = *minvar = 0 ;
	highermoms_done= false ;

	/* use xc = 0 to indicate the centroid has not been found */
	if (!(*xc))
		blobcentroid(pic,xd,yd,bx,by,xc,yc,debug) ;

	/* init.globals for blobop() */
	xyinrt = xinrt = yinrt = 0 ;

	/* blobop fails is there is no blob!! */
	if (!blobop(pic,xd,yd,bx,by,xy_sumsqs,debug))
		return(false) ;

	/* maxinertia = inertia along major axis (similar to varaiance)) */
	Qroots(1.0,-xinrt-yinrt,xinrt*yinrt-xyinrt*xyinrt,&cm20,&cm02);

	/* ensure radians in range 0 -> pi 
	 * if diff = 0 then its a symmetrical blob in both axes
	 * (e.g a square) */ 

	diff = xinrt -yinrt;
	if (diff) 
		*majdir = atan2(2*xyinrt,diff)/2 ;
	else
		*majdir = 0 ; /* could also be along Y axis ! */

	cm20 = cm20/area ;
	cm02 = cm02/area ;
	*majvar = cm20 ;
	*minvar = cm02 ;

	/* try and associate a direction to the principal axis: 
	   so calc dist of centroid to each end of praxis and point 
	   the pr.axis in the longest direction (7/2/90) */
	/* This code based on code in bloblen.c */

	get_inc(&incx,&incy,(float)*majdir) ;
	x1 = x3 = bx ;
	y1 = y3 = by ;

	tanpic = getpicbuf(xd,yd) ;
	find_end(pic,tanpic,xd,yd,&x1,&y1,incx,incy,1);
	x2 = x1 + (100 * incy) ;
	y2 = y1 - (100 * incx) ;
	len1 = ( x1*(y2- (*yc)) + x2*((*yc)-y1) + (*xc)*(y1-y2) ) /
			hypot((double) x2-x1 , (double) y2-y1 ) ;

	find_end(pic,tanpic,xd,yd,&x3,&y3,-incx,-incy,1);
	x2 = x3 + (100 * incy) ;
	y2 = y3 - (100 * incx) ;
	len2 = ( x3*(y2-(*yc)) + x2*((*yc)-y3) + (*xc)*(y3-y2) ) /
			hypot((double) x2-x3 , (double) y2-y3 ) ;
	if (len1 < len2) 
		*majdir += MY_PI ;
	free(tanpic) ;

	return(true) ;
}

void xy_sums(x,y)

int x,y;

{
	area++ ;
	xcnt += x ;	/* times PIXVAL if greylevel */
	ycnt += y ;	/* ditto  */
}

/* xcnt & ycnt could be float for greater accuracy
 * but this would slow things down */
void xy_sumsqs(x,y)

int x,y;

{
	register int cx,cy ;

	cx = x - xcnt ;
	cy = y - ycnt ;
	xyinrt += cx * cy ;	/* *(PIXVAL) for grey*/
	xinrt += cx * cx ;	/* ditto */
	yinrt += cy * cy ;	/* ditto */
}

void high_ssqs(x,y)

int x,y;

{
	register int cx,cy,nx,ny,nx2,ny2 ;

	cx = x - xcnt ;
	cy = y - ycnt ;
	nx = cx*cosmj + cy*sinmj ;
	ny = cy*cosmj - cx*sinmj ;
	nx2= nx*nx ;
	ny2= ny*ny ;

	/* symetry about min & maj axis */
	cm30 += nx2*nx  ;
	cm03 += ny2*ny  ;
	/* kurtosis about major & minor axis */
	cm40 += nx2*nx2  ;
	cm04 += ny2*ny2  ;

	/* symetries about origin */
	/*cm12 += nx*ny2  ;
	cm21 += nx2*ny  ;
	cm31 += nx2*nx*ny  ;
	cm13 += ny2*ny*nx  ;
	cm22 += nx2*ny2  ;
	*/
}

/*---------------------------------------------------------------*/
/* compute 3rd & 4th moments about the centroid
 * using majdir & mindir as co-od axis
 * this assumes centroid has been found */

int blobsymetries(pic,xd,yd,xst,yst,majdir,code,debug)

unsigned char *pic;
int xd,yd,xst,yst,code,debug;
float majdir;

{	
	double index,x ;
	int  i ;

	if (highermoms_done)
		return(momt[code]);

	/* init. globals for blobop() */
	cm03 = cm30 = cm40 = cm04 = 0 ;
	cosmj = cos((double)majdir) ;
	sinmj = sin((double)majdir) ;
	blobop(pic,xd,yd,xst,yst,high_ssqs,debug);

	for (i=0;i<6;i++)
		momt[i] = 0 ;

	/* skewness & kurtosis in directions of principal axes */
	/* div by area to make magnif invariant (castleman P328) (seems wrong)*/
	/*div by appr sdev power to make indep of scale of data (snedecor p86)*/

	index = 1.5 ;
	if ((x=pow(cm02,index)*area))
		momt[M03] =  100*cm03/x ;
	if ((x=pow(cm20,index)*area))
		momt[M30] =  100*cm30/x ;
	if ((x=cm02*cm02*area))
		momt[M04] =  100*cm04/x ;
	if ((x=cm20*cm20*area))
		momt[M40] =  100*cm40/x ;
	momt[M03] = abs(momt[M03]) ;
	momt[M30] = abs(momt[M30]) ;
	highermoms_done = true ;
	return(momt[code]) ;
}

/*---------------------------------------------------*/
/* roots of a quadratic egn.  */

void Qroots(a,b,c,bigroot,smroot)
double a,b,c,*bigroot,*smroot ;
{
	double i;
	i = sqrt(b*b - 4*a*c) ;
	*bigroot= (-b+i)/2*a ;
	*smroot = (-b-i)/2*a ;
	if (*bigroot < *smroot)
	{	i = *bigroot ;
		*bigroot = *smroot ;
		*smroot = i ;
	}
}

