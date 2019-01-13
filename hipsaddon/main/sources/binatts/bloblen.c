#include 		<math.h>

#define pix(bf,x,y,dim) (*((unsigned char *) (bf+((int)(x))+((int)(y))*(dim))))
#define PY		((int) *fy)
#define PX		((int) *fx)
#define pi		3.141597265
#define INPIC(x,y)	((x >= 0) && (y >= 0) && (x < xdim) && (y < ydim))
#define POSPIC		0

int abs();
void set_frame(),merge();

void next_pix(x,y,incx,incy)
float *x, *y, incx, incy ;
{
	*x += incx ;
	*y += incy ;
}


void get_inc(incx,incy,rad)
float *incx, *incy, rad ;
{
	float degr, degr0_360 ;

	degr0_360 = (rad*180)/pi ;
	while (degr0_360 < 0)
		degr0_360 += 360 ;
	while (degr0_360 >= 360)
		degr0_360 -= 360 ;

	if (degr0_360 >= 180)
		degr = degr0_360 - 180 ;
	else
		degr = degr0_360 ;

	if ((degr > 45) && (degr < 135)) {
		*incx = (float)tan( ((double)fabs(90 - degr)*pi)/180.0 ) ; 
		if ( degr0_360 > 180) {
			*incy = -1;
			if (degr0_360 < 270)
				*incx = -(*incx) ;
		}
		else { 
			*incy = 1 ;
			if (degr0_360 > 90)
				*incx = -(*incx) ;
		}
	}
	else {
		/* if deg is in 135->180 or 315-360 regions then
		   the incy calculated in next line would be negative
		   without the fabs() */
		*incy = (float)fabs( (double)tan( ((double)degr*pi)/180.0) ) ;

		if ((degr0_360 >= 135) && (degr0_360 <= 225)) {
			*incx = -1 ;
			if (degr0_360 > 180)
				*incy = - (*incy) ;
		}
		else {
			*incx = 1 ;
			if (degr0_360 >= 315)
				*incy = - (*incy) ;
		}
	}
}

void notsooldget_inc(incx,incy,rad)
float *incx, *incy, rad ;
{
	int degr, degr0_360 ;

	degr0_360 = (int)((rad/pi)*180) % 360 ;
	if (degr0_360 < 0)
		degr0_360 += 360 ;
	degr = degr0_360  % 180 ;

	if ((degr > 45) && (degr < 135)) {
		*incx = tan((double)(((float) abs(90 - degr))/180.0)*pi) ; 
		if ( degr0_360 > 180) {
			*incy = -1;
			if (degr0_360 < 270)
				*incx = -(*incx) ;
		}
		else { 
			*incy = 1 ;
			if (degr0_360 > 90)
				*incx = -(*incx) ;
		}
	}
	else {
		/* if deg is in 135->180 or 315-360 regions then
		   the incy calculated in next line would be negative
		   without the fabs() */
		*incy = fabs(tan((double)(((float)degr)/180.0)*pi)) ;

		if ((degr0_360 >= 135) && (degr0_360 <= 225)) {
			*incx = -1 ;
			if (degr0_360 > 180)
				*incy = - (*incy) ;
		}
		else {
			*incx = 1 ;
			if (degr0_360 >= 315)
				*incy = - (*incy) ;
		}
	}
}

/* old get_inc */
/*get_inc(incx,incy,rad)
float *incx, *incy, rad ;
{
	int degr = ((int)((rad / pi ) * 180)) % 180 ;

	if ((degr > 45) && (degr < 135)) {
		*incy = 1 ;
		*incx = tan((double)(((float) abs(90 - degr))/180.0)*pi) ; 
		if (degr > 90)
			*incx = - (*incx) ;
	}
	else {
		*incx = 1 ;
		*incy = tan((double)(((float)degr)/180.0)*pi) ;
	}
}*/

/*find_pic(x,y)
int *x,*y ;
{
	*x = 48 ;
	*y = 48 ;
}*/

void find_end(bp,tanpic,xdim,ydim,lx,ly,xdir,ydir,flag)

unsigned char *bp,*tanpic ;
int xdim,ydim,flag;
float *lx, *ly, xdir, ydir ;

{
	float fx, fy ;

	fx = *lx ; fy = *ly ;

	while (INPIC(fx,fy))  
		if (pix(bp,fx,fy,xdim)) {
			next_pix(&fx,&fy,xdir,ydir) ;
			*lx = fx ;
			*ly = fy ;
		}
		else 
			next_pix(&fx,&fy,(ydir*flag),(xdir*(-flag))) ;

	fx = *lx ; fy = *ly ;

	while (INPIC(fx,fy)) {
		if (pix(bp,fx,fy,xdim)) {
			find_end(bp,tanpic,xdim,ydim,&fx,&fy,xdir,ydir,-flag) ;
			*lx = fx ; *ly = fy ;
			return ;
		}
		next_pix(&fx,&fy,(ydir*(-flag)),(xdir*flag)) ;
	}

/* add a 'caliper' tangent to tanpic.
 * generating this produces a kind of bounding polygon, which is used
 * for another attribute (hollows area) */

	fx = *lx ; fy = *ly ;

	while (INPIC(fx,fy))
	{	pix(tanpic,fx,fy,xdim) = 64 ;
		next_pix(&fx,&fy,(ydir*flag),(xdir*(-flag))) ;
	}	

	fx = *lx ; fy = *ly ;

	while (INPIC(fx,fy))
	{	pix(tanpic,fx,fy,xdim) = 64 ;
		next_pix(&fx,&fy,(ydir*(-flag)),(xdir*flag)) ;
	}	

}

/* pic = pic for length calc; tp = pic to put the 2 tangents (calipers) in */

float length(pic,tp,xdim,ydim,xst,yst,rad) 

unsigned char *pic,*tp;
int xdim,ydim,xst,yst;
float rad;

{
	float incx, incy ;
	float x1,y1,x2,y2,x3,y3 ;
	float len ;

	get_inc(&incx,&incy,rad);
	/*find_blob(pic,xdim,ydim,&intx,&inty);*/
	x1 = x3 = xst ;
	y1 = y3 = yst ;

	set_frame(tp,xdim,ydim,0) ;
	find_end(pic,tp,xdim,ydim,&x1,&y1,incx,incy,1);
	find_end(pic,tp,xdim,ydim,&x3,&y3,-incx,-incy,1);

	x2 = x1 + (100 * incy) ;
	y2 = y1 - (100 * incx) ;
	
	len = ( x1*(y2-y3) + x2*(y3-y1) + x3*(y1-y2) ) /
			hypot((double) x2-x1 , (double) y2-y1 ) ;
	if ( len < 0 ) len = (-len) ;
	return(len) ;
}

/* pic = picture for length calc; poly=pict to put bounding poly in */

float maxlen(pic,poly,wrk,xdim,ydim,xst,yst,mrad)

unsigned char *pic,*poly,*wrk;
int xdim,ydim,xst,yst;
float *mrad;

{
	register int i,deginc ;
	float len,maxlen ;
	float rad,radinc ;

	deginc = 1 ;		/* was 5 in old binatts */
	radinc = ((float)deginc*pi)/180.0 ;
	rad = maxlen = 0.0 ;
	set_frame(poly,xdim,ydim,0) ;

	for (i=0;i<180;i=i+deginc)
	{	len =length(pic,wrk,xdim,ydim,xst,yst,rad);
		if (len > maxlen)
		{	maxlen = len ;
			*mrad = rad ;
		}
		rad += radinc ;
		merge(wrk,poly,xdim,ydim) ;
	}
	return(maxlen) ;
}
