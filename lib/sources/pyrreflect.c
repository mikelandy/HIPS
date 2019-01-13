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
 * pyrreflect.c - reflection routines
 *
 * These routines fill the border areas around an image.  There are seven
 * ways of filling these areas, specified by the reflection type argument:
 *
 *	1	Even reflection:		c b | a b c d e | d c
 *	2	Wrap around (left=right):	c d | (a+e)/2 b c d (a+e)/2 | b
 *	3	Copy edge values:		a a | a b c d e | e e
 *	4	Odd reflection:			(2*a-c) (2*a-b) | a b c etc.
 *	5	Zero out border:		0 0 | a b c d e | 0 0
 *	6	Wrap around:			d e | a b c d e | a b
 *	7	Even reflection with repeat:	b a | a b c d e | e d
 */

#include <hipl_format.h>

int hor_reflectf(h,border,rtype)

FIMAGE h;
int border,rtype;

{
	register float **img = h.ptr;
	register int l=0,r=h.nc-1;
	register int j,m;
	float avg;

	if (rtype == 1)				/* even reflection */ 
	for (j=0; j < h.nr; j++)
		for (m=1; m <= border; m++) {
			img[j][l-m] = img[j][MIN(l+m,r)];
			img[j][r+m] = img[j][MAX(r-m,l)];
		}
	else if (rtype == 2)			/* wrap around (left=right) */
		for (j=0; j < h.nr; j++) {
			for (m=1; m <= border; m++) {
				img[j][l-m] = img[j][MAX(r-m,l)];
				img[j][r+m] = img[j][MIN(l+m,r)];
			}
			avg = (img[j][l] + img[j][r])/2; 
			img[j][l] = img[j][r] = avg;
		}
	else if (rtype == 3)			/* copy edge values */
		for (j=0; j < h.nr; j++)
			for (m=1; m <= border; m++) {
				img[j][l-m] = img[j][l];
				img[j][r+m] = img[j][r];
				;
			}
	else if (rtype == 4)			/* odd reflection */
		for (j=0; j < h.nr; j++)
			for (m=1; m <= border; m++) {
				img[j][l-m] = 2*img[j][l] - img[j][MIN(l+m,r)];
				img[j][r+m] = 2*img[j][r] - img[j][MAX(r-m,l)];
			}
	else if (rtype == 5)			/* zero out border */
		for (j=0; j < h.nr; j++)
			for (m=1; m <= border; m++) {
				img[j][l-m] = 0.0;
				img[j][r+m] = 0.0;
			}
	else if (rtype == 6)			/* wrap around */
		for (j=0; j < h.nr; j++)
			for (m=1; m <= border; m++) {
				img[j][l-m] = img[j][MAX(r-m+1,l)];
				img[j][r+m] = img[j][MIN(l+m-1,r)];
			}
	else if (rtype == 7)		/* even reflection with repeat*/
		for (j=0; j < h.nr; j++)
			for (m=1; m <= border; m++) {
				img[j][l-m] = img[j][MIN(l+m-1,r)];
				img[j][r+m] = img[j][MAX(r-m+1,l)];
			}
	else
		return(perr(HE_REFL,"hor_reflectf",rtype));
	return(HIPS_OK);
}

int ver_reflectf(v,border,rtype)

FIMAGE v;
int border,rtype;

{
	register float **img = v.ptr;
	register int b=0, t=v.nr-1;
	register int i,m;
	float avg;

	if (rtype == 1)				/* even reflection */
		for (m=1; m <= border; m++)
			for (i=0; i < v.nc; i++) {
				img[b-m][i] = img[MIN(b+m,t)][i];
				img[t+m][i] = img[MAX(t-m,b)][i];
			}
	else if (rtype == 2)			/* wrap around (bottom=top) */
		for (i=0; i < v.nc; i++) {
			for (m=1; m <= border; m++) {
				img[b-m][i] = img[MAX(t-m,b)][i];
				img[t+m][i] = img[MIN(b+m,t)][i];
			}
			avg = (img[b][i] + img[t][i])/2; 
			img[b][i] = img[t][i] = avg;
		}
	else if (rtype == 3)			/* copy edge values */
		for (m=1; m <= border; m++)
			for (i=0; i < v.nc; i++) {
				img[b-m][i] = img[b][i];
				img[t+m][i] = img[t][i];
			}
	else if (rtype == 4)			/* odd reflection */
		for (m=1; m <= border; m++)
			for (i=0; i < v.nc; i++) {
				img[b-m][i] = 2*img[b][i] - img[MIN(b+m,t)][i];
				img[t+m][i] = 2*img[t][i] - img[MAX(t-m,b)][i];
			}
	else if (rtype == 5)			/* zero out border */
		for (m=1; m <= border; m++)
			for (i=0; i < v.nc; i++) {
				img[b-m][i] = 0.0;
				img[t+m][i] = 0.0;
			}
	else if (rtype == 6)			/* wrap around */
		for (m=1; m <= border; m++)
			for (i=0; i < v.nc; i++) {
				img[b-m][i] = img[MAX(t-m+1,b)][i];
				img[t+m][i] = img[MIN(b+m-1,t)][i];
			}
	else if (rtype == 7)		/* even reflection with repeat*/
		for (m=1; m <= border; m++)
			for (i=0; i < v.nc; i++) {
				img[b-m][i] = img[MIN(b+m-1,t)][i];
				img[t+m][i] = img[MAX(t-m+1,b)][i];
			}
	else
		return(perr(HE_REFL,"ver_reflectf",rtype));
	return(HIPS_OK);
}

int reflectf(image,border,rtype)

FIMAGE image;
int border,rtype;

{
	register float **img = image.ptr;
	register int l=0,r=image.nc-1,b=0,t=image.nr-1;
	register int i,j,m,wl= -border, wr=image.nc+border;
	float avg;

	if (rtype == 1) {			/* even reflection */

		for (j=0; j < image.nr; j++)
			for (m=1; m <= border; m++) {
				img[j][l-m] = img[j][MIN(l+m,r)];
				img[j][r+m] = img[j][MAX(r-m,l)];
			}
		for (m=1; m <= border; m++)
			for (i=wl; i < wr; i++) {
				img[b-m][i] = img[MIN(b+m,t)][i];
				img[t+m][i] = img[MAX(t-m,b)][i];
			}
	} else if (rtype == 2) {		/* wrap around (left=right) */
		for (j=0; j < image.nr; j++) {
			for (m=1; m <= border; m++) {
				img[j][l-m] = img[j][MAX(r-m,l)];
				img[j][r+m] = img[j][MIN(l+m,r)];
			}
			avg = (img[j][l] + img[j][r])/2; 
			img[j][l] = img[j][r] = avg;
		}
		for (m=1; m <= border; m++) {
			for (i=wl; i < wr; i++) {
				img[b-m][i] = img[MAX(t-m,b)][i];
				img[t+m][i] = img[MIN(b+m,t)][i];
			}
			avg = (img[b][i] + img[t][i])/2; 
			img[b][i] = img[t][i] = avg;
		}
	} else if (rtype == 3) {		/* copy edge values */
		for (j=0; j < image.nr; j++)
			for (m=1; m <= border; m++) {
				img[j][l-m] = img[j][l];
				img[j][r+m] = img[j][r];
				;
			}
		for (m=1; m <= border; m++)
			for (i=wl; i < wr; i++) {
				img[b-m][i] = img[b][i];
				img[t+m][i] = img[t][i];
			}
	} else if (rtype == 4) {		/* odd reflection */
		for (j=0; j < image.nr; j++)
			for (m=1; m <= border; m++) {
				img[j][l-m] = 2*img[j][l] - img[j][MIN(l+m,r)];
				img[j][r+m] = 2*img[j][r] - img[j][MAX(r-m,l)];
			}
		for (m=1; m <= border; m++)
			for (i=wl; i < wr; i++) {
				img[b-m][i] = 2*img[b][i] - img[MIN(b+m,t)][i];
				img[t+m][i] = 2*img[t][i] - img[MAX(t-m,b)][i];
			}
	} else if (rtype == 5) {		/* zero out border */
		for (j=0; j < image.nr; j++)
			for (m=1; m <= border; m++) {
				img[j][l-m] = 0.0;
				img[j][r+m] = 0.0;
			}
		for (m=1; m <= border; m++)
			for (i=wl; i < wr; i++) {
				img[b-m][i] = 0.0;
				img[t+m][i] = 0.0;
			}
	} else if (rtype == 6) {		/* wrap around */
		for (j=0; j < image.nr; j++)
			for (m=1; m <= border; m++) {
				img[j][l-m] = img[j][MAX(r-m+1,l)];
				img[j][r+m] = img[j][MIN(l+m-1,r)];
			}
		for (m=1; m <= border; m++)
			for (i=wl; i < wr; i++) {
				img[b-m][i] = img[MAX(t-m+1,b)][i];
				img[t+m][i] = img[MIN(b+m-1,t)][i];
			}
	} else
		return(perr(HE_REFL,"reflectf",rtype));
	return(HIPS_OK);
}

int hor_reflecti(h,border,rtype)

IIMAGE h;
int border,rtype;

{
	register int **img = h.ptr;
	register int l=0,r=h.nc-1;
	register int j,m;
	float avg;

	if (rtype == 1)				/* even reflection */ 
	for (j=0; j < h.nr; j++)
		for (m=1; m <= border; m++) {
			img[j][l-m] = img[j][MIN(l+m,r)];
			img[j][r+m] = img[j][MAX(r-m,l)];
		}
	else if (rtype == 2)			/* wrap around (left=right) */
		for (j=0; j < h.nr; j++) {
			for (m=1; m <= border; m++) {
				img[j][l-m] = img[j][MAX(r-m,l)];
				img[j][r+m] = img[j][MIN(l+m,r)];
			}
			avg = (img[j][l] + img[j][r])/2; 
			img[j][l] = img[j][r] = avg;
		}
	else if (rtype == 3)			/* copy edge values */
		for (j=0; j < h.nr; j++)
			for (m=1; m <= border; m++) {
				img[j][l-m] = img[j][l];
				img[j][r+m] = img[j][r];
				;
			}
	else if (rtype == 4)			/* odd reflection */
		for (j=0; j < h.nr; j++)
			for (m=1; m <= border; m++) {
				img[j][l-m] = 2*img[j][l] - img[j][MIN(l+m,r)];
				img[j][r+m] = 2*img[j][r] - img[j][MAX(r-m,l)];
			}
	else if (rtype == 5)			/* zero out border */
		for (j=0; j < h.nr; j++)
			for (m=1; m <= border; m++) {
				img[j][l-m] = 0.0;
				img[j][r+m] = 0.0;
			}
	else if (rtype == 6)			/* wrap around */
		for (j=0; j < h.nr; j++)
			for (m=1; m <= border; m++) {
				img[j][l-m] = img[j][MAX(r-m+1,l)];
				img[j][r+m] = img[j][MIN(l+m-1,r)];
			}
	else if (rtype == 7)		/* even reflection with repeat*/
		for (j=0; j < h.nr; j++)
			for (m=1; m <= border; m++) {
				img[j][l-m] = img[j][MIN(l+m-1,r)];
				img[j][r+m] = img[j][MAX(r-m+1,l)];
			}
	else
		return(perr(HE_REFL,"hor_reflecti",rtype));
	return(HIPS_OK);
}

int ver_reflecti(v,border,rtype)

IIMAGE v;
int border,rtype;

{
	register int **img = v.ptr;
	register int b=0, t=v.nr-1;
	register int i,m;
	float avg;

	if (rtype == 1)				/* even reflection */
		for (m=1; m <= border; m++)
			for (i=0; i < v.nc; i++) {
				img[b-m][i] = img[MIN(b+m,t)][i];
				img[t+m][i] = img[MAX(t-m,b)][i];
			}
	else if (rtype == 2)			/* wrap around (bottom=top) */
		for (i=0; i < v.nc; i++) {
			for (m=1; m <= border; m++) {
				img[b-m][i] = img[MAX(t-m,b)][i];
				img[t+m][i] = img[MIN(b+m,t)][i];
			}
			avg = (img[b][i] + img[t][i])/2; 
			img[b][i] = img[t][i] = avg;
		}
	else if (rtype == 3)			/* copy edge values */
		for (m=1; m <= border; m++)
			for (i=0; i < v.nc; i++) {
				img[b-m][i] = img[b][i];
				img[t+m][i] = img[t][i];
			}
	else if (rtype == 4)			/* odd reflection */
		for (m=1; m <= border; m++)
			for (i=0; i < v.nc; i++) {
				img[b-m][i] = 2*img[b][i] - img[MIN(b+m,t)][i];
				img[t+m][i] = 2*img[t][i] - img[MAX(t-m,b)][i];
			}
	else if (rtype == 5)			/* zero out border */
		for (m=1; m <= border; m++)
			for (i=0; i < v.nc; i++) {
				img[b-m][i] = 0.0;
				img[t+m][i] = 0.0;
			}
	else if (rtype == 6)			/* wrap around */
		for (m=1; m <= border; m++)
			for (i=0; i < v.nc; i++) {
				img[b-m][i] = img[MAX(t-m+1,b)][i];
				img[t+m][i] = img[MIN(b+m-1,t)][i];
			}
	else if (rtype == 7)		/* even reflection with repeat*/
		for (m=1; m <= border; m++)
			for (i=0; i < v.nc; i++) {
				img[b-m][i] = img[MIN(b+m-1,t)][i];
				img[t+m][i] = img[MAX(t-m+1,b)][i];
			}
	else
		return(perr(HE_REFL,"ver_reflecti",rtype));
	return(HIPS_OK);
}

int reflecti(image,border,rtype)

IIMAGE image;
int border,rtype;

{
	register int **img = image.ptr;
	register int l=0,r=image.nc-1,b=0,t=image.nr-1;
	register int i,j,m,wl= -border, wr=image.nc+border;
	float avg;

	if (rtype == 1) {			/* even reflection */

		for (j=0; j < image.nr; j++)
			for (m=1; m <= border; m++) {
				img[j][l-m] = img[j][MIN(l+m,r)];
				img[j][r+m] = img[j][MAX(r-m,l)];
			}
		for (m=1; m <= border; m++)
			for (i=wl; i < wr; i++) {
				img[b-m][i] = img[MIN(b+m,t)][i];
				img[t+m][i] = img[MAX(t-m,b)][i];
			}
	} else if (rtype == 2) {		/* wrap around (left=right) */
		for (j=0; j < image.nr; j++) {
			for (m=1; m <= border; m++) {
				img[j][l-m] = img[j][MAX(r-m,l)];
				img[j][r+m] = img[j][MIN(l+m,r)];
			}
			avg = (img[j][l] + img[j][r])/2; 
			img[j][l] = img[j][r] = avg;
		}
		for (m=1; m <= border; m++) {
			for (i=wl; i < wr; i++) {
				img[b-m][i] = img[MAX(t-m,b)][i];
				img[t+m][i] = img[MIN(b+m,t)][i];
			}
			avg = (img[b][i] + img[t][i])/2; 
			img[b][i] = img[t][i] = avg;
		}
	} else if (rtype == 3) {		/* copy edge values */
		for (j=0; j < image.nr; j++)
			for (m=1; m <= border; m++) {
				img[j][l-m] = img[j][l];
				img[j][r+m] = img[j][r];
				;
			}
		for (m=1; m <= border; m++)
			for (i=wl; i < wr; i++) {
				img[b-m][i] = img[b][i];
				img[t+m][i] = img[t][i];
			}
	} else if (rtype == 4) {		/* odd reflection */
		for (j=0; j < image.nr; j++)
			for (m=1; m <= border; m++) {
				img[j][l-m] = 2*img[j][l] - img[j][MIN(l+m,r)];
				img[j][r+m] = 2*img[j][r] - img[j][MAX(r-m,l)];
			}
		for (m=1; m <= border; m++)
			for (i=wl; i < wr; i++) {
				img[b-m][i] = 2*img[b][i] - img[MIN(b+m,t)][i];
				img[t+m][i] = 2*img[t][i] - img[MAX(t-m,b)][i];
			}
	} else if (rtype == 5) {		/* zero out border */
		for (j=0; j < image.nr; j++)
			for (m=1; m <= border; m++) {
				img[j][l-m] = 0.0;
				img[j][r+m] = 0.0;
			}
		for (m=1; m <= border; m++)
			for (i=wl; i < wr; i++) {
				img[b-m][i] = 0.0;
				img[t+m][i] = 0.0;
			}
	} else if (rtype == 6) {		/* wrap around */
		for (j=0; j < image.nr; j++)
			for (m=1; m <= border; m++) {
				img[j][l-m] = img[j][MAX(r-m+1,l)];
				img[j][r+m] = img[j][MIN(l+m-1,r)];
			}
		for (m=1; m <= border; m++)
			for (i=wl; i < wr; i++) {
				img[b-m][i] = img[MAX(t-m+1,b)][i];
				img[t+m][i] = img[MIN(b+m-1,t)][i];
			}
	} else
		return(perr(HE_REFL,"reflecti",rtype));
	return(HIPS_OK);
}
