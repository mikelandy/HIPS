static char *SccsId = "%W%      %G%";

/*      Copyright (c) 1991 The Turing Institute

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * h_sigmatemp - estimate the standard deviations of random noise
 * of the input image. The double-formated value sigma is return
 * by 'sigma'. The pixel format of the images is byte.
 *
 * usage:	h_sigmatemp(hd1,hd2,pure,sigma)
 *
 * where
 *        hd1,hd2:  header of the input image.
 *        pure:     one of the images is without noise.
 *        sigma:    sigma returned.
 *
 * to load:	cc -c h_sigmatemp.c -lhips -lm 
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

int h_sigmatemp(hd1,hd2,pure,sigma)
struct header   *hd1,*hd2;
h_boolean		pure;
float		*sigma;
{
	switch(hd1->pixel_format)
	{
	case PFBYTE:    return(h_sigmatemp_b(hd1,hd2,pure,sigma));
	default:        return(perr(HE_FMTSUBR,"h_sigmatemp",
				hformatname(hd1->pixel_format)));
	}
}

int h_sigmatemp_b(hd1,hd2,pure,sigma)
struct header   *hd1,*hd2;
h_boolean		pure;
float		*sigma;
{
	return(h_sigmatemp_B(hd1->firstpix,hd2->firstpix,hd1->rows,hd1->cols,
		hd1->ocols,pure,sigma));
}

int h_sigmatemp_B(image1,image2,nr,nc,ocols,pure,sigma)
byte    *image1,*image2;
int     nr,nc,ocols;
h_boolean	pure;
float	*sigma;
{
	int	i,j;
	double  w;
	double	p = (pure==TRUE)? 1.0: 0.5;
	byte	*p1=image1;
	byte	*p2=image2;
	int	nexi = ocols-nc;
	double	sig = 0.0;

	for(i=0; i<nr; i++)
	{
		for(j=0; j<nc; j++)
		{
			w = (double)*p1++ - *p2++;
			sig += w * w;
		}
		p1+=nexi;
		p2+=nexi;
	}
	sig = sqrt((double)(p*sig/(nc*nr)));
	fprintf(stderr,"sigma = %f\n",sig);
	*sigma= (float)sig;
	return(HIPS_OK);
}
