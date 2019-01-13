/* util.h */

/* This file contains global type-declarations and different utility
   routines operating on vectors and matrices. 

				Karsten Hartelius  august 6th 1992 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <hipl_format.h>
#include <math.h>


#define  BIG  1E37
#define  DOUBLE_EPS 1E-9
#define  FLOAT_EPS 1E-5
#define  NO   	0
#define  YES  	1
#define  pi  	3.1415927
#define  rad 	57.29577951
#define  byte unsigned char
#define  Boolean byte
#define  round(x) ((int)(x+0.5))
#define  sqr(x) (x*x)



/********** type-definitions *************************/

typedef float *Fvector, **Fmatrix;
typedef byte *Bvector, **Bmatrix;
typedef int  *Ivector, **Imatrix;
typedef double *Dvector, **Dmatrix;



/************** vector-creating routines  ***********************/

Fvector fvector(n)
int n;
{
	Fvector v;
	v=(float *)halloc(n,sizeof(float));
	if (!v) perr(HE_MSG,"allocation failure in fvector()");
	return v;
}

Ivector ivector(n)
int n;
{
	Ivector v;
	v=(int *)halloc(n,sizeof(int));
	if (!v) perr(HE_MSG,"allocation failure in ivector()");
	return v;
}


Bvector bvector(n)
int n;
{
	Bvector v;
	v=(byte *)halloc(n,sizeof(byte));
	if (!v) perr(HE_MSG,"allocation failure in bvector()");
	return v;
}

Dvector dvector(n)
int n;
{
	Dvector v;
	v=(double *)halloc(n,sizeof(double));
	if (!v) perr(HE_MSG,"allocation failure in dvector()");
	return v;
}


void shift_vector(v,l,byt)
int		l, byt;
Bvector	*v;
{
	*v -= l*byt;  
}


/******************* matrix-creating routines **************************/

Fmatrix fmatrix(nr,nc)
int nr,nc;
{
	Fmatrix  m;
	int  i;

	m=(float **) halloc(nr,sizeof(float*));
	if (!m) perr(HE_MSG,"allocation failure in fmatrix()");
	for(i=0;i<nr;i++) m[i]=fvector(nc);
	return m;
}

Imatrix imatrix(nr,nc)
int nr,nc;
{
	Imatrix  m;
	int  i;

	m=(int **) halloc(nr,sizeof(int*));
	if (!m) perr(HE_MSG,"allocation failure in imatrix()");
	for(i=0;i<nr;i++) m[i]=ivector(nc);
	return m;
}


Dmatrix dmatrix(nr,nc)
int nr,nc;
{
	Dmatrix  m;
	int  i;

	m=(double **) halloc(nr,sizeof(double*));
	if (!m) perr(HE_MSG,"allocation failure in dmatrix()");
	for(i=0;i<nr;i++) m[i]=dvector(nc);
	return m;
}

Bmatrix bmatrix(nr,nc)
int nr,nc;
{
	Bmatrix  m;
	int  i;

	m=(byte **) halloc(nr,sizeof(byte*));
	if (!m) perr(HE_MSG,"allocation failure in bmatrix()");
	for(i=0;i<nr;i++) m[i]=bvector(nc);
	return m;
}

/*
void shift_matrix(m,lr,r,lc,byt)
int		lr, r, lc, byt;
Bmatrix	*m;
{
	int	i;
	*m -= lr;
	for (i=lr;i<(lr+r);i++)
		(*m)[i] -= lc*byt;
}
*/

void shift_matrix(m,lr,byt)
int		lr, byt;
Bmatrix	*m;
{
	*m -= lr;
}

/******************** routines for destroying vectors and matrices. ********/
void free_fvector(v)
Fvector  v;
{
	free((float*) (v));
}


void free_bvector(v)
Bvector  v;
{
	free((byte*) (v));
}

void free_ivector(v)
Ivector v;
{
	free((int*) (v));
}

void free_dvector(v)
Dvector v;
{
	free((double*) v);
}


void free_fmatrix(m,nr)
Fmatrix  m;
int nr;
{
	int i;
	for(i=nr-1;i>=0;i--) free((float*) (m[i]));
	free((float**) m);
}

void free_imatrix(m,nr)
Imatrix  m;
int nr;
{
	int i;
	for(i=nr-1;i>=0;i--) free((int*) (m[i]));
	free((int**) m);
}

void free_dmatrix(m,nr)
Dmatrix  m;
int nr;
{
	int i;
	for(i=nr-1;i>=0;i--) free((double*) (m[i]));
	free((double**) m);
}

void free_bmatrix(m,nr)
Bmatrix  m;
int nr;
{
	int i;
	for(i=nr-1;i>=0;i--) free((byte*) (m[i]));
	free((byte**) m);
}



/****************** Read-routines ****************************/

void fread_vec(fil,v,n,byt)
Bvector	v;
int		n, byt;
FILE		*fil;
{
   if (fread(v,byt,n,fil) != n)
      perr(HE_MSG,"error during read");
}


void fread_to_fvec(fil,v,n,method)
Fvector	v;
int  	n, method;
FILE		*fil;
{
	int  i; 
	Dvector Ddum;
	Bvector Bdum;
	Ivector Idum;
	switch (method){
	case 0: 	Bdum=bvector(n);
			fread_vec(fil,Bdum,n,sizeof(byte));
			for (i=0;i<n;i++) v[i]=(float)Bdum[i];
			free_bvector(Bdum);
			break;
	case 2:	Idum=ivector(n);
			fread_vec(fil,(Bvector) Idum,n,sizeof(int));
			for (i=0;i<n;i++) v[i]=(float)Idum[i];
			free_ivector(Idum);
			break;
	case 6:	Ddum=dvector(n);
			fread_vec(fil,(Bvector) Ddum,n,sizeof(double));
			for (i=0;i<n;i++) v[i]=(float)Ddum[i];
			free_dvector(Ddum);
			break;
	case 3:	fread_vec(fil,(Bvector) v,n,sizeof(float));
	}
}


void fread_to_dvec(fil,v,n,method)
Dvector	v;
int  	n, method;
FILE		*fil;
{
	int  i; 
	Fvector Fdum;
	Bvector Bdum;
	Ivector Idum;
	switch (method){
	case 0: 	Bdum=bvector(n);
			fread_vec(fil,Bdum,n,sizeof(byte));
			for (i=0;i<n;i++) v[i]=(double)Bdum[i];
			free_bvector(Bdum);
			break;
	case 2:	Idum=ivector(n);
			fread_vec(fil,(Bvector) Idum,n,sizeof(int));
			for (i=0;i<n;i++) v[i]=(double)Idum[i];
			free_ivector(Idum);
			break;
	case 3:	Fdum=fvector(n);
			fread_vec(fil,(Bvector) Fdum,n,sizeof(float));
			for (i=0;i<n;i++) v[i]=(double)Fdum[i];
			free_fvector(Fdum);
			break;
	case 6:	fread_vec(fil,(Bvector) v,n,sizeof(double));
	}
}


void fread_mat(fil,m,nr,nc,byt)
Bmatrix  m;
int  nr, nc, byt;
FILE *fil;
{
	int  i; 
	for (i=0;i<nr;i++)
		if (fread(m[i],byt,nc,fil) != nc)
	perr(HE_MSG,"error during read");
}


void fread_to_fmat(fil,m,nr,nc,method)
Fmatrix m;
int  nr, nc, method;
FILE	*fil;
{
	int  i; 
	for (i=0;i<nr;i++)
		fread_to_fvec(fil,m[i],nc,method);
}


void fread_to_dmat(fil,m,nr,nc,method)
Dmatrix m;
int  nr, nc, method;
FILE	*fil;
{
	int  i; 
	for (i=0;i<nr;i++)
		fread_to_dvec(fil,m[i],nc,method);
}




/**********************  write-routines ****************************/

void fwrite_vec(fil,v,n,byt)
Bvector  	v;
int  	n, byt;
FILE 	*fil;
{
	if (fwrite(v,byt,n,fil) != n)
		perr(HE_MSG,"error during write");
}


void fwrite_from_fvec(fil,v,n,method)
Fvector 	v;
int  	n, method;
FILE		*fil;
{
	Dvector Ddum;
	Bvector Bdum;
	Ivector Idum;
	int  i;
	switch (method){
	case 0: 	Bdum=bvector(n);
			for (i=0;i<n;i++) Bdum[i]=(byte)v[i];
			fwrite_vec(fil,Bdum,n,sizeof(byte));
			free_bvector(Bdum);
			break;
	case 2: 	Idum=ivector(n);
			for (i=0;i<n;i++) Idum[i]=(int)v[i];
			fwrite_vec(fil,(Bvector) Idum,n,sizeof(int));
			free_ivector(Idum);
			break;
	case 3: 	fwrite_vec(fil,(Bvector) v,n,sizeof(float));
			break;
	case 6: 	Ddum=dvector(n);
			for (i=0;i<n;i++) Ddum[i]=(double)v[i];
			fwrite_vec(fil,(Bvector) Ddum,n,sizeof(double));
			free_dvector(Ddum);
			break;
    }
}


void fwrite_from_dvec(fil,v,n,method)
Dvector 	v;
int  	n, method;
FILE		*fil;
{
	Fvector Fdum;
	Bvector Bdum;
	Ivector Idum;
	int  i;
	switch (method){
	case 0: 	Bdum=bvector(n);
			for (i=0;i<n;i++) Bdum[i]=(byte)v[i];
			fwrite_vec(fil,Bdum,n,sizeof(byte));
			free_bvector(Bdum);
			break;
	case 2: 	Idum=ivector(n);
			for (i=0;i<n;i++) Idum[i]=(int)v[i];
			fwrite_vec(fil,(Bvector) Idum,n,sizeof(int));
			free_ivector(Idum);
			break;
	case 3: 	Fdum=fvector(n);
			for (i=0;i<n;i++) Fdum[i]=(float)v[i];
			fwrite_vec(fil,(Bvector) Fdum,n,sizeof(float));
			free_fvector(Fdum);
			break;
	case 6: 	fwrite_vec(fil,(Bvector) v,n,sizeof(double));
			break;
    }
}


void fwrite_mat(fil,m,nr,nc,byt)
Bmatrix  	m;
int  	nr, nc, byt;
FILE 	*fil;
{
	int  i;
	for (i=0;i<nr;i++)
		if (fwrite(m[i],byt,nc,fil) != nc)
	perr(HE_MSG,"error during write");
}


void fwrite_from_fmat(fil,m,nr,nc,method)
Fmatrix	m;
int  	nr, nc, method;
FILE		*fil;
{
	int	i;
	for (i=0;i<nr;i++)
		fwrite_from_fvec(fil,m[i],nc,method);
}


void fwrite_from_dmat(fil,m,nr,nc,method)
Dmatrix	m;
int  	nr, nc, method;
FILE		*fil;
{
	int	i;
	for (i=0;i<nr;i++)
		fwrite_from_dvec(fil,m[i],nc,method);
}


byte number2(i,argc,argv)
int	argc, i;
char	*argv[];
{
	int  	j;
	char		*s;

	if (i>=argc)
		return(0);  
	s=argv[i];
	if (s[0]=='-') 
		j=1;
	else j=0;
	while (s[j]!='\0'){ 
		if ( s[j]!='.' && s[j]!=',' && (s[j]<'0' || s[j]>'9')) 
			return(0);
		j++;
		}
	return(1);
}

byte number(s)
char *s;
{
	int  	i;

	if (s[0]=='-') i=1;
	else i=0;
	while (s[i]!='\0'){ 
		if ((s[i]!='.')&&(s[i]!=',')&&((s[i]<'0')||(s[i]>'9'))) 
			return(FALSE);
		i++;
		}
	return(TRUE);
}


