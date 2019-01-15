/*
	COMPLEX . H
*/

#ifndef	FType
#define	FType	float
#endif

#ifndef	COMPLEX_H
#define	COMPLEX_H

#define pi	3.1415926535897932384626434

typedef struct {
	FType	re, im;
	} COMPLEX;

typedef struct {
	double	re, im;
	} DBCOMPLEX;

#define		c_re(c)		((c).re)
#define		c_im(c)		((c).im)

/*
 * C_conj substitutes c by its complex conjugate.
 */
#define c_conj(c)		{ c_im (c) = -c_im (c); }

#define	c_realdiv(c, real)	{ c_re (c) /= (real); c_im (c) /= (real); }

extern DBCOMPLEX *DBW_factors, *DBcmpx_in, *DBcmpx_out;
extern COMPLEX *W_factors, *cmpx_in, *cmpx_out;
extern unsigned DBNfactors, Nfactors;

/*
 * W gives the (already computed) Wn ^ k (= e ^ (2pi * i * k / n)).
 * Notice that the powerseries of Wn has period Nfactors.
 */
#define	DBW(n, k)	(DBW_factors [((k) * (DBNfactors / (n))) % DBNfactors])
#define	W(n, k)		(W_factors [((k) * (Nfactors / (n))) % Nfactors])

#endif	COMPLEX_H
