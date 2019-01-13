/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/* read_frame(...) - to read a frame from a file into a buffer.
 *		The sr tests integrity of frame by checking for
 *		frame-end at the correct position.
 *		Returns the number of bytes transfered into the buffer.
 *		(frame-end is not counted.)
 *		Sets the shift-vector, rotation-matrix and the flags
 *		integer associated with them.
 *
 * load with "pread" from hips library.
 *	also expects function "perror" to print messages on "stderr" & exit.
 *
 *
 * Yoav Cohen 11/3/82
 * modified for HIPS 2 - msl - 1/3/91
 */

#include <stdio.h>
#include <hipl_format.h>

int read_frame(fp,buf,buf_limit,flags,shift_vector,rot_matrix,fr,fname)

FILE *fp;
char *buf;
double shift_vector[3],rot_matrix[3][3];
int *flags,fr,buf_limit;
Filename fname;

{
	int vflag,mflag,nbytes,i,j;
	char b;

	if (fread(&b,1,1,fp) != 1)
		return(perr(HE_READFRFILE,fr,fname));
	*flags=b; mflag=b&01; vflag=b>>1;
	if (!vflag)
		for(i=0;i<3;i++) shift_vector[i]=0.0;
	if (!mflag)
		for(i=0;i<3;i++)
		for(j=0;j<3;j++)
			rot_matrix[i][j]=(i==j)?1.0:0.0;
	if (vflag) {
		if (fread(shift_vector,sizeof(double),3,fp) != 3)
			return(perr(HE_READFRFILE,fr,fname));
	}
	if (mflag) {
		if (fread(rot_matrix,sizeof(double),9,fp) != 9)
			return(perr(HE_READFRFILE,fr,fname));
	}
	if (fread(&nbytes,sizeof(int),1,fp) != 1)
		return(perr(HE_READFRFILE,fr,fname));
	if (nbytes>buf_limit)
		perr(HE_BUF,"read_frame");
	if (fread(buf,nbytes,1,fp) != 1)
		return(perr(HE_READFRFILE,fr,fname));
	if (buf[--nbytes]!='e')
		return(perr(HE_FRMEND));
	return(nbytes);
}
