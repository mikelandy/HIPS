/*	Copyright (c) 1990 Jens Michael Carstensen, IMSOR 

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 *  patchwork.c - patch one image from a sequence 
 *
 *  Usage: patchwork [-ce] <inseq >outseq
 *
 *  [-c npics] option merges npics images per row
 *
 *  [-e] option embeds frame 1 in frame 0
 *
 *  to load: cc -o patchwork patchwork.c -lhips
 *
 */

#define  Byte     unsigned char

#include <hipl_format.h>
#include <stdio.h>
#include <math.h>

int main(argc,argv)
int argc;
char **argv;
{
        struct header hd, hdo;
        int    i,j,f,nf,npix,nrows,ncols,orows,ocols,npics;
	int    mflag=2, eflag=0;
	Byte   *bp,**ifr,gray;

	Progname = strsave(*argv);
	for (i=1;i<argc;i++) {
           if (argv[i][0]=='-') {
		switch (argv[i][1]) {
			case 'p':
			case 'c': mflag=atoi(argv[++i]);
				  break;
			case 'g': gray=atoi(argv[++i]);
				  break;
			case 'e': eflag=1;
				  mflag=0;
				  break;
			default:
				perr(HE_MSG,"patchwork [-ce] < inseq > outseq");
		}
           }
	}
	read_header(&hd);
	if (hd.pixel_format != PFBYTE)
	   	perr(HE_MSG,"image must be in byte format");
	if (hd.num_frame <2)
		perr(HE_MSG,"minimum 2 frames required");
	nf = hd.num_frame;
	npics = (mflag) ? mflag : 2;
	nrows = hd.orows;
	ncols = hd.ocols;
	npix  = nrows * ncols;
	orows = (mflag) ? ((nf+npics-1)/npics)*nrows : nrows;
	ocols = (mflag) ? ncols*npics : ncols;
	
	dup_headern(&hd,&hdo);
        setsize(&hdo,orows,ocols);
	hdo.num_frame = 1;
        write_headeru2(&hd,&hdo,argc,argv,FALSE);

	ifr = (Byte **) halloc(npics,sizeof(Byte *));
	for (i=0;i<npics;i++) 
		ifr[i] = (Byte *) halloc(npix,sizeof(Byte));

	/* Main */

	if (mflag) {

        for (f=0;f<nf/npics;f++) {
		for (i=0;i<npics;i++)
			if (fread(ifr[i],1,npix,stdin)!=npix)
				perr(HE_MSG,"error during read");
		for (i=0;i<nrows;i++) for (j=0;j<npics;j++)
			if (fwrite(ifr[j]+i*ncols,1,ncols,stdout)!=ncols)
				perr(HE_MSG,"error during write");
	}		
	if (nf%npics) {
		for (i=0;i<nf%npics;i++)
			if (fread(ifr[i],1,npix,stdin)!=npix)
				perr(HE_MSG,"error during read");
		for (i=nf%npics;i<npics;i++) for (j=0;j<npix;j++) ifr[i][j]=gray;
		for (i=0;i<nrows;i++) for (j=0;j<npics;j++)
			if (fwrite(ifr[j]+i*ncols,1,ncols,stdout)!=ncols)
				perr(HE_MSG,"error during write");
	}
	} /* end mflag */

	if (eflag) {

	for (i=0;i<2;i++)
		if (fread(ifr[i],1,npix,stdin)!=npix)
			perr(HE_MSG,"error during read");
        for (i=nrows/4;i<3*nrows/4;i++)
		for (j=ncols/4;j<3*ncols/4;j++) 
			*(ifr[0]+i*ncols+j)= *(ifr[1]+i*ncols+j);
	if (fwrite(ifr[0],1,npix,stdout)!=npix)
		perr(HE_MSG,"error during write");
	}
        return(0);
}

