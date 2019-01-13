/*	Copyright (c) 1991 The Turing Institute 

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * h_nns - apply a nearest neighbour smoothing filter to an image
 *
 * usage:	h_nns(hdi,hdo,size,k)
 *
 * where "size" is the window size in which nns is computed
 * and "k" is the number of nearest neighbours to be averaged.
 * This filter works on byte images.
 *
 * to load:	cc -o h_nns h_nns.c -lhips
 *
 * Peter Mowforth and Jin Zhengping - 8/5/85
 * Rewritten by Jin Zhengping - 20 August 1991
 */

#include <hipl_format.h>

int h_nns(hdi,hdo,size,k)
struct header	*hdi,*hdo;
int		size,k;
{
	switch(hdi->pixel_format)
	{
	case PFBYTE:	return(h_nns_b(hdi,hdo,size,k));
	default:	return(perr(HE_FMTSUBR,"h_nns",
				hformatname(hdi->pixel_format)));
	}
}

int h_nns_b(hdi,hdo,size,k)
struct header	*hdi,*hdo;
int		size,k;
{
	return(h_nns_B(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
		    hdi->ocols,size,k));
}

#define	N	256

int h_nns_B(imagei,imageo,nr,nc,ocols,size,k)
byte	*imagei,*imageo;
int	nr,nc,ocols;
int	size,k;
{
	int		acck,incv,cenv,curv,ir,ic,sum;
	byte		*nnp;
	int		his[N];
	register int	i,j,ii,jj;

	int		plus = size/2;
	int		minus = plus-size+1;
	int		top = -minus;
	int		bot = nr - plus;
	int		left = -minus;
	int		right = nc - plus;
	int		nexi = ocols-nc;
	int		upleft = ocols*minus + minus;
	int		nexw = ocols-size;
	byte		*ip = imagei;
	byte		*op = imageo;
	char		*memsetl();

	for(i=0;i<nr;i++) 
	{
		for(j=0;j<nc;j++) 
		{
			memsetl((char *)his,0,N*sizeof(*his));
			if (i<top || i>=bot || j<left || j>=right) 
			{
				for(ii=minus;ii<=plus;ii++)
					for(jj=minus;jj<=plus;jj++) 
					{
					    ir = i + ii;
					    ic = j + jj;
					    ir = ir<0?0:(ir>=nr)?nr-1:ir;
					    ic = ic<0?0:(ic>=nc)?nc-1:ic;
					    ++his[imagei[ir*ocols+ic]];
					}
			}
			else {
				nnp = ip + upleft;
				for(ii=minus;ii<=plus;ii++) 
				{
					for(jj=minus;jj<=plus;jj++)
						++his[*nnp++];
					nnp += nexw;
				}
			}
			incv = 1;
			cenv = curv = *ip;
			acck = his[cenv];
			sum = cenv * acck;
			while (acck < k) 
			{
				curv = cenv + incv;
				incv = (incv > 0) ? -incv : (-incv + 1);
				if (curv >= N) curv = N-1;
				else if (curv < 0) curv = 0;
				sum += curv * his[curv];
				acck += his[curv];
			}
			sum -= curv * (acck - k);
			ip++;
			*op++ = sum / k;
		}
		ip += nexi;
		op += nexi;
	}
	return(HIPS_OK);
}
