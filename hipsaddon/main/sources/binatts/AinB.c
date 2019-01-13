
#define TRUE 1
#define FALSE 0
#define pix(b,x,y,xl)	*(b+(x+(y)*(xl)))
#define POSPIC	0
#define NEGPIC	1

int AinB(Abf,Bbf,xdim,ydim)

unsigned char *Abf,*Bbf;
int xdim,ydim;

{
	register unsigned char *Abp,*Bbp,*picend ;
	register int  cnt ;

	Abp = Abf ;
	Bbp = Bbf ;
	picend = Abf + xdim*ydim ;
	while ( Abp < picend)
	{	if ( *Abp && !(*Bbp) )
			return(FALSE) ;
		Abp++ ;
		Bbp++ ;
	}
	return(TRUE) ;
}
