/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/* getplot - to get a plot element from a frame buffer.
 *		returns the new index to the frame-buffer.
 *
 * Loading note: expects function "perror" to print error condition & exit.
 *
 * Yoav Cohen 11/11/82
 * modified for HIPS 2 - msl - 1/3/91
 */

#include <hipl_format.h>
#include "floatcopy.h"

int getplot(buf,index,op,b,x1,y1,z1,x2,y2,z2)

char *buf;
int *op,index;
double *b,*x1,*x2,*y1,*y2,*z1,*z2;

{
	float *fbuf;
	int addin;

	*op= buf[index++];
	fbuf=(float *)(buf+index);
	switch (*op) {
	case	'p':	copyfloat(b,fbuf); fbuf++;
			copyfloat(x1,fbuf); fbuf++;
			copyfloat(y1,fbuf); fbuf++;
			copyfloat(z1,fbuf);
			addin= 4*sizeof(float);
			break;
	case	'v':	copyfloat(b,fbuf); fbuf++;
			copyfloat(x1,fbuf); fbuf++;
			copyfloat(y1,fbuf); fbuf++;
			copyfloat(z1,fbuf); fbuf++;
			copyfloat(x2,fbuf); fbuf++;
			copyfloat(y2,fbuf); fbuf++;
			copyfloat(z2,fbuf);
			addin= 7*sizeof(float);
			break;
	case 	'n':	copyfloat(x2,fbuf); fbuf++;
			copyfloat(y2,fbuf); fbuf++;
			copyfloat(z2,fbuf);
			addin= 3*sizeof(float);
			break;
	default:	return(perr(HE_CODE,"getplot"));
	}
	return(index+addin);
}
