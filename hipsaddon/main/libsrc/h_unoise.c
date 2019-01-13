static char *SccsId = "%W%      %G%";

/*      Copyright (c) 1991 The Turing Institute

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/* h_unoise - add random noise of uniform distribution to each image of
 *          the input sequence
 *		 
 * usage: h_unoise(hd,sdev)
 *
 * where
 *    sdev:   sdev is the standard deviation
 *
 * to load: cc -o h_unoise h_unoise.c -lhips -lm
 *
 * Jin Zhengping 11/9/86
 * Rewritten by Jin Zhengping - 31 August 1991
 */

#include <hipl_format.h>
#include <math.h>

double random_uniform();

int h_unoise(hd,sdev)
struct header	*hd;
double		sdev;
{
	switch(hd->pixel_format)
	{
	case PFBYTE:	return(h_unoise_b(hd,sdev));
	case PFFLOAT:	return(h_unoise_f(hd,sdev));
	default:	return(perr(HE_FMTSUBR,"h_unoise",
				hformatname(hd->pixel_format)));
	}
}

int h_unoise_b(hd,sdev)
struct header	*hd;
double		sdev;
{
	return(h_unoise_B(hd->firstpix,hd->rows,hd->cols,hd->ocols,sdev));
}

int h_unoise_f(hd,sdev)
struct header	*hd;
double		sdev;
{
	return(h_unoise_F((float *)hd->firstpix,hd->rows,hd->cols,hd->ocols,sdev));
}

int h_unoise_B(image,nr,nc,ocols,sdev)
byte	*image;
int	nr,nc,ocols;
double	sdev;
{
	byte	*ip=image;
	int	nexi=ocols-nc;
	int	i,j,m;
	double	random_uniform();

	for(i=0;i<nr;i++)
	{
		for(j=0;j<nc;j++)
		{
			m= random_uniform()*sdev + (int)*ip;
			if(m<0)m=0;
			else if(m>255)m=255;
			*ip++=m;
		}
		ip+=nexi;
	}
	return(HIPS_OK);
}

int h_unoise_F(image,nr,nc,ocols,sdev)
float	*image;
int	nr,nc,ocols;
double	sdev;
{
	float	*ip=image;
	int	nexi=ocols-nc;
	int	i,j;
	double	random_uniform();

	for(i=0;i<nr;i++)
	{
		for(j=0;j<nc;j++,ip++)
			*ip += random_uniform()*sdev;
		ip+=nexi;
	}
	return(HIPS_OK);
}

/* random_uniform - to generate random samples
 * 	from the uniform distribution with mean=0
 *	and s.d.=1.0.
 *
 * The routine  calls sr "random" from the standard library.
 * When using this routine load also -lm
 *
 * Jin Zhengping 11/9/86
 */
static double scale1 = (3.46410161513775458705/H__MAXRAND);
static double scale2 = 1.73205080756887729352;

double
random_uniform()
{
	H__RANDTYPE	H__RANDOM();
	return((double)H__RANDOM()*scale1-scale2);
}
