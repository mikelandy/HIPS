static char *SccsId = "%W%      %G%";

/*	Copyright (c) 1991 The Turing Institute

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * h_project - project each image in the input sequence onto a tilted
 *             plane in turn 
 *
 * usage:	h_project(hdi,hdo,angle,distance,scale)
 *
 *  angle       "angle" is a double-floating point value for the degree of the
 *              angle subtended by the projecting plane and the image plane.
 *              The projecting plane only rotates about the horizontal axis
 *              of the image plane.
 *
 *  distance    "distance" is a double-floating poin value for the distance
 *              from perspective focus point to the centre of the image.
 *
 *  scale       "scale" is a double-floating poin value for the scale by which
 *              the image is magnified.
 *
 *  Note: any part of the projected image that lie outside the output
 *  image will be clipped.
 *
 *
 * to load:	cc -o h_project h_project.c -lhipsa -lhips -lm
 *
 * Jin Zhengping -8/3/89 
 * Rewritten by Jin Zhengping - 31 August 1991
 * Added h_interpo_F - Mike Landy - 23 June 1998
 *
 */

#include <hipl_format.h>
#include <math.h>

byte h_interpo_B();
float h_interpo_F();

int h_project(hdi,hdo,angle,distance,scale)
struct header	*hdi,*hdo;
double		angle,distance,scale;
{
	switch(hdi->pixel_format)
	{
	case PFBYTE:	return(h_project_b(hdi,hdo,angle,distance,scale));
	default:	return(perr(HE_FMTSUBR,"h_project",
				hformatname(hdi->pixel_format)));
	}
}

int h_project_b(hdi,hdo,angle,distance,scale)
struct header	*hdi,*hdo;
double		angle,distance,scale;
{
	return(h_project_B(hdi->image,hdo->image,hdi->orows,hdi->ocols,
		    hdo->orows,hdo->ocols,angle,distance,scale));
}

#define BACKGROUND	0
#define MY_PI      	3.1415927

int h_project_B(imagei,imageo,nri,nci,nro,nco,angle,distance,scale)
byte	*imagei,*imageo;
int	nri,nci,nro,nco;
double	angle,distance,scale;
{
	int	i, j;
	double	i3f,j3f,tmpf;
	int	nri1 = nri-1;
	int	nci1 = nci-1;
	double	nri2 = 0.5*nri ;
	double	nci2 = 0.5*nci ;
	double	nro2 = 0.5*nro;
	double	nco2 = 0.5*nco;
	double	cosl = cos(angle*MY_PI/180.0);
	double	sinl = sin(angle*MY_PI/180.0);
	double	ss = distance/scale;
	byte	*ofrp = imageo ;
	byte	h_interpo_B() ;

	for(i = -nro2; i<nro2; i++)
	{
		tmpf = ss/(sinl*(i+nro2) + distance);
		i3f = cosl*i*tmpf + nri2;
		for(j = -nco2; j<nco2; j++)
		{
			j3f = j*tmpf + nci2;
			if(i3f>=0.0 && i3f<=nri1 && j3f>=0.0 && j3f<=nci1)
				*ofrp++ = h_interpo_B(imagei,nci,i3f,j3f);
			else
				*ofrp++ = BACKGROUND ;
		}
	}
	return(HIPS_OK);
}

byte
h_interpo_B(pic, nc, rf, cf)
byte	*pic;
int	nc ;
double   rf, cf ;
{
	int	ri = (int)rf;
	int	ci = (int)cf;
	byte	*picp = pic + ri*nc + ci;
	double	lam0 = rf - ri ;
	double	lam1 = 1.0 - lam0 ;
	double	tmp0 = lam1 * *(picp) + lam0 * *(picp+nc) ;
	double	tmp1 = lam1 * *(picp+1) + lam0 * *(picp+nc+1) ;

	lam0 = cf - ci ;
	lam1 = 1.0 - lam0 ;
	return((byte)(lam1*tmp0 + lam0*tmp1));
}

float
h_interpo_F(pic, nc, rf, cf)
float	*pic;
int	nc ;
double   rf, cf ;
{
	int	ri = (int)rf;
	int	ci = (int)cf;
	float	*picp = pic + ri*nc + ci;
	double	lam0 = rf - ri ;
	double	lam1 = 1.0 - lam0 ;
	double	tmp0 = lam1 * *(picp) + lam0 * *(picp+nc) ;
	double	tmp1 = lam1 * *(picp+1) + lam0 * *(picp+nc+1) ;

	lam0 = cf - ci ;
	lam1 = 1.0 - lam0 ;
	return((float)(lam1*tmp0 + lam0*tmp1));
}
