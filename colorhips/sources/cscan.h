/*
	@(#)cscan.h	1.3 8/24/88
	Copyright 1988 Alan Shaw and Eric Schwartz.
	No part of this software may be distributed or sold without the prior
	agreement of Prof. Eric Schwartz, Dept. of Psychiatry, NYU School of
	Medicine, 550 1st Ave., New York, New York, 10016.
 
	 Changelog:
	 12/15/08 - rld - re-defined INFINITY to INFINITY_HIPS for compatibility with OSX
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>

#define COLORS		32768		/* number of points in rgb space
					as specified in scanfile */
#define LEVELS		32		/* cube root of COLORS */
#define LEVELSSQ	1024		/* LEVELS squared */
#define MYBITS		5		/* log2 of LEVELS */
#define	trunc(c)	(((u_char)(c)) >> (8 - MYBITS))
#define	untrunc(c)	((u_char)((c) << (8 - MYBITS)))

#define	distsq(i, R, G, B) \
	(*(*(redsquares + (R)) + (i)) \
	+ *(*(greensquares + (G)) + (i)) \
	+ *(*(bluesquares + (B)) + (i)))

struct Cellist {
	short		bindex;		/* lut point index */
	short		celldistsq;	/* its distance from the cell */
	struct Cellist	*next;
};

#define	INFINITY_HIPS	3073	/* must be greater than the square of the
				main diagonal of a cube LEVELSxLEVELSxLEVELS */

#define	SUCCESS	1
#define	FAIL	0
