/*
 * The pyramid utilities are derived from code originally written by
 * Raj Hingorani at SRI/David Sarnoff Research Institute.  The original
 * Gaussian and Laplacian pyramid algorithms were designed by Peter Burt (also
 * currently at SRI/DSRC).  See:  Computer Graphics and Image Processing,
 * Volume 16, pp. 20-51, 1981, and IEEE Transactions on Communications,
 * Volume COM-31, pp. 532-540, 1983.
 *
 * new HIPS 2 routines - msl - 1/14/91
 */

/*
 * pyrcopy.c - image and pyramid copy routines
 */

#include <stdio.h>
#include <hipl_format.h>

int copy_itoii(hd,img)

struct header *hd;
IIMAGE img;

{
	register int **p,**fp,i,*ip,*iip,nc;
	int nr;

	ip = (int *) hd->image;
	nc = img.nc;
	nr = img.nr;
	for (p=img.ptr, fp=p+nr; p < fp; p++) {
		iip = *p;
		for (i=0;i<nc;i++)
			*iip++ = *ip++;
	}
	return(HIPS_OK);
}

int copy_ftoff(hd,img)

struct header *hd;
FIMAGE img;

{
	register int i,nc;
	register float **p,**fp,*ip,*iip;
	int nr;

	ip = (float *) hd->image;
	nc = img.nc;
	nr = img.nr;
	for (p=img.ptr, fp=p+nr; p < fp; p++) {
		iip = *p;
		for (i=0;i<nc;i++)
			*iip++ = *ip++;
	}
	return(HIPS_OK);
}
