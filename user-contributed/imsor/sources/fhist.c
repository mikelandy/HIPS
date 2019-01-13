/*	Copyright (c) 1991 Jens Michael Carstensen
Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability. */

/*
 * fhist - stretch float images for display
 *	   Default is a match to gaussian
 *
 * usage:	fhist [-enrsabp] < iseq > oseq
 *
 * Options:	-e	for histogram equalisation 
 *		-n ng	number of gray levels
 *		-r	result is an int image with ranked pixel values
 *		-s sd	standard deviation for gaussian match
 *		-a ap	alpha parameter for beta match (default=2.0)
 *		-b bp	beta parameter for beta match (default=2.0)
 *		-p	print first-order statistics
 *
 * For the beta match the following might help in choosing the parameters
 *		ap=bp=1.0	equalization
 *		ap>1.0 & bp>1.0 convex histogram
 *		ap<1.0 & bp<1.0 concave histogram
 *		ap=bp=4.0	approximate gaussian
 *
 * This program works on float images only.
 *
 * to load:	cc -o fhist fhist.c -lhips -lm
 *
 * HIPS-2 - 19/2-1993
 */

#include <stdio.h>
#include <math.h>
#include <hipl_format.h>

#define	Byte		unsigned char

int main(argc,argv)

int argc;
char *argv[];

{
	float 	*ifr,*fp,sumref,zmean,zsdev,cumref[256],alpha=2.0,beta=2.0;
	double	temp;
	int	*dfr,*rfr,cumtable[256],count;
	Byte	*bfr;
	struct header hd;
	int	f,fr,npix,nrows,ncols,ncolors,i,j;
	int	equalize=FALSE,gaussmatch=TRUE,betamatch=FALSE,rflag=FALSE;
	int	pflag=FALSE;
	double	mn,adev,stdev,var,skew,curt,corr,scorr,dcorr;
	void	indexsort(),firstorder(),secondorder();


	Progname = strsave(*argv);

	ncolors=256;
	zsdev=40.0;
	for (i=1;i<argc;i++) {
           if (argv[i][0]=='-') {
		switch (argv[i][1]) {
			case 'e': equalize=TRUE; break;
			case 'n': ncolors=atoi(argv[++i]); break;
			case 'r': rflag=TRUE; break;
			case 's': zsdev=atof(argv[++i]); break;
			case 'a': alpha=atof(argv[++i]); 
				  betamatch=TRUE; break;
			case 'b': beta=atof(argv[++i]); 
				  betamatch=TRUE; break;
			case 'p': pflag=TRUE; break;
			default:
				perr(HE_MSG,"Usage: fhist [-enrsabp] < iseq > outseq");
		}
           }
	}
	if (equalize || betamatch) gaussmatch=FALSE;
	if (betamatch) equalize=FALSE;
	if (pflag) {equalize=gaussmatch=betamatch=FALSE;}
	read_header(&hd);
	if (hd.pixel_format == PFFLOAT) {
		nrows = hd.rows;
		ncols = hd.cols;
		npix  = hd.rows * hd.cols;
		hd.pixel_format = PFBYTE;
	}
	else
		perr(HE_MSG,"pixel format must be float");
	if (!pflag) {
		update_header(&hd,argc,argv);
		write_header(&hd);
	}
	fr = hd.num_frame;

	/* calculate the cumulative reference histogram */
	if (equalize) 
 		for (i=0;i<ncolors;i++) 
			cumtable[i]=((i+1)*npix)/ncolors;
	else if (gaussmatch) {
		zmean=(float)(ncolors-1)/2.0;
		sumref=0.0;
		for (i=0;i<ncolors;i++) {
		     temp=((double)i-zmean)/zsdev;
		     sumref+=exp(-0.5*(temp*temp));
		     cumref[i]=sumref;
     		}
 		for (i=0;i<ncolors;i++) 
			cumtable[i]=(int)((cumref[i]*npix)/sumref);
		cumtable[ncolors-1]=npix;
	}
	else if (betamatch) {
		sumref=0.0;
		for (i=0;i<ncolors;i++) {
		     temp=((double)i+0.5)/ncolors;
		     sumref+=pow(temp,alpha-1.0)*pow(1.0-temp,beta-1.0);
		     cumref[i]=sumref;
     		}
 		for (i=0;i<ncolors;i++) 
			cumtable[i]=(int)((cumref[i]*npix)/sumref);
		cumtable[ncolors-1]=npix;

	}

	ifr = (float *) halloc(npix,sizeof(float));
	if (!pflag) dfr = (int *) halloc(npix,sizeof(int));
	bfr = (Byte *) ifr;
	rfr = (int *) ifr;

	for (f=0;f<fr;f++) {
		if (fread(ifr,sizeof(float),npix,stdin) != npix)
			perr(HE_MSG,"error during read");
		if (!pflag) {
		  for (i=0;i<npix;i++) dfr[i]=i;
		  indexsort(ifr-1,dfr-1,npix);
		  for (i=1,count=1;i<npix;i++) if (ifr[i] != ifr[i-1]) count++;
		  fprintf(stderr,"Frame %d:\n",f);
		  fprintf(stderr,"%d values represented.\n",count);
		  fprintf(stderr,"min           : %f\n",ifr[0]);
		  fprintf(stderr,"5%% quantile   : %f\n",ifr[npix/20]);
		  fprintf(stderr,"median        : %f\n",ifr[npix/2]);
		  fprintf(stderr,"95%% quantile  : %f\n",ifr[19*npix/20]);
		  fprintf(stderr,"max           : %f\n",ifr[npix-1]);
		}
		if (pflag) {
		  firstorder(ifr,npix,&mn,&adev,&stdev,&var,&skew,&curt);
		  secondorder(ifr,nrows,ncols,mn,stdev,&corr,&scorr,&dcorr);
		  fprintf(stderr,"%f %f %f %f %f %f %f %f %f\n",mn,adev,stdev,var,skew,curt,corr,scorr,dcorr);
		}
		else if (rflag) {
			rfr[0]=0;
			for (i=1,count=0;i<npix;i++) {
				if (ifr[i] != ifr[i-1]) count++;
				rfr[i]=count;
			}
			if (fwrite(rfr,sizeof(int),npix,stdout) != npix)
				perr(HE_MSG,"error during write");
		}
		else {
			for (i=0,j=0;i<ncolors;i++)
				for (;j<cumtable[i];j++)
					bfr[dfr[j]]=(Byte)i;
			if (fwrite(bfr,sizeof(Byte),npix,stdout) != npix)
				perr(HE_MSG,"error during write");
		}
	}
	return(0);
}

void firstorder(image,npix,mn,adev,stdev,var,skew,curt)
float 	image[];
int 	npix;
double	*mn,*adev,*stdev,*var,*skew,*curt;

{
	double	sum,ssq;
	int	i;

	sum=0.0;
	for (i=0;i<npix;i++) sum+=image[i];
	*mn = sum/npix;
	*adev = *var = *skew = *curt = 0.0;
	for (i=0;i<npix;i++) {
		sum=image[i]-(*mn);
		ssq=sum*sum;
		*adev += fabs(sum);
		*var  += ssq;
		*skew += ssq*sum;
		*curt += ssq*ssq;
	}
	*adev /= npix;
	*var  /= npix; 
	if (*var) {
	*stdev = sqrt(*var);
	*skew /= (*var)*(*stdev)*npix;
	*curt /= (*var)*(*var)*npix;
	*curt -= 3.0;
	}
	else {
	*stdev = *skew = *curt = 0.0;
	}
}

void secondorder(image,nr,nc,mn,stdev,corr,scorr,dcorr)
float 	image[];
int 	nr,nc;
double	mn,stdev,*corr,*scorr,*dcorr;

{
	double	sum,ssum,dsum,var,im1,im2;
	double	corr1,scorr1,dcorr1,corr2,scorr2,dcorr2;
	int	i,j,k;
	
	var=stdev*stdev;

	sum=ssum=dsum=0.0;
	for (i=0;i<nr;i++) for (j=1;j<nc;j++) {
		k=i*nc+j;
		im1=image[k-1]-mn;
		im2=image[k]-mn;
		sum+=im1*im2;
		ssum+=im1*im2*(im1+im2);
		dsum+=im1*im2*(im1-im2);
	}
	sum/=nr*(nc-1);
	ssum/=nr*(nc-1);
	dsum/=nr*(nc-1);
	corr1=sum/var;
	scorr1=ssum/(var*stdev*sqrt(2.0+2.0*corr1));
	dcorr1=dsum/(var*stdev*sqrt(2.0-2.0*corr1));

	sum=ssum=dsum=0.0;
	for (i=1;i<nr;i++) for (j=0;j<nc;j++) {
		k=i*nc+j;
		im1=image[k-nc]-mn;
		im2=image[k]-mn;
		sum+=im1*im2;
		ssum+=im1*im2*(im1+im2);
		dsum+=im1*im2*(im1-im2);
	}
	sum/=(nr-1)*nc;
	ssum/=(nr-1)*nc;
	dsum/=(nr-1)*nc;
	corr2=sum/var;
	scorr2=ssum/(var*stdev*sqrt(2.0+2.0*corr2));
	dcorr2=dsum/(var*stdev*sqrt(2.0-2.0*corr2));

	*corr = (corr1+corr2)/2.0;
	*scorr = (scorr1+scorr2)/2.0;
	*dcorr = (dcorr1+dcorr2)/2.0;
}

void indexsort(vals,index,n)
float 	vals[];
int	index[];
int n;
{
	int 	i,j,k,kk;
	int 	idx;
	float 	val;

	k=n;
	kk=n/2+1;
	while (1) {
		if (kk>1) {
			val=vals[--kk];
			idx=index[kk];
		} 
		else {
			val=vals[k];
			idx=index[k];
			vals[k]=vals[1];
			index[k--]=index[1];
			if (k==1) {
				vals[1]=val;
				index[1]=idx;
				return;
			}
		}
		i=kk;
		j=kk*2;
		while (j<=k)	{
			if (j<k && vals[j]<vals[j+1]) ++j;
			if (val<vals[j]) {
				vals[i]=vals[j];
				index[i]=index[j];
				j += (i=j);
			}
			else j=k+1;
		}
		vals[i]=val;
		index[i]=idx;
	}
}
