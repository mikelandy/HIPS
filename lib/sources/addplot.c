/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/* addplot.c - collection of functions to add plot-elements: points,
 *		vectors and half-vectors to a frame-buffer.
 *		It is assummed that the frame-buffer is already transformed,
 *		i.e. the shift-vector and the rotation-matrix which are
 *		associated with the buffer are the zero and identity resp.
 *
 *		All functions return the updated buffer-index - the next
 *		available space in the buffer.
 *
 * Loading note: the functions expect a function "perror" that reports
 *		an error condition (on "stderr") and exits, to be associated
 *		with the main program.
 *
 *
 * Yoav Cohen 11/11/82
 * modified for HIPS 2 - msl - 1/3/91
 */

#include "floatcopy.h"
#include <hipl_format.h>

int addpoint(buf,index,limit,b,x,y,z)

char *buf;
double b,x,y,z;
int index,limit;

{
	static float *fbuf;
	static int addin;

	if((addin=index+4*sizeof(float)+sizeof(char)) > limit)
		return(perr(HE_BUF,"addpoint"));
	buf[index++]='p';
	fbuf=(float *)(buf+index);
	storefloat(fbuf,b); fbuf++;
	storefloat(fbuf,x); fbuf++;
	storefloat(fbuf,y); fbuf++;
	storefloat(fbuf,z);

	return(addin);
}

int addvec(buf,index,limit,b,x1,y1,z1,x2,y2,z2)

char *buf;
double b,x1,y1,z1,x2,y2,z2;
int index,limit;

{
	static float *fbuf;
	static int addin;

	if((addin=index+7*sizeof(float)+sizeof(char)) > limit)
		return(perr(HE_BUF,"addvec"));
	buf[index++]='v';
	fbuf=(float *)(buf+index);
	storefloat(fbuf,(float)b); fbuf++;
	storefloat(fbuf,(float)x1); fbuf++;
	storefloat(fbuf,(float)y1); fbuf++;
	storefloat(fbuf,(float)z1); fbuf++;
	storefloat(fbuf,(float)x2); fbuf++;
	storefloat(fbuf,(float)y2); fbuf++;
	storefloat(fbuf,(float)z2);
	
	return(addin);
}

int addend(buf,index,limit,x,y,z)

char *buf;
double x,y,z;
int index,limit;

{
	static float *fbuf;
	static int addin;

	if((addin=index+3*sizeof(float)+sizeof(char)) > limit)
		return(perr(HE_BUF,"addend"));
	buf[index++]='n';
	fbuf=(float *)(buf+index);
	storefloat(fbuf,(float)x); fbuf++;
	storefloat(fbuf,(float)y); fbuf++;
	storefloat(fbuf,(float)z);

	return(addin);
}
