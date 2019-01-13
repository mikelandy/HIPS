static char *SccsId = "%W%      %G%";

/*      Copyright (c) 1991 The Turing Institute

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * h_translatei - translate each image in the input sequence with interpolation
 *
 * usage:	h_translatei(hdi,hdo,srow,scol)
 *
 * where
 *    srow,scol     "srow,scol" is a pair of double-floating point
 *                  values for the offset to be translated.
 *
 *  Note: any part of the translated image that lie outside the output
 *  image will be clipped.
 *
 * to load:	cc -o h_translatei h_translatei.c -lhipsa -lhips -lm
 *
 * Jin Zhengping -8/3/89 
 * Rewritten by Jin Zhengping - 31 August 1991
 */

#include <math.h>
#include <hipl_format.h>

int h_translatei(hdi,hdo,srow,scol)
struct header	*hdi,*hdo;
double		srow,scol;
{
	switch(hdi->pixel_format)
	{
	case PFBYTE:	return(h_translatei_b(hdi,hdo,srow,scol));
	default:	return(perr(HE_FMTSUBR,"h_translatei",
				hformatname(hdi->pixel_format)));
	}
}

int h_translatei_b(hdi,hdo,srow,scol)
struct header	*hdi,*hdo;
double		srow,scol;
{
	return(h_translatei_B(hdi->image,hdo->image,hdi->orows,hdi->ocols,
		    hdo->orows,hdo->ocols,srow,scol));
}

#define BACKGROUND	0

int h_translatei_B(imagei,imageo,nri,nci,nro,nco,srow,scol)
byte	*imagei,*imageo;
int	nri,nci,nro,nco;
double	srow,scol;
{
	int	i, j;
	double	i3f,j3f;
	int	nri1 = nri-1;
	int	nci1 = nci-1;
	double	diffr = 0.5*(nri-nro)-srow;
	double	diffc = 0.5*(nci-nco)-scol;
	byte	*ofrp = imageo ;
	byte	h_interpo_B() ;

	for(i3f=diffr,i=0; i<nro; i++,i3f++)
		for(j3f=diffc,j=0; j<nco; j++,j3f++)
		{
			if(i3f>=0.0 && i3f<=nri1 && j3f>=0.0 && j3f<=nci1)
				*ofrp++ = h_interpo_B(imagei,nci,i3f,j3f);
			else
				*ofrp++ = BACKGROUND ;
		}
	return(HIPS_OK);
}
