/* Expand all blobs in a frame until they reach a single closed border.
 * The closed border is specified in a separte input frame: perim.
 * The source frame (containing the blobs) is modified rather than
 * creating a new destination frame.
 * 
 * B.A.Shepherd, 1984.
 *
 * Note: This algorithm is not very efficient since it is essentially
 * a parallel algorithm implemented serially.
 * However, before rewriting, proof that this function is useful is needed.
 * If the source frame contains only one blob then set_blob,
 * which is much quicker, can be used.
 *
 */

#define OBJVAL	100
#define INPIC(x,y,xd,yd)	( (x>0) && (y>0) && (x<xd) && (y<yd) )
#define pix(b,x,y,xl)	*(b+(x+(y)*(xl)))
#define UL(ptr)	(*(ptr-(xdplus1)))
#define UP(ptr)	(*(ptr-xdim))
#define UR(ptr)	(*(ptr-(xdless1)))
#define LT(ptr)	(*(ptr-1))
#define RT(ptr)	(*(ptr+1))
#define DL(ptr)	(*(ptr+(xdplus1)))
#define DW(ptr)	(*(ptr+xdim))
#define DR(ptr)	(*(ptr+(xdless1)))
#define	CHECK(A)	{if ( (!(*sp)) && (A) && (!(*fp)) )\
			{	*sp = OBJVAL ;  \
				changed += 1 ; } \
			sp++ ; \
			fp++ ; }

/* NOTE: Perim is a picture containing the OUTER perimeter of */
/* the shape, the dst picture is expanded until this perimeter is */
/* reached but this perimeter does not form part of the filled shape */

int fill(perim,dst,xdim,ydim)

unsigned char *dst,*perim;
int xdim,ydim;

{
	register unsigned char *sp,*fp ;
	register int x,y ;
	int xdless1,ydless1,xdplus1,changed,area ;

	xdless1 = xdim -1 ;
	xdplus1 = xdim +1 ;
	ydless1 = ydim -1 ;
	area = 0 ;

	do
	{
		fp = perim ;
		sp = dst ;
		changed = 0 ;
		/* left corner */
		CHECK(RT(sp)||DW(sp))

		/* top row */
		for (x=1; x<xdless1;x++)
			CHECK(LT(sp)||RT(sp)||DW(sp))

		/* right corner */
		CHECK(LT(sp)||DW(sp))

		for (y=1;y<ydless1;y++)
		{	/* left edge */
			CHECK(UP(sp)||RT(sp)||DW(sp))

			/* main part of pic */
			for (x=1;x<xdless1;x++)
				CHECK(LT(sp)||UP(sp)||RT(sp)||DW(sp))

			/* right edge */
			CHECK(LT(sp)||UP(sp)||DW(sp))
		}
		/* bot left corner */
		CHECK(RT(sp)||UP(sp))

		/* bottom row */
		for (x=1; x<xdless1;x++)
			CHECK(LT(sp)||UP(sp)||RT(sp))

		/* bot right corner */
		CHECK(LT(sp)||UP(sp))

		area += changed ;
	}
	while (changed) ;
	return(area) ;
}

