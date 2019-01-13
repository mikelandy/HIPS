/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_discedge2.c - a discrete domain regional edge detector with offsets
 *
 * size is the length of a side of the nonoverlapping domains in which
 * the algorithm operates, varcrit is the threshold on the variance
 * in the normalized region (normalized by mean only) below which no
 * edge is sought in that region, and edgethresh is a threshold applied to
 * edge values.
 *
 * This program is an implementation of the discrete domain regional operator
 * described by G. B. Shaw (Comp. Graph. and Image Proc., 9, pp. 135-149 (1979).
 * The algorithm outlined therein is sketchy and contains errors, which
 * hopefully are corrected here.  Also, the article does not clarify what to do
 * with edges which appear to travel along a border of the region (the algorithm
 * purports to be symmetric with respect to horizontal and vertical edges, but
 * isn't really).  In this implementation, the first pixels on the light
 * side of a light/dark edge are marked, and when the light/dark boundary
 * travels along the boundary of the region, only the "middlemost" pixel is
 * marked, since otherwise horizontal edges will all include little "tails" at
 * an edge of each region.  Lastly, note that any excess after multiples of
 * size in rows and columns is not edge detected.
 * The computations are done with integer arithmetic with all pixels scaled by
 * size*size in order that the normalization by the mean can be exact.
 *
 * This program is a modification of h_discedge.c in which the algorithm
 * operates twice on an image, at offsets of (0,0), (1,1),...,(size-1,size-1).
 * The edge value which the algorithm would give to a given pixel at a given
 * offset is thresholded by edge-thresh, and if above threshold, then a bit is
 * set in the output image at that pixel. Bit 0 is set for offset (0,0), bit 1
 * for offset (1,1), and so on. Thus, the output can be thresholded at 1 or 
 * (2**size)-1 to give the "or" or "and" of the two offset images, respectively.
 * The output image is in byte format.
 *
 * pixel formats: BYTE
 *
 * Mike Landy 6/1/82
 * HIPS 2 - msl - 8/8/91
 */

#include <hipl_format.h>
#include <math.h>

static int dalloc = FALSE;
static int *a,*b,*template,*normal;
static int savesize = -1;

int h_discedge2(hdi,hdo,size,varcrit,edgethresh)

struct header *hdi,*hdo;
int size,edgethresh;
float varcrit;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	return(h_discedge2_b(hdi,hdo,size,varcrit,edgethresh));
	default:	return(perr(HE_FMTSUBR,"h_discedge2",
				hformatname(hdi->pixel_format)));
	}
}

int h_discedge2_b(hdi,hdo,size,varcrit,edgethresh)

struct header *hdi,*hdo;
int size,edgethresh;
float varcrit;

{
	return(h_discedge2_B(hdi->firstpix,hdo->firstpix,hdi->rows,
		hdi->cols,hdi->ocols,hdo->ocols,size,varcrit,edgethresh));
}

int h_discedge2_B(imagei,imageo,nr,nc,nlpi,nlpo,size,varcrit,edgethresh)

byte *imagei,*imageo;
int nr,nc,nlpi,nlpo,size,edgethresh;
float varcrit;

{
	float sumsq;
	int *np,*ap,*bp,*b1,*b2,*b3,*b4;
	int *b1p,*b2p,*b3p,*b4p,bedge;
	int *b1pp,*b2pp,*b3pp,*b4pp;
	int sum,min,max,offset;
	int *tp,pos,bpos,btype;
	int r,c,i,rr,rc,nexi;
	byte *ip;

	if (dalloc && savesize < size) {
		free(a); free(b); free(template); free(normal);
		dalloc = FALSE;
	}
	if (!dalloc) {
		if ((a = (int *)  memalloc(size*(size+1),sizeof (int))) ==
			(int *) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((b = (int *)  memalloc(size*(size+1)*4,sizeof (int))) ==
			(int *) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((template = (int *)  memalloc(size,sizeof (int))) ==
			(int *) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((normal = (int *)  memalloc(size*size,sizeof (int))) ==
			(int *) HIPS_ERROR)
				return(HIPS_ERROR);
		dalloc = TRUE;
		savesize = size;
	}
	nexi = nlpi - size;
	b1 = b;
	b2 = b + (size+1)*size;
	b3 = b2 + (size+1)*size;
	b4 = b3 + (size+1)*size;

	h_setimage_B(imageo,nr,nc,nlpo,(byte) 0);
	for (offset=0;offset<size;offset++) {
	    for (rr=offset;rr<=nr-size;rr+=size) {
		for (rc=offset;rc<=nc-size;rc+=size) {
				/* extract the region */
			ip = imagei + rr*nlpi + rc;
			np = normal;
			sum = 0;
			for (r=0;r<size;r++) {
				for (c=0;c<size;c++) {
					sum += *ip;
					*np++ = *ip++ * size * size;
				}
				ip += nexi;
			}
			np = normal;
			sumsq = 0;
				/* normalize by mean, compute variance */
			for (i=0;i<size*size;i++) {
				*np -= sum;
				sumsq += *np * *np;
				np++;
			}
			sumsq /= size*size*size*size;
				/* skip region if variance is too low */
			if (sumsq <= varcrit)
				continue;
				/* compute a */
			ap = a;
			for (r=0;r<size;r++) {
				for (c=0;c<=size;c++) {
					*ap = 0;
					np = &normal[r*size];
					for (i=0;i<c;i++)
						*ap += *np++;
					for (;i<size;i++)
						*ap -= *np++;
					ap++;
				}
			}
				/* the first row of b1,...,b4 equals the
				   first row of a */
			b1p = b1;
			b2p = b2;
			b3p = b3;
			b4p = b4;
			ap = a;		
			for (c=0;c<=size;c++) {
				*b1p++ = *ap;
				*b2p++ = *ap;
				*b3p++ = *ap;
				*b4p++ = *ap++;
			}
			for (r=1;r<size;r++) {
					/* compute b1/b2 from left to right*/
				b1p = b1 + r*(size+1);
				b1pp = b1p - (size+1);
				b2p = b2 + r*(size+1);
				b2pp = b2p - (size+1);
				ap = a + r*(size+1);
				max = *b1pp++;
				min = *b2pp++;
				*b1p++ = max + *ap;
				*b2p++ = min + *ap++;
				for (c=1;c<=size;c++) {
					max = *b1pp>max ? *b1pp : max;
					b1pp++;
					*b1p++ = max + *ap;
					min = *b2pp<min ? *b2pp : min;
					b2pp++;
					*b2p++ = min + *ap++;
				}
					/* compute b3/b4 from right to left */
				b3pp = b3 + r*(size+1);
				b3p = b3pp + (size+1);
				b4pp = b4 + r*(size+1);
				b4p = b4pp + (size+1);
				ap = a + (r+1)*(size+1);
				max = *--b3pp;
				min = *--b4pp;
				*--b3p = max + *--ap;
				*--b4p = min + *ap;
				for (c=size-1;c>=0;c--) {
					max = *--b3pp>max ? *b3pp : max;
					*--b3p = max + *--ap;
					min = *--b4pp<min ? *b4pp : min;
					*--b4p = min + *ap;
				}
			}
				/* find best edge value/type/pos in top row */
			b1p = b1 + (size-1)*(size+1);
			b2p = b2 + (size-1)*(size+1);
			b3p = b3 + (size-1)*(size+1);
			b4p = b4 + (size-1)*(size+1);
			bedge = *b1p;
			bpos = 0;
			btype = 1;
			for (c=0;c<=size;c++) {
				if (*b1p>bedge) {
					bedge = *b1p++;
					bpos = c;
					btype = 1;
				}
				else
					b1p++;
				if (-*b2p>bedge) {
					bedge = - *b2p++;
					bpos = c;
					btype = 2;
				}
				else
					b2p++;
				if (*b3p>bedge) {
					bedge = *b3p++;
					bpos = c;
					btype = 3;
				}
				else
					b3p++;
				if (-*b4p>bedge) {
					bedge = - *b4p++;
					bpos = c;
					btype = 4;
				}
				else
					b4p++;
			}
			if (bedge<edgethresh)
				continue;
			tp = template + size;
			*--tp = bpos;
			switch (btype) {
	/* light on left, edge moves rightward as row increases */
			case 1:
					/* compute template */
				b1pp = b1 + (size-1)*(size+1) + bpos;
				pos = bpos;
				for (r=size-1;r>=0;r--) {
					b1p = b1pp - (size+1);
					max = *b1p;
					bp = b1p;
					if (pos != 0) {
						for (c=pos;c>=0;c--) {
							if (*b1p>max) {
								pos = c;
								bp = b1p;
								max = *b1p--;
							}
							else
								b1p--;
						}
					}
					*--tp = pos;
					b1pp = bp;
				}
					/* compute edgels */
				pos = template[0] - 1;
				tp = template - 1;
				for (r=0;r<size;r++) {
					if (*++tp == 0)
						continue;
					if (pos+1>=*tp)
						pos = *tp - 2;
					for (c=pos+1;c<*tp;c++)
						imageo[(rr+r)*nlpo+
						  rc+c] |= 1<<offset;
					pos = *tp - 1;
					if (pos == size-1)
						break;
				}
				break;
	/* light on right, edge moves rightward as row increases */
			case 2:
					/* compute template */
				b2pp = b2 + (size-1)*(size+1) + bpos;
				pos = bpos;
				for (r=size-1;r>=0;r--) {
					b2p = b2pp - (size+1);
					max = - *b2p;
					bp = b2p;
					if (pos != 0) {
						for (c=pos;c>=0;c--) {
							if (-*b2p>max) {
								pos = c;
								bp = b2p;
								max = - *b2p--;
							}
							else
								b2p--;
						}
					}
					*--tp = pos;
					b2pp = bp;
				}
					/* compute edgels */
				pos = template[size-1];
				tp = template + size;
				for (r=size-1;r>=0;r--) {
					if (*--tp == size)
						continue;
					if (pos-1<*tp)
						pos = *tp + 1;
					for (c=pos-1;c>=*tp;c--)
						imageo[(rr+r)*nlpo+
						  rc+c] |= 1<<offset;
					pos = *tp;
					if (pos == 0)
						break;
				}
				break;
	/* light on left, edge moves leftward as row increases */
			case 3:
					/* compute template */
				b3pp = b3 + (size-1)*(size+1) + bpos;
				pos = bpos;
				for (r=size-1;r>=0;r--) {
					b3p = b3pp - (size+1);
					max = *b3p;
					bp = b3p;
					if (pos != size) {
						for (c=pos;c<=size;c++) {
							if (*b3p>max) {
								pos = c;
								bp = b3p;
								max = *b3p++;
							}
							else
								b3p++;
						}
					}
					*--tp = pos;
					b3pp = bp;
				}
					/* compute edgels */
				pos = template[size-1] - 1;
				tp = template + size;
				for (r=size-1;r>=0;r--) {
					if (*--tp == 0)
						continue;
					if (pos+1 >= *tp)
						pos = *tp - 2;
					for (c=pos+1;c<*tp;c++)
						imageo[(rr+r)*nlpo+
						  rc+c] |= 1<<offset;
					pos = *tp - 1;
					if (pos == size-1)
						break;
				}
				break;
	/* light on right, edge moves leftward as row increases*/
			case 4:
					/* compute template */
				b4pp = b4 + (size-1)*(size+1) + bpos;
				pos = bpos;
				for (r=size-1;r>=0;r--) {
					b4p = b4pp - (size+1);
					max = - *b4p;
					bp = b4p;
					if (pos != size) {
						for (c=pos;c<=size;c++) {
							if (-*b4p>max) {
								pos = c;
								bp = b4p;
								max = - *b4p++;
							}
							else
								b4p++;
						}
					}
					*--tp = pos;
					b4pp = bp;
				}
					/* compute edgels */
				pos = template[0];
				tp = template - 1;
				for (r=0;r<size;r++) {
					if (*++tp == size)
						continue;
					if (pos-1<*tp)
						pos = *tp + 1;
					for (c=pos-1;c>=*tp;c--)
						imageo[(rr+r)*nlpo+
						  rc+c] |= 1<<offset;
					pos = *tp;
					if (pos == 0)
						break;
				}
				break;
			default:
				perr(HE_MSG,"type confusion!!");
			}
		}
	    }
	}
	return(HIPS_OK);
}
