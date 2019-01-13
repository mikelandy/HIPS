/*	Copyright (c) 1991  Jens Michael Carstensen, IMSOR.

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/* glcm - generates cooccurrence matrices and features from these. 
 *
 * usage: glcm [-rcps] <iseq >oseq
 *
 * argument -r: sets row displacement
 * 	    -c: sets column displacement
 * 	    -s: cuts the number of graylevels
 *
 * to load: cc -o glcm glcm.c -lhips -lm
 *
 */

#include <math.h>
#include <hipl_format.h>
#include <stdio.h>

#define Byte     unsigned char

int	glcm[256][256],nrows,ncols,ncolors=256;
Byte    **pic;

int main(argc,argv)
int	argc;
char	*argv[];
{
	struct header hd;
	int	gldh[256],glsh[2*256-1],pr[256],pc[256];
	int	dr,dc,distpool=0;
	int     i,j,f,npix,norm,shiftcolor=0,pflag=0;
	double	rmean,cmean,rvar,cvar,s0,sc,sm;
	double	prob,prob1;
	double	hxy1,hxy2;
	double	energy,entropy,maxprob,correl,diagcorr,kappa;
	double	denergy,dentropy,inertia,homogeneity;
	double	senergy,sentropy,svariance,sshade,sprominence,sdif;
	void 	update_glcm(), symmetric_glcm();
	

	Progname = strsave(*argv);

	dr=0;
	dc=1;
	for (i=1;i<argc;i++) {
           if (argv[i][0]=='-') {
		switch (argv[i][1]) {
			case 'r': dr=atoi(argv[++i]); break;
			case 'c': dc=atoi(argv[++i]); break;
			case 'd': distpool=atoi(argv[++i]); break;
			case 'p': pflag++; break;
			case 's': shiftcolor=atoi(argv[++i]); 
				  if (shiftcolor>7 || shiftcolor<0)
					perr(HE_MSG,"shift must be from 0 to 7");
				  ncolors>>=shiftcolor;
				  break;
			default:
				perr(HE_MSG,"glcm [-rcps] < iseq > oseq");
		}
           }
	}
	read_header(&hd);
	nrows=hd.orows;
	ncols=hd.ocols;
	npix=nrows*ncols;
	if (hd.pixel_format != PFBYTE)
		perr(HE_MSG,"pixel format must be byte");
        pic    = (Byte **) halloc(nrows,sizeof(Byte *));
        for (i=0;i<nrows;i++)
        	pic[i]    = (Byte *) halloc(ncols,sizeof(Byte));

/*****  Main loop *****/
	for (f=0;f<hd.num_frame;f++) { 

        for (i=0;i<nrows;i++) 
	    if (fread(pic[i],1,ncols,stdin) != ncols) 
		perr(HE_MSG,"error during read");
	if (shiftcolor)
        	for (i=0;i<nrows;i++) for (j=0;j<ncols;j++) 
			pic[i][j]>>=shiftcolor;

/* Construct GLCM */
	norm=0;
        for (i=0;i<ncolors;i++) for (j=0;j<ncolors;j++) glcm[i][j]=0;
	switch (distpool) {
	case 1: 
		update_glcm(1,0);
		update_glcm(0,1);
		update_glcm(1,1);
		update_glcm(1,-1);
		symmetric_glcm();
		norm=8*npix-6*ncols-6*nrows;
		break;
	default:	
		update_glcm(dr,dc);
		norm=npix-ABS(dr)*ncols-ABS(dc)*nrows;
		break;
	}

/* Construct marginal probabilities */
        for (i=0;i<ncolors;i++) { pr[i]=0; pc[i]=0; }
        for (i=0;i<ncolors;i++) for (j=0;j<ncolors;j++) {
		pr[i]+=glcm[i][j];
		pc[j]+=glcm[i][j];
	}
	rmean=rvar=0.0;
        for (i=0;i<ncolors;i++) { 
		rmean+=i*(double)pr[i]/norm; 
		rvar+=i*i*(double)pr[i]/norm; 
	}
	rvar=rvar-rmean*rmean;
	cmean=cvar=0.0;
        for (i=0;i<ncolors;i++) { 
		cmean+=i*(double)pc[i]/norm; 
		cvar+=i*i*(double)pc[i]/norm; 
	}
	cvar=cvar-cmean*cmean;

/* Construct GLDH */
        for (i=0;i<ncolors;i++) gldh[i]=0;
        for (i=0;i<ncolors;i++) for (j=0;j<ncolors;j++) 
		gldh[ABS(i-j)]+=glcm[i][j];

/* Construct GLSH */
        for (i=0;i<2*ncolors-1;i++) glsh[i]=0;
        for (i=0;i<ncolors;i++) for (j=0;j<ncolors;j++)
		glsh[i+j]+=glcm[i][j];

/* GLCM measures */
	energy=entropy=hxy1=hxy2=maxprob=correl=diagcorr=0.0;
        for (i=0;i<ncolors;i++) for (j=0;j<ncolors;j++) {
		prob=(double)glcm[i][j]/norm;
		prob1=((double)pr[i]*(double)pc[j])/((double)norm*(double)norm);
		energy+=prob*prob;
		if (prob>0.0) entropy-=prob*log(prob);
		if (prob1>0.0) {
			hxy1-=prob*log(prob1);
			hxy2-=prob1*log(prob1);
		}
		if (prob>maxprob) maxprob=prob;
		correl+=i*j*prob;
		diagcorr+=ABS(i-j)*(i+j-rmean-cmean)*prob;
	}
	correl=(correl-rmean*cmean)/(sqrt(rvar)*sqrt(cvar));

	/* Compute kappa feature */
	s0=sc=sm=0.0;
        for (i=0;i<ncolors;i++) {
		s0+=(double)glcm[i][i]/norm;
		sc+=((double)pr[i]/norm)*((double)pc[i]/norm);
		sm+=(double)(MIN(pr[i],pc[i]))/norm;
	}
	kappa=(s0-sc)/(sm-sc);

/* GLDH measures */
	denergy=dentropy=inertia=homogeneity=0.0;
        for (i=0;i<ncolors;i++) {
		prob=(double)gldh[i]/norm;
		denergy+=prob*prob;
		if (prob>0.0) dentropy-=prob*log(prob);
		inertia+=(double)(i*i)*prob;
		homogeneity+=prob/(double)(1+i*i);
	}	

/* GLSH measures */
	senergy=sentropy=svariance=sshade=sprominence=0.0;
        for (i=0;i<2*ncolors-1;i++) {
		prob=(double)glsh[i]/norm;
		sdif=(double)(i-rmean-cmean);
		senergy+=prob*prob;
		if (prob>0.0) sentropy-=prob*log(prob);
		svariance+=sdif*sdif*prob;
		sshade+=sdif*sdif*sdif*prob;
		sprominence+=sdif*sdif*sdif*sdif*prob;
	}	

	fprintf(stderr,"%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n",
		energy,entropy,maxprob,correl,diagcorr,kappa,
		denergy,dentropy,inertia,homogeneity,
		senergy,sentropy,svariance,sshade,sprominence);
	/*
	fprintf(stderr,"%f  %f %f\n", entropy,hxy1,hxy2);
	*/
	if (pflag) {
		hd.orows=hd.rows=ncolors;
		hd.ocols=hd.cols=ncolors;
		hd.pixel_format=PFINT;
		update_header(&hd,argc,argv);
		write_header(&hd);
        	for (i=0;i<ncolors;i++) 
		    if (fwrite(&glcm[i][0],sizeof(int),ncolors,stdout)!=ncolors)
				perr(HE_MSG,"error during write");
	}

	}
/*****  End main loop *****/
        return(0);
}

void update_glcm(dr,dc)
int	dr,dc;
{
	int rbeg,rend,cbeg,cend,i,j;

	rbeg=(dr<0) ? -dr   : 0;
	rend=(dr<0) ? nrows : nrows-dr;
	cbeg=(dc<0) ? -dc   : 0;
	cend=(dc<0) ? ncols : ncols-dc;
        for (i=rbeg;i<rend;i++) for (j=cbeg;j<cend;j++) 
		glcm[pic[i][j]][pic[i+dr][j+dc]]++;
}

void symmetric_glcm()
{
	int i,j;

	for (i=0;i<ncolors;i++) for (j=i;j<ncolors;j++)
		glcm[i][j]+=glcm[j][i];
	for (i=0;i<ncolors;i++) for (j=i;j<ncolors;j++)
		glcm[i][j]=glcm[j][i];
}
