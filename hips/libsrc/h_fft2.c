/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_fft2.c - fast fourier transform, adapted from 
 * Gonzalez & Wintz p.87.
 *
 * No division by N is performed.
 *
 * These are the versions with separate real and imaginary arrays.
 *
 * Calling sequence:
 *
 * float *rvec,*ivec;
 * int loglen,skip;
 *
 * h_fft_ri_c(rvec,ivec,loglen)
 * performs a 1-dimensional fft
 * 
 * h_fft2d_ri_c(rvec,ivec,loglen)
 * performs a 2-dimensional fft where loglen is the log of the length of
 * a side of the array
 *
 * h_fft2dgen_ri_c(rvec,ivec,logrows,logcols)
 * performs a 2-dimensional fft where logrows is the log of the number of
 * rows, and logcols is the log of the number of columns
 *
 * h_fftn_ri_c(rvec,ivec,loglen,skip)
 * performs a 1-dimensional fft on every skip-th entry
 *
 * h_fft_ri_dc, h_fft2d_ri_dc, h_fft2dgen_ri_dc and h_fftn_ri_dc are the
 * double complex version.
 *
 * modified for HIPS 2 - msl - 1/3/91
 */

#include <hipl_format.h>
#include <stdio.h>
#include <math.h>

#define pi 3.14159265358979

static float *cconstr,*cconsti;
static int cstoresize = 0, cconstrsize = 0;

int h_fft_ri_c(rvec,ivec,loglen)

float *rvec,*ivec;
int loglen;

{
	return(h_fftn_ri_c(rvec,ivec,loglen,1));
}

int h_fft2d_ri_c(rvec,ivec,loglen)

float *rvec,*ivec;
int loglen;

{
	int i,size;

	size = 1<<loglen;
	for (i=0;i<size*size;i+=size) {
		if (h_fftn_ri_c(rvec+i,ivec+i,loglen,1) == HIPS_ERROR)
			return(HIPS_ERROR);
	}
	for (i=0;i<size;i++) {
		if (h_fftn_ri_c(rvec+i,ivec+i,loglen,size) == HIPS_ERROR)
			return(HIPS_ERROR);
	}
	return(HIPS_OK);
}

int h_fft2dgen_ri_c(rvec,ivec,logrows,logcols)

float *rvec,*ivec;
int logrows,logcols;

{
	int i,rows,cols,size;

	rows = 1<<logrows;
	cols = 1<<logcols;
	size = rows * cols;
	for (i=0;i<size;i+=cols) {
		if (h_fftn_ri_c(rvec+i,ivec+i,logcols,1) == HIPS_ERROR)
			return(HIPS_ERROR);
	}
	for (i=0;i<cols;i++) {
		if (h_fftn_ri_c(rvec+i,ivec+i,logrows,cols) == HIPS_ERROR)
			return(HIPS_ERROR);
	}
	return(HIPS_OK);
}

int h_fftn_ri_c(rvec,ivec,loglen,nskip)

float *rvec,*ivec;
int loglen,nskip;

{
	int n,nv2,nm1,i,j,k,l,le,le1,c,nle;
	float	*rveci , *rvecj , *iveci , *ivecj ;
	float	t,wr,wi,tr,ti ;


	if(loglen==0)
		return(HIPS_OK);
	n=1<<loglen ;
	nv2=n >> 1 ; nm1=n-1 ; j=0 ;
	if (cstoresize<nv2) {
		if ((0==(cconstr=(float *)malloc(nv2*sizeof(float)))) ||
		    (0==(cconsti=(float *)malloc(nv2*sizeof(float)))))
			return(perr(HE_ALLOCSUBR,"h_fftn_ri_c"));
		cstoresize = nv2;
	}
	if (cconstrsize!=nv2) {
		cconstrsize = nv2;
		wr =  cos(2*pi/n);
		wi = -sin(2*pi/n);
		cconstr[0] = 1.;
		cconsti[0] = 0.;
		for (i=1;i<nv2;i++) {
			cconstr[i] = wr*cconstr[i-1] - wi*cconsti[i-1];
			cconsti[i] = wr*cconsti[i-1] + wi*cconstr[i-1];
		}
	}
	for (i=0;i<nm1;i++) {
		if(i<j) {
			rveci=rvec+i*nskip ; rvecj=rvec+j*nskip ;
			t=(*rvecj) ; *rvecj=(*rveci) ; *rveci=t ; 
			iveci=ivec+i*nskip ; ivecj=ivec+j*nskip ;
			t=(*ivecj) ; *ivecj=(*iveci) ; *iveci=t ;
		}
		k=nv2 ;
		while (k<=j) {
			j-=k ; k>>=1 ;
		}
		j+=k ;
	}
	le=1 ;
	for (l=0;l<loglen;l++) {
		le1=le ; le+=le ; c = 0; nle = n/le;
		for (j=0;j<le1;j++) {
			for (i=j;i<n;i+=le) {
				if(i+le1>=n)
					return(perr(HE_FFTI,
						"h_fftn_ri_c",i+le1));
				rveci=rvec+i*nskip ; rvecj=rvec+(i+le1)*nskip;
			  	iveci=ivec+i*nskip ; ivecj=ivec+(i+le1)*nskip;

			  	if (c==0) {
					tr = *rvecj;
					ti = *ivecj;
				}
				else {
				    tr = *rvecj*cconstr[c] - *ivecj*cconsti[c];
				    ti = *rvecj*cconsti[c] + *ivecj*cconstr[c];
				}
				*rvecj = *rveci - tr;
				*ivecj = *iveci - ti;

				*rveci += tr;
				*iveci += ti;
			}
			c += nle;
		}
	}
/** Division by n
	for(i=0;i<n;i++)
		{rvec[i*nskip]/=n ; ivec[i*nskip]/=n ;}
****************/
	return(HIPS_OK);
}

static double *dcconstr,*dcconsti;
static int dcstoresize = 0, ddcconstrsize = 0;

int h_fft_ri_dc(rvec,ivec,loglen)

double *rvec,*ivec;
int loglen;

{
	return(h_fftn_ri_dc(rvec,ivec,loglen,1));
}

int h_fft2d_ri_dc(rvec,ivec,loglen)

double *rvec,*ivec;
int loglen;

{
	int i,size;

	size = 1<<loglen;
	for (i=0;i<size*size;i+=size) {
		if (h_fftn_ri_dc(rvec+i,ivec+i,loglen,1) == HIPS_ERROR)
			return(HIPS_ERROR);
	}
	for (i=0;i<size;i++) {
		if (h_fftn_ri_dc(rvec+i,ivec+i,loglen,size) == HIPS_ERROR)
			return(HIPS_ERROR);
	}
	return(HIPS_OK);
}

int h_fft2dgen_ri_dc(rvec,ivec,logrows,logcols)

double *rvec,*ivec;
int logrows,logcols;

{
	int i,rows,cols,size;

	rows = 1<<logrows;
	cols = 1<<logcols;
	size = rows * cols;
	for (i=0;i<size;i+=cols) {
		if (h_fftn_ri_dc(rvec+i,ivec+i,logcols,1) == HIPS_ERROR)
			return(HIPS_ERROR);
	}
	for (i=0;i<cols;i++) {
		if (h_fftn_ri_dc(rvec+i,ivec+i,logrows,cols) == HIPS_ERROR)
			return(HIPS_ERROR);
	}
	return(HIPS_OK);
}

int h_fftn_ri_dc(rvec,ivec,loglen,nskip)

double *rvec,*ivec;
int loglen,nskip;

{
	int n,nv2,nm1,i,j,k,l,le,le1,c,nle;
	double	*rveci , *rvecj , *iveci , *ivecj ;
	double	t,wr,wi,tr,ti ;


	if (loglen==0)
		return(HIPS_OK);
	n=1<<loglen ;
	nv2=n >> 1 ; nm1=n-1 ; j=0 ;
	if (dcstoresize<nv2) {
		if ((0==(dcconstr=(double *)malloc(nv2*sizeof(double)))) ||
		    (0==(dcconsti=(double *)malloc(nv2*sizeof(double)))))
			return(perr(HE_ALLOCSUBR,"h_fftn_ri_dc"));
		dcstoresize = nv2;
	}
	if (ddcconstrsize!=nv2) {
		ddcconstrsize = nv2;
		wr =  cos(2*pi/n);
		wi = -sin(2*pi/n);
		dcconstr[0] = 1.;
		dcconsti[0] = 0.;
		for (i=1;i<nv2;i++) {
			dcconstr[i] = wr*dcconstr[i-1] - wi*dcconsti[i-1];
			dcconsti[i] = wr*dcconsti[i-1] + wi*dcconstr[i-1];
		}
	}
	for (i=0;i<nm1;i++) {
		if(i<j) {
			rveci=rvec+i*nskip ; rvecj=rvec+j*nskip ;
			t=(*rvecj) ; *rvecj=(*rveci) ; *rveci=t ; 
			iveci=ivec+i*nskip ; ivecj=ivec+j*nskip ;
			t=(*ivecj) ; *ivecj=(*iveci) ; *iveci=t ;
		}
		k=nv2 ;
		while (k<=j) {
			j-=k ; k>>=1 ;
		}
		j+=k ;
	}
	le=1 ;
	for (l=0;l<loglen;l++) {
		le1=le ; le+=le ; c = 0; nle = n/le;
		for (j=0;j<le1;j++) {
			for (i=j;i<n;i+=le) {
				if(i+le1>=n)
					return(perr(HE_FFTI,
						"h_fftn_ri_dc",i+le1));
				rveci=rvec+i*nskip ; rvecj=rvec+(i+le1)*nskip;
			  	iveci=ivec+i*nskip ; ivecj=ivec+(i+le1)*nskip;

			  	if (c==0) {
					tr = *rvecj;
					ti = *ivecj;
				}
				else {
				    tr = *rvecj*dcconstr[c] -
					*ivecj*dcconsti[c];
				    ti = *rvecj*dcconsti[c] +
					*ivecj*dcconstr[c];
				}
				*rvecj = *rveci - tr;
				*ivecj = *iveci - ti;

				*rveci += tr;
				*iveci += ti;
			}
			c += nle;
		}
	}
/** Division by n
	for(i=0;i<n;i++)
		{rvec[i*nskip]/=n ; ivec[i*nskip]/=n ;}
****************/
	return(HIPS_OK);
}
