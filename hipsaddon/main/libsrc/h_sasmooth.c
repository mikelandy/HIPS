static char *SccsId = "%W%      %G%";

/*      Copyright (c) 1991 The Turing Institute

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * h_sasmooth - apply a selective-averaging-smoothing filter to each image of
 *              the input sequence
 *
 * usage:	h_sasmooth(hdi,hdo,size,t)
 *
 * where "size" is the size of the window in which sas is computed and
 * defaults to 3. threshold is a threshold and defaults to 2.
 *
 * to load:	cc -o h_sasmooth h_sasmooth.c -lhips
 *
 * Peter Mowforth and Jin Zhengping - 8/5/85
 * Rewritten by Jin Zhengping - 20 August 1991
 */

#include <hipl_format.h>

int h_sasmooth(hdi,hdo,size,t)
struct header	*hdi,*hdo;
int		size,t;
{
	switch(hdi->pixel_format)
	{
	case PFBYTE:	return(h_sasmooth_b(hdi,hdo,size,t));
	default:	return(perr(HE_FMTSUBR,"h_sasmooth",
				hformatname(hdi->pixel_format)));
	}
}

int h_sasmooth_b(hdi,hdo,size,t)
struct header	*hdi,*hdo;
int		size,t;
{
	return(h_sasmooth_B(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
		    hdi->ocols,size,t));
}

int h_sasmooth_B(imagei,imageo,nr,nc,ocols,size,t)
byte	*imagei,*imageo;
int	nr,nc,ocols;
int	size,t;
{
	register int	i,j,ii,jj;
	int	ir,ic,sum;
	int	min,max,d[4],iimin,step,work;
	int	*nb,*nbp,*ne,*nep,*nepp,*nw,*nwp,*nwpp;

	int	sizesq = size * size;
	int	sizecir = 4 * (size - 1);
	int	st = 3 * (size - 1);
	int	plus = size/2;
	int	minus = plus-size+1;
	int	top = -minus;
	int	bot = nr - plus;
	int	left = -minus;
	int	right = nc - plus;
	int	nexi = ocols-nc;
	int	upleft = ocols*minus + minus;
	int	nexw = ocols-size;
	byte	*nnp;
	byte	*ip = imagei;
	byte	*op = imageo;
	void	*malloc();


	if((nb = (int *) malloc((unsigned)sizesq*sizeof (*nb))) == 0 ||
	   (ne = (int *) malloc((unsigned)sizecir*sizeof (*ne))) == 0 ||
	   (nw = (int *) malloc((unsigned)size*sizeof (*nw))) == 0)
		perr(HE_ALLOC);

	for(nwp=nw; nwp<nw+size; nwp++)
		*nwp = 1;
	*(nw+1) = 2;
	for(nwpp=nw+3; nwpp< nw+size; nwpp++)
	{
		nwp = nwpp;
		while(--nwp>nw)
			*nwp += *(nwp - 1);
	}
	for(ii=1; ii<size; ii++)
		t*=2;

	for(i=0;i<nr;i++) 
	{
		for(j=0;j<nc;j++) 
		{
			if(i<top || i>=bot || j<left || j>=right) 
			{
				nbp = nb;
				for(ii=minus;ii<=plus;ii++)
					for(jj=minus;jj<=plus;jj++) 
					{
					    ir = i + ii;
					    ic = j + jj;
					    ir = ir<0?0:(ir>=nr)?nr-1:ir;
					    ic = ic<0?0:(ic>=nc)?nc-1:ic;
					    *nbp++ = imagei[ir*ocols+ic];
					}
			}
			else {
				nnp = ip + upleft;
				nbp = nb;
				for(ii=minus;ii<=plus;ii++) 
				{
					for(jj=minus;jj<=plus;jj++)
						*nbp++ = *nnp++;
					nnp += nexw;
				}
			}


			for(nep=ne,nbp=nb,ii=minus; ii<plus; ii++)
				*nep++ = *nbp++;
			for(ii=minus; ii<plus; ii++)
			{
				*nep++ = *nbp;
				nbp += size;
			}
			for(ii=minus; ii<plus; ii++)
				*nep++ = *nbp--;
			for(ii=minus; ii<plus; ii++)
			{
				*nep++ = *nbp;
				nbp -= size;
			}
			for(nep=ne,ii=0; ii<4; ii++)
			{
				sum = 0;
				nepp = nep + st;
				if(nepp>=(ne+sizecir))
					nepp -= sizecir;
				for(nwp=nw,jj=0; jj<size; jj++)
				{
					work = (*nep++  - *nepp--) * *nwp++;
					sum += ((work<0)? (-work) : work);
					if(nepp<ne)
						nepp += sizecir;
				}
				d[ii] = sum;
				nep -= (plus+1);
			}

			min = max = d[0];
			iimin = 0;
			for(ii=1; ii<4; ii++)
				if(d[ii]<min)
				{
					min = d[ii];
					iimin = ii;
				} else if(d[ii]>max)
					max = d[ii];
			if((max-min)<t)
			{
				sum = 0;
				nbp = nb;
				for(ii=minus; ii<=plus; ii++)
					for(jj=minus; jj<=plus; jj++)
						sum += *nbp++;
				sum /= sizesq;
			} else
			{
				switch(iimin)
				{
				case 0:
					nbp = nb + plus;
					step = size;
					break;
				case 1:
					nbp = nb + size - 1;
					step = size - 1;
					break;
				case 2:
					nbp = nb + size * plus;
					step = 1;
					break;
				default:
					nbp = nb;
					step = size + 1;
				} 
				sum = 0;
				for(ii=minus; ii<=plus; ii++)
				{
					sum += *nbp;
					nbp += step;
				}
				sum /= size;
			}
			ip++;
			*op++ = sum;
		}
		ip += nexi;
		op += nexi;
	}
	free((char*)nb);
	free((char*)ne);
	free((char*)nw);
	return(HIPS_OK);
}
