
#define FALSE	0
#define TRUE	1
#define CON8	8
#define CON4	4
#define pix(b,x,y,xl)	*(b+(x+(y)*(xl)))
#define UL(ptr)	(*(ptr-(xdim+1)))
#define UP(ptr)	(*(ptr-xdim))
#define UR(ptr)	(*(ptr-(xdim-1)))
#define LT(ptr)	(*(ptr-1))
#define RT(ptr)	(*(ptr+1))
#define DL(ptr)	(*(ptr+(xdim+1)))
#define DW(ptr)	(*(ptr+xdim))
#define DR(ptr)	(*(ptr+(xdim-1)))


#ifdef OLDSHRINK
void oshrink(sbuf,dbuf,cnt,xdim,ydim)
unsigned char *sbuf,*dbuf ;
int *cnt,xdim,ydim ;
{
	register unsigned char *sorbuf,*dstbuf ;
	register int x,y, ;
	int area=0 ;

	sorbuf = sbuf ;
	dstbuf = dbuf ;
	for (y=0;y<ydim;y++)
		for (x=0;x<xdim;x++)
			if ( pix(sorbuf,x,y,xdim) ) {	
				if (pix(sorbuf,x,y-1,xdim) &&
					pix(sorbuf,x-1,y,xdim) &&
						pix(sorbuf,x+1,y,xdim)&&
						 pix(sorbuf,x,y+1,xdim))
				{	pix(dstbuf,x,y,xdim) = pix(sorbuf,x,y,xdim) ;	
					area += 1 ;
				}
				else
					pix(dstbuf,x,y,xdim) = 0 ;

			}
			else
				pix(dstbuf,x,y,xdim) = 0 ;
	if ( (*cnt)-- && area )
	{
		oshrink(dstbuf,sorbuf,cnt,xdim,ydim) ;
	}
}	
#endif 

int shrink(sbuf,dbuf,xdim,ydim,type)

unsigned char *sbuf,*dbuf;
int xdim,ydim,type;

{
	register unsigned char *sbp,*dbp,*bufend ;
	register int changed,i,j,up,down,left,right,xdminus,ydminus ;

	sbp = sbuf ;
	dbp = dbuf ;
	bufend = sbuf + xdim*ydim ;
	changed = 0 ;
	i = j = 0 ;
	xdminus = xdim -1 ;
	ydminus = ydim -1 ;

	if (type == CON4) {
		while ( sbp < bufend) {

			if ( *sbp ) {
				/************************************/
				/* handle edge of image */
				if (!j) {
					up = 1 ;
					down = DW(sbp) ;
				}
				else {
					up = UP(sbp) ;
					if (j >= ydminus)
						down = 1 ;
					else {
						down = DW(sbp) ;
					}
				}
				if (!i) {
					left =  1;
					right = RT(sbp) ;
					i++ ;
				}
				else {
					left = LT(sbp) ;
					if (i >= xdminus) {
						right = 1 ;
						i = 0 ;
						j++ ;
					}
					else {
						right = RT(sbp) ;
						i++ ;
					}
				}
				/************************************/

				/* if (UP(sbp) && DW(sbp)
						&& LT(sbp) && RT(sbp)) */

				if ( up && down && left && right )
					*dbp++ = *sbp ;	
				else {
					*dbp++ = 0 ;
					changed += 1 ;
				}
			}
			else {
				*dbp++ = 0 ;
				if (i >= xdminus) {
					i = 0 ;
					j++ ;
				}
				else
					i++ ;
			}
			sbp++ ;
		}
	}
	else {
		while ( sbp < bufend) {
			if ( *sbp ) {	
				if (UP(sbp) && DW(sbp) && LT(sbp) &&
					RT(sbp) && UL(sbp) && UR(sbp)
						&& DW(sbp) && DR(sbp))
					*dbp++ = *sbp ;	
				else {
					*dbp++ = 0 ;
					changed += 1 ;
				}
			}
			else
				*dbp++ = 0 ;
			sbp++ ;
		}
	}
	return(changed) ;
}

void remsingpix(sbuf,dbuf,xdim,ydim)

unsigned char *sbuf,*dbuf;
int xdim,ydim;

{
	register unsigned char *sbp,*dbp ;
	register int x,y ;

	sbp = sbuf ;
	dbp = dbuf ;

	for (x=1;x<=xdim;x++)
	{	*dbp++ = 0 ;
		sbp++ ;
	}
	for (y=2;y<ydim;y++)
	{	for (x=1;x<=xdim;x++)
		{	if ( (x==1) || (x==xdim) || !(*sbp) )
				*dbp++ = 0 ;
			else	
			{	if (UP(sbp) || DW(sbp) || LT(sbp) || RT(sbp) || UL(sbp) || UR(sbp) || DW(sbp) || DR(sbp))
					*dbp++ = *sbp ;	
				else
					*dbp++ = 0 ;
			}
			sbp++ ;
		}
	}
	for (x=1;x<=xdim;x++)
		*dbp++ = 0 ;
}
