/*	Copyright (c) 1991 Ulrik P.V. Skands
 *
 * Subroutine to distmin.c
 *
 * SEE ALSO:	distmin.c
 *		h_hexagon.c
 *		h_sedt.c
 *
 * h_chamfer.c - calculates global distances in binary images on the basis
 *	       of city block/chessboard grid. Horizontal/vertical and 
 *	       diagonal unit distances can be given explicit or set default
 *	       to 1 and 1.3507.
 */


#include <stdio.h>
#include <hipl_format.h>

#ifndef HNOVALUESH
#include <values.h>
#else
#define MAXINT	(0x7fffffff)
#endif

#define MINIMUM(A,B) (((A)<(B))?(A):(B))

int h_chamfer(hdi,hdo,d1,d2)

struct header	*hdi,*hdo;
double		d1,d2;

{
register	int		r,c,i,j;
register	unsigned int	npix, m;
register	float		v1,v2,v3,v4;
register	float		tmp1,tmp2,tmp3;
register	unsigned char	*ifr, *ip;
register	float		*ofr, *op;

ifr=(unsigned char *)hdi->firstpix;
ofr=(float *)hdo->firstpix;
r = hdi->orows; 
c = hdi->ocols; 
npix=r*c;

	for (ip=ifr, op=ofr, m=0;m<npix;m++, ip++, op++){
	  if(*ip)
	    *op=MAXINT;
	  else
	    *op=0;
	  }

	/* Forward pass */
	for (op=ofr, i=0;i<r;i++)
		for (j=0;j<c;j++, op++) 
		if(*op)
		{	
			if((i==0)||(j==0)) 
				v1=MAXINT;
			else	v1=op[-c-1]+d2;
			if(i==0)
				v2=MAXINT;
			else	v2=op[-c]+d1;
			if((i==0)||(j==(c-1)))	
				v3=MAXINT;
			else	v3=op[-c+1]+d2;
			if((i==(r-1))||(j==0)) 
				v4=MAXINT;
			else	v4=op[-1]+d1;

			tmp1=MINIMUM(v1,v2);
			tmp2=MINIMUM(v3,v4);	
			tmp3=MINIMUM(tmp1,tmp2);
			*op=MINIMUM(*op,tmp3);
		}

	op--;
	/* Backward pass 	*/
	for (i=(r-1);i>=0;i--) 
		for (j=(c-1);j>=0;j--, op--) 
		if(*op)	
		{	
			if((i==(r-1))||(j==(c-1)))
				v1=MAXINT;
			else	v1=op[c+1]+d2;
			if((i==0)||(j==(c-1)))
				v2=MAXINT;
			else	v2=op[1]+d1;
			if((i==(r-1))||(j==0))
				v3=MAXINT;
			else	v3=op[c-1]+d2;
			if(i==(r-1))
				v4=MAXINT;
			else	v4=op[c]+d1;

			tmp1=MINIMUM(v1,v2);
			tmp2=MINIMUM(v3,v4);
			tmp3=MINIMUM(tmp1,tmp2);
			*op=MINIMUM(*op,tmp3);
		} 
	return(HIPS_OK);
}
