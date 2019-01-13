/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/* trans_frame - to transform a frame according to the values
 *		 of the shift vector and the rotation matrix.
 *		 The sr updates the value of "flags" and the vector
 *		 and matrix, to reflect the changes.
 *		 The sr assumes that "flags" reflect the true state of
 *		 affairs on entry.
 *
 * Loading note:  the sr calls a function "perror" to print messages on "stderr"
 *	     and exit.
 *
 * Efficiency: some address calculations are redundant, left as is for the
 *		sake of readability.
 *
 * Yoav Cohen  11/3/82
 * modified for HIPS 2 - msl - 1/3/91
 */

#include "floatcopy.h"
#include <hipl_format.h>

int trans_frame(buf,nbuf,shift_v,rot_m,flags)

char *buf;
double shift_v[3],rot_m[3][3];
int *flags,nbuf;

{
	static int	op,indx,i,j,ip;
	static double	x,y,z;
	static double	s1,s2,s3,r11,r12,r13,r21,r22,r23,r31,r32,r33;
	static int	rx,ry,rz,sx,sy,sz,tx,ty,tz;
	static float 	*fbuf;

	if (*flags==0) return(HIPS_OK);

	s1=shift_v[0]; s2=shift_v[1]; s3=shift_v[2];
	r11=rot_m[0][0]; r12=rot_m[0][1]; r13=rot_m[0][2];
	r21=rot_m[1][0]; r22=rot_m[1][1]; r23=rot_m[1][2];
	r31=rot_m[2][0]; r32=rot_m[2][1]; r33=rot_m[2][2];
	rx= !(r11==1.0 && r12==0.0 && r13==0.0);
	ry= !(r21==0.0 && r22==1.0 && r23==0.0);
	rz= !(r31==0.0 && r32==0.0 && r33==1.0);
	sx= !(s1==0.0); sy= !(s2==0.0); sz= !(s3==0.0);
	tx= rx|sx; ty= ry|sy; tz= rz|sz;

	for(indx=0;indx<nbuf;) {
		op=buf[indx++];
		switch (op) {
		case 'p':	fbuf=(float *)(buf+indx+sizeof(float));
				getfloat(x,fbuf);
				getfloat(y,fbuf+1);
				getfloat(z,fbuf+2);
				if(tx)
				    storefloat(fbuf,((rx)
					?(r11*x+r12*y+r13*z):x)+s1);
				if(ty)
				    storefloat(fbuf+1,((ry)
					?(r21*x+r22*y+r23*z):y)+s2);
				if(tz)
				    storefloat(fbuf+2,((rz)
					?(r31*x+r32*y+r33*z):z)+s3);
				indx+=4*sizeof(float);
				break;
		case 'v':	fbuf=(float *)(buf+indx+sizeof(float));
				for(ip=0;ip<2;ip++) {
				    getfloat(x,fbuf);
				    getfloat(y,fbuf+1);
				    getfloat(z,fbuf+2);
				    if(tx)
					storefloat(fbuf,((rx)
						?(r11*x+r12*y+r13*z):x)+s1);
				    if(ty)
					storefloat(fbuf+1,((ry)
						?(r21*x+r22*y+r23*z):y)+s2);
				    if(tz)
					storefloat(fbuf+2,((rz)
						?(r31*x+r32*y+r33*z):z)+s3);
				    fbuf+=3;
				}
				indx+=7*sizeof(float);
				break;
		case 'n':	fbuf=(float *)(buf+indx);
				getfloat(x,fbuf);
				getfloat(y,fbuf+1);
				getfloat(z,fbuf+2);
				if(tx)
				    storefloat(fbuf,((rx)
					?(r11*x+r12*y+r13*z):x)+s1);
				if(ty)
				    storefloat(fbuf+1,((ry)
					?(r21*x+r22*y+r23*z):y)+s2);
				if(tz)
				    storefloat(fbuf+2,((rz)
					?(r31*x+r32*y+r33*z):z)+s3);
				indx+=3*sizeof(float);
				break;
		default : 	return(perr(HE_CODE,"trans_frame"));
		}
	}
	for(i=0;i<3;i++) {
		shift_v[i]=0.0;
		for(j=0;j<3;j++)
			rot_m[i][j]=(i==j)?1.0:0.0;
	}
	*flags = 0;
	return(HIPS_OK);
}
