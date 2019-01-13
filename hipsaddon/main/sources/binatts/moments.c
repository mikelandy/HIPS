/* Compute moments of picture */

#include <stdio.h>
#include <math.h>

#define MAXX	96
#define MAXY	96
/* for grey level pics make PIXVAL = (*pixptr) */
#define PIXVAL	1
#define TRUE 	1
#define FALSE	0
#define M20	0
#define M02	1
#define M30	2
#define M03	3
#define M40	4
#define M04	5
#define	MAXMOM	6

int mom[MAXMOM],highermomsdone,abs();
float farea ;
double m20,m02 ;
void qroots();

/* ------------------------------------*/
/* compute x & y centriods          */

int centroid(buf,xdim,ydim,xc,yc)

unsigned char *buf ;
int xdim,ydim;
float *xc,*yc ;

{
	register int x,y ;
	register unsigned char *pixptr ;
	float fxcent,fycent;

	farea = 0 ;
	fxcent = 0 ;
	fycent = 0 ;
	pixptr = buf;

	for (y=0;y<ydim;y++)
		for (x=0;x<xdim;x++)
			if (*(pixptr++))
			{	farea += 1 ;
				fxcent += x ;	/* times PIXVAL if greylevel */
				fycent += y ;   /* ditto  */
			}
	*xc = fxcent/farea ;
	*yc = fycent/farea ;
	return( (int)farea);
}

/*--------------------------------------------*/
/* compute 2nd order central moments                    */

void praxis(buf,xdim,ydim,xcent,ycent,majdir,majvar,minvar,given_centriod)

unsigned char *buf ;
int xdim,ydim,given_centriod;
float *xcent,*ycent ;
double *majdir ;
double *majvar,*minvar ;

{
	register int x,y ;
	register unsigned char *pixptr ;
	int topx,topy ;
	double xinert,yinert,xyinert ;

	if (!given_centriod)
		centroid(buf,xdim,ydim,xcent,ycent) ;
	xyinert = xinert = yinert = 0 ;
	topx = xdim - (int)(*xcent) ;
	topy = ydim - (int)(*ycent) ;
	pixptr = buf ;

	for (y=(-(int)(*ycent));y<topy;y++)
		for (x=(-(int)(*xcent));x<topx;x++)
			if (*(pixptr++))
			{	xyinert += x * y ;	/* *(PIXVAL) for grey*/
				xinert += x * x ;	/* ditto */
				yinert += y * y ;	/* ditto */
			}

	/* maxinertia = inertia along major axis (similar to varaiance)) */

	qroots(1.0,-xinert-yinert,xinert*yinert-xyinert*xyinert,&m20,&m02);

	/* ensure radians in range 0 -> pi */
	/* *majdir = atan2(m20-xinert,xyinert) ;  old (incorrect!) way */
	*majdir = atan2(2*xyinert,xinert-yinert)/2 ;

	m20 = m20/farea ;
	m02 = m02/farea ;
	*majvar = m20 ;
	*minvar = m02 ;
	highermomsdone= FALSE ;
}

/*---------------------------------------------------------------*/
/* compute 2nd,3rd & 4th moments using majdir & mindir as co-od axis */

int symetries(buf,xdim,ydim,xcent,ycent,majdir,code)

unsigned char *buf;
int xdim,ydim,code;
float majdir;
float xcent,ycent;

{	
	register int x,y ;
	register unsigned char *pixptr ;
	int topx,topy ;
	float nx,ny,nx2,ny2 ;
	float cosmaj,sinmaj ;
	double m30,m03,m40,m04,index ;

	if (highermomsdone)
		return(mom[code]);

	m03 = m30 = m40 = m04 = 0.0 ;
	cosmaj = cos((double)majdir) ;
	sinmaj = sin((double)majdir) ;
	topx = xdim - (int)xcent ;
	topy = ydim - (int)ycent ;
	pixptr= buf ;

	for (y=(-(int)ycent);y<topy;y++)
		for (x=(-(int)xcent);x<topx;x++)
			if (*(pixptr++))
			{	nx = x*cosmaj + y*sinmaj ;
				ny = y*cosmaj - x*sinmaj ;
				ny2= ny*ny ;
				nx2= nx*nx ;

				/* symetry about min & maj axis */
				m30 += nx2*nx * PIXVAL ;
				m03 += ny2*ny * PIXVAL ;
				/* kurtosis about major & minor axis */
				m40 += nx2*nx2 * PIXVAL ;
				m04 += ny2*ny2 * PIXVAL ;

				/* symetries about origin */
				/*m12 += nx*ny2 * PIXVAL ;
				m21 += nx2*ny * PIXVAL ;
				m31 += nx2*nx*ny * PIXVAL ;
				m13 += ny2*ny*nx * PIXVAL ;
				m22 += nx2*ny2 * PIXVAL ;
				*/
			}

/* skewness & kurtosis in directions of principal axes */
/* div by area to make magnif invariant (castleman P328) (seems wrong)*/
/* div by appr stdev power to make indep of scale of data (snedecor p86)*/

	index = 1.5 ;
	mom[M03] =  100* ( m03/(pow(m02,index)*farea) );
	mom[M30] =  100* ( m30/(pow(m20,index)*farea) );
	mom[M04] =  100* ( m04/(m02*m02*farea)) ;
	mom[M40] =  100* ( m40/(m20*m20*farea)) ;
	mom[M03] = abs(mom[M03]) ;
	mom[M30] = abs(mom[M30]) ;
	highermomsdone = TRUE ;
	return(mom[code]) ;
}

/*---------------------------------------------------*/
/* roots of a quadratic egn.  */

void qroots(a,b,c,bigroot,smroot)
double a,b,c,*bigroot,*smroot ;
{
	double i;
	i = sqrt((double)(b*b - 4*a*c)) ;
	*bigroot= (-b+i)/2*a ;
	*smroot = (-b-i)/2*a ;
	if (*bigroot < *smroot)
	{	i = *bigroot ;
		*bigroot = *smroot ;
		*smroot = i ;
	}
}

