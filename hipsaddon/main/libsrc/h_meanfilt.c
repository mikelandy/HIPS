static char *SccsId = "%W%      %G%";

/*      Copyright (c) 1991 The Turing Institute

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * h_meanfilt - apply a mean filter to each image of the input sequence 
 *
 * usage:	h_meanfilt(hdi,hdo,size)
 *
 * where "size" is the width of the window in which the mean averaging
 * is operated.
 *
 * to load:	cc -o h_meanfilt h_meanfilt.c -lhips
 *
 * Peter Mowforth & Jin Zhengping - 8/5/85
 * Rewritten by Jin Zhengping - 31 August 1991
 *
 */

#include <hipl_format.h>

int h_meanfilt(hdi,hdo,size)
struct header	*hdi,*hdo;
int		size;
{
	switch(hdi->pixel_format)
	{
	case PFBYTE:	return(h_meanfilt_b(hdi,hdo,size));
	default:	return(perr(HE_FMTSUBR,"h_meanfilt",
				hformatname(hdi->pixel_format)));
	}
}

int h_meanfilt_b(hdi,hdo,size)
struct header	*hdi,*hdo;
int		size;
{
	return(h_meanfilt_B(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
		    hdi->ocols,size));
}

int h_meanfilt_B(imagei,imageo,nr,nc,ocols,size)
byte	*imagei,*imageo;
int	nr,nc,ocols;
int	size;
{
	int		sum,ir,ic;
	byte		*nnp;
	register int	i,j,ii,jj;

	int	sizesq = size*size;
	int	plus = size/2;
	int	minus = plus-size+1;

	int	top = -minus;
	int	bot = nr-plus;
	int	left = -minus;
	int	right = nc-plus;
	byte	*ip = imagei;
	byte	*op = imageo;
	int	nexi = ocols-nc;
	int	upleft = minus*ocols + minus;
	int	nexw = ocols - size;

	for (i=0;i<nr;i++)
	{
		for (j=0;j<nc;j++)
		{
			sum = 0;
			if(i<top || i>=bot || j<left || j>=right)
			{
				for (ii=minus;ii<=plus;ii++)
					for (jj=minus;jj<=plus;jj++) {
					    ir = i + ii;
					    ic = j + jj;
					    ir = ir<0?0:(ir>=nr)?nr-1:ir;
					    ic = ic<0?0:(ic>=nc)?nc-1:ic;
					    sum += imagei[ir*ocols+ic];
					}
			} else
			{
				nnp = ip + upleft;
				for (ii=minus;ii<=plus;ii++) {
					for (jj=minus;jj<=plus;jj++)
						sum += *nnp++;
					nnp += nexw;
				}
			}
			ip++;
			*op++ = sum/sizesq;
		}
		ip+=nexi;
		op+=nexi;
	}
	return(HIPS_OK); ;
}
