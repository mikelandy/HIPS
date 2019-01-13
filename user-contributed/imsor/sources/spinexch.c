/*	Copyright (c) 1992 Jens Michael Carstensen, IMSOR 

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 *  spinexch.c - Metropolis spin-exchange Markov random field sampler 
 *
 *  usage: spinexch >oseq
 *
 *  to load: cc -o spinexch spinexch.c -lhips -lm
 */

#define  Byte     unsigned char
#define incr(w) ((w<nrows-1) ? w+1 : 0)
#define decr(w) ((w>0     ) ? w-1 : nrows-1)
#define incs(w) ((w<ncols-1) ? w+1 : 0)
#define decs(w) ((w>0     ) ? w-1 : ncols-1)
#define pic(ii,jj) ibfr[ncols*ii+jj]

#include <hipl_format.h>
#include <stdio.h>
#include <math.h>

int main(argc,argv)
int argc;
char **argv;
{
        struct header hd;
	int    	i,j,k,kk,nrows,ncols,npix,nf,f,nit,niter;
	Byte    *ibfr;
	int	count,temp0,temp1,swap,r0,s0,r1,s1;
	float	sum0,sum1,beta,frac1;
	double 	drand48();

	Progname = strsave(*argv);

	niter = 100;
	nrows = 128;
	ncols = 128;
	nf = 1;
	beta=3.0;
	frac1=0.5;

	for (i=1;i<argc;i++) {
           if (argv[i][0]=='-') {
		switch (argv[i][1]) {
			case '0': frac1=1.0-atof(argv[++i]); break;
			case '1': frac1=atof(argv[++i]); break;
			case 'b': beta=atof(argv[++i]); break;
			case 'f': nf=atoi(argv[++i]); break;
			case 'n': niter=atoi(argv[++i]); break;
			case 's': nrows=ncols=atoi(argv[++i]); break;
			default:
				perr(HE_MSG,"spinexch [-bfns] > outseq");
		}
           }
	}
	frac1=BETWEEN(frac1,0.0,1.0);
	init_header(&hd,"","",nf,"",nrows,ncols,PFBYTE,1,"");
	update_header(&hd,argc,argv);
	write_header(&hd);
	npix = nrows*ncols;

	ibfr = (Byte *) halloc(npix,sizeof(Byte));
	hd.image=ibfr;

	/* for each frame */

	for (f = 0; f<nf; f++) {
	fprintf(stderr,"Frame : %d\n",f);

	for (kk=0;kk<npix;kk++) ibfr[kk]=(drand48()<(1.0-frac1)) ? 0 : 1;

	/* Perform niter iterations */

	for (nit=0;nit<niter;nit++) {

		count=0;
		for (kk=0;kk<npix;kk++) {

		/* Get random 1-pixel and 0-pixel*/
		do {
			temp1=(int)(drand48()*(double)npix); 
			temp0=(int)(drand48()*(double)npix); 
			
		} while (ibfr[temp1]==ibfr[temp0]); 

		if (ibfr[temp0]==1) {swap=temp1;temp1=temp0;temp0=swap;}

		r1=temp1/ncols; s1=temp1%ncols;
		sum1=pic(r1,decs(s1))+pic(r1,incs(s1))
		    +pic(decr(r1),s1)+pic(incr(r1),s1);
		ibfr[temp1]=0;

		r0=temp0/ncols; s0=temp0%ncols;
		sum0=pic(r0,decs(s0))+pic(r0,incs(s0))
		    +pic(decr(r0),s0)+pic(incr(r0),s0);

		if (exp(beta*(double)(sum0-sum1))>=drand48()) 
			{ ibfr[temp0]=1; count++; }
		else ibfr[temp1]=1;
		
		} /* Next exchange attempt */
		/*
		fprintf(stderr,"%d %f\n",nit,(float)count*100.0/npix);
		*/

	} /* Next iteration */

	/*  Write frame */
	write_image(&hd,f);

      	} /* Next frame */

	exit(0);
}
