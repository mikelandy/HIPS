/*
 * Modified version (HIPS-2):
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*	Original Copyright (c) 1987 Pierre Landau

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * h_median.c - apply a median filter to an image
 *
 * where size is the length of the side of the neighborhood in which the
 * median is computed.
 *
 * pixel formats: BYTE
 *
 * Mike Landy - 5/28/82
 * median algorithm replaced <Pierre Landau 1/6/87>
 * HIPS 2 - msl - 6/16/91
 */

#include <hipl_format.h>
static h_boolean nballoc = FALSE;
static int nbsize,*nb;
static h_boolean colalloc = FALSE;
static int *col,saverows,savecols;
int sselect();

int h_median(hdi,hdo,size)

struct header *hdi,*hdo;
int size;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	return(h_median_b(hdi,hdo,size));
	default:	return(perr(HE_FMTSUBR,"h_median",
				hformatname(hdi->pixel_format)));
	}
}

int h_median_b(hdi,hdo,size)

struct header *hdi,*hdo;
int size;

{
	return(h_median_B(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
	    hdi->ocols,hdo->ocols,size));
}

int h_median_B(imagei,imageo,nr,nc,nlpi,nlpo,size)

byte *imagei,*imageo;
int nr,nc,nlpi,nlpo,size;

{
	int	sizesq,halfsz,ir,ic;
	int	minus,plus,*np,top,bot,left,right,nexi,nexo,nextrow;
	byte	*ip,*op,*nnp;
	register int i,j,ii,jj;		/* loop counters */

	sizesq = size*size;
	halfsz = (sizesq + 1) / 2;
	plus = size / 2;
	minus = plus - size + 1;
	top = -minus;
	bot = nr - plus;
	left = -minus;
	right = nc - plus;
	nextrow = nlpi*minus + minus;
	if (!nballoc || nbsize < size) {
		if (nballoc)
			free(nb);
		nb = (int *) memalloc(size*size,sizeof(int));
		nballoc = TRUE;
		nbsize = size;
	}
	if (!colalloc || saverows < nr) {
		if (colalloc)
			free(col);
		col = (int *) memalloc(nr,sizeof(int));
		colalloc = TRUE;
		savecols = nlpi+1; /* force computation */
	}
	if (saverows != nr || savecols != nlpi) {
		saverows = nr;
		savecols = nlpi;
		for (ic = -nlpi,i=0;i<nr;i++)
			col[i] = (ic += nlpi); /* vector array */
	}
	ip = imagei;
	op = imageo;
	nexi = nlpi-nc;
	nexo = nlpo-nc;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			if (i<top || i>=bot || j<left || j>=right) {
				np = nb;
				for (ii=minus;ii<=plus;ii++)
					for (jj=minus;jj<=plus;jj++) {
					    ir = i + ii;
					    ic = j + jj;
					    ir = ir<0?0:(ir>=nr)?nr-1:ir;
					    ic = ic<0?0:(ic>=nc)?nc-1:ic;
					    *np++ = imagei[col[ir]+ic];
					}
			}
			else {
				nnp = ip + nextrow;
				np = nb;
				for (ii=minus;ii<=plus;ii++) {
					for (jj=minus;jj<=plus;jj++)
						*np++ = *nnp++;
					nnp += nlpi - size;
				}
			}
			ip++;
			*op++ = sselect(halfsz,nb,nb+sizesq-1);
		}
		ip += nexi;
		op += nexo;
	}
	return(HIPS_OK);
}

#define exchi(a,b) {tmpi = a; a = b; b = tmpi;}	/* exchange ints */
#define exchp(a,b) {tmpp = a; a = b; b = tmpp;}	/* exchange pointers */

int sselect(k,lo,hi)

int k,*lo,*hi;

/* select the k'th element from the list between lo and hi
 * this is a implementation of R.W.Floyd's improvement to Hoare's
 * original algorithm, as published in CACM, vol 18 no 3 (march 1975) 
 */

{
	register int *i,*j;
	int *val[3],t,*tmpp,tmpi,df;

	while(1) {
		if (hi == lo)
			return(*lo);
		if ((t = hi-lo) <= 2) { /* if the sequence is short (n<3) sort
					it directly */
			val[0] = lo;
			val[1] = lo+1;
			val[2] = hi;
			if (t == 1) {
			    return(*val[0] < *val[1] ? *val[k-1] : *val[2-k]);
			}
			else {
				if (*val[0] > *val[1]) exchp(val[0],val[1])
				if (*val[0] > *val[2]) exchp(val[0],val[2])
				if (*val[1] > *val[2]) exchp(val[1],val[2])
				return (*val[k-1]);
			}
		}
		else {
			t = *lo; /* take first element of list as pivot */
			i = lo;
			j = hi;
			if (*hi > t)
				exchi(*hi,*lo) /* set up for first exchange */
			while (i < j) {
				exchi(*i,*j)
				i++;
				j--;
				while (*i < t) i++;
					/* scan list for pair to exchange */
				while (*j > t)
					j--;
			}
			if (*lo == t)
				exchi(*lo,*j)
					/* put pivot back where it belongs */
			else {
				j++;
				exchi(*j,*hi)
			}

			/* now adjust hi,lo so they surround the subset
			   containing the k-l+1th element */

			df = j-lo+1;
			if (df < k) {
				k = k-(df);
				lo = j+1;
			}
			else {
				if (df == k)
					return (*j);
				else
					hi = j - 1;
			}
		}
	}
}
