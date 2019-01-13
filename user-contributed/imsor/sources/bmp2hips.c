/*	Copyright (c) 1993 Jens Michael Carstensen, IMSOR 

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 *  bmp2hips.c - converts an image file from Windows bmp-format to hips-format 
 *
 *  Usage: bmp2hips [-v] <intel-file >outseq
 *
 *  -v sets verbose mode
 *
 *  to load: cc -o bmp2hips bmp2hips.c -lhips 
 *
 */

#define  Byte     unsigned char
#define  Bool     unsigned char
#define  True		1
#define  False		0

#include <hipl_format.h>
#include <stdio.h>

int main(argc,argv)
int argc;
char **argv;
{
        struct header hd;
        int    	i,j,k;
 	Byte	*ifr,*rp;
	int	nrows,ncols,npix,nf,offset,bitcount,rlen,hsize,ncolors;
	int	cmap[256][3],pix_format;
	Byte	fhead[14],*infohead,cmapstr[768];
	Bool	debug_flag=False;

	Progname = strsave(*argv);

	nf=1;

	for (i=1;i<argc;i++) {
           if (argv[i][0]=='-') {
		switch (argv[i][1]) {
			case 'v': 
				debug_flag=True;
				break;
			default:
				perr(HE_MSG,"bmp2hips [-v] <iseq >oseq");
		}
           }
	}

  	if (fread(fhead,1,14,stdin) != 14)
		perr(HE_MSG,"error during read of fhead");
	if (fhead[0]!='B' || fhead[1]!='M')
		perr(HE_MSG,"file is not a Windows bitmap");
	offset=(int)fhead[10]+(int)fhead[11]*256;
	infohead=(Byte *) halloc(offset-14,1);
  	if (fread(infohead,1,offset-14,stdin) != offset-14)
		perr(HE_MSG,"error during read of infohead");
	hsize=(int)infohead[0]+(int)infohead[1]*256;
	ncols=(int)infohead[4]+(int)infohead[5]*256;
	nrows=(int)infohead[8]+(int)infohead[9]*256;
	bitcount=(int)infohead[14]+(int)infohead[15]*256;
	switch (bitcount) {
	case 1: 
		if (debug_flag) fprintf(stderr,"Monochrome bitmap\n");
		pix_format=PFBYTE;
		ncolors=2;
		break;
	case 4: 
		if (debug_flag) fprintf(stderr,"16-color bitmap\n");
		pix_format=PFBYTE;
		ncolors=16;
		break;
	case 8: 
		if (debug_flag) fprintf(stderr,"256-color bitmap\n");
		pix_format=PFBYTE;
		ncolors=256;
		break;
	case 24: 
		if (debug_flag) fprintf(stderr,"24-bit bitmap\n");
		pix_format=PFRGB;
		ncolors=256;
		break;
	}

	if (bitcount!=24)
	for (i=0;i<ncolors;i++) {
		cmap[i][2]=(int)infohead[hsize+4*i];
		cmap[i][1]=(int)infohead[hsize+4*i+1];
		cmap[i][0]=(int)infohead[hsize+4*i+2];
		if (debug_flag) fprintf(stderr,"%3d : %3d %3d %3d\n",i,
			cmap[i][0],cmap[i][1],cmap[i][2]);
		cmapstr[i]=infohead[hsize+4*i+2];
		cmapstr[i+ncolors]=infohead[hsize+4*i+1];
		cmapstr[i+2*ncolors]=infohead[hsize+4*i];
	}

	nf=1;
	init_header(&hd,"","",nf,"",nrows,ncols,pix_format,1,"");
	if (bitcount!=24)
		setparam(&hd,"cmap",PFBYTE,3*ncolors,cmapstr);
	update_header(&hd,argc,argv);
	write_header(&hd);
	npix = nrows * ncols;
	if (bitcount==24) npix*=3;
	rlen=(((ncols*bitcount+7)/8+3)/4)*4;
	if (debug_flag) 
		fprintf(stderr,"Size : %d %d\n",offset+rlen*nrows,rlen);
	
	ifr = (Byte *) halloc(npix,1);
	rp  = (Byte *) halloc(rlen,1);
	for (i=nrows-1;i>=0;i--) {
  		if (fread(rp,1,rlen,stdin) != rlen) {
			fprintf(stderr,"error during read of line %d\n",i);
			exit(-1);
		}
		switch (bitcount) {
		case 1:
			for (j=0;j<ncols;j+=8) for (k=0;k<8;k++) {
			  if (j<ncols-k) 
				ifr[i*ncols+j+k]=(rp[j/8] & (1<<(7-k))) ? 1 : 0;
			}
			break;
		case 4:
			for (j=0;j<ncols;j+=2) {
				ifr[i*ncols+j]=rp[j/2]>>4;
				if (j<ncols-1) ifr[i*ncols+j+1]=rp[j/2] & 0x0f;
			}
			break;
		case 8:
			for (j=0;j<ncols;j++) ifr[i*ncols+j]=rp[j]; 
			break;
		case 24:
			for (j=0;j<3*ncols;j++) ifr[i*3*ncols+j]=rp[j]; 
			for (j=0;j<ncols;j++) {
				ifr[i*3*ncols+j*3]=rp[j*3+2]; 
				ifr[i*3*ncols+j*3+1]=rp[j*3+1]; 
				ifr[i*3*ncols+j*3+2]=rp[j*3]; 
			}
			break;
		}
	}

  	if (fwrite(ifr,1,npix,stdout) != npix)
		perr(HE_MSG,"error during write");

        return(0);
}
