static char *SccsId = "%W%      %G%";

/*      Copyright (c) 1991 The Turing Institute

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * h_mls - apply a maximum-likelihood-smoothing filter to each image
 *       of the input sequence
 *
 * usage:	h_mls(hdi,hdo,perc,size,ssize)
 *
 * where "perc" is the percentage (x 100) of the nearest neighbours
 * in the original window which have the closest grey levels to that
 * of the central pixel. "size" is the
 * width of the window in which h_mls filtering is performed. 
 * "Ssize" is the width of the windows which make
 * up the original window, to find the window with the greatest
 * concentration of the nearest neighbours.
 *
 * to load:	cc -o h_mls h_mls.c -lhips
 *
 * Peter Mowforth & Jin Zhengping - 8/5/85
 * Rewritten by Jin Zhengping - 31 August 1991
 *
 */

#include <hipl_format.h>

int h_mls(hdi,hdo,k,size,ssize)
struct header	*hdi,*hdo;
int		k,size,ssize;
{
	switch(hdi->pixel_format)
	{
	case PFBYTE:	return(h_mls_b(hdi,hdo,k,size,ssize));
	default:	return(perr(HE_FMTSUBR,"h_mls",
				hformatname(hdi->pixel_format)));
	}
}

int h_mls_b(hdi,hdo,k,size,ssize)
struct header	*hdi,*hdo;
int		k,size,ssize;
{
	return(h_mls_B(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
		    hdi->ocols,k,size,ssize));
}

#define  N 256

int h_mls_B(imagei,imageo,nr,nc,ocols,k,size,ssize)
byte	*imagei,*imageo;
int	nr,nc,ocols;
int	k,size,ssize;
{

	int	i,j,l,i2,j2,k2,l2,i3,j3,l3,ir,ic,sum,sizet;
	int	low,up,work,max,count;	
	byte	*nnp;
	int	his[N],*win,*winl;

	int	ssq = ssize*ssize;
	int	ss1 = ssize-1;
	int	plus = size/2;
	int	minus = plus-size+1;
	int	top = -minus;
	int	bot = nr-plus;
	int	left = -minus;
	int	right = nc-plus;
	int	nexi = ocols-nc;
	int	upleft = minus*ocols+minus;
	int	nexw = ocols-size;
	byte	*ip = imagei;
	byte	*op = imageo;
	void	*malloc();
	char	*memsetl();

	if(((win=(int *)malloc((unsigned)size*size*sizeof(*win)))==0) ||
	   ((winl=(int *)malloc((unsigned)size*size*sizeof(*winl)))==0))
		perr(HE_ALLOC);
	k = k*size*size/100;

	for(i=0;i<nr;i++) 
	{
		for(j=0;j<nc;j++) 
		{
			memsetl((char *)his, (int)0, N*sizeof(*his));
			if(i<top || i>=bot || j<left || j>=right) 
			{
				for(i3=0,i2=minus;i2<=plus;i2++,i3++)
					for(j3=0,j2=minus;j2<=plus;j2++,j3++) 
					{
					    ir = i + i2;
					    ic = j + j2;
					    ir = ir<0?0:(ir>=nr)?nr-1:ir;
					    ic = ic<0?0:(ic>=nc)?nc-1:ic;
					    work = imagei[ir*ocols+ic];
					    ++his[work];
					    win[i3*size+j3] = work;
					    winl[i3*size+j3] = 0;
					}
			} else
			{
				nnp = ip + upleft;
				for(i3=0,i2=minus;i2<=plus;i2++,i3++) 
				{
					for(j3=0,j2=minus;j2<=plus;j2++,j3++)
					{
						work = *nnp++;
						++his[work];
						win[i3*size+j3] = work;
						winl[i3*size+j3] = 0;
					}
					nnp += nexw;
				}
			}

/* find boundaries of grey levels of nearest neighbours */

			l = 0;
			l2 = l3 = *ip;
			k2 = his[l2] - 1;
			while (k2 < k) 
			{
				l = (l > 0) ? -l : (-l + 1);
				l3 = l2 + l;
				if(l3 > 255) l3 = 255;
				else if(l3 < 0) l3 = 0;
				k2 += his[l3];
			}
			if(l > 0)
			{
				low = l2 - l + 1;
				up = l2 + l;
			} else
			{
				low = l2 + l;
				up = l2 - l;
			}

/* find maximum number of nearest neighbours that a small wimdow contains */

			for(i3=0; i3<size; i3++)
				for(j3=0; j3<size; j3++)
					if((win[i3*size+j3] <= up) &&
					   (win[i3*size+j3] >= low))
						winl[i3*size+j3] = 1;

			max = 0;
			for(i2=ss1; i2<size; i2++)
				for(j2=ss1; j2<size; j2++)
				{
					count = 0;
					for(i3=i2-ss1; i3<=i2; i3++)
						for(j3=j2-ss1; j3<=j2; j3++)
							if(winl[i3*size+j3])
								count++;
					if(count>max)
						max = count;
				}

/* average the central pixel */

			sum = sizet = 0;
			for(i2=ss1; i2<size; i2++)
				for(j2=ss1; j2<size; j2++)
				{
					count = 0;
					for(i3=i2-ss1; i3<=i2; i3++)
						for(j3=j2-ss1; j3<=j2; j3++)
							if(winl[i3*size+j3])
								count++;
					if(count==max) 
					{
						for(i3=i2-ss1; i3<=i2; i3++)
							for(j3=j2-ss1; j3<=j2; j3++)
								sum += win[i3*size+j3];
						sizet += ssq;
					}
				}

			ip++;
			*op++ = sum/sizet;
		}
		ip+=nexi;
		op+=nexi;
	}
	free((char *)win);
	free((char *)winl);
	return(HIPS_OK); ;
}
