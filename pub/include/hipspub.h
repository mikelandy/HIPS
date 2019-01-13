/*
 * Copyright (c) 1992 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * hipspub.h - definitions of raster HIPS formats
 *
 * Michael Landy - 9/30/92
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * Pixel Format Codes (raster formats only)
 */

#define	PFBYTE		0	/* Bytes interpreted as unsigned integers */
#define PFSHORT		1	/* Short integers (2 bytes) */
#define PFINT		2	/* Integers (4 bytes) */
#define	PFFLOAT		3	/* Float's (4 bytes)*/
#define	PFCOMPLEX 	4	/* 2 Float's interpreted as (real,imaginary) */
#define PFASCII		5	/* ASCII rep, with linefeeds after each row */
#define	PFDOUBLE 	6	/* Double's (8 byte floats) */
#define	PFDBLCOM 	7	/* Double complex's (2 Double's) */
#define	PFMSBF		30	/* packed, most-significant-bit first */
#define	PFLSBF		31	/* packed, least-significant-bit first */
#define	PFSBYTE		32	/* signed bytes */
#define	PFUSHORT	33	/* unsigned shorts */
#define	PFUINT		34	/* unsigned ints */
#define	PFRGB		35	/* RGBRGBRGB bytes */
#define	PFRGBZ		36	/* RGB0RGB0RGB0 bytes */

#define	LINELENGTH 200		/* max characters per line in header vars */

#define MSBFIRST 1	/* HIPS-1 bit_packing value for PFMSBF */
#define LSBFIRST 2	/* HIPS-1 bit_packing value for PFLSBF */

/* extended parameter list structure */

struct hpub_xparlist {
	int	numparam;	/* Count of additional parameters */
	struct hpub_extpar *params;	/* Additional parameters */
};

/* the extended parameter item structure */

struct hpub_extpar {
	char *name;		/* name of this variable */
	int format;		/* format of values (PFBYTE, PFINT, etc.) */
	int count;		/* number of values */
	int offset;		/* offset into binary area for arrays */
	union {
		unsigned char v_b;	/* PFBYTE/PFASCII, count = 1 */
		int v_i;	/* PFINT, count = 1 */
		short v_s;	/* PFSHORT, count = 1 */
		float v_f;	/* PFFLOAT, count = 1 */
		unsigned char *v_pb;	/* PFBYTE/PFASCII, count > 1 */
		int *v_pi;	/* PFINT, count > 1 */
		short *v_ps;	/* PFSHORT, count > 1 */
		float *v_pf;	/* PFFLOAT, count > 1 */
	} val;
	struct hpub_extpar *nextp;	/* next parameter in list */
};

#define	HP_NULLPAR	((struct hpub_extpar *) 0)

char *hpub_strsave();
unsigned char hpub_getparamb(),*hpub_getparamb2();
short hpub_getparams(),*hpub_getparams2();
int hpub_getparami(),*hpub_getparami2();
float hpub_getparamf(),*hpub_getparamf2();
struct hpub_extpar *hpub_xpar_findalloc(),*hpub_checkfind(),*hpub_findparam();
