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
 *  rgb2ihs.c - RGB to IHS (or LUV) transformation
 *
 *  Usage: rgb2ihs -T < inseq > outseq
 *
 *	-T	transform to Taylor coordinates only (LUV)
 *
 *  to load: cc rgb2ihs.c -o rgb2ihs -lhips -lm
 *
 *  HIPS-2 03.03.93/AA
 *  (byte,) short, int, float input 12.03.93/AA
 */

#define	Byte	unsigned char

#include <hipl_format.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

char usage[]="Usage: rgb2ihs -T < inseq > outseq";

int main(argc,argv)
int argc;
char *argv[];
{
	struct	header hd;
	int	nr,nc,nf,np,i,p;
	int	sizepix;
	int	Tflag=0;
	Byte	**brgb;
	short	**srgb;
	int	**irgb;
	float	**rgb,**prgb;
	float	**ihs,**pihs;
	float	U,V,H,S;

	Progname = strsave(*argv);

	for (i=1;i<argc;i++) {
		if (argv[i][0] == '-')
		switch (argv[i][1]) {
		case 'T': Tflag++; break;
		case 'D': break;
		default: perr(HE_MSG,usage); break;
		}
        }

	read_header(&hd);
	nf = hd.num_frame;
	if (nf != 3) perr(HE_MSG,"implemented for 3-frame sequence only");
	nr = hd.orows;
	nc = hd.ocols;
	np = nr * nc;

	if      (hd.pixel_format == PFBYTE ) sizepix = sizeof(Byte);
	else if (hd.pixel_format == PFSHORT) sizepix = sizeof(short);
	else if (hd.pixel_format == PFINT  ) sizepix = sizeof(int);
	else if (hd.pixel_format == PFFLOAT) sizepix = sizeof(float);
	else perr(HE_MSG,"format must be byte, short, int or float");

	rgb = (float **) halloc(nf,sizeof(float *));
	prgb= (float **) halloc(nf,sizeof(float *));
	ihs = (float **) halloc(nf,sizeof(float *));
	pihs= (float **) halloc(nf,sizeof(float *));

	/* read frame */
	for (i=0;i<nf;i++) {
	    ihs[i] = (float *) halloc(np,sizeof(float));
	    rgb[i] = (float *) halloc(np,sizeof(float));
	    if (hd.pixel_format == PFBYTE) {
	        brgb= (Byte  **) halloc(nf,sizeof(Byte  *));
		brgb[i] = (Byte *) halloc(np,sizepix);
		if (fread(brgb[i],sizepix,np,stdin) != np) {
	       	   fprintf(stderr,"%s: error reading frame %d\n",Progname,i);
		   exit(1);
		}
	        for (p=0;p<np;p++) rgb[i][p] = (float)brgb[i][p];
	    }
	    else if (hd.pixel_format == PFSHORT) {
	        srgb= (short **) halloc(nf,sizeof(short *));
		srgb[i] = (short *) halloc(np,sizepix);
		if (fread(srgb[i],sizepix,np,stdin) != np) {
	       	   fprintf(stderr,"%s: error reading frame %d\n",Progname,i);
		   exit(1);
		}
	        for (p=0;p<np;p++) rgb[i][p] = (float)srgb[i][p];
	    }
	    else if (hd.pixel_format == PFINT) {
	        irgb = (int   **) halloc(nf,sizeof(int   *));
		irgb[i] = (int *) halloc(np,sizepix);
		if (fread(irgb[i],sizepix,np,stdin) != np) {
	       	   fprintf(stderr,"%s: error reading frame %d\n",Progname,i);
		   exit(1);
		}
	        for (p=0;p<np;p++) rgb[i][p] = (float)irgb[i][p];
	    }
	    else {
		if (fread(rgb[i],sizepix,np,stdin) != np) {
	       	   fprintf(stderr,"%s: error reading frame %d\n",Progname,i);
		   exit(1);
		}
	    }
   	    pihs[i] = ihs[i];
   	    prgb[i] = rgb[i];
	}
		
	/* RGB to LUV transformation (Taylor coordinates) */
   	for (p=0;p<np;p++,prgb[0]++,prgb[1]++,prgb[2]++,pihs[0]++,pihs[1]++,pihs[2]++) {
		*pihs[0]=0.5773503*( (float)(*prgb[0])+(float)(*prgb[1])+    (float)(*prgb[2]));
		U       =0.4082483*(-(float)(*prgb[0])-(float)(*prgb[1])+2.0*(float)(*prgb[2]));
		V       =0.7071068*( (float)(*prgb[0])-(float)(*prgb[1])                      );
		V=-V;

		/* LUV to IHS transformation (Munsell coordinates) */
		if (Tflag == 0) {
			/*
			if (fabs(U)<0.000001) H=1.5707963; else H=atan(V/U);
			if (V<0.0) H=H+3.1415927;
			*/
			if (fabs(U)<0.000001) {
				if (V>0.0) H= 1.5707963;
				else       H=-1.5707963;
				if (fabs(V)<0.000001) H= 0.0;
			}
			else H=atan2(V,U);
			if (H<0.0) H=H+6.2831854;
			S=sqrt(V*V+U*U);
			*pihs[1] = H;
			*pihs[2] = S;
		}
		else {
			*pihs[1] = U;
			*pihs[2] = V;
		}
	}

	hd.pixel_format = PFFLOAT;
	hd.numcolor=3;
	update_header(&hd,argc,argv);
	write_header(&hd);

	/* write new pixels */
	for (i=0;i<nf;i++) {
		if(fwrite(ihs[i],sizeof(float),np,stdout) != np) {
		  fprintf(stderr,"%s: error writing frame %d\n",Progname,i);
		  exit(1);
		}
	}
exit(0);
}
