
#define FALSE	0
#define TRUE	1
#define CON8	8
#define CON4	4
#define OBJVAL	190
#define pix(b,x,y,xl)	*(b+(x+(y)*(xl)))
#define UL(ptr)	(*(ptr-(xdplus1)))
#define UP(ptr)	(*(ptr-xdim))
#define UR(ptr)	(*(ptr-(xdless1)))
#define LT(ptr)	(*(ptr-1))
#define RT(ptr)	(*(ptr+1))
#define DL(ptr)	(*(ptr+(xdplus1)))
#define DW(ptr)	(*(ptr+xdim))
#define DR(ptr)	(*(ptr+(xdless1)))

#define CHECK(A)	{ if ( (!(*sp)) && (A) )\
				{ *dp++ = val ;  \
				area += 1 ;}  \
			else\
				*dp++ = *sp ; \
			sp++ ; } 
int frame_expand();

int expand(sor,dst,xdim,ydim,type)

unsigned char *sor,*dst;
int xdim,ydim,type;

{
	return(frame_expand(sor,dst,xdim,ydim,type,127)) ;
}

int frame_expand(sor,dst,xdim,ydim,type,val)

unsigned char *sor,*dst ;
int xdim,ydim,type,val;

{
	register unsigned char *sp,*dp ;
	register int x,y ;
	int xdless1,ydless1,xdplus1,area ;

	sp = sor ;
	dp = dst ;
	xdless1 = xdim -1 ;
	xdplus1 = xdim +1 ;
	ydless1 = ydim -1 ;

	area = 0 ;
	if (type == CON8)
	{	/* left corner */
		CHECK(RT(sp)||DR(sp)||DW(sp))

		/* top row */
		for (x=1; x<xdless1;x++)
			CHECK(LT(sp)||RT(sp)||DL(sp)||DW(sp)||DR(sp))

		/* right corner */
		CHECK(LT(sp)||DL(sp)||DW(sp))

		for (y=1;y<ydless1;y++)
		{	/* left edge */
			CHECK(UP(sp)||UR(sp)||RT(sp)||DR(sp)||DW(sp))

			/* main part of pic */
			for (x=1;x<xdless1;x++)
				CHECK(UP(sp)||UR(sp)||RT(sp)||DR(sp)||DW(sp)||DL(sp)||LT(sp)||UL(sp))

			/* right edge */
			CHECK(UP(sp)||UL(sp)||LT(sp)||DL(sp)||DW(sp))
		}
		/* bot left corner */
		CHECK(RT(sp)||UR(sp)||UP(sp))

		/* bottom row */
		for (x=1; x<xdless1;x++)
			CHECK(LT(sp)||RT(sp)||UL(sp)||UP(sp)||UR(sp))

		/* bot right corner */
		CHECK(LT(sp)||UL(sp)||UP(sp))
	}
	else
	{	/* left corner */
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
				CHECK(UP(sp)||RT(sp)||DW(sp)||LT(sp))

			/* right edge */
			CHECK(UP(sp)||LT(sp)||DW(sp))
		}
		/* bot left corner */
		CHECK(RT(sp)||UP(sp))

		/* bottom row */
		for (x=1; x<xdless1;x++)
			CHECK(LT(sp)||RT(sp)||UP(sp))

		/* bot right corner */
		CHECK(LT(sp)||UP(sp))
	}
	return(area) ;
}

