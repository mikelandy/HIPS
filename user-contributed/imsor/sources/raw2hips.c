/*	Copyright (c) 1993 Jens Michael Carstensen, IMSOR 

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 *  raw2hips.c - converts an image file from raw-format to hips-format 
 *		 by guessing the number of rows and colums
 *
 *  Usage: raw2hips [-acrv] raw-file >outseq
 *
 *  Options:
 *
 *  [-a weight] 	selects weights
 *
 *  [-c ncols]		fixes number of columns to ncols
 *
 *  [-r nrows]		fixes number of rows to nrows
 *
 *  [-v] 		selects verbose mode
 *
 *  to load: cc -o raw2hips raw2hips.c -lhips 
 *
 */

#define  Byte     unsigned char

#include <hipl_format.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(argc,argv)
int argc;
char **argv;
{
        struct header hd;
        int    	i,j,k;
 	Byte	*ifr;
	int	nrows,ncols,npix,nf;
	int 	filesize();
	char 	*infile;
	FILE 	*infp;
	int	verbose=0,rset=0,cset=0;
	int	ncand,cand[50][2],candsel,temp;
	double	sum,sum1,minsum,a;

	Progname = strsave(*argv);

	a=0.0;
	for (i=1;i<argc;i++) {
           if (argv[i][0]=='-') {
		switch (argv[i][1]) {
			case 'v': 
				verbose++;
				break;
			case 'a': 
				a=(double)atof(argv[++i]); 
				break;
			case 'r': 
				nrows=atoi(argv[++i]); 
				rset++;
				break;
			case 'c': 
				ncols=atoi(argv[++i]);	
				cset++;
				break;
			default:
				perr(HE_MSG,"raw2hips [-acrv] iseq >oseq");
		}
           }
	   else {
		infile=argv[i];
		if ((infp=fopen(infile,"r")) == NULL) 
			perr(HE_MSG,"can't open file");
		npix=filesize(infile);
		if (rset) {
			ncols=npix/nrows;
			npix=ncols*nrows;
		}
		if (cset) {
			nrows=npix/ncols;
			npix=ncols*nrows;
		}
		if (verbose)
			fprintf(stderr,"Size : %d\n",npix);
	   }
	}
	ifr = (Byte *) halloc(npix,sizeof(Byte));
  	if (fread(ifr,1,npix,infp) != npix)
		perr(HE_MSG,"error during read");

	/* Start guessing */
	if (rset==0 && cset==0) {
	if (verbose)
		fprintf(stderr,"a=%f\n",a);
	ncand=0;
	for (i=20;i<=npix/20;i++) 
		if (npix%i==0) {
			cand[ncand][0]=i;
			cand[ncand][1]=npix/i;
			ncand++;
		}

	if (ncand==0)
		perr(HE_MSG,"no suitable guess for number of rows");
	
	minsum=256.0*256.0;
	candsel=0;
	for (i=0;i<ncand;i++) {
		nrows=cand[i][0];
		ncols=cand[i][1];
		sum=0.0;
		for (j=0;j<nrows-1;j++) for (k=0;k<ncols;k++) {
			temp = (int)ifr[j*ncols+k]-(int)ifr[(j+1)*ncols+k];
			sum += (double)(temp*temp);
		}
		sum /= (double)((nrows-1)*ncols);
		sum1=0.0;
		for (j=0;j<nrows;j++) {
			temp = (int)ifr[j*ncols]-(int)ifr[(j+1)*ncols-1];
			sum1 += (double)(temp*temp);
		}
		sum1 /= (double)nrows;
		if (verbose)
			fprintf(stderr,"%4d : %4d x %4d , %f %f %f\n",
				i,nrows,ncols,sum,sum1,sum-a*sum1);
		if (sum-a*sum1<minsum) {
			minsum=sum-a*sum1;
			candsel=i;
		}
	}
	nrows=cand[candsel][0];
	ncols=cand[candsel][1];
	} /* End guess */

	init_header(&hd,"","",1,"",nrows,ncols,PFBYTE,1,"");
	update_header(&hd,argc,argv);
	write_header(&hd);

  	if (fwrite(ifr,1,npix,stdout) != npix)
		perr(HE_MSG,"error during write");

        return(0);
}

#include <sys/stat.h>

int filesize(fname)
char *fname;
{
	struct stat buf;

	stat(fname,&buf);
	return ((int) buf.st_size);
}
