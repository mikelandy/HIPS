/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * halftone - halftones using the Floyd-Steinberg algorithm
 *
 * halftone(hd) = halftone2(hd,0,255,0,7,3,5,1)
 * halftone2(hd,lower,upper,rflag,a,b,c,d)
 *
 * Halftone converts an 8-bit sequence to a bi-level sequence using the
 * Floyd-Steinberg error diffusion algorithm.  The image must be in byte
 * format, and is halftoned in place.  The values for cleared and set
 * pixels are taken from hips_lchar/hips_hchar.
 * In the input image, the values below `lower' are treated as black, and above
 * `upper' are treated as white.  The effective range is thus from `lower'
 * to `upper'.  The algorithm is based on diffusing the error to adjacent
 * pixels created when a pixel is changed to 0 or 255 (despite the fact that 0
 * and 255 are not necessarily used).  The algorithm proceeds across each row
 * from left to right, one row at a time.  The error is parceled out to four
 * neighbors: `right', `below-left', `below', and `below-right', using
 * relative weights of the error a, b, c, and d, respectively.  They are
 * non-negative integers which must sum to 16.  Finally, the process
 * may be randomly dithered (switch rflag), which uses a random threshold for
 * each pixel chosen uniformly across the pixel range.
 *
 * pixel formats: BYTE
 *
 * Mike Landy - 7/28/87 (based on code by Lou Salkind and Jim Bergen)
 * HIPS 2 - msl - 1/8/91
 */

#include <hipl_format.h>
#define	RANDOMVALS	1201
static int allocnc1;
static h_boolean erralloc = FALSE;
static int oldrange = -1;
static h_boolean ralloc = FALSE;
static int oldlower = -1;
static int oldupper = -1;
static short trans[256],*error,*rtab;

int h_halftone(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_halftone2(hdi,hdo,0,255,0,7,3,5,1));
}

int h_halftone2(hdi,hdo,lower,upper,rflag,alpha,beta,gamma,delta)

struct header *hdi,*hdo;
int lower,upper,alpha,beta,gamma,delta;
h_boolean rflag;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	return(h_halftone_b(hdi,hdo,lower,upper,rflag,
				alpha,beta,gamma,delta));
	default:	return(perr(HE_FMTSUBR,"h_halftone2",
				hformatname(hdi->pixel_format)));
	}
}

int h_halftone_b(hdi,hdo,lower,upper,rflag,alpha,beta,gamma,delta)

struct header *hdi,*hdo;
int lower,upper,alpha,beta,gamma,delta;
h_boolean rflag;

{
	return(h_halftone_B(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
	    hdi->ocols,hdo->ocols,lower,upper,rflag,alpha,beta,gamma,delta));
}

int h_halftone_B(imagei,imageo,nr,nc,nlpi,nlpo,lower,upper,rflag,alpha,beta,gamma,
	delta)

byte *imagei,*imageo;
int nr,nc,nlpi,nlpo,lower,upper,alpha,beta,gamma,delta;
h_boolean rflag;

{
	int nc1,i,j,k,range,range2,rptr,eptr,nexi,nexo;
	byte *bpi,*bpo;
	long time();

	nc1 = nc + 1;
	range = upper - lower;
	range2 = range/2;
	if (range <= 0 || range > 255 || lower < 0 || upper > 255)
		return(perr(HE_RNG,"halftone2"));
	if (erralloc) {
		if (allocnc1 != nc1) {
			free(error);
			error = (short *) memalloc(nc1,sizeof(short));
			allocnc1 = nc1;
		}
	}
	else {
		error = (short *) memalloc(nc1,sizeof(short));
		allocnc1 = nc1;
		erralloc = TRUE;
	}
	if (rflag) {
		if (!ralloc) {
			rtab = (short *) memalloc(RANDOMVALS,sizeof(short));
			ralloc = TRUE;
		}
		/*
		 * initialize table of random numbers
		 * in the range of pixel values
		 */
		if (oldrange != range) {
			H__SRANDOM((int)time((int *)0));
			for (i=0;i<RANDOMVALS;i++)
				rtab[i] = H__RANDOM() % range;
			oldrange = range;
		}
		rptr = 0;
	}

	/* set up transformation matrix */
	if (lower!=oldlower || upper!=oldupper) {
		for (i = 0; i < 256; i++) {
			/* insure value between 0 and range */
			k = i - lower;
			if (k < 0)
				k = 0;
			else if (k > range)
				k = range;
			trans[i] = k;
		}
		oldlower = lower;
		oldupper = upper;
	}
	for (i=0;i<=nc;i++)
		error[i]=0;
	bpi = imagei;
	bpo = imageo;
	nexi = nlpi-nc;
	nexo = nlpo-nc;
	eptr = 0;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			k = error[eptr%nc1] + trans[*bpi++];
			if (k > (rflag ? rtab[rptr++] : range2)) {
				*bpo++ = hips_hchar;
				k -= range;
			}
			else
				*bpo++ = hips_lchar;
			if (rflag && rptr >= RANDOMVALS)
				rptr = 0;
			if (j == 0) {
				error[(eptr+1)%nc1] +=
					(alpha * k)>>4;
				error[(eptr+nc)%nc1] =
					(gamma * k)>>4;
				error[eptr%nc1] =
					(delta * k)>>4;
			}
			else {
				error[(eptr+1)%nc1] +=
					(alpha * k)>>4;
				error[(eptr+nc-1)%nc1] +=
					(beta * k)>>4;
				error[(eptr+nc)%nc1] +=
					(gamma * k)>>4;
				error[eptr%nc1] =
					(delta * k)>>4;
			}
			eptr++;
		}
		bpi += nexi;
		bpo += nexo;
	}
	return(HIPS_OK);
}
