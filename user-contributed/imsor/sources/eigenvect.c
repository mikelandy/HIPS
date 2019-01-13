#include <stdio.h>
#include <math.h>
#include <hipl_format.h>

#define maxiter         50
#define epsilon         5e-10


double sturmseq(mat, nband, x, signs)
double  *mat;
int     nband;
double  x;
int     *signs;
{
        int     i;
        double pk0, pk1, pk2, TEMP;

        pk2=1.0;
        pk1=x - mat[0];
        if (pk1 < 0)
                *signs=1;
        else
                *signs=0;
        for (i=1; i < nband; i++) {
                TEMP=mat[i*nband+i - 1];
                pk0=(x - mat[i*nband+i])*pk1 - TEMP*TEMP*pk2;
                pk2=pk1;
                pk1=pk0;
                if ((pk1 < 0 && pk2 > 0) || (pk1 > 0 && pk2 < 0))
                        (*signs)++;
        }
        return pk1;
}


void gausselimination(mat, nband, ls)
double  *mat;
double  *ls;
int     nband;
{
        int     i, j, k;
        double  aa;

        /* Forward elimination */
        for (i=0; i < nband; i++) {
                k=i;
                for (j=i+1; j < nband; j++) {
                        if (fabs(mat[j*nband+i]) > fabs(mat[j*nband+k]))
                                k=j;
                }
                /* interchange row i and k */
                for (j=i; j < nband; j++) {
                        aa=mat[i*nband+j];
                        mat[i*nband+j]=mat[k*nband+j];
                        mat[k*nband+j]=aa;
                }
                aa=ls[i];
                ls[i]=ls[k];
                ls[k]=aa;
                /* end of interchange */
                if (mat[i*nband+i]==0) 
                        return;	/* was return(-1) */
                else
                        aa = 1/mat[i*nband+i];
                for (j=i; j < nband; j++)
                        mat[i*nband+j] *= aa;
                ls[i] *= aa;
                for (j=i+1; j < nband; j++) {
                        aa = -mat[j*nband+i];
                        for (k=i; k < nband; k++)
                                mat[j*nband+k] += aa*mat[i*nband+k];
                        ls[j] += aa*ls[i];
                }
        }
        /* Backward elimination */
        for (i=nband - 1; i >= 0; i--) {
                for (j=i - 1; j >= 0; j--) {
                        aa = -mat[j*nband+i];
                        for (k=j+1; k < nband; k++)
                                mat[j*nband+k] += aa*mat[i*nband+k];
                        ls[j] += aa*ls[i];
                }
        }
}


void eigenvectors(s2,nband,eigenvect,eigenval)
 double *s2, *eigenvect, *eigenval;
 int    nband;
{
        int     i, j, k, m, n, iter;
        int     signs;
        double  lngth, TEMP, mineigv, maxeigv, eigenvalue, x, maxval;
        double  *tridiag, *leftside, *w, *darr1, *darr2;

        tridiag = (double *)malloc(nband*nband*sizeof(double));
        darr1   = (double *)malloc(nband*nband*sizeof(double));
        darr2   = (double *)malloc(nband*nband*sizeof(double));
        leftside= (double *)malloc(nband*sizeof(double));
        w       = (double *)malloc(nband*sizeof(double));

        /* Init the (in the end-) tridiagonal matrix with upper triangle of s2 */
        for (i=0; i < nband; i++) {
                for (j=i; j < nband; j++) {
                        tridiag[i*nband+j]=s2[i*nband+j];
                        s2[j*nband+i]     =s2[i*nband+j];
                        tridiag[j*nband+i]=tridiag[i*nband+j];
                }
        }


        /* Transform tridiag into a tridiagonal matrix via similar matrices */
        /* Householder's method is used */
        for (i=1; i <= nband - 2; i++) {
                /* Find w (a vector) */
                for (j=i; j < nband; j++)
                        w[j]=tridiag[(i - 1)*nband+j];
                lngth=0.0;
                for (j=i; j < nband; j++) {
                        TEMP=w[j];
                        lngth += TEMP*TEMP;
                }
                lngth=sqrt(lngth);
                if (w[i] > 0)
                        tridiag[(i - 1)*nband+i] = -lngth;
                else
                        tridiag[(i - 1)*nband+i] = lngth;
                tridiag[i*nband+i - 1]=tridiag[(i - 1)*nband+i];
                for (j=i+1; j < nband; j++) {
                        tridiag[(i - 1)*nband+j]=0.0;
                        tridiag[j*nband+i - 1]=0.0;
                }
                TEMP=sqrt(2*lngth*(lngth+fabs(w[i])));
                w[i]=(w[i] - tridiag[(i - 1)*nband+i]) / TEMP;
                for (j=i+1; j < nband; j++)
                        w[j] /= TEMP;

                /* Form darr1=I - 2 W W' */
                for (j=i; j < nband; j++) {
                        for (k=i; k <= j; k++) {
                                darr1[j*nband+k] = -2*w[j]*w[k];
                                if (j == k)
                                        darr1[j*nband+k]++;
                                else
                                        darr1[k*nband+j]=darr1[j*nband+k];
                        }
                }

                /* Make tridiag_i+1=Darr1 tridiag_i Darr1 */
                for (j=i; j < nband; j++) {
                        for (m=i; m < nband; m++) {
                                TEMP=0.0;
                                for (n=i; n < nband; n++)
                                        TEMP += tridiag[j*nband+n]*darr1[n*nband+m];
                                darr2[j*nband+m]=TEMP;
                        }
                }
                for (j=i; j < nband; j++) {
                        for (m=i; m < nband; m++) {
                                TEMP=0.0;
                                for (n=i; n < nband; n++)
                                        TEMP += darr1[j*nband+n]*darr2[n*nband+m];
                                tridiag[j*nband+m]=TEMP;
                        }
                }

        }

        /* Find the range of the eigen values of tridiag */
        mineigv=tridiag[0] - fabs(tridiag[1]);
        maxeigv=tridiag[0]+fabs(tridiag[1]);
        for (i=1; i < nband; i++) {
                TEMP=fabs(tridiag[i*nband+i - 1]);
                if (i < nband - 1)
                        TEMP += fabs(tridiag[i*nband+i+1]);
                if (mineigv > tridiag[i*nband+i] - TEMP)
                        mineigv=tridiag[i*nband+i] - TEMP;
                if (maxeigv < tridiag[i*nband+i]+TEMP)
                        maxeigv=tridiag[i*nband+i]+TEMP;
        }

        /* Find eigenvalues using Sturm sequences
           and a VERY primitive bisection search,
           Find eigenvectors */
        for (i=nband - 1; i >= 0; i--) {
                x=maxeigv;
                eigenvalue=x;
                do {
                        sturmseq(tridiag, nband, eigenvalue, &signs);
                        if (signs > i)
                                mineigv=eigenvalue;
                        else
                                x=eigenvalue;
                        eigenvalue=(x+mineigv) / 2;
                } while (fabs(x - mineigv) > epsilon*(1+fabs(x)));

                /* Find the eigenvector */
                for (m=0; m < nband; m++)
                        leftside[m]= H__RANDOM()/30000.0;/* It should be checked
                                                        that all eigenvectors
                                                        are orthogonal */
                iter=0;
                do {
                        for (m=0; m < nband; m++) {
                                for (n=0; n < nband; n++) {
                                        if (m == n)
                                                darr1[m*nband+n]=s2[m*nband+n] - eigenvalue;
                                        else
                                                darr1[m*nband+n]=s2[m*nband+n];
                                }
                        }
                        for (m=0; m < nband; m++)
                                w[m]=leftside[m];
                        gausselimination(darr1, nband, leftside);
                        maxval=0.0;
                        for (m=0; m < nband; m++) {
                                if (fabs(leftside[m]) > fabs(maxval))
                                        maxval=leftside[m];
                        }
                        for (m=0; m < nband; m++)
                                leftside[m] /= maxval;
                        maxval=fabs(w[0] - leftside[0]);
                        for (m=0; m < nband; m++) {
                                if (fabs(w[m] - leftside[m]) > maxval)
                                        maxval=fabs(w[m] - leftside[m]);
                        }
                        iter++;
                } while (maxval >= 100*epsilon && iter <= maxiter);




                for (m=0; m<nband; m++)
                        eigenvect[i*nband+m] = leftside[m];
                eigenval[i] = eigenvalue;       /* this could be improved by
                                                   using Rayleigh coeff, but it
                                                   is not important here */
        }


        free(w);
        free(leftside);
        free(darr2);
        free(darr1);
        free(tridiag);

}
