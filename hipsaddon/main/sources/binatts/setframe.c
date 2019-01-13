#define pix(b,x,y,xl)	*(b+(x+(y)*(xl)))
#define POSPIC	0
#define NEGPIC	1

void set_frame(bf,xdim,ydim,val)

unsigned char *bf;
int xdim,ydim,val;

{
	register unsigned char *bp,*picend ;

	bp = bf ;
	picend = bf + xdim*ydim ;
	while ( bp < picend)
		*bp++ = val  ;
}
/*
setplane(bf,plabit,xdim,ydim)
unsigned char *bf ;
{
	register unsigned char *bp,*picend ;

	bp = bf ;
	picend = bf + xdim*ydim ;
	while ( bp++ < picend)
		*bp |= plabit  ;
}

clrplane(bf,plabit,xdim,ydim)
unsigned char *bf ;
{
	register unsigned char *bp,*picend ;

	bp = bf ;
	picend = bf + xdim*ydim ;
	plabit = ~plabit ;
	while ( bp++ < picend)
		*bp &= plabit ;
}
*/
