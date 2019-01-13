/*
 * Copyright (c) 1991
 *
 *	Allan Aasbjerg Nielsen
 *	Rasmus Larsen
 *	IMSOR/TUD
 *	The Institute of Mathematical Statististics and Operations Research
 *	The Technical University of Denmark
 * 
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 * 
 */
/*
 *  pcdecorr.c - RGB to principal components (PCs), stretch PCs, PCs to RGB
 *
 *  Usage: pcdecorr [-s s1 s2 s3] [-c]
 *
 *  defaults: s1 scaling factor for PC1 (defaults to 1.0)
 *	      s2 scaling factor for PC2 (defaults to 0.5)
 *	      s3 scaling factor for PC3 (defaults to 0.2)
 *  -c        if present numcolor=3 in outseq
 *
 *  cc pcdecorr.c /ext1/math/eispack.a -o pcdecorr -lhips -lm -lcl -O -v
 *
 *  HIPS-2 10.12.92 /AA (added -c)
 */

#define	Byte	unsigned char
#define	MAX_DIM 3

#include <hipl_format.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void tred2();
void tql2();

char *Progname;

char usage[]="Usage: pcdecorr [-s s1 s2 s3] [-c]";

int main(argc,argv)
int argc;
char *argv[];
{
	struct	header hd;
	int	nr,nc,nf,np,i,p,k;
	Byte	**rgb,**prgb;
	float	**pcs,**ppcs;
	Byte	*maskrgb,maskvalue='0';
	int	niterations;
	int	lb_rows,lb_cols,ub_rows,ub_cols,shift[2]; 
	double	mean[3],cov[3][3],corr[3][3];
	double	lambda[3],fv1[3],trans[3][3];
	int	nm=3,ierr;
	double	pc[3],scale[3];
	int	Mflag=0;
	int	colflag=0;

	void	initmat();
	int	calccov();
	void	calccorr();

	Progname = strsave(*argv);

	scale[0]=1.0;
	scale[1]=0.5;
	scale[2]=0.2;

	for (i=0;i<argc;i++){
		if (argv[i][0] == '-')
		switch (argv[i][1]) {
		case 's': scale[0]=atof(argv[++i]);
			  scale[1]=atof(argv[++i]);
			  scale[2]=atof(argv[++i]);
			  break;
		case 'c': colflag=1;
			  break;
		case 'U':
		default:  fprintf(stdout,"%s: %s\n",Progname,usage); exit(1);
			  break;
		}
	}
	fprintf(stderr,"%s: scales are %f %f %f\n",Progname,scale[0],scale[1],scale[2]);

	read_header(&hd);
	if (hd.pixel_format != PFBYTE) {
	   	fprintf(stdout,"%s: image must be in byte format\n",Progname);
		exit(1);
	}
	nf = hd.num_frame;
	if (nf != 3) {
	   fprintf(stdout,"%s: implemented for 3-frame sequence only\n",Progname);
	   exit(1);
	}
	nr = hd.orows;
	nc = hd.ocols;
	np = nr * nc;
	if (colflag==1) hd.numcolor=3;
	hd.pixel_format = PFFLOAT;
	update_header(&hd,argc,argv);
	write_header(&hd);

	rgb = (Byte **) halloc(nf,sizeof(Byte *));
	prgb= (Byte **) halloc(nf,sizeof(Byte *));
	pcs = (float **) halloc(nf,sizeof(float *));
	ppcs= (float **) halloc(nf,sizeof(float *));
	for (i=0;i<nf;i++) {
		rgb[i] = (Byte *) halloc(np,sizeof(Byte));
		pcs[i] = (float *) halloc(np,sizeof(float));
	}

	/* read frame */
	for (i=0;i<nf;i++)
		if (fread(rgb[i],sizeof(Byte),np,stdin) != np) {
	           fprintf(stdout,"%s: error reading frame %d\n",Progname,i);
		   exit(1);
		}
		
   	for (i=0;i<nf;i++) {
   		ppcs[i] = pcs[i];
   		prgb[i] = rgb[i];
	}

	initmat(cov,nf,nf); initmat(corr,nf,nf);
	for (i=0;i<nf;i++) mean[i]=0.0;
	lb_rows=0;
	lb_cols=0;
	ub_rows=nr;
	ub_cols=nc;
	shift[0]=shift[1]=0;
	niterations=calccov(cov,mean,nf,rgb,Mflag,maskrgb,maskvalue,nr,nc,lb_rows,lb_cols,ub_rows,ub_cols,shift); 
	for (i=0;i<nf;i++) for (k=0;k<nf;k++) cov[i][k] /= (niterations-1);
	/*
	calccorr(cov,corr,nb);
	fprintf(stderr,"Nobs=%d\n",niterations);
	for (i=0;i<nf;i++) {
		fprintf(stderr,"%f\n",mean[i]);
		fprintf(stderr,"%f %f %f\n",cov[i][0],cov[i][1],cov[i][2]);
		fprintf(stderr,"%f %f %f\n",corr[i][0],corr[i][1],corr[i][2]);
	}
	*/

	/* tred2(&nm,&nf,(cflag) ? corr:cov,lambda,fv1,trans); */
	tred2(&nm,&nf,cov,lambda,fv1,trans);
	tql2(&nm,&nf,lambda,fv1,trans,&ierr);

	fprintf(stderr,"%s: eigenvalues are %f %f %f\n",Progname,lambda[2],lambda[1],lambda[0]);
	/*
	for (i=0;i<nf;i++) {
		fprintf(stderr,"%f\n",lambda[i]);
		fprintf(stderr,"%f %f %f\n",trans[i][0],trans[i][1],trans[i][2]);
	}
	*/

   	for (p=0;p<np;p++,prgb[0]++,prgb[1]++,prgb[2]++,ppcs[0]++,ppcs[1]++,ppcs[2]++) {
	    pc[0]=trans[2][0]*((float)(*prgb[0])-mean[0])+trans[2][1]*((float)(*prgb[1])-mean[1])+trans[2][2]*((float)(*prgb[2])-mean[2]);
	    pc[1]=trans[1][0]*((float)(*prgb[0])-mean[0])+trans[1][1]*((float)(*prgb[1])-mean[1])+trans[1][2]*((float)(*prgb[2])-mean[2]);
	    pc[2]=trans[0][0]*((float)(*prgb[0])-mean[0])+trans[0][1]*((float)(*prgb[1])-mean[1])+trans[0][2]*((float)(*prgb[2])-mean[2]);

	    /*
	    *ppcs[0] = pc[0];
	    *ppcs[1] = pc[1];
	    *ppcs[2] = pc[2];
	    */

	    for (i=0;i<nf;i++) pc[i]*=scale[i]/sqrt(lambda[nf-1-i]);

	    *ppcs[0] = trans[2][0]*pc[0]+trans[1][0]*pc[1]+trans[0][0]*pc[2];
	    *ppcs[1] = trans[2][1]*pc[0]+trans[1][1]*pc[1]+trans[0][1]*pc[2];
	    *ppcs[2] = trans[2][2]*pc[0]+trans[1][2]*pc[1]+trans[0][2]*pc[2];
	}

	for (i=0;i<nf;i++) {
		if (fwrite(pcs[i],sizeof(float),np,stdout) != np) {
		   fprintf(stdout,"%s: error writing frame %d\n",Progname,i);
		   exit(1);
		}
	}
exit(0);
}

/* initmat: sets matrix elements to zero */
void initmat(mat,rows,cols)
     double (*mat)[MAX_DIM];
     int    rows,cols;
     {
	int i,k;
	for (i=0;i<rows;i++) for (k=0;k<cols;k++) mat[i][k] = 0;
     }

/* calccov: calculates covariance matrix */
int calccov(cov,mean,dim,buffer,Mflag,maskbfr,maskvalue,rows,cols,lb_rows,lb_cols,ub_rows,ub_cols,shift)
    double (*cov)[MAX_DIM],*mean;
    int    dim,shift[2];
    int    rows,cols;
    int    lb_rows,lb_cols,ub_rows,ub_cols;
    int    Mflag;
    Byte   **buffer;
    Byte   *maskbfr;
    Byte   maskvalue;

    {   double meanold[MAX_DIM];
	int    i,k,n,m,flag,index1,index2,niterations = 0;
	double valk1,valk2;
	double vali1,vali2;

	if (shift[0] == 0 && shift[1] == 0) flag =0; 
	else {
	   flag = 1;
	   if (lb_rows < -shift[1]) lb_rows = - shift[1];
	   if (lb_cols < -shift[0]) lb_cols = - shift[0];
	   if (rows-ub_rows < shift[1]) ub_rows = rows- shift[1];
	   if (cols-ub_cols < shift[0]) ub_cols = cols- shift[0];
	}

	for (i=0;i<dim;i++) meanold[i] = 0.0;

	for (m=lb_rows;m<ub_rows;m++) for (n=lb_cols;n<ub_cols;n++) {
	   index1 = m*cols + n;  
	   if(Mflag && maskbfr[index1] != maskvalue) continue;
	   if (flag == 1) 
	      index2 = index1 + shift[0] + shift[1] * cols;
			      
	   niterations += 1;

	   for (i=0;i<dim;i++) for (k=i;k<dim;k++) {
	      valk1 = buffer[k][index1];
	      vali1 = buffer[i][index1];
	      if (flag == 0) valk2 = vali2 = 0;
	      else {
		 valk2 = buffer[k][index2];
		 vali2 = buffer[i][index2];
	      }
	      if (i==0) {
		 meanold[k] = mean[k];
		 mean[k] += (valk1-valk2-mean[k])/niterations;
	      }
	      cov[i][k] += (valk1-valk2-meanold[k])*(vali1-vali2-mean[i]);
	   }
	}
	for (i=0;i<dim;i++) {
	   for (k=0;k<i;k++) cov[i][k] = cov[k][i];
	}
	return niterations;
     }

/* calccorr: calculates correlation matrix */
void calccorr(mat1,mat2,dim)
     double (*mat1)[MAX_DIM];
     double (*mat2)[MAX_DIM];
     int    dim;
     {
	int i,k;
	double temp;

	for (i=0;i<dim;i++) for (k=0;k<dim;k++) {
	   temp = mat1[i][i]*mat1[k][k];
	   mat2[i][k] = mat1[i][k]/sqrt(temp);
        }
     }
