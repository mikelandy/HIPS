static char *SccsId = "%W%      %G%";

/*      Copyright (c) 1991 The Turing Institute

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * h_snn - apply a symmetric-nearest-neighbour filter to each image
 *       of the input sequence
 *
 * usage:   h_snn(hdi,hdo,,med,size)
 *
 * where 
 *
 *
 *    med        specifies the labelling method involved. With -d "median
 *               value method" is used while without it "mean value method" 
 *               is used.
 *
 *    size       is an integer which specifies the width of the window 
 *               masked on every pixel when the pixel is processed. It 
 *               should be an odd number.
 *
 * to load:	cc -o h_snn h_snn.c -lhips
 *
 * Jin Zhengping -16/10/86 
 * Rewritten by Jin Zhengping - 31 August 1991
 */

#include <hipl_format.h>

int h_snn(hdi,hdo,med,size)
struct header	*hdi,*hdo;
int		med,size;
{
	switch(hdi->pixel_format)
	{
	case PFBYTE:	return(h_snn_b(hdi,hdo,med,size));
	default:	return(perr(HE_FMTSUBR,"h_snn",
				hformatname(hdi->pixel_format)));
	}
}

int h_snn_b(hdi,hdo,med,size)
struct header	*hdi,*hdo;
int		med,size;
{
	return(h_snn_B(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
		    hdi->ocols,med,size));
}


int h_snn_B(imagei,imageo,nr,nc,ocols,med,size)
byte	*imagei,*imageo;
int	nr,nc,ocols;
int	med,size;
{
	int	i,j,ii,jj,xi,yi,work;
	short	im,im2,pap,pan;

	short	*bop,*bopp;
	void	*malloc();

	int	wwh = size/2;
	int	ww2 = size*size/2;
	int	ww4 = ww2/2;

	if((bop=(short *)malloc((unsigned)ww2*sizeof(*bop)))==0)
		perr(HE_ALLOC);

	for(i=wwh; i<nr-wwh; i++)
		for(j=wwh; j<nc-wwh; j++)
		{
			bopp = bop;
			im = *(imagei+i*ocols+j);
			im2 = im*2;
			for (xi = -wwh; xi<0; xi++)
				for (yi = -wwh; yi<=wwh; yi++)
				{
					pap = *(imagei+(i-xi)*ocols+(j-yi));
					pan = *(imagei+(i+xi)*ocols+(j+yi));
					if((pap+pan)>im2)
						*bopp++ = (pap>pan)? pan: pap;
					else if((pap+pan)<im2)
						*bopp++ = (pap>pan)? pap: pan;
					else *bopp++ = im;
				}

			for (yi = -wwh; yi<0; yi++)
			{
				pap = *(imagei+i*ocols+(j-yi));
				pan = *(imagei+i*ocols+(j+yi));
				if((pap+pan)>im2)
					*bopp++ = (pap>pan)? pan: pap;
				else if((pap+pan)<im2)
					*bopp++ = (pap>pan)? pap: pan;
				else *bopp++ = im;
			}

			if(med==TRUE)
			{
				for(ii=ww2-1; ii>=ww4; ii--)
					for(jj=0, bopp=bop; jj<ii; jj++, bopp++)
						if(*bopp < *(bopp+1))
						{
							work = *(bopp+1);
							*(bopp+1) = *bopp;
							*bopp = work;
						}
				*(imageo+(i*ocols)+j) = *(bop+ww4);
			} else 
			{
				work = 0;
				bopp = bop; 
				for(ii=0; ii<ww2; ii++)
					work += *bopp++;
				*(imageo+(i*ocols)+j) = work/ww2;
			}
		}

	free((char *)bop);
	return(HIPS_OK);
}
