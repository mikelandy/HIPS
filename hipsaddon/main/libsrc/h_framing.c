static char *SccsId = "%W%      %G%";

/*      Copyright (c) 1991 The Turing Institute

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * h_framing - add a thin frame around the image 
 *
 * usage:       h_framing(hdi,hdo,frameval)
 *
 *  where
 *      frameval:   the gray level for the frame.
 *
 * to load:	cc -o h_framing h_framing.c -lhips
 *
 * Jin Zhengping - 16/12/87
 * Rewritten by Jin Zhengping - 31 August 1991
 */

#include <hipl_format.h>

int h_framing(hdi,hdo,frameval)
struct header	*hdi,*hdo;
int		frameval;
{
	switch(hdi->pixel_format)
	{
	case PFBYTE:	return(h_framing_b(hdi,hdo,frameval));
	default:	return(perr(HE_FMTSUBR,"h_framing",
				hformatname(hdi->pixel_format)));
	}
}

int h_framing_b(hdi,hdo,frameval)
struct header	*hdi,*hdo;
int		frameval;
{
	return(h_framing_B(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
		    (hdi->ocols-hdi->cols),frameval));
}

int h_framing_B(imagei,imageo,nr,nc,nex,frameval)
byte	*imagei,*imageo;
int	nr,nc,nex;
int	frameval;
{
	register int	i,j;
	int		r1=nr-1;
	int		c1=nc-1;
	byte		*imageop = imageo;
	byte		*imageip = imagei;

	for (i=0;i<nr;i++) 
	{
		for (j=0;j<nc;j++) 
		{
			if(i==0 || i==r1 || j==0 ||j==c1)
				*imageop++ = frameval , imageip++ ;
			else
				*imageop++ = *imageip++ ;
		}
		imageip+=nex;
		imageop+=nex;
	}
	return(HIPS_OK); ;
}
