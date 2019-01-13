/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_bclean.c - remove small 8-connected components
 *
 * Delete 8-connected components in a white-on-black or thinned and
 * categorized image of size less than or equal to a given amount.
 *
 * pixel formats: BYTE
 *
 * Mike Landy - 11/24/82
 * HIPS 2 - msl - 8/8/91
 */

#include <hipl_format.h>
static int dx[8] = {1,1,0,-1,-1,-1,0,1};
static int dy[8] = {0,-1,-1,-1,0,1,1,1};
static short *x,*y;
static int npoint,NUMpoint,savenlp,savenr,savenc;
static byte *saveimage;
int addneigh();
h_boolean inlist();

int h_bclean(hd,size)

struct header *hd;
int size;

{
	switch(hd->pixel_format) {
	case PFBYTE:	return(h_bclean_b(hd,size));
	default:	return(perr(HE_FMTSUBR,"h_bclean",
				hformatname(hd->pixel_format)));
	}
}

int h_bclean_b(hd,size)

struct header *hd;
int size;

{
	return(h_bclean_B(hd->firstpix,hd->rows,hd->cols,hd->ocols,size));
}

int h_bclean_B(image,nr,nc,nlp,size)

byte *image;
int nr,nc,nlp,size;

{
	int *first;
	int ncomp,NUMcomp,i,j,nex;
	byte *p;

	savenlp = nlp;
	saveimage = image;
	savenr = nr;
	savenc = nc;
	nex = nlp-nc;
	NUMcomp = 3*nr*nc/10;
	NUMpoint = nr*nc;
	if ((first = (int *) memalloc(NUMcomp+1,sizeof (int *))) ==
		(int *) HIPS_ERROR)
			return(HIPS_ERROR);
	if ((x = (short *) memalloc(NUMpoint,sizeof (short *))) ==
		(short *) HIPS_ERROR)
			return(HIPS_ERROR);
	if ((y = (short *) memalloc(NUMpoint,sizeof (short *))) ==
		(short *) HIPS_ERROR)
			return(HIPS_ERROR);
	p = image;
	ncomp = npoint = 0;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			if (*p++) {
			    if (!inlist(j,i)) {
				if (++ncomp>NUMcomp)
					return(perr(HE_COMPOV));
				if (npoint>=NUMpoint)
					return(perr(HE_PNTOV));
				first[ncomp-1]=npoint;
				x[npoint] = j;
				y[npoint++] = i;
				if (addneigh(j,i,ncomp) == HIPS_ERROR)
					return(HIPS_ERROR);
			    }
			}
		}
		p += nex;
	}
	first[ncomp]=npoint;
	for (i=0;i<ncomp;i++) {
		if (first[i+1]-first[i]<=size) {
			for (j=first[i];j<first[i+1];j++)
				image[y[j]*nlp+x[j]] = 0;
		}
	}
	free(first); free(x); free(y);
	return(HIPS_OK);
}

int addneigh(xx,yy,comp)

int xx,yy,comp;

{
	int i,nx,ny;

	for (i=0;i<8;i++) {
		nx = xx + dx[i];
		ny = yy + dy[i];
		if (nx<0 || nx>=savenc || ny<0 || ny>savenr)
			continue;
		if (saveimage[ny*savenlp+nx]) {
			if (!inlist(nx,ny)) {
				if (npoint>=NUMpoint)
					return(perr(HE_PNTOV));
				x[npoint] = nx;
				y[npoint++] = ny;
				if (addneigh(nx,ny,comp) == HIPS_ERROR)
					return(HIPS_ERROR);
			}
		}
	}
	return(HIPS_OK);
}

h_boolean inlist(xx,yy)

int xx,yy;

{
	int i;

	for (i=0;i<npoint;i++)
		if (x[i]==xx && y[i]==yy)
			return(TRUE);
	return(FALSE);
}
