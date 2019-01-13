/*
%	vfft_2p -- virtual fast fourier transform by size power of 2.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

This software is copyright (C) by the Lawrence Berkeley Laboratory.
Permission is granted to reproduce this software for non-commercial
purposes provided that this notice is left intact.

It is acknowledged that the U.S. Government has rights to this software
under Contract DE-AC03-765F00098 between the U.S.  Department of Energy
and the University of California.

This software is provided as a professional and academic contribution
for joint exchange. Thus, it is experimental, and is provided ``as is'',
with no warranties of any kind whatsoever, no support, no promise of
updates, or printed documentation. By using this software, you
acknowledge that the Lawrence Berkeley Laboratory and Regents of the
University of California shall have no liability with respect to the
infringement of other copyrights by any part of this software.

For further information about this notice, contact William Johnston,
Bld. 50B, Rm. 2239, Lawrence Berkeley Laboratory, Berkeley, CA, 94720.
(wejohnston@lbl.gov)

For further information about this software, contact:
	Jin Guojun
	Bld. 50B, Rm. 2275, Lawrence Berkeley Laboratory, Berkeley, CA, 94720
	g_jin@lbl.gov

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% Note:	No division by total_size is performed and no multiply 2 used so that
%	this median transform is no a standard transform. It is only used for
%	applying filter on frequency domain and transfer back to spatial
%	domain (this is done by performing conjugate on filtered transform,
%	and applying vfft again and dividing by N = size to get final result.
%	The DCVTOB can convert vfft to regular fft format.
%	This version is much faster than vfft_2p.c.pln on different
%	cache controller.
%		Sparc-2 (10/20)	:	40% (24%)
%		Alpha / SGI	:	35%
%
% Timing Consuming:	real (ARCH) - user + sys time	[times	] <FileSys>
%			real - (user + sys) = io time
%
%	256 x 256 :	2 sec (Sun4 sparc 2, SunOS 4.1.3)	<LFS>
%			12 sec (Sun4 280) - 11 sec	[14.0u 0.6s] <LFS>
%	256 x 256 x 8:	29 sec (sparc 2)  - 14 sec	[12.9u 1.0s]
%			13 sec (sparc 2)  - 13 sec	[12.8u 0.7s] <LFS>
%		3D VFFT
%	256 x 256 x 8:	35 sec (sparc 2)  - 15.9 sec	[14.6u 1.3s]
%			17 sec (sparc 2)  - 15.3 sec	[14.5u 0.8s] <LFS>
%			12 sec (sparc 20) - 4 sec	[4u 0.0s]
%			5 sec (sparc 20) - 4 sec	[4u 0.0s] <LFS>
%	256 x 256 x 128:	fourtr3d - 217.0u 29.0s 18:46 21% <LFS>
%			152 sec (sparc 20) - 106 sec	[101u 5s] <LFS>
%
%
% Routines:
%
% FType	*re_plane, *im_plane;
% int	loglen, skip;
%
% vfft_3d(cplx, logfrms, logrows, logcols)
%	performs a 3-dimensional vfft
%
% vfft_2d(cplx, logrows, logcols)
%	performs a 2-dimensional vfft where logrows is the log of the number of
%	rows, and logcols is the log of the number of columns
%
% vfftn(cplx, loglen, skip)
%	performs a 1-dimensional vfft on every skip-th entry
% vfftn(cplx_in, loglen, skip, cplx_out, vskip)	changed on 1/6/95
%
% vfftr(cplx, loglen)	add on 12/2/94
%	performs a 1-dimensional vfft on a row (regular)
%
%
%	for double format, put letter 'd' before routine name.
%	example:
%		dfft_2d(...)
%
% 2/16/1995:	add (d)wr_xxx for multi-threads
%
% AUTHOR:	Jin Guojun - Lawrence Berkelry Laboratory	5/1/91
*/

#include <math.h>
#include "imagedef.h"
#include "complex.h"


float	*w_re,*w_im, *wr_re, *wr_im;

w_init(halfcol, halfrow)
MType	halfcol, halfrow;
{
	wr_re = (float*)NZALLOC(halfcol, (MType)sizeof(*w_re), "wrc");
	wr_im = (float*)NZALLOC(halfcol, (MType)sizeof(*w_im), "wri");
	w_re = (float*)NZALLOC(halfrow, (MType)sizeof(*w_re), "wc");
	w_im = (float*)NZALLOC(halfrow, (MType)sizeof(*w_im), "wi");
}

wr_load(n)
{
int	nv2 = n>>1;
register float	wr, wi;
register int	i;
static int	constsize;
	if (constsize != nv2) {
		constsize = nv2;
		wr =  cos(2*M_PI/n);
		wi = -sin(2*M_PI/n);
		wr_re[0] = 1.;
		wr_im[0] = 0.;
		for (i=1; i<nv2; i++) {
			wr_re[i] = wr*wr_re[i-1] - wi*wr_im[i-1];
			wr_im[i] = wr*wr_im[i-1] + wi*wr_re[i-1];
		}
	}
}

w_load(n)
{
int	nv2 = n>>1;
register float	wr, wi;
register int	i;
static int	constsize;
	if (constsize != nv2) {
		constsize = nv2;
		wr =  cos(2*M_PI/n);
		wi = -sin(2*M_PI/n);
		w_re[0] = 1.;
		w_im[0] = 0.;
		for (i=1; i<nv2; i++) {
			w_re[i] = wr*w_re[i-1] - wi*w_im[i-1];
			w_im[i] = wr*w_im[i-1] + wi*w_re[i-1];
		}
	}
}


vfft(cplx, loglen)
COMPLEX	*cplx;
int	loglen;
{
	wr_load(1<<loglen);
	vfftr(cplx, loglen);
}

vfft_2d(cplx_in, logrows, logcols, cplx_out)
COMPLEX	*cplx_in, *cplx_out;
int	logrows, logcols;
{
register int	i;
int	rows = 1<<logrows,
	cols = 1<<logcols,
	size = rows * cols;
	if (!cplx_out)
		cplx_out = cplx_in;

	wr_load(cols);
	for (i=0; i<size; i+=cols)
		vfftr(cplx_in+i, logcols);
#ifdef	_DEBUG_
	CONSUMED_TIME("\n1D VFFT");
#endif
	w_load(rows);
	i = (cols>>1) + 1;
	for (size = cplx_in==cplx_out ? cols : i; i--;)
		vfftn(cplx_in+i, logrows, cols, cplx_out+i, size);
#ifdef	_DEBUG_
	CONSUMED_TIME("\n2D VFFT");
#endif
}

vrft_2d(cplx_in, logrows, logcols, cplx_out)
COMPLEX	*cplx_in, *cplx_out;
int	logrows, logcols;
{
int	rows = 1<<logrows,
	cols = 1<<logcols,
	size;
register int	i = (cols>>1) + 1;
	if (!cplx_out)
		cplx_out = cplx_in;

	w_load(rows);
	for (size = cplx_in==cplx_out ? cols : i; i--;)
		vfftn(cplx_in+i, logrows, size, cplx_out+i, cols);
	if (cols > 1)
	for (i=0; i<rows; i++)	{
	register int	j;
	register COMPLEX*	cplx1 = cplx_out + i*cols;
	    for (j=cols>>1; --j;)	{
		cplx1[cols-j].re = cplx1[j].re;
		cplx1[cols-j].im = -cplx1[j].im;
	    }
	}
	size = rows * cols;
	wr_load(cols);
	for (i=0; i<size; i+=cols)
		vfftr(cplx_out+i, logcols);
}

vfftr(cplx, loglen)
COMPLEX	*cplx;
int	loglen;
{
if(loglen)	{
register int	i;
int	n, nv2, nm1, j, k, l, le, le1, c, nle;
float	tr, ti;
register COMPLEX	*x, *y;

	n=1<<loglen;
	nv2=n >> 1; nm1=n-1;

	for (i=1, j=nv2; i<nm1; i++) {
	register COMPLEX	tmp;
		if(i < j) {	/* do swapping */
			x = cplx+i;	y = cplx+j;
			tmp = *y;	*y = *x;	*x = tmp;
		}
		k = nv2;
		while (k <= j) {
			j -= k; k >>= 1;
		}
		j += k;
	}

	le = 1;
	for (l=0; l<loglen; l++) {
		le1 = le;
		le += le;
		nle = n/le;
		for (c=j=0; j<le1; j++) {
		    for (i=j; i<n; i+=le) {
#ifdef	_DEBUG_
			if(i+le1 >= n)
				syserr("vfftr: strange index=%d",i+le1);
#endif
			x=cplx+i; y=cplx+(i+le1);

			if (c==0) {
				tr = y->re;
				ti = y->im;
			}
			else {
			    tr = y->re*wr_re[c] - y->im*wr_im[c];
			    ti = y->re*wr_im[c] + y->im*wr_re[c];
			}
			y->re = x->re - tr;
			y->im = x->im - ti;

			x->re += tr;
			x->im += ti;
		    }
		    c += nle;
		}
	}
}
}

vfftn(cplx_in, loglen, nskip, cplx_out, vskip)
COMPLEX	*cplx_in, *cplx_out;
int	loglen;
register int   nskip;
{
register int	i;
int	n, nv2, nm1, j, k, l, le, le1, c, nle;
float	tr, ti;
register COMPLEX	*x, *y;


    if(loglen)	{
	n=1<<loglen;
	nv2=n >> 1; nm1=n-1;

	if (cplx_in == cplx_out) for (i=1, j=nv2; i<nm1; i++) {
		if(i < j) {	/* do swapping */
		register COMPLEX	tmp;
			x = cplx_in+i*nskip;	y = cplx_in+j*nskip;
			tmp = *x;	*x = *y;	*y = tmp;
		}
		k = nv2;
		while (k <= j) {
			j -= k; k >>= 1;
		}
		j += k;
	}
	else	{
		cplx_out[0] = cplx_in[0];
		cplx_out[nm1*vskip] = cplx_in[nm1*nskip];
	    for (i=1, j=nv2; i<nm1; i++) {
		l = i*vskip;
		if(i < j) {
			cplx_out[l] = cplx_in[j*nskip];
			l = j*vskip;
			cplx_out[l] = cplx_in[i*nskip];
		} else if (i == j)
			cplx_out[l] = cplx_in[i*nskip];
		k = nv2;
		while (k <= j) {
			j -= k; k >>= 1;
		}
		j += k;
	    }	nskip = vskip;
	}

	le = 1;
	for (l=0; l<loglen; l++) {
		le1 = le;
		le += le;
		nle = n/le;
		for (c=j=0; j<le1; j++) {
		    for (i=j; i<n; i+=le) {
			if(i+le1 >= n)
				syserr("vfftn: strange index=%d",i+le1);
			x=cplx_out+i*nskip;	y=cplx_out+(i+le1)*nskip;

			if (c==0) {
				tr = y->re;
				ti = y->im;
			}
			else {
			    tr = y->re*w_re[c] - y->im*w_im[c];
			    ti = y->re*w_im[c] + y->im*w_re[c];
			}
			y->re = x->re - tr;
			y->im = x->im - ti;

			x->re += tr;
			x->im += ti;
		    }
		    c += nle;
		}
	}
    }
}



/*=======================================================================
%	below is same routines bu for double (8 bytes) processing	%
=======================================================================*/

double	*dw_re,*dw_im, *dwr_re, *dwr_im;

dw_init(halfcol, halfrow)
MType	halfcol, halfrow;
{
	dwr_re = (double*)NZALLOC(halfcol, (MType)sizeof(*dw_re), "dwrc");
	dwr_im = (double*)NZALLOC(halfcol, (MType)sizeof(*dw_im), "dwri");
	dw_re = (double*)NZALLOC(halfrow, (MType)sizeof(*dw_re), "dwc");
	dw_im = (double*)NZALLOC(halfrow, (MType)sizeof(*dw_im), "dwi");
}

dwr_load(n)
{
int	nv2 = n >> 1;
register double	dwr, dwi;
register int	i;
static int	constsize;
	if (constsize != nv2) {
		constsize = nv2;
		dwr =  cos(2*M_PI/n);
		dwi = -sin(2*M_PI/n);
		dwr_re[0] = 1.;
		dwr_im[0] = 0.;
		for (i=1; i<nv2; i++) {
			dwr_re[i] = dwr*dwr_re[i-1] - dwi*dwr_im[i-1];
			dwr_im[i] = dwr*dwr_im[i-1] + dwi*dwr_re[i-1];
		}
	}
}

dw_load(n)
{
int	nv2 = n >> 1;
register double	dwr, dwi;
register int	i;
static int	constsize=0;
	if (constsize != nv2) {
		constsize = nv2;
		dwr =  cos(2*M_PI/n);
		dwi = -sin(2*M_PI/n);
		dw_re[0] = 1.;
		dw_im[0] = 0.;
		for (i=1; i<nv2; i++) {
			dw_re[i] = dwr*dw_re[i-1] - dwi*dw_im[i-1];
			dw_im[i] = dwr*dw_im[i-1] + dwi*dw_re[i-1];
		}
	}
}


dvfft(cplx, loglen)
DBCOMPLEX	*cplx;
int	loglen;
{
	dwr_load(1<<loglen);
	dvfftr(cplx, loglen);
}

dvfft_2d(cplx_in, logrows, logcols, cplx_out)
DBCOMPLEX	*cplx_in, *cplx_out;
int	logrows, logcols;
{
register int	i;
int	rows = 1<<logrows,
	cols = 1<<logcols,
	size = rows * cols;
	if (!cplx_out)
		cplx_out = cplx_in;

	dwr_load(cols);
	for (i=0; i<size; i+=cols)
		dvfftr(cplx_in+i, logcols);
	dw_load(rows);
	i = (cols>>1) + 1;
	for (size = cplx_in==cplx_out ? cols : i; i--;)
		dvfftn(cplx_in+i, logrows, cols, cplx_out+i, size);
}

dvrft_2d(cplx_in, logrows, logcols, cplx_out)
DBCOMPLEX	*cplx_in, *cplx_out;
int	logrows, logcols;
{
int	rows = 1<<logrows,
	cols = 1<<logcols,
	size;
register int	i=(cols>>1)+1;
	if (!cplx_out)
		cplx_out = cplx_in;

	dw_load(rows);
	for (size = cplx_in==cplx_out ? cols : i; i--;)
		dvfftn(cplx_in+i, logrows, size, cplx_out+i, cols);
	if (cols > 1)
	for (i=0; i<rows; i++)	{
	register int	j;
	register DBCOMPLEX*	cplx1 = cplx_out + i*cols;
	    for (j=cols>>1; --j;)	{
		cplx1[cols-j].re = cplx1[j].re;
		cplx1[cols-j].im = -cplx1[j].im;
	    }
	}
	size = rows * cols;
	dwr_load(cols);
	for (i=0; i<size; i+=cols)
		dvfftr(cplx_out+i, logcols);
}

dvfftr(cplx, loglen)
DBCOMPLEX	*cplx;
int	loglen;
{
if(loglen)	{
register int	i;
int	n, nv2, nm1, j, k, l, le, le1, c, nle;
float	tr, ti;
register DBCOMPLEX	*x, *y;

	n=1<<loglen;
	nv2=n >> 1; nm1=n-1;

	for (i=1, j=nv2; i<nm1; i++) {
	register DBCOMPLEX	tmp;
		if(i < j) {	/* do swapping */
			x = cplx+i;	y = cplx+j;
			tmp = *y;	*y = *x;	*x = tmp;
		}
		k = nv2;
		while (k <= j) {
			j -= k; k >>= 1;
		}
		j += k;
	}

	le = 1;
	for (l=0; l<loglen; l++) {
		le1 = le;
		le += le;
		nle = n/le;
		for (c=j=0; j<le1; j++) {
		    for (i=j; i<n; i+=le) {
#ifdef	_DEBUG_
			if(i+le1 >= n)
				syserr("dvfftr: strange index=%d",i+le1);
#endif
			x=cplx+i; y=cplx+(i+le1);

			if (c==0) {
				tr = y->re;
				ti = y->im;
			}
			else {
			    tr = y->re*dwr_re[c] - y->im*dwr_im[c];
			    ti = y->re*dwr_im[c] + y->im*dwr_re[c];
			}
			y->re = x->re - tr;
			y->im = x->im - ti;

			x->re += tr;
			x->im += ti;
		    }
		    c += nle;
		}
	}
}
}

dvfftn(cplx_in, loglen, nskip, cplx_out, vskip)
DBCOMPLEX	*cplx_in, *cplx_out;
int	loglen;
register int   nskip;
{
register int	i;
int	n, nv2, nm1, j, k, l, le, le1, c, nle;
float	tr, ti;
register DBCOMPLEX	*x, *y;


    if(loglen)	{
	n=1<<loglen;
	nv2=n >> 1; nm1=n-1;

	if (cplx_in == cplx_out) for (i=1, j=nv2; i<nm1; i++) {
		if(i < j) {	/* do swapping */
		register DBCOMPLEX	tmp;
			x = cplx_in+i*nskip;	y = cplx_in+j*nskip;
			tmp = *x;	*x = *y;	*y = tmp;
		}
		k = nv2;
		while (k <= j) {
			j -= k; k >>= 1;
		}
		j += k;
	}
	else	{
		cplx_out[0] = cplx_in[0];
		cplx_out[nm1*vskip] = cplx_in[nm1*nskip];
	    for (i=1, j=nv2; i<nm1; i++) {
		l = i*vskip;
		if(i < j) {
			cplx_out[l] = cplx_in[j*nskip];
			l = j*vskip;
			cplx_out[l] = cplx_in[i*nskip];
		} else if (i == j)
			cplx_out[l] = cplx_in[i*nskip];
		k = nv2;
		while (k <= j) {
			j -= k; k >>= 1;
		}
		j += k;
	    }	nskip = vskip;
	}

	le = 1;
	for (l=0; l<loglen; l++) {
		le1 = le;
		le += le;
		nle = n/le;
		for (c=j=0; j<le1; j++) {
		    for (i=j; i<n; i+=le) {
			if(i+le1 >= n)
				syserr("dvfftn: strange index=%d",i+le1);
			x=cplx_out+i*nskip;	y=cplx_out+(i+le1)*nskip;

			if (c==0) {
				tr = y->re;
				ti = y->im;
			}
			else {
			    tr = y->re*dw_re[c] - y->im*dw_im[c];
			    ti = y->re*dw_im[c] + y->im*dw_re[c];
			}
			y->re = x->re - tr;
			y->im = x->im - ti;

			x->re += tr;
			x->im += ti;
		    }
		    c += nle;
		}
	}
    }
}

