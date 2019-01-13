/*
 * The pyramid utilities are derived from code originally written by
 * Raj Hingorani at SRI/David Sarnoff Research Institute.  The original
 * Gaussian and Laplacian pyramid algorithms were designed by Peter Burt (also
 * currently at SRI/DSRC).  See:  Computer Graphics and Image Processing,
 * Volume 16, pp. 20-51, 1981, and IEEE Transactions on Communications,
 * Volume COM-31, pp. 532-540, 1983.
 *
 * modified for HIPS 2 - msl - 1/3/91
 */

/*
 * pyrfiltio.c - read pyramid filter definitions
 *
 * Note that the pyramid code is based only upon symmetric filters with an
 * odd number of taps.  Filters are stored in Ascii with only the taps from
 * the center (tap 0) rightward or downward through tap n, so that taps -1 to
 * -n are defined to be the same as taps 1 to n.  The file format is:
 *
 *	nred scalered
 *	reduce filter tap 0
 *	reduce filter tap 1
 *		.
 *		.
 *		.
 *	reduce filter tap nred
 *	nexp scaleexp
 *	expand filter tap 0
 *	expand filter tap 1
 *		.
 *		.
 *		.
 *	expand filter tap exp
 *
 * All tap values are divided by the corresponding value of scale.
 */

#include <stdio.h>
#include <hipl_format.h>

int getpyrfilters(filename,rf,ef)

Filename filename;
FILTER *rf,*ef;

{
	FILE *stream;
	int default_1dfilter(),read_1dfilter();

	if (filename == (Filename) 0) {
		if (default_1dfilter(rf) == HIPS_ERROR)
			return(HIPS_ERROR);
		return(default_1dfilter(ef));
	}
	else {
		if ((stream=fopen(filename,"r")) == NULL)
			return(perr(HE_OPEN,filename));
		if (read_1dfilter(rf,stream,filename) == HIPS_ERROR)
			return(HIPS_ERROR);
		if (read_1dfilter(ef,stream,filename) == HIPS_ERROR)
			return(HIPS_ERROR);
		fclose(stream);
	}
	return(HIPS_OK);
}

int read_1dfilter(f,stream,filename)

FILTER *f;
FILE *stream;
Filename filename;

{
	register float *p;
	int ntaps,i,j;
	float val,scale;

	j = fscanf(stream,"%d %f",&ntaps,&scale);
	if (j != 2)
		return(perr(HE_READFILE,filename));
	if ((p = (float *) memalloc(ntaps+1,sizeof(*p))) ==
	    (float *) HIPS_ERROR)
		return(HIPS_ERROR);
	for (i=0; i <= ntaps; i++) {
		j = fscanf(stream,"%f",&val);
		if (j != 1)
			return(perr(HE_READFILE,filename));
		p[i] = val/scale;
	}
	f->k = p;
	f->taps2 = ntaps;
	return(HIPS_OK);
}

int default_1dfilter(f)

FILTER *f;

{
	float *p;

	f->taps2 = 2;
	if ((p = (float *) memalloc(3,sizeof(*p))) == (float *) HIPS_ERROR)
		return(HIPS_ERROR);
	p[0] = .4;
	p[1] = .25;
	p[2] = .05;
	f->k = p;
	return(HIPS_OK);
}
