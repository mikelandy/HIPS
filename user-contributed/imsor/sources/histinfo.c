/*	Copyright (c) 1990 Jens Michael Carstensen, IMSOR 

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 *  histinfo.c - display histogram information 
 *
 *  Usage: histinfo [-z] [-t] [-p] [-c] <inseq >outseq
 *
 *  [-z] option ignores zero valued pixels
 *
 *  [-t] option prints histogram table 
 *
 *  [-p] option prints histogram table in percent 
 *
 *  [-c] option generates one histogram for entire sequence 
 *
 *  to load: cc -o histinfo histinfo.c -lhips -lm
 *
 *  HIPS-2 - 19/2-1993
 */

#define  Byte     unsigned char

#include <hipl_format.h>
#include <stdio.h>
#include <math.h>

int main(argc,argv)
int argc;
char **argv;
{
        struct header hd,mhd;
        int    i,j;
        int    min,max,num,hmax,median,modus,temp;
	int    f,nf,npix,nrows,ncols;
	int    *hist,zflag=0,tflag=0,pflag=0,cflag=0,gflag=0,aflag=0;
	int	shiftflag=0,ncolors=256;
	Byte   *bp,*ifr,*mpic;
        double m1,m2,m3,m4,var,sdev,cv,skew,kurt,gmean;
	double	cumhist[256],entropy,energy,prob;
        double 	sum,sum1,sum2,sum3,sum4,cum,logsum;
	char	*maskfile;
	int	mask=0;
	FILE	*mfp;

	Progname = strsave(*argv);
	for (i=1;i<argc;i++) {
           if (argv[i][0]=='-') {
		switch (argv[i][1]) {
			case 'a': pflag++; aflag++; break;
			case 'z': zflag++; break;
			case 't': tflag++; break;
			case 'm': maskfile=argv[++i]; mask=1; break;
			case 'p': pflag++; break;
			case 's': shiftflag++; break;
			case 'c': cflag++; break;
			case 'g': gflag++; break;
			default:
				perr(HE_MSG,"histinfo [-ztpc] < inseq > outseq");
		}
           }
	}
	read_header(&hd);
	if (hd.pixel_format != PFBYTE)
	   	perr(HE_MSG,"image must be in byte format");
	nrows = hd.rows;
	ncols = hd.cols;
	nf = hd.num_frame;
	npix = nrows * ncols;

	if (mask) {
		if ((mfp = fopen(maskfile,"r")) == NULL) 
			perr(HE_MSG,"can't open mask file");
		fread_header(mfp,&mhd,maskfile);
		mpic = (Byte *) halloc(npix,sizeof(Byte *));
	  	if (fread(mpic,1,npix,mfp) != npix ) 
				perr(HE_MSG,"error during read");
		fclose(mfp);
	}

	hist = (int *) halloc(256,sizeof(int));
	ifr = (Byte *) halloc(npix,sizeof(Byte));

	/* for each frame display histogram information */

        for (f=0;f<nf;f++) {
		if (shiftflag) ncolors>>=1;
		for (i=0;i<ncolors;i++) hist[i]=0;
		for (i=0;i<((cflag) ? nf : 1);i++) {
	  		if (fread(ifr,1,npix,stdin) != npix ) 
				perr(HE_MSG,"error during read");
			if (shiftflag)
				for (j=0;j<npix;j++) ifr[j]>>=1;
         		for (j=0;j<npix;j++) 
				if (mask==0 ||  mpic[j]==0)
					hist[ifr[j]]++;
       		}
		if (cflag) f=nf;
		for (i=0,npix=0;i<ncolors;i++) npix+=hist[i];

                if (zflag) npix-=hist[0];
		if (npix==0) perr(HE_MSG,"image contains only zeros");
		cum=0.0;
	        for (i=0;i<ncolors;i++) {
			    cum+=(float)hist[i]*100.0/(float)npix;
			    cumhist[i]=cum;
		}

	        min=zflag;
	        while (hist[min]==0) min++;
	        max=ncolors-1;
	        while (hist[max]==0) max--;
	        
	        hmax=0;
	        num=0;
	        sum=sum1=sum2=sum3=sum4=logsum=0.0;
	        median=ncolors;
		entropy=energy=0.0;
	        for (i=zflag;i<ncolors;i++) {
	          if (hist[i]>hmax) {hmax=hist[i]; modus=i;}
	          temp=i*hist[i];
		  logsum+=(double)hist[i]*log((double)i+1.0);
	          sum1+=(double)temp;
	          sum2+=(double)i*temp;
	          sum3+=(double)i*i*temp;
	          sum4+=(double)i*i*i*temp;
	          sum+=(double)hist[i];
	          if (sum>=(double)(npix)/2.0 && median==ncolors) median=i;
	          if (hist[i]>0) num++;
		  prob=(double) hist[i]/npix;
		  if (hist[i]!=0) entropy-=prob*log(prob);
		  energy+=prob*prob;
	        }
	        m1=sum1/npix;
	        m2=sum2/npix;
	        m3=sum3/npix;
	        m4=sum4/npix;
		gmean=exp(logsum/npix);
	        var=m2-m1*m1;
		sdev=sqrt(var);
		cv=sdev/m1;
	        skew=(m3-3*m1*m2+2*m1*m1*m1)/(var*sqrt(var));
	        kurt=(m4-4*m1*m3+6*m1*m1*m2-3*m1*m1*m1*m1)/(var*var)-3;
		if (gflag) {
	        	fprintf(stderr,"%f %f %f %f %f %d %f %f\n",
				m1,var,cv,skew,kurt,median,entropy,energy);
		}
		else {
	        fprintf(stderr,"\nHistogram information\n");
	        fprintf(stderr,"_______________________________________________________________\n");
	        fprintf(stderr,"Min    :  %3d    Mean     : %9.4f    Skewness : %9.4f\n",min,m1,skew);
	        fprintf(stderr,"Max    :  %3d    Variance : %9.4f    Kurtosis : %9.4f\n",max,var,kurt);
	        fprintf(stderr,"Rep#   :  %3d    Std.dev. : %9.4f    Gmean    : %9.4f\n",num,sdev,gmean);
	        fprintf(stderr,"Median :  %3d    CV       : %9.4f    Npix     :   %7d\n",median,cv,npix);
		fprintf(stderr,"Modus  :  %3d    Hmax     :   %5.2f%%     Entropy  : %9.4f\n",modus,hmax*100.0/npix,entropy);
	        fprintf(stderr,"_______________________________________________________________\n\n");

		/* Print the entire histogram in total number (tflag)
		   or in percent (pflag) */

		if (tflag || pflag) {
		        fprintf(stderr,"\n      ");
			for (j=0;j<8;j++) fprintf(stderr,"%8d ",32*j);
			fprintf(stderr,"\n______");
			for (j=0;j<8;j++) fprintf(stderr,"_________");
			fprintf(stderr,"\n");
			for (i=0;i<32;i++) {
			  fprintf(stderr,"%3d  |",i);
			  for (j=0;j<8;j++) {
			    if (pflag) {
			       if (aflag)
			       	fprintf(stderr,"%8.2f ",cumhist[32*j+i]);
			       else
			       	fprintf(stderr,"%8.2f ",
					(float)hist[32*j+i]*100.0/(float)npix);
			    }
			    else
			       fprintf(stderr,"%8d ",hist[32*j+i]);
	                  }
	 		  fprintf(stderr,"\n");
			}
			fprintf(stderr,"______");
			for (j=0;j<8;j++) fprintf(stderr,"_________");
			fprintf(stderr,"\n\n");
		}
		} /* end if (gflag) ... */
	}
        return(0);
}
