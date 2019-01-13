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
 * pyrnumpix.c - calculate the number of pixels in a pyramid
 */

#include <hipl_format.h>

int pyrnumpix(toplev,nr,nc)

int toplev,nr,nc;

{
	int j,lev;

	j = nr*nc;
	if (toplev < 0)
		return(perr(HE_PYRTLZ));
	for (lev=1; lev <= toplev; lev++) {
		nr = (nr + 1)/2;
		nc = (nc + 1)/2;
		if (nr <= 0 || nc <= 0)
			return(perr(HE_PYRTL));
		j += nr*nc;
	}
	return(j);
}
