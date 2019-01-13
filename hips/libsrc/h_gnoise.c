/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_gnoise.c - subroutines to add Gaussian noise to an image
 *
 * pixel formats: SHORT, FLOAT
 *
 * Yoav Cohen 3/15/82
 * modified for float images: Mike Landy 10/19/83
 * bug fixed for byte (short) images: Mike Landy 11/4/87
 * HIPS 2 - msl - 8/5/91
 */

#include <hipl_format.h>
#include <math.h>

double rand_gauss(),rand_g();

int h_gnoise(hdi,hdo,p,fastflag)

struct header *hdi,*hdo;
double p;
h_boolean fastflag;

{
	switch(hdi->pixel_format) {
	case PFSHORT:	return(h_gnoise_s(hdi,hdo,p,fastflag));
	case PFFLOAT:	return(h_gnoise_f(hdi,hdo,p,fastflag));
	default:	return(perr(HE_FMTSUBR,"h_gnoise",
				hformatname(hdi->pixel_format)));
	}
}

int h_gnoise_s(hdi,hdo,p,fastflag)

struct header *hdi,*hdo;
double p;
h_boolean fastflag;

{
	return(h_gnoise_S((short *) hdi->firstpix,(short *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,p,fastflag));
}

int h_gnoise_f(hdi,hdo,p,fastflag)

struct header *hdi,*hdo;
double p;
h_boolean fastflag;

{
	return(h_gnoise_F((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,p,fastflag));
}

int h_gnoise_S(imagei,imageo,nr,nc,nlpi,nlpo,p,fastflag)

short *imagei,*imageo;
int nr,nc,nlpi,nlpo;
double p;
h_boolean fastflag;

{
	register int i,j,nexi,nexo;
	register short *pi,*po;
	double x;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	if (fastflag) {
	    for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			x = rand_g()*p;
			*po++ = *pi++ +
				((int) ((x>=0) ? (x + 0.5) : (x - 0.5)));
				/* this maps (-.5,.5) to 0, so value 0
				   doesn't occur twice as often as it
				   should */
		}
		pi += nexi;
		po += nexo;
	    }
	}
	else {
	    for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			x = rand_gauss()*p;
			*po++ = *pi++ +
				((int) ((x>=0) ? (x + 0.5) : (x - 0.5)));
				/* this maps (-.5,.5) to 0, so value 0
				   doesn't occur twice as often as it
				   should */
		}
		pi += nexi;
		po += nexo;
	    }
	}
	return(HIPS_OK);
}

int h_gnoise_F(imagei,imageo,nr,nc,nlpi,nlpo,p,fastflag)

float *imagei,*imageo;
int nr,nc,nlpi,nlpo;
double p;
h_boolean fastflag;

{
	register int i,j,nexi,nexo;
	register float *pi,*po;

	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	if (fastflag) {
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++)
				*po++ = *pi++ + rand_g()*p;
			pi += nexi;
			po += nexo;
		}
	}
	else {
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++)
				*po++ = *pi++ + rand_gauss()*p;
			pi += nexi;
			po += nexo;
		}
	}
	return(HIPS_OK);
}

/*
 * rand_gauss - generate random samples from a standard normal distribution
 *
 * The method is the one outlined in algorithm #267 of the ACM.
 * (based on the Box&Muller method for generating pairs
 *  of normal deviates).
 *
 * The routine  calls "random" from the standard library.
 * Resetting is therefore done by calling "srandom(seed)".
 * Note that if the main program contains independent
 * calls to "random", the sequence may not be reproducible.
 *
 * Yoav Cohen 6/2/82
 */

double rand_gauss()

{
	static double scale = 2147483648.0;
	static h_boolean sw = TRUE;
	static double rand1,rand2,delta,phi;
	static int k;

	if (sw) {
		do k = H__RANDOM(); while (k==0);
		rand1 = k/scale;
		do k = H__RANDOM(); while (k==0);
		rand2 = k/scale;
		phi = H_2PI*rand2;
		delta = sqrt(-2*log(rand1));
		sw = FALSE;
		return(delta*cos(phi));
	}
	sw = TRUE;
	return(delta*sin(phi));
}

/*
 * rand_g - compute random normal deviates by a
 *		method based on the central limit theorem.
 *
 * doesn't use the routine "random()", initialization
 * should be done via srand_g(seed).
 * Fairly inaccurate - and never produces a sample more than 6 standard
 * deviations from the mean.
 *
 * Yoav Cohen 6/4/82
 */

static long grandx = 1;

int srand_g(x)

unsigned x;

{
	grandx=x;
	return(HIPS_OK);
}

double rand_g()

{
	static int i,sum;

	sum=0.0;
	for(i=0;i<12;i++)
	sum += (((grandx=grandx*1103515245+12345)&0x7fffffff)>>4);
	return(sum/134217728.0-6.0);
}
