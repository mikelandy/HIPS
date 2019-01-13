 /*
 * Copyright (c) 1990
 *
 *      Michael Landy; Lin, Shou-Tsung 04.12.85 (histoeq.c)
 *
 *	Allan Aasbjerg Nielsen
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
 *  histobe.c - Histogram match to Beta distribution 
 *
 *  Usage: histobe [-z] [-a alpha] [-b beta] [-g numgrey] [-I]
 *                 [-e [nl ns [sl ss]] | -M mask_file value]
 *
 *  [-z] 	ignore zero valued pixels
 *  [-a]	wanted alpha (defaults to 4.0)
 *  [-b]	wanted beta  (defaults to 4.0)
 *  [-g]	wanted number of grey levels  (defaults to 256)
 *  [-I]	stretch intensity rather than each frame (3-frame sequence only)
 *  [-e]	calculate histogram under defined rectangle only
 *  [-M]	calculate histogram where value of mask image is maskvalue only
 *              (maskvalue defaults to 0)
 *
 *  to load: cc histobe.c -o histobe -lhips -lm
 *
 *  -g option added 22.05.91/AA
 *  -I option added 17.06.91/AA
 *  -M option added 11.11.91/AA
 *  -e option added 11.11.91/AA
 *  HIPS-2          07.01.93/AA
 *  beta() removed  03.03.93/AA
 */

#define  Byte     unsigned char
#define  numbin   256 /* #grey levels in */

#include <hipl_format.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>

char usage[]="Usage: histobe [-z] [-a alpha] [-b beta] [-g numgrey] [-I] [-e [nl ns [sl ss]] | -M mask_file [value]]";

int main(argc,argv)
int argc;
char *argv[];
{
	struct header hd,mask;
	FILE   *fp;
	int    z,zz,zmin,zlast,*new;
	int    r,c,nf,npix,i,j,*hist,fr;
	int    nrhis=0,nchis=0,srhis,schis,starthisflag=0;
        int    p,zflag=0,zcount=0,Iflag=0;
        int    gflag=0,numgrey=256; /* #grey levels out */
        int    eflag=0,Mflag=0;
	int    npmask;
	char   *maskfile;
	Byte   *maskifr,*pmaskifr,maskvalue=(Byte)0;
	Byte   *ifr,*pifr;
	Byte   **bfr,**pbfr;
	double *cumref,*cuminp,sumref,suminp,diff,difmin,temp;
	float  alpha=4.0,bbeta=4.0;
	float  I,*v1,*pv1,*v2,*pv2;
	float  R,G,B;

	Progname = strsave(*argv);

	for (i=1;i<argc;i++) {
		if (argv[i][0] == '-')
		switch (argv[i][1]) {
		case 'z': case 'Z': zflag++; break;
		case 'a':
			alpha = atof(argv[++i]);
			if (alpha < 0.0 || alpha > 20.0 )
				perr(HE_MSG,"-a  0 <= alpha <= 20");
			break;
		case 'b':
			bbeta = atof(argv[++i]);
			if (bbeta < 0.0 || bbeta > 20.0 )
				perr(HE_MSG,"-b  0 <= beta <= 20");
			break;
		case 'g':
			gflag=1;
			numgrey=atoi(argv[++i]);
			if (numgrey>256)
				perr(HE_MSG,"-g  number of greylevels <= 256");
			break;
		case 'I': Iflag++; break;
		case 'e':
			  eflag++;
			  /*fprintf(stderr,"i+2 = %d, argc = %d\n",i+2,argc);*/
			  if ((i+2)<argc && argv[i+1][0]!='-' && argv[i+2][0]!='-') {
				if ((nrhis=atoi(argv[++i]))<=0)
				   perr(HE_MSG,"number of rows in -e must be >0");
				if ((nchis=atoi(argv[++i]))<=0)
				   perr(HE_MSG,"number of cols in -e must be >0");
			  }
			  /*fprintf(stderr,"i+2 = %d, argc = %d\n",i+2,argc);*/
			  if ((i+2)<argc && argv[i+1][0]!='-' && argv[i+2][0]!='-') {
				srhis=atoi(argv[++i]);
				schis=atoi(argv[++i]);
				starthisflag++;
			  }
			  break;
		case 'M':
			  Mflag++;
			  maskfile=argv[++i];
			  if ((fp=fopen(maskfile,"r")) == NULL) {
			     fprintf(stderr,"%s: unable to open maskfile %s\n",Progname,maskfile);
			     exit(1);
			  }
			  if ((i+1)<argc && argv[i+1][0]!='-')
			     maskvalue=(Byte)atoi(argv[++i]);
			  fprintf(stderr,"%s: maskvalue = %d\n",Progname,maskvalue);
			  break;
		case 'D': break;
		default: perr(HE_MSG,usage); break;
		}
        }

	read_header(&hd);
	if (hd.pixel_format != PFBYTE)
	   	perr(HE_MSG,"image must be in byte format");
	r = hd.orows;
	c = hd.ocols;
	npix = r * c;
	nf = hd.num_frame;
	update_header(&hd,argc,argv);
	write_header(&hd);

	hist = (int *) halloc(numbin,sizeof(int));
	new = (int *) halloc(numbin,sizeof(int));
        cumref = (double *) halloc(numbin,sizeof(double));
        cuminp = (double *) halloc(numbin,sizeof(double));
	ifr = (Byte *) halloc(npix,sizeof(Byte));

	/* read mask */
	if (Mflag==1){
	fread_header(fp,&mask,maskfile);
	if ( (mask.orows != r) || (mask.ocols != c) )
		perr(HE_MSG,"mismatch between mask size and image size");
	if (mask.pixel_format != PFBYTE)
		perr(HE_MSG,"mask format must be byte");
	if (mask.num_frame != 1)
		fprintf(stderr,"%s: more then one frame in mask image %s\n",Progname,maskfile);
	maskifr = (Byte *) halloc(npix,sizeof(Byte));
	/*
	if (fread(maskifr,sizeof(Byte),npix,maskfile) != npix)
	*/
	if (fread(maskifr,sizeof(Byte),npix,fp) != npix)
		perr(HE_MSG,"error reading mask");
	fclose(fp);
	}

	if (eflag == 1) {
		if (Mflag == 1) perr(HE_MSG,"-M and -e not allowed simultaneously");
		if (nrhis==0 || nchis==0) {
		   nrhis=r/2;
		   nchis=c/2;
		}
		if (starthisflag == 0) {
		   srhis=(r-nrhis)/2;
		   schis=(c-nchis)/2;
		}
		if ( (srhis+nrhis)>r || (schis+nchis)>c )
		   perr(HE_MSG,"wrong specifications for -e");
		fprintf(stderr,"%s: %d %d %d %d\n",Progname,nrhis,nchis,srhis,schis);
		Mflag=1;
		maskifr = (Byte *) halloc(npix,sizeof(Byte));
		pmaskifr=maskifr;
		for (i=0;i<r;i++) {
		    for (j=0;j<c;j++) {
			*pmaskifr=(Byte)255;
			if ( i>=srhis && i<(srhis+nrhis) &&
			     j>=schis && j<(schis+nchis) )
				*pmaskifr=(Byte)0;
			pmaskifr++;
		    }
		}
	}
	pmaskifr=maskifr;

	/*??? -M doesn't work with -I */
	if (Iflag == 1 ) {
		if (nf != 3) perr(HE_MSG,"-I implemented for 3-frame sequence only");

		bfr = (Byte **) halloc(nf,sizeof(Byte *));
		pbfr= (Byte **) halloc(nf,sizeof(Byte *));
		for (i=0;i<nf;i++)
			bfr[i] = (Byte *) halloc(npix,sizeof(Byte));
		v1  = (float *) halloc(npix,sizeof(float));
		v2  = (float *) halloc(npix,sizeof(float));

		npmask=0;
		/* read frame */
		for (i=0;i<nf;i++)
	    	if (fread(bfr[i],sizeof(Byte),npix,stdin) != npix) {
	        	fprintf(stderr,"%s: error reading frame %d\n",Progname,i);
			exit(1);
		}
		
		/* RGB to LUV (Taylor) transformation */
   		for (i=0;i<nf;i++) pbfr[i] = bfr[i];
   		for (p=0,pifr=ifr,pmaskifr=maskifr,pv1=v1,pv2=v2;p<npix;p++,pv1++,pv2++,pifr++,pmaskifr++,pbfr[0]++,pbfr[1]++,pbfr[2]++)
		if (Mflag == 0 || *pmaskifr == maskvalue) {
		I     =0.3333333*( (float)(*pbfr[0])+(float)(*pbfr[1])+    (float)(*pbfr[2]));
		*pv1  =0.4082483*(-(float)(*pbfr[0])-(float)(*pbfr[1])+2.0*(float)(*pbfr[2]));
		*pv2  =0.7071068*( (float)(*pbfr[0])-(float)(*pbfr[1])                      );
		*pifr = (Byte) I;
		}

		/* calculate the histogram */
   		for (i=0;i<numbin;i++) hist[i]=0;
   		for (p=0,pifr=ifr,pmaskifr=maskifr;p<npix;p++,pifr++,pmaskifr++)
		    if (Mflag == 0 || *pmaskifr == maskvalue) {
		       npmask++;
		       ++hist[*pifr];
		    }
		/*AA 18.3.93*/
		if (zflag==1) {zcount=hist[0]; hist[0]=0; npmask=npix-zcount;}
		fprintf(stderr,"%s: %d pixels included in stretch\n",Progname,npmask);

		/* calculate the cumulative histogram 
		   for input and reference            */
                for (z=0,sumref=0.0,suminp=0.0;z<numbin;z++) {
		  temp=((double)z+0.5)/(double)numbin;
		  sumref+=pow(temp,alpha-1.0)*pow(1.0-temp,bbeta-1.0);
                  cumref[z]=sumref;
		  if (Mflag==0)
		     suminp+=(double)hist[z]/(double)(npix-zcount);
		  else
		     suminp+=(double)hist[z]/(double)(npmask-zcount);
		  cuminp[z]=suminp;
                }
                for (z=0;z<numbin;z++) cumref[z]/=cumref[numbin-1];

                /* make new LUT, zflag is 0 or 1 */
		for (z=zflag,new[0]=0,zlast=zflag;z<numbin;z++) {
		  for (zz=zlast,difmin=10.0;zz<numbin;zz++) {
		    diff=fabs(cuminp[z]-cumref[zz]);
		    if (diff<=difmin) {zmin=zz; difmin=diff;}
                  }
		  new[z]=zlast=(gflag<1) ? zmin : (int)floor((double)zmin*(double)numgrey/(double)numbin+0.5);
		  /* fprintf(stderr,"\nLUT[%i] = %i",z,new[z]); */
		}

		/* update pixels */
		for (p=0,pifr=ifr;p<npix;p++,pifr++) *pifr = new[*pifr];
		
		/* LUV (Taylor) to RGB transformation */
   		for (i=0;i<nf;i++) pbfr[i] = bfr[i];
   		for (p=0,pifr=ifr,pmaskifr=maskifr,pv1=v1,pv2=v2;p<npix;p++,pifr++,pmaskifr++,v1++,pv2++,pbfr[0]++,pbfr[1]++,pbfr[2]++)
		if (Mflag == 0 || *pmaskifr == maskvalue) {
		R = (float)(*pifr)-0.4082483*(*pv1)+0.7071068*(*pv2);
		G = (float)(*pifr)-0.4082483*(*pv1)-0.7071068*(*pv2);
		B = (float)(*pifr)+0.8164966*(*pv1)                 ;
		if (R <   0.0) R =   0.0;
		if (G <   0.0) G =   0.0;
		if (B <   0.0) B =   0.0;
		if (R > 255.0) R = 255.0;
		if (G > 255.0) G = 255.0;
		if (B > 255.0) B = 255.0;
		*pbfr[0] = (Byte) R;
		*pbfr[1] = (Byte) G;
		*pbfr[2] = (Byte) B;
/*
c           xr=0.5773503*xint-0.4082483*v1+0.7071068*v2
c           xg=0.5773503*xint-0.4082483*v1-0.7071068*v2
c           xb=0.5773503*xint+0.8164966*v1
c           xr=0.3333333*xint-0.4082483*v1+0.7071068*v2
c           xg=0.3333333*xint-0.4082483*v1-0.7071068*v2
c           xb=0.3333333*xint+0.8164966*v1
*/
		}

		/* write new pixels */
   		for (i=0;i<nf;i++) {
		if(fwrite(bfr[i],sizeof(Byte),npix,stdout) != npix) {
			fprintf(stderr,"%s: error writing frame %d\n",Progname,i);
			exit(1);
		}
		}
	exit(0);
	}

	/* for each frame do histogram match to beta distribution */

	for (fr=0;fr<nf;fr++) {
		npmask=0;
		/* read frame */
	    	if (fread(ifr,sizeof(Byte),npix,stdin) != npix) {
	        	fprintf(stderr,"%s: error reading frame %d\n",Progname,fr);
			exit(1);
		}

		/* calculate the histogram */
   		for (i=0;i<numbin;i++) hist[i]=0;
   		for (p=0,pifr=ifr,pmaskifr=maskifr;p<npix;p++,pifr++,pmaskifr++)
		    if (Mflag == 0 || *pmaskifr == maskvalue) {
		       npmask++;
		       ++hist[*pifr];
		    }
		/*AA 18.3.93*/
		if (zflag==1) {zcount=hist[0]; hist[0]=0; npmask=npix-zcount;}
		if (fr==0)
		   fprintf(stderr,"%s: %d pixels included in stretch\n",Progname,npmask);
   
		/* calculate the cumulative histogram 
		   for input and reference            */
                for (z=0,sumref=0.0,suminp=0.0;z<numbin;z++) {
		  temp=((double)z+0.5)/(double)numbin;
		  sumref+=pow(temp,alpha-1.0)*pow(1.0-temp,bbeta-1.0);
                  cumref[z]=sumref;
		  if (Mflag==0)
		     suminp+=(double)hist[z]/(double)(npix-zcount);
		  else
		     suminp+=(double)hist[z]/(double)(npmask-zcount);
		  cuminp[z]=suminp;
                }
                for (z=0;z<numbin;z++) cumref[z]/=cumref[numbin-1];

                /* make new LUT, zflag is 0 or 1 */
		for (z=zflag,new[0]=0,zlast=zflag;z<numbin;z++) {
		  for (zz=zlast,difmin=10.0;zz<numbin;zz++) {
		    diff=fabs(cuminp[z]-cumref[zz]);
		    if (diff<=difmin) {zmin=zz; difmin=diff;}
                  }
		  new[z]=zlast=(gflag<1) ? zmin : (int)floor((double)zmin*(double)numgrey/(double)numbin+0.5);
		  /*fprintf(stderr,"\nLUT[%i] = %i",z,new[z]);*/
		}

		/* update pixels */
		for (p=0,pifr=ifr;p<npix;p++,pifr++) *pifr = new[*pifr];

		/* write new pixels */
		if(fwrite(ifr,sizeof(Byte),npix,stdout) != npix) {
			fprintf(stderr,"%s: error writing frame %d\n",Progname,fr);
			exit(1);
		}
      	}
	exit(0);
}
