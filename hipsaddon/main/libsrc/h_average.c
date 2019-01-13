static char *SccsId = "%W%      %G%";

/*      Copyright (c) 1991 The Turing Institute

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * h_average - average sub-sequence of frames pixel-by-pixel without normalization
 *
 * usage:	h_average(hdi,hdo) 
 *
 * where
 *    hdi -- header of the input sequence to be averaged.
 *    hdo -- header of the output image.
 *
 * to load:	cc -o h_average h_average.c -lhips
 *
 * Peter Mowforth & Jin Zhengping - 8/5/85
 * Rewritten by Jin Zhengping - 31 August 1991
 */

#include <hipl_format.h>

int h_average(hdi,hdo)
struct header   *hdi,*hdo;
{
	switch(hdi->pixel_format)
	{
	case PFBYTE:    return(h_average_b(hdi,hdo));
	default:        return(perr(HE_FMTSUBR,"h_average",
				hformatname(hdi->pixel_format)));
	}
}

int h_average_b(hdi,hdo)
struct header   *hdi,*hdo;
{
	return(h_average_B(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
			hdi->orows,hdi->ocols,(hdi->num_frame)));
}

int h_average_B(imagei,imageo,nr,nc,onr,onc,fr)
byte    *imagei,*imageo;
int     nr,nc,onr,onc;
int     fr;
{
	int     i,j,f;
	byte    *picp;
	int     *acc,*accp;
	int	nex = onc-nc;
	void	*malloc();

	if((acc=(int *)malloc((unsigned)nr*nc*sizeof(*acc)))==0)
		perr(HE_ALLOC);
	picp = imagei;
	accp = acc;
	for (i=0;i<nr;i++) 
	{
		for (j=0;j<nc;j++) 
			*accp++ = *picp++;
		picp += nex;
	}
	for (f=1;f<fr;f++)
	{
		picp = imagei+onr*onc*f;
		accp = acc;
		for (i=0;i<nr;i++) 
		{
			for (j=0;j<nc;j++) 
				*accp++ += *picp++;
			picp += nex;
		}
	}
	picp = imageo;
	accp = acc;
	for (i=0;i<nr;i++)
	{
		for (j=0;j<nc;j++) 
			*picp++ = *accp++/fr;
		picp += nex;
	}
	free((char *)acc);

	return(HIPS_OK) ;
}
