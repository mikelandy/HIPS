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
 *  decorr.c - RGB to min/max autocorrelation factors (MAFs), stretch MAFs, MAFs to RGB
 *
 *  Usage: decorr [-s s1 s2 s3] [-p [-c]] [-1 | -I]
 *                [-e [nr nc [sr sc]] | -M mask_file [mask_value]] [-color]
 *
 *  defaults: s1 scaling factor for MAF1 (defaults to 1.0)
 *	      s2 scaling factor for MAF2 (defaults to 0.5)
 *	      s3 scaling factor for MAF3 (defaults to 0.2)
 *
 *  -p	      use principal components instead of MAFs
 *  -c	      use principal components based on the correlation matrix
 *  -1	      replace MAF1/PC1 with this image (same size and format)
 *  -I	      same as -1
 *  -e	      extract statistics within a rectangular area only
 *  -M	      extract statistics where mask_file has value mask_value only
 *  -color    numcolor=3 in outseq
 *
 *  cc decorr.c /ext1/math/eispack.a /ext1/math/linpack.a -o decorr -lhips -lm -lcl -O -v
 *
 * HIPS-2 11.12.92 /AA (-color added)
 */

#define	Byte	unsigned char
#define	MAX_DIM 3

#include <hipl_format.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <math.h>

void tred2();
void tql2();
void reduc();
void tred2();
void tql2();
void rebak();
void dgefa();
void dgedi();

char usage[]="Usage: decorr [-s s1 s2 s3] [-p [-c]] [-I | -1] [-e [nr nc [sr sc]] | -M mask_file [mask_value]] [-color]";

int main(argc,argv)
int argc;
char *argv[];
{
	struct	header hd,hd1,mask;
	int	nr,nc,nf,np,i,p,k,j;
	int	nrsta,ncsta,srsta,scsta;
	Byte	**rgb,**prgb,*new1;
	float	**pcs,**ppcs,**pwk;
	Byte	*maskfr,*pmaskfr,maskvalue=(Byte)0;
	int	niterations,niter,nshifts,shifts[2][2];
	int	lb_rows,lb_cols,ub_rows,ub_cols,shift[2]; 
	double	mean[3],stddev[3],cov[3][3],corr[3][3];
	double	mn[3],covnoise[3][3],covnoise1[3][3],corrnoise[3][3];
	double	lambda[3],sqlambda[3],fv1[3],fv2[3],trans[3][3];
	int	nm=3,ierr;
	double	pc[3],scale[3];
	double	det[2],work[3];
	int	startstaflag=0,eflag=0,Mflag=0,pflag=0,cflag=0,flag1=0;
	FILE	*maskfile,*file1;
	char	*maskfname,*f1name;
	int	ipvt[3];
	int	colflag=0;

	void	initmat();
	int	calccov();
	void	calccorr();

	Progname = strsave(*argv);

	scale[0]=1.0;
	scale[1]=0.5;
	scale[2]=0.2;
	j=0;
	for (i=1;i<argc;i++) {
		if (argv[i][0]=='-') {
			switch (argv[i][1]) {
			case 's': scale[0]=atof(argv[++i]);
			          scale[1]=atof(argv[++i]);
			          scale[2]=atof(argv[++i]);
				  break;
			case '1': case 'I':
				  flag1=1;
				  fprintf(stderr,"%s: -I/-1 not properly implemented;\nthe user must handle variance of replacement image\n",Progname);
				  f1name=argv[++i];
				  if ((file1=fopen(f1name,"r")) == NULL) {
				     fprintf(stderr,"%s: can't open %s\n",Progname,f1name);
				     exit(1);
				  }
				  break;
			case 'p': pflag=1; break;
			case 'c': if (strncmp(argv[i],"-co",3) == 0) colflag=1;
				  else cflag=1;
				  break;
			case 'e':
				  eflag=1;
				  fprintf(stderr,"%s: i+2 = %d, argc = %d\n",Progname,i+2,argc);
				  if ((i+2)<argc && argv[i+1][0]!='-' && argv[i+2][0]!='-') {
					if ((nrsta=atoi(argv[++i]))<=0) {
					   fprintf(stderr,"%s: number of rows in -e must be >0\n",Progname);
					   exit(1);
					}
					if ((ncsta=atoi(argv[++i]))<=0) {
					   fprintf(stderr,"%s: number of cols in -e must be >0\n",Progname);
					   exit(1);
					}
				  }
				  fprintf(stderr,"%s: i+2 = %d, argc = %d\n",Progname,i+2,argc);
				  if ((i+2)<argc && argv[i+1][0]!='-' && argv[i+2][0]!='-') {
					srsta=atoi(argv[++i]);
					scsta=atoi(argv[++i]);
					startstaflag=1;
				  }
				  break;
			case 'M':
				  Mflag=1;
				  maskfname=argv[++i];
				  if ((maskfile=fopen(maskfname,"r")) == NULL)
				     perr(HE_MSG,"unable to open maskfile");
				  if ((i+1)<argc && argv[i+1][0]!='-')
				     maskvalue=(Byte)atoi(argv[++i]);
				  fprintf(stderr,"%s: maskvalue = %d\n",Progname,maskvalue);
				  break;
			case 'D': break;
			case 'U':
			default:  fprintf(stderr,"%s: %s\n",Progname,usage); exit(1);
				  break;
			}
		}
		else {
			j += 1;
			if (j==1) scale[0]=atof(argv[i]);
			if (j==2) scale[1]=atof(argv[i]);
			if (j==3) scale[2]=atof(argv[i]);
		}
	}
	if (pflag==0) {
	   fprintf(stderr,"%s: the MAF colour decorrelation does not work so far\n",Progname);
	   exit(1);
	}
	if (cflag==1 && pflag==0) {
	   fprintf(stderr,"%s: -c allowd only with -p\n",Progname);
	   exit(1);
	}
	fprintf(stderr,"%s: scales are %f %f %f\n",Progname,scale[0],scale[1],scale[2]);
	/*
	fprintf(stderr,"pflag=%d\ncflag=%d\n",pflag,cflag);
	*/

	read_header(&hd);
	if (hd.pixel_format != PFBYTE) {
	   	fprintf(stderr,"%s: image must be in byte format\n",Progname);
		exit(1);
	}
	nr = hd.orows;
	nc = hd.ocols;
	np = nr * nc;
	nf = hd.num_frame;
	if (nf != 3) {
	   fprintf(stderr,"%s: implemented for 3-frame sequence only\n",Progname);
	   exit(1);
	}
	if (colflag==1) hd.numcolor=3;
	hd.pixel_format = PFFLOAT;
	update_header(&hd,argc,argv);
	write_header(&hd);

	rgb = (Byte **) halloc(nf,sizeof(Byte *));
	prgb= (Byte **) halloc(nf,sizeof(Byte *));
	pcs = (float **) halloc(nf,sizeof(float *));
	ppcs= (float **) halloc(nf,sizeof(float *));
	pwk = (float **) halloc(nf,sizeof(float *));
	for (i=0;i<nf;i++) {
		rgb[i] = (Byte *) halloc(np,sizeof(Byte));
		pcs[i] = (float *) halloc(np,sizeof(float));
		pwk[i] = (float *) halloc(np,sizeof(float));
	}

	/* read mask */
	if (Mflag==1){
	fread_header(maskfile,&mask,maskfname);
	if ( (mask.orows != nr) || (mask.ocols != nc) ) {
	   fprintf(stderr,"%s: mismatch between mask size and image size\n",Progname);
	   exit(1);
	}
	if (mask.pixel_format != PFBYTE) {
	   fprintf(stderr,"%s: mask image must be byte\n",Progname);
	   exit(1);
	}
	if (mask.num_frame != 1)
	   fprintf(stderr,"%s: more then one frame in mask image\n",Progname);
	maskfr = (Byte *) halloc(np,sizeof(Byte));
	if (fread(maskfr,sizeof(Byte),np,maskfile) != np) {
	   fprintf(stderr,"%s: error reading mask\n",Progname);
	   exit(1);
	}
	}

	if (eflag == 1) {
		if (Mflag == 1) {
		fprintf(stderr,"%s: -M and -e not allowed simultaneously\n",Progname);
		exit(1);
		}
		if (nrsta==0 || ncsta==0) {
		   nrsta=nr/2;
		   ncsta=nc/2;
		}
		if (startstaflag == 0) {
		   srsta=(nr-nrsta)/2;
		   scsta=(nc-ncsta)/2;
		}
		if ( (srsta+nrsta)>nr || (scsta+ncsta)>nc ) {
		   fprintf(stderr,"%s: wrong specifications for -e\n",Progname);
		   exit(1);
		}
		fprintf(stderr,"%s: %d %d %d %d\n",Progname,nrsta,ncsta,srsta,scsta);
		Mflag=1;
		maskfr = (Byte *) halloc(np,sizeof(Byte));
		pmaskfr=maskfr;
		for (i=0;i<nr;i++) {
		    for (j=0;j<nc;j++) {
			*pmaskfr=(Byte)255;
			if ( i>=srsta && i<(srsta+nrsta) &&
			     j>=scsta && j<(scsta+ncsta) )
				*pmaskfr=(Byte)0;
			pmaskfr++;
		    }
		}
	}
	pmaskfr=maskfr;

	/* read frame */
	for (i=0;i<nf;i++)
		if (fread(rgb[i],sizeof(Byte),np,stdin) != np) {
	        	fprintf(stderr,"%s: error reading frame %d\n",Progname,i);
			exit(1);
		}
		
   	for (i=0;i<nf;i++) {
   		ppcs[i] = pcs[i];
   		prgb[i] = rgb[i];
	}

	/* read new MAF1/PC1 HIPS file if wanted */

	if (flag1==1) {
		fread_header(file1,&hd1,f1name);
		if (nr != hd1.orows || nc != hd1.ocols) {
			fprintf(stderr,"%s: new MAF1/PC1 image must have same size as RGB image\n",Progname);
			exit(1);
		}
		if (hd1.pixel_format != PFBYTE) {
		   fprintf(stderr,"%s: new MAF1/PC1 image must be byte format\n",Progname);
		   exit(1);
		}
		new1 = (Byte *) halloc(np,sizeof(Byte));
		if (fread(new1,sizeof(Byte),np,file1) != np) {
	           fprintf(stderr,"%s: error reading new MAF1/PC1 image\n",Progname);
		   exit(1);
		}
	}

	initmat(cov,nf,nf); if (cflag==1) initmat(corr,nf,nf);
	for (i=0;i<nf;i++) mean[i]=0.0;
	lb_rows=0;
	lb_cols=0;
	ub_rows=nr;
	ub_cols=nc;
	shift[0]=shift[1]=0;
	niterations=calccov(cov,mean,nf,rgb,Mflag,maskfr,maskvalue,nr,nc,lb_rows,lb_cols,ub_rows,ub_cols,shift); 
	for (i=0;i<nf;i++) for (k=0;k<nf;k++) cov[i][k] /= (niterations-1);
	if (cflag==1) {
		calccorr(cov,corr,nf);
		for (i=0;i<nf;i++) stddev[i]=sqrt(cov[i][i]);
	}
	/*
	fprintf(stderr,"Nobs=%d\n",niterations);
	for (i=0;i<nf;i++) {
		fprintf(stderr,"%f\n",mean[i]);
		fprintf(stderr,"%f %f %f\n",cov[i][0],cov[i][1],cov[i][2]);
		fprintf(stderr,"%f %f %f\n",corr[i][0],corr[i][1],corr[i][2]);
	}
	*/

	if (pflag==0) {
	nshifts=2;
	shifts[0][0]=shifts[1][1]=-1;
	niterations = 0;
	for (j=0;j<nshifts;j++) {
	   for (i=0; i<nf; i++) mn[i] = 0.0;
	   initmat(covnoise1,nf,nf); /* initmat(corrnoise,nf,nf); */

	   niter =calccov(covnoise1,mn,nf,rgb,Mflag,maskfr,maskvalue,nr,nc,lb_rows,lb_cols,ub_rows,ub_cols,shifts[j]);
	   niterations += niter;

	   for (i=0;i<nf;i++) for (k=0;k<nf;k++) {
	      covnoise[i][k] += covnoise1[i][k];
	      covnoise1[i][k] /= (niter - 1);
	   }
	   /* calccorr(covnoise1,corrnoise,nf); */
	}

	for (i=0;i<nf;i++) for (k=0;k<nf;k++)
	   covnoise[i][k] /= (niterations - nshifts);

	reduc(&nm,&nf,covnoise,cov,fv2,&ierr);
	if (ierr != 0) {
	   fprintf(stderr,"%s: covariance matrix is not positive definite\nCholesky factorization does not exist\n",Progname);
	   exit(1);
	}
	tred2(&nm,&nf,covnoise,lambda,fv1,trans);
	tql2(&nm,&nf,lambda,fv1,trans,&ierr);
	if (ierr != 0) {
	fprintf(stderr,"%s: more than 30 iterations to determine eigenvalue (%d)\neigenvalues and -vectors 1 to %d should be OK (but unordered)\n",Progname,ierr,ierr-1);
	exit(1);
	}
	rebak(&nm,&nf,cov,fv2,&nf,trans);
	/* Remember reverse order for MAFs.  Use fv1 and fv2 now that we have them */
	for (i=0;i<nf;i++) {
		fv1[i]=trans[2][i];
		trans[2][i]=trans[0][i];
		trans[0][i]=fv1[i];
	}
	fv2[0]=lambda[2];
	lambda[2]=lambda[0];
	lambda[0]=fv2[0];
	}
	else {
	tred2(&nm,&nf,(cflag) ? corr:cov,lambda,fv1,trans);
	tql2(&nm,&nf,lambda,fv1,trans,&ierr);
	if (ierr != 0) {
	fprintf(stderr,"%s: more than 30 iterations to determine eigenvalue (%d)\neigenvalues and -vectors 1 to %d should be OK (but unordered)\n",Progname,ierr,ierr-1);
	exit(1);
	}
	for (i=0;i<nf;i++) {
	    	sqlambda[i]=sqrt(lambda[nf-1-i]);
	}
	}
	fprintf(stderr,"%s: eigenvalues are ",Progname);
	for (i=0;i<nf;i++) fprintf(stderr,"%f ",lambda[nf-1-i]);
	fprintf(stderr,"\n");
	/*
	for (i=0;i<nf;i++) {
		fprintf(stderr,"%f\n",lambda[i]);
		fprintf(stderr,"%f %f %f\n",trans[i][0],trans[i][1],trans[i][2]);
	}
	*/
   	for (p=0;p<np;p++,prgb[0]++,prgb[1]++,prgb[2]++,ppcs[0]++,ppcs[1]++,ppcs[2]++,new1++) {
	    for (i=0;i<nf;i++) {
	   	*pwk[i] = (float)(*prgb[i]) - mean[i];
	    	if (cflag==1) *pwk[i] /= stddev[i];
	    }
	    if (flag1==1) pc[0]=(float)(*new1);
	    else
	    pc[0]=trans[2][0]*(*pwk[0]) + trans[2][1]*(*pwk[1]) + trans[2][2]*(*pwk[2]);
	    pc[1]=trans[1][0]*(*pwk[0]) + trans[1][1]*(*pwk[1]) + trans[1][2]*(*pwk[2]);
	    pc[2]=trans[0][0]*(*pwk[0]) + trans[0][1]*(*pwk[1]) + trans[0][2]*(*pwk[2]);

	    for (i=0;i<nf;i++) {
	    	if (pflag==1) pc[i]*=scale[i]/sqlambda[i];
		/* if (pflag==1) pc[i]*=scale[i]/sqrt(lambda[nf-1-i]); */
	        else          pc[i]*=scale[i];
	    }

	    /*
	    *ppcs[0] = pc[0];
	    *ppcs[1] = pc[1];
	    *ppcs[2] = pc[2];
	    */

	    if (pflag==1) {
	    *ppcs[0] = trans[2][0]*pc[0] + trans[1][0]*pc[1] + trans[0][0]*pc[2];
	    *ppcs[1] = trans[2][1]*pc[0] + trans[1][1]*pc[1] + trans[0][1]*pc[2];
	    *ppcs[2] = trans[2][2]*pc[0] + trans[1][2]*pc[1] + trans[0][2]*pc[2];
	    }
	    else {
	for (i=0;i<nf;i++) {
		fv1[i]=trans[2][i];
		trans[2][i]=trans[0][i];
		trans[0][i]=fv1[i];
	}
	    /* MAF transformation matrix is not orthogonal */
	    dgefa(trans,&nf,&nf,&ipvt,&ierr);
	    if (ierr != 0) {
	    fprintf(stderr,"%s: error in calculating inverse of MAF transformation matrix\n",Progname);
	    exit(1);
	    }
	    ierr=1; /* ierr is now a job specifier for dgedi */
	    dgedi(trans,&nf,&nf,&ipvt,det,work,&ierr);

	    /* ??? THIS PART DOES NOT WORK ??? !!! */

	    *ppcs[0] = trans[0][0]*pc[0] + trans[0][1]*pc[1] + trans[0][2]*pc[2];
	    *ppcs[1] = trans[1][0]*pc[0] + trans[1][1]*pc[1] + trans[1][2]*pc[2];
	    *ppcs[2] = trans[2][0]*pc[0] + trans[2][1]*pc[1] + trans[2][2]*pc[2];

	    /*

	    *ppcs[0] = trans[0][2]*pc[0] + trans[0][1]*pc[1] + trans[0][0]*pc[2];
	    *ppcs[1] = trans[1][2]*pc[0] + trans[1][1]*pc[1] + trans[1][0]*pc[2];
	    *ppcs[2] = trans[2][2]*pc[0] + trans[2][1]*pc[1] + trans[2][0]*pc[2];

	    *ppcs[0] = trans[0][0]*pc[0] + trans[0][1]*pc[1] + trans[0][2]*pc[2];
	    *ppcs[1] = trans[1][0]*pc[0] + trans[1][1]*pc[1] + trans[1][2]*pc[2];
	    *ppcs[2] = trans[2][0]*pc[0] + trans[2][1]*pc[1] + trans[2][2]*pc[2];

	    *ppcs[0] = trans[2][0]*pc[0] + trans[2][1]*pc[1] + trans[2][2]*pc[2];
	    *ppcs[1] = trans[1][0]*pc[0] + trans[1][1]*pc[1] + trans[1][2]*pc[2];
	    *ppcs[2] = trans[0][0]*pc[0] + trans[0][1]*pc[1] + trans[0][2]*pc[2];

	    *ppcs[0] = trans[2][2]*pc[0] + trans[2][1]*pc[1] + trans[2][0]*pc[2];
	    *ppcs[1] = trans[1][2]*pc[0] + trans[1][1]*pc[1] + trans[1][0]*pc[2];
	    *ppcs[2] = trans[0][2]*pc[0] + trans[0][1]*pc[1] + trans[0][0]*pc[2];

	    *ppcs[0] = trans[0][0]*pc[0] + trans[1][0]*pc[1] + trans[2][0]*pc[2];
	    *ppcs[1] = trans[0][1]*pc[0] + trans[1][1]*pc[1] + trans[2][1]*pc[2];
	    *ppcs[2] = trans[0][2]*pc[0] + trans[1][2]*pc[1] + trans[2][2]*pc[2];

	    *ppcs[0] = trans[0][2]*pc[0] + trans[1][2]*pc[1] + trans[2][2]*pc[2];
	    *ppcs[1] = trans[0][1]*pc[0] + trans[1][1]*pc[1] + trans[2][1]*pc[2];
	    *ppcs[2] = trans[0][0]*pc[0] + trans[1][0]*pc[1] + trans[2][0]*pc[2];

	    *ppcs[0] = trans[2][0]*pc[0] + trans[1][0]*pc[1] + trans[0][0]*pc[2];
	    *ppcs[1] = trans[2][1]*pc[0] + trans[1][1]*pc[1] + trans[0][1]*pc[2];
	    *ppcs[2] = trans[2][2]*pc[0] + trans[1][2]*pc[1] + trans[0][2]*pc[2];

	    *ppcs[0] = trans[2][2]*pc[0] + trans[1][2]*pc[1] + trans[0][2]*pc[2];
	    *ppcs[1] = trans[2][1]*pc[0] + trans[1][1]*pc[1] + trans[0][1]*pc[2];
	    *ppcs[2] = trans[2][0]*pc[0] + trans[1][0]*pc[1] + trans[0][0]*pc[2];
	    */
	    }
	}

	/* write new pixels */
	for (i=0;i<nf;i++) {
		if(fwrite(pcs[i],sizeof(float),np,stdout) != np) {
			fprintf(stderr,"%s: error writing frame %d\n",Progname,i);
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
