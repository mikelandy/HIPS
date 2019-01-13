static char *SccsId = "%W%      %G%";

/*      Copyright (c) 1991 The Turing Institute

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * h_extend - double the size of each image in the input sequence by
 *          filling the extended area with 0, the boundary values of
 *          that image, or the opposite half image as if the image
 *          is first extended by repeating itself and then wrapped
 *          by half the image size. (Used for FFT.)
 *
 * usage:	h_extend(hdi,hdo,boundary,wrapping)
 *
 *     -b:       a flag specifying that boundary values are to be 
 *               filled the extended area, otherwise 0 is to be used
 *               if -w is not set.
 *
 *     -w:       a flag specifying that wrapping is to be applied to
 *               fill the extended area, otherwise 0 is to be used
 *               if -b is not set.
 *
 * The module handles images of byte, float, or double format.
 *
 * to load:	cc -o h_extend h_extend.c -lhips
 *
 * Jin Zhengping - 17/7/87
 * Rewritten by Jin Zhengping - 31 August 1991
 */

#include <stdlib.h>
#include <hipl_format.h>

int h_extend(hdi,hdo,boundary,wrapping)
struct header	*hdi,*hdo;
h_boolean		boundary,wrapping;
{
        switch(hdi->pixel_format)
        {
        case PFBYTE:    return(h_extend_b(hdi,hdo,boundary,wrapping));
        case PFFLOAT:   return(h_extend_f(hdi,hdo,boundary,wrapping));
        case PFDOUBLE:  return(h_extend_d(hdi,hdo,boundary,wrapping));
        default:        return(perr(HE_FMTSUBR,"h_extend",
                                hformatname(hdi->pixel_format)));
        }
}

int h_extend_b(hdi,hdo,boundary,wrapping)
struct header	*hdi,*hdo;
h_boolean		boundary,wrapping;
{
        return(h_extend_B(hdi->image,hdo->image,
			  hdi->orows,hdi->ocols,
			  boundary,wrapping));
}

int h_extend_f(hdi,hdo,boundary,wrapping)
struct header	*hdi,*hdo;
h_boolean		boundary,wrapping;
{
        return(h_extend_F((float *)hdi->image,(float *)hdo->image,
			  hdi->orows,hdi->ocols,
			  boundary,wrapping));
}

int h_extend_d(hdi,hdo,boundary,wrapping)
struct header	*hdi,*hdo;
h_boolean		boundary,wrapping;
{
        return(h_extend_D((double *)hdi->image,(double *)hdo->image,
			  hdi->orows,hdi->ocols,
			  boundary,wrapping));
}

int h_extend_B(imagei,imageo,nr,nc,boundary,wrapping)
byte	*imagei,*imageo;
int	nr,nc;
h_boolean	boundary,wrapping;
{
	int		i,j;
	byte		*ip, *op ;
	int		inrc = nr*nc;
	unsigned	osize = inrc*sizeof(*imagei)*4 ; 
	int		nr2 = nr*2;
	int		nc2 = nc*2;
	int		hnr = nr/2;
	int		hnc = nc/2;
	int		hnr3 = nr+hnr;
	int		hnc3 = nc+hnc;

	if(boundary==TRUE)
	{
		op=imageo ;
		for(i=0;i<hnr;i++)
		{
			ip=imagei ;
			for(j=0;j<hnc;j++)
				*op++ = *ip ;
			for(j=hnc;j<hnc3;j++)
				*op++ = *ip++ ;
			ip-- ;
			for(j=hnc3;j<nc2;j++)
				*op++ = *ip ;
		}
		ip=imagei ;
		for(i=hnr;i<hnr3;i++)
		{
			for(j=0;j<hnc;j++)
				*op++ = *ip ;
			for(j=hnc;j<hnc3;j++)
				*op++ = *ip++ ;
			ip-- ;
			for(j=hnc3;j<nc2;j++)
				*op++ = *ip ;
			ip++ ;
		}
		for(i=hnr3;i<nr2;i++)
		{
			ip=imagei+inrc-nc ;
			for(j=0;j<hnc;j++)
				*op++ = *ip ;
			for(j=hnc;j<hnc3;j++)
				*op++ = *ip++ ;
			ip-- ;
			for(j=hnc3;j<nc2;j++)
				*op++ = *ip ;
		}
	} else if(wrapping==TRUE)
	{
		byte	*imaget,*op0,*op1,*op2,*op3;
		int	ii,jj;
		void	*malloc();
		if((imaget=(byte *)malloc(osize*sizeof(*imaget)))==0)
			perr(HE_ALLOC);
		ip=imagei;
		op0=imaget;
		op1=imaget+nc;
		op2=imaget+nr*nc2;
		op3=imaget+nr*nc2+nc;
		for(i=0;i<nr;i++)
		{
			for(j=0;j<nc;j++,ip++)
			{
				*op0++ = *ip;
				*op1++ = *ip;
				*op2++ = *ip;
				*op3++ = *ip;
			}
			op0 += nc;
			op1 += nc;
			op2 += nc;
			op3 += nc;
		}
		op=imageo;
		for(ii=hnr,i=0;i<nr2;i++,ii++)
		{
			op0=imaget+(ii%nr2)*nc2;
			for(jj=hnc,j=0;j<nc2;j++,jj++)
				*op++ = *(op0+(jj%nc2));
		}
		free((char *)imaget);
	} else
	{
		char	*memsetl();
		memsetl((char *)imageo, (int)0, (int)osize);
		ip=imagei , op=imageo+hnr*nc2+hnc ;
		for(i=0;i<nr;i++)
		{
			for(j=0;j<nc;j++)
				*op++ = *ip++ ;
			op += nc ;
		}
	}

	return(HIPS_OK);
}

int h_extend_F(imagei,imageo,nr,nc,boundary,wrapping)
float	*imagei,*imageo;
int	nr,nc;
h_boolean	boundary,wrapping;
{
	int		i,j;
	float		*ip, *op ;
	int		inrc = nr*nc;
	unsigned	osize = inrc*sizeof(*imagei)*4 ; 
	int		nr2 = nr*2;
	int		nc2 = nc*2;
	int		hnr = nr/2;
	int		hnc = nc/2;
	int		hnr3 = nr+hnr;
	int		hnc3 = nc+hnc;

	if(boundary==TRUE)
	{
		op=imageo ;
		for(i=0;i<hnr;i++)
		{
			ip=imagei ;
			for(j=0;j<hnc;j++)
				*op++ = *ip ;
			for(j=hnc;j<hnc3;j++)
				*op++ = *ip++ ;
			ip-- ;
			for(j=hnc3;j<nc2;j++)
				*op++ = *ip ;
		}
		ip=imagei ;
		for(i=hnr;i<hnr3;i++)
		{
			for(j=0;j<hnc;j++)
				*op++ = *ip ;
			for(j=hnc;j<hnc3;j++)
				*op++ = *ip++ ;
			ip-- ;
			for(j=hnc3;j<nc2;j++)
				*op++ = *ip ;
			ip++ ;
		}
		for(i=hnr3;i<nr2;i++)
		{
			ip=imagei+inrc-nc ;
			for(j=0;j<hnc;j++)
				*op++ = *ip ;
			for(j=hnc;j<hnc3;j++)
				*op++ = *ip++ ;
			ip-- ;
			for(j=hnc3;j<nc2;j++)
				*op++ = *ip ;
		}
	} else if(wrapping==TRUE)
	{
		float	*imaget,*op0,*op1,*op2,*op3;
		int	ii,jj;
		void	*malloc();
		if((imaget=(float *)malloc(osize*sizeof(*imaget)))==0)
			perr(HE_ALLOC);
		ip=imagei;
		op0=imaget;
		op1=imaget+nc;
		op2=imaget+nr*nc2;
		op3=imaget+nr*nc2+nc;
		for(i=0;i<nr;i++)
		{
			for(j=0;j<nc;j++,ip++)
			{
				*op0++ = *ip;
				*op1++ = *ip;
				*op2++ = *ip;
				*op3++ = *ip;
			}
			op0 += nc;
			op1 += nc;
			op2 += nc;
			op3 += nc;
		}
		op=imageo;
		for(ii=hnr,i=0;i<nr2;i++,ii++)
		{
			op0=imaget+(ii%nr2)*nc2;
			for(jj=hnc,j=0;j<nc2;j++,jj++)
				*op++ = *(op0+(jj%nc2));
		}
		free((char *)imaget);
	} else
	{
		char	*memsetl();
		memsetl((char *)imageo, (int)0, (int)osize);
		ip=imagei , op=imageo+hnr*nc2+hnc ;
		for(i=0;i<nr;i++)
		{
			for(j=0;j<nc;j++)
				*op++ = *ip++ ;
			op += nc ;
		}
	}

	return(HIPS_OK);
}

int h_extend_D(imagei,imageo,nr,nc,boundary,wrapping)
double	*imagei,*imageo;
int	nr,nc;
h_boolean	boundary,wrapping;
{
	int		i,j;
	double		*ip, *op ;
	int		inrc = nr*nc;
	unsigned	osize = inrc*sizeof(*imagei)*4 ; 
	int		nr2 = nr*2;
	int		nc2 = nc*2;
	int		hnr = nr/2;
	int		hnc = nc/2;
	int		hnr3 = nr+hnr;
	int		hnc3 = nc+hnc;

	if(boundary==TRUE)
	{
		op=imageo ;
		for(i=0;i<hnr;i++)
		{
			ip=imagei ;
			for(j=0;j<hnc;j++)
				*op++ = *ip ;
			for(j=hnc;j<hnc3;j++)
				*op++ = *ip++ ;
			ip-- ;
			for(j=hnc3;j<nc2;j++)
				*op++ = *ip ;
		}
		ip=imagei ;
		for(i=hnr;i<hnr3;i++)
		{
			for(j=0;j<hnc;j++)
				*op++ = *ip ;
			for(j=hnc;j<hnc3;j++)
				*op++ = *ip++ ;
			ip-- ;
			for(j=hnc3;j<nc2;j++)
				*op++ = *ip ;
			ip++ ;
		}
		for(i=hnr3;i<nr2;i++)
		{
			ip=imagei+inrc-nc ;
			for(j=0;j<hnc;j++)
				*op++ = *ip ;
			for(j=hnc;j<hnc3;j++)
				*op++ = *ip++ ;
			ip-- ;
			for(j=hnc3;j<nc2;j++)
				*op++ = *ip ;
		}
	} else if(wrapping==TRUE)
	{
		double	*imaget,*op0,*op1,*op2,*op3;
		int	ii,jj;
		void	*malloc();
		if((imaget=(double *)malloc(osize*sizeof(*imaget)))==0)
			perr(HE_ALLOC);
		ip=imagei;
		op0=imaget;
		op1=imaget+nc;
		op2=imaget+nr*nc2;
		op3=imaget+nr*nc2+nc;
		for(i=0;i<nr;i++)
		{
			for(j=0;j<nc;j++,ip++)
			{
				*op0++ = *ip;
				*op1++ = *ip;
				*op2++ = *ip;
				*op3++ = *ip;
			}
			op0 += nc;
			op1 += nc;
			op2 += nc;
			op3 += nc;
		}
		op=imageo;
		for(ii=hnr,i=0;i<nr2;i++,ii++)
		{
			op0=imaget+(ii%nr2)*nc2;
			for(jj=hnc,j=0;j<nc2;j++,jj++)
				*op++ = *(op0+(jj%nc2));
		}
		free((char *)imaget);
	} else
	{
		char	*memsetl();
		memsetl((char *)imageo, (int)0, (int)osize);
		ip=imagei , op=imageo+hnr*nc2+hnc ;
		for(i=0;i<nr;i++)
		{
			for(j=0;j<nc;j++)
				*op++ = *ip++ ;
			op += nc ;
		}
	}

	return(HIPS_OK);
}
