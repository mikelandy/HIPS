/*
 * "fourier.c"
 */

#include	"complex.h"
#include	"stdef.h"
#include	<math.h>
#define pi_2	pi * 2

COMPLEX*	W_factors;		/* array of W-factors */
unsigned	Nfactors;		/* number of entries in W-factors */

/*
 * load_w puts Wn ^ k (= e ^ (2pi * i * k / n)) in W_factors [k], 0 <= k < n.
 * If n is equal to Nfactors then nothing is done, so the same W_factors
 * array can used for several transforms of the same number of samples.
 * Notice the explicit calculation of sines and cosines, an iterative approach
 * introduces substantial errors.
 */
load_w(n)
unsigned n;
{
register unsigned	k;
register FType	scale = pi_2 / n;

	if (n == Nfactors)
		return	0;
	if (Nfactors != 0 && W_factors != 0)
		CFREE((char *) W_factors);
	if ((Nfactors=n) == 0)
		return 0;
	if ((W_factors = (COMPLEX *) NZALLOC(n, sizeof(COMPLEX), 0)) == 0)
		return -1;
	{
	register FType	pos=0;
	    for (k=0; k < n; k++) {
		c_re(W_factors[k]) = cos(pos);
		c_im(W_factors[k]) = sin(pos);
		pos += scale;
		}
	}
return 0;
}

/*
 * Give smallest possible radix for n samples.
 * Determines (in a rude way) the smallest primefactor of n.
 */
static unsigned radix(n)
register unsigned	n;
{
register unsigned	r=2, q;

	if (n < r)	return 1;

	if (n & 1)	{
		q = n << 1 + (n>16);
		while (++r < q && n % r);
		if (r == q)	r = n;
	}
	return	r;
}

/*
 * Split array in of r * m samples in r parts of each m samples,
 * such that in [i] goes to out [(i % r) * m + (i / r)].
 * Then call for each part of out Fourier, so the r recursively
 * transformed parts will go back to in.
 */
static	split(in, r, m, out)
COMPLEX *in;
register unsigned r, m;
COMPLEX *out;
{
	register unsigned k, s, i, j;

	for (k = j = 0; k < r; k++)
		for (s = 0, i = k; s < m; s++, i += r, j++)
			out [j] = in [i];

	for (k = 0; k < r; k++, out += m, in += m)
		Fourier (out, m, in);
}

/*
 * Sum the n / m parts of each m samples of in to n samples in out.
 * 		   r - 1
 * Out [j] becomes  sum  in [j % m] * W (j * k).  Here in is the k-th
 * 		   k = 0   k	       n		 k
 * part of in (indices k * m ... (k + 1) * m - 1), and r is the radix.
 * For k = 0, a complex multiplication with W (0) is avoided.
 */
static	join(in, m, n, out)
register COMPLEX	*in;
register unsigned	m, n;
register COMPLEX	*out;
{
register unsigned i, j, jk, s;

	for (s = 0; s < m; s++)
	    for (j = s; j < n; j += m) {
		COMPLEX	C1, C2;
		out [j] = in [s];
		for (i = s + m, jk = j; i < n; i += m, jk += j)	{
			C1 = in[i],	C2 = W (n, jk);
			c_re(out[j]) += C1.re * C2.re - C1.im * C2.im;
			c_im(out[j]) += C1.re * C2.im + C1.im * C2.re;
		}
	    }
}

/*
 * Recursive (reverse) complex fast Fourier transform on the n
 * complex samples of array in, with the Cooley-Tukey method.
 * The result is placed in out.  The number of samples, n, is arbitrary.
 * The algorithm costs O (n * (r1 + .. + rk)), where k is the number
 * of factors in the prime-decomposition of n (also the maximum
 * depth of the recursion), and ri is the i-th primefactor.
 */
Fourier(in, n, out)
register COMPLEX	*in;
register unsigned	n;
register COMPLEX	*out;
{
register unsigned	r;
unsigned radix ();

	if ((r = radix(n)) < n)
		split(in, r, n / r, out);
	join(in, n / r, n, out);
}

/*
 * "Double Fourier.c"
 */

static DBCOMPLEX*	DBW_factors;		/* array of DBW-factors */
static unsigned	DBNfactors;		/* number of entries in DBW-factors */

/*
 * load_w puts Wn ^ k (= e ^ (2pi * i * k / n)) in DBW_factors [k], 0 <= k < n.
 * If n is equal to DBNfactors then nothing is done, so the same DBW_factors
 * array can used for several transforms of the same number of samples.
 * Notice the explicit calculation of sines and cosines, an iterative approach
 * introduces substantial errors.
 */
load_DBw(n)
unsigned n;
{
register unsigned	k;
register double	scale = pi_2 / n;

	if (n == DBNfactors)
		return 0;
	if (DBNfactors != 0 && DBW_factors != 0)
		CFREE((char *) DBW_factors);
	if ((DBNfactors = n) == 0)
		return 0;
	if (!(DBW_factors = (DBCOMPLEX *) NZALLOC(n, sizeof(DBCOMPLEX), 0)))
		return -1;
	{
	register double	pos=0;
		for (k=0; k < n; k++) {
			c_re(DBW_factors[k]) = cos(pos);
			c_im(DBW_factors[k]) = sin(pos);
			pos += scale;
		}
	}
return 0;
}

DBFourier(in, n, out)
register DBCOMPLEX	*in;
register unsigned	n;
register DBCOMPLEX	*out;
{
register unsigned	r;
unsigned	radix();

	if ((r = radix(n)) < n)
		DBsplit (in, r, n / r, out);
	DBjoin(in, n / r, n, out);
}

static	DBsplit(in, r, m, out)
DBCOMPLEX *in;
register unsigned r, m;
DBCOMPLEX *out;
{
register unsigned k, s, i, j;

	for (k=j=0; k < r; k++)
		for (s=0, i=k; s < m; s++, i += r, j++)
			out[j] = in[i];

	for (k = 0; k < r; k++, out += m, in += m)
		DBFourier(out, m, in);
}

static	DBjoin(in, m, n, out)
register DBCOMPLEX	*in;
register unsigned	m, n;
register DBCOMPLEX	*out;
{
register unsigned i, j, jk, s;

	for (s = 0; s < m; s++)
	    for (j = s; j < n; j += m) {
		DBCOMPLEX	C1, C2;
		out[j] = in[s];
		for (i = s + m, jk = j; i < n; i += m, jk += j)	{
			C1 = in[i],	C2 = DBW(n, jk);
			c_re(out[j]) += C1.re * C2.re - C1.im * C2.im;
			c_im(out[j]) += C1.re * C2.im + C1.im * C2.re;
		}
	    }
}
