/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/* write_frame(...) - to write a frame from a buffer into a file.
 *		The sr adds frame-end at the correct position.
 *		Sets the shift-vector, rotation-matrix and the flags
 *		associated with them.
 *
 * Yoav Cohen 11/3/82
 * modified for HIPS 2 - msl - 1/3/91
 */

#include <hipl_format.h>
#include <stdio.h>

int write_frame(fp,buf,nbytes,shift_v,rot_m,fr)

FILE *fp;
char *buf;
double shift_v[3],rot_m[3][3];
int nbytes,fr;

{
	int vflag,mflag,i,j;
	char b;

	vflag=2; mflag=0;
	
	if(shift_v[0]==0.0 && shift_v[1]==0.0 && shift_v[2]==0.0 )
		vflag=0;
	for(i=0;i<3;i++)
	for(j=0;j<3;j++)
		if(rot_m[i][j] != ((i==j)?1.0:0.0))
			mflag=1;
	b= vflag|mflag;
	if (fwrite(&b,1,1,fp) != 1)
		return(perr(HE_WRITEFR,fr));
	if (vflag) {
		if (fwrite(shift_v,sizeof(double),3,fp) != 3)
			return(perr(HE_WRITEFR,fr));
	}
	if (mflag) {
		if (fwrite(rot_m,sizeof(double),9,fp) != 9)
			return(perr(HE_WRITEFR,fr));
	}
	nbytes++;
	if (fwrite(&nbytes,sizeof(int),1,fp) != 1)
		return(perr(HE_WRITEFR,fr));
	if ((--nbytes)>0) {
		if (fwrite(buf,nbytes,1,fp) != 1)
			return(perr(HE_WRITEFR,fr));
	}
	b='e';
	if (fwrite(&b,1,1,fp) != 1)
		return(perr(HE_WRITEFR,fr));
	return(HIPS_OK);
}
