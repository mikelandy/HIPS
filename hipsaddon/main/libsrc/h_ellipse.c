static char *SccsId = "%W%      %G%";

/*      Copyright (c) 1991 The Turing Institute

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * h_ellipse - generate an ellipse line drawing image
 *
 * usage:	h_ellipse(hd,cenr,cenc,radr,radc)
 *
 * where  
 *
 *    cenr cenc:  the row and column position of the center of the 
 *                ellipse and defaults to the middle of the image.
 *
 *    radr radc:  the vertical and horizontal radiuses of the ellipse
 *                and defaults to the value as large as the ellipse thus
 *                drawn can be contained totally within the image.
 *
 *
 * to load:	cc -o h_ellipse h_ellipse.c -lhips -lm
 *
 * Jin Zhengping -12/6/85 
 * Jin Zhengping -14/2/86 Ed.2 
 * Rewritten by Jin Zhengping - 31 August 1991
 */

#include <stdio.h>
#include <hipl_format.h>
#include <math.h>

int h_ellipse(hd,cenr,cenc,radr,radc)
struct header   *hd;
int		cenr,cenc;
double		radr,radc;
{
	switch(hd->pixel_format)
	{
	case PFBYTE:    return(h_ellipse_b(hd,cenr,cenc,radr,radc));
	default:        return(perr(HE_FMTSUBR,"h_ellipse",
				hformatname(hd->pixel_format)));
	}
}

int h_ellipse_b(hd,cenr,cenc,radr,radc)
struct header   *hd;
int		cenr,cenc;
double		radr,radc;
{
	return(h_ellipse_B(hd->image,hd->orows,hd->ocols,
			   cenr,cenc,radr,radc));
}

#define BACKGROUND	20
#define ELLIPSE		235
#define MY_PI		3.1415926
int h_ellipse_B(image,nr,nc,cenr,cenc,radr,radc)
byte    *image;
int     nr,nc;
int	cenr,cenc;
double	radr,radc;
{
	double	rstep,rr,rup;
	int	sin_in,cos_in;
	char	*memsetl();

	memsetl((char *)image, (int)BACKGROUND, nr*nc*sizeof(*image));

	rstep = 0.9/((radr>radc)? radr: radc);
	rup = 2*MY_PI+rstep;
	for(rr=0.0; rr<=rup; rr+=rstep)
	{
		sin_in = sin(rr) * radc + 0.5;
		cos_in = cos(rr) * radr + 0.5;
		image[(cenr+cos_in)*nc+cenc+sin_in] = ELLIPSE;
	}
	return(HIPS_OK); ;
}
