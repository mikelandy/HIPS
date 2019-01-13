/*	Copyright (c) 1991 Ulrik P.V. Skands
 *
 * Subroutine to distmin.c.
 *
 * SEE ALSO:	distmin.c
 *		h_chamfer.c
 *		h_sedt.c
 *
 * h_hexagon.c - calculates global distances in binary images on the basis
 *	       of hexagon grid.
 */

#include <stdio.h>
#include <hipl_format.h>

#ifndef HNOVALUESH
#include <values.h>
#else
#define MAXINT  (0x7fffffff)
#endif

#define MINIMUM(A,B) (((A)<(B))?(A):(B))


int h_hexagon(hdi,hdo)
struct header *hdi,*hdo;

{
register	int		r,c,i,j;
register	unsigned int	v1,v2,v3,tmp1,tmp2,npix,m;
register	unsigned char	*ifr, *ip;
register	unsigned int	*ofr, *op;

ifr=(unsigned char *)hdi->firstpix;
ofr=(unsigned int *)hdo->firstpix;
r=hdi->orows;
c=hdi->ocols;
npix=r*c;

	for (op=ofr, ip=ifr, m=0;m<npix;m++, ip++, op++){
	  if(*ip)
  	    *op=MAXINT;	
	  else
	    *op=0;
	  }

	/* Forward pass */
	for (op=ofr, i=0;i<r;i++) 
	{
	  if((i%2)==0)
	    {
		for (j=0;j<c;j++, op++) 
		if(*op)
		{	
			if((i==0)||(j==0))
				v1=MAXINT;
			else	v1=op[-c-1]+1;
			if(i==0)
				v2=MAXINT;
			else	v2=op[-c]+1;
			if(j==0)
				v3=MAXINT;
			else	v3=op[-1]+1;
			tmp1=MINIMUM(v1,v2);
			tmp2=MINIMUM(v3,*op);
			*op=MINIMUM(tmp1,tmp2);
		}
	    }
	    else
	    {
		for (j=0;j<c;j++,op++) 
		if(*op)
		{	
			if(i==0)
				v1=MAXINT;
			else	v1=op[-c]+1;
			if((i==0)||(j==(c-1)))
				v2=MAXINT;
			else	v2=op[-c+1]+1;
			if(j==0)
				v3=MAXINT;
			else	v3=op[-1]+1;
			tmp1=MINIMUM(v1,v2);
			tmp2=MINIMUM(*op,v3);
			*op=MINIMUM(tmp1,tmp2);
		}
	    }
	}
	op--;
	/* Backward pass 	*/
	for (i=(r-1);i>=0;i--) 
	{
	  if((i%2)==0)
	  {
		for (j=(c-1);j>=0;j--, op--) 
		if(*op)	
		{	
			if(i==(r-1))
				v1=MAXINT;
			else	v1=op[c]+1;
			if((i==(r-1))||(j==0))
				v2=MAXINT;
			else	v2=op[c-1]+1;
			if(j==(c-1))
				v3=MAXINT;
			else	v3=op[1]+1;
			tmp1=MINIMUM(v1,v2);
			tmp2=MINIMUM(*op,v3);
			*op=MINIMUM(tmp1,tmp2);
		}
	  }
	  else
	  {
		for (j=(c-1);j>=0;j--, op--) 
		if(*op)	
		{	
			if(i==(r-1))
				v1=MAXINT;
			else	v1=op[c]+1;
			if((i==(r-1))||(j==(c-1)))
				v2=MAXINT;
			else	v2=op[c+1]+1;
			if(j==(c-1))
				v3=MAXINT;
			else	v3=op[1]+1;
			tmp1=MINIMUM(v1,v2);
			tmp2=MINIMUM(*op,v3);
			*op=MINIMUM(tmp1,tmp2);
		}
	   }
	} 
	return(HIPS_OK);
}
