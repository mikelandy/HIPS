static char *SccsId = "%W%      %G%";

/*      Copyright (c) 1991 The Turing Institute

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * h_sigmaspat - estimate the standard deviations of random noise
 * of the input image. The double-formated value sigma is return
 * by 'sigma'.
 *
 * usage:	h_sigmaspat(hdi,sig)
 *
 * where
 *        hdi:    header of the input image
 *        sigma:  sigma returned.
 *
 * The pixel format of the images is byte. The greater
 * ROI dimension of the sequence must be larger than 15 to ensure
 * a reliable estimation. The other less ROI dimenion must be greater
 * than 1.
 *
 * to load:	cc -c h_sigmaspat.c -lhips -lm 
 *
 * Peter Mowforth & Jin Zhengping -7/5/85 
 * Modified by Jin Zhengping -5/1/90 
 *      1)  lift of the maximum image size limit;
 *      2)  change of window-width default value from 9 to 1.
 * Rewritten by Jin Zhengping - 31 August 1991
 */

#include <stdio.h>
#include <math.h>
#include <hipl_format.h>

int h_sigmaspat(hdi,sigma)
struct header	*hdi;
float		*sigma;
{
	switch(hdi->pixel_format)
	{
	case PFBYTE:    return(h_sigmaspat_b(hdi,sigma));
	default:        return(perr(HE_FMTSUBR,"h_sigmaspat",
				hformatname(hdi->pixel_format)));
	}
}

int h_sigmaspat_b(hdi,sigma)
struct header   *hdi;
float          *sigma;
{
	return(h_sigmaspat_B(hdi->firstpix,hdi->rows,hdi->cols,
		hdi->ocols,sigma));
}

#define A   0.001163 /* Parameters A, B and C form a    */
#define B  -0.017740 /* binomial curve which compensate */
#define C   1.219382 /* the underestimation of sigma.   */

int h_sigmaspat_B(imagei,nr,nc,ocols,sigma)
byte    *imagei;
int     nr,nc,ocols;
float	*sigma;
{
	int	i,j;
	byte	*ip;
	double  w,ss;
	double	sig = (nr+nc)*256*256;
	int	nexi=ocols-nc;

	if(nc>=nr)
	{
		ip=imagei+ocols;
		for(i=1; i<(nr-1); i++)
		{
			ss=0.0;
			for(j=0; j<nc; j++,ip++)
			{
				w = (double)*(ip-ocols) - *ip ;
				ss += w * w;
				w = (double)*(ip+ocols) - *ip ;
				ss += w * w;
			}
			if(ss<sig) sig = ss;
			ip+=nexi;
		}
		sig = sqrt((double)(sig/nc))*0.5;
	} else
	{
		for(j=1; j<(nc-1); j++)
		{
			ip=imagei+j;
			ss=0.0;
			for(i=0; i<nr; i++,ip+=ocols)
			{
				w = (double)*(ip-1) - *ip ;
				ss += w * w;
				w = (double)*(ip+1) - *ip ;
				ss += w * w;
			}
			if(ss<sig) sig = ss;
		}
		sig = sqrt((double)(sig/nr))*0.5;
	}

	if(sig<0.5)
		sig *= A*0.5*0.5 + B*0.5 + C;
	else if(sig>8.0)
		sig *= A*8.0*8.0 + B*8.0 + C;
	else sig *= A*sig*sig + B*sig + C;
	fprintf(stderr,"sigma = %f\n",sig);
	*sigma = (float)sig;
	return(HIPS_OK);
}
