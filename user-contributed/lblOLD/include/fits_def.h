/*	FITS_DEF . H
#
%	for FITS and many other images.
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
	jin@george.lbl.gov

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% AUTHOR:	Jin Guojun - LBL	8/1/90
*/

#ifndef	F_ICH
#define	F_ICH

#include <ctype.h>
#include <stdio.h>

#define	index	strchr
#define	rindex	strrchr

typedef short   FITSType;

typedef	struct	{
	char	sample, *datatype, *format, *date, *history;
	int	bits_pxl,
		naxis, naxis1, naxis2, naxis3,
		naux, minval, maxval;
	float	scale,		/* Compression scale factor	*/
		bscale, BZero,	/*	convert factor		*/
		crval1, crval2, seeing;	/* convert coefficient	*/
}	FITS_BASE;

#ifndef	MaxUNIXFortranBlock
#define	MaxUNIXFortranBlock	0x1BD6E
#endif
#ifndef	MaxVAXFortranBlock
#define	MaxVAXFortranBlock	2044
#endif
#ifndef	MaxPCFortranBlock
#define	MaxPCFortranBlock	128
#endif
#ifndef	FITSBlock
#define	FITSBlock	2880
#endif

#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

#ifdef	TC_Need
#define	_DEBUG_
/*   pc i/o defines	*/
#ifndef	O_RDONLY
#define O_RDONLY	0x0000	/* open for reading only	*/
#define O_BINARY	0x8000	/* file mode is binary		*/
#endif
#endif	TC_Need

extern	FITS_BASE	fhd;

typedef	char	Elem;
typedef char*	MAT;
typedef	unsigned short	MCtrlV;

extern	MCtrlV	N;	/* dimension of matrixes */

/* Tool to provide variable sized matrix indexing */

#define M(m,i,j)	m[(i)*N + j]
#define	MatCopy(a,b)	memcpy(b, a, N*N*sizeof(Elem))

typedef	union
	{
	char	c;
	char*	s;
	short	h;
	long_32	l;
	float	f;
	double	d;
	} convts;

typedef	union /* this union is used to swap 16 and 32 bit integers	*/
	{
	char	ichar[4];
	short	slen;
	long_32	llen;
	} SwapUnion;

typedef	union	{
	char	ichar[2];
	short	ilen;
	} Sonion;

extern	FILE	*get_infile(), *get_outfile();
extern	long_32	read_var();
extern	int	check_host(), hostype;
extern	bool	FORTRAN,/* Fortran file flag	*/
		FTy,	/* Fortran file type	*/
		FFCL,	/* Fortran Ctrl word length:	UNIX = 4, PC = 1. */
		DCMP, Msg,
		CONVT;	/* default convert type is byte	*/

#endif	F_ICH
