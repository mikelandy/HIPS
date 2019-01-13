static char *SccsId = "%W%      %G%";

/*      Copyright (c) 1991 The Turing Institute 

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * h_cshift - shift circularlly each image in the input sequence.
 *
 * usage:  h_cshift(hdi,hdo,srow,scol)
 *
 * where
 *       hdi,hdo:      headers of input and output images.
 *       srow (scol):  starting row (column) that will be shifted to
 *                     the 0th row (column), and defaults to rows/2,
 *                     (cols/2) where rows (cols) is the number of rows
 *                     (columns) of the image.
 * This module handles images of byte, short, int, float, double, complex, 
 * and double complex formats.
 *
 * to load:	cc -o h_cshift h_cshift.c -lhips
 *
 * Jin Zhengping - 17/7/87
 * Rewritten by Jin Zhengping - 31 August 1991
 */

#include <hipl_format.h>
#include <stdio.h>

int h_cshift(hdi,hdo,srow,scol)
struct header   *hdi,*hdo;
int             srow,scol;
{
	switch(hdi->pixel_format)
	{
	case PFBYTE:    return(h_cshift_b(hdi,hdo,srow,scol));
	case PFSHORT:   return(h_cshift_s(hdi,hdo,srow,scol));
	case PFINT:     return(h_cshift_i(hdi,hdo,srow,scol));
	case PFFLOAT:   return(h_cshift_f(hdi,hdo,srow,scol));
	case PFDOUBLE:  return(h_cshift_d(hdi,hdo,srow,scol));
	case PFCOMPLEX: return(h_cshift_c(hdi,hdo,srow,scol));
	case PFDBLCOM:  return(h_cshift_dc(hdi,hdo,srow,scol));
	default:        return(perr(HE_FMTSUBR,"h_cshift",
				hformatname(hdi->pixel_format)));
	}
}

int h_cshift_b(hdi,hdo,srow,scol)
struct header   *hdi,*hdo;
int             srow,scol;
{
	return(h_cshift_B(hdi->image,hdo->image,
			  hdi->orows,hdi->ocols,srow,scol));
}

int h_cshift_s(hdi,hdo,srow,scol)
struct header   *hdi,*hdo;
int             srow,scol;
{
	return(h_cshift_S((short *)hdi->image,(short *)hdo->image,
			  hdi->orows,hdi->ocols,srow,scol));
}

int h_cshift_i(hdi,hdo,srow,scol)
struct header   *hdi,*hdo;
int             srow,scol;
{
	return(h_cshift_I((int *)hdi->image,(int *)hdo->image,
			  hdi->orows,hdi->ocols,srow,scol));
}

int h_cshift_f(hdi,hdo,srow,scol)
struct header   *hdi,*hdo;
int             srow,scol;
{
	return(h_cshift_F((float *)hdi->image,(float *)hdo->image,
			  hdi->orows,hdi->ocols,srow,scol));
}

int h_cshift_d(hdi,hdo,srow,scol)
struct header   *hdi,*hdo;
int             srow,scol;
{
	return(h_cshift_D((double *)hdi->image,(double *)hdo->image,
			  hdi->orows,hdi->ocols,srow,scol));
}


int h_cshift_c(hdi,hdo,srow,scol)
struct header   *hdi,*hdo;
int             srow,scol;
{
	return(h_cshift_C((float *)hdi->image,(float *)hdo->image,
			  hdi->orows,hdi->ocols,srow,scol));
}

int h_cshift_dc(hdi,hdo,srow,scol)
struct header   *hdi,*hdo;
int             srow,scol;
{
	return(h_cshift_DC((double *)hdi->image,(double *)hdo->image,
			  hdi->orows,hdi->ocols,srow,scol));
}

int h_cshift_B(imagei,imageo,row,col,srow,scol)
byte    *imagei,*imageo;
int	row,col,srow,scol ;
{
	int	i,j,ii,jj;
	byte	*ipic=imagei,*opic;

	for(ii=srow,i=0;i<row;i++,ii++)
	{
		opic=imageo+(ii%row)*col;
		for(jj=scol,j=0;j<col;j++,jj++)
			*(opic+jj%col) = *ipic++;
	}
	return(HIPS_OK);
}

int h_cshift_S(imagei,imageo,row,col,srow,scol)
short    *imagei,*imageo;
int	row,col,srow,scol ;
{
	int	i,j,ii,jj;
	short	*ipic=imagei,*opic;

	for(ii=srow,i=0;i<row;i++,ii++)
	{
		opic=imageo+(ii%row)*col;
		for(jj=scol,j=0;j<col;j++,jj++)
			*(opic+jj%col) = *ipic++;
	}
	return(HIPS_OK);
}

int h_cshift_I(imagei,imageo,row,col,srow,scol)
int    *imagei,*imageo;
int	row,col,srow,scol ;
{
	int	i,j,ii,jj;
	int	*ipic=imagei,*opic;

	for(ii=srow,i=0;i<row;i++,ii++)
	{
		opic=imageo+(ii%row)*col;
		for(jj=scol,j=0;j<col;j++,jj++)
			*(opic+jj%col) = *ipic++;
	}
	return(HIPS_OK);
}

int h_cshift_F(imagei,imageo,row,col,srow,scol)
float    *imagei,*imageo;
int	row,col,srow,scol ;
{
	int	i,j,ii,jj;
	float	*ipic=imagei,*opic;

	for(ii=srow,i=0;i<row;i++,ii++)
	{
		opic=imageo+(ii%row)*col;
		for(jj=scol,j=0;j<col;j++,jj++)
			*(opic+jj%col) = *ipic++;
	}
	return(HIPS_OK);
}

int h_cshift_D(imagei,imageo,row,col,srow,scol)
double    *imagei,*imageo;
int	row,col,srow,scol ;
{
	int	i,j,ii,jj;
	double	*ipic=imagei,*opic;

	for(ii=srow,i=0;i<row;i++,ii++)
	{
		opic=imageo+(ii%row)*col;
		for(jj=scol,j=0;j<col;j++,jj++)
			*(opic+jj%col) = *ipic++;
	}
	return(HIPS_OK);
}

int h_cshift_C(imagei,imageo,row,col,srow,scol)
float    *imagei,*imageo;
int	row,col,srow,scol ;
{
	int	i,j,ii,jj;
	float	*ipic=imagei,*opic;

	for(ii=srow,i=0;i<row;i++,ii++)
	{
		opic=imageo+(ii%row)*col*2;
		for(jj=scol,j=0;j<col;j++,jj++)
		{
			*(opic+(jj%col)*2) = *ipic++;
			*(opic+(jj%col)*2+1) = *ipic++;
		}
	}
	return(HIPS_OK);
}

int h_cshift_DC(imagei,imageo,row,col,srow,scol)
double    *imagei,*imageo;
int	row,col,srow,scol ;
{
	int	i,j,ii,jj;
	double	*ipic=imagei,*opic;

	for(ii=srow,i=0;i<row;i++,ii++)
	{
		opic=imageo+(ii%row)*col*2;
		for(jj=scol,j=0;j<col;j++,jj++)
		{
			*(opic+(jj%col)*2) = *ipic++;
			*(opic+(jj%col)*2+1) = *ipic++;
		}
	}
	return(HIPS_OK);
}
