/*
%	VFFT_FILTER . H
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
	Bld. 50B, Rm. 2275, Lawrence Berkeley Laboratory, Berkeley, CA, 94720.
	g_jin@lbl.gov

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% AUTHOR:	Jin, Guojun - LBL	5/1/91
*/

#include <math.h>
#include "header.def"
#include "imagedef.h"

#define	CURVE_CONCAVE	0
#define	CURVE_CONVEX	1
#define	CURVE_LINEAR	2

#define	FUNC_ELASTIC	0
#define	FUNC_IDEAL	1
#define	FUNC_EXP	2
#define	FUNC_BUTTERWORTH	3
#define	FUNC_RIGHT_TRI_ANGLE	4
#define	FUNC_STD_TABLE		5
#define	FUNC_ASCII_TABLE	6
#define	FUNC_BINARY_TABLE	7

#ifndef	Filter
#define	Filter	float
#endif

#ifndef	MaxSample
#define	MaxSample	1024
#endif

#define	lktFrm	2
#define	lktRow	1
#define	lktCol	0

#define	GValue(type)	avset(argc, argv, &j, &f, type)

#define	ibuf	uimg.src
#define	obuf	uimg.dest
#define	row	uimg.height
#define	cln	uimg.width
#define	frm	uimg.frames
