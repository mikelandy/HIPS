/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_dct.c - discrete cosine transform and its inverse in 2-d.
 *
 * usage:	h_dct_2d(input vector, output vector, log length of a side)
 *		h_dctinv_2d(  "      ,       "      ,          "          )
 *
 * to load:	cc -c dct.c -lm
 *
 *
 * Michael Landy - 3/11/82
 * modified for HIPS 2 - msl - 1/3/91
 *
 * Ulrik Skands - 17/11/92
 * HIPS 2 - upvs - 17/11/92
 */

#include <hipl_format.h>
#include <math.h>
#include <stdio.h>

static float *workvec,*iworkvec,*cconst,*iconst;
static int worksize = 0;
static int constsize = 0;

int h_dct(hdi,hdo,iflag)

struct header *hdi,*hdo;
Boolean iflag;

{
	switch(hdi->pixel_format) {
	case PFFLOAT: return(h_dct_f(hdi,hdo,iflag));
	default:        return(perr(HE_FMTSUBR,"h_dct",
				hformatname(hdi->pixel_format)));
	}
}

int h_dct_f(hdi,hdo,iflag)

struct header *hdi,*hdo;
Boolean iflag;

{
	return(h_dct_F((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,iflag));
}

int h_dct_F(imagei,imageo,nr,nc,iflag)

float *imagei,*imageo;
int nr,nc;
Boolean iflag;

{
	int i,j,logrows,logcols;

	logrows = logcols = -1;
	i = 0; j = 1;
	while (1) {
		if (j == nr)
			logrows = i;
		else if (j > nr)
			break;
		if (j == nc)
			logcols = i;
		else if (j > nc)
			break;
                if (logrows >= 0 && logcols >= 0)
			break;
		i++; j <<= 1;
	}
	if (logrows == -1 || logcols == -1)
		return(perr(HE_POW2));
	if(iflag) return(h_dctinv_2d(imagei,imageo,logcols));
	else return(h_dct_2d(imagei,imageo,logcols));
}

int h_dct_2d(ivec,ovec,loglen)

float *ivec,*ovec;
int loglen;

{
	register int size,i,j;
	register float *w,*iw,*p,sq2;

	size = 1<<loglen;
	if (worksize < size*2) {
		if ((0==(workvec=(float *)malloc(4*size*size*sizeof(float)))) ||
		   (0==(iworkvec=(float *)malloc(4*size*size*sizeof(float)))) ||
		   (0==(cconst=(float *)malloc(size*sizeof(float)))) ||
		   (0==(iconst=(float *)malloc(size*sizeof(float)))))
			return(perr(HE_ALLOCSUBR,"dct_2d"));
		worksize = 2*size;
	}
	if (constsize != size*2) {
		constsize = 2*size;
		w = cconst;
		iw = iconst;
		for (i=0;i<size;i++) {
			*w++ = cos(-H_PI*i/(2*size));
			*iw++ = sin(-H_PI*i/(2*size));
		}
	}
	w = workvec;
	iw = iworkvec;
	p = ivec;
	for (i=0;i<2*size;i++)
		for (j=0;j<2*size;j++) {
			*iw++=0.;
			if (i<size && j<size)
				*w++ = *p++;
			else
				*w++ = 0.;
		}
	for (i=0;i<2*size*size;i+=2*size)
		fftn(workvec+i,iworkvec+i,loglen+1,1);
	w = workvec;
	iw = iworkvec;
	for (i=0;i<size;i++) {
		for (j=0;j<size;j++) {
			*w++ = (*w * *(cconst+j)) - (*iw * *(iconst+j));
			*iw++ = 0.;
		}
		w+=size;
		iw+=size;
	}
	for (i=0;i<size;i++)
		fftn(workvec+i,iworkvec+i,loglen+1,size*2);
	p = ovec;
	w = workvec;
	iw = iworkvec;
	sq2 = sqrt( (double) 2.);
	for (i=0;i<size;i++) {
		for (j=0;j<size;j++) {
			*p = 2. * ((*w++ * *(cconst+i)) -
				(*iw++ * *(iconst+i)))/size;
			if (j == 0 && i == 0)
				*p /= 2.;
			else if (j == 0 || i == 0)
				*p /= sq2;
			*p++;
		}
		w += size;
		iw += size;
	}
	return(HIPS_OK);
}

int h_dctinv_2d(ivec,ovec,loglen)

float *ivec,*ovec;
int loglen;

{
	register int size,i,j;
	register float *w,*iw,*p,sq2;

	size = 1<<loglen;
	if (worksize < size*2) {
		if ((0==(workvec=(float *)malloc(4*size*size*sizeof(float)))) ||
		   (0==(iworkvec=(float *)malloc(4*size*size*sizeof(float)))) ||
		   (0==(cconst=(float *)malloc(size*sizeof(float)))) ||
		   (0==(iconst=(float *)malloc(size*sizeof(float)))))
			return(perr(HE_ALLOCSUBR,"dctinv_2d"));
		worksize = size*2;
	}
	if (constsize != size*2) {
		constsize = 2*size;
		w = cconst;
		iw = iconst;
		for (i=0;i<size;i++) {
			*w++ = cos(-H_PI*i/(2*size));
			*iw++ = sin(-H_PI*i/(2*size));
		}
	}
	w = workvec;
	iw = iworkvec;
	p = ivec;
	sq2 = sqrt( (double) 2.0);
	for (i=0;i<2*size;i++)
		for (j=0;j<2*size;j++) {
			if (i<size && j<size) {
				*w = *p++;
				if (i==0 && j==0)
					*w /= 2.0;
				else if (i==0 || j==0)
					*w /= sq2;
				*iw++ = *w * *(iconst+j);
				*w++ = *w * *(cconst+j);
			}
			else {
				*w++ = 0.;
				*iw++ = 0.;
			}
		}
	for (i=0;i<2*size*size;i+=2*size)
		fftn(workvec+i,iworkvec+i,loglen+1,1);
	w = workvec;
	iw = iworkvec;
	for (i=0;i<size;i++) {
		for (j=0;j<size;j++) {
			*iw++ = *w * *(iconst+i);
			*w++ = *w * *(cconst+i);
		}
		w += size;
		iw += size;
	}
	for (i=0;i<size;i++)
		fftn(workvec+i,iworkvec+i,loglen+1,2*size);
	p = ovec;
	w = workvec;
	iw = iworkvec;
	for (i=0;i<size;i++) {
		for (j=0;j<size;j++)
			*p++ = 2. * *w++ /size;
		w += size;
		iw += size;
	}
	return(HIPS_OK);
}
