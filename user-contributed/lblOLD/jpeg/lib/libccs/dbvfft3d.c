/*	DBVFFT_3D . C
#
%	Copyright (c)	Jin Guojun - All Rights are Reserved
%
%	The 1st dimension is shrinked to (m/2 + 1), buf
%	the 2nd & 3rd dimensions are doubled by complex format.
%
% AUTHOR:	Jin Guojun - LBL	5/1/91
*/

#include	"complex.h"
#include	"imagedef.h"

DBCOMPLEX	*DBcmpx_in, *DBcmpx_out;

void	DBwork_space_init(max_len)
{
static MType	space_len=0;

if (space_len < max_len)	{
	space_len = max_len;
	if (DBcmpx_in)	CFREE(DBcmpx_in);
	if (DBcmpx_out)	CFREE(DBcmpx_out);
	DBcmpx_in = (DBCOMPLEX*)NZALLOC(space_len, (MType)sizeof(*DBcmpx_in), "cmx_in");
	DBcmpx_out = (DBCOMPLEX*)NZALLOC(space_len, (MType)sizeof(*DBcmpx_out), "cmx_out");
}
}

double	*DBvfft2d(src, x, y, buf)
VType	*src;
unsigned x, y;
VType	*buf;
{
double	*in = (double *) src, *out = (double *) buf;
register unsigned	i, loop, n;
register DBCOMPLEX	*cmptr;

if (!x || load_DBw(x) == -1)	return	in;

if (x>1){
register int	len = ((x>>1) + 1) << 1;
    for (n=x, loop=0; loop<y;)	{
#ifdef	NON_CACHE_ARCH
    register double*	dp = in + loop * n;
	cmptr = DBcmpx_in;
	for (i=n; i--;) {
		c_re (cmptr[i]) = dp[i];
		c_im (cmptr[i]) = 0;
	}
#else
	cmptr = (DBCOMPLEX*)src + loop * n;
#endif
	i = len * loop++;
	if (loop == y)	{
		DBFourier(cmptr, n, DBcmpx_out);
		memcpy(out + i, DBcmpx_out, len * sizeof(*out));
	} else	DBFourier(cmptr, n, out + i);

#ifdef	_DEBUG_
	printcmpl(cmptr, n, loop, "FX");
#endif
    }
    in = buf;
}

if (y<2 || load_DBw(y) == -1)	return	in;
#ifdef	_DEBUG_
printsamp(in, x*y);	/* here, in=out=buf */
#endif

{
register int	dimen1len = (x>>1) + 1;/* 1st dimension is shorted to x/2 + 1 */

	for (n=y, loop=dimen1len; loop--;){
	register DBCOMPLEX *cp = (DBCOMPLEX*)in + loop; /* carefully convert */
		cmptr = DBcmpx_in;
		for (i = 0; i < n; i++, cp+=dimen1len)
			cmptr[i] = *cp;

		DBFourier(cmptr, n, DBcmpx_out);

		cp = (DBCOMPLEX*)out + loop;
		cmptr = DBcmpx_out;
		for (i=0; i < n; i++, cp+=dimen1len)
			*cp = cmptr[i];
    }
}
return	out;
}


double	*DBvrft2d(src, x, y, buf)
VType	*src;
unsigned x, y;
VType	*buf;
{
double	*in=(double *) src, *out=(double *) buf;
register unsigned	i, loop, n;
register DBCOMPLEX	*cmptr;
register int	dimension1 = (x>>1) + 1; /* for input only */

if (!x || load_DBw(y) == -1)	return	in;

if (y>1) {

   for (n=y, loop=dimension1; loop--;){
   register DBCOMPLEX	*cp = (DBCOMPLEX*)in + loop;
	cmptr = DBcmpx_in;
	for (i = 0; i < n; i++, cp+=dimension1)
		cmptr[i] = *cp;

	DBFourier(cmptr, n, DBcmpx_out);

	cp = (DBCOMPLEX*)in + loop;	/* back to src */
	cmptr = DBcmpx_out;
	for (i=0; i < n; i++, cp+=dimension1)
		*cp = cmptr[i];
#ifdef	_DEBUG_
	printcmpl(cmptr, n, loop, "RY");
#endif
    }
}

if (x<2 || load_DBw(x) == -1)	return	in;
#ifdef	_DEBUG_
printsamp(in, x*y);	/* here, in=src, out=buf */
#endif

for (n=x, loop=0; loop<y; loop++){
register DBCOMPLEX*	cp = (DBCOMPLEX*)in + loop*dimension1;
	cmptr = DBcmpx_in;
	cmptr [0] = cp[0];		/* dc */
	for (i=(n+1 >> 1); --i;) {	/* conj. symm. harmonischen */
		cmptr[i] = cp[i];
		c_re(cmptr[n-i]) = c_re(cp[i]);
		c_im(cmptr[n-i]) = -c_im(cp[i]);
	}
	if ((n & 1) == 0)		/* Nyquist */
		cmptr[n >> 1] = cp[n >> 1];
#ifdef	NON_CACHE_ARCH
	DBFourier(cmptr, n, DBcmpx_out);

	{
	register double	*dp = out + loop*n;
	    for (i=0, cmptr=DBcmpx_out; i < n; i++)
		dp[i] = c_re (cmptr [i]);
	}
#else
	DBFourier(cmptr, n, (DBCOMPLEX*)buf + loop*n);
#endif
#ifdef	_DEBUG_
	printcmpl(cmptr, n, loop, "RX");
#endif
}
#ifdef	_DEBUG_
printsamp(out, x*y);
#endif
return	out;
}

VType	*DBvfft3d(src, x, y, z, dst)
double	*src;
DBCOMPLEX	*dst;
{
register int	i, dimen1len = (x>>1) + 1, dimen2size=dimen1len * y;
int	fsize=x*y;

DBwork_space_init(MAX(MAX(x, y), z));
for (i=0; i<z; i++)
	DBvfft2d(src+i*fsize, x, y, dst+i*dimen2size);
if (z>1){
int	r;
register DBCOMPLEX	*cmptr, *cp;
register int	c, n=z;

load_DBw(z);

for (r=0; r<y; r++)
    for (c=0; c<dimen1len; c++){
	cp = dst + r*dimen1len+c;
	cmptr = DBcmpx_in;
	for (i=0; i < n; i++, cp+=dimen2size)
		cmptr[i] = *cp;

	DBFourier(cmptr, n, DBcmpx_out);

	cp = dst + r*dimen1len+c;	/* store back to dst */
	cmptr = DBcmpx_out;
	for (i = 0; i < n; i++, cp+=dimen2size)
		*cp = cmptr[i];
#ifdef	_DEBUG_
	printcmpl(cmptr, n, r, "FZ");
#endif
    }
}
return	(VType*)dst;
}

VType	*DBvrft3d(src, x, y, z, dst)
DBCOMPLEX	*src;
double	*dst;
{
register int	i, dimen1len=(x>>1)+1, dimen2size=dimen1len*y;
int	fsize=x*y;

if (z>1){
int	r;
register DBCOMPLEX	*cmptr, *cp;
register int	c, n=z;

DBwork_space_init(MAX(MAX(x, y), z));
load_DBw(n);

for (r=0; r<y; r++)
    for (c=0; c<dimen1len; c++){
	cp = src + r*dimen1len+c;
	cmptr = DBcmpx_in;
	for (i = 0; i < n; i++, cp+=dimen2size) {
		c_re (cmptr [i]) = c_re(*cp);
		c_im (cmptr [i]) = -c_im(*cp);
	}

	DBFourier(cmptr, n, DBcmpx_out);

	cp = src + r*dimen1len+c;	/* store back to inbuf */
	cmptr = DBcmpx_out;
	for (i = 0; i < n; i++, cp+=dimen2size)
		*cp = cmptr[i];
#ifdef	_DEBUG_
	printcmpl(cmptr, n, r, "FZ");
#endif
    }
}
for (i=0; i<z; i++)
	vrft2d(src + i*dimen2size, x, y, dst + i*fsize);
return	(VType*)dst;
}
