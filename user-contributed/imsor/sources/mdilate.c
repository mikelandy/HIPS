/*	Copyright (c) 1991 Jens Michael Carstensen, IMSOR 

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 *  mdilate.c - gray level dilation 
 *
 *  Usage: mdilate [nd] <inseq >outseq
 *
 *  [nd] option sets number of dilations (default 1)
 *
 *  to load: cc -o mdilate mdilate.c -lhips 
 *
 *  Input sequence must be in byte or float format.
 */

#define  Byte     unsigned char

#include <hipl_format.h>
#include <stdio.h>

int main(argc,argv)
int argc;
char **argv;
{
        struct header hd;
	int    i,j,k,nrows,ncols,npix,nf,f,nd,ndilate=1;
	Byte   *ibfr, *obfr;
	float  *iffr, *offr;

	Progname = strsave(*argv);
        if (argc > 1) ndilate=atoi(argv[1]);

	read_header(&hd);
	if (hd.pixel_format != PFBYTE && hd.pixel_format != PFFLOAT)
	   	perr(HE_MSG,"image must be in byte or float format");

	update_header(&hd,argc,argv);
	write_header(&hd);
	nrows = hd.orows;
	ncols = hd.ocols;
	nf = hd.num_frame;
	npix = nrows*ncols;

	switch (hd.pixel_format) {

	case PFBYTE:

	ibfr = (Byte *) halloc(npix,sizeof(Byte));
	obfr = (Byte *) halloc(npix,sizeof(Byte));
	hd.image=ibfr;

	/* for each frame */

	for (f = 0; f<nf; f++) {

		/* Read frame */

		read_image(&hd,f);

		/* Perform nd dilations */

		for (nd=0;nd<ndilate;nd++) {

		/* Dilation */

		/* Vertical */
		for (i=0;i<ncols;i++) obfr[i]=MAX(ibfr[i],ibfr[i+ncols]);
		for (i=1;i<nrows-1;i++) for (j=0;j<ncols;j++) {
			k=i*ncols+j;
			obfr[k]=MAX(MAX(ibfr[k],ibfr[k-ncols]),ibfr[k+ncols]);
		}
		for (i=npix-ncols;i<npix;i++) 
			obfr[i]=MAX(ibfr[i],ibfr[i-ncols]);

		/* Horizontal */
		for (i=0;i<nrows;i++) 
			ibfr[i*ncols]=MAX(obfr[i*ncols],obfr[i*ncols+1]);
		for (i=0;i<nrows;i++) for (j=1;j<ncols-1;j++) {
			k=i*ncols+j;
			ibfr[k]=MAX(MAX(obfr[k],obfr[k-1]),obfr[k+1]);
		}
		for (i=1;i<=nrows;i++) 
			ibfr[i*ncols-1]=MAX(obfr[i*ncols-1],obfr[i*ncols-2]);

		} /* Next dilation */
		
		/*  Write frame */
		write_image(&hd,f);
      	} /* Next frame */
	break;

	case PFFLOAT:

	iffr = (float *) halloc(npix,sizeof(float));
	offr = (float *) halloc(npix,sizeof(float));
	hd.image=(Byte *) iffr;

	/* for each frame */

	for (f = 0; f<nf; f++) {

		/* Read frame */

		read_image(&hd,f);

		/* Perform nd dilations */

		for (nd=0;nd<ndilate;nd++) {

		/* Dilation */

		/* Vertical */
		for (i=0;i<ncols;i++) offr[i]=MAX(iffr[i],iffr[i+ncols]);
		for (i=1;i<nrows-1;i++) for (j=0;j<ncols;j++) {
			k=i*ncols+j;
			offr[k]=MAX(MAX(iffr[k],iffr[k-ncols]),iffr[k+ncols]);
		}
		for (i=npix-ncols;i<npix;i++) 
			offr[i]=MAX(iffr[i],iffr[i-ncols]);

		/* Horizontal */
		for (i=0;i<nrows;i++) 
			iffr[i*ncols]=MAX(offr[i*ncols],offr[i*ncols+1]);
		for (i=0;i<nrows;i++) for (j=1;j<ncols-1;j++) {
			k=i*ncols+j;
			iffr[k]=MAX(MAX(offr[k],offr[k-1]),offr[k+1]);
		}
		for (i=1;i<=nrows;i++) 
			iffr[i*ncols-1]=MAX(offr[i*ncols-1],offr[i*ncols-2]);

		} /* Next dilation */
		
		/*  Write frame */
		write_image(&hd,f);
      	} /* Next frame */
	break;

	} /* End switch */
	exit(0);
}
