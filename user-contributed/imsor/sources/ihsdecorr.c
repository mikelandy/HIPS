/*
 * Copyright (c) 1991
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
 *  ihsdecorr.c - RGB to IHS (or LUV), replace I, stretch I/S, IHS (or LUV) to RGB
 *
 *  Usage: ihsdecorr [-i [Iamp [Ioff]]] [-S [Samp [Soff]]]
 *		     [-s [Samp [Soff]]] [-I i.hips] [-c]
 *
 *	-S overwrites -s (-s is the default action)
 *
 *	-I	replace intensity with this image (same size and format)
 *
 *	-i	stretch intensity with Iamp (defaults to 2.0) and Ioff
 *		(defaults to 0.0)
 *
 *	-S	intensity dependent saturation stretch wanted,
 *	  	stretch saturation with Samp (defaults to 2.5) and Soff
 *		(defaults to 0.0)
 *
 *	-s	(default) stretch saturation with Samp (defaults to 2.0)
 *		and Soff (defaults to 0.0)
 *	-c	sets numcolor to 3
 *
 *  to load: cc ihsdecorr.c -o ihsdecorr -lhips -lm
 *
 *  HIPS-2 10.12.92/AA (added -c)
 */

#define	Byte	unsigned char

#include <hipl_format.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <math.h>

char usage[]="Usage: ihsdecorr [-i [Iamp [Ioff]]] [-S [Samp [Soff]]] [-s [Samp [Soff]]] [-I i.hips] [-c]";

int main(argc,argv)
int argc;
char *argv[];
{
	struct	header hd,hdI;
	int	nr,nc,nf,np,i,p;
	int	Iflag=0,iflag=0,Sflag=0,sflag=0;
	/*int	Ifile;*/
	FILE	*Ifile;
	char	*inewfname;
	Byte	**rgb,**prgb,*Inew;
	float	Ioff=0.0,Iamp=2.0,Soff=0.0,Samp=2.0,Snew;
	float	**out,**pout;
	float	II,U,V,S;
	int colflag=0;

	Progname = strsave(*argv);

	for (i=1;i<argc;i++) {
		if (argv[i][0] == '-')
		switch (argv[i][1]) {
		case 'I': Iflag=1;
			  /*Ifile=open(argv[++i],O_RDONLY,0);*/
			  inewfname=argv[++i];
			  if ((Ifile=fopen(inewfname,"r")) == NULL) {
			     fprintf(stderr,"ihsdecorr: can't open '%s'\n",inewfname);
			     exit(1);
			  }
		  	  break;
		case 'i': iflag=1; break;
		case 'S': Sflag=1;
			  Samp=2.5;
			  if ( (i+1)<argc && argv[i+1][0] != '-') {
				Samp=atof(argv[++i]);
			  	if ( (i+1)<argc && argv[i+1][0] != '-') Soff=atof(argv[++i]);
			  }
			  break;
		case 's': sflag=1;
			  if ( (i+1)<argc && argv[i+1][0] != '-') {
				Samp=atof(argv[++i]);
			  	if ( (i+1)<argc && argv[i+1][0] != '-') Soff=atof(argv[++i]);
			  }
			  break;
		case 'c': colflag=1;
			  break;
		case 'D': break;
		case 'U':
		default: fprintf(stderr,"ihsdecorr: %s\n",usage); exit(1); break;
		}
        }
	if (iflag==0 && Iflag==0 && sflag==0 && Sflag==0) sflag=1;
	   /*fprintf(stderr,"ihsdecorr: no action wanted?!\n"); exit(1);*/
	if (sflag==1 && Sflag==1) sflag=0;

	read_header(&hd);
	if (hd.pixel_format != PFBYTE) {
	   fprintf(stderr,"ihsdecorr: image must be in byte format\n");
	   exit(1);
	}
	nf = hd.num_frame;
	if (nf != 3) {
	   fprintf(stderr,"ihsdecorr: implemented for 3-frame sequence only\n");
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
	out = (float **) halloc(nf,sizeof(float *));
	pout= (float **) halloc(nf,sizeof(float *));
	for (i=0;i<nf;i++) {
		rgb[i] = (Byte *) halloc(np,sizeof(Byte));
		out[i] = (float *) halloc(np,sizeof(float));
	}

	/* read frame */
	for (i=0;i<nf;i++)
		if (fread(rgb[i],sizeof(Byte),np,stdin) != np) {
	           fprintf(stderr,"ihsdecorr: error reading frame %d\n",i);
		   exit(1);
		}
		
   	for (i=0;i<nf;i++) {
   		pout[i] = out[i];
   		prgb[i] = rgb[i];
	}

	/* read new intensity HIPS file if wanted */

	if (Iflag==1) {
		fread_header(Ifile,&hdI,inewfname);
		if (nr != hdI.orows || nc != hdI.ocols) {
		   fprintf(stderr,"ihsdecorr: new intensity image must have same size as RGB image\n");
		   exit(1);
		}
		if (hdI.pixel_format != PFBYTE) {
		   fprintf(stderr,"ihsdecorr: new intensity image must be byte format\n");
		   exit(1);
		}
		Inew = (Byte *) halloc(np,sizeof(Byte));
		if (fread(Inew,sizeof(Byte),np,Ifile) != np) {
	          fprintf(stderr,"ihsdecorr: error reading new intensity image\n");
		  exit(1);
		}
	}

   	for (p=0;p<np;p++,prgb[0]++,prgb[1]++,prgb[2]++,pout[0]++,pout[1]++,pout[2]++,Inew++) {
		/* Remember that old intensity lies in interval 0 to 255*sqrt(3) and
		                 new intensity lies in interval 0 to 255
		   Old intensity is therefore divided by sqrt(3)
		II=0.5773503*( (float)(*prgb[0])+(float)(*prgb[1])+    (float)(*prgb[2]));
		*/
		if (Iflag==1) II=(float)(*Inew);
		else
		II=0.3333333*( (float)(*prgb[0])+(float)(*prgb[1])+    (float)(*prgb[2]));
		U=0.4082483*(-(float)(*prgb[0])-(float)(*prgb[1])+2.0*(float)(*prgb[2]));
		V=0.7071068*( (float)(*prgb[0])-(float)(*prgb[1])                      );

		if (iflag==1) II=Ioff+Iamp*II;

		if (sflag == 1) {
			S=sqrt(V*V+U*U);
			Snew=Soff+Samp*S;
			U *= Snew/S;
			V *= Snew/S;
		}
		if (Sflag == 1) {
			S=sqrt(V*V+U*U);
			Snew=Soff+Samp*S*(1.0-fabs(II/127.5-1.0));
			U *= Snew/S;
			V *= Snew/S;
		}

		*pout[0] = II-0.4082483*U+0.7071068*V;
		*pout[1] = II-0.4082483*U-0.7071068*V;
		*pout[2] = II+0.8164966*U            ;
	}

	for (i=0;i<nf;i++) {
	    if (fwrite(out[i],sizeof(float),np,stdout) != np) {
	       fprintf(stderr,"ihsdecorr: error writing frame %d\n",i);
	       exit(1);
	    }
	}
exit(0);
}
