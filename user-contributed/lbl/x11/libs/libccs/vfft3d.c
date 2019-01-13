/*	VFFT_3D . C
#
%	Copyright (c)	Jin Guojun -	All rights reserved
%
%	The 1st dimension is shrinked to (m/2 + 1), but
%	the 2nd & 3rd dimensions are doubled by floating point complex format.
%	The DBVFFT3D handles the DOUBLE COMPLEX VFFT transform.
%
%	This library handles pure image with float format
%
% AUTHOR:	Jin Guojun - LBL	5/1/91
*/

#include	"complex.h"
#include	"imagedef.h"

COMPLEX	*cmpx_in, *cmpx_out;

void	work_space_init(max_len)
{
static MType	space_len=0;

if (space_len < max_len){
	space_len = max_len;
	if (cmpx_in)	CFREE(cmpx_in);
	if (cmpx_out)	CFREE(cmpx_out);
	cmpx_in = (COMPLEX*)NZALLOC(space_len, (MType)sizeof(*cmpx_in), "cmx_in");
	cmpx_out = (COMPLEX*)NZALLOC(space_len, (MType)sizeof(*cmpx_out), "cmx_out");
}
}


FType	*vfft2d(src, x, y, buf)
VType	*src;
unsigned x, y;
VType	*buf;
{
FType	*in = (FType *)src, *out = (FType *)buf;
register unsigned	i, loop, n;
register COMPLEX	*cmptr;

if (!x || load_w(x) == -1)	return	in;

if (x>1){
register int	len = ((x>>1) + 1) << 1;
    for (n=x, loop=0; loop<y;)	{
#ifdef	NON_CACHE_ARCH
    register FType*	dp = in + loop * n;
	cmptr = cmpx_in;
	for (i=n; i--;) {
		c_re (cmptr[i]) = dp[i];
		c_im (cmptr[i]) = 0;
	}
#else
	cmptr = (COMPLEX*)src + loop * n;
#endif
	i = len * loop++;
	if (loop == y)	{
		Fourier(cmptr, n, cmpx_out);
		memcpy(out + i, cmpx_out, len * sizeof(*out));
	} else	Fourier(cmptr, n, out + i);

#ifdef	_DEBUG_
	printcmpl(cmptr, n, loop, "FX");
#endif
    }
}
else	{
	cmptr = (COMPLEX*) out;
	for (i=y; i--;)
		c_re(cmptr[i]) = in[i],
		c_im(cmptr[i]) = 0;
}
in = out;

if (y<2 || load_w(y) == -1)	return	in;
#ifdef	_DEBUG_
printsamp(in, x*y);
#endif

{
register int	dimen1len = (x>>1) + 1;/* 1st dimension is shorted to x/2 + 1 */

	for (n=y, loop=dimen1len; loop--;)	{
	register COMPLEX *cp = (COMPLEX*)in + loop; /* carefully convert */
		cmptr = cmpx_in;
		for (i=0; i < n; i++, cp+=dimen1len)
			cmptr[i] = *cp;

		Fourier(cmptr, n, cmpx_out);

		cp = (COMPLEX*)out + loop;
		cmptr = cmpx_out;
		for (i=0; i < n; i++, cp+=dimen1len)
			*cp = cmptr[i];	/* expensive mcopy	*/
    }
}
return	out;
}


FType	*vrft2d(src, x, y, buf)
VType	*src;
unsigned x, y;
VType	*buf;
{
FType	*in=(FType*)src, *out=(FType*)buf;
register unsigned	i, loop, n;
register COMPLEX	*cmptr;
register int	dimension1 = (x>>1) + 1; /* for input only */

if (!x || load_w(y) == -1)	return	in;

if (y > 1)	{

   for (n=y, loop=dimension1; loop--;){
   register COMPLEX	*cp = (COMPLEX*)in + loop;
	cmptr = cmpx_in;
	for (i=0; i < n; i++, cp+=dimension1)
		cmptr[i] = *cp;

	Fourier(cmptr, n, cmpx_out);

	cp = (COMPLEX*)in + loop;	/* back to src */
	cmptr = cmpx_out;
	for (i=0; i < n; i++, cp+=dimension1)
		*cp = cmptr[i];
#ifdef	_DEBUG_
	printcmpl(cmptr, n, loop, "RY");
#endif
    }
}

if (x<2 || load_w(x) == -1)	return	in;
#ifdef	_DEBUG_
printsamp(in, x*y);	/* here, in=src, out=buf */
#endif

for (n=x, loop=0; loop<y; loop++){
register COMPLEX*	cp = (COMPLEX*)in + loop*dimension1;
	cmptr = cmpx_in;
	cmptr [0] = cp[0];		/* dc */
	for (i=(n+1 >> 1); --i;) {	/* conj. symm. harmonischen */
		cmptr[i] = cp[i];
		c_re(cmptr[n-i]) = c_re(cp[i]);
		c_im(cmptr[n-i]) = -c_im(cp[i]);
	}
	if ((n & 1) == 0)		/* Nyquist */
		cmptr[n >> 1] = cp[n >> 1];
#ifdef	NON_CACHE_ARCH
	Fourier(cmptr, n, cmpx_out);

	{
	register FType	*dp = out + loop*n;
	    for (i=0, cmptr=cmpx_out; i < n; i++)
		dp[i] = c_re (cmptr [i]);
	}
#else
	Fourier(cmptr, n, (COMPLEX*)buf + loop*n);
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


VType	*vfft3d(src, x, y, z, dst)
FType	*src;
COMPLEX	*dst;
{
register int	i, dimen1len = (x>>1) + 1, dimen2size=dimen1len * y;
int	fsize=x*y;

work_space_init(MAX(MAX(x, y), z));
for (i=0; i<z; i++)
	vfft2d(src+i*fsize, x, y, dst+i*dimen2size);
if (z>1){
int	r;
register COMPLEX	*cmptr, *cp;
register int	c, n=z;

load_w(z);

for (r=0; r<y; r++)
    for (c=0; c<dimen1len; c++){
	cp = dst + r*dimen1len+c;
	cmptr = cmpx_in;
	for (i = 0; i < n; i++, cp+=dimen2size)
		cmptr[i] = *cp;

	Fourier(cmptr, n, cmpx_out);

	cp = dst + r*dimen1len+c;	/* store back to dst */
	cmptr = cmpx_out;
	for (i = 0; i < n; i++, cp+=dimen2size)
		*cp = cmptr[i];
#ifdef	_DEBUG_
	printcmpl(cmptr, n, r, "FZ");
#endif
    }
}
return	(VType*)dst;
}

VType	*vrft3d(src, x, y, z, dst)
COMPLEX	*src;
FType	*dst;
{
register int	i, dimen1len=(x>>1)+1, dimen2size=dimen1len*y;
int	fsize=x*y;

if (z>1){
int	r;
register COMPLEX	*cmptr, *cp;
register int	c, n=z;

work_space_init(MAX(MAX(x, y), z));
load_w(n);

for (r=0; r<y; r++)
    for (c=0; c<dimen1len; c++){
	cp = src + r*dimen1len+c;
	cmptr = cmpx_in;
	for (i = 0; i < n; i++, cp+=dimen2size) {
		c_re (cmptr [i]) = c_re(*cp);
		c_im (cmptr [i]) = -c_im(*cp);
	}

	Fourier(cmptr, n, cmpx_out);

	cp = src + r*dimen1len+c;	/* store back to inbuf */
	cmptr = cmpx_out;
	for (i=0; i < n; i++, cp+=dimen2size)
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
