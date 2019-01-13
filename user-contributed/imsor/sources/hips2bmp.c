/*	Copyright (c) 1995 Jens Michael Carstensen, IMSOR 

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 *  hips2bmp.c - converts an image file from hips-format to Windows bmp-format 
 *
 *  Usage: hips2bmp [-v] <in >out
 *
 *  -v sets verbose mode
 *
 *  to load: cc -o hips2bmp hips2bmp.c -lhips 
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
	int	fmt,count,compression,planes;
	Byte	fhead[2048],*cmapstr;
	Bool	debug_flag=False;

	Progname = strsave(*argv);
	for (i=0;i<2048;i++)
		fhead[i]=0;

	for (i=1;i<argc;i++) {
           if (argv[i][0]=='-') {
		switch (argv[i][1]) {
			case 'v': 
				debug_flag=True;
				break;
			default:
				perr(HE_MSG,"hips2bmp [-v] <iseq >oseq");
		}
           }
	}

	read_header(&hd);
	nrows = hd.orows;
	ncols = hd.ocols;
	npix  = nrows * ncols;
	planes = 1;
	compression = 0;
	cmapstr = (Byte *) halloc(768,1);
	switch (hd.pixel_format) {
	case PFBYTE: 
		if (debug_flag) fprintf(stderr,"256-color bitmap\n");
		bitcount=8;
		hsize=40;
		offset=14+hsize+1024;
		ncolors=256;
		count=768;
		fmt=PFBYTE;
		if (findparam(&hd,"cmap") != NULLPAR) {
			if (debug_flag) fprintf(stderr,"Bitmap found\n");
			getparam(&hd,"cmap",fmt,&count,&cmapstr);
		}
		else {
			if (debug_flag) fprintf(stderr,"Bitmap not found\n");
			for (i=0;i<3*ncolors;i++) cmapstr[i]=i%256;
		}
		break;
	case PFRGB: 
		if (debug_flag) fprintf(stderr,"24-bit bitmap\n");
		bitcount=24;
		hsize=40;
		offset=14+hsize;
		ncolors=256;
		break;
	default:
		perr(HE_MSG,"Pixel format not supported");
	}

	fhead[0] ='B';
	fhead[1] ='M';
	fhead[10]=offset%256;
	fhead[11]=offset/256;
	fhead[14]=hsize%256;
	fhead[15]=hsize/256;
	fhead[18]=ncols%256;
	fhead[19]=ncols/256;
	fhead[22]=nrows%256;
	fhead[23]=nrows/256;
	fhead[26]=planes%256;
	fhead[27]=planes/256;
	fhead[28]=bitcount%256;
	fhead[29]=bitcount/256;
	fhead[30]=compression%256;
	fhead[31]=compression/256;
	if (bitcount!=24)
	for (i=0;i<ncolors;i++) {
		if (debug_flag) fprintf(stderr,"%3d : %3d %3d %3d\n",i,
			fhead[14+hsize+4*i+2],fhead[14+hsize+4*i+1],fhead[14+hsize+4*i]);
		fhead[14+hsize+4*i+2]=cmapstr[i];
		fhead[14+hsize+4*i+1]=cmapstr[i+ncolors];
		fhead[14+hsize+4*i]=cmapstr[i+2*ncolors];
	}
  	if (fwrite(fhead,1,offset,stdout) != offset)
		perr(HE_MSG,"error during write of fhead");

	npix = nrows * ncols;
	if (bitcount==24) npix*=3;
	rlen=(((ncols*bitcount+7)/8+3)/4)*4;
	if (debug_flag) 
		fprintf(stderr,"Size : %d %d\n",offset+rlen*nrows,rlen);
	
	ifr = (Byte *) halloc(npix,1);
	rp  = (Byte *) halloc(rlen,1);

  	if (fread(ifr,1,npix,stdin) != npix)
		perr(HE_MSG,"error during read");

	for (i=nrows-1;i>=0;i--) {
		switch (hd.pixel_format) {
		case PFBYTE:
			for (j=0;j<ncols;j++) rp[j]=ifr[i*ncols+j]; 
			break;
		case PFRGB:
			for (j=0;j<ncols;j++) {
				rp[j*3+2]=ifr[i*3*ncols+j*3  ]; 
				rp[j*3+1]=ifr[i*3*ncols+j*3+1]; 
				rp[j*3  ]=ifr[i*3*ncols+j*3+2]; 
			}
			break;
		}
  		if (fwrite(rp,1,rlen,stdout) != rlen) {
			fprintf(stderr,"error during write of line %d\n",i);
			exit(-1);
		}
	}

        return(0);
}
