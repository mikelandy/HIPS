
#define pix(b,x,y,xl)	*(b+(x+(y)*(xl)))
#define POSPIC	0
#define NEGPIC	1

int area_frame(bf,xdim,ydim)

unsigned char *bf;
int xdim,ydim;

{
	register unsigned char *bp,*picend ;
	register int  areacnt ;

	bp = bf ;
	picend = bf + xdim*ydim ;
	areacnt = 0 ;
	while ( bp < picend)
		if ( *bp++ )
			areacnt += 1 ;
	return(areacnt) ;
}
