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
 * pyrredexp.c - pyramid routines for the reduce and expand operations
 *
 * Only symmetric kernels of odd size are handled.  Kernel symmetries are
 * used since the filters are only stored with half of the taps specified.
 */

#include <hipl_format.h>

static FIMAGE fscr;
int hor_reflectf(),ver_reflectf(),reflectf(),hor_reflecti(),ver_reflecti();
int reflecti(),_ireduce_odd(),_freduce_odd(),_iexpand_odd(),_fexpand_odd();

int freduce(pyr,botlev,toplev,rf,rtype)

FPYR pyr;
FILTER rf;

int botlev,toplev,rtype;
{
	int lev;

	for (lev=botlev; lev < toplev; lev++) {
		if (_freduce_odd(pyr[lev],pyr[lev+1],rf,rtype) == HIPS_ERROR)
			return(HIPS_ERROR);
	}
	return(HIPS_OK);
}

int fexpand(pyr,botlev,toplev,ef,mode,rtype)

FPYR pyr;
FILTER ef;
int botlev,toplev;
int mode,rtype;

{
	int lev,i,j;

	if (mode > 0) {	/* Expand and add (reconstruct from Laplacian) */
		for (lev=toplev; lev > botlev; lev--) {
			if (_fexpand_odd(pyr[lev],pyr[lev-1],ef,1,rtype)
				== HIPS_ERROR)
					return(HIPS_ERROR);
		}
	} else if (mode < 0) {	/* Expand and subtract (build Laplacian) */
		for (lev=botlev+1; lev <= toplev; lev++) {
			if (_fexpand_odd(pyr[lev],pyr[lev-1],ef,-1,rtype)
				== HIPS_ERROR)
					return(HIPS_ERROR);
		}
	} else if (mode == 0) {/* Zero expand and add (Smoothing) */
		for (lev=toplev-1; lev >= botlev; lev--) {
			for (j=0; j < pyr[lev].nr; j++)
				for (i=0; i < pyr[lev].nc; i++)
					pyr[lev].ptr[j][i] = 0.0;
			if (_fexpand_odd(pyr[lev+1],pyr[lev],ef,1,rtype)
				== HIPS_ERROR)
					return(HIPS_ERROR);
		}
	}
	return(HIPS_OK);
}

int _freduce_odd(in,out,rf,rtype)

FIMAGE in,out;
FILTER rf;
int rtype;

{
	register float **a=in.ptr,**b=out.ptr,**s;
	register float *fk=rf.k;
	float val;
	int rtaps=rf.taps2;
	register int n,i,j,il,jl;

	/* allocate scratch image */

	fscr.nr = in.nr;  
	fscr.nc = out.nc;
	if (alloc_fimage(&fscr) == HIPS_ERROR)
		return(HIPS_ERROR);
	s = fscr.ptr;

	if (hor_reflectf(in,rtaps,rtype) == HIPS_ERROR)
		return(HIPS_ERROR);
	for (j=0; j < in.nr; j++) {		/* horizontal filtering */
		for (il=0, i=0; il < out.nc; il++, i+=2) {
			for (val = fk[0]*a[j][i], n=1; n <= rtaps; n++)
				val += fk[n]*(a[j][i+n] + a[j][i-n]);
			s[j][il] = val;
		}
	}

	if (ver_reflectf(fscr,rtaps,rtype) == HIPS_ERROR)
		return(HIPS_ERROR);
	for (jl = 0,j = 0; jl < out.nr; jl++, j+=2) { /* vertical filtering */
		for (i=0; i < out.nc; i++) {
			val = fk[0]*s[j][i];
			for (n=1; n <= rtaps; n++)
				val += fk[n]*(s[j+n][i] + s[j-n][i]);
			b[jl][i] = val;
		}
	}
	if (free_fimage(fscr) == HIPS_ERROR)
		return(HIPS_ERROR);
	return(HIPS_OK);
}

int _fexpand_odd(in,out,ef,mode,rtype)

FIMAGE in,out;
FILTER ef;
int mode,rtype;

{
	register float **a=in.ptr,**b=out.ptr,**s;
	register float *ek=ef.k;
	float val;
	int etaps=ef.taps2;

	mode = 4*mode;
	/* allocate scratch image */
	fscr.nr = out.nr;  
	fscr.nc = in.nc;
	if (alloc_fimage(&fscr) == HIPS_ERROR)
		return(HIPS_ERROR);
	s=fscr.ptr;

	/* vertical filtering */
	{
		register int n,i,j,jj,jj1,jb,jt;

		if (ver_reflectf(in,etaps+1,rtype) == HIPS_ERROR)
			return(HIPS_ERROR);
		for (jj=0, j=0; jj < out.nr; j++,jj+=2) {
			jj1 = jj + 1;
			for (i=0; i < in.nc; i++) {
				for (val=ek[0]*a[j][i],n=2, jb=j-1, jt=j+1;
				    n <= etaps; n+=2)
					val += ek[n]*(a[jb][i] + a[jt][i]);
				s[jj][i] = val;
				for (val=0.0, n=1, jb=j, jt=j+1;
				    n <= etaps; n+=2)
					val += ek[n]*(a[jb][i] + a[jt][i]);
				s[jj1][i] = val;
			}
		}
	}

	/* horizontal filtering */
	{
		register int n,iout,i,j,il,ir;

		if (hor_reflectf(fscr,etaps+1,rtype) == HIPS_ERROR)
			return(HIPS_ERROR);
		for (j=0; j < out.nr; j++)
			for (iout=0, i=0; iout < out.nc; i++) {
				for (val=ek[0]*s[j][i], n=2, il=i-1, ir=i+1;
				    n <= etaps; n+=2)
					val += ek[n]*(s[j][il--] + s[j][ir++]);
				b[j][iout++] += mode*val;
				for (val=0.0, n=1, il=i, ir=i+1;
				    n <= etaps; n+=2)
					val += ek[n]*(s[j][il--] + s[j][ir++]);
				b[j][iout++] += mode*val;
			}
	}
	if (free_fimage(fscr) == HIPS_ERROR)
		return(HIPS_ERROR);
	return(HIPS_OK);
}

int ireduce(pyr,botlev,toplev,rf,rtype)

IPYR pyr;
FILTER rf;
int rtype;

int botlev,toplev;
{
	int lev;

	for (lev=botlev; lev < toplev; lev++) {
		if (_ireduce_odd(pyr[lev],pyr[lev+1],rf,rtype) == HIPS_ERROR)
			return(HIPS_ERROR);
	}
	return(HIPS_OK);
}

int iexpand(pyr,botlev,toplev,ef,mode,rtype)

IPYR pyr;
FILTER ef;
int botlev,toplev;
int mode,rtype;

{
	int lev,i,j;

	if (mode > 0) {	/* Expand and add (reconstruct from Laplacian) */
		for (lev=toplev; lev > botlev; lev--) {
			if (_iexpand_odd(pyr[lev],pyr[lev-1],ef,1,rtype)
				== HIPS_ERROR)
					return(HIPS_ERROR);
		}
	} else if (mode < 0) {	/* Expand and subtract (build Laplacian) */
		for (lev=botlev+1; lev <= toplev; lev++) {
			if (_iexpand_odd(pyr[lev],pyr[lev-1],ef,-1,rtype)
				== HIPS_ERROR)
					return(HIPS_ERROR);
		}
	} else if (mode == 0) {/* Zero expand and add (Smoothing) */
		for (lev=toplev-1; lev >= botlev; lev--) {
			for (j=0; j < pyr[lev].nr; j++)
				for (i=0; i < pyr[lev].nc; i++)
					pyr[lev].ptr[j][i] = 0;
			if (_iexpand_odd(pyr[lev+1],pyr[lev],ef,1,rtype)
				== HIPS_ERROR)
					return(HIPS_ERROR);
		}
	}
	return(HIPS_OK);
}

int _ireduce_odd(in,out,rf,rtype)

IIMAGE in,out;
FILTER rf;
int rtype;

{
	register int **a=in.ptr,**b=out.ptr;
	register float *fk=rf.k,**s;
	float val;
	int rtaps=rf.taps2;
	register int n,i,j,il,jl;

	/* allocate scratch image */

	fscr.nr = in.nr;  
	fscr.nc = out.nc;
	if (alloc_fimage(&fscr) == HIPS_ERROR)
		return(HIPS_ERROR);
	s = fscr.ptr;

	if (hor_reflecti(in,rtaps,rtype) == HIPS_ERROR)
		return(HIPS_ERROR);
	for (j=0; j < in.nr; j++) {		/* horizontal filtering */
		for (il=0, i=0; il < out.nc; il++, i+=2) {
			for (val = fk[0]*a[j][i], n=1; n <= rtaps; n++)
				val += fk[n]*(a[j][i+n] + a[j][i-n]);
			s[j][il] = val;
		}
	}

	if (ver_reflectf(fscr,rtaps,rtype) == HIPS_ERROR)
		return(HIPS_ERROR);
	for (jl = 0,j = 0; jl < out.nr; jl++, j+=2) { /* vertical filtering */
		for (i=0; i < out.nc; i++) {
			val = fk[0]*s[j][i];
			for (n=1; n <= rtaps; n++)
				val += fk[n]*(s[j+n][i] + s[j-n][i]);
			b[jl][i] = val > 0. ? (val + 0.5) : (val - 0.5);
		}
	}
	if (free_fimage(fscr) == HIPS_ERROR)
		return(HIPS_ERROR);
	return(HIPS_OK);
}

int _iexpand_odd(in,out,ef,mode,rtype)

IIMAGE in,out;
FILTER ef;
int mode,rtype;

{
	register int **a=in.ptr,**b=out.ptr;
	register float *ek=ef.k,**s;
	float val;
	int etaps=ef.taps2;

	mode = 4*mode;
	/* allocate scratch image */
	fscr.nr = out.nr;  
	fscr.nc = in.nc;
	if (alloc_fimage(&fscr) == HIPS_ERROR)
		return(HIPS_ERROR);
	s=fscr.ptr;

	/* vertical filtering */
	{
		register int n,i,j,jj,jj1,jb,jt;

		if (ver_reflecti(in,etaps+1,rtype) == HIPS_ERROR)
			return(HIPS_ERROR);
		for (jj=0, j=0; jj < out.nr; j++,jj+=2) {
			jj1 = jj + 1;
			for (i=0; i < in.nc; i++) {
				for (val=ek[0]*a[j][i],n=2, jb=j-1, jt=j+1;
				    n <= etaps; n+=2)
					val += ek[n]*(a[jb][i] + a[jt][i]);
				s[jj][i] = val;
				for (val=0.0, n=1, jb=j, jt=j+1;
				    n <= etaps; n+=2)
					val += ek[n]*(a[jb][i] + a[jt][i]);
				s[jj1][i] = val;
			}
		}
	}

	/* horizontal filtering */
	{
		register int n,iout,i,j,il,ir;

		if (hor_reflectf(fscr,etaps+1,rtype) == HIPS_ERROR)
			return(HIPS_ERROR);
		for (j=0; j < out.nr; j++)
			for (iout=0, i=0; iout < out.nc; i++) {
				for (val=ek[0]*s[j][i], n=2, il=i-1, ir=i+1;
				    n <= etaps; n+=2)
					val += ek[n]*(s[j][il--] + s[j][ir++]);
				val = b[j][iout] + mode*val;
				b[j][iout++] = val > 0. ? (val + 0.5) :
					(val - 0.5);
				for (val=0.0, n=1, il=i, ir=i+1;
				    n <= etaps; n+=2)
					val += ek[n]*(s[j][il--] + s[j][ir++]);
				val = b[j][iout] + mode*val;
				b[j][iout++] = val > 0. ? (val + 0.5) :
					(val - 0.5);
			}
	}
	if (free_fimage(fscr) == HIPS_ERROR)
		return(HIPS_ERROR);
	return(HIPS_OK);
}
