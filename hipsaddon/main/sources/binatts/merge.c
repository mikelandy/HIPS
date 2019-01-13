

#define pix(b,x,y,xl)	*(b+(x+(y)*(xl)))
#define POSPIC	0
#define NEGPIC	1

void merge(sor,dest,xdim,ydim)

unsigned char *sor,*dest ;
int xdim,ydim;

{
	register unsigned char *sp,*dp,*picend ;

	sp = sor ;
	dp = dest ;
	picend = sor + xdim*ydim ;
	while ( sp < picend)
	{	if (*sp)
			*dp++ = *sp ;
		else
			dp++;
		sp++ ;
	}
}
