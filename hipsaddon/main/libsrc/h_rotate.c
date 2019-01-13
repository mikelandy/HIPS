static char *SccsId = "%W%      %G%";

/*      Copyright (c) 1991 The Turing Institute

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * h_rotate - h_rotate each image in the input sequence about its central pixel
 *
 * usage:	h_rotate(hdi,hdo,angle)
 *
 * where
 *    angle       "angle" is a double-floating point value for the degree of
 *                angle to be rotated.
 *
 *  Note: any part of the rotated image that lie outside the output
 *  image will be clipped.
 *
 * to load:	cc -o h_rotate h_rotate.c -lhipsa -lhips -lm
 *
 * Jin Zhengping -8/3/89 
 * Rewritten by Jin Zhengping - 31 August 1991
 * added float images - Mike Landy - 23 June 1998
 */

#include <math.h>
#include <hipl_format.h>

int h_rotate(hdi,hdo,angle)
struct header	*hdi,*hdo;
double		angle;
{
	switch(hdi->pixel_format)
	{
	case PFBYTE:	return(h_rotate_b(hdi,hdo,angle));
	case PFFLOAT:	return(h_rotate_f(hdi,hdo,angle));
	default:	return(perr(HE_FMTSUBR,"h_rotate",
				hformatname(hdi->pixel_format)));
	}
}

int h_rotate_b(hdi,hdo,angle)
struct header	*hdi,*hdo;
double		angle;
{
	return(h_rotate_B(hdi->image,hdo->image,hdi->orows,hdi->ocols,
		    hdo->orows,hdo->ocols,angle));
}

int h_rotate_f(hdi,hdo,angle)
struct header	*hdi,*hdo;
double		angle;
{
	return(h_rotate_F((float *) hdi->image,(float *) hdo->image,hdi->orows,
		hdi->ocols,hdo->orows,hdo->ocols,angle));
}

#define BACKGROUND	0
#define MY_PI      3.1415927

int h_rotate_B(imagei,imageo,nri,nci,nro,nco,angle)
byte	*imagei,*imageo;
int	nri,nci,nro,nco;
double	angle;
{
	int	i, j;
	double	cf,sf,i3f,j3f;
	int	nri1 = nri-1;
	int	nci1 = nci-1;
	double	nri2 = 0.5*nri ;
	double	nci2 = 0.5*nci ;
	double	nro2 = 0.5*nro;
	double	nco2 = 0.5*nco;
	double	cosl = cos(angle*MY_PI/180.0);
	double	psinl = sin(angle*MY_PI/180.0);
	double	msinl = -psinl;
	byte	*ofrp = imageo ;
	byte	h_interpo_B() ;

	for(i = -nro2; i<nro2; i++)
	{
		cf = cosl*i + nri2 ;
		sf = msinl*i + nci2 ;
		for(j = -nco2; j<nco2; j++)
		{
			i3f = cf + psinl*j;
			j3f = sf + cosl*j;
			if(i3f>=0.0 && i3f<=nri1 && j3f>=0.0 && j3f<=nci1)
				*ofrp++ = h_interpo_B(imagei,nci,i3f,j3f);
			else
				*ofrp++ = BACKGROUND ;
		}
	}
	return(HIPS_OK);
}

int h_rotate_F(imagei,imageo,nri,nci,nro,nco,angle)
float	*imagei,*imageo;
int	nri,nci,nro,nco;
double	angle;
{
	int	i, j;
	double	cf,sf,i3f,j3f;
	int	nri1 = nri-1;
	int	nci1 = nci-1;
	double	nri2 = 0.5*nri ;
	double	nci2 = 0.5*nci ;
	double	nro2 = 0.5*nro;
	double	nco2 = 0.5*nco;
	double	cosl = cos(angle*MY_PI/180.0);
	double	psinl = sin(angle*MY_PI/180.0);
	double	msinl = -psinl;
	float	*ofrp = imageo ;
	float	h_interpo_F() ;

	for(i = -nro2; i<nro2; i++)
	{
		cf = cosl*i + nri2 ;
		sf = msinl*i + nci2 ;
		for(j = -nco2; j<nco2; j++)
		{
			i3f = cf + psinl*j;
			j3f = sf + cosl*j;
			if(i3f>=0.0 && i3f<=nri1 && j3f>=0.0 && j3f<=nci1)
				*ofrp++ = h_interpo_F(imagei,nci,i3f,j3f);
			else
				*ofrp++ = BACKGROUND ;
		}
	}
	return(HIPS_OK);
}
